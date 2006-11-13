/*
 * various display routines and flag checking functions
 *
 * @(#)disply.c	7.0	(Bell Labs)	10/08/82
 */

#include "rogue.h"
#include <ctype.h>
#include "rogue_ext.h"


/*
 * displevl:
 *	Display detailed level for wizard and scroll
 */

extern int isatrap (char ch);
extern int illeg_ch (char ch);
extern int updpack (int getmax);
extern void msg (const char *fmt, ...);

displevl(void)
{
	reg char ch, mch;
	reg int i,j;
	reg struct room *rp;

	for(rp = rooms; rp < &rooms[MAXROOMS]; rp++)
		rp->r_flags &= ~ISDARK;

	for(i = 0 ; i < LINES - 1 ; i++) {
	    for(j = 0 ; j < COLS - 1 ; j++) {
		ch = mvinch(i,j);
		if (isatrap(ch)) {
		    struct trap *what;

		    what = trap_at(i, j);
		    if (what != NULL)
			what->tr_flags |= ISFOUND;
		}
		else if (ch == SECRETDOOR) {
		    ch = DOOR;
		    mvaddch(i, j, ch);
		}
		else if (illeg_ch(ch)) {
		    ch = FLOOR;
		    mvaddch(i, j, ch);
		}
		if (mvwinch(mw, i, j) != ' ') {
		    struct linked_list *what;
		    struct thing *it;

		    what = find_mons(i, j);
		    if (what == NULL) {
			ch = FLOOR;
			mvaddch(i, j, ch);
		    }
		    else {
			it = THINGPTR(what);
			it->t_oldch = ch;
		    }
		}
		if (isalpha((mch = mvwinch(cw, i, j))))
		    ch = mch;
		mvwaddch(cw, i, j, ch);
	    }
	}
	nochange = FALSE;	/* display status again */
	draw(cw);
}


/*
 * dispmons:
 *	Show monsters for wizard and potion
 */
dispmons(void)
{
	reg int i,j;
	reg char ch;
	reg struct thing *it;
	reg struct linked_list *item;

	for (i = 0 ; i < LINES - 1 ; i++) {
	    for (j = 0; j < COLS - 1 ; j++) {
		ch = mvwinch(mw,i,j);
		if (ch != ' ') {		/* display any monsters */
		    item = find_mons(i,j);
		    if (item != NULL) {
			mvwaddch(cw, i, j, ch);
			it = THINGPTR(item);
			it->t_flags |= ISFOUND;
			if (ch == 'M')			/* if a mimic */
			    it->t_disguise = 'M';	/* give it away */
		    }
		}
	    }
	}
	draw(cw);
}


/*
 * prntstats:
 *	Print hero info dexterity, constitution, wisdom, encumberence
 */
prntstats(char prtype)
{
	reg int totwght, carwght, wght;
	reg struct stats *st;

	updpack(TRUE);			/* get all weight info */
	st = &player.t_stats;
	totwght = st->s_carry / 10;
	carwght = st->s_pack / 10;
	wght = totwght - carwght;
	if(wght < 0)
	    wght = 0;
	switch(prtype) {
	case 1: msg("Maximum = %d  :  Current = %d  :  Available = %d",
		totwght,carwght,wght);
	when 2: msg("Dexterity = %d : Constitution = %d : Wisdom = %d",
		st->s_ef.a_dex,st->s_ef.a_con,st->s_ef.a_wis);
	}
}


/*
 * winat:
 *	Get whatever character is at a location on the screen
 */
winat(int y, int x)
{
	reg char ch;

	if (mvwinch(mw,y,x) == ' ')
		ch = mvwinch(stdscr,y,x);	/* non-monsters */
	else
		ch = winch(mw);			/* monsters */
	return (ch);
}


/*
 * pl_on:
 *	Returns TRUE if the player's flag is set
 */
long
pl_on(long int what)
{
	return (player.t_flags & what);
}


/*
 * pl_off:
 *	Returns TRUE when player's flag is reset
 */
long
pl_off(long int what)
{
	return (!(player.t_flags & what));
}


/*
 * o_on:
 *	Returns TRUE in the objects flag is set
 */
int o_on(struct object *what, int bit)
{
    return (what->o_flags & bit);
}


/*
 * o_off:
 *	Returns TRUE is the objects flag is reset
 */
o_off(struct object *what, int bit)
{
	return (!(what->o_flags & bit));
}


/*
 * setoflg:
 *	Set the specified flag for the object
 */
setoflg(struct object *what, int bit)
{
	what->o_flags |= bit;
}


/*
 * resoflg:
 *	Reset the specified flag for the object
 */
resoflg(struct object *what, int bit)
{
	what->o_flags &= ~bit;
}
