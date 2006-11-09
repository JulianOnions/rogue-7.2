/*
 * save and restore routines
 *
 * @(#)save.c	7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "rogue.h"
#include "rogue.ext"
#include <errno.h>

EXTCHAR version[], encstr[];
EXTCHAR *ctime();


typedef struct stat STAT;
STAT sbuf;

/*
 * save_game:
 *	Save the current game
 */
save_game()
{
	reg FILE *savef;
	reg int c;
	char buf[LINLEN];

	mpos = 0;
	if (file_name[0] != NULL) {
	    msg("Save file (%s)? ", file_name);
	    do {
		c = lower(getchar());
		if(c == ESCAPE) {
		    msg("");
		    return FALSE;
		}
	    } while (c != 'n' && c != 'y');
	    mpos = 0;
	    if (c == 'y')
		goto gotfile;
	}
	msg("File name: ");
	mpos = 0;
	buf[0] = NULL;
	if(get_str(buf, cw) == QUIT) {
	    msg("");
	    return FALSE;
	}
	strcpy(file_name, buf);
gotfile:
	setuid(getuid());	/* restore id for writing file */
	setgid(getgid());
	umask(022);
	if ((savef = fopen(file_name, "w")) == NULL) {
	    msg(strerror(errno));
	    return FALSE;
	}
	/*
	 * write out encrpyted file (after a stat). The fwrite
	 * is to force allocation of the buffer before the write.
	 */
	save_file(savef);
	return TRUE;
}

/*
 * auto_save:
 *	Automatically save a game
 */
auto_save()
{
	reg FILE *savef;
	reg int i;

	for (i = 0; i < NSIG; i++)
		signal(i, SIG_IGN);
	setuid(getuid());
	setgid(getgid());
	umask(022);
	if (file_name[0] != NULL)
	    if ((savef = fopen(file_name,"w")) != NULL)
		save_file(savef);
	byebye(1);
}

/*
 * save_file:
 *	Write the saved game on the file
 */
save_file(savef)
reg FILE *savef;
{
	wclear(hw);
	draw(hw);
	fstat(fileno(savef), &sbuf);
	fwrite("junk", 1, 5, savef);
	fseek(savef, 0L, 0);
	encwrite(version,(unsigned int)(sbrk(0) - version), savef);
	fclose(savef);
}

/*
 * restore:
 *	Restore a saved game from a file
 */
restore(file, envp)
reg char *file;
char **envp;
{
	reg int inf, pid;
	int ret_status;
	extern char **environ;
	char buf[LINLEN];
	STAT sbuf2;

	if ((inf = open(file, 0)) < 0) {
	    printf("Cannot read save game %s\n",file);
	    return FALSE;
	}
	fflush(stdout);
	encread(buf, strlen(version) + 1, inf);
	if (strcmp(buf, version) != 0) {
	    printf("Sorry, saved game is out of date.\n");
	    return FALSE;
	}

	fstat(inf, &sbuf2);
	fflush(stdout);
	brk(version + sbuf2.st_size);
	lseek(inf, 0L, 0);
	if (wizard)	/* don't do checks if '-w' flag was specified */
		encread(version, (unsigned int) sbuf2.st_size, inf);
	else {
		encread(version, (unsigned int) sbuf2.st_size, inf);
		if (!wizard) {
		    if(sbuf2.st_ino!=sbuf.st_ino || sbuf2.st_dev!=sbuf.st_dev) {
			printf("Sorry, saved game is not in the same file.\n");
			return FALSE;
		    }
		    else if (sbuf2.st_ctime - sbuf.st_ctime > 30) {
			printf("Sorry, file has been touched.\n");
			return FALSE;
		    }
		}
	}
	/*
	 * we do not close the file so that we will have a hold of the
	 * inode for as long as possible
	 */
	mpos = 0;
	mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf2.st_mtime));
	/*
	 * defeat multiple restarting from the same place
	 */
	if (!wizard) {
	    if (sbuf2.st_nlink != 1) {
		printf("Cannot restore from a linked file\n");
		return FALSE;
	    }
	    else {
		while((pid = fork()) < 0)
			sleep(1);
		if(pid == 0) {		/* set id to unlink file */
		    setuid(getuid());
		    setgid(getgid());
		    exit(unlink(file));
		}
		else {			/* wait for unlink to finish */
		    while(wait(&ret_status) != pid)
			continue;
		    if (ret_status < 0) {
			printf("Cannot unlink file\n");
			return FALSE;
		    }
		}
	    }
	}
	environ = envp;
	if (isatty(2)) {
	    reg char	*sp;

	    gettmode();
	    if ((sp = getenv("TERM")) == NULL)
		sp = "xterm";
	    setterm(sp);
	}
	else
	    setterm("xterm");
	strcpy(file_name, file);
	setup();
	clearok(curscr, TRUE);
	touchwin(cw);
	srand(getpid());
	playit();
}

/*
 * perform an encrypted write
 */
encwrite(start, size, outf)
reg char *start;
unsigned int size;
reg FILE *outf;
{
	reg char *ep;

	ep = encstr;

	while (size--)
	{
		putc(*start++ ^ *ep++, outf);
		if (*ep == '\0')
			ep = encstr;
	}
}


/*
 * perform an encrypted read
 */
encread(start, size, inf)
reg char *start;
unsigned int size;
reg int inf;
{
	reg char *ep, *buff;
	reg int rdsiz;
	reg unsigned int siz;

	buff = start;
	for (siz = size; siz > 0;) {
	    rdsiz = read(inf,buff,(siz > 16384 ? 16384 : siz));
	    if (rdsiz == -1 || rdsiz == 0)
		return rdsiz;
	    siz -= rdsiz;
	    buff += rdsiz;
	}

	ep = encstr;
	siz = size;
	while (siz--) {
	    *start++ ^= *ep++;
	    if (*ep == NULL)
		ep = encstr;
	}
	return size;
}
