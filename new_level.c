/*
 * Do anything associated with a new dungeon level
 *
 * @(#)new_level.c	7.0	(Bell Labs)	10/15/82
 */

#include "rogue.h"
#include "rogue.ext"

/*
 * new_level:
 *	Dig and draw a new level 
 */
new_level(post)
bool post;
{
	reg struct linked_list *mptr;
	reg struct thing *tptr;
	reg int rm, i, cnt;
	reg char ch;
	struct coord traploc;

	if (level > max_level)
		max_level = level;
	wclear(cw);
	wclear(mw);
	clear();
	for (mptr = mlist; mptr != NULL; mptr = next(mptr)) {
		tptr = THINGPTR(mptr);
		free_list(tptr->t_pack);	/* free all monsters packs */
	}
	free_list(mlist);			/* free monster list */
	/*
	 * If a player is carrying too much weight and was fighting
	 *  at the time and he comes down to the new level, make sure
	 *  the fighting flag is reset
	 */
	fuse(notfight,TRUE,1,BEFORE);
	if (post) {
		tradelev = TRUE;	/* at trading post */
		do_post();		/* do it !! */
	}
	else {
	    tradelev = FALSE;		/* no flea-market */
	    lev_mon();			/* fill in monster list */
	    do_rooms();			/* Draw rooms */
	    do_passages();		/* Draw passages */
	    no_food++;
	    put_things();		/* Place objects (if any) */
	}
	/*
	 * Place the staircase down.
	 */
	cnt = 0;
	do {
	    rm = (post ? 0 : rnd_room());
	    rnd_pos(&rooms[rm], &stairs);
	    if(++cnt > 500) {
		debug("Stair failure in new_level");
		break;		/* use this spot anyway */
	    }
	} until (winat(stairs.y, stairs.x) == FLOOR);
	mvaddch(stairs.y,stairs.x,STAIRS);
	if (!post) {
	/*
	 * Place the traps (dont do this for trading posts)
	 */
	    if (rnd(10) < level) {
		ntraps = rnd(level / 4) + 1;
		if (ntraps > MAXTRAPS)
		    ntraps = MAXTRAPS;
		for(i = 0 ; i < ntraps ; i++) {
		    traps[i].tr_flags = 0;
		    cnt = 0;
		    do {
			rm = rnd_room();
			rnd_pos(&rooms[rm], &traploc);
			if (++cnt > 500) {
			    debug("Trap failed in new_level");
			    traps[i].tr_flags |= ISFOUND;
			    break;
			}
		    } until (winat(traploc.y, traploc.x) == FLOOR);
again:
		    switch(rnd(TYPETRAPS + 1)) {
			case 0: if (rnd(100) > 25)
				    goto again;
				else
				    ch = POST;
			when 1: ch = TRAPDOOR;
			when 2: ch = BEARTRAP;
			when 3: ch = SLEEPTRAP;
			when 4: ch = ARROWTRAP;
			when 5: ch = TELTRAP;
			when 6: ch = DARTTRAP;
			when 7:
			case 8: if (rnd(100) > 80)
				    goto again;
				else
				    ch = POOL;
		    }
		    mvaddch(traploc.y,traploc.x,ch);
		    traps[i].tr_type = ch;
		    traps[i].tr_pos = traploc;
		    if(ch == POOL || ch == POST)
			traps[i].tr_flags |= ISFOUND;
		}
	    }
	}
	do {
	    rm = rnd_room();
	    rnd_pos(&rooms[rm], &hero);
	    if(++cnt > 500) {
		debug("Hero failure in new_level");
		break;			/* use this spot anyway */
	    }
	} until(winat(hero.y, hero.x) == FLOOR);
	light(&hero);
	mvwaddch(cw,hero.y,hero.x,PLAYER);
	extinguish(chkstairs);
	chkstairs(FALSE);		/* check for stairs */
	if(roomin(&hero) == NULL) {	/* defensive check */
	    sprintf(errbuf,"Hero not in a room");
	    debug(errbuf);
	    if (!author())
		msg(errbuf);		/* show everybody */
	}
	nochange = FALSE;		/* will redisply stats */
}


/*
 * rnd_room:
 *	Pick a room that is really there
 */
rnd_room()
{
	reg int rm;

	do {
		rm = rnd(MAXROOMS);
	} while (rooms[rm].r_flags & ISGONE);
	return rm;
}


/*
 * put_things:
 *	put potions and scrolls on this level
 */

put_things()
{
	reg int i, cnt;
	reg struct linked_list *item;
	reg struct object *cur;
	reg int rm;
	struct coord tp;

	/*
	 * Throw away stuff left on the previous level (if anything)
	 */
	free_list(lvl_obj);
	/*
	 * The only way to get new stuff is to go down into the dungeon.
	 */
	if (level < max_level)
		return;
	/*
	 * Do MAXOBJ attempts to put things on a level
	 */
	for (i = 0; i < MAXOBJ; i++)
	if (rnd(100) < 40) {
	    /*
	     * Pick a new object and link it in the list
	     */
	    item = new_thing(FALSE);	/* not in treasure room */
	    attach(lvl_obj, item);
	    cur = OBJPTR(item);
	    /*
	     * Put it somewhere
	     */
	    cnt = 0;
	    do {
		int cnta = 0;
		do {
		    rm = rnd_room();		/* skip treasure rooms */
		    if(++cnta > 500)		/* use it anyway */
			break;
		} while(rooms[rm].r_flags & ISTREAS);
		rnd_pos(&rooms[rm], &tp);
		if(++cnt > 500) {
			debug("Item failure");
			break;			/* use this spot anyway */
		}
	    } until (winat(tp.y, tp.x) == FLOOR);
	    mvaddch(tp.y, tp.x, cur->o_type);
	    cur->o_pos = tp;
	}
	/*
	 * If he is really deep in the dungeon and he hasn't found the
	 * amulet yet, put it somewhere on the ground
	 */
	if (level >= AMLEVEL && !amulet && rnd(100) < 70) {
		item = new_item(sizeof *cur);
		attach(lvl_obj, item);
		cur = OBJPTR(item);
		cur->o_hplus = cur->o_dplus = 0;
		cur->o_damage = "0d0";
		cur->o_hurldmg = "40d8";	/* if thrown, WOW!!! */
		cur->o_ac = 11;
		cur->o_count = 1;
		cur->o_type = AMULET;
		cur->o_weight = things[TYP_AMULET].mi_wght;
		cnt = 0;
		do {
		    rm = rnd_room();
		    rnd_pos(&rooms[rm], &tp);
		    if(++cnt > 500) {
			debug("Amulet failure");
			break;
		    }
		} until (winat(tp.y, tp.x) == FLOOR);
		mvaddch(tp.y, tp.x, cur->o_type);
		cur->o_pos = tp;
	}
	for(i = 0;i < MAXROOMS;i++) {		/* loop through all */
	    if(rooms[i].r_flags & ISTREAS) {	/*  treasure rooms */
		int numthgs;

		numthgs = rnd(level / 3) + 6;
		while(numthgs-- >= 0) {
		    do {
			item = new_thing(TRUE);
			cur = OBJPTR(item);
		    } while(cur->o_type == FOOD );
		    attach(lvl_obj,item);
		    cnt = 0;
		    do {
			rnd_pos(&rooms[i],&tp);
			if(++cnt > 1000)
			     break;
		    } until(winat(tp.y,tp.x) == FLOOR);
		    mvaddch(tp.y,tp.x,cur->o_type);
		    cur->o_pos = tp;
		}
	    }
	}
}
