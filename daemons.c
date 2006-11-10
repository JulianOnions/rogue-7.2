/*
 * All the daemon and fuse functions are in here
 *
 * @(#)daemons.c	7.0	(Bell Labs)	10/08/82
 */

#include "rogue.h"
#include "rogue_ext.h"


/*
 * doctor:
 *	A healing daemon that restores hit points after rest
 */

extern int isring (int hand, int ring);
extern int daemon (int (*func) (/* ??? */), int arg, int type);
extern int roll (int number, int sides);
extern int wanderer (void);
extern int kill_daemon (int (*func) (/* ??? */));
extern int fuse (int (*func) (/* ??? */), int arg, int time, int type);
extern void msg (const char *fmt, ...);
extern int extinguish (int (*func) (/* ??? */));
extern int light (struct coord *cp);
extern int ring_eat (int hand);
extern int updpack (int getmax);
extern int wghtchk (int type);
extern int chg_abil (register int what, register int amt, register int how);
extern void debug (char *errstr);
extern int rnd_room (void);
extern int rnd_pos (struct room *rp, struct coord *cp);
extern int winat (int y, int x);

doctor(int fromfuse)
{
	reg int lv, ohp, ccon;

	lv = pstats.s_lvl;
	ohp = pstats.s_hpt;
	quiet += 1;

	ccon = pstats.s_ef.a_con;
	if (ccon > 15)
		pstats.s_hpt += rnd(ccon - 14);
	if (lv < 8) {
	    if (quiet > 20 - lv * 2)
		pstats.s_hpt += 1;
	}
	else {
	    if (quiet >= 3)
		pstats.s_hpt += rnd(lv - 7) + 1;
	}
	if (isring(LEFT, R_REGEN))
		pstats.s_hpt += 1;
	if (isring(RIGHT, R_REGEN))
		pstats.s_hpt += 1;
	if (pl_on(ISREGEN))
		pstats.s_hpt += 1;
	if (ohp != pstats.s_hpt) {
	    nochange = FALSE;
	    if (pstats.s_hpt > max_hp)
		pstats.s_hpt = max_hp;
	    quiet = 0;
	}
}


/*
 * Swander:
 *	Called when it is time to start rolling for wandering monsters
 */
swander(int fromfuse)
{
	daemon(rollwand, TRUE, BEFORE);
}


/*
 * rollwand:
 *	Called to ng monster starts up
 */
rollwand(int fromfuse)
{
	static int between = 0;

	if (++between >= 4) {
	    if (roll(1, 6) == 4) {
		if (!tradelev)		/* no monsters for posts */
		    wanderer();
		kill_daemon(rollwand);
		fuse(swander, TRUE, WANDERTIME, BEFORE);
	    }
	    between = 0;
	}
}


/*
 * unconfuse:
 *	Release the poor player from his confusion
 */
unconfuse(int fromfuse)
{
	if (pl_on(ISHUH))
	    msg("You feel less confused now");
	player.t_flags &= ~ISHUH;
}


/*
 * unsee:
 *	He lost his see invisible power
 */
unsee(int fromfuse)
{
	player.t_flags &= ~CANSEE;
}


/*
 * sight:
 *	He gets his sight back
 */
sight(int fromfuse)
{
	int sight(int fromfuse);

	if (pl_on(ISBLIND)) {
	    extinguish(sight);
	    light(&hero);
	    msg("The veil of darkness lifts");
	}
	player.t_flags &= ~ISBLIND;
}


/*
 * nohaste:
 *	End the hasting
 */
nohaste(int fromfuse)
{
	if (pl_on(ISHASTE))
	    msg("You feel yourself slowing down.");
	player.t_flags &= ~ISHASTE;
}


/*
 * digest the hero's food
 */
stomach(int fromfuse)
{
	reg int oldfood, old_hunger;

	old_hunger = hungry_state;
	if (food_left <= 0) {		 /* the hero is fainting */
	    if (no_command > 0 || rnd(100) > 20)
		return;
	    no_command = rnd(8)+4;
	    msg("You faint");
	    running = FALSE;
	    count = 0;
	    hungry_state = F_FAINT;
	    nochange = FALSE;
	}
	else {
	    oldfood = food_left;
	    food_left -= ring_eat(LEFT)+ring_eat(RIGHT) + foodlev - amulet;
	    if(no_command > 0)	/* wait till he can move */
		return;
	    if (food_left < MORETIME && oldfood >= MORETIME) {
		msg("You are starting to feel weak");
		hungry_state = F_WEAK;
	    }
	    else if(food_left < 2*MORETIME && oldfood >= 2*MORETIME) {
		msg("Getting hungry");
		hungry_state = F_HUNGRY;
	    }
	}
	if(old_hunger != hungry_state) {
	    nochange = FALSE;
	    updpack(FALSE);		/* new pack weight */
	}
	wghtchk(FALSE);
}

/*
 * noteth:
 *	Hero is no longer etherereal
 */
noteth(int fromfuse)
{
	if (pl_on(ISETHREAL))
	    msg("You begin to feel more corporeal.");
	player.t_flags &= ~ISETHREAL;
}

/*
 * sapem:
 *	Sap the hero's life away
 */
sapem(int fromfuse)
{

	chg_abil(rnd(4) + 1, -1, TRUE);
	fuse(sapem,TRUE,150,AFTER);
	nochange = FALSE;
}

/*
 * notslow:
 *	Restore the hero's normal speed
 */
notslow(int fromfuse)
{
	player.t_flags &= ~ISSLOW;
}

/*
 * notfight:
 *	Hero is no longer fighting
 */
notfight(int fromfuse)
{
	isfight = FALSE;
}

/*
 * notregen:
 *	Hero is no longer regenerative
 */
notregen(int fromfuse)
{
	if (pl_on(ISREGEN))
	    msg("You no longer feel bolstered.");
	player.t_flags &= ~ISREGEN;
}


/*
 * notinvinc:
 *	Hero not invincible any more
 */
notinvinc(int fromfuse)
{
	if (pl_on(ISINVINC))
	    msg("You no longer feel invincible.");
	player.t_flags &= ~ISINVINC;
}


/*
 * chkstairs:
 *	Check that the stariway down is in a room. If not, then
 *	put it somewhere.
 */
chkstairs(int fromfuse)
{
	reg int cnt, rm;
	int chkstairs(int fromfuse);

	if(roomin(&stairs) == NULL) {
	    sprintf(errbuf,"Stairs not in a room: %d",fromfuse);
	    debug(errbuf);
	    cnt = 0;
	    do {
		rm = rnd_room();
		rnd_pos(&rooms[rm], &stairs);
		if(++cnt > 500)		/* use this spot anyway */
		    break;
	    } until (winat(stairs.y, stairs.x) == FLOOR);
	    mvaddch(stairs.y,stairs.x,STAIRS);
	}
	fuse(chkstairs,TRUE,500,AFTER);
}
