/*
 * Anything to do with trading posts
 *
 * #(@)trader.c	7.0	(Bell Labs)	10/22/82
 */

#include "rogue.h"
#include "rogue_ext.h"


/*
 * do_post:
 *	Put a trading post room and stuff on the screen
 */

extern int _free_list (register struct linked_list **ptr);
extern int draw_room (struct room *rp);
extern int roll (int number, int sides);
extern int _attach (register struct linked_list **list, register struct linked_list *item);
extern int setoflg (struct object *what, int bit);
extern int rnd_pos (struct room *rp, struct coord *cp);
int trans_line (void);
int open_market (void);
int get_worth (struct object *obj);
extern void msg (const char *fmt, ...);
extern int readchar (void);
extern int add_pack (struct linked_list *item, NCURSES_BOOL silent);
extern int drop (struct linked_list *item);
extern int magring (struct object *what);

do_post(void)
{
	struct coord tp;
	reg int i;
	reg struct room *rp;
	reg struct object *op;
	reg struct linked_list *ll;

	free_list(lvl_obj);		/* throw old items away */

	for (rp = rooms; rp < &rooms[MAXROOMS]; rp++) {
	    rp->r_goldval = 0;			/* no gold */
	    rp->r_nexits = 0;			/* no exits */
	    rp->r_flags = ISGONE;		/* kill all rooms */
	}
	rp = &rooms[0];				/* point to only room */
	rp->r_flags = 0;			/* this room NOT gone */
	rp->r_max.x = 40;
	rp->r_max.y = 10;			/* 10 * 40 room */
	rp->r_pos.x = (COLS - rp->r_max.x) / 2;	/* center horizontal */
	rp->r_pos.y = 1;			/* 2nd line */
	draw_room(rp);				/* draw the only room */
	i = roll(4,10);				/* 10 to 40 items */
	for (; i > 0 ; i--) {			/* place all the items */
	    ll = new_thing(FALSE);		/* get something */
	    attach(lvl_obj, ll);
	    op = OBJPTR(ll);
	    setoflg(op, ISPOST);	/* object in trading post */
	    do {
		rnd_pos(rp,&tp);
	    } until (mvinch(tp.y, tp.x) == FLOOR);
	    op->o_pos = tp;
	    mvaddch(tp.y,tp.x,op->o_type);
	}
	trader = 0;
	wmove(cw,12,0);
	waddstr(cw,"Welcome to Friendly Fiend's Flea Market\n\r");
	waddstr(cw,"=======================================\n\r");
	waddstr(cw,"$: Prices object that you stand upon.\n\r");
	waddstr(cw,"#: Buys the object that you stand upon.\n\r");
	waddstr(cw,"%: Trades in something in your pack for gold.\n\r");
	trans_line();
}

/*
 * price_it:
 *	Price the object that the hero stands on
 */
price_it(void)
{
	static char *bargain[] = {
	    "great bargain",
	    "quality product",
	    "exceptional find",
	};
	reg struct linked_list *item;
	reg struct object *obj;
	reg int worth;
	reg char *str;

	if (!open_market())		/* after buying hours */
	    return FALSE;
	if ((item = find_obj(hero.y,hero.x)) == NULL)
	    return FALSE;
	obj = OBJPTR(item);
	worth = get_worth(obj);
	if (worth <= 0) {
	    msg("That's not for sale.");
	    return FALSE;
	}
	worth *= 3;		/* slightly expensive */
	str = typ_name(obj);
	sprintf(prbuf,"That %s is a %s for only %d pieces of gold",
		str,bargain[rnd(3)],worth);
	msg(prbuf);
	curprice = worth;		/* save price */
	strcpy(curpurch,str);		/* save item */
	return TRUE;
}

/*
 * buy_it:
 *	Buy the item on which the hero stands
 */
buy_it(void)
{
	reg int wh;
	char buff[LINLEN];

	if (purse <= 0) {
	    msg("You have no money.");
	    return;
	}
	if (curprice < 0) {		/* if not yet priced */
	    wh = price_it();
	    if (!wh)			/* nothing to price */
		return;
	    msg("Do you want to buy it? ");
	    do {
		wh = lower(readchar());
		if (wh == ESCAPE || wh == 'n') {
		    msg("");
		    return;
		}
	    } until(wh == 'y');
	}
	mpos = 0;
	if (curprice > purse) {
	    sprintf(buff,"You can't afford to buy that %s !",curpurch);
	    msg(buff);
	    return;
	}
	/*
	 * See if the hero has done all his transacting
	 */
	if (!open_market())
	    return;
	/*
	 * The hero bought the item here
	 */
	mpos = 0;
	wh = add_pack(NULL,FALSE);	/* try to put it in his pack */
	if (wh) {			/* he could get it */
	    purse -= curprice;		/* take his money */
	    ++trader;			/* another transaction */
	    trans_line();		/* show remaining deals */
	    curprice = -1;		/* reset stuff */
	    curpurch[0] = 0;
	}
}

/*
 * sell_it:
 *	Sell an item to the trading post
 */
sell_it(void)
{
	reg struct linked_list *item;
	reg struct object *obj;
	reg int wo, ch;
	char buff[LINLEN];

	if (!open_market())		/* after selling hours */
	    return;

	if ((item = get_item("sell",0)) == NULL)
	    return;
	obj = OBJPTR(item);
	wo = get_worth(obj);
	if (wo <= 0) {
	    mpos = 0;
	    msg("We don't buy those.");
	    return;
	}
	sprintf(buff,"Your %s is worth %d pieces of gold.",
		typ_name(obj),wo);
	msg(buff);
	msg("Do you want to sell it? ");
	do {
	    ch = lower(readchar());
	    if (ch == ESCAPE || ch == 'n') {
		msg("");
		return;
	    }
	} until (ch == 'y');
	mpos = 0;
	if (drop(item) == TRUE) {		/* drop this item */	
	    nochange = FALSE;			/* show gold value */
	    purse += wo;			/* give him his money */
	    ++trader;				/* another transaction */
	    sprintf(buff,"Sold %s",inv_name(obj,TRUE));
	    msg(buff);
	    trans_line();			/* show remaining deals */
	}
}

/*
 * open_market:
 *	Retruns TRUE when ok do to transacting
 */
open_market(void)
{
	if (trader >= MAXPURCH) {
	    msg("The market is closed. The stairs are that-a-way.");
	    return FALSE;
	}
	else {
	    return TRUE;
	}
}

/*
 * typ_name:
 * 	Return the name for this type of object
 */
char *
typ_name(struct object *obj)
{
	static char buff[20];
	reg int wh;

	switch (obj->o_type) {
		case POTION:  wh = TYP_POTION;
		when SCROLL:  wh = TYP_SCROLL;
		when STICK:   wh = TYP_STICK;
		when RING:    wh = TYP_RING;
		when ARMOR:   wh = TYP_ARMOR;
		when WEAPON:  wh = TYP_WEAPON;
		when AMULET:  wh = TYP_AMULET;
		when FOOD:    wh = TYP_FOOD;
		otherwise:    wh = -1;
	}
	if (wh < 0)
		strcpy(buff,"unknown");
	else
		strcpy(buff,things[wh].mi_name);
	return (buff);
}

/*
 * get_worth:
 *	Calculate an objects worth in gold
 */
get_worth(struct object *obj)
{
	reg int worth, wh;

	worth = 0;
	wh = obj->o_which;
	switch (obj->o_type) {
	    case FOOD:
		worth = 2;
	    when WEAPON:
		if (wh < MAXWEAPONS) {
		    worth = weaps[wh].w_worth;
		    worth *= (2 + (10 * obj->o_hplus + 10 * obj->o_dplus));
		}
	    when ARMOR:
		if (wh < MAXARMORS) {
		    worth = armors[wh].a_worth;
		    worth *= (1 + (10 * (armors[wh].a_class - obj->o_ac)));
		}
	    when SCROLL:
		if (wh < MAXSCROLLS)
		    worth = s_magic[wh].mi_worth;
	    when POTION:
		if (wh < MAXPOTIONS)
		    worth = p_magic[wh].mi_worth;
	    when RING:
		if (wh < MAXRINGS) {
		    worth = r_magic[wh].mi_worth;
		    if (magring(obj)) {
			if (obj->o_ac > 0)
			    worth += obj->o_ac * 40;
			else
			    worth = 50;
		    }
		}
	    when STICK:
		if (wh < MAXSTICKS) {
		    worth = ws_magic[wh].mi_worth;
		    worth += 20 * obj->o_charges;
		}
	    when AMULET:
		worth = 1000;
	    otherwise:
		worth = 0;
	}
	if (worth < 25)
	    worth = 25;
	return (worth);
}

/*
 * trans_line:
 *	Show how many transactions the hero has left
 */
trans_line(void)
{
	sprintf(prbuf,"You have %d transactions remaining.",
		MAXPURCH - trader);
	mvwaddstr(cw,LINES - 3,0,prbuf);
}
