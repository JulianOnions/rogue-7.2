#include <stddef.h>
#include <stdlib.h>

#define EXTLKL	extern struct linked_list
#define EXTTHG	extern struct thing
#define EXTOBJ	extern struct object
#define EXTSTAT extern struct stats
#define EXTCORD	extern struct coord
#define EXTMON	extern struct monster
#define EXTARM	extern struct init_armor
#define EXTWEP	extern struct init_weps
#define EXTMAG	extern struct magic_item
#define EXTROOM	extern struct room
#define EXTTRAP	extern struct trap
#define EXTLONG	extern long
#define EXTINT	extern int
#define EXTBOOL	extern bool
#define EXTCHAR	extern char

void quit(int junk);

extern char **environ;
extern int prntstats(char prtype);
extern struct h_list helpstr[];
extern int _attach (register struct linked_list **list, register struct linked_list *item);
extern int _detach (register struct linked_list **list, register struct linked_list *item);
extern int _free_list (register struct linked_list **ptr);
extern int activity (void);
extern int add_dam (int str);
extern int add_haste (NCURSES_BOOL potion);
extern int add_pack (struct linked_list *item, NCURSES_BOOL silent);
extern int add_pass (void);
extern int aggravate (void);
extern int attack (struct thing *mp);
extern int author (void);
extern int buy_it (void);
extern int cansee (int y, int x);
extern int check_level (void);
extern int chg_abil (register int what, register int amt, register int how);
extern int chg_hpt (int howmany, NCURSES_BOOL alsomax, char whichmon);
extern int command (void);
extern int create_obj (NCURSES_BOOL fscr);
extern int cur_null (struct object *op);
extern int daemon (int (*func) (/* ??? */), int arg, int type);
extern int dead_end (char ch);
extern int death (char monst);
extern int del_pack (struct linked_list *what);
extern int diag_ok (struct coord *sp, struct coord *ep);
extern int discard (register struct linked_list *item);
extern int displevl (void);
extern int dispmons (void);
extern int do_daemons (int flag);
extern int do_fuses (int flag);
extern int do_motion (struct object *obj, int ydelta, int xdelta);
extern int do_move (int dy, int dx);
extern int do_passages (void);
extern int do_post (void);
extern int do_rooms (void);
extern int do_run (char ch);
extern int do_zap (NCURSES_BOOL gotdir);
extern int draw_room (struct room *rp);
extern int drop (struct linked_list *item);
extern int dropcheck (struct object *op);
extern int eat (void);
extern int encread (char *start, unsigned int size, int inf);
extern int encwrite (char *start, unsigned int size, FILE *outf);
extern int extinguish (int (*func) (/* ??? */));
extern int fall (struct linked_list *item, NCURSES_BOOL pr);
extern int fallpos (struct coord *pos, struct coord *newpos, NCURSES_BOOL passages);
extern int fatal (char *s);
extern int fight (struct coord *mp, char mn, struct object *weap, NCURSES_BOOL thrown);
extern int fix_stick (struct object *cur);
extern int fuse (int (*func) (/* ??? */), int arg, int time, int type);
extern int genocide (void);
extern int get_dir (void);
extern int get_str (register char *opt, WINDOW *awin);
extern int get_worth (struct object *obj);
extern int getbless (void);
extern int getpcon (int econ);
extern int getpdex (int edex, NCURSES_BOOL heave);
extern int getpwis (int ewis);
extern int hit_monster (int y, int x, struct object *obj);
extern int hitweight (void);
extern int idenpack (void);
extern int illeg_ch (char ch);
extern int init_colors (void);
extern int init_materials (void);
extern int init_names (void);
extern int init_player (void);
extern int init_ring (struct object *what, NCURSES_BOOL fromwiz);
extern int init_stones (void);
extern int init_things (void);
extern int init_weapon (struct object *weap, char type);
extern int inventory (struct linked_list *list, int type);
extern int is_current (struct object *obj);
extern int is_magic (struct object *obj);
extern int isatrap (char ch);
extern int isring (int hand, int ring);
extern int iswearing (int ring);
extern int itemweight (struct object *wh);
extern int kill_daemon (int (*func) (/* ??? */));
extern int killed (struct linked_list *item, NCURSES_BOOL pr);
extern int lengthen (int (*func) (/* ??? */), int xtime);
extern int lev_mon (void);
extern int light (struct coord *cp);
extern int look (NCURSES_BOOL wakeup);
extern int magring (struct object *what);
extern int missile (int ydelta, int xdelta);
extern int mon_index (char whichmon);
extern int money (void);
extern int new_level (NCURSES_BOOL post);
extern int new_monster (struct linked_list *item, char type, struct coord *cp, NCURSES_BOOL treas);
extern int o_off (struct object *what, int bit);
extern int o_on (struct object *what, int bit);
extern int option (void);
extern int parse_opts (register char *str);
extern int passwd (void);
extern int pick_up (char ch);
extern int picky_inven (void);
extern int playit (void);
extern int price_it (void);
extern int quaff (void);
extern int raise_level (void);
extern int randmonster (NCURSES_BOOL wander, NCURSES_BOOL baddie);
extern int read_scroll (void);
extern int readchar (void);
extern int removelist (struct coord *mp, struct linked_list *item);
extern int resoflg (struct object *what, int bit);
extern int restore (char *file, char **envp);
extern int ring_eat (int hand);
extern int ring_off (void);
extern int ring_on (void);
extern int ringabil (void);
extern int rnd_pos (struct room *rp, struct coord *cp);
extern int rnd_room (void);
extern int roll (int number, int sides);
extern int runto (struct coord *runner, struct coord *spot);
extern int save (int which);
extern int save_game (void);
extern int save_throw (int which, struct thing *tp);
extern int sell_it (void);
extern int setoflg (struct object *what, int bit);
extern int setup (void);
extern int show (int y, int x);
extern int sight (int fromfuse);
extern int step_ok (char ch);
extern int str_plus (int str);
extern int strucpy (register char *s1, register char *s2, register int len);
extern int take_off (void);
extern int teleport (void);
extern int toss_ring (struct object *what);
extern int total_winner (void);
extern int totalenc (void);
extern int updpack (int getmax);
extern int wanderer (void);
extern int waste_time (void);
extern int wear (void);
extern int wghtchk (int type);
extern int whatis (struct linked_list *what);
extern int wield (void);
extern int winat (int y, int x);
extern void addmsg (const char *fmt, ...);
extern void debug (char *errstr);
extern void msg (const char *fmt, ...);
extern void prhwfile (char *fname);
extern void score (int amount, int aflag, char monst);
extern void wait_for(register char ch);
void dbotline(WINDOW *scr, char *message);
void restscr(WINDOW *scr);
void status(int fromfuse);
EXTTHG player;
EXTWEP weaps[];
EXTARM armors[];
EXTMON monsters[], *mtlev[];
EXTTRAP *trap_at(int y, int x), traps[];
EXTROOM *roomin(struct coord *cp), *oldrp, rooms[];
EXTCORD *rndmove(struct thing *who), delta, stairs, oldpos;
EXTLKL *mlist, *lvl_obj, *new_item(int size), *new_thing(NCURSES_BOOL treas);
EXTLKL *find_mons(int y, int x), *wake_monster(int y, int x), *find_obj(int y, int x), *get_item(char *purpose, int type);
EXTOBJ *cur_armor, *cur_weapon, *cur_ring[];
EXTMAG r_magic[], s_magic[], ws_magic[], p_magic[], things[];
EXTLONG pl_on(long int what), pl_off(long int what);
EXTINT max_hp, quiet, food_left, hungry_state, level, max_level, no_food;
EXTINT foodlev, total, count, inpack, demoncnt, fung_hit, ntraps;
EXTINT lastscore, purse, no_command, mpos, seed, dnum, no_move;
EXTINT curprice, trader, group, levcount;
EXTINT chkstairs(int fromfuse), rollwand(int fromfuse), swander(int fromfuse), notslow(int fromfuse), notfight(int fromfuse);
EXTINT rchg_str(int amt), stomach(int fromfuse), doctor(int fromfuse), runners(void), rnd(int range);
EXTINT prntfile(), auto_save(void), endit(void), unconfuse(int fromfuse), sapem(int fromfuse);
EXTINT noteth(int fromfuse), notregen(int fromfuse), notinvinc(int fromfuse), unsee(int fromfuse), nohaste(int fromfuse);
EXTBOOL running, nochange, after, inwhgt, isfight, firstmove, keyhit;
EXTBOOL wizard, waswizard, in_shell, amulet, door_stop, playing, pooltelep;
EXTBOOL notify, ws_know[], p_know[], s_know[], r_know[], tradelev;
EXTCHAR home[], file_name[], whoami[], fruit[], curpurch[];
EXTCHAR *ws_guess[], *s_guess[], *r_guess[], *p_guess[];
EXTCHAR *r_stones[], *p_colors[], *s_names[], *ws_type[], *ws_made[];
EXTCHAR errbuf[], prbuf[], huh[], *morestr, *spacemsg, *typ_name(struct object *obj);
EXTCHAR *new(int size),  *inv_name(struct object *obj, NCURSES_BOOL drop), pack_char(struct object *obj);
EXTCHAR *tr_name(char ch), *release, take, runch;
EXTCHAR *num(int n1, int n2), *ring_num(struct object *what), *charge_str(struct object *obj), *vowelstr(char *str), *gettime(void), lower(char c);
EXTSTAT max_stats;
extern struct real re_stats;

void byebye(int how);
void status(int fromfuse);
void wait_for(register char ch);
void dbotline(WINDOW *scr, char *message);
void restscr(WINDOW *scr);
void quit(int junk);
