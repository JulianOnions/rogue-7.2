/*
 * Read and execute the user commands
 *
 * @(#)command.c	7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include <signal.h>
#include "rogue.h"
#include "mach_dep.h"
#include "rogue_ext.h"


/*
 * command:
 *	Process the user commands
 */


command(void)
{
    reg char ch;
    reg int ntimes = 1;		/* Number of player moves */
    static char countch, direction, newcount = FALSE;
    extern int prntstats(char prtype);

    if (pl_on(ISHASTE))
	ntimes++;
    /*
     * Let the daemons start up
     */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    while (ntimes-- > 0) {
	look(TRUE);
	if (!running)
	    door_stop = FALSE;
	lastscore = purse;
	wmove(cw, hero.y, hero.x);
	if (!(running || count))
	    draw(cw);			/* Draw screen */
	take = 0;
	after = TRUE;
	/*
	 * Read command or continue run
	 */
	if (wizard)
	    waswizard = TRUE;
	if (no_command <= 0) {
	    no_command = 0;
	    if (running)
		ch = runch;
	    else if (count)
		ch = countch;
	    else {
		ch = readchar();
		if(mpos != 0 && !running) /* Erase message if its there */
		    msg("");
	    }
	}
	else
	    ch = ' ';
	if(no_command > 0) {
	    if (--no_command <= 0)
		msg("You can move again.");	
	}
	else {
	    /*
	     * check for prefixes
	     */
	    if(isdigit(ch)) {
		count = 0;
		newcount = TRUE;
		while (isdigit(ch)) {
		    count = count * 10 + (ch - '0');
		    ch = readchar();
		}
		countch = ch;
		/*
		 * turn off count for commands which don't make sense
		 * to repeat
		 */
		switch (ch) {
		    case 'h': case 'j': case 'k': case 'l':
		    case 'y': case 'u': case 'b': case 'n':
		    case 'H': case 'J': case 'K': case 'L':
		    case 'Y': case 'U': case 'B': case 'N':
		    case 'q': case 'r': case 's': case 'f':
		    case 't': case 'C': case 'I': case ' ':
		    case 'z': case 'p':
			break;
		    default:
			count = 0;
		}
	    }
	    switch (ch) {
		case 'f':
		case 'g':
		    if (pl_off(ISBLIND)) {
			door_stop = TRUE;
			firstmove = TRUE;
		    }
		    if (count && !newcount)
			ch = direction;
		    else
			ch = readchar();
		    switch (ch) {
			case 'h': case 'j': case 'k': case 'l':
			case 'y': case 'u': case 'b': case 'n':
			    ch = toupper(ch);
		    }
		    direction = ch;
	    }
	    newcount = FALSE;
	    /*
	     * execute a command
	     */
	    if (count && !running)
		count--;
	    switch (ch) {
		case '!' : shell(); after = FALSE;
		when 'h' : do_move(0, -1);
		when 'j' : do_move(1, 0);
		when 'k' : do_move(-1, 0);
		when 'l' : do_move(0, 1);
		when 'y' : do_move(-1, -1);
		when 'u' : do_move(-1, 1);
		when 'b' : do_move(1, -1);
		when 'n' : do_move(1, 1);
		when 'H' : do_run('h');
		when 'J' : do_run('j');
		when 'K' : do_run('k');
		when 'L' : do_run('l');
		when 'Y' : do_run('y');
		when 'U' : do_run('u');
		when 'B' : do_run('b');
		when 'N' : do_run('n');
		when 't':
		    if (!get_dir())
			after = FALSE;
		    else
			missile(delta.y, delta.x);
		when 'Q' : after = FALSE; quit(0);
		when 'i' : after = FALSE; inventory(pack, 0);
		when 'I' : after = FALSE; picky_inven();
		when 'd' : drop(NULL);
		when 'q' : quaff();
		when 'r' : read_scroll();
		when 'e' : eat();
		when 'w' : wield();
		when 'W' : wear();
		when 'T' : take_off();
		when 'P' : ring_on();
		when 'R' : ring_off();
		when 'o' : option();
		when 'c' : call();
		when '>' : after = FALSE; d_level();
		when '<' : after = FALSE; u_level();
		when '?' : after = FALSE; help();
		when '/' : after = FALSE; identify();
		when 's' : search();
		when 'z' : do_zap(FALSE);
		when 'p':
		    if (get_dir())
			do_zap(TRUE);
		    else
			after = FALSE;
		when 'v': msg("Super Rogue version %s.",release);
		when CTRL('L') : after = FALSE; restscr(cw);
		when CTRL('R') : after = FALSE; msg(huh);
		when 'a': after = FALSE; prntstats(1);
		when 'x': after = FALSE; prntstats(2);
		when CTRL('B') : after = FALSE; prhwfile(BUGFILE);
		when '@' : if (author())
			      msg("Hero @ %d,%d : Stairs @ %d,%d",
				hero.y,hero.x,stairs.y,stairs.x);
		when 'S' : 
		    after = FALSE;
		    if (save_game()) {
			wclear(cw);
			draw(cw);
			endwin();
			byebye(0);
		    }
		when ' ' : ;			/* Rest command */
		when '=' :
		    if (author()) {
			activity();
			after = FALSE;
		    }
		when CTRL('P') :
		    after = FALSE;
		    if (wizard) {
			wizard = FALSE;
			msg("Not wizard any more");
		    }
		    else {
			if (wizard = passwd()) {
			    msg("Hello Mister Wizard!!!!!");
			    waswizard = TRUE;
			}
			else
			    msg("Sorry");
		    }
		when ESCAPE :	/* Escape */
		    door_stop = FALSE;
		    count = 0;
		    after = FALSE;
		when '#':
		    if (tradelev)		/* buy something */
			buy_it();
		    after = FALSE;
		when '$':
		    if (tradelev)		/* price something */
			price_it();
		    after = FALSE;
		when '%':
		    if (tradelev)		/* sell something */
			sell_it();
		    after = FALSE;
		otherwise :
		    after = FALSE;
		    if (wizard) switch (ch) {
			case 'C' :	create_obj(FALSE);
			when CTRL('I') :	inventory(lvl_obj, 1);
			when CTRL('W') :	whatis(NULL);
			when CTRL('D') :	level++; new_level(FALSE);
			when CTRL('U') :	level--; new_level(FALSE);
			when CTRL('F') :	displevl();
			when CTRL('X') :	dispmons();
			when CTRL('T') :	teleport();
			when CTRL('E') :	msg("food left: %d", food_left);
			when CTRL('A') :	msg("%d things in your pack",inpack);
			when CTRL('G') :	add_pass();
			when 'M' : {
			    int tlev;
			    prbuf[0] = 0;
			    msg("Which level? ");
			    if(get_str(prbuf,cw) == NORM) {
				tlev = atoi(prbuf);
				if(tlev < 1) {
				    mpos = 0;
				    msg("Illegal level.");
				}
				else {
				    if (tlev > 99) {
					tradelev = TRUE;
					level = tlev % 100 + 1;
				    } else {
					tradelev = FALSE;
					level = tlev;
				    }
				    new_level(tradelev);
				}
			    }
			}
			when CTRL('N') : {
			    struct linked_list *item;

			    item = get_item("charge", STICK);
			    if (item != NULL) {
				(OBJPTR(item))->o_charges = 10000;
				msg("");
			    }
			}
			when CTRL('H') : {
			    reg int i;
			    reg struct linked_list *item;
			    reg struct object *obj;

			    for (i = 0; i < 9; i++)
				raise_level();
			    /*
			     * Give the rogue a very good sword
			     */
			    item = new_item(sizeof *obj);
			    obj = OBJPTR(item);
			    obj->o_type = WEAPON;
			    obj->o_which = TWOSWORD;
			    init_weapon(obj, SWORD);
			    obj->o_hplus = 3;
			    obj->o_dplus = 3;
			    setoflg(obj,ISKNOW);
			    add_pack(item, TRUE);
			    cur_weapon = obj;
			    /*
			     * And his suit of armor
			     */
			    item = new_item(sizeof *obj);
			    obj = OBJPTR(item);
			    obj->o_type = ARMOR;
			    obj->o_which = PLATEARMOR;
			    obj->o_weight = armors[PLATEARMOR].a_wght;
			    obj->o_ac = -8;
			    obj->o_count = 1;
			    setoflg(obj,ISKNOW);
			    cur_armor = obj;
			    add_pack(item, TRUE);
			    nochange = FALSE;
			}
			otherwise :
			    msg("Illegal command '%s'.", unctrl(ch));
			    count = 0;
		    }
		    else {
			msg("Illegal command '%s'.", unctrl(ch));
			count = 0;
		    }
	    }
	    /*
	     * turn off flags if no longer needed
	     */
	    if (!running)
		door_stop = FALSE;
	}
	/*
	 * If he ran into something to take, let the
	 * hero pick it up if not in a trading post.
	 */
	if (take != 0 && !tradelev)
	    pick_up(take);
	if (!running)
	    door_stop = FALSE;
    }
    /*
     * Kick off the rest if the daemons and fuses
     */

    if (after) {
	int blessing, tchances, j;

	look(FALSE);
	do_daemons(AFTER);
	do_fuses(AFTER);
	if(pl_on(ISSLOW))
		waste_time();
	for(j = LEFT ; j <= RIGHT ; j++) {
		if(cur_ring[j] == NULL)
			continue;		/* no ring on this hand */
		if(cur_ring[j]->o_which == R_SEARCH)
			search();
		else if(cur_ring[j]->o_which == R_TELEPORT) {
			blessing = cur_ring[j]->o_ac;
			if(blessing < 0)
				tchances = (-blessing * 3) + 1;
			else
				tchances = (blessing * 3 / 2) + 1;
			if(rnd(100) < tchances)
				teleport();
		}
	}
    }
}


/*
 * quit:
 *	Have player make certain, then exit.
 */
void quit(int junk)
{
    /*
     * Reset the signal in case we got here via an interrupt
     */
    if (signal(SIGINT, quit) != quit)
	mpos = 0;
    msg("Really quit?");
    draw(cw);
    if (lower(readchar()) == 'y') {
	clear();
	move(LINES-1, 0);
	draw(stdscr);
	score(purse, 1, 0);
	byebye(0);
    }
    else {
	signal(SIGINT, quit);
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	draw(cw);
	mpos = 0;
	count = 0;
	nochange = FALSE;
    }
}

/*
 * search:
 *	Player gropes about him to find hidden things.
 */

search(void)
{
    reg int x, y;
    reg char ch;

    /*
     * Look all around the hero, if there is something hidden there,
     * give him a chance to find it.  If its found, display it.
     */
    if (pl_on(ISBLIND))
	return;
    for (x = hero.x - 1; x <= hero.x + 1; x++) {
	for (y = hero.y - 1; y <= hero.y + 1; y++) {
	    ch = winat(y, x);
	    if(isatrap(ch)) {		/* see if its a trap */
		    reg struct trap *tp;

		    if((tp = trap_at(y, x)) == NULL)
			break;
		    if(tp->tr_flags & ISFOUND)
			break;		/* no message if its seen */
		    if (mvwinch(cw, y, x) == ch)
			break;
		    if (rnd(100) > (pstats.s_lvl * 9 + herowis() * 5))
			 break;
		    tp->tr_flags |= ISFOUND;
		    mvwaddch(cw, y, x, (tp->tr_type));
		    count = 0;
		    running = FALSE;
		    msg(tr_name(tp->tr_type));
	    }
	    else if(ch == SECRETDOOR) {
		if (rnd(100) < (pstats.s_lvl * 4 + herowis() * 5)) {
		    mvaddch(y, x, DOOR);
		    count = 0;
		    }
		}

	    }
	}
}

/*
 * help:
 *	Give single character help, or the whole mess if he wants it
 */
help(void)
{
    extern struct h_list helpstr[];
    reg struct h_list *strp;
    reg char helpch;
    reg int cnt;

    strp = &helpstr[0];
    msg("Character you want help for (* for all): ");
    helpch = readchar();
    mpos = 0;
    /*
     * If its not a *, print the right help string
     * or an error if he typed a funny character.
     */
    if (helpch != '*') {
	wmove(cw, 0, 0);
	while (strp->h_ch) {
	    if (strp->h_ch == helpch) {
		msg("%s%s", unctrl(strp->h_ch), strp->h_desc);
		break;
	    }
	    strp++;
	}
	if (strp->h_ch != helpch)
	    msg("Unknown character '%s'", unctrl(helpch));
	return;
    }
    /*
     * Here we print help for everything.
     * Then wait before we return to command mode
     */
    wclear(hw);
    cnt = 0;
    while (strp->h_ch) {
	mvwaddstr(hw, cnt % 23, cnt > 22 ? 40 : 0, unctrl(strp->h_ch));
	waddstr(hw, strp->h_desc);
	cnt++;
	strp++;
    }
    wmove(hw, LINES-1, 0);
    wprintw(hw,spacemsg);
    draw(hw);
    wait_for(' ');
    wclear(hw);
    draw(hw);
    wmove(cw, 0, 0);
    wclrtoeol(cw);
    touchwin(cw);
    nochange = FALSE;
}


/*
 * identify:
 *	Tell the player what a certain thing is.
 */
identify(void)
{
    reg char ch, *str;

    msg("What do you want identified? ");
    ch = readchar();
    mpos = 0;
    if (ch == ESCAPE) {
	msg("");
	return;
    }
    if (isalpha(ch))
	str = monsters[mon_index(ch)].m_name;
    else switch(ch) {
	case '|':
	case '-':	str = "wall of a room";
	when GOLD:	str = "gold";
	when STAIRS:	str = "passage leading down";
	when DOOR:	str = "door";
	when FLOOR:	str = "room floor";
	when PLAYER:	str = "you";
	when PASSAGE:	str = "passage";
	when POST:	str = "trading post";
	when TRAPDOOR:	str = "trapdoor";
	when ARROWTRAP:	str = "arrow trap";
	when SLEEPTRAP:	str = "sleeping gas trap";
	when BEARTRAP:	str = "bear trap";
	when TELTRAP:	str = "teleport trap";
	when DARTTRAP:	str = "dart trap";
	when POOL:	str = "magic pool";
	when POTION:	str = "potion";
	when SCROLL:	str = "scroll";
	when FOOD:	str = "food";
	when WEAPON:	str = "weapon";
	when ' ' :	str = "solid rock";
	when ARMOR:	str = "armor";
	when AMULET:	str = "The Amulet of Yendor";
	when RING:	str = "ring";
	when STICK:	str = "wand or staff";
	otherwise:	str = "unknown character";
    }
    msg("'%s' : %s", unctrl(ch), str);
}

/*
 * d_level:
 *	He wants to go down a level
 */

d_level(void)
{
    if (winat(hero.y, hero.x) != STAIRS)
	msg("I see no way down.");
    else {
	if (pl_on(ISHELD)) {
		msg("You are being held.  You can't go down");
		return;
	}
	level++;
	new_level(FALSE);
    }
}

/*
 * u_level:
 *	He wants to go up a level
 */

u_level(void)
{
    if (winat(hero.y, hero.x) == STAIRS)  {
	if (pl_on(ISHELD)) {
	    msg("You are being held.");
	    return;
	}
	else {				/* player not held here */
	    if (amulet) {
		level--;
		if (level == 0)
		    total_winner();
		new_level(FALSE);
		msg("You feel a wrenching sensation in your gut.");
		return;
	    }
	}
    }
    msg("I see no way up.");
}


/*
 * Let him escape for a while
 */
shell(void)
{
    reg int pid;
    reg char *sh;
    int ret_status;

    /*
     * Set the terminal back to original mode
     */
    sh = getenv("SHELL");
    wclear(hw);
    wmove(hw, LINES-1, 0);
    draw(hw);
    endwin();
    in_shell = TRUE;
    fflush(stdout);
    /*
     * Fork and do a shell
     */
    while((pid = fork()) < 0)
	sleep(1);
    if (pid == 0) {
	setuid(getuid());	/* Set back to original user */
	setgid(getgid());
	execl(sh == NULL ? "/bin/sh" : sh, "shell", "-i", 0);
	perror("No shelly");
	byebye(-1);
    }
    else {
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while (wait(&ret_status) != pid)
	    continue;
	signal(SIGINT, quit);
	signal(SIGQUIT, endit);
	printf("\n[Press return to continue]");
	noecho();
	cbreak();
	in_shell = FALSE;
	wait_for('\n');
	restscr(cw);
    }
}


/*
 * call:
 *	Allow a user to call a potion, scroll, or ring something
 */
call(void)
{
    reg struct object *obj;
    reg struct linked_list *item;
    reg char **guess, *elsewise;
    int wh;

    if ((item = get_item("call", CALLABLE)) == NULL)
	return;
    obj = OBJPTR(item);
    wh = obj->o_which;
    switch (obj->o_type) {
	case RING:
	    guess = r_guess;
	    elsewise = (r_guess[wh] != NULL ? r_guess[wh] : r_stones[wh]);
	when POTION:
	    guess = p_guess;
	    elsewise = (p_guess[wh] != NULL ? p_guess[wh] : p_colors[wh]);
	when SCROLL:
	    guess = s_guess;
	    elsewise = (s_guess[wh] != NULL ? s_guess[wh] : s_names[wh]);
	when STICK:
	    guess = ws_guess;
	    elsewise = (ws_guess[wh] != NULL ? ws_guess[wh] : ws_made[wh]);
	otherwise:
	    sprintf(prbuf,"You can't call %ss anything",typ_name(obj));
	    msg(prbuf);
	    return;
    }
    msg("Was called \"%s\"", elsewise);
    msg("Call it: ");
    if (guess[wh] != NULL)
	FREE(guess[wh]);
    strcpy(prbuf, elsewise);
    if (get_str(prbuf, cw) == NORM) {
	guess[wh] = new(strlen(prbuf) + 1);
	strcpy(guess[wh], prbuf);
    }
}
