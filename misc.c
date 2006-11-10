/*
 * all sorts of miscellaneous routines
 *
 * @(#)misc.c	7.0	(Bell Labs)	10/08/82
 */

#include "rogue.h"
#include <ctype.h>
#include "rogue_ext.h"


/*
 * tr_name:
 *	print the name of a trap
 */


extern int show (int y, int x);
extern int isatrap (char ch);
extern int iswearing (int ring);
int secretdoor (int y, int x);
extern void msg (const char *fmt, ...);
extern int o_on (struct object *what, int bit);
extern int del_pack (struct linked_list *what);
extern int check_level (void);
extern int updpack (int getmax);
extern int runto (struct coord *runner, struct coord *spot);
extern int readchar (void);

char *
tr_name(char ch)
{
    reg char *s;

    switch (ch) {
	case TRAPDOOR:
	    s = "A trapdoor.";
	when BEARTRAP:
	    s = "A beartrap.";
	when SLEEPTRAP:
	    s = "A sleeping gas trap.";
	when ARROWTRAP:
	    s = "An arrow trap.";
	when TELTRAP:
	    s = "A teleport trap.";
	when DARTTRAP:
	    s = "A dart trap.";
	when POOL:
	    s = "A magic pool.";
	when POST:
	    s = "A trading post.";
	otherwise:
	    s = "A bottomless pit.";		/* shouldn't get here */
    }
    return s;
}

/*
 * Look:
 *	A quick glance all around the player
 */

look(NCURSES_BOOL wakeup)
{
    reg int x, y;
    reg char ch;
    reg int oldx, oldy;
    reg bool inpass;
    reg int passcount = 0;
    reg struct room *rp;
    reg int ey, ex;

    getyx(cw, oldy, oldx);
    if(oldrp != NULL && (oldrp->r_flags & ISDARK) && pl_off(ISBLIND)) {
	for (x = oldpos.x - 1; x <= oldpos.x + 1; x += 1)
	    for (y = oldpos.y - 1; y <= oldpos.y + 1; y += 1)
		if ((y != hero.y || x != hero.x) && show(y, x) == FLOOR)
		    mvwaddch(cw, y, x, ' ');
    }
    inpass = ((rp = roomin(&hero)) == NULL);
    ey = hero.y + 1;
    ex = hero.x + 1;
    for (x = hero.x - 1; x <= ex; x += 1)
	if (x >= 0 && x < COLS) for (y = hero.y - 1; y <= ey; y += 1) {
	    if (y <= 0 || y >= LINES - 1)
		continue;
	    if (isalpha(mvwinch(mw, y, x))) {
		reg struct linked_list *it;
		reg struct thing *tp;

		if (wakeup)
		    it = wake_monster(y, x);
		else
		    it = find_mons(y, x);
		if (it == NULL) {		/* lost monster */
		    mvaddch(y, x, FLOOR);
		}
		else {
		    tp = THINGPTR(it);
		    if (isatrap(tp->t_oldch = mvinch(y, x))) {
			struct trap *trp;
			if((trp = trap_at(y,x)) == NULL)
			    break;
			if(iswearing(R_FTRAPS))
			    trp->tr_flags |= ISFOUND;
			if (trp->tr_flags & ISFOUND)
			    tp->t_oldch = trp->tr_type;
			else
			    tp->t_oldch = FLOOR;
		    }
		    if (tp->t_oldch == FLOOR && (rp->r_flags & ISDARK)
		      && pl_off(ISBLIND))
			tp->t_oldch = ' ';
		}
	    }
	    /*
	     * Secret doors show as walls
	     */
	    if ((ch = show(y, x)) == SECRETDOOR)
		ch = secretdoor(y, x);
	    /*
	     * Don't show room walls if he is in a passage
	     */
	    if (pl_off(ISBLIND)) {
		if (y == hero.y && x == hero.x
		 || (inpass && (ch == '-' || ch == '|')))
			continue;
	    }
	    else if (y != hero.y || x != hero.x)
		continue;
	    wmove(cw, y, x);
	    waddch(cw, ch);
	    if (door_stop && !firstmove && running) {
		switch (runch) {
		    case 'h':
			if (x == ex)
			    continue;
		    when 'j':
			if (y == hero.y - 1)
			    continue;
		    when 'k':
			if (y == ey)
			    continue;
		    when 'l':
			if (x == hero.x - 1)
			    continue;
		    when 'y':
			if ((x + y) - (hero.x + hero.y) >= 1)
			    continue;
		    when 'u':
			if ((y - x) - (hero.y - hero.x) >= 1)
			    continue;
		    when 'n':
			if ((x + y) - (hero.x + hero.y) <= -1)
			    continue;
		    when 'b':
			if ((y - x) - (hero.y - hero.x) <= -1)
			    continue;
		}
		switch (ch) {
		    case DOOR:
			if (x == hero.x || y == hero.y)
			    running = FALSE;
			break;
		    case PASSAGE:
			if (x == hero.x || y == hero.y)
			    passcount += 1;
			break;
		    case FLOOR:
		    case '|':
		    case '-':
		    case ' ':
			break;
		    default:
			running = FALSE;
			break;
		}
	    }
	}
    if (door_stop && !firstmove && passcount > 1)
	running = FALSE;
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    wmove(cw, oldy, oldx);
    oldpos = hero;
    oldrp = rp;
}

/*
 * secret_door:
 *	Figure out what a secret door looks like.
 */

secretdoor(int y, int x)
{
    reg int i;
    reg struct room *rp;
    reg struct coord *cpp;
    static struct coord cp;

    cp.y = y;
    cp.x = x;
    cpp = &cp;
    for (rp = rooms, i = 0; i < MAXROOMS; rp += 1, i += 1)
	if (inroom(rp, cpp))
	    if (y == rp->r_pos.y || y == rp->r_pos.y + rp->r_max.y - 1)
		return('-');
	    else
		return('|');

    return('p');
}

/*
 * find_obj:
 *	find the unclaimed object at y, x
 */

struct linked_list *
find_obj(int y, int x)
{
    reg struct linked_list *obj;
    reg struct object *op;

    for (obj = lvl_obj; obj != NULL; obj = next(obj)) {
	op = OBJPTR(obj);
	if (op->o_pos.y == y && op->o_pos.x == x)
		return obj;
    }
    return NULL;
}

/*
 * eat:
 *	She wants to eat something, so let her try
 */

eat(void)
{
    reg struct linked_list *item;
    reg struct object *obj;
    reg int goodfood, cursed;


    if ((item = get_item("eat", FOOD)) == NULL)
	return;
    obj = OBJPTR(item);
    if (obj->o_type != FOOD) {
	msg("That's Inedible!");
	return;
    }
    if (o_on(obj,ISCURSED))
	cursed = 2;
    else
	cursed = 1;
    del_pack(item);		/* get rid of the food */
    if (obj->o_which == 1) {
	msg("My, that was a yummy %s", fruit);
	goodfood = 100;
    }
    else
	if (rnd(100) > 80) {
	    msg("Yuk, this food tastes like ARA");
	    goodfood = 300;
	    pstats.s_exp += 1;
	    check_level();
	}
	else {
	    msg("Yum, that tasted good");
	    goodfood = 200;
	}
    goodfood *= cursed;
    if ((food_left += HUNGERTIME + rnd(400) - goodfood) > STOMACHSIZE)
	food_left = STOMACHSIZE;
    hungry_state = F_OK;
    updpack(TRUE);			/* update pack */
    if (obj == cur_weapon)
	cur_weapon = NULL;
}

/*
 * aggravate:
 *	aggravate all the monsters on this level
 */

aggravate(void)
{
    reg struct linked_list *mi;

    for (mi = mlist; mi != NULL; mi = next(mi))
	runto(&(THINGPTR(mi))->t_pos, &hero);
}

/*
 * for printfs: if string starts with a vowel, return "n" for an "an"
 */
char *
vowelstr(char *str)
{
    switch (*str) {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	    return "n";
	default:
	    return "";
    }
}

/* 
 * see if the object is one of the currently used items
 */
is_current(struct object *obj)
{
    if (obj == NULL)
	return FALSE;
    if (obj == cur_armor || obj == cur_weapon || obj == cur_ring[LEFT]
	|| obj == cur_ring[RIGHT]) {
	msg("Already in use.");
	return TRUE;
    }
    return FALSE;
}

/*
 * set up the direction co_ordinate for use in varios "prefix" commands
 */
get_dir(void)
{
    reg char *prompt;
    reg bool gotit;

    prompt = "Direction: ";
    do {
	gotit = TRUE;
	switch (readchar()) {
	    case 'h': case'H': delta.y =  0; delta.x = -1;
	    when 'j': case'J': delta.y =  1; delta.x =  0;
	    when 'k': case'K': delta.y = -1; delta.x =  0;
	    when 'l': case'L': delta.y =  0; delta.x =  1;
	    when 'y': case'Y': delta.y = -1; delta.x = -1;
	    when 'u': case'U': delta.y = -1; delta.x =  1;
	    when 'b': case'B': delta.y =  1; delta.x = -1;
	    when 'n': case'N': delta.y =  1; delta.x =  1;
	    when ESCAPE: return FALSE;
	    otherwise:
		mpos = 0;
		msg(prompt);
		gotit = FALSE;
	}
    } until (gotit);
    if (pl_on(ISHUH) && rnd(100) > 80)
	do {
	    delta.y = rnd(3) - 1;
	    delta.x = rnd(3) - 1;
	} while (delta.y == 0 && delta.x == 0);
    mpos = 0;
    return TRUE;
}

char
lower(char c)
{
	return (isupper(c) ? c - 'A' + 'a' : c);
}
