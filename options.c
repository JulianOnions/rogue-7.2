/*
 * This file has all the code for the option command.
 *
 * @(#)options.c	7.0	(Bell Labs)	 10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include "rogue.ext"

/*
 * description of an option and what to do with it
 */
struct optstruct {
    char	*o_name;	/* option name */
    char	*o_prompt;	/* prompt for interactive entry */
    char	*o_opt;		/* pointer to thing to set */
};

typedef struct optstruct	OPTION;

int	put_str(), get_str();

OPTION	optlist[] = {
    {"name", "Name: ", whoami },
    {"fruit", "Fruit: ", fruit },
    {"file", "Save file: ", file_name }
};
#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))

/*
 * print and then set options from the terminal
 */
option()
{
    register OPTION	*op;
    register int	wh;

    wclear(hw);
    touchwin(hw);
    /*
     * Display current values of options
     */
    for (op = optlist; op < &optlist[NUM_OPTS]; op++) {
	wh = op - optlist;
	mvwaddstr(hw, wh, 0, op->o_prompt);
	mvwaddstr(hw, wh, 16, op->o_opt);
    }
    /*
     * Set values
     */
    wmove(hw, 0, 0);
    for (op = optlist; op < &optlist[NUM_OPTS]; op++) {
	wmove(hw, (int)(op - optlist), 16);
	if ((wh = get_str(op->o_opt, hw))) {
	    if (wh == QUIT)
		break;
	    else if (op > optlist) {	/* MINUS */
		wmove(hw, (int)(op - optlist) - 1, 0);
		op -= 2;
	    }
	    else {		/* trying to back up beyond the top */
		putchar('\007');
		wmove(hw, 0, 0);
		op--;
	    }
	}
    }
    /*
     * Switch back to original screen
     */
    dbotline(hw,spacemsg);
    restscr(stdscr);
    after = FALSE;
}


/*
 * get_str:
 *	Set a string option
 */
get_str(opt, awin)
register char *opt;
WINDOW *awin;
{
    register char *sp;
    register int c, oy, ox;
    char buf[70];

    draw(awin);
    getyx(awin, oy, ox);
    /*
     * loop reading in the string, and put it in a temporary buffer
     */
    for (sp = buf; (c=readchar()) != '\n' && c != '\r' && c != ESCAPE;
      wclrtoeol(awin), draw(awin)) {
	if (( (int)sp - (int)buf ) >= 50) {
	    *sp = NULL;			/* line was too long */
	    strcpy(opt,buf);
	    mvwaddstr(awin, 0, 0, "Name was truncated --More--");
	    wclrtoeol(awin);
	    draw(awin);
	    wait_for(' ');
	    sprintf(errbuf,"Called: %s --More--",opt);
	    mvwaddstr(awin, 0, 0, errbuf);
	    draw(awin);
	    wait_for(' ');
	    wmove(awin, 0, 0);
	    wclrtoeol(awin);
	    draw(awin);
	    return NORM;
	}
	if (c == -1)
	    continue;
	else if(c == erasechar())	{	/* process erase char */
	    if (sp > buf) {
		register int i;

		sp--;
		for (i = strlen(unctrl(*sp)); i; i--)
		    waddch(awin, '\b');
	    }
	    continue;
	}
	else if (c == killchar())	{   /* process kill character */
	    sp = buf;
	    wmove(awin, oy, ox);
	    continue;
	}
	else if (sp == buf)
	    if (c == '-')
		break;
	    else if (c == '~') {
		strcpy(buf, home);
		waddstr(awin, home);
		sp += strlen(home);
		continue;
	    }
	*sp++ = c;
	waddstr(awin, unctrl(c));
    }
    *sp = NULL;
    if(sp > buf)	/* only change option if something was typed */
	strucpy(opt, buf, strlen(buf));
    wmove(awin, oy, ox);
    waddstr(awin, opt);
    waddstr(awin, "\012\015");
    draw(awin);
    if (awin == cw)
	mpos += sp - buf;
    if (c == '-')
	return MINUS;
    else if (c == ESCAPE)
	return QUIT;
    else
	return NORM;
}

/*
 * parse options from string, usually taken from the environment.
 * the string is a series of comma seperated values, with booleans
 * being stated as "name" (true) or "noname" (false), and strings
 * being "name=....", with the string being defined up to a comma
 * or the end of the entire option string.
 */

parse_opts(str)
register char *str;
{
    register char *sp;
    register OPTION *op;
    register int len;

    while (*str) {
	for (sp = str; isalpha(*sp); sp++)	/* get option name */
	    continue;
	len = sp - str;
	for (op = optlist; op < &optlist[NUM_OPTS]; op++) {
	    if (EQSTR(str, op->o_name, len)) {
		register char *start;
		for (str = sp + 1; *str == '='; str++)
		    continue;
		if (*str == '~') {
		    strcpy((char *) op->o_opt, home);
		    start = (char *) op->o_opt + strlen(home);
		    while (*++str == '/')
			continue;
		}
		else
		    start = (char *) op->o_opt;
		for (sp = str + 1; *sp && *sp != ','; sp++)
		    continue;
		strucpy(start, str, (int)(sp - str));
	    }
	}
	/*
	 * skip to start of next option name
	 */
	while (*sp && !isalpha(*sp))
	    sp++;
	str = sp;
    }
}

/*
 * copy string using unctrl for things
 */
strucpy(s1, s2, len)
register char *s1, *s2;
register int len;
{
    register char *sp;

    while (len-- > 0) {
	strcpy(s1, (sp = unctrl(*s2++)));
	s1 += strlen(sp);
    }
    *s1 = '\0';
}
