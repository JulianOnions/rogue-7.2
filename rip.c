/*
 * File for the fun ends, death or a total win
 *
 * @(#)rip.c	3.20 (Bell Labs) 2/17/82
 */

#ifdef BSD4_2
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>
#include "mach_dep.h"
#include "rogue.h"
#include "rogue_ext.h"


static char *rip[] = {
"                       __________",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |",
"                  |                  |",
"                  |   killed by a    |",
"                  |                  |",
"                  |       1982       |",
"                 *|     *  *  *      | *",
"         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
};
#define RIP_LINES (sizeof rip / (sizeof (char *)))

char	*killname(char monst);

/*
 * death:
 *	Do something really fun when he dies
 */


void score (int amount, int aflag, char monst);
extern int idenpack (void);
extern int author (void);
extern int encread (char *start, unsigned int size, int inf);
extern int encwrite (char *start, unsigned int size, FILE *outf);
extern int get_worth (struct object *obj);
extern int mon_index (char whichmon);

death(char monst)
{
    reg char dp, *killer;
    reg struct tm *lt;
    time_t date;
    char buf[LINLEN];
    struct tm *localtime(const time_t *);

    time(&date);
    lt = localtime(&date);
    clear();
    move(8, 0);
    for (dp = 0; dp < RIP_LINES; dp++)
	printw("%s\n", rip[dp]);
    mvaddstr(14,28 - ((strlen(whoami) + 1) / 2),whoami);
    purse -= purse/10;
    sprintf(buf, "%d Au", purse);
    mvaddstr(15, 28 - ((strlen(buf) + 1) / 2), buf);
    killer = killname(monst);
    mvaddstr(17, 28 - ((strlen(killer) + 1) / 2), killer);
    mvaddstr(16, 33, vowelstr(killer));
    sprintf(prbuf, "%2d", lt->tm_year);
    mvaddstr(18, 28,prbuf);
    move(LINES-1, 0);
    draw(stdscr);
    score(purse, 0, monst);
    endwin();
    byebye(0);
}

/*
 * score -- figure score and post it.
 */

/* VARARGS2 */
void score(int amount, int aflag, char monst)
{
    static struct sc_ent {
	int sc_score;		/* gold */
	char sc_name[LINLEN];	/* players name */
	int sc_flags;		/* reason for being here */
	int sc_level;		/* dungeon level */
	int sc_uid;		/* user ID */
	char sc_monster;	/* killer */
	int sc_explvl;		/* experience level */
	long int sc_exppts;	/* experience points */
	char sc_date[30];	/* time this score was posted */
    } top_ten[10];
    reg struct sc_ent *scp;
    reg int i, fd, prflags = 0;
    reg struct sc_ent *sc2;
    reg FILE *outf;
    reg char *killer;
    static char *reason[] = {"Killed","Chickened out","A total winner"};

    struct object *obj;
    char *packend, ch;

    signal(SIGINT, byebye);
    if(aflag == 0 || aflag == 1) {
	if(aflag) packend = "when you chickened out";
	     else packend = "at your untimely demise";
	printf("[Hit return to continue]");
	fflush(stdout);
	getch();
	wclear(hw);
	wprintw(hw,"Contents of your pack %s:\n\n",packend);
	idenpack();		/* identify all the pack */
	scrollok(hw, TRUE);
	for(ch='a' ; pack != NULL ; pack = next(pack) , ch++) {
		obj = OBJPTR(pack);
		wprintw(hw,"%c) %s\n",ch,inv_name(obj,FALSE));
	}
	wprintw(hw,"---   %d gold pieces   ---\n",purse);
	draw(hw);
    }
    if (aflag != -1) {
	endwin();
	resetty();
    }
    /*
     * Open file and read list
     */

    if ((fd = open(SCOREFILE, 2)) < 0)
	return;
    outf = fdopen(fd, "w");

    for (scp = top_ten; scp < &top_ten[10]; scp++) {
	scp->sc_score = 0;
	for (i = 0; i < 80; i++)
	    scp->sc_name[i] = rnd(255);
	scp->sc_flags = rand();
	scp->sc_level = rand();
	scp->sc_monster = rand();
	scp->sc_uid = rand();
	scp->sc_date[0] = 0;
    }


    if (aflag != -1) {
	mvwaddstr(hw,LINES-1,0,"[Press return to continue]");
	draw(hw);
	fflush(stdout);
	getch();
	wclear(cw);
	draw(cw);
    }
    if (author() || wizard)
	if (strcmp(prbuf, "names") == 0)
	    prflags = 1;
	else if (strcmp(prbuf, "edit") == 0)
	    prflags = 2;
    encread((char *) top_ten, sizeof top_ten, fd);
    /*
     * Insert her in list if need be
     */
    if (!waswizard) {
	for (scp = top_ten; scp < &top_ten[10]; scp++)
	    if (amount > scp->sc_score)
		break;
	if (scp < &top_ten[10]) {
	    for (sc2 = &top_ten[9]; sc2 > scp; sc2--)
		*sc2 = *(sc2-1);
	    scp->sc_score = amount;
	    strcpy(scp->sc_name, whoami);
	    scp->sc_flags = aflag;
	    if (aflag == 2)
		scp->sc_level = max_level;
	    else
		scp->sc_level = level;
	    scp->sc_monster = monst;
	    scp->sc_uid = getuid();
	    scp->sc_explvl = pstats.s_lvl;
	    scp->sc_exppts = pstats.s_exp;
	    strcpy(scp->sc_date,gettime());
	}
    }
    /*
     * Print the list
     */
    printf("\nTop Ten Adventurers:\nRank\tScore\tName\n");
    for (scp = top_ten; scp < &top_ten[10]; scp++) {
	if (scp->sc_score) {
	    printf("%d\t%d\t%s: %s\t\t--> %s on level %d",(int)
	 	(scp - top_ten) + 1,scp->sc_score,scp->sc_name,
		scp->sc_date,reason[scp->sc_flags],scp->sc_level);
	    if (scp->sc_flags == 0) {
		printf(" by a");
		killer = killname(scp->sc_monster);
		if (*killer == 'a' || *killer == 'e' || *killer == 'i' ||
		    *killer == 'o' || *killer == 'u')
			putchar('n');
		printf(" %s", killer);
	    }
	    printf(" [Exp: %d/%ld]",scp->sc_explvl,scp->sc_exppts);
	    if (prflags == 1) {
		struct passwd *pp, *getpwuid(__uid_t);

		if ((pp = getpwuid(scp->sc_uid)) == NULL)
		    printf(" (%d)", scp->sc_uid);
		else
		    printf(" (%s)", pp->pw_name);
		putchar('\n');
	    }
	    else if (prflags == 2) {
		fflush(stdout);
		fgets(prbuf, LINLEN, stdin);
		if (prbuf[0] == 'd') {
		    for (sc2 = scp; sc2 < &top_ten[9]; sc2++)
			*sc2 = *(sc2 + 1);
		    top_ten[9].sc_score = 0;
		    for (i = 0; i < 80; i++)
			top_ten[9].sc_name[i] = rnd(255);
		    top_ten[9].sc_flags = rand();
		    top_ten[9].sc_level = rand();
		    top_ten[9].sc_monster = rand();
		    top_ten[9].sc_date[0] = 0;
		    scp--;
		}
	    }
	    else
		printf("\n");
	}
    }
    fseek(outf, 0L, 0);
    /*
     * Update the list file
     */
    encwrite((char *) top_ten, sizeof top_ten, outf);
    fclose(outf);
}

total_winner(void)
{
    reg struct linked_list *item;
    reg struct object *obj;
    reg int worth, oldpurse, wh, cnt;
    reg char c;

    clear();
    addstr("                                                               \n");
    addstr("  @   @               @   @           @          @@@  @     @  \n");
    addstr("  @   @               @@ @@           @           @   @     @  \n");
    addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr("                                                               \n");
    addstr("     Congratulations, you have made it to the light of day!    \n");
    addstr("\nYou have joined the elite ranks of those who have escaped the\n");
    addstr("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
    addstr("a great profit and are admitted to the fighters guild.\n");
    mvaddstr(LINES - 1, 0,spacemsg);
    refresh();
    wait_for(' ');
    clear();
    mvaddstr(0, 0, "   Worth  Item");
    oldpurse = purse;
    cnt = 1;
    idenpack();
    for (item = pack; item != NULL; item = next(item)) {
	obj = OBJPTR(item);
	worth = get_worth(obj);
	worth *= obj->o_count;
	mvprintw(cnt, 0, "  %5d  %s",worth,inv_name(obj,FALSE));
	purse += worth;
	if (++cnt >= LINES - 2) {
	    cnt = 1;
	    mvaddstr(LINES - 1, 0, morestr);
	    refresh();
	    wait_for(' ');
	    clear();
	}
    }
    mvprintw(cnt + 1,0,"---  %5d  Gold Pieces  ---",oldpurse);
    refresh();
    score(purse, 2, 0);
    byebye(0);
}

char *
killname(char monst)
{
    if (isalpha(monst))
	return monsters[mon_index(monst)].m_name;
    else		/* things other than monsters */
	switch (monst) {
	    case 1:	return "arrow";
	    case 2:	return "dart";
	    case 3:	return "bolt";
	    case 4:	return "magic pool";
	    case 5:	return "exploding rod";
	    case 6:	return "burning scroll";
	}
    return "";
}
