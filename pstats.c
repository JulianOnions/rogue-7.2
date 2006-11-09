/*
 * Players status routines
 *
 * @(#)pstats.c	(Bell Labs)	10/08/82
 *
 */

#include "rogue.h"
#include "rogue.ext"


/*
 * chg_hpt:
 *	Changes players hit points
 */
chg_hpt(howmany,alsomax,whichmon)
int howmany;
char whichmon;
bool alsomax;
{
	nochange = FALSE;
	if(alsomax)
		max_hp += howmany;
	pstats.s_hpt += howmany;
	if(pstats.s_hpt < 1)
		death(whichmon);
}


/*
 * rchg_str:
 *	Update the players real strength 
 */
rchg_str(amt)
int amt;
{
	chg_abil(STR,amt,TRUE);
}

/*
 * chg_abil:
 *	Used to modify the hero's abilities
 */
chg_abil(what,amt,how)
register int amt, what, how;
{
	if (amt == 0)
	    return;
	if (how == TRUE) {			/* real (must be 1st) */
	    updabil(what,amt,&pstats.s_re,TRUE);
	    how = FALSE;
	}
	updabil(what,amt,&pstats.s_ef,how);	/* effective */
	updpack(TRUE);
	wghtchk(FALSE);
	nochange = FALSE;
}

/*
 * updabil:
 *	Do the actual abilities updating
 */
updabil(what, amt, pst, how)
register struct real *pst;
register int what, amt, how;
{
	register int *wh, *mx, *mr;
	struct real *mst, *msr;
	bool is_str = FALSE;

	msr = &pstats.s_re;
	if (how == TRUE)			/* max real abilities */
	    mst = &max_stats.s_re;
	else					/* max effective abil */
	    mst = &max_stats.s_ef;
	switch (what) {
	    case STR:	is_str = TRUE;
			wh = &pst->a_str;
			mx = &mst->a_str;
			mr = &msr->a_str;
	    when DEX:	wh = &pst->a_dex;
			mx = &mst->a_dex;
			mr = &msr->a_dex;
	    when CON:	wh = &pst->a_con;
			mx = &mst->a_con;
			mr = &msr->a_con;
	    when WIS:	wh = &pst->a_wis;
			mx = &mst->a_wis;
			mr = &msr->a_wis;
	    otherwise:	return;
	}
	*wh += amt;			/* update by amt */
	if(amt < 0) {			/* if decrement */
	    if (*wh < MINABIL)		/* minimum = 3 */
		*wh = MINABIL;
	    if (how == FALSE) {
		if (*wh < *mr)		/* if less than real abil */
		    *wh = *mr;		/* make equal to real */
	    }
	}
	else {				/* increment */
	    if (*wh > MAXOTHER) {	/* see if > max */
		if (is_str) {		/* check for strength */
		    if (*wh > MAXSTR)
			*wh = MAXSTR;	/* str max = 24 */
		}
		else {			/* other than strength */
		    *wh = MAXOTHER;	/* max = 18 */
		}
	    }
	    /*
	     * Check for updating the max player stats.
	     */
	    if (*wh > *mx)
		*mx = *wh;
	}
}


/*
 * add_haste:
 *	add a haste to the player
 */
add_haste(potion)
bool potion;
{
    if (pl_on(ISHASTE)) {
	msg("You faint from exhaustion.");
	no_command += rnd(8);
	player.t_flags &= ~ISHASTE;
	extinguish(nohaste);
    }
    else {
	player.t_flags |= ISHASTE;
	if (potion)
	    fuse(nohaste, TRUE, roll(10,10), AFTER);
	else
	    fuse(nohaste, TRUE, roll(40,20), AFTER);
    }
}

/*
 * getpdex:
 *	Gets players added dexterity for fighting
 */
getpdex(edex,heave)
reg int edex;
bool heave;
{
	if(heave) {		/* an object was thrown here */
		switch(edex) {
ult: return -3;
		}
	}
	else {		/* object NOT thrown here (affects armor class) */
		switch(edex) {
			case 18: return -4;
			case 17: return -3;
			case 16: return -2;
			case 15: return -1;
			case 14:
			case 13:
			case 12:
			case 11:
			case 10:
			case 9:
			case 8: 
			case 7: return 0;
			case 6: return 1;
			case 5: return 2;
			case 4: return 3;
			default: return 4;
		}
	}
}

/*
 * getpwis:
 *	Get a players wisdom for fighting
 */
getpwis(ewis)
reg int ewis;
{
	switch(ewis) {
		case 18: return 4;
		case 17: return 3;
		case 16: return 2;
		case 15: return 1;
		case 14:
		case 13:
		case 12:
		case 11:
		case 10:
		case 9:
		case 8: return 0;
		case 7:
		case 6: return -1;
		case 5:
		case 4: return -2;
		default: return -3;
	}
}

/*
 * getpcon:
 *	Get added hit points from players constitution
 */
getpcon(econ)
reg int econ;
{
	switch(econ) {
		case 18: return 4;
		case 17: return 3;
		case 16: return 2;
		case 15: return 1;
		case 14:
		case 13:
		case 12:
		case 11:
		case 10:
		case 9:
		case 8:
		case 7: return 0;
		case 6:
		case 5:
		case 4: return -1;
		default: return -2;
	}
}


/*
 * str_plus:
 *	compute bonus/penalties for strength on the "to hit" roll
 */
str_plus(str)
reg int str;
{
	reg int hitplus = 0;

	if (str == 24)			/* 24 */
		hitplus = 3;
	else if (str > 20)		/* 21 to 23 */
		hitplus = 2;
	else if(str >= 17)		/* 17 to 20 */
		hitplus = 1;
	else if(str > 7)		/* 8 to 16 */
		hitplus = 0;
	else if(str > 5)		/* 6 to 7 */
		hitplus = -1;
	else if(str > 3)		/* 4 to 5 */
		hitplus = -2;
	else
		hitplus = -3;		/* < 4 */
	return(hitplus + hitweight());
}


/*
 * add_dam:
 *	Compute additional damage done depending on strength
 */
 add_dam(str)
 reg int str;
 {
	reg int exdam = 0;

	if (str == 24)		/* 24 */
		exdam = 6;
	else if (str == 23)	/* 23 */
		exdam = 5;
	else if (str > 20)	/* 21 to 22 */
		exdam = 4;
	else if (str > 18)	/* 19 to 20 */
		exdam = 3;
	else if (str == 18)
		exdam = 2;	/* 18 */
	else if (str > 15)	/* 16 to 17 */
		exdam = 1;
	else if (str > 6)	/* 7 to 14 */
		exdam = 0;
	else
		exdam = -1;	/* 3 to 6 */

	exdam += hungdam();	/* add hungry state */
	return(exdam);
}


/*
 * hungdam:
 *	Calculate damage depending on players hungry state
 */
hungdam()
{
	reg int howmuch;

	switch(hungry_state) {
		case F_OK:
		case F_HUNGRY:	howmuch = 0;
		when F_WEAK:	howmuch = -1;
		when F_FAINT:	howmuch = -2;
	}
	return howmuch;
}

