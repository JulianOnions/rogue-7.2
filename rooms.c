/*
 * Draw the nine rooms on the screen
 *
 * @(#)rooms.c	7.0	(Bell Labs)	10/14/82
 */

#include "rogue.h"
#include "rogue_ext.h"


/*
 * do_rooms:
 *	Place the rooms in the dungeon
 */

extern int rnd_room (void);
int rnd_pos (struct room *rp, struct coord *cp);
extern void debug (char *errstr);
int draw_room (struct room *rp);
extern int new_monster (struct linked_list *item, char type, struct coord *cp, NCURSES_BOOL treas);
extern int randmonster (NCURSES_BOOL wander, NCURSES_BOOL baddie);
extern int mon_index (char whichmon);
extern int _attach (register struct linked_list **list, register struct linked_list *item);
int vert (int cnt);
int horiz (int cnt);

do_rooms(void)
{
    int mloops, mchance, nummons;
    bool treas = FALSE;
    reg int i;
    reg struct room *rp;
    reg struct linked_list *item;
    reg struct thing *tp;
    reg int left_out;
    struct coord top;
    struct coord bsze;
    struct coord mp;
    /*
     * bsze is the maximum room size
     */

    bsze.x = COLS/3;
    bsze.y = LINES/3;
    /*
     * Clear things for a new level
     */
    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	rp->r_goldval = rp->r_nexits = rp->r_flags = 0;
    /*
     * Put the gone rooms, if any, on the level
     */
    left_out = rnd(4);
    for (i = 0; i < left_out; i++)
	rooms[rnd_room()].r_flags |= ISGONE;
    /*
     * dig and populate all the rooms on the level
     */
    for (i = 0, rp = rooms; i < MAXROOMS; rp++, i++) {
	/*
	 * Find upper left corner of box that this room goes in
	 */
	top.x = (i%3) * bsze.x + 1;
	top.y = i/3 * bsze.y;
	if (top.y == 0)
		top.y = 1;
	if(rp->r_flags & ISGONE) {
	    /*
	     * Place a gone room.  Make certain that there is a blank line
	     * for passage drawing.
	     */
	    rp->r_pos.x = top.x + rnd(bsze.x-2) + 1;
	    rp->r_pos.y = top.y + rnd(top.y == 1 ? bsze.y-3 : bsze.y-2) + 1;
	    rp->r_max.x = -COLS;
	    rp->r_max.x = -LINES;
	    continue;
	}
	if (rnd(10) < level-1)
	    rp->r_flags |= ISDARK;
	/*
	 * Find a place and size for a random room
	 */
	rp->r_max.x = rnd(bsze.x - 4) + 4;
	rp->r_max.y = rnd(top.y == 1 ? bsze.y - 5 : bsze.y - 4) + 4;
	rp->r_pos.x = top.x + rnd(bsze.x - rp->r_max.x);
	rp->r_pos.y = top.y + rnd(top.y == 1 ? bsze.y - 1 - rp->r_max.y : bsze.y - rp->r_max.y);
	if(level < max_level)
		mchance = 25;	/* 25% when going up (all monsters) */
	else
		mchance = 3;	/* 3% when going down */
	treas = FALSE;
	if(rnd(100) < mchance && level >= 8 &&
		(rp->r_max.x * rp->r_max.y) > (bsze.x * bsze.y /100*60)) {
		rp->r_flags |= ISTREAS;
		treas = TRUE;
		rp->r_flags |= ISDARK;	/* always a dark room */
	}
	/*
	 * Put the gold in
	 */
	if (rnd(100) < (treas ? 100 : 50) /* treas room always has gold */
		&& (!amulet || level >= max_level)) {
	    rp->r_goldval = GOLDCALC;
	    if(treas)
		rp->r_goldval = rp->r_goldval * rnd(level) + 2;
	    rnd_pos(rp, &rp->r_gold);
	    if (roomin(&rp->r_gold) != rp) {
		sprintf(errbuf,
		"Gold goofed: @ %d,d",rp->r_gold.y,rp->r_gold.x);
		debug(errbuf);
	    }
	}
	draw_room(rp);
	/*
	 * Put the monster in
	 */
	if(treas) {			/* is it a treasure room? */
		mloops = rnd(level / 3) + 6;
		mchance = 1;		/* always place a monster */
	}
	else {
		mloops = 1;
		mchance = 100;		/* 25% or 80% for a monster */
	}
	for(nummons = 0 ; nummons < mloops ; nummons++) {
		if (rnd(mchance) < (rp->r_goldval > 0 ? 80 : 25)) {
		    item = new_item(sizeof *tp);
		    tp = THINGPTR(item);
		    do {
			rnd_pos(rp, &mp);
		    } until(mvwinch(stdscr, mp.y, mp.x) == FLOOR);
		    new_monster(item, randmonster(FALSE,FALSE), &mp,
			mloops == 1 ? FALSE : TRUE);
		    /*
		     * See if monster has a treasure
		     */
		    if (rnd(100) < monsters[mon_index(tp->t_type)].m_carry)
			attach(tp->t_pack, new_thing(FALSE));
		}
	}
    }
}


/*
 * draw_room:
 *	Draw a box around a room
 */
draw_room(struct room *rp)
{
    reg int j, k;

    move(rp->r_pos.y, rp->r_pos.x+1);
    vert(rp->r_max.y-2);			/* Draw left side */
    move(rp->r_pos.y+rp->r_max.y-1, rp->r_pos.x);
    horiz(rp->r_max.x);				/* Draw bottom */
    move(rp->r_pos.y, rp->r_pos.x);
    horiz(rp->r_max.x);				/* Draw top */
    vert(rp->r_max.y-2);			/* Draw right side */
    /*
     * Put the floor down
     */
    for (j = 1; j < rp->r_max.y - 1; j++) {
	move(rp->r_pos.y + j, rp->r_pos.x + 1);
	for (k = 1; k < rp->r_max.x - 1; k++) {
		addch(FLOOR);
	}
    }
    /*
     * Put the gold there
     */
    if (rp->r_goldval)
	mvaddch(rp->r_gold.y, rp->r_gold.x, GOLD);
}

/*
 * horiz:
 *	draw a horizontal line
 */
horiz(int cnt)
{
    while (cnt-- > 0)
	addch('-');
}


/*
 * vert:
 *	draw a vertical line
 */
vert(int cnt)
{
    reg int x, y;

    getyx(stdscr, y, x);
    x--;
    while (cnt-- > 0) {
	move(++y, x);
	addch('|');
    }
}


/*
 * rnd_pos:
 *	pick a random spot in a room
 */
rnd_pos(struct room *rp, struct coord *cp)
{
    cp->x = rp->r_pos.x + rnd(rp->r_max.x-2) + 1;
    cp->y = rp->r_pos.y + rnd(rp->r_max.y-2) + 1;
}
