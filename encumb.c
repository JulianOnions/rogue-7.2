/*
 * Stuff to do with encumberence
 *
 * @(#)encumb.c	7.0	(Bell Labs)	10/08/82
 */

#include "rogue.h"
#include "rogue_ext.h"


/*
 * updpack:
 *	Update his pack weight and adjust fooduse accordingly
 */

int totalenc (void);
int packweight (void);
int itemweight (struct object *wh);
extern int iswearing (int ring);
extern int o_on (struct object *what, int bit);
extern int extinguish (int (*func) (/* ??? */));
extern int fuse (int (*func) (/* ??? */), int arg, int time, int type);
extern void msg (const char *fmt, ...);
extern int drop (struct linked_list *item);
extern void debug (char *errstr);

updpack(int getmax)
{

	reg int topcarry, curcarry;

	if (getmax)
	    pstats.s_carry = totalenc();	/* get total encumb */
	curcarry = packweight();		/* get pack weight */
	topcarry = pstats.s_carry / 5;		/* 20% of total carry */
	if(curcarry > 4 * topcarry) {
	    if(rnd(100) < 80)
		foodlev = 3;			/* > 80% of pack */
	} else if(curcarry > 3 * topcarry) {
	    if(rnd(100) < 60)
		foodlev = 2;			/* > 60% of pack */
	} else
	    foodlev = 1;			/* <= 60% of pack */
	pstats.s_pack = curcarry;		/* update pack weight */
}


/*
 * packweight:
 *	Get the total weight of the hero's pack
 */
packweight(void)
{
	reg struct object *obj;
	reg struct linked_list *pc;
	reg int weight;

	weight = 0;
	for(pc = pack ; pc != NULL ; pc = next(pc)) {
	    obj = OBJPTR(pc);
	    weight += itemweight(obj) * obj->o_count;
	}
	if(weight < 0)		/* in case of amulet */
	     weight = 0;
	if(iswearing(R_HEAVY))
	    weight += weight / 4;
	return(weight);
}


/*
 * itemweight:
 *	Get the weight of an object
 */
itemweight(struct object *wh)
{
	reg int weight;

	weight = wh->o_weight;		/* get base weight */
	switch(wh->o_type) {
	    case ARMOR:
		if ((armors[wh->o_which].a_class - wh->o_ac) > 0)
			weight /= 2;
	    when WEAPON:
		if ((wh->o_hplus + wh->o_dplus) > 0)
			weight /= 2;
	}
	if(o_on(wh,ISCURSED))
		weight += weight / 5;	/* 20% more for cursed */
	return(weight);
}


/*
 * playenc:
 *	Get hero's carrying ability above norm
 */
playenc(void)
{
	switch(pstats.s_ef.a_str) {
		case 24: return 3000;
		case 23: return 2000;
		case 22: return 1500;
		case 21: return 1250;
		case 20: return 1100;
		case 19: return 1000;
		case 18: return 750;
		case 17: return 500;
		case 16: return 350;
		case 15:
		case 14: return 200;
		case 13:
		case 12: return 100;
		case 11:
		case 10:
		case  9:
		case  8: return 0;
		case  7:
		case  6: return -150;
		case  5:
		case  4: return -250;
	}
	return -350;
}


/*
 * totalenc:
 *	Get total weight that the hero can carry
 */
totalenc(void)
{
	reg int wtotal;

	wtotal = NORMENCB + playenc();
	switch(hungry_state) {
		case F_OK:
		case F_HUNGRY:	;			/* no change */
		when F_WEAK:	wtotal -= wtotal / 10;	/* 10% off weak */
		when F_FAINT:	wtotal /= 2;		/* 50% off faint */
	}
	return(wtotal);
}



/*
 * whgtchk:
 *	See if the hero can carry his pack
 */

wghtchk(int junk)
{
	reg int dropchk, err = TRUE;
	reg char ch;

	inwhgt = TRUE;
	if (pstats.s_pack > pstats.s_carry) {
	    ch = mvwinch(stdscr, hero.y, hero.x);
	    if((ch != FLOOR && ch != PASSAGE) || isfight) {
		extinguish(wghtchk);
		fuse(wghtchk,TRUE,1,AFTER);
		inwhgt = FALSE;
		return;
	    }
	    extinguish(wghtchk);
	    msg("Your pack is too heavy for you");
	    do {
		dropchk = drop(NULL);
		if(dropchk == SOMTHERE) {
		    debug("wghtchk dropped on an item");
		    err = FALSE;
		}
		else if(dropchk == 0) {
		    mpos = 0;
		    msg("You must drop something");
		}
		if(dropchk == TRUE)
		    err = FALSE;
	    } while(err);
	}
	inwhgt = FALSE;
}


/*
 * hitweight:
 *	Gets the fighting ability according to current weight
 * 	This returns a  +1 hit for light pack weight
 * 			 0 hit for medium pack weight
 *			-1 hit for heavy pack weight
 */

hitweight(void)
{
	return(2 - foodlev);
}
