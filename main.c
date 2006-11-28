/*
 * Rogue
 * Exploring the dungeons of doom
 *
 * @(#)main.c	7.0	(Bell Labs)	 10/08/82
 */

#include <signal.h>
#include <pwd.h>
#include "mach_dep.h"
#include "rogue.h"
#ifdef BSD4_2
#include <sys/time.h>
#else
#include <time.h>
#endif
#include "rogue_ext.h"


#ifdef CHECKTIME
static int num_checks;		/* times we've gone over in checkout() */
#endif

#ifdef SYS3
#define FIND(a,b)	strrchr((a),(b))
extern char *strrchr();
#else /*!SYS3*/
#define FIND(a,b)	rindex((a),(b))
extern char *rindex();
#endif /*SYS3*/


int author (void);
int holiday (void);
extern void score (int amount, int aflag, char monst);
extern int parse_opts (register char *str);
extern int strucpy (register char *s1, register char *s2, register int len);
extern int restore (char *file, char **envp);
extern int init_player (void);
extern int init_things (void);
extern int init_names (void);
extern int init_colors (void);
extern int init_stones (void);
extern int init_materials (void);
int setup (void);
extern int new_level (NCURSES_BOOL post);
extern int fuse (int (*func) (/* ??? */), int arg, int time, int type);
extern int daemon (int (*func) (/* ??? */), int arg, int type);
extern int init_weapon (struct object *weap, char type);
extern int setoflg (struct object *what, int bit);
extern int add_pack (struct linked_list *item, NCURSES_BOOL silent);
int playit (void);
int fatal (char *s);
extern int command (void);

main(int argc, char **argv, char **envp)
{
	reg char *env;
	reg struct passwd *pw;
	reg struct linked_list *item;
	reg struct object *obj;
	reg char *ptr;
	struct passwd *getpwuid(__uid_t);
	bool alldone, wpt;
	char *getpass(), *crypt();
	char gamename[50];
	int lowtime;
	long now;

	/*
	 * check for print-score option
	 */

	/*
	 * play at proper times
	 */
	if (!author() && !holiday()) {
		printf("Rogue cannot be played at this time!\n");
		exit(0);
	}

	strcpy(gamename,argv[0]);	/* save the game's name */	
	savetty();			/* save tty params */

	if(argc >= 2 && strcmp(argv[1], "-s") == 0) {
		waswizard = TRUE;
		score(0, -1, 0);
		exit(0);
	}
	if(argc >= 2 && author() && strcmp(argv[1],"-a") == 0) {
		wizard = TRUE;
		argv++;
		argc--;
	}
	/*
	 * Check to see if he is a wizard
	 */
	if(argc >= 2 && strcmp(argv[1],"-w") == 0)
		if(strcmp(PASSWD,crypt(getpass("Wizard's password: "),SALT)) == 0)	{
		    wizard = TRUE;
		    argv++;
		    argc--;
		}
	time(&now);
	lowtime = (int) now;
	/*
	 * get home and options from environment
	 */
	if ((env = getenv("HOME")) != NULL)
		strcpy(home, env);
	else if ((pw = getpwuid(getuid())) != NULL)
		strcpy(home, pw->pw_dir);
	else
		home[0] = '\0';
	strcat(home, "/");
	strcpy(file_name, home);
	ptr = FIND(gamename,'/');
	if(ptr == NULL) {	/* not the full path here */
		strcat(file_name,gamename);
	}
	else {		/* get basename from full path name */
		++ptr;
		strcat(file_name,ptr);
	}
	strcat(file_name,".save");

	if((env = getenv("ROGUEOPTS")) != NULL)
		parse_opts(env);
	if(env == NULL || whoami[0] == '\0')
		if((pw = getpwuid(getuid())) == NULL) {
			printf("Say, who are you?\n");
			exit(1);
		}
		else
		    strucpy(whoami, pw->pw_name, strlen(pw->pw_name));
	if(env == NULL || fruit[0] == '\0')
		strcpy(fruit, "slime-mold");

#if MAXLOAD|MAXUSERS
	if (too_much() && !wizard && !author()) {
	   printf("Sorry, %s, but the system is too loaded now.\n",whoami);
	   printf("Try again later. Meanwhile, why not enjoy a%s %s?\n",
	   vowelstr(fruit), fruit);
	   exit(1);
	}
#endif
	if (argc == 2)
		if(!restore(argv[1], envp)) /* NOTE: NEVER RETURNS */
			exit(1);

	dnum = (wizard && getenv("SEED") != NULL ?
		atoi(getenv("SEED")) : lowtime + getpid());
	if(wizard)
	    printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
	else
	    printf("Hello %s,just a moment while I dig the dungeon...",
		   whoami);
	fflush(stdout);
	seed = dnum;
	srand(seed);			/* init rnd number gen */
	signal(SIGINT,endit);		/* just in case */
	signal(SIGQUIT,endit);
	init_player();			/* Roll up the rogue */
	init_things();			/* Set up probabilities */
	init_names();			/* Set up names of scrolls */
	init_colors();			/* Set up colors of potions */
	init_stones();			/* Set up stones in rings */
	init_materials();		/* Set up materials of wands */
	initscr();			/* Start up cursor package */
	if (strcmp(ttytype,"dumb") == 0) {
	    printf("\n\nERROR in terminal parameters.\n");
	    printf("Check TERM in environment.\n");
	    byebye(1);
	}
	if (LINES < 20 || COLS < 80) {
	    printf("\nERROR: screen size too small\n");
	    byebye(1);
	}
	setup();
	/*
	 * Set up windows
	 */
	cw = newwin(LINES, COLS, 0, 0);
	mw = newwin(LINES, COLS, 0, 0);
	hw = newwin(LINES, COLS, 0, 0);
	waswizard = wizard;
	new_level(FALSE);			/* Draw current level */

	/*
	 * Start up daemons and fuses
	 */
	fuse(swander, TRUE, WANDERTIME, AFTER);
	fuse(chkstairs, TRUE, 500, AFTER);
	daemon(status, TRUE, BEFORE);
	daemon(doctor, TRUE, AFTER);
	daemon(stomach, TRUE, AFTER);
	daemon(runners, TRUE, AFTER);

	/*
	 * Give the rogue his weaponry.
	 */
	item = new_item(sizeof *obj);
	obj = OBJPTR(item);
	obj->o_type = WEAPON;
	alldone = FALSE;
	do {
	    int i, ch;
	    i = rnd(10);	/* number of acceptable weapons */
	    switch(i) {
		case 0: ch = 25; wpt = MACE;
		when 1: ch = 25; wpt = SWORD;
		when 2: ch = 10; wpt = TWOSWORD;
		when 3: ch = 10; wpt = SPEAR;
		when 4: ch = 15; wpt = TRIDENT;
		when 5: ch = 20; wpt = SPETUM;
		when 6: ch = 15; wpt = BARDICHE;
		when 7: ch = 15; wpt = PIKE;
		when 8: ch = 10; wpt = BASWORD;
		when 9: ch = 20; wpt = HALBERD;
	    }
	    if(rnd(100) < ch) {		/* create this weapon */
		alldone = TRUE;
	    }
	} while(!alldone);
	obj->o_which = wpt;		/* save weapon type */
	init_weapon(obj,wpt);		/* init the weapon */
	obj->o_hplus = rnd(3);		/* 0 - 2 plusses */
	obj->o_dplus = rnd(3);
	setoflg(obj,ISKNOW);
	add_pack(item, TRUE);
	cur_weapon = obj;
	/*
	 * Now a bow
	 */
	item = new_item(sizeof *obj);
	obj = OBJPTR(item);
	obj->o_type = WEAPON;
	obj->o_which = BOW;
	init_weapon(obj, BOW);
	obj->o_hplus = rnd(3);
	obj->o_dplus = rnd(3);
	setoflg(obj,ISKNOW);
	add_pack(item, TRUE);
	/*
	 * Now some arrows
	 */
	item = new_item(sizeof *obj);
	obj = OBJPTR(item);
	obj->o_type = WEAPON;
	obj->o_which = ARROW;
	init_weapon(obj, ARROW);
	obj->o_count = 25+rnd(15);
	obj->o_hplus = rnd(2);
	obj->o_dplus = rnd(2);
	setoflg(obj,ISKNOW);
	add_pack(item, TRUE);
	/*
	 * And his suit of armor
	 */
	item = new_item(sizeof *obj);
	obj = OBJPTR(item);
	obj->o_type = ARMOR;
	obj->o_which = RINGMAIL;
	obj->o_ac = armors[RINGMAIL].a_class - rnd(4);
	obj->o_weight = armors[RINGMAIL].a_wght;
	obj->o_count = 1;
	obj->o_group = 0;
	obj->o_flags = 0;
	setoflg(obj,ISKNOW);
	cur_armor = obj;
	add_pack(item, TRUE);
	/*
	 * Give him some food too
	 */
	item = new_item(sizeof *obj);
	obj = OBJPTR(item);
	obj->o_type = FOOD;
	obj->o_count = 1;
	obj->o_which = 0;
	obj->o_group = 0;
	obj->o_flags = 0;
	obj->o_weight = things[TYP_FOOD].mi_wght;
	add_pack(item, TRUE);

	playit();
}


/*
 * endit:
 *	Exit the program abnormally.
 */
endit(void)
{
	fatal("Ok, if you want to exit that badly, I'll have to allow it");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */

fatal(char *s)
{
	clear();
	move(LINES-2, 0);
	printw("%s", s);
	draw(stdscr);
	endwin();
	printf("\r\n");
	byebye(2);
}

/*
 * byebye:
 *	Exit here and reset the users terminal parameters
 *	to the way they were when he started
 */

void byebye(int how)
{
/*    echo();
    nocbreak();
    resetty();		/* restore tty params */
    exit(how);		/* exit like flag says */
}


/*
 * rnd:
 *	Pick a very random number.
 */

rnd(int range)
{
	reg int wh;

	if (range == 0)
		wh = 0;
	else
		wh = abs(rand() % range);
	return (wh);
}

/*
 * roll:
 *	roll a number of dice
 */

roll(int number, int sides)
{
	reg int dtotal = 0;

	while(number-- > 0)
	    dtotal += rnd(sides)+1;
	return dtotal;
}
# ifdef brokenSIGTSTP
/*
 * handle stop and start signals
 */
tstp()
{
	mvcur(0, COLS - 1, LINES - 1, 0);
	endwin();
	fflush(stdout);
	kill(0, SIGTSTP);
	signal(SIGTSTP, tstp);
	cbreak();
	noecho();
	clearok(curscr, TRUE);
	touchwin(cw);
	draw(cw);
	flushout();		/* flush type ahead */
}
# endif

setup(void)
{

#ifdef CHECKTIME
	int  checkout();
#endif

	signal(SIGHUP, auto_save);
#if 0
	signal(SIGILL, auto_save);
	signal(SIGTRAP, auto_save);
	signal(SIGIOT, auto_save);
#ifdef SIGEMT
	signal(SIGEMT, auto_save);
#endif
	signal(SIGFPE, auto_save);
	signal(SIGBUS, auto_save);
	signal(SIGSEGV, auto_save);
	signal(SIGSYS, auto_save);
#endif
	signal(SIGPIPE, auto_save);
	signal(SIGTERM, auto_save);
	signal(SIGQUIT, endit);
	signal(SIGINT,quit);

#ifdef brokenSIGTSTP
	signal(SIGTSTP, tstp);
#endif

#ifdef CHECKTIME
	if (!author()) {
		signal(SIGALRM, checkout);
		alarm(CHECKTIME * 60);
		num_checks = 0;
	}
#endif
	cbreak();				/* Cbreak mode */
	noecho();				/* Echo off */
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 *	refreshing things and looking at the proper times.
 */

playit(void)
{
	reg char *opts;

	/*
	 * parse environment declaration of options
	 */
	if ((opts = getenv("ROGUEOPTS")) != NULL)
		parse_opts(opts);

	oldpos = hero;
	oldrp = roomin(&hero);
	nochange = FALSE;
	while (playing)
		command();		/* Command execution */
	endit();
}


#if MAXLOAD|MAXUSERS
/*
 * see if the system is being used too much for this game
 */
too_much()
{
#ifdef MAXLOAD
	double avec[3];
#else
	reg int cnt;
#endif

#ifdef MAXLOAD
	loadav(avec);
	return (avec[2] > (MAXLOAD / 10.0));
#else
	return (ucount() > MAXUSERS);
#endif
}
#endif

/*
 * author:
 *	See if a user is an author of the program
 */
author(void)
{
	switch (getuid()) {
		case 0:	/* user id of author (was rdk)  */
			return TRUE;
		default:
			return FALSE;
	}
}


#ifdef CHECKTIME
checkout()
{
	static char *msgs[] = {
	"The system is too loaded for games. Please leave in %d minutes",
	"Please save your game.  You have %d minutes",
	"This is your last chance. You had better leave in %d minutes",
	};
	int checktime;

	signal(SIGALRM, checkout);
	if (!keyhit)			/* hasn't touched kbrd */
	    auto_save();		/* NO RETURN */
	keyhit = FALSE;			/* reset flag */
	if (!holiday() && !author()) {
	    wclear(cw);
	    mvwaddstr(cw, LINES / 2, 0,
		"Game time is over. Your game is being saved.\n\n");
	    draw(cw);
	    auto_save();		/* NO RETURN */
	}
	if (too_much())	{
	    if (num_checks >= 3)
		fatal("You didn't listen, so now you are DEAD !!\n");
	    checktime = CHECKTIME / (num_checks + 1);
		chmsg(msgs[num_checks++], checktime);
		alarm(checktime * 60);
	}
	else {
	    if (num_checks) {
		chmsg("The load has dropped. You have a reprieve.");
		num_checks = 0;
	    }
	    alarm(CHECKTIME * 60);
	}
}

/*
 * checkout()'s version of msg.  If we are in the middle of a shell, do a
 * printf instead of a msg to avoid the refresh.
 */
chmsg(fmt, arg)
char *fmt;
int arg;
{
	if (in_shell) {
		printf(fmt, arg);
		putchar('\n');
		fflush(stdout);
	}
	else
		msg(fmt, arg);
}
#endif

#ifdef LOADAV

#include <nlist.h>

struct nlist avenrun =
{
	"_avenrun"
};

loadav(avg)
reg double *avg;
{
	reg int kmem;

	if ((kmem = open("/dev/kmem", 0)) < 0)
		goto bad;
	nlist(NAMELIST, &avenrun);
	if (avenrun.n_type == 0) {
bad:
		avg[0] = avg[1] = avg[2] = 0.0;
		return;
	}
	lseek(kmem, (long) avenrun.n_value, 0);
	read(kmem, avg, 3 * sizeof (double));
}
#endif

#ifdef UCOUNT
/*
 * ucount:
 *	Count the number of people on the system
 */
#include <sys/types.h>
#include <utmp.h>
struct utmp buf;
ucount()
{
	reg struct utmp *up;
	reg FILE *utmp;
	reg int count;

	if ((utmp = fopen(UTMP, "r")) == NULL)
	    return 0;

	up = &buf;
	count = 0;
	while (fread(up, 1, sizeof (*up), utmp) > 0)
		if (buf.ut_type == USER_PROCESS)
			count++;
	fclose(utmp);
	return count;
}

/*
 * holiday:
 *	Returns TRUE when it is a good time to play rogue
 */
holiday()
{
	long now;
	struct tm *localtime();
	reg struct tm *ntime;

	time(&now);			/* get the current time */
	ntime = localtime(&now);
	if(ntime->tm_wday == 0 || ntime->tm_wday == 6)
		return TRUE;		/* OK on Sat & Sun */
	if(ntime->tm_hour < 8 || ntime->tm_hour >= 17)
		return TRUE;		/* OK before 8AM & after 5PM */
	if(ntime->tm_yday <= 7 || ntime->tm_yday >= 350)
		return TRUE;		/* OK during Christmas */
	if (access("/usr/tmp/.ryes",0) == 0)
	    return TRUE;		/* if author permission */

	return FALSE;			/* All other times are bad */
}
# else
holiday (void) { return TRUE; }
# endif
