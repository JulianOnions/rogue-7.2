/*
 * Various input/output functions
 *
 * @(#)io.c	7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include "mach_dep.h"
#include "rogue.ext"
#include <stdarg.h>

static void doadd(const char *fmt, va_list ap);
static void endmsg();

/*
 * msg:
 *	Display a message at the top of the screen.
 */

static char msgbuf[BUFSIZ];
static int newpos = 0;


/*VARARGS1*/
void msg(const char *fmt, ...)
{
    va_list ap;
    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0')
    {
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	mpos = 0;
	return;
    }
    va_start(ap, fmt);
    /*
     * otherwise add to the message and flush it out
     */
    doadd(fmt, ap);
    va_end(ap);
    endmsg();
}

/*
 * add things to the current message
 */
void addmsg(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    doadd(fmt, ap);
    va_end(ap);
}

/*
 * Display a new msg (giving him a chance to see the previous one if it
 * is up there with the --More--)
 */
void endmsg()
{
    strcpy(huh, msgbuf);
    if (mpos)
    {
	mvwaddstr(cw, 0, mpos, morestr);
	draw(cw);
	wait_for(' ');
    }
    mvwaddstr(cw, 0, 0, msgbuf);
    wclrtoeol(cw);
    draw(cw);
    mpos = newpos;
    newpos = 0;
}

void doadd(const char *fmt, va_list ap)
{
    vsprintf(&msgbuf[newpos], fmt, ap);
    newpos = strlen(msgbuf);
}

/*
 * step_ok:
 *	Returns TRUE if it is ok to step on ch
 */

int step_ok(ch)
char ch;
{
	if (dead_end(ch))
		return FALSE;
	else
		return (!isalpha(ch));
}


/*
 * dead_end:
 *	Returns TRUE if you cant walk through that character
 */
int dead_end(ch)
char ch;
{
	if (ch == '-' || ch == '|' || ch == ' ' || ch == SECRETDOOR)
		return TRUE;
	else
		return FALSE;
}


/*
 * readchar:
 *	flushes stdout so that screen is up to date and then returns
 *	getchar.
 */

int readchar()
{
    char c;
    int failcount = 0;

    fflush(stdout);
    while (read(0, &c, 1) <= 0) {
        if(failcount++ > 64) auto_save();
	continue;
    }
    return c;
}

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */

void status(int fromfuse)
{
    register int oy, ox;
    register char *pb;
    static char buf[LINLEN];

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    if (nochange)
	return;

    nochange = TRUE;	/* assume no change until something changes it */
    getyx(cw, oy, ox);
    sprintf(buf, "Level: %d  Gold: %-5d  Hp: %d(%d)  Str: %-2d",
	level, purse, pstats.s_hpt, max_hp, pstats.s_ef.a_str);
    pb = &buf[strlen(buf)];
    sprintf(pb, "  Ac: %-2d  Exp: %d/%ld",
	cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm, pstats.s_lvl,
	pstats.s_exp);
    mvwaddstr(cw, LINES - 1, 0, buf);
    switch (hungry_state) {
	case F_HUNGRY:
	    waddstr(cw, "  Hungry");
	when F_WEAK:
	    waddstr(cw, "  Weak");
	when F_FAINT:
	    waddstr(cw, "  Fainting");
    }
    wclrtoeol(cw);
    wmove(cw, oy, ox);
}

/*
 * illeg_ch:
 * 	Returns TRUE if a char shouldn't show on the screen
 */
int illeg_ch(char ch)
{
    if (ch < 32 || ch > 127)
	return TRUE;
    if (ch >= '0' && ch <= '9')
	return TRUE;
    return FALSE;
}

/*
 * wait_for
 *	Sit around until the guy types the right key
 */

void wait_for(register char ch)
{
    register char c;

    if (ch == '\n')
        while ((c = readchar()) != '\n' && c != '\r')
	    continue;
    else
        while (readchar() != ch)
	    continue;
}

#include <pwd.h>

/*
 * This routine reports the info from a debug call
 */
void debug(errstr)
char *errstr;
{
	register FILE *dfp;
	char *timeptr, *gettime();
	struct passwd *pp, *getpwuid();

	if((dfp = fopen(BUGFILE,"a")) != NULL) { /* save in bug file */
		timeptr = gettime();		/* get time string */
		pp = getpwuid(getuid());	/* get password entry */
		fprintf(dfp,"%s: %s: %s",pp->pw_name,errstr,timeptr);
		fclose(dfp);
	}
	if(wizard || author()) {
		msg(errstr);		/* print message if wizard */
	}
}

/*
 * gettime:
 *	This routine returns the current time as a string
 */
#ifdef BSD4_2
#include <sys/time.h>
#else
#include <time.h>
#endif
char *gettime()
{
	register char *timeptr;
	char *ctime();
	long int now, time();

	time(&now);		/* get current time */
	timeptr = ctime(&now);	/* convert to string */
	return timeptr;		/* return the string */
}


/*
 * prhwfile:
 *	This prints out files for the wizard
 */
void prhwfile(char *fname)
{
	register FILE *tfp;
	register char c;
	register int linecount;

	if(!author())		/* only for the authors eyes */
		return;
	if((tfp = fopen(fname,"r")) == NULL) {
		msg("File not readable");
		return;		/* go if file not readable */
	}
	wclear(hw);
	linecount = 0;
	while((c = fgetc(tfp)) != EOF) {
		waddch(hw,c);
		if(c == '\n')
			++linecount;
		if(linecount >= LINES - 2) {
			dbotline(hw,morestr);
			linecount = 0;
			wclear(hw);
		}
	}
	dbotline(hw,spacemsg);
	restscr(cw);			/* redraw level */
	return;
}


/*
 * dbotline:
 *	Displays message on bottom line and waits for a space to return
 */
void dbotline(WINDOW *scr, char *message)
{
	mvwaddstr(scr,LINES-1,0,message);
	draw(scr);
	wait_for(' ');	
}


/*
 * restscr:
 *	Restores the screen to the terminal
 */
void restscr(WINDOW *scr)
{
	clearok(scr,TRUE);
	touchwin(scr);
}
