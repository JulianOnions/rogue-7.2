/*
 * File with various monster functions in it
 *
 * @(#)monsters.c	7.0	(Bell Labs)	10/8/82
 */

#include "rogue.h"
#include <ctype.h>
#include "rogue_ext.h"


/*
 * randmonster:
 *	Pick a monster to show up.  The lower the level,
 *	the meaner the monster.
 */


extern void debug (char *errstr);
extern int _attach (register struct linked_list **list, register struct linked_list *item);
extern int roll (int number, int sides);
extern int iswearing (int ring);
extern int runto (struct coord *runner, struct coord *spot);
extern int rnd_room (void);
extern int rnd_pos (struct room *rp, struct coord *cp);
extern int step_ok (char ch);
extern void msg (const char *fmt, ...);
extern int save (int which);
extern int lengthen (int (*func) (/* ??? */), int xtime);
extern int fuse (int (*func) (/* ??? */), int arg, int time, int type);
extern int readchar (void);
extern int removelist (struct coord *mp, struct linked_list *item);

randmonster(NCURSES_BOOL wander, NCURSES_BOOL baddie)
            
            		/* TRUE when from a polymorph stick */
{
	reg int i, oktocreate;

	if(baddie) {
	    while(1) {			/* do until return */
		i = rnd(MAXMONS);	/* pick ANY monster */

		/*
		 * If this monster has been genocided,
		 * then don't create it.
		 */
		if(monsters[i].m_lev.l_lev < 0 ||
		   monsters[i].m_lev.h_lev < 0)
			continue;
		if(i < (MAXMONS / 2))
		    return(i + 'A');		/* 1st 26 monsters */
		else
		    return(i + 'a' - (MAXMONS/2));   /* 2nd 26 monsters */
	    }
	}
	oktocreate = FALSE;	/* assume this monster can't be made */
	do {
		/*
		 * get a random monster from this range
		 */
		i = rnd(levcount);
		/*
		 * Only get non-genocided monsters
		 */
		if (mtlev[i] != NULL) {
		    /*
		     * Only create a wandering monster if
		     * we want one
		     */
		    if (!wander || mtlev[i]->m_lev.d_wand)
			oktocreate = TRUE;
		}
	} while(!oktocreate);
	return(mtlev[i]->m_show);	/* get the char of the monster */
}

/*
 * mon_index:
 *	This returns an index to 'whichmon'
 */
mon_index(char whichmon)
{
	if (!isalpha(whichmon)) {
	    sprintf(errbuf, "bad monster type in mon_index: %s.",
		unctrl(whichmon));
	    debug(errbuf);
	    return(1);	/* return a bat */
	}
	if (isupper(whichmon))
		return(whichmon - 'A');	/* 0 to 25 for uppercase */
	else
		return(whichmon - 71);	/* 26 to 51 for lowercase */
}

/*
 * lev_mon:
 *	This gets all monsters possible on this level
 */
lev_mon(void)
{
    reg int i,lev;

    levcount = 0;
    lev = level > 0 ? (level < 100 ? level : 99) : 1;
    for (i = 0 ; i < MAXMONS ; i++) {
	if (monsters[i].m_lev.h_lev >= lev &&
	    monsters[i].m_lev.l_lev <= lev) {
		mtlev[levcount] = &monsters[i];
		if (++levcount >= MONRANGE)
		    break;
	}
    }
    if (levcount == 0) {		/* if no monsters are possible */
	mtlev[0] = &monsters[1];	/* bats 'B' */
	sprintf(errbuf,"No monsters on level %d",level);
	debug(errbuf);
    }
}

/*
 * new_monster:
 *	Pick a new monster and add it to the list
 */
new_monster(struct linked_list *item, char type, struct coord *cp, NCURSES_BOOL treas)
{
    reg struct thing *tp;
    reg struct monster *mp;
    float killexp;		/* experience gotten for killing him */

    attach(mlist, item);
    tp = THINGPTR(item);
    tp->t_type = type;
    tp->t_pos = *cp;
    tp->t_oldch = mvwinch(cw, cp->y, cp->x);
    mvwaddch(mw, cp->y, cp->x, tp->t_type);
    mp = &monsters[mon_index(tp->t_type)];
    tp->t_stats = mp->m_stats;		/* copy monster data */
    /*
     * Get hit points for monster depending on his experience
     */
    tp->t_stats.s_hpt = roll(mp->m_stats.s_lvl, 8);

    /*
     * Adjust experience point we get for killing it by the
     *  strength of this particular monster by ~~ +- 50%
     */
    killexp = mp->m_stats.s_exp * (0.47 + (float)tp->t_stats.s_hpt /
		(8 * (float)tp->t_stats.s_lvl));

    tp->t_stats.s_exp = killexp;	/* use float for accuracy */
    if(tp->t_stats.s_exp < 1)
	tp->t_stats.s_exp = 1;		/* minimum 1 experience point */
    tp->t_flags = mp->m_flags;
    /*
     * If monster in treasure room, then MEAN
     */
    if(treas)
	tp->t_flags |= ISMEAN;
    tp->t_turn = TRUE;
    tp->t_pack = NULL;
    /*
     * Dont wander if treas room
     */
    if (iswearing(R_AGGR) && !treas)
	runto(cp, &hero);
    if (type == 'M') {
	char mch;

	if (tp->t_pack != NULL)
	    mch = (OBJPTR(tp->t_pack))->o_type;
	else
	    switch (rnd(level >= AMLEVEL ? 9 : 8)) {
		case 0: mch = GOLD;
		when 1: mch = POTION;
		when 2: mch = SCROLL;
		when 3: mch = STAIRS;
		when 4: mch = WEAPON;
		when 5: mch = ARMOR;
		when 6: mch = RING;
		when 7: mch = STICK;
		when 8: mch = AMULET;
	    }
	tp->t_disguise = mch;
    }
}

/*
 * wanderer:
 *	A wandering monster has awakened and is headed for the player
 */

wanderer(void)
{
    reg int i, ch;
    reg struct room *rp, *hr = roomin(&hero);
    reg struct linked_list *item;
    reg struct thing *tp;
    struct coord cp;

    do {
	i = rnd_room();
	if ((rp = &rooms[i]) == hr)
	    continue;
	rnd_pos(rp, &cp);
	ch = mvwinch(stdscr, cp.y, cp.x);
    } until(hr != rp && step_ok(ch));
    item = new_item(sizeof *tp);
    new_monster(item, randmonster(TRUE,FALSE), &cp, FALSE);
    tp = THINGPTR(item);
    tp->t_flags |= ISRUN;
    tp->t_pos = cp;
    tp->t_dest = &hero;
    if (wizard)
     msg("Started a wandering %s",monsters[mon_index(tp->t_type)].m_name);
}

/*
 * what to do when the hero steps next to a monster
 */
struct linked_list *
wake_monster(int y, int x)
{
    reg struct thing *tp;
    reg struct linked_list *it;
    reg struct room *rp;
    reg char ch;

    if ((it = find_mons(y, x)) == NULL) {
	return NULL;
    }
    tp = THINGPTR(it);
    ch = tp->t_type;
    /*
     * Every time he sees mean monster, it might start chasing him
     */
    rp = roomin(&hero);
    if (rp != NULL && (rnd(100) > 33 || rp->r_flags & ISTREAS)
      && on(*tp, ISMEAN) && off(*tp, ISHELD) && !iswearing(R_STEALTH)) {
	tp->t_flags |= ISRUN;
	tp->t_dest = &hero;
    }
    if(ch == 'U' && pl_off(ISBLIND)) {
        rp = roomin(&hero);
	if ((rp != NULL && !(rp->r_flags & ISDARK))
	    || DISTANCE(y, x, hero.y, hero.x) < 3) {
	    if(off(*tp,ISFOUND) && !save(VS_PETRIFICATION)
	      && !iswearing(R_SUSAB) && pl_off(ISINVINC)) {
		msg("The umber hulk's gaze has confused you.");
		if (pl_on(ISHUH))
		    lengthen(unconfuse,rnd(20)+HUHDURATION);
		else
		    fuse(unconfuse,TRUE,rnd(20)+HUHDURATION,AFTER);
		player.t_flags |= ISHUH;
	    }
	    tp->t_flags |= ISFOUND;
	}
    }
    /*
     * Hide invisible monsters
     */
    if ((tp->t_flags & ISINVIS) && pl_off(CANSEE))
	ch = mvwinch(stdscr, y, x);
    /*
     * Let greedy ones guard gold
     */
    if (on(*tp, ISGREED) && off(*tp, ISRUN))
	if (rp != NULL && rp->r_goldval) {
	    tp->t_dest = &rp->r_gold;
	    tp->t_flags |= ISRUN;
	}
    return it;
}

/*
 * genocide:
 *	Eradicate a monster forevermore
 */
genocide(void)
{
    reg struct linked_list *ip;
    reg struct thing *mp;
    reg char c;
    reg int i, ii;
    reg struct linked_list *nip;

    i = TRUE;		/* assume an error now */
    while (i) {
	msg("Which monster (remember UPPER & lower case)?");
	c = readchar();		/* get a char */
	if (c == ESCAPE) {	/* he can abort (the fool) */
	    msg("");
	    return;
	}
	if(isalpha(c))		/* valid char here */
	    i = FALSE;		/* exit the loop */
	else {			/* he didn't type a letter */
	    mpos = 0;
	    msg("Please specify a letter between 'A' and 'z'");
	}
    }
    for (ip = mlist; ip; ip = nip) {
	mp = THINGPTR(ip);
	nip = next(ip);
	if (mp->t_type == c)
	    removelist(&mp->t_pos, ip);
    }
    i = mon_index(c);			/* get index to monster */
    monsters[i].m_lev.l_lev = -1;	/* get rid of it */
    monsters[i].m_lev.h_lev = -1;
    for(ii = 0;ii < MONRANGE;ii++) {	/* Make sure that this */
	if(mtlev[ii] == &monsters[i]) {	/* monster can't appear */
		mtlev[ii] = NULL;	/* on this level anymore */
		break;
	}
    }
    mpos = 0;
    msg("You have wiped out the %s.",monsters[i].m_name);
}
