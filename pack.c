/*
 * Routines to deal with the pack
 *
 * @(#)pack.c	7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include "rogue.ext"

/*
 * add_pack:
 * Pick up an object and add it to the pack.  If the argument is non-null
 * use it as the linked_list pointer instead of getting it off the ground.
 */
add_pack(item, silent)
reg struct linked_list *item;
bool silent;
{
    reg struct linked_list *ip, *lp;
    reg struct object *obj, *op;
    reg bool exact, from_floor;

    if (item == NULL) {
	from_floor = TRUE;
	if ((item = find_obj(hero.y, hero.x)) == NULL)
	    return FALSE;
    }
    else
	from_floor = FALSE;
    obj = OBJPTR(item);
	/*
	 * See if this guy can carry any more weight
	 */
    if(itemweight(obj) + pstats.s_pack > pstats.s_carry) {
	msg("Too much for you to carry.");
	return FALSE;
    }
    /*
     * Link it into the pack. Search the pack for a object of similar type.
     * If there isn't one, put it at the start, if there is, look for one
     * that is exactly the same and just increment the count if there is.
     * it that. Food is always put at the beginning for ease of access,
     * but is not ordered so that you can't tell good food from bad.
     * First check to see if there is something in the same group and
     * if there is then increment the count.
     */
    if (obj->o_group) {
	for (ip = pack; ip != NULL; ip = next(ip)) {
	    op = OBJPTR(ip);
	    if (op->o_group == obj->o_group) {

		/*
		 * Put it in the pack and notify the user
		 */
		op->o_count++;
		if (from_floor)	{
	 		detach(lvl_obj, item);
	  		mvaddch(hero.y, hero.x,
			  (roomin(&hero) == NULL ? PASSAGE : FLOOR));
		}
		discard(item);
		item = ip;
		goto picked_up;
	    }
	}
    }
    /*
     * Check if there is room
     */
    if ((inpack + (obj->o_group ? 1 : obj->o_count)) > MAXPACK) {
	msg("You can't carry anything else.");
	return FALSE;
    }

    /*
     * Check for and deal with scare monster scrolls
     */
    if (obj->o_type == SCROLL && (obj->o_which == S_SCARE))
	if (o_on(obj,ISFOUND)) {
	    msg("The scroll turns to dust as you pick it up.");
	    detach(lvl_obj, item);
	    discard(item);
	    mvaddch(hero.y, hero.x, FLOOR);
	    return FALSE;
	}
	else
	    setoflg(obj,ISFOUND);

    inpack += obj->o_group ? 1 : obj->o_count;
    if (from_floor) {
	detach(lvl_obj, item);
	mvaddch(hero.y,hero.x,(roomin(&hero) == NULL ? PASSAGE : FLOOR));
    }
    /*
     * Search for an object of the same type
     */
    exact = FALSE;
    for (ip = pack; ip != NULL; ip = next(ip)) {
	op = OBJPTR(ip);
	if (obj->o_type == op->o_type)
	    break;
    }
    if (ip == NULL) {
	/*
	 * Put it at the end of the pack since it is a new type
	 */
	for (ip = pack; ip != NULL; ip = next(ip)) {
	    op = OBJPTR(ip);
	    if (op->o_type != FOOD)
		break;
	    lp = ip;
	}
    }
    else {
	/*
	 * Search for an object which is exactly the same
	 * We must keep objects that are from trading posts and
	 * objects that aren't seperate, however.
	 */
	while (ip != NULL && op->o_type == obj->o_type) {
	    if (op->o_which == obj->o_which &&
	      o_on(op, ISPOST) == o_on(obj, ISPOST)) {
		exact = TRUE;
		break;
	    }
	    lp = ip;
	    if ((ip = next(ip)) == NULL)
		break;
	    op = OBJPTR(ip);
	}
    }
    if (ip == NULL) {
	/*
	 * Didn't find an exact match, just stick it here
	 */
	if (pack == NULL)
	    pack = item;
	else {
	    lp->l_next = item;
	    item->l_prev = lp;
	    item->l_next = NULL;
	}
    }
    else {
	/*
	 * If we found an exact match.  If it is a potion, food, or a 
	 * scroll, increase the count, otherwise put it with its clones.
	 */
	if (exact && ISMULT(obj->o_type)) {
	    op->o_count += obj->o_count;
	    discard(item);
	    item = ip;
	    goto picked_up;
	}
	if ((item->l_prev = prev(ip)) != NULL)
	    item->l_prev->l_next = item;
	else
	    pack = item;
	item->l_next = ip;
	ip->l_prev = item;
    }
picked_up:
    /*
     * Notify the user
     */
    obj = OBJPTR(item);
    if (notify && !silent)
	msg("%s (%c)",inv_name(obj,FALSE),pack_char(obj));
    if (obj->o_type == AMULET)
	amulet = TRUE;
    updpack(FALSE);			/* new pack weight */
    nochange = FALSE;
    return TRUE;
}

/*
 * inventory:
 *	Show what items are in a specific list
 */
inventory(list, type)
struct linked_list *list;
int type;
{
    reg struct linked_list *pc;
    reg struct object *obj;
    reg char ch;
    reg int cnt;
    char inv_temp[LINLEN];

    if (list == NULL) {			/* empty list */
	msg(type == 0 ? "Empty handed." : "Nothing appropriate.");
	return FALSE;
    }
    else if (next(list) == NULL) {	/* only 1 item in list */
	obj = OBJPTR(list);
	sprintf(inv_temp, "a) %s", inv_name(obj, FALSE));
	msg(inv_temp);
	return TRUE;
    }
    cnt = 0;
    wclear(hw);
    for (ch = 'a', pc = list; pc != NULL; pc = next(pc)) {
	obj = OBJPTR(pc);
	wprintw(hw,"%c) %s\n\r",ch,inv_name(obj, FALSE));
	if (++ch > 'z')
	    ch = 'A';
	if (++cnt >= LINES - 2) {
	    dbotline(hw, morestr);	/* if screen would scroll */
	    cnt = 0;
	    wclear(hw);
	} 
    }
    dbotline(hw,spacemsg);
    restscr(cw);
    return TRUE;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
pick_up(ch)
char ch;
{
    nochange = FALSE;
    switch(ch) {
	case GOLD:
	    money();
	when ARMOR:
	case POTION:
	case FOOD:
	case WEAPON:
	case SCROLL:	
	case AMULET:
	case RING:
	case STICK:
	    add_pack((struct linked_list *)NULL, FALSE);
	otherwise:
	    sprintf(errbuf,"pickup: %s.",unctrl(ch));
	    debug(errbuf);
    }
}

/*
 * picky_inven:
 *	Allow player to inventory a single item
 */
picky_inven()
{
    reg struct linked_list *item;
    reg char ch, mch;

    if (pack == NULL)
	msg("You aren't carrying anything");
    else if (next(pack) == NULL)
	msg("a) %s", inv_name(OBJPTR(pack), FALSE));
    else {
	msg("Item: ");
	mpos = 0;
	if ((mch = readchar()) == ESCAPE) {
	    msg("");
	    return;
	}
	for (ch = 'a', item = pack; item != NULL; item = next(item), ch++)
	    if (ch == mch) {
		msg("%c) %s",ch,inv_name(OBJPTR(item), FALSE));
		return;
	    }
	msg("Range is 'a' to '%c'", --ch);
    }
}

/*
 * get_item:
 *	pick something out of a pack for a purpose
 */
struct linked_list *
get_item(purpose, type)
char *purpose;
int type;
{
    reg struct linked_list *obj, *pit, *savepit;
    struct object *pob;
    reg char ch, och, anr;
    int cnt;

    if (pack == NULL)
	msg("You aren't carrying anything.");
    else  {
	/* see if we have any of the type requested */
	if(type != 0 && type != CALLABLE) {
	    pit = pack;
	    anr = 0;
	    for(ch = 'a' ; pit != NULL ; pit = next(pit), ch++) {
		pob = OBJPTR(pit);
		if(type == pob->o_type) {
		    ++anr;
		    savepit = pit;	/* save in case of only 1 */
		}
	    }
	    if (anr == 0 && type != WEAPON) {
		msg("Nothing to %s",purpose);
		after = FALSE;
		return NULL;
	    }
	    else if (anr == 1) {	/* only found one of 'em */
		do {
		    struct object *opb;
		    opb = OBJPTR(savepit);
		    msg("%s what (* for the item)?",purpose);
		    och = lower(readchar());
		    if (och == '*') {
			mpos = 0;
			msg("%c) %s",pack_char(opb),inv_name(opb,FALSE));
			continue;
		    }
		    if (och == ESCAPE) {
			msg("");
			after = FALSE;
			return NULL;
		    }
		    if (type != WEAPON) {
			if(isalpha(och) && och != pack_char(opb)) {
			    mpos = 0;
			    msg("You can't %s that",purpose);
			    after = FALSE;
			    return NULL;
			}
		    }
		} while(!isalpha(och));
		mpos = 0;
		return savepit;		/* return this item */
	    }
	}
	for (;;) {
	    msg("%s what? (* for list): ",purpose);
	    ch = readchar();
	    mpos = 0;
	    if (ch == ESCAPE) {		/* abort if escape hit */
		after = FALSE;
		msg("");		/* clear display */
		return NULL;
	    }
	    if (ch == '*') {
		wclear(hw);
		pit = pack;		/* point to pack */
		cnt = 0;
		for(ch = 'a'; pit != NULL ; pit = next(pit), ch++) {
		    pob = OBJPTR(pit);
		    if(type==0 || type==CALLABLE || type==pob->o_type) {
		      wprintw(hw,"%c) %s\n\r",ch,inv_name(pob,FALSE));
		      if (++cnt >= LINES - 2 && next(pit) != NULL) {
			cnt = 0;
			dbotline(hw, morestr);
			wclear(hw);
		      }
		    }
		}
		wmove(hw, LINES - 1,0);
		wprintw(hw,"%s what? ",purpose);
		draw(hw);		/* write screen */
		anr = FALSE;
		do {
		    ch = lower(readchar());
		    if (isalpha(ch) || ch == ESCAPE)
			anr = TRUE; 
		} while(!anr);		/* do till we got it right */
		restscr(cw);		/* redraw orig screen */
		if(ch == ESCAPE) {
		    after = FALSE;
		    msg("");		/* clear top line */
		    return NULL;	/* all done if abort */
		}
		/* ch has item to get from pack */
	    }
	    for(obj = pack,och = 'a'; obj != NULL;obj = next(obj),och++)
		if (ch == och)
		    break;
	    if (obj == NULL) {
		msg("Please specify a letter between 'a' and '%c'", och-1);
		continue;
	    }
	    else 
		return obj;
	}
    }
    return NULL;
}

/*
 * pack_char:
 *	Get the character of a particular item in the pack
 */
char
pack_char(obj)
reg struct object *obj;
{
    reg struct linked_list *item;
    reg char c;

    c = 'a';
    for (item = pack; item != NULL; item = next(item))
	if (OBJPTR(item) == obj)
	    return c;
	else
	    c++;
    return 'z';
}

/*
 * idenpack:
 *	Identify all the items in the pack
 */
idenpack()
{
	reg struct linked_list *pc;

	for (pc = pack ; pc != NULL ; pc = next(pc))
		whatis(pc);
}


/* 
 * del_pack:
 *	Take something out of the hero's pack
 */
del_pack(what)
reg struct linked_list *what;
{
	reg struct object *op;

	inpack--;
	op = OBJPTR(what);
	cur_null(op);		/* check for current stuff */
	if (op->o_count > 1) {
	    op->o_count--;
	}
	else {
	    detach(pack,what);
	    discard(what);
	}
	updpack(FALSE);
}

/*
 * cur_null:
 *	This updates cur_weapon etc for dropping things
 */
cur_null(op)
reg struct object *op;
{
	if (op == cur_weapon)
	    cur_weapon = NULL;
	else if (op == cur_armor)
	    cur_armor = NULL;
	else if (op == cur_ring[LEFT])
	    cur_ring[LEFT] = NULL;
	else if (op == cur_ring[RIGHT])
	    cur_ring[RIGHT] = NULL;
}
