/*
 * routines dealing specifically with rings
 *
 * @(#)rings.c	7.0	(Bell Labs)	10/08/82
 */

#include "rogue.h"
#include "rogue_ext.h"


/*
 * ring_on:
 *	Put on a ring
 */

extern void msg (const char *fmt, ...);
extern int is_current (struct object *obj);
int gethand (void);
extern int setoflg (struct object *what, int bit);
extern int add_haste (NCURSES_BOOL potion);
extern int chg_abil (register int what, register int amt, register int how);
extern int light (struct coord *cp);
extern int aggravate (void);
extern int updpack (int getmax);
extern int look (NCURSES_BOOL wakeup);
extern int fuse (int (*func) (/* ??? */), int arg, int time, int type);
extern int get_str (register char *opt, WINDOW *awin);
extern int dropcheck (struct object *op);
extern int extinguish (int (*func) (/* ??? */));
extern int sight (int fromfuse);
int ringabil (void);
extern int readchar (void);
extern int o_on (struct object *what, int bit);
int magring (struct object *what);
extern int getbless (void);
int isring (int hand, int ring);

ring_on(void)
{
    reg struct object *obj;
    reg struct linked_list *item;
    reg int ring, wh;
    char buf[LINLEN];

    /*
     * Make certain that it is somethings that we want to wear
     */
    if ((item = get_item("put on", RING)) == NULL)
	return;

    obj = OBJPTR(item);
    if (obj->o_type != RING) {
	msg("That won't fit on your finger.");
	return;
    }

    /*
     * find out which hand to put it on
     */
    if (is_current(obj))
	return;

    if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL) {
	if ((ring = gethand()) < 0)
	    return;
    }
    else if (cur_ring[LEFT] == NULL)
	ring = LEFT;
    else if (cur_ring[RIGHT] == NULL)
	ring = RIGHT;
    else  {
	msg("Already wearing two rings");
	return;
    }
    cur_ring[ring] = obj;
    wh = obj->o_which;

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch (wh) {
	case R_SPEED:
	    if(--obj->o_ac < 0) {	/* see how many charges */
		obj->o_ac = 0;		/* ring has. if none, then */
		setoflg(obj,ISCURSED);
	    }				/* curse it to cause trouble */
	    else {			/* otherwise a good ring still */
		add_haste(FALSE);	/* add haste from ring */
		msg("You find yourself moving must faster");
	    }
	when R_GIANT:			/* to 24 */
	case R_ADDSTR:
	    chg_abil(STR,obj->o_ac,FROMRING);
	when R_KNOW:
	    chg_abil(WIS,obj->o_ac,FROMRING);
	when R_DEX:
	    chg_abil(DEX,obj->o_ac,FROMRING);
	when R_SEEINVIS:
	    player.t_flags |= CANSEE;
	    light(&hero);
	    mvwaddch(cw, hero.y, hero.x, PLAYER);
	when R_AGGR:
	    aggravate();
	when R_HEAVY:
	    updpack(FALSE);		/* new pack weight */
	when R_BLIND:
	    r_know[R_BLIND] = TRUE;
	    player.t_flags |= ISBLIND;
	    look(FALSE);
	when R_SLOW:
	    player.t_flags |= ISSLOW;
	when R_SAPEM:
	    fuse(sapem,TRUE,150,AFTER);
	when R_LIGHT: {
		struct room *rop;
		r_know[R_LIGHT] = TRUE;
		if((rop = roomin(&hero)) != NULL) {
			rop->r_flags &= ~ISDARK;
			light(&hero);
			mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
	}
    }
    if(r_know[wh] && r_guess[wh]) {
	FREE(r_guess[wh]);
	r_guess[wh] = NULL;
    }
    else if(!r_know[wh] && r_guess[wh] == NULL) {
	mpos = 0;
	buf[0] = 0;
	msg("Call it: ");
	if (get_str(buf, cw) == NORM) {
	    r_guess[wh] = new(strlen(buf) + 1);
	    strcpy(r_guess[wh], buf);
	}
    }
    mpos = 0;
    msg("Now wearing %s",inv_name(obj,TRUE));
}


/*
 * ring_off:
 *	Take off some ring
 */
ring_off(void)
{
    reg int ring;
    reg struct object *obj;

    if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL) {
	msg("You're not wearing any rings.");
	return;
    }
    else if (cur_ring[LEFT] == NULL)
	ring = RIGHT;
    else if (cur_ring[RIGHT] == NULL)
	ring = LEFT;
    else
	if ((ring = gethand()) < 0)
	    return;
    mpos = 0;
    obj = cur_ring[ring];
    if (obj == NULL) {
	msg("Not wearing such a ring.");
	return;
    }
    if (dropcheck(obj))
	msg("Was wearing %s", inv_name(obj, TRUE));
    nochange = FALSE;
}


/*
 * toss_ring:
 *	Remove a ring and stop its effects
 */
toss_ring(struct object *what)
{
	cur_ring[what == cur_ring[LEFT] ? LEFT : RIGHT] = NULL;
	switch (what->o_which) {
	    case R_SPEED:
		extinguish(nohaste);	/* kill fuse */
		nohaste(FALSE);		/* end hasting */
	    when R_BLIND: sight(FALSE);
	    when R_SLOW:  player.t_flags &= ~ISSLOW;
	    when R_SAPEM: extinguish(sapem);
	    when R_GIANT: pstats.s_ef.a_str = pstats.s_re.a_str;
			  ringabil();
	    when R_ADDSTR:
		chg_abil(STR,-what->o_ac,FALSE);
	    when R_KNOW:
		chg_abil(WIS,-what->o_ac,FALSE);
	    when R_DEX:
		chg_abil(DEX,-what->o_ac,FALSE);
	    when R_SEEINVIS:
		player.t_flags &= ~CANSEE;
		extinguish(unsee);
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	}
}


/*
 * gethand:
 *	Get a hand to wear a ring
 */
gethand(void)
{
    reg int c;

    for (;;) {
	msg("Left or Right ring? ");
	if ((c = lower(readchar())) == 'l')
	    return LEFT;
	else if (c == 'r')
	    return RIGHT;
	else if (c == ESCAPE)
	    return -1;
	mpos = 0;
	msg("L or R");
    }
}

/*
 * ring_eat:
 *	How much food does this ring use up?
 */
ring_eat(int hand)
{
    if (cur_ring[hand] == NULL)
	return 0;
    switch (cur_ring[hand]->o_which) {
	case R_REGEN:
	case R_GIANT:
	    return 2;
	case R_SPEED:
	case R_SUSTSTR:
	case R_SUSAB:
	    return 1;
	case R_SEARCH:
	    return (rnd(100) < 33);
	case R_DIGEST:
	    switch(cur_ring[hand]->o_ac) {
		case -3: if(rnd(100) < 25) return 3;
		case -2: if(rnd(100) < 50) return 2;
		case -1: return 1;
		case  0: return -(rnd(100) < 50);
		case  3: if(rnd(100) < 25) return -3;		
		case  2: if(rnd(100) < 50) return -2;
		case  1:
		default: return -1;
	    }
	default:
	    return 0;
    }
}


/*
 * ring_num:
 *	Print ring bonuses
 */
char *
ring_num(struct object *what)
{
    static char number[5];

    number[0] = 0;
    if (o_on(what,ISKNOW)) {	/* only if hero knows */
	if (magring(what)) {	/* only rings with numbers */
	    number[0] = ' ';
	    strcpy(&number[1], num(what->o_ac, 0));
	}
    }
    return number;
}


/*
 * magring:
 *	Returns TRUE if a ring has a number, i.e. +2
 */
magring(struct object *what)
{
	switch(what->o_which) {
		case R_SPEED:
		case R_TELEPORT:
		case R_ADDSTR:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDDAM:
		case R_DIGEST:
		case R_KNOW:
		case R_DEX:
			return TRUE;
		default:
			return FALSE;
	}
}


/*
 * ringabil:
 *	Compute effective abilities due to rings
 */
ringabil(void)
{
	reg struct object *rptr;
	reg int i;

	for(i = LEFT; i <= RIGHT; i++) {
	    rptr = cur_ring[i];
	    if(rptr != NULL) {
		switch(rptr->o_which) {
		    case R_ADDSTR:
		    case R_GIANT:
			chg_abil(STR,rptr->o_ac,FROMRING);
		    when R_DEX: 
			chg_abil(DEX,rptr->o_ac,FROMRING);
		    when R_KNOW:
			chg_abil(WIS,rptr->o_ac,FROMRING);
		}
	    }
	}
}


/*
 * init_ring:
 *	Initialize a ring
 */
init_ring(struct object *what, NCURSES_BOOL fromwiz)
                    
             			/* TRUE when from wizards */
{
	reg int much;
	switch (what->o_which) {
	    case R_DIGEST:		/* -3 to +3 rings */
	    case R_ADDSTR:
	    case R_PROTECT:
	    case R_ADDHIT:
	    case R_ADDDAM:
	    case R_DEX:
	    case R_KNOW:
		if (fromwiz) {
		    much = getbless();		/* get wizards response */
		}
		else {				/* normal users */
		    if (rnd(100) < 25)
			much = -rnd(3) - 1;
		    else
			much = rnd(3) + 1;
		}
		what->o_ac = much;
		if (much < 0)
		    setoflg(what,ISCURSED);
	    when R_SPEED:
		what->o_ac = rnd(4) + 1;
	    when R_AGGR:
	    case R_DELUS:
	    case R_HEAVY:
	    case R_BLIND:
	    case R_SLOW:
	    case R_SAPEM:
	    case R_NOP:
		setoflg(what,ISCURSED);	
	    when R_TELEPORT:
		setoflg(what,ISCURSED);
		what->o_ac = rnd(7) - 3;	/* +3 to -3 */
	    when R_GIANT:
		what->o_ac = 50;		/* lots !! of STR */
	    otherwise:
		what->o_ac = 1;
	}
	what->o_weight = things[TYP_RING].mi_wght;
}


/*
 * iswearing:
 *	Returns TRUE when the hero is wearing a certain type of ring
 */
iswearing(int ring)
{
	return (isring(LEFT,ring) || isring(RIGHT,ring));
}

/*
 * isring:
 *	Returns TRUE if a ring is on a hand
 */
isring(int hand, int ring)
{
	if (cur_ring[hand] != NULL && cur_ring[hand]->o_which == ring)
		return TRUE;
	return FALSE;
}
