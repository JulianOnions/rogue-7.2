/*
 * Hero movement commands
 *
 * @(#)move.c	7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include "rogue_ext.h"


/*
 * Used to hold the new hero position
 */

struct coord nh;

/*
 * do_run:
 *	Start the hero running
 */


extern void msg (const char *fmt, ...);
extern int iswearing (int ring);
extern int diag_ok (struct coord *sp, struct coord *ep);
extern int winat (int y, int x);
extern int dead_end (char ch);
int show (int y, int x);
int isatrap (char ch);
int be_trapped (struct coord *tc);
extern int illeg_ch (char ch);
extern int teleport (void);
int light (struct coord *cp);
extern int fight (struct coord *mp, char mn, struct object *weap, NCURSES_BOOL thrown);
extern int new_level (NCURSES_BOOL post);
extern int getpdex (int edex, NCURSES_BOOL heave);
extern int chg_hpt (int howmany, NCURSES_BOOL alsomax, char whichmon);
extern int roll (int number, int sides);
extern int init_weapon (struct object *weap, char type);
extern int setoflg (struct object *what, int bit);
extern int fall (struct linked_list *item, NCURSES_BOOL pr);
extern int save (int which);
extern int chg_abil (register int what, register int amt, register int how);
extern int o_off (struct object *what, int bit);
extern int resoflg (struct object *what, int bit);
extern int magring (struct object *what);
extern int death (char monst);
extern void debug (char *errstr);
extern int step_ok (char ch);

do_run(char ch)
{
    running = TRUE;
    after = FALSE;
    runch = ch;
}

/*
 * do_move:
 *	Check to see that a move is legal.  If it is handle the
 *	consequences (fighting, picking up, etc.)
 */

do_move(int dy, int dx)
{
    reg int ch;

    keyhit = TRUE;		/* active player */
    firstmove = FALSE;
    curprice = -1;

    if(no_move > 0) {
	no_move -= 1;
	msg("You are still stuck in the bear trap");
	return;
    }
    /*
     * Do a confused move (maybe)
     */
    if ((rnd(100) < 80 && pl_on(ISHUH)) ||
	(iswearing(R_DELUS) && rnd(100) < 25))
	nh = *rndmove(&player);
    else {
	nh.y = hero.y + dy;
	nh.x = hero.x + dx;
    }

    /*
     * Check if he tried to move off the screen or make an illegal
     * diagonal move, and stop him if he did.
     */
    if (nh.x < 0 || nh.x > COLS-1 || nh.y < 0 || nh.y > LINES - 2 ||
      (pl_off(ISETHREAL) && !diag_ok(&hero, &nh))) {
	after = running = FALSE;
	return;
    }
    if (running) {
	ch = winat(nh.y, nh.x);
	if(dead_end(ch)) {
	    int gox, goy, apsg, whichway;

		gox = goy = apsg = 0;
		if(dy == 0) {
			ch = show(hero.y+1,hero.x);
			if(ch == PASSAGE) {
				apsg += 1;
				goy = 1;
			}
			ch = show(hero.y-1,hero.x);
			if(ch == PASSAGE) {
				apsg += 1;
				goy = -1;
			}
		}
		else if(dx == 0) {
			ch = show(hero.y,hero.x+1);
			if(ch == PASSAGE) {
				gox = 1;
				apsg += 1;
			}
			ch = show(hero.y,hero.x-1);
			if(ch == PASSAGE) {
				gox = -1;
				apsg += 1;
			}
		}
		if(apsg != 1) {
			running = after = FALSE;
			return;
		}
		else {			/* can still run here */
			nh.y = hero.y + goy;
			nh.x = hero.x + gox;
			whichway = (goy + 1) * 3 + gox + 1;
			switch(whichway) {
				case 0: runch = 'y';
				when 1: runch = 'k';
				when 2: runch = 'u';
				when 3: runch = 'h';
				when 4: runch = '.';	/* shouldn't do */
				when 5: runch = 'l';
				when 6: runch = 'b';
				when 7: runch = 'j';
				when 8: runch = 'n';
			}
		}
	}
    }
    if(running && ce(hero, nh)) {
	after = running = FALSE;
    }
    ch = winat(nh.y, nh.x);
    if (pl_on(ISHELD) && ch != 'F' && ch != 'd') {
	msg("You are being held");
	return;
    }
    if (pl_off(ISETHREAL)) {
	if(isatrap(ch)) {
	    ch = be_trapped(&nh);
	    if (ch == TRAPDOOR || ch == TELTRAP || 
	      ch == POST || pooltelep == TRUE) {
		pooltelep = FALSE;
		return;
	    }
	}
 	else if (dead_end(ch)) {
	    after = running = FALSE;
	    return;
	}
	else {
	    switch(ch) {
		case GOLD:	case POTION:	case SCROLL:
		case FOOD:	case WEAPON:	case ARMOR:
		case RING:	case AMULET:	case STICK:
		    running = FALSE;
		    take = ch;
		default:
		    if (illeg_ch(ch)) {
			running = FALSE;
			mvaddch(nh.y, nh.x, FLOOR);
			teleport();
			light(&nh);
			msg("The spatial warp disappears !");
			return;
		    }
	    }
	}
    }
    if ((ch == PASSAGE && winat(hero.y, hero.x) == DOOR) ||
      pl_on(ISETHREAL))
	light(&hero);
    else if (ch == DOOR) {
	running = FALSE;
	if (winat(hero.y, hero.x) == PASSAGE)
	    light(&nh);
    }
    else if (ch == STAIRS && pl_off(ISETHREAL))
	running = FALSE;
    else if (isalpha(ch) && pl_off(ISETHREAL)) {
	running = FALSE;
	fight(&nh, ch, cur_weapon, FALSE);
	return;
    }
    if (pl_on(ISBLIND))
	ch = ' ';
    else
	ch = winat(hero.y, hero.x);
    wmove(cw, unc(hero));
    waddch(cw, ch);
    hero = nh;
    wmove(cw, unc(hero));
    waddch(cw, PLAYER);
}

/*
 * Called to illuminate a room.
 * If it is dark, remove anything that might move.
 */

light(struct coord *cp)
{
    reg struct room *rp;
    reg int j, k, x, y;
    reg char ch, rch;
    reg struct linked_list *item;

    if ((rp = roomin(cp)) != NULL && pl_off(ISBLIND)) {
	if (iswearing(R_LIGHT))
	    rp->r_flags &= ~ISDARK;
	for (j = 0; j < rp->r_max.y; j += 1) {
	    for (k = 0; k < rp->r_max.x; k += 1) {
		y = rp->r_pos.y + j;
		x = rp->r_pos.x + k;
		ch = show(y, x);
		wmove(cw, y, x);
		/*
		 * Figure out how to display a secret door
		 */
		if (ch == SECRETDOOR) {
		    if (j == 0 || j == rp->r_max.y - 1)
			ch = '-';
		    else
			ch = '|';
		}
		/*
		 * If the room is a dark room, we might want to remove
		 * monsters and the like from it (since they might
		 * move)
		 */
		if (isalpha(ch)) {
		    struct thing *mit;

		    item = wake_monster(y, x);
		    if (item == NULL) {
			ch = FLOOR;
			mvaddch(y, x, ch);
		    }
		    else {
			mit = THINGPTR(item);
			if (mit->t_oldch == ' ')
			    if (!(rp->r_flags & ISDARK))
				mit->t_oldch = mvwinch(stdscr, y, x);
		    }
		}
		if (rp->r_flags & ISDARK) {
		    rch = mvwinch(cw, y, x);
		    if(isatrap(rch)) {
			ch = rch;	/* if its a trap */
		    }
		    else {		/* try other things */
			switch (rch) {
			    case DOOR:	case STAIRS:	case '|':
			    case '-':	case ' ':
				ch = rch;
			    otherwise:
				ch = ' ';
			}
		    }
		}
		mvwaddch(cw, y, x, ch);
	    }
	}
    }
}

/*
 * show:
 *	returns what a certain thing will display as to the un-initiated
 */
show(int y, int x)
{
    reg char ch = winat(y, x);
    reg struct linked_list *it;
    reg struct thing *tp;

    if(isatrap(ch)) {
	if(iswearing(R_FTRAPS))
	    trap_at(y,x)->tr_flags |= ISFOUND;
	return(trap_at(y,x)->tr_flags & ISFOUND) ?
		(trap_at(y,x)->tr_type) : FLOOR;
    }
    if(ch == SECRETDOOR && iswearing(R_FTRAPS)) {
	mvaddch(y,x,DOOR);
	return(DOOR);
    }
    if ((it = find_mons(y, x)) != NULL) {	/* maybe a monster */
	tp = THINGPTR(it);
	if (ch == 'M' || (tp->t_flags & ISINVIS)) {
	    if (ch == 'M')
		ch = tp->t_disguise;
	    else if(ch == 's' && !iswearing(R_SEEINVIS))
		ch = ' ';		/* shadows show as a blank */
	    else if (pl_off(CANSEE))
		ch = mvwinch(stdscr, y, x);	/* hide invisibles */
	}
    }
    return ch;
}

/*
 * be_trapped:
 *	The guy stepped on a trap.... Make him pay.
 */

be_trapped(struct coord *tc)
{
    reg struct trap *tp;
    reg char ch;
    reg int i;

    tp = trap_at(tc->y, tc->x);
    count = running = FALSE;
    mvwaddch(cw, tp->tr_pos.y, tp->tr_pos.x, (tp->tr_type));
    tp->tr_flags |= ISFOUND;
    switch (ch = tp->tr_type) {
	case POST:
	    new_level(TRUE);		/* build trading post */
	when TRAPDOOR:
	    level += 1;
	    new_level(FALSE);
	    msg("You fell into a trap!");
	when BEARTRAP:
	    no_move += BEARTIME;
	    msg("You are caught in a bear trap");
	when SLEEPTRAP:
	    if (pl_on(ISINVINC))
		msg("You feel momentarily dizzy");
	    else {
		no_command += SLEEPTIME;
		msg("You fall asleep in a strange white mist");
	    }
	when ARROWTRAP:
	    if (pl_off(ISINVINC) &&
	       (rnd(20) + 1) > pstats.s_lvl - herodex()) {
		msg("Oh no! An arrow shot you");
		chg_hpt(-roll(1,6),FALSE,1);
	    }
	    else {
		reg struct linked_list *item;
		reg struct object *arrow;
		reg int val = rnd(7) - 3;
		msg("An arrow shoots past you.");
		item = new_item(sizeof *arrow);
		arrow = OBJPTR(item);
		arrow->o_type = WEAPON;
		arrow->o_which = ARROW;
		init_weapon(arrow, ARROW);
		arrow->o_hplus = val;
		arrow->o_dplus =   setoflg(arrow,ISCURSED);
		arrow->o_count = 1;
		arrow->o_pos = hero;
		fall(item, FALSE);
	    }
	when TELTRAP:
	    teleport();
	when DARTTRAP:
	  if ((rnd(20) + 1 > pstats.s_lvl - herodex()) &&
	    !iswearing(R_SUSAB) && pl_off(ISINVINC)) {
		msg("A small dart just hit you in the shoulder");
		if(!save(VS_POISON))
		    chg_abil(CON,-1,TRUE);
		if (!iswearing(R_SUSTSTR))
		    chg_abil(STR,-1,TRUE);
		chg_hpt(-roll(1, 4),FALSE,2);
	    }
	    else
		msg("A small dart whizzes by your ear and vanishes.");
        when POOL:
        if ((tp->tr_flags & ISGONE) == 0) {
	    reg int wh;

	    tp->tr_flags |= ISGONE;	/* pool good only once */
	    if (cur_weapon != NULL && o_off(cur_weapon,ISPROT)) {
	    	wh = cur_weapon->o_which;
		setoflg(cur_weapon,ISKNOW);
		switch(cur_weapon->o_type) {
		case WEAPON:
		    if(rnd(100) < 20) {		/* enchant weapon here */
			if (o_off(cur_weapon,ISCURSED)) {
				cur_weapon->o_hplus += 1;
				cur_weapon->o_dplus += 1;
			}
			else {		/* weapon was prev cursed here */
				cur_weapon->o_hplus = rnd(2);
				cur_weapon->o_dplus = rnd(2);
			}
			resoflg(cur_weapon,ISCURSED);
		    }
		    else if(rnd(100) < 10) {	/* curse weapon here */
			if (o_off(cur_weapon,ISCURSED)) {
				cur_weapon->o_hplus = -(rnd(2)+1);
				cur_weapon->o_dplus = -(rnd(2)+1);
			}
			else {			/* if already cursed */
				cur_weapon->o_hplus--;
				cur_weapon->o_dplus--;
			}
			setoflg(cur_weapon,ISCURSED);
		    }			
		    msg("Your %s glows for a moment.",weaps[wh].w_name);
		when ARMOR:
		    if (rnd(100) < 30) {	/* enchant armor */
			if(o_off(cur_weapon,ISCURSED))
			    cur_weapon->o_ac -= rnd(2) + 1;
			else
			    cur_weapon->o_ac = -rnd(3)+ armors[wh].a_class;
			resoflg(cur_weapon,ISCURSED);
		    }
		    else if(rnd(100) < 15){	/* curse armor */
			if (o_off(cur_weapon,ISCURSED))
			    cur_weapon->o_ac = rnd(3)+ armors[wh].a_class;
			else
			    cur_weapon->o_ac += rnd(2) + 1;
			setoflg(cur_weapon,ISCURSED);
		    }
		    msg("Your %s glows for a moment.",armors[wh].a_name);
		when STICK:
		    i = rnd(8) + 1;
		    if(rnd(100) < 25)		/* add charges */
			cur_weapon->o_charges += i;
		    else if(rnd(100) < 10) {	/* remove charges */
			if(cur_weapon->o_charges < i)
			    cur_weapon->o_charges = 0;
			else
			    cur_weapon->o_charges -= i;
		    }
		    ws_know[wh] = TRUE;
		    msg("Your %s %s glows for a moment.",
			ws_made[wh],ws_type[wh]);
		when SCROLL:
		    s_know[wh] = TRUE;
		    msg("Your '%s' scroll unfurls",s_names[wh]);
		when POTION:
		    p_know[wh] = TRUE;
		    msg("Your %s potion bubbles for a moment",p_colors[wh]);
		when RING:
		    r_know[wh] = TRUE;
		    if(magring(cur_weapon)) {
			if(rnd(100) < 25) {	 /* enchant ring */
			    if (o_off(cur_weapon,ISCURSED))
				cur_weapon->o_ac += rnd(2) + 1;
			    else
				cur_weapon->o_ac = rnd(2) + 1;
			    resoflg(cur_weapon,ISCURSED);
			}
			else if(rnd(100) < 10) { /* curse ring */
			    if (o_off(cur_weapon,ISCURSED))
				cur_weapon->o_ac = -(rnd(2) + 1);
			    else
				cur_weapon->o_ac -= (rnd(2) + 1);
			    setoflg(cur_weapon,ISCURSED);
			}
		    }
		    msg("Your %s ring vibrates for a moment",r_stones[wh]);
		otherwise:
		    msg("The pool bubbles for a moment.");
		}
	}
	    else msg("You get all wet.");	/* if no weap in hand */
    }
    else {	/* if trap used, then maybe other things can happen */
	if(rnd(100) < 10) {
	    pooltelep = TRUE;  /* indicate this for rest of do_move */
	    if(rnd(100) < 15) {
		teleport();	   /* teleport away */
	    }
	    else if(rnd(100) < 15 && level > 2) {
		level -= rnd(2) + 1;		/* go up a level or 2 */
		new_level(FALSE);		/* keep in dungeon */
		msg("You here a faint groan from below");
	    }
	    else if(rnd(100) < 40) {		/* go down a level 0 to 3*/
		level += rnd(4);
		new_level(FALSE);
		msg("You find yourself in strange surroundings");
	    }
	    else if(rnd(100) < 6) {
		msg("Oh no!!! You drown in the pool!!! --More--");
		wait_for(' ');
		death(4);
	    }
	    else
		 pooltelep = FALSE;	/* if not one of the above */
	}
    }
    }			/* end of trap switch casing */
    flushout();		/* flush typeahead */
    return(ch);
}

/*
 * trap_at:
 *	find the trap at (y,x) on screen.
 */

struct trap *
trap_at(int y, int x)
{
    reg struct trap *tp, *ep;

    ep = &traps[ntraps];
    for (tp = traps; tp < ep; tp += 1)
	if (tp->tr_pos.y == y && tp->tr_pos.x == x)
	    break;
    if (tp == ep) {
	sprintf(errbuf,"unknown trap at %d,%d",y,x);
	debug(errbuf);
	tp = traps;	/* point to 1st trap */
    }
    return tp;
}

/*
 * rndmove:
 *	move in a random direction if the monster/person is confused
 */

struct coord *
rndmove(struct thing *who)
{
    reg int x, y;
    reg char ch;
    reg int ex, ey, nopen = 0;
    reg struct linked_list *item;
    reg struct object *obj;
    static struct coord ret;  /* what we will be returning */
    static struct coord dest;

    ret = who->t_pos;
    /*
     * Now go through the spaces surrounding the player and
     * set that place in the array to true if the space can be
     * moved into
     */
    ey = ret.y + 1;
    ex = ret.x + 1;
    for (y = who->t_pos.y - 1; y <= ey; y += 1)
	if (y >= 0 && y < LINES)
	    for (x = who->t_pos.x - 1; x <= ex; x += 1) {
		if (x < 0 || x >= COLS)
		    continue;
		ch = winat(y, x);
		if (step_ok(ch)) {
		    dest.y = y;
		    dest.x = x;
		    if (!diag_ok(&who->t_pos, &dest))
			continue;
		    if (ch == SCROLL) {
			item = NULL;
			for (item=lvl_obj;item != NULL;item = next(item)) {
			    obj = OBJPTR(item);
			    if (y == obj->o_pos.y && x == obj->o_pos.x)
				break;
			}
			if (item != NULL && obj->o_which == S_SCARE)
			    continue;
		    }
		    if (rnd(++nopen) == 0)
			ret = dest;
		}
	    }
    return &ret;
}

/*
 * isatrap:
 *	Returns TRUE if this character is some kind of trap
 */
isatrap(char ch)
{
	switch(ch) {
		case POST:
		case DARTTRAP:
		case POOL:
		case TELTRAP:
		case TRAPDOOR:
		case ARROWTRAP:
		case SLEEPTRAP:
		case BEARTRAP:	return(TRUE);
		default:	return(FALSE);
	}
}
