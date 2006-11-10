/*
 * Code for one object to chase another
 *
 * @(#)chase.c	7.0	(Bell Labs)	10/08/82
 */

#include "rogue.h"
#include "rogue_ext.h"

struct coord ch_ret;			/* Where chasing takes you */

/*
 * runners:
 *	Make all the running monsters move.
 */

int do_chase (struct thing *th);
int chase (struct thing *tp, struct coord *ee);
extern int attack (struct thing *mp);
int cansee (int y, int x);
int diag_ok (struct coord *sp, struct coord *ep);
extern int winat (int y, int x);
extern int step_ok (char ch);

runners(void)
{
    reg struct linked_list *item;
    reg struct thing *tp;

    for (item = mlist; item != NULL; item = next(item)) {
	tp = THINGPTR(item);
	if (off(*tp, ISHELD) && on(*tp, ISRUN)) {
	    if (off(*tp, ISSLOW) || tp->t_turn)
		do_chase(tp);
	    if (on(*tp, ISHASTE))
		do_chase(tp);
	    tp->t_turn ^= TRUE;
	}
    }
}


/*
 * do_chase:
 *	Make one thing chase another.
 */
do_chase(struct thing *th)
{
    reg struct room *rer, *ree;
    reg int mindist = 32767, i, dist;
    reg bool stoprun = FALSE;
    reg char sch;
    struct coord this;			/* Temporary dest for chaser */

    rer = roomin(&th->t_pos);		/* Find room of chaser */
    ree = roomin(th->t_dest);		/* Find room of chasee */
    /*
     * We don't count doors as inside rooms for this routine
     */
    if (mvwinch(stdscr, th->t_pos.y, th->t_pos.x) == DOOR)
	rer = NULL;
    this = *th->t_dest;
    /*
     * If the object of our desire is in a different room, 
     * than we are and we ar not in a corridor, run to the
     * door nearest to our goal.
     */
    if (rer != NULL && rer != ree)
	for (i = 0; i < rer->r_nexits; i += 1) {
	    dist = DISTANCE(th->t_dest->y, th->t_dest->x,
			    rer->r_exit[i].y, rer->r_exit[i].x);
	    if (dist < mindist) {		/* minimize distance */
		this = rer->r_exit[i];
		mindist = dist;
	    }
	}
    /*
     * this now contains what we want to run to this time
     * so we run to it.  If we hit it we either want to fight it
     * or stop running
     */
    if (!chase(th, &this)) {
	if (ce(this, hero)) {
	    isfight = TRUE;
	    attack(th);
	    return;
	}
	else if (!(th->t_flags & (ISSTUCK | ISPARA)))
	    stoprun = TRUE;
    }
    else if ((th->t_flags & (ISSTUCK | ISPARA)))
	return;				/* if paralyzed or stuck */
    mvwaddch(cw, th->t_pos.y, th->t_pos.x, th->t_oldch);
    sch = mvwinch(cw, ch_ret.y, ch_ret.x);
    if (rer != NULL && (rer->r_flags & ISDARK) && sch == FLOOR
	&& DISTANCE(ch_ret.y, ch_ret.x, th->t_pos.y, th->t_pos.x) < 3
	&& pl_off(ISBLIND))
	    th->t_oldch = ' ';
    else
	th->t_oldch = sch;

    if (cansee(unc(ch_ret)) && (th->t_flags & ISINVIS) == 0)
        mvwaddch(cw, ch_ret.y, ch_ret.x, th->t_type);
    mvwaddch(mw, th->t_pos.y, th->t_pos.x, ' ');
    mvwaddch(mw, ch_ret.y, ch_ret.x, th->t_type);
    th->t_pos = ch_ret;
    /*
     * And stop running if need be
     */
    if (stoprun && ce(th->t_pos, *(th->t_dest)))
	th->t_flags &= ~ISRUN;
}


/*
 * runto:
 *	Set a monster running after something
 */
runto(struct coord *runner, struct coord *spot)
{
    reg struct linked_list *item;
    reg struct thing *tp;

    if ((item = find_mons(runner->y, runner->x)) == NULL)
	return;
    tp = THINGPTR(item);
    if (tp->t_flags & ISPARA)
	return;
    tp->t_dest = spot;
    tp->t_flags |= ISRUN;
    tp->t_flags &= ~ISHELD;
}


/*
 * chase:
 *	Find the spot for the chaser(er) to move closer to the
 *	chasee(ee).  Returns TRUE if we want to keep on chasing later
 *	FALSE if we reach the goal.
 */
chase(struct thing *tp, struct coord *ee)
{
    reg int x, y;
    reg int dist, thisdist;
    reg struct linked_list *item;
    reg struct object *obj;
    reg struct coord *er = &tp->t_pos;
    reg char ch;

    /*
     * If the thing is confused, let it move randomly. Invisible
     * Stalkers are slightly confused all of the time, and bats are
     * quite confused all the time
     */
    if (on(*tp, ISHUH) ||
       (tp->t_type == 'I' && rnd(100) < 20) ||
       (tp->t_type == 'B' && rnd(100) < 50)) {
	/*
	 * get a valid random move
	 */
	ch_ret = *rndmove(tp);
	dist = DISTANCE(ch_ret.y, ch_ret.x, ee->y, ee->x);
	/*
	 * Small chance that it will become un-confused 
	 */
	if (rnd(1000) < 25)
	    tp->t_flags &= ~ISHUH;
    }
    /*
     * Otherwise, find the empty spot next to the chaser that is
     * closest to the chasee.
     */
    else {
	reg int ey, ex;

	/*
	 * This will eventually hold where we move to get closer
	 * If we can't find an empty spot, we stay where we are.
	 */
	dist = DISTANCE(er->y, er->x, ee->y, ee->x);
	ch_ret = *er;

	ey = er->y + 1;
	ex = er->x + 1;
	for (x = er->x - 1; x <= ex; x += 1)
	    for (y = er->y - 1; y <= ey; y += 1) {
		struct coord tryp;

		tryp.x = x;
		tryp.y = y;
		if (!diag_ok(er, &tryp))
		    continue;
		ch = winat(y, x);
		if (step_ok(ch)) {
		    /*
		     * If it is a scroll, it might be a scare monster.
		     * so we need to look it up to see what type it is.
		     */
		    if (ch == SCROLL) {
			for(item = lvl_obj;item != NULL;item = next(item)){
			    obj = OBJPTR(item);
			    if (y == obj->o_pos.y && x == obj->o_pos.x)
				break;
			}
			if (item != NULL && obj->o_which == S_SCARE)
			    continue;
		    }
		    /*
		     * If we didn't find any scrolls at this place or it
		     * wasn't a scare scroll, then this place counts
		     */
		    thisdist = DISTANCE(y, x, ee->y, ee->x);
		    if (thisdist < dist) {
			ch_ret = tryp;
			dist = thisdist;
		    }
		}
	    }
    }
    return (dist != 0);
}


/*
 * roomin:
 *	Find what room some coordinates are in. NULL means they aren't
 *	in any room.
 */
struct room *
roomin(struct coord *cp)
{
    reg struct room *rp;

    for (rp = rooms; rp < &rooms[MAXROOMS]; rp += 1)
	if (inroom(rp, cp))
	    return rp;
    return NULL;
}


/*
 * find_mons:
 *	Find the monster from his coordinates
 */
struct linked_list *
find_mons(int y, int x)
{
    reg struct linked_list *item;
    reg struct thing *th;

    for (item = mlist; item != NULL; item = next(item)) {
	th = THINGPTR(item);
	if (th->t_pos.y == y && th->t_pos.x == x)
	    return item;
    }
    return NULL;
}


/*
 * diag_ok:
 *	Check to see if the move is legal if it is diagonal
 */
diag_ok(struct coord *sp, struct coord *ep)
{
    if (ep->x == sp->x || ep->y == sp->y)
	return TRUE;
    return (step_ok(mvinch(ep->y,sp->x)) && step_ok(mvinch(sp->y,ep->x)));
}


/*
 * cansee:
 *	returns true if the hero can see a certain coordinate.
 */
cansee(int y, int x)
{
    reg struct room *rer;
    struct coord tp;

    if (pl_on(ISBLIND))
	return FALSE;
    tp.y = y;
    tp.x = x;
    rer = roomin(&tp);
    /*
     * We can only see if the hero in the same room as
     * the coordinate and the room is lit or if it is close.
     */
    return(rer!=NULL && rer == roomin(&hero) && !(rer->r_flags&ISDARK)) ||
	    DISTANCE(y, x, hero.y, hero.x) < 3;
}
