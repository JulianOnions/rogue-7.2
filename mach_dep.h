/*
 * machine dependencies
 *
 * @(#)mach_dep.h	7.0	(Bell Labs)	10/14/82
 */

/*
 * where scorefile should live
 */
#define SCOREFILE	"/usr/sheriff/jpo/games/lib/rogue_roll7.2"

/*
 * Where the "debug" will print its stuff
 */
#define BUGFILE		"/usr/sheriff/jpo/games/lib/rogue_bugs7.2"

/*
 * Variables for checking to make sure the system isn't too loaded
 * for people to play
 */

#ifdef NEVERDEF
#define	MAXUSERS	25	/* max number of users for this game */
#undef	MAXLOAD		40	/* 10 * max 15 minute load average */
#endif

#ifdef MAXUSERS|MAXLOAD
#define	CHECKTIME	5	/* number of minutes between load checks */
				/* if not defined, checks only on start */
#endif

#ifdef MAXLOAD
#define	LOADAV			/* defined if rogue provides loadav() */

#ifdef LOADAV
#define	NAMELIST	"/unix"	/* where the system namelist lives */
#endif
#endif

#ifdef MAXUSERS
#define	UCOUNT			/* defined if rogue provides ucount() */

#ifdef UCOUNT
#define UTMP	"/etc/utmp"	/* where utmp file lives */
#endif
#endif
