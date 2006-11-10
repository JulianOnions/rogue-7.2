/*
 * Read a scroll and let it happen
 *
 * @(#)scrolls.c	7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include "rogue_ext.h"


/*
 * read_scroll:
 *	Let the hero read a scroll
 */

extern void msg (const char *fmt, ...);
extern int del_pack (struct linked_list *what);
extern int idenpack (void);
extern int chg_abil (register int what, register int amt, register int how);
extern int light (struct coord *cp);
extern int o_off (struct object *what, int bit);
extern int o_on (struct object *what, int bit);
extern int resoflg (struct object *what, int bit);
extern int step_ok (char ch);
extern int winat (int y, int x);
extern int new_monster (struct linked_list *item, char type, struct coord *cp, NCURSES_BOOL treas);
extern int randmonster (NCURSES_BOOL wander, NCURSES_BOOL baddie);
extern int whatis (struct linked_list *what);
extern int displevl (void);
extern int teleport (void);
extern int setoflg (struct object *what, int bit);
extern int aggravate (void);
extern int genocide (void);
extern int new_level (NCURSES_BOOL post);
extern int chg_hpt (int howmany, NCURSES_BOOL alsomax, char whichmon);
extern int roll (int number, int sides);
extern int create_obj (NCURSES_BOOL fscr);
extern int look (NCURSES_BOOL wakeup);
extern int get_str (register char *opt, WINDOW *awin);

read_scroll(void)
{
    reg struct object *obj;
    reg struct linked_list *item;
    reg struct room *rp;
    reg int i, j, wh;
    reg char ch, nch;
    reg struct linked_list *titem;
    char buf[LINLEN];

    if ((item = get_item("read", SCROLL)) == NULL)
	return;

    obj = OBJPTR(item);
    if (obj->o_type != SCROLL) {
	msg("Nothing to read");
	return;
    }
    msg("As you read the scroll, it vanishes.");
    wh = obj->o_which;

    del_pack(item);		/* Get rid of the thing */

    /*
     * Calculate the effect it has on the hero
     */
    switch(wh) {
	case S_KNOWALL:
		idenpack();	/* identify all the pack */
		msg("You feel more knowledgable.");
		chg_abil(WIS,1,TRUE);
	when S_CONFUSE:
	    /*
	     * Scroll of monster confusion.  Give him that power.
	     */
	    msg("Your hands begin to glow red.");
	    player.t_flags |= CANHUH;
	    s_know[S_CONFUSE] = TRUE;
	when S_LIGHT:
	    if ((rp = roomin(&hero)) == NULL) {
		s_know[S_LIGHT] = TRUE;
		msg("The corridor glows and then fades.");
	    }
	    else {
		if (rp->r_flags & ISDARK) {
		    s_know[S_LIGHT] = TRUE;
		    msg("The room is lit.");
		    rp->r_flags &= ~ISDARK;
		    light(&hero);
		    mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
	    }
	when S_ARMOR:
	    if (cur_armor != NULL && o_off(cur_armor,ISPROT)) {
		s_know[S_ARMOR] = TRUE;
		msg("Your armor glows faintly for a moment.");
		if (o_on(cur_armor,ISCURSED))
		    cur_armor->o_ac = armors[cur_armor->o_which].a_class;
		else
		    cur_armor->o_ac--;
		resoflg(cur_armor,ISCURSED);
	    }
	when S_HOLD:
	    /*
	     * Hold monster scroll.  Stop all monsters within 3 spaces
	     * from chasing after the hero.
	     */
	    {
		reg int x,y;
		reg struct linked_list *mon;

		for (x = hero.x - 3; x <= hero.x + 3; x++)
		    for (y = hero.y - 3; y <= hero.y + 3; y++)
			if (y > 0 && x > 0 && isalpha(mvwinch(mw, y, x)))
			    if ((mon = find_mons(y, x)) != NULL) {
				reg struct thing *th;

				th = THINGPTR(mon);
				th->t_flags &= ~ISRUN;
				th->t_flags |= ISHELD;
				th->t_flags |= ISSTUCK;
			    }
	    }
	when S_SLEEP:
	    /*
	     * Scroll which makes you fall asleep
	     */
	    s_know[S_SLEEP] = TRUE;
	    msg("You fall asleep.");
	    no_command += 4 + rnd(SLEEPTIME);
	when S_CREATE:
	    /*
	     * Create a monster
	     * First look in a circle around him, next try his room
	     * otherwise give up
	     */
	    {
		reg int x, y;
		reg bool appear = 0;
		struct coord mp;

		/*
		 * Search for an open place
		 */
		for (y = hero.y; y <= hero.y+1; y++)
		    for (x = hero.x; x <= hero.x+1; x++) {
			/*
			 * Don't put a monster in top of the player.
			 */
			if (y == hero.y && x == hero.x)
			    continue;
			/*
			 * Or anything else nasty
			 */
			if (step_ok(winat(y, x))) {
			    if (rnd(++appear) == 0) {
				mp.y = y;
				mp.x = x;
			    }
			}
		    }
		if (appear) {
		    s_know[S_CREATE] = TRUE;
		    titem = new_item(sizeof (struct thing));
		    new_monster(titem,randmonster(FALSE,FALSE),&mp,FALSE);
		}
		else
		  msg("You hear a faint cry of anguish in the distance.");
	    }
	when S_IDENT:
	    /*
	     * Identify, let the rogue figure something out
	     */
	    msg("This scroll is an identify scroll");
	    s_know[S_IDENT] = TRUE;
	    whatis(NULL);
	when S_MAP:
	    /*
	     * Scroll of magic mapping.
	     */
	    s_know[S_MAP] = TRUE;
	    if(rnd(100) < 10) {
		msg("Oh, now this scroll has a very detailed map on it.");
		displevl();
	    } else {
	    msg("Oh, now this scroll has a map on it.");
	    overwrite(stdscr, hw);
	    /*
	     * Take all the things we want to keep hidden out of the window
	     */
	    for (i = 0; i < LINES; i++)
		for (j = 0; j < COLS; j++) {
		    switch (nch = ch = mvwinch(hw, i, j)) {
			case SECRETDOOR:
			    mvaddch(i, j, nch = DOOR);
			case '-':
			case '|':
			case DOOR:
			case PASSAGE:
			case ' ':
			case STAIRS:
			    if (mvwinch(mw, i, j) != ' ') {
				reg struct thing *it;

				it = THINGPTR(find_mons(i,j));
				if (it->t_oldch == ' ')
				    it->t_oldch = nch;
			    }
			    break;
			default:
			    nch = ' ';
		    }
		    if (nch != ch)
			waddch(hw, nch);
		}
	    overlay(cw, hw);
	    overwrite(hw, cw);
	    }
	when S_GFIND:
	    /*
	     * Scroll of gold detection
	     */
	    {
		int gtotal = 0;
		struct room *rp;

		wclear(hw);
		for (rp = rooms; rp < &rooms[MAXROOMS]; rp++) {
		    gtotal += rp->r_goldval;
		    if (rp->r_goldval != 0 &&
			mvwinch(stdscr,rp->r_gold.y,rp->r_gold.x) == GOLD)
			mvwaddch(hw,rp->r_gold.y,rp->r_gold.x,GOLD);
		}
		if (gtotal) {
		    s_know[S_GFIND] = TRUE;
		    msg("You begin to feel greedy and you sense gold.");
		    overlay(hw,cw);
		}
		else
		    msg("You begin to feel a pull downward.");
	    }
	when S_TELEP:
	    /*
	     * Scroll of teleportation:
	     * Make him dissapear and reappear
	     */
	    {
		int rm;
		struct room *cur_room;

		cur_room = roomin(&hero);
		rm = teleport();
		if (cur_room != &rooms[rm])
		    s_know[S_TELEP] = TRUE;
	    }
	when S_ENCH:
	    if (cur_weapon == NULL || (cur_weapon != NULL &&
	       (o_on(cur_weapon,ISPROT) || cur_weapon->o_type != WEAPON)))
		msg("You feel a strange sense of loss.");
	    else {
		s_know[S_ENCH] = TRUE;
		if(o_on(cur_weapon,ISCURSED)) {
			resoflg(cur_weapon,ISCURSED);
			cur_weapon->o_hplus = rnd(2);
			cur_weapon->o_dplus = rnd(2);
		}
		else {		/* weapon was not cursed here */
		    if (rnd(100) < 50)
			cur_weapon->o_hplus += 1;
		    else
			cur_weapon->o_dplus += 1;
		}
		setoflg(cur_weapon, ISKNOW);
		msg("Your %s glows blue for a moment.",
		    weaps[cur_weapon->o_which].w_name);
	    }
	when S_SCARE:
	    /*
	     * A monster will refuse to step on a scare monster scroll
	     * if it is dropped.  Thus reading it is a mistake and produces
	     * laughter at the poor rogue's boo boo.
	     */
	    msg("You hear maniacal laughter in the distance.");
	when S_REMOVE:
	    if (cur_armor != NULL && o_off(cur_armor,ISPROT))
		resoflg(cur_armor,ISCURSED);
	    if (cur_weapon != NULL && o_off(cur_weapon,ISPROT))
		resoflg(cur_weapon,ISCURSED);
	    if (cur_ring[LEFT]!=NULL && o_off(cur_ring[LEFT],ISPROT))
		resoflg(cur_ring[LEFT],ISCURSED);
	    if (cur_ring[RIGHT]!=NULL && o_off(cur_ring[RIGHT],ISPROT))
		resoflg(cur_ring[RIGHT],ISCURSED);
	    msg("You feel as if somebody is watching over you.");
	    s_know[S_REMOVE] = TRUE;
	when S_AGGR:
	    /*
	     * This scroll aggravates all the monsters on the current
	     * level and sets them running towards the hero
	     */
	    aggravate();
	    msg("You hear a high pitched humming noise.");
	when S_NOP:
	    msg("This scroll seems to be blank.");
	when S_GENOCIDE:
	    msg("You have been granted the boon of genocide.");
	    genocide();
	    s_know[S_GENOCIDE] = TRUE;
	when S_DCURSE: {
	    struct linked_list *ll;
	    struct object *lb;
	    msg("Your pack shudders.");
	    for (ll = pack ; ll != NULL ; ll = next(ll)) {
		lb = OBJPTR(ll);
		if (o_off(lb,ISPROT))		/* not protected */
		    setoflg(lb,ISCURSED);
	    }
	}
	when S_DLEVEL: {
	    int much = rnd(9) - 4;
	    if (much != 0) {
		level += much;
		if (level < 1)
		    level = 1;
		else if (level > 99)
		    level = 95;
		mpos = 0;
		new_level(FALSE);		/* change levels */
		msg("You are whisked away to another region.");
		s_know[S_DLEVEL] = TRUE;
	    }
	}
	when S_PROTECT: {
	    struct linked_list *ll;
	    struct object *lb;
	    msg("You are granted the power of protection.");
	    if ((ll = get_item("protect",0)) != NULL) {
		lb = OBJPTR(ll);
		setoflg(lb,ISPROT);
	    }
	    s_know[S_PROTECT] = TRUE;
	}
	when S_ALLENCH: {
	    struct linked_list *ll;
	    struct object *lb;
	    int howmuch, ac;
	    msg("You are granted the power of enchantment.");
	    if ((ll = get_item("enchant",0)) != NULL) {
		lb = OBJPTR(ll);
		resoflg(lb,ISCURSED);
		resoflg(lb,ISPROT);
		howmuch = rnd(3) + 2;
		switch(lb->o_type) {
		    case RING:
			if (lb->o_ac < 0)
			    lb->o_ac = 0;
			lb->o_ac += howmuch;
		    when ARMOR:
			ac = armors[lb->o_which].a_class;
			if (lb->o_ac > ac)
			    lb->o_ac = ac;
			lb->o_ac -= howmuch;
		    when STICK:
			lb->o_charges += howmuch + 10;
		    when WEAPON:
			if (lb->o_dplus < 0)
			    lb->o_dplus = 0;
			if (lb->o_hplus < 0)
			    lb->o_hplus = 0;
			lb->o_hplus += howmuch;
			lb->o_dplus += howmuch;
		    otherwise:
			msg("You are injured as the scroll flashes & bursts into flames !!!");
			chg_hpt(-roll(6,6),FALSE,6);
		}
	    }
	    s_know[S_ALLENCH] = TRUE;
	}
	when S_BLESS: {
	    struct linked_list *ll;
	    struct object *lb;
	    msg("Your pack glistens brightly.");
	    for (ll = pack ; ll != NULL ; ll = next(ll)) {
		lb = OBJPTR(ll);
		resoflg(lb,ISCURSED);
	    }
	}
	when S_MAKEIT:
	    msg("You have been endowed with the power of creation.");
	    s_know[S_MAKEIT] = TRUE;
	    create_obj(TRUE);
	when S_BAN: {
	    int howlow = 25 + rnd(26);
	    if (level < howlow) {
		level = howlow;
		new_level(FALSE);
		mpos = 0;
		msg("You are banished to the lower regions.");
		s_know[S_BAN] = TRUE;
	    }
	}
	when S_CWAND: {
	    struct linked_list *ll;
	    struct object *lb;
	    bool wands = FALSE;
	    for (ll = pack ; ll != NULL ; ll = next(ll)) {
		lb = OBJPTR(ll);
		if (lb->o_type == STICK) {
		    lb->o_charges += rnd(11) + 5;
		    wands = TRUE;
		}
	    }
	    if (wands) {
		msg("Your sticks gleam.");
		s_know[wh] = TRUE;
	    }
	}	    
	otherwise:
	    msg("What a puzzling scroll!");
	    return;
    }
    look(TRUE);
    nochange = FALSE;
    if (s_know[wh] && s_guess[wh]) {
	FREE(s_guess[wh]);
	s_guess[wh] = NULL;
    }
    else if(!s_know[wh] && s_guess[wh]==NULL) {
	buf[0] = 0;
	msg("Call it: ");
	if (get_str(buf, cw) == NORM) {
	    s_guess[wh] = new(strlen(buf) + 1);
	    strcpy(s_guess[wh], buf);
	}
    }
}
