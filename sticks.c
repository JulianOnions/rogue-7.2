/*
 * Functions to implement the various sticks one might find
 * while wandering around the dungeon.
 *
 * @(#)sticks.c	7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include "rogue.ext"


/*
 * fix_stick:
 *	Init a stick for the hero
 */
fix_stick(cur)
reg struct object *cur;
{
    cur->o_charges = 4 + rnd(5);
    cur->o_hurldmg = "1d1";
    if (strcmp(ws_type[cur->o_which], "staff") == 0) {
	cur->o_weight = 100;
	cur->o_damage = "2d3";
	cur->o_charges += rnd(5) + 1;
    }
    else {
	cur->o_weight = 60;
	cur->o_damage = "1d1";
    }
    switch (cur->o_which) {
	case WS_HIT:
	    if(rnd(100) < 15) {
		cur->o_hplus = 9;
		cur->o_dplus = 9;
		cur->o_damage = "3d8";
	    }
	    else {
		cur->o_hplus = 3;
		cur->o_dplus = 3;
		cur->o_damage = "1d8";
	    }
	when WS_LIGHT:
	    cur->o_charges += 7 + rnd(9);
    }
}

do_zap(gotdir)
bool gotdir;
{
    reg struct linked_list *item;
    reg struct object *obj;
    reg struct room *rp;
    reg struct thing *tp;
    reg int y, x, wh;

    if ((item = get_item("zap with", STICK)) == NULL) {
	return;
    }
    obj = OBJPTR(item);
    wh = obj->o_which;
    if (obj->o_type != STICK) {
	msg("You can't zap with that!");
	after = FALSE;
	return;
    }
    if (obj->o_charges == 0) {
	msg("Nothing happens.");
	return;
    }
    if (!gotdir)
	do {
	    delta.y = rnd(3) - 1;
	    delta.x = rnd(3) - 1;
	} while (delta.y == 0 && delta.x == 0);
    switch (wh) {
	case WS_SAPLIFE:
	    if(pstats.s_hpt > 1)
		pstats.s_hpt /= 2;	/* zap half his hit points */
	when WS_CURE:
	    ws_know[WS_CURE] = TRUE;
	    pstats.s_hpt += roll(pstats.s_lvl,6) + 3;
	    if(pstats.s_hpt > max_hp)
		pstats.s_hpt = max_hp;
	when WS_PYRO:
	    msg("The rod explodes !!!");
	    chg_hpt(-roll(5,6),FALSE,5);
	    ws_know[WS_PYRO] = TRUE;
	    del_pack(item);		/* throw it away */
	when WS_HUNGER: {
	    struct linked_list *ip; struct object *eobj;
	    food_left /= 3;
	    if((ip = pack) != NULL) {
		eobj = OBJPTR(ip);
		if(eobj->o_type == FOOD) {
		    if((eobj->o_count -= roll(1,4)) < 1) {
			--inpack;
			detach(pack,ip);
			discard(ip);
		    }
		}
	    }
	}
	when WS_PARZ:
	case WS_MREG:
	case WS_MDEG:
	case WS_ANNIH: {
	    struct linked_list *mitem;
	    struct thing *it;
	    int i,j;
	    for(i = hero.y - 3 ; i <= hero.y + 3 ; i++) {
		for(j = hero.x - 3; j <= hero.x + 3 ; j++) {
		    if(isalpha(mvwinch(mw,i,j))) {
			if((mitem = find_mons(i,j)) != NULL) {
			    it = THINGPTR(mitem);
			    switch(wh) {
			    case WS_ANNIH:
				killed(mitem,FALSE);
			    when WS_MREG:
				it->t_stats.s_hpt *= 2;
			    when WS_MDEG:
				it->t_stats.s_hpt /= 2;
				if (it->t_stats.s_hpt < 1)
				    killed(mitem,FALSE);
			    when WS_PARZ:
				it->t_flags |= ISPARA;
				it->t_flags &= ~ISRUN;
			    }
			}
		    }
		}
	    }
	}
	when WS_LIGHT:
	    /*
	     * Reddy Kilowat wand.  Light up the room
	     */
	    ws_know[WS_LIGHT] = TRUE;
	    if ((rp = roomin(&hero)) == NULL)
		msg("The corridor glows and then fades");
	    else {
		msg("The room is lit.");
		rp->r_flags &= ~ISDARK;
		/*
		 * Light the room and put the player back up
		 */
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	    }
	when WS_DRAIN:
	    /*
	     * Take away 1/2 of hero's hit points, then take it away
	     * evenly from the monsters in the room (or next to hero
	     * if he is in a passage)
	     */
	    if (pstats.s_hpt < 2) {
		msg("You are too weak to use it.");
		return;
	    }
	    else if ((rp = roomin(&hero)) == NULL)
		drain(hero.y-1, hero.y+1, hero.x-1, hero.x+1);
	    else
		drain(rp->r_pos.y, rp->r_pos.y+rp->r_max.y,
		    rp->r_pos.x, rp->r_pos.x+rp->r_max.x);
	when WS_POLYMORPH:
	case WS_TELAWAY:
	case WS_TELTO:
	case WS_CANCEL:
	case WS_MINVIS:
	{
	    reg char monster, oldch;
	    reg int rm;

	    y = hero.y;
	    x = hero.x;
	    while (step_ok(winat(y, x))) {
		y += delta.y;
		x += delta.x;
	    }
	    if (isalpha(monster = mvwinch(mw, y, x))) {
		reg char omonst = monster;

		if (wh != WS_MINVIS &&
		   (monster == 'F' || monster == 'd'))
		    player.t_flags &= ~ISHELD;
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (wh == WS_POLYMORPH) {
		    detach(mlist, item);
		    oldch = tp->t_oldch;
		    delta.y = y;
		    delta.x = x;
		    new_monster(item,monster=randmonster(FALSE,TRUE),
			&delta,FALSE);
		    if (!(tp->t_flags & ISRUN))
			runto(&delta, &hero);
		    if (isalpha(mvwinch(cw, y, x)))
			mvwaddch(cw, y, x, monster);
		    tp->t_oldch = oldch;
		    ws_know[WS_POLYMORPH] |= (monster != omonst);
		}
		else if (wh == WS_MINVIS) {
		    tp->t_flags |= ISINVIS;
		    mvwaddch(cw,y,x,tp->t_oldch);	/* hide em */
		    if (!(tp->t_flags & ISRUN))
			runto(&tp->t_pos, &hero);
		}
		else if (wh == WS_CANCEL) {
		    tp->t_flags |= ISCANC;
		    tp->t_flags &= ~ISINVIS;
		}
		else {
		    if(wh == WS_TELAWAY) {
			do {
			    rm = rnd_room();
			    rnd_pos(&rooms[rm], &tp->t_pos);
			} until(winat(tp->t_pos.y, tp->t_pos.x) == FLOOR);
		    }
		    else {
			tp->t_pos.y = hero.y + delta.y;
			tp->t_pos.x = hero.x + delta.x;
		    }
		    if (isalpha(mvwinch(cw, y, x)))
			mvwaddch(cw, y, x, tp->t_oldch);
		    tp->t_dest = &hero;
		    tp->t_flags |= ISRUN;
		    mvwaddch(mw, y, x, ' ');
		    mvwaddch(mw, tp->t_pos.y, tp->t_pos.x, monster);
		    if (tp->t_pos.y != y || tp->t_pos.x != x)
			tp->t_oldch = mvwinch(cw,tp->t_pos.y,tp->t_pos.x);
		}
	    }
	}
	when WS_MISSILE:
	{
	    static struct object bolt = {
		'*', {0, 0}, "", 0, 0, "6d6", 0, 0, 100, 1
	    };

	    do_motion(&bolt, delta.y, delta.x);
	    if (isalpha(mvwinch(mw, bolt.o_pos.y, bolt.o_pos.x))
		&& !save_throw(VS_MAGIC,ldata(find_mons(unc(bolt.o_pos)))))
		    hit_monster(unc(bolt.o_pos), &bolt);
	    else
		msg("Missle vanishes");
	    ws_know[WS_MISSILE] = TRUE;
	}
	when WS_NOP:
	{
		msg("Your %s glows red and then fades",
			ws_type[wh]);
	}
	when WS_HIT:
	{
	    reg char ch;

	    delta.y += hero.y;
	    delta.x += hero.x;
	    ch = winat(delta.y, delta.x);
	    if (isalpha(ch)) {
		fight(&delta, ch, obj, FALSE);
	    }
	}
	when WS_HASTE_M:
	case WS_CONFMON:
	case WS_SLOW_M:
	case WS_MOREMON: {
	    reg int m1,m2;
	    struct coord mp;
	    struct linked_list *titem;
	    y = hero.y;
	    x = hero.x;
	    while (step_ok(winat(y, x))) {
		y += delta.y;
		x += delta.x;
	    }
	    if (isalpha(mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (wh == WS_HASTE_M) {	/* haste it */
		    if (on(*tp, ISSLOW))
			tp->t_flags &= ~ISSLOW;
		    else
			tp->t_flags |= ISHASTE;
		}
		else if (wh == WS_CONFMON) {
		    tp->t_flags |= ISHUH;
		}
		else if (wh == WS_SLOW_M) {	/* slow it */
		    if (on(*tp, ISHASTE))
			tp->t_flags &= ~ISHASTE;
		    else
			tp->t_flags |= ISSLOW;
		    tp->t_turn = TRUE;
		}
		else {			/* WS_MOREMON: multiply it */
		    char ch;
		    struct thing *th;
		    for (m1=tp->t_pos.x-1 ; m1 <= tp->t_pos.x+1 ; m1++) {
			for(m2=tp->t_pos.y-1 ; m2<=tp->t_pos.y+1 ; m2++) {
			    ch = winat(m2,m1);
			    if (step_ok(ch) && ch != PLAYER) {
				mp.x = m1;	/* create it */
				mp.y = m2;
				titem = new_item(sizeof(struct thing));
				new_monster(titem,tp->t_type,&mp,FALSE);
				th = THINGPTR(titem);
				th->t_flags |= ISMEAN;
				runto(&mp,&hero);
			    }
			}
		    }
		}
		delta.y = y;
		delta.x = x;
		runto(&delta, &hero);
	    }
	}
	when WS_ELECT:
	case WS_FIRE:
	case WS_COLD:
	{
	    reg char dirch, ch, *name;
	    reg bool bounced, used;
	    int boingcnt;
	    struct coord pos;
	    struct coord spotpos[BOLT_LENGTH];
	    static struct object bolt =
	    {
		'*' , {0, 0}, "", 0, 0, "6d6" , 0, 0, 100, 0
	    };
	    switch (delta.y + delta.x) {
		case 0: dirch = '/';
		when 1: case -1: dirch = (delta.y == 0 ? '-' : '|');
		when 2: case -2: dirch = '\\';
	    }
	    pos = hero;
	    bounced = FALSE;
	    boingcnt = 0;
	    used = FALSE;
	    if (wh == WS_ELECT)
		name = "bolt";
	    else if (wh == WS_FIRE)
		name = "flame";
	    else
		name = "ice";
	    for (y = 0; y < BOLT_LENGTH && !used; y++) {
		ch = winat(pos.y, pos.x);
		spotpos[y] = pos;
		switch (ch) {
		    case SECRETDOOR:
		    case '|':
		    case '-':
		    case ' ':
			    bounced = TRUE;
			    if (++boingcnt > 6) 
				used = TRUE;	/* only so many bounces */
			    delta.y = -delta.y;
			    delta.x = -delta.x;
			    y--;
			    msg("The bolt bounces");
			    break;
		    default:
			if (!bounced && isalpha(ch)) {
			    if(!save_throw(VS_MAGIC,
			      ldata(find_mons(unc(pos))))) {
				bolt.o_pos = pos;
				hit_monster(unc(pos), &bolt);
				used = TRUE;
			    }
			    else if(ch!='M' || show(pos.y,pos.x) == 'M') {
				    msg("%s misses", name);
				runto(&pos, &hero);
			    }
			}
			else if (bounced && pos.y == hero.y &&
			         pos.x == hero.x) {
			    bounced = FALSE;
			    if (!save(VS_MAGIC)) {
				msg("The %s hits you", name);
				chg_hpt(-roll(6, 6),FALSE,3);
				used = TRUE;
			    }
			    else
				msg("The %s whizzes by you", name);
			}
			mvwaddch(cw, pos.y, pos.x, dirch);
			draw(cw);
		}
		pos.y += delta.y;
		pos.x += delta.x;
	    }
	    for (x = 0; x < y; x++)
		mvwaddch(cw, spotpos[x].y, spotpos[x].x,
		   show(spotpos[x].y, spotpos[x].x));
	    ws_know[wh] = TRUE;
	}
	when WS_ANTIM: {
	    reg int m1, m2, x1, y1;
	    char ch;
	    struct linked_list *ll;
	    struct object *lp;
	    struct thing *lt;

	    y1 = hero.y;
	    x1 = hero.x;
	    do {
		y1 += delta.y;
		x1 += delta.x;
		ch = winat(y1,x1);
	    } while (ch == PASSAGE || ch == FLOOR);
	    for (m1 = x1 - 1 ; m1 <= x1 + 1 ; m1++) {
		for(m2 = y1 - 1 ; m2 <= y1 + 1 ; m2++) {
		    ch = winat(m2,m1);
		    if (m1 == hero.x && m2 == hero.y)
			continue;
		    if (ch != ' ') {
			ll = find_obj(m2,m1);
			if (ll != NULL) {
			    detach(lvl_obj,ll);
			    discard(ll);
			}
			ll = find_mons(m2,m1);
			if (ll != NULL) {
			    detach(mlist,ll);
			    discard(ll);
			    mvwaddch(mw,m2,m1,' ');
			}
			mvaddch(m2,m1,' ');
			mvwaddch(cw,m2,m1,' ');
		    }
		}
	    }
	    touchwin(cw);
	    touchwin(mw);
	}
	otherwise:
	    msg("What a bizarre schtick!");
    }
    obj->o_charges--;
}

/*
 * drain:
 *	Do drain hit points from player shtick
 */

drain(ymin, ymax, xmin, xmax)
int ymin, ymax, xmin, xmax;
{
    reg int i, j, cnt;
    reg struct thing *ick;
    reg struct linked_list *item;

    /*
     * First count how many things we need to spread the hit points among
     */
    cnt = 0;
    for (i = ymin; i <= ymax; i++)
	for (j = xmin; j <= xmax; j++)
	    if (isalpha(mvwinch(mw, i, j)))
		cnt++;
    if (cnt == 0) {
	msg("You have a tingling feeling");
	return;
    }
    cnt = pstats.s_hpt / cnt;
    pstats.s_hpt /= 2;
    /*
     * Now zot all of the monsters
     */
    for (i = ymin; i <= ymax; i++)
	for (j = xmin; j <= xmax; j++)
	    if(isalpha(mvwinch(mw, i, j)) &&
	      ((item = find_mons(i, j)) != NULL)) {
		ick = THINGPTR(item);
		if ((ick->t_stats.s_hpt -= cnt) < 1)
		  killed(item,cansee(i,j) && (ick->t_flags & ISINVIS)==0);
	    }
}

/*
 * charge a wand for wizards.
 */
char *
charge_str(obj)
reg struct object *obj;
{
    static char buf[20];

    if (o_off(obj,ISKNOW))	/* quit if he doesnt know */
	buf[0] = NULL;
    else
	sprintf(buf, " [%d]", obj->o_charges);
    return buf;
}
