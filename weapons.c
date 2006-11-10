/*
 * Functions for dealing with weapons
 *
 * @(#)weapons.c	7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include "rogue_ext.h"


/*
 * missile:
 *	Fire a missile in a given direction
 */


extern int dropcheck (struct object *op);
extern int is_current (struct object *obj);
extern void msg (const char *fmt, ...);
extern int readchar (void);
extern int _detach (register struct linked_list **list, register struct linked_list *item);
extern int updpack (int getmax);
int do_motion (struct object *obj, int ydelta, int xdelta);
int hit_monster (int y, int x, struct object *obj);
int fall (struct linked_list *item, NCURSES_BOOL pr);
extern int cansee (int y, int x);
extern int show (int y, int x);
extern int step_ok (char ch);
extern int winat (int y, int x);
int fallpos (struct coord *pos, struct coord *newpos, NCURSES_BOOL passages);
extern int light (struct coord *cp);
extern int _attach (register struct linked_list **list, register struct linked_list *item);
extern int discard (register struct linked_list *item);
extern int o_on (struct object *what, int bit);
extern int fight (struct coord *mp, char mn, struct object *weap, NCURSES_BOOL thrown);

missile(int ydelta, int xdelta)
{
    reg struct object *obj, *nowwield;
    reg struct linked_list *item, *nitem;

    /*
     * Get which thing we are hurling
     */
    nowwield = cur_weapon;		/* must save current weap */
    if ((item = get_item("throw", WEAPON)) == NULL)
	return;
    obj = OBJPTR(item);
    if (!dropcheck(obj) || is_current(obj))
	return;
    if(obj == nowwield || obj->o_type != WEAPON) {
	reg int c;
	msg("Are you sure? (y or n)");
	do {
		c = lower(readchar());
		if(c == ESCAPE || c == 'n') {
			msg("");
			cur_weapon = nowwield;
			after = FALSE;		/* ooops, a mistake */
			return;
		}
	} while (c != 'y');	/* keep looking for good ans */
    }
    /*
     * Get rid of the thing.  If it is a non-multiple item object, or
     * if it is the last thing, just drop it.  Otherwise, create a new
     * item with a count of one.
     */
    if (obj->o_count < 2) {
	detach(pack, item);
	inpack--;
    }
    else {
	obj->o_count--;
	if (obj->o_group == 0)
	    inpack--;
	nitem = new_item(sizeof *obj);
	obj = OBJPTR(nitem);
	*obj = *(OBJPTR(item));
	obj->o_count = 1;
	item = nitem;
    }
    updpack(FALSE);			/* new pack weight */
    do_motion(obj, ydelta, xdelta);
    /*
     * AHA! Here it has hit something.  If it is a wall or a door,
     * or if it misses (combat) the mosnter, put it on the floor
     */
    if (!isalpha(mvwinch(mw, obj->o_pos.y, obj->o_pos.x))
	|| !hit_monster(unc(obj->o_pos), obj))
	    fall(item, TRUE);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
}

/*
 * do the actual motion on the screen done by an object traveling
 * across the room
 */
do_motion(struct object *obj, int ydelta, int xdelta)
{
    /*
     * Come fly with us ...
     */
    obj->o_pos = hero;
    for (;;) {
	reg int ch;

	/*
	 * Erase the old one
	 */
	if (!ce(obj->o_pos, hero) && cansee(unc(obj->o_pos)) &&
	    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ')
		    mvwaddch(cw, obj->o_pos.y, obj->o_pos.x,
			    show(obj->o_pos.y, obj->o_pos.x));
	/*
	 * Get the new position
	 */
	obj->o_pos.y += ydelta;
	obj->o_pos.x += xdelta;
	if(step_ok(ch = winat(obj->o_pos.y,obj->o_pos.x)) && ch != DOOR) {
	    /*
	     * It hasn't hit anything yet, so display it
	     * If it alright.
	     */
	    if (cansee(unc(obj->o_pos)) &&
	      mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
		mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, obj->o_type);
		draw(cw);
	    }
	    continue;
	}
	break;
    }
}

/*
 * fall:
 *	Drop an item someplace around here.
 */

fall(struct linked_list *item, NCURSES_BOOL pr)
{
    reg struct object *obj;
    reg struct room *rp;
    reg char *ptr;
    static struct coord fpos;

    obj = OBJPTR(item);
    if (fallpos(&obj->o_pos, &fpos, TRUE)) {
	mvaddch(fpos.y, fpos.x, obj->o_type);
	obj->o_pos = fpos;
	if ((rp = roomin(&hero)) != NULL && !(rp->r_flags & ISDARK)) {
	    light(&hero);
	    mvwaddch(cw, hero.y, hero.x, PLAYER);
	}
	attach(lvl_obj, item);
	return;
    }
    if (pr) {
	ptr = inv_name(obj,TRUE);
	if (strncmp("some",ptr,4) == 0 || strncmp("the",ptr,3) == 0
	 || *ptr == 'a') {
	    while(*ptr != ' ') ptr++;
	    ptr++;
	}
	msg("Your %s vanishes as it hits the ground.",ptr);
    }
    discard(item);
}

/*
 * init_weapon:
 *	Set up the initial goodies for a weapon
 */

init_weapon(struct object *weap, char type)
{
    reg struct init_weps *iwp;

    iwp = &weaps[type];
    weap->o_damage = iwp->w_dam;
    weap->o_hurldmg = iwp->w_hrl;
    weap->o_launch = iwp->w_launch;
    weap->o_flags = iwp->w_flags;
    weap->o_weight = iwp->w_wght;
    if (o_on(weap,ISMANY)) {
	weap->o_count = rnd(8) + 8;
	weap->o_group = newgrp();
    }
    else
	weap->o_count = 1;
}

/*
 * Does the missile hit the monster
 */

hit_monster(int y, int x, struct object *obj)
{
    static struct coord mp;

    mp.y = y;
    mp.x = x;
    return fight(&mp, winat(y, x), obj, TRUE);
}

/*
 * num:
 *	Figure out the plus number for armor/weapons
 */

char *
num(int n1, int n2)
{
    static char numbuf[LINLEN];

    if (n1 == 0 && n2 == 0)
	return "+0";
    if (n2 == 0)
	sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1);
    else
	sprintf(numbuf,"%s%d,%s%d",n1<0 ? "":"+",n1,n2<0 ? "":"+",n2);  
    return numbuf;
}

/*
 * wield:
 *	Pull out a certain weapon
 */

wield(void)
{
    reg struct linked_list *item;
    reg struct object *obj, *oweapon;

    oweapon = cur_weapon;
    if (!dropcheck(cur_weapon)) {
	cur_weapon = oweapon;
	return;
    }
    cur_weapon = oweapon;
    if ((item = get_item("wield", WEAPON)) == NULL) {
	msg("");
bad:
	after = FALSE;
	return;
    }
    obj = OBJPTR(item);
    if (is_current(obj))
        goto bad;
    msg("Wielding %s", inv_name(obj, TRUE));
    cur_weapon = obj;
}

/*
 * pick a random position around the give (y, x) coordinates
 */
fallpos(struct coord *pos, struct coord *newpos, NCURSES_BOOL passages)
{
    reg int y, x, cnt, ch;

    cnt = 0;
    for (y = pos->y - 1; y <= pos->y + 1; y++)
	for (x = pos->x - 1; x <= pos->x + 1; x++) {
	    /*
	     * check to make certain the spot is empty, if it is,
	     * put the object there, set it in the level list
	     * and re-draw the room if he can see it
	     */
	    if (y == hero.y && x == hero.x)
		continue;
	    if(((ch = winat(y, x)) == FLOOR || (passages && ch == PASSAGE))
	      && rnd(++cnt) == 0) {
		newpos->y = y;
		newpos->x = x;
	    }
	}
    return (cnt != 0);
}
