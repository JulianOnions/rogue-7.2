/*
 * Contains functions for dealing with things like
 * potions and scrolls
 *
 * @(#)things.c		7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include "rogue_ext.h"


/*
 * inv_name:
 *	return the name of something as it would appear in an
 *	inventory.
 */

extern int o_on (struct object *what, int bit);
extern void debug (char *errstr);
extern void msg (const char *fmt, ...);
int dropcheck (struct object *op);
extern int _detach (register struct linked_list **list, register struct linked_list *item);
extern int discard (register struct linked_list *item);
extern int fall (struct linked_list *item, NCURSES_BOOL pr);
extern int _attach (register struct linked_list **list, register struct linked_list *item);
extern int updpack (int getmax);
extern int cur_null (struct object *op);
extern int waste_time (void);
extern int toss_ring (struct object *what);
int pick_one (struct magic_item *magic, int nitems);
int extras (void);
extern int init_weapon (struct object *weap, char type);
extern int setoflg (struct object *what, int bit);
extern int init_ring (struct object *what, NCURSES_BOOL fromwiz);
extern int fix_stick (struct object *cur);
extern int author (void);

char *
inv_name(struct object *obj, NCURSES_BOOL drop)
{
    reg char *pb;
    reg int wh;

    wh = obj->o_which;
    switch(obj->o_type) {
	case SCROLL:
	    if (obj->o_count == 1)
		strcpy(prbuf, "A scroll ");
	    else
		sprintf(prbuf, "%d scrolls ", obj->o_count);
	    pb = &prbuf[strlen(prbuf)];
	    if (s_know[wh] || o_on(obj,ISPOST))
		sprintf(pb, "of %s", s_magic[wh].mi_name);
	    else if (s_guess[wh])
		sprintf(pb, "called %s", s_guess[wh]);
	    else
		sprintf(pb, "titled '%s'", s_names[wh]);
	    break;
        case POTION:
	    if (obj->o_count == 1)
		strcpy(prbuf, "A potion ");
	    else
		sprintf(prbuf, "%d potions ", obj->o_count);
	    pb = &prbuf[strlen(prbuf)];
	    if (p_know[wh] || o_on(obj, ISPOST)) {
		sprintf(pb, "of %s", p_magic[wh].mi_name);
		if (p_know[wh]) {
		    pb = &prbuf[strlen(prbuf)];
		    sprintf(pb,"(%s)",p_colors[wh]);
		}
	    }
	    else if (p_guess[wh])
		sprintf(pb,"called %s(%s)", p_guess[wh],p_colors[wh]);
	    else if (obj->o_count == 1)
		sprintf(prbuf, "A%s %s potion",vowelstr(p_colors[wh]),
		    p_colors[wh]);
	    else
		sprintf(prbuf,"%d %s potions",obj->o_count,p_colors[wh]);
	    break;
	case FOOD:
	    if (wh == 1)
		if (obj->o_count == 1)
		    sprintf(prbuf, "A%s %s", vowelstr(fruit), fruit);
		else
		    sprintf(prbuf, "%d %ss", obj->o_count, fruit);
	    else
		if (obj->o_count == 1)
		    strcpy(prbuf, "Some food");
		else
		    sprintf(prbuf, "%d rations of food", obj->o_count);
	    break;
	case WEAPON:
	    if (obj->o_count > 1)
		sprintf(prbuf, "%d ", obj->o_count);
	    else
		strcpy(prbuf, "A ");
	    pb = &prbuf[strlen(prbuf)];
	    if (o_on(obj,ISKNOW) || o_on(obj, ISPOST))
		sprintf(pb, "%s %s", num(obj->o_hplus, obj->o_dplus),
		    weaps[wh].w_name);
	    else
		sprintf(pb, "%s", weaps[wh].w_name);
	    if (obj->o_count > 1)
		strcat(prbuf, "s");
	    break;
	    case ARMOR:
	    if (o_on(obj,ISKNOW) || o_on(obj, ISPOST))
		sprintf(prbuf, "%s %s",
		    num(armors[wh].a_class - obj->o_ac, 0),
		    armors[wh].a_name);
	    else
		sprintf(prbuf, "%s", armors[wh].a_name);
	    break;
	    case AMULET:
	    strcpy(prbuf, "The Amulet of Yendor");
	    break;
	case STICK:
	    sprintf(prbuf, "A %s ", ws_type[wh]);
	    pb = &prbuf[strlen(prbuf)];
	    if (ws_know[wh] || o_on(obj, ISPOST)) {
		sprintf(pb,"of %s%s",ws_magic[wh].mi_name,charge_str(obj));
		if (ws_know[wh]) {
		    pb = &prbuf[strlen(prbuf)];
		    sprintf(pb,"(%s)",ws_made[wh]);
		}
	    }
	    else if (ws_guess[wh])
		sprintf(pb, "called %s(%s)", ws_guess[wh],
		    ws_made[wh]);
	    else
		sprintf(&prbuf[2], "%s %s", ws_made[wh],
		    ws_type[wh]);
	    break;
        case RING:
	    if (r_know[wh] || o_on(obj, ISPOST)) {
		sprintf(prbuf, "A%s ring of %s", ring_num(obj),
		    r_magic[wh].mi_name);
		if (r_know[wh]) {
		    pb = &prbuf[strlen(prbuf)];
		    sprintf(pb,"(%s)", r_stones[wh]);
		}
	    }
	    else if (r_guess[wh])
		sprintf(prbuf, "A ring called %s(%s)",
		    r_guess[wh], r_stones[wh]);
	    else
	      sprintf(prbuf,"A%s %s ring",vowelstr(r_stones[wh]),
		    r_stones[wh]);
	    break;
	default:
	    sprintf(errbuf, "Something bizarre %s", unctrl(obj->o_type));
	    debug(errbuf);
	    break;
    }
    if (obj == cur_armor)
	strcat(prbuf, " (being worn)");
    if (obj == cur_weapon)
	strcat(prbuf, " (weapon in hand)");
    if (obj == cur_ring[LEFT])
	strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[RIGHT])
	strcat(prbuf, " (on right hand)");
    if (drop && isupper(prbuf[0]))
	prbuf[0] = lower(prbuf[0]);
    else if (!drop && islower(*prbuf))
	*prbuf = toupper(*prbuf);
    if (!drop)
	strcat(prbuf, ".");
    return prbuf;
}

/*
 * money:
 *	Add to characters purse
 */
money(void)
{
    reg struct room *rp;

    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	if (ce(hero, rp->r_gold)) {
	    if (notify)
		msg("%d gold pieces.", rp->r_goldval);
	    purse += rp->r_goldval;
	    rp->r_goldval = 0;
	    cmov(rp->r_gold);
	    addch(FLOOR);
	    return;
	}
    msg("That gold must have been counterfeit");
}


/*
 * drop:
 *	put something down
 */
drop(struct linked_list *item)
{
    reg char ch;
    reg struct linked_list *ll, *nll;
    reg struct object *op;
    reg char *ptr;

    if (item == NULL) {
	ch = mvwinch(stdscr, hero.y, hero.x);
	if (ch != FLOOR && ch != PASSAGE && ch != POOL) {
	    msg("There is something there already.");
	    after = FALSE;
	    return SOMTHERE;
	}
	if ((ll = get_item("drop", 0)) == NULL)
	    return FALSE;
    }
    else {
	ll = item;
    }
    op = OBJPTR(ll);
    if (!dropcheck(op))
	return CANTDROP;
    /*
     * Take it out of the pack
     */
    if (op->o_count >= 2 && op->o_type != WEAPON) {
	nll = new_item(sizeof *op);
	op->o_count--;
	op = OBJPTR(nll);
	*op = *(OBJPTR(ll));
	op->o_count = 1;
	ll = nll;
	if (op->o_group != 0)
		inpack++;
    }
    else
	detach(pack, ll);
    inpack--;
    if(ch == POOL) {
	ptr = inv_name(op,TRUE);
	if (strncmp("some",ptr,4) == 0 || strncmp("the",ptr,3) == 0
	 || *ptr == 'a') {
	    while(*ptr != ' ') ptr++;
	    ptr++;
	}
	msg("Your %s sinks out of sight.",ptr);
	discard(ll);
    }
    else {			/* put on dungeon floor */
	if (tradelev) {
	    op->o_pos = hero;	/* same place as hero */
	    fall(ll,FALSE);
	    if (item == NULL)	/* if item wasn't sold */
		msg("Thanks for your donation to the Fiend's flea market.");
	}
	else {
	    attach(lvl_obj, ll);
	    mvaddch(hero.y, hero.x, op->o_type);
	    op->o_pos = hero;
	    msg("Dropped %s", inv_name(op, TRUE));
	}
    }
    updpack(FALSE);			/* new pack weight */
    return TRUE;
}


/*
 * dropcheck:
 *	Do special checks for dropping or unweilding|unwearing|unringing
 */
dropcheck(struct object *op)
{
    if (op == NULL)
	return TRUE;
    if (tradelev) {
	if (o_on(op,ISCURSED) && o_on(op,ISKNOW)) {
	    msg("The trader does not accept shoddy merchandise.");
	    return FALSE;
	}
	else {
	    cur_null(op);	/* update cur_weapon, etc */
	    return TRUE;
	}
    }
    if (op != cur_armor && op != cur_weapon
	&& op != cur_ring[LEFT] && op != cur_ring[RIGHT])
	    return TRUE;
    if (o_on(op,ISCURSED)) {
	msg("You can't.  It appears to be cursed.");
	return FALSE;
    }
    if (op == cur_weapon)
	cur_weapon = NULL;
    else if (op == cur_armor) {
	waste_time();
	cur_armor = NULL;
    }
    else if (op == cur_ring[LEFT] || op == cur_ring[RIGHT])
	toss_ring(op);
    return TRUE;
}


/*
 * new_thing:
 *	Return a new thing
 */
struct linked_list *
new_thing(NCURSES_BOOL treas)
{
    reg struct linked_list *item;
    reg struct object *cur;
    reg int chance, whi;

    item = new_item(sizeof *cur);
    cur = OBJPTR(item);
    cur->o_hplus = cur->o_dplus = 0;
    cur->o_damage = cur->o_hurldmg = "0d0";
    cur->o_ac = 11;
    cur->o_count = 1;
    cur->o_group = 0;
    cur->o_flags = 0;
    cur->o_weight = 0;
    /*
     * Decide what kind of object it will be
     * If we haven't had food for a while, let it be food.
     */
    if (no_food > 3 && !treas)
	whi = TYP_FOOD;
    else
	whi = pick_one(things, NUMTHINGS);

    switch (whi) {
    case TYP_POTION:
	cur->o_type = POTION;
	cur->o_which = pick_one(p_magic, MAXPOTIONS);
	cur->o_weight = things[TYP_POTION].mi_wght;
	cur->o_count += extras();
	break;
    case TYP_SCROLL:
	cur->o_type = SCROLL;
	cur->o_which = pick_one(s_magic, MAXSCROLLS);
	cur->o_weight = things[TYP_SCROLL].mi_wght;
	cur->o_count += extras();
	break;
    case TYP_FOOD:
	no_food = 0;
	cur->o_weight = things[TYP_FOOD].mi_wght;
	cur->o_type = FOOD;
	cur->o_count += extras();
	if (rnd(100) > 15)
	    cur->o_which = 0;	/* normal food */
	else
	    cur->o_which = 1;	/* hero's "fruit" */
	break;
    case TYP_WEAPON:
	cur->o_type = WEAPON;
	cur->o_which = rnd(MAXWEAPONS);
	init_weapon(cur, cur->o_which);
	if ((chance = rnd(100)) < 10) {
	    setoflg(cur,ISCURSED);
	    cur->o_hplus -= rnd(3)+1;
	    cur->o_dplus -= rnd(3)+1;
	}
	else if (chance < 15) {
	    cur->o_hplus += rnd(3)+1;
	    cur->o_dplus += rnd(3)+1;
	}
	break;
    case TYP_ARMOR:
    {
	int tot = 0, wha = 0;
	cur->o_type =ARMOR;
	for (chance = rnd(100); wha < MAXARMORS; wha++) {
	    tot += armors[wha].a_prob;
	    if (chance < tot)
		break;
	}
	if (wha == MAXARMORS) {
	    sprintf(errbuf,"Picked a bad armor %d",chance);
	    debug(errbuf);
	    wha = 0;
	}
	cur->o_which = wha;
	cur->o_ac = armors[wha].a_class;
	cur->o_weight = armors[wha].a_wght;
	if ((chance = rnd(100)) < 20) {
	    setoflg(cur,ISCURSED);
	    cur->o_ac += rnd(3)+1;
	}
	else if (chance < 30)
	    cur->o_ac -= rnd(3)+1;
    }
	break;
    case TYP_RING:
	cur->o_type = RING;
	cur->o_which = pick_one(r_magic, MAXRINGS);
	init_ring(cur,FALSE);
	break;
    case TYP_STICK:
    default:
	cur->o_type = STICK;
	cur->o_which = pick_one(ws_magic, MAXSTICKS);
	fix_stick(cur);
	break;
    }
    return item;
}


/*
 * extras:
 *	Return the number of extra items to be created
 */
extras(void)
{
	reg int i;

	i = rnd(100);
	if (i < 4)		/* 4% for 2 more */
	    return (2);
	else if (i < 11)	/* 7% for 1 more */
	    return (1);
	else			/* otherwise no more */
	    return (0);
}


/*
 * pick an item out of a list of nitems possible magic items
 */
pick_one(struct magic_item *magic, int nitems)
{
    reg struct magic_item *end;
    reg int i;
    reg struct magic_item *start;

    start = magic;
    for (end = &magic[nitems], i = rnd(1000); magic < end; magic++)
	if (i < magic->mi_prob)
	    break;
    if (magic == end) {
	sprintf(errbuf,"bad pick_one: %d from %d items", i, nitems);
	debug(errbuf);
	if (author() || wizard) {
	    for (magic = start; magic < end; magic++)
		msg("%s: %d%%", magic->mi_name, magic->mi_prob);
	}
	magic = start;
    }
    return magic - start;
}
