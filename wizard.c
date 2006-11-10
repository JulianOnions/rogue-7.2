/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances)
 *
 * @(#)wizard.c	7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include <pwd.h>
#include "mach_dep.h"
#include "rogue_ext.h"


/*
 * whatis:
 *	What a certain object is
 */

extern int setoflg (struct object *what, int bit);
extern void msg (const char *fmt, ...);
extern int readchar (void);
int makemons (void);
extern int isatrap (char ch);
int getbless (void);
extern int init_weapon (struct object *weap, char type);
extern int init_ring (struct object *what, NCURSES_BOOL fromwiz);
extern int fix_stick (struct object *cur);
extern int add_pack (struct linked_list *item, NCURSES_BOOL silent);
extern int discard (register struct linked_list *item);
extern int step_ok (char ch);
extern int winat (int y, int x);
extern int new_monster (struct linked_list *item, char type, struct coord *cp, NCURSES_BOOL treas);
extern int look (NCURSES_BOOL wakeup);
extern int rnd_room (void);
extern int rnd_pos (struct room *rp, struct coord *cp);
extern int light (struct coord *cp);
extern int mon_index (char whichmon);

whatis(struct linked_list *what)
{
    reg struct object *obj;
    reg struct linked_list *item;
    reg int wh;

    if (what == NULL) {		/* we need to ask */
	if ((item = get_item("identify", 0)) == NULL)
	    return;
    }
    else			/* no need to ask */
	item = what;
    obj = OBJPTR(item);
    setoflg(obj, ISKNOW);
    wh = obj->o_which;
    switch (obj->o_type) {
	case SCROLL:
	    s_know[wh] = TRUE;
	    if (s_guess[wh]) {
		FREE(s_guess[wh]);
		s_guess[wh] = NULL;
	    }
        when POTION:
	    p_know[wh] = TRUE;
	    if (p_guess[wh]) {
		FREE(p_guess[wh]);
		p_guess[wh] = NULL;
	    }
	when STICK:
	    ws_know[wh] = TRUE;
	    if (ws_guess[wh]) {
		FREE(ws_guess[wh]);
		ws_guess[wh] = NULL;
	    }
        when RING:
	    r_know[wh] = TRUE;
	    if (r_guess[wh]) {
		FREE(r_guess[wh]);
		r_guess[wh] = NULL;
	    }
    }
    if (what == NULL)
	msg(inv_name(obj, FALSE));
}


/*
 * create_obj:
 *	Create any object for wizard or scroll (almost)
 */
create_obj(NCURSES_BOOL fscr)
{
    reg struct linked_list *item;
    reg struct object *obj;
    reg int wh;
    reg char ch, newitem, newtype, whc, msz, *pt;
    WINDOW *thiswin;

    thiswin = cw;
    msg("Type of item to create? (* for a list) ");
    if((newitem = readchar()) == ESCAPE) {
	msg("");
	return;
    }
    else if (newitem == '*') {
	bool nogood;
	thiswin = hw;
	wclear(hw);
	wprintw(hw,"Item\tKey\n\n");
	wprintw(hw,"%s\t %c\n%s\t %c\n",things[TYP_RING].mi_name,RING,
		things[TYP_STICK].mi_name,STICK);
	wprintw(hw,"%s\t %c\n%s\t %c\n",things[TYP_POTION].mi_name,POTION,
		things[TYP_SCROLL].mi_name,SCROLL);
	wprintw(hw,"%s\t %c\n%s\t %c\n",things[TYP_ARMOR].mi_name,ARMOR,
		things[TYP_WEAPON].mi_name,WEAPON);
	wprintw(hw,"%s\t %c\n",things[TYP_FOOD].mi_name,FOOD);
	if (wizard) {
	    wprintw(hw,"%s\t %c\n",things[TYP_AMULET].mi_name,AMULET);
	    waddstr(hw,"monster\t m");
	}
	wprintw(hw,"\n\nWhat do you want to create? ");
	draw(hw);
	do {
	    ch = readchar();
	    if (ch == ESCAPE) {
		restscr(cw);
		return;
	    }
	    switch (ch) {
		case RING:	case STICK:	case POTION:
		case SCROLL:	case ARMOR:	case WEAPON:
		case FOOD:
		    nogood = FALSE;
		    break;
		case 'm':
		    if (wizard) {
			nogood = FALSE;
			break;
		    }
		case AMULET:
		    if (wizard) {
			nogood = FALSE;
			break;
		    }
		default:
		    nogood = TRUE;
	    }
	} while (nogood);
	newitem = ch;
    }
    pt = "those";
    msz = 0;
    if(newitem == 'M') {
	if (thiswin == hw)
	    restscr(cw);
	makemons();		/* make monster and be done with it */
	return;
    }
    if(newitem == GOLD)
	pt = "gold";
    else if(isatrap(newitem))
	pt = "traps";
    switch(newitem) {
	case POTION:	whc = TYP_POTION;	msz = MAXPOTIONS;
	when SCROLL:	whc = TYP_SCROLL;	msz = MAXSCROLLS;
	when FOOD:	whc = TYP_FOOD;		msz = MAXFOODS;
	when WEAPON:	whc = TYP_WEAPON;	msz = MAXWEAPONS;
	when ARMOR:	whc = TYP_ARMOR;	msz = MAXARMORS;
	when RING:	whc = TYP_RING;		msz = MAXRINGS;
	when STICK:	whc = TYP_STICK;	msz = MAXSTICKS;
	when AMULET:	whc = TYP_AMULET;	msz = MAXAMULETS;
	otherwise:
	    if (thiswin == hw)
		restscr(cw);
	    mpos = 0;
	    msg("Even wizards can't create %s !!",pt);
	    return;
    }
    if(msz == 1) {		/* if only one type of item */
	ch = 'a';
    }
    else {			/* more than one */
	mpos = 0;
	if (fscr)		/* normal players get all items */
	    ch = '*';
	else {
	    sprintf(prbuf,"Which %s do you want? (* for a list)",
		things[whc].mi_name);
	    if (thiswin == hw) {
		mvwaddstr(hw,LINES-1,0,prbuf);	/* if help screen */
		draw(hw);
	    }
	    else {			/* just do msg if normal screen */
		msg(prbuf);
	    }
	    if ((ch = readchar()) == ESCAPE) {
		if (thiswin == hw)
		    restscr(cw);		/* restore normal screen */
		msg("");
		return;
	    }
	}
	if (ch == '*') {
	    struct magic_item *wmi;
	    char wmn;
	    int ii;

	    wmi = NULL;
	    wmn = 0;
	    switch(newitem) {
		case POTION:	wmi = &p_magic[0];
		when SCROLL:	wmi = &s_magic[0];
		when RING:	wmi = &r_magic[0];
		when STICK:	wmi = &ws_magic[0];
		when WEAPON:	wmn = 1;
		when ARMOR:	wmn = 2;
	    }
	    wclear(hw);
	    thiswin = hw;
	    if (wmi != NULL) {
		for (ii = 0 ; ii < msz ; ii++) {
		    mvwaddch(hw,ii % 13,ii > 12 ? COLS/2 : 0, ii + 'a');
		    waddstr(hw,") ");
		    waddstr(hw,wmi->mi_name);
		    wmi++;
		}
	    }
	    else if (wmn != 0) {
		for(ii = 0 ; ii < msz ; ii++) {
		    mvwaddch(hw,ii % 13,ii > 12 ? COLS/2 : 0, ii + 'a');
		    waddstr(hw,") ");
		    if(wmn == 1)
			waddstr(hw,weaps[ii].w_name);
		    else
			waddstr(hw,armors[ii].a_name);
		}
	    }
	    sprintf(prbuf,"Which %s? ",things[whc].mi_name);
	    mvwaddstr(hw,LINES - 1, 0, prbuf);
	    draw(hw);
	    do {
		ch = readchar();
		if (ch == ESCAPE) {
		    restscr(cw);
		    msg("");
		    return;
		}
	    } until (isalpha(ch));
	}
    }
    if (thiswin == hw)			/* restore screen if need be */
	restscr(cw);
    newtype = lower(ch) - 'a';
    if(newtype < 0 || newtype >= msz) {		/* if an illegal value */
	mpos = 0;
	msg("There is no such %s",things[whc].mi_name);
	return;
    }
    item = new_item(sizeof *obj);	/* get some memory */
    obj = OBJPTR(item);
    obj->o_type = newitem;		/* store the new items */
    obj->o_which = newtype;
    obj->o_group = 0;
    obj->o_count = 1;
    obj->o_flags = 0;
    if (fscr)				/* let normal players know */
	setoflg(obj,ISKNOW);
    obj->o_dplus = obj->o_hplus = 0;
    obj->o_weight = 0;
    wh = obj->o_which;
    mpos = 0;
    switch (obj->o_type) {
	case WEAPON:
	case ARMOR:
	    if (fscr)			/* users get +3 to -3 */
		whc = rnd(7) - 3;
	    else			/* wizard gets to choose */
		whc = getbless();
	    if (whc < 0)
		setoflg(obj,ISCURSED);
	    if (obj->o_type == WEAPON) {
		init_weapon(obj, wh);
		obj->o_hplus += whc;
		obj->o_dplus += whc;
	    }
	    else {				/* armor here */
		obj->o_weight = armors[wh].a_wght;
		obj->o_ac = armors[wh].a_class - whc;
	    }
	when RING:
	    if (fscr) {
		init_ring(obj,FALSE);		/* normal players */
		r_know[wh] = TRUE;
	    }
	    else {
		init_ring(obj,TRUE);		/* wizards only */
	    }
	when STICK:
	    if (fscr)
		ws_know[wh] = TRUE;	/* let mortals know */
	    fix_stick(obj);
	when AMULET:
	    obj->o_weight = things[TYP_AMULET].mi_wght;
	when SCROLL:
	    obj->o_weight = things[TYP_SCROLL].mi_wght;
	    if (fscr)
		s_know[wh] = TRUE;
	when POTION:
	    obj->o_weight = things[TYP_POTION].mi_wght;
	    if (fscr)
		p_know[wh] = TRUE;
    }
    mpos = 0;
    wh = add_pack(item, FALSE);
    if (wh == FALSE)			/* won't fit in pack */
	discard(item);
}


/*
 * getbless:
 *	Get a blessing for a wizards object
 */
getbless(void)
{
	reg char bless;

	msg("Blessing? (+,-,n)");
	bless = readchar();
	if (bless == '+')
		return (rnd(3) + 1);
	else if (bless == '-')
		return (-rnd(3) - 1);
	else
		return (0);
}

/*
 * makemons:
 *	Make a monster for the wizard
 */
makemons(void)
{
	reg struct thing *it;
	reg struct linked_list *item;
	reg int ch, x, y;
	struct coord mp;
	bool oktomake;

	do {
	    mpos = 0;
	    msg("Which monster? ");
	    ch = readchar();
	    if (ch == ESCAPE) {
		msg("");
		return;
	    }
	} until(isalpha(ch));
	oktomake = FALSE;
	for (x = hero.x - 1 ; x <= hero.x + 1 ; x++) {	
	    for (y = hero.y - 1 ; y <= hero.y + 1 ; y++) {
		if (x != hero.x || y != hero.y) {
		    if (step_ok(winat(y, x))) {
			mp.x = x;
			mp.y = y;
			oktomake = TRUE;
			break;
		    }
		}
	    }
	}
	if (oktomake) {
	    item = new_item(sizeof(struct thing));
	    new_monster(item, ch, &mp, FALSE);
	    look(FALSE);
	}
}

/*
 * telport:
 *	Bamf the hero someplace else
 */
teleport(void)
{
    reg int rm;
    struct coord c;

    c = hero;
    mvwaddch(cw, hero.y, hero.x, mvwinch(stdscr, hero.y, hero.x));
    do {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &hero);
    } until(winat(hero.y, hero.x) == FLOOR);
    light(&c);
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    /*
     * turn off ISHELD in case teleportation was done while fighting
     * a Fungi or Bone Devil.
     */
    if (pl_on(ISHELD)) {
	player.t_flags &= ~ISHELD;
	fung_hit = 0;
	monsters[mon_index('F')].m_stats.s_dmg = "000d0";
    }
    count = 0;
    no_move = 0;
    running = FALSE;
    flushout();			/* flush typeahead */
    nochange = FALSE;
    return rm;
}

/*
 * passwd:
 *	See if user knows password
 */

passwd(void)
{
    reg char *sp, c;
    char buf[LINLEN], *crypt();

    msg("Wizard's Password:");
    mpos = 0;
    sp = buf;
    while ((c = getchar()) != '\n' && c != '\r' && c != ESCAPE)
	if (c == killchar())
	    sp = buf;
	else if (c == erasechar() && sp > buf)
	    sp--;
	else
	    *sp++ = c;
    if (sp == buf)
	return FALSE;
    *sp = '\0';
    return (strcmp(PASSWD, crypt(buf, SALT)) == 0);
}
