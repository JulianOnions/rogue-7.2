/*
 * global variable declaration
 *
 * @(#)global.c	7.0	(Bell Labs)	10/14/82
 */

#include "rogue.h"

struct room rooms[MAXROOMS];		/* One for each room -- A level */
struct room *oldrp;			/* Roomin(&oldpos) */
struct linked_list *mlist = NULL;	/* List of monsters on the level */
struct thing player;			/* The rogue */
struct stats max_stats;			/* The maximum for the player */
struct linked_list *lvl_obj = NULL;	/* List of objects on this level */
struct object *cur_weapon = NULL;	/* Which weapon he is weilding */
struct object *cur_armor = NULL;	/* the rogue's armor */
struct object *cur_ring[2];		/* Which rings are being worn */
struct trap traps[MAXTRAPS];		/* traps on this level */

int level = 1;				/* What level rogue is on */
int levcount = 0;			/* # of active mons this level */
int trader = 0;				/* no. of purchases */
int curprice = -1;			/* current price of item */
int purse = 0;				/* How much gold the rogue has */
int mpos = 0;				/* Where cursor is on top line */
int ntraps;				/* Number of traps on this level */
int no_move = 0;			/* Number of turns held in place */
int no_command = 0;			/* Number of turns asleep */
int inpack = 0;				/* Number of things in pack */
int max_hp = 0;				/* Player's max hit points */
int total = 0;				/* Total dynamic memory bytes */
int demoncnt = 0;			/* number of active daemons */
int lastscore = -1;			/* Score before this turn */
int no_food = 0;			/* Number of levels without food */
int seed;				/* Random number seed */
int dnum;				/* Dungeon number */
int count = 0;				/* Number of times to repeat cmd */
int fung_hit = 0;			/* Number of time fungi has hit */
int quiet = 0;				/* Number of quiet turns */
int max_level = 1;			/* Deepest player has gone */
int food_left = HUNGERTIME;		/* Amount of food stomach */
int group = 1;				/* Current group number */
int hungry_state = F_OK;		/* How hungry is he */
int foodlev = 1;			/* how fast he eats food */
char take;				/* Thing the rogue is taking */
char runch;				/* Direction player is running */
char curpurch[15];			/* name of item ready to buy */

char errbuf[LINLEN];			/* used for debug */
char prbuf[LINLEN];			/* Buffer for sprintfs */
char whoami[LINLEN];			/* Name of player */
char fruit[LINLEN];			/* Favorite fruit */
char huh[LINLEN];			/* The last message printed */
char file_name[LINLEN];			/* Save file name */
char home[LINLEN];			/* User's home directory */
char outbuf[BUFSIZ];			/* Output buffer for stdout */

char *s_guess[MAXSCROLLS];		/* his guess at what scroll is */
char *p_guess[MAXPOTIONS];		/* his guess at what potion is */
char *r_guess[MAXRINGS];		/* his guess at what ring is */
char *ws_guess[MAXSTICKS];		/* his guess at what wand is */
char *ws_type[MAXSTICKS];		/* Is it a wand or a staff */

bool keyhit = FALSE;			/* true when a key was hit */
bool isfight = FALSE;			/* true if player is fighting */
bool pooltelep = FALSE;			/* true if pool did teleporting */
bool inwhgt = FALSE;			/* true if from wghtchk() */
bool running = FALSE;			/* True if player is running */
bool playing = TRUE;			/* True until he quits */
bool tradelev = FALSE;			/* True when at trading post */
bool wizard = FALSE;			/* True if he is a wizard */
bool after = TRUE;			/* True if we want after daemons */
bool notify = TRUE;			/* True if player wants to know */
bool door_stop = FALSE;			/* Stop run when we pass a door */
bool firstmove = FALSE;			/* First move after door_stop */
bool waswizard = FALSE;			/* Was a wizard sometime */
bool amulet = FALSE;			/* He found the amulet */
bool in_shell = FALSE;			/* True if executing a shell */
bool nochange = FALSE;			/* true if last stat same as now */

bool s_know[MAXSCROLLS];		/* Does he know about a scroll */
bool p_know[MAXPOTIONS];		/* Does he know about a potion */
bool r_know[MAXRINGS];			/* Does he know about a ring */
bool ws_know[MAXSTICKS];		/* Does he know about a stick */

char *spacemsg = "--Press space to continue--";
char *morestr = "--More--";

struct coord oldpos;			/* Pos before last look() call */
struct coord delta;			/* Change indicated to get_dir() */
struct coord stairs;			/* where the stairs are put */

struct monster *mtlev[MONRANGE];	/* no monsters yet */

#define _r {10,10,10,10}	/* real ability (unused) */
#define _p 0,0,0		/* hit points, pack, carry (unused) */
#define _c 10			/* constitution (unused) */

/*
 * NAME SHOW CARRY {LEVEL} FLAGS _r {STR DEX WIS _c} EXP LVL ARM _p DMG
 */
struct monster monsters[MAXMONS] = {
{"giant ant",'A',0,{3,12,1},ISMEAN,_r,{10,16,5,_c},10,2,3,_p,"1d6"},
{"bat",'B',0,{1,6,1},0,_r,{10,10,10,_c},1,1,3,_p,"1d2"},
{"centaur",'C',15,{8,17,1},0,_r,{16,10,15,_c},15,4,4,_p,"1d6/1d6"},
{"red dragon",'D',100,{21,99,0},ISGREED,_r,{17,10,17,_c},9000,10,-1,_p,"1d8/1d8/3d10"},
{"floating eye",'E',0,{2,11,0},0,_r,{10,10,10,_c},5,1,9,_p,"0d0"},
{"violet fungi",'F',0,{15,24,0},ISMEAN|ISSTUCK,_r,{10,5,3,_c},85,8,2,_p,"000d0"},
{"gnome",'G',10,{6,15,1},0,_r,{10,10,10,_c},8,1,5,_p,"1d6"},
{"hobgoblin",'H',0,{1,8,1},ISMEAN,_r,{10,10,10,_c},3,1,5,_p,"1d8"},
{"invisible stalker",'I',0,{16,25,1},ISINVIS,_r,{10,15,15,_c},120,8,3,_p,"4d4"},
{"jackal",'J',0,{1,6,1},ISMEAN,_r,{10,10,10,_c},2,1,7,_p,"1d2"},
{"kobold",'K',0,{1,6,1},ISMEAN,_r,{10,10,10,_c},1,1,8,_p,"1d4"},
{"leprechaun",'L',0,{7,16,0},0,_r,{10,15,16,_c},10,3,8,_p,"1d1"},
{"mimic",'M',30,{19,99,0},0,_r,{10,10,10,_c},140,7,7,_p,"3d4"},
{"nymph",'N',100,{11,20,0},0,_r,{10,18,10,_c},40,3,9,_p,"0d0"},
{"orc",'O',15,{4,13,1},0,_r,{10,10,10,10},5,1,6,_p,"1d8"},
{"purple worm",'P',70,{22,99,0},0,_r,{18,5,10,_c},7000,15,6,_p,"2d12/2d4"},
{"quasit",'Q',30,{10,19,1},ISMEAN,_r,{10,15,16,_c},35,3,2,_p,"1d2/1d2/1d4"},
{"rust monster",'R',0,{9,18,1},ISMEAN,_r,{10,10,10,_c},25,5,2,_p,"0d0/0d0"},
{"snake",'S',0,{1,7,1},ISMEAN,_r,{10,10,10,_c},3,1,5,_p,"1d3"},
{"troll",'T',50,{13,22,0},ISMEAN|ISREGEN,_r,{10,10,10,_c},55,6,4,_p,"1d8/1d8/2d6"},
{"umber hulk",'U',40,{18,99,1},ISMEAN,_r,{17,10,10,_c},130,8,2,_p,"3d4/3d4/2d5"},
{"vampire",'V',20,{20,99,1},ISMEAN|ISREGEN,_r,{21,16,16,_c},380,8,1,_p,"1d10"},
{"wraith",'W',0,{14,23,1},ISMEAN,_r,{10,10,10,_c},55,5,4,_p,"1d6"},
{"xorn",'X',0,{17,26,1},ISMEAN,_r,{17,6,10,_c},120,7,-2,_p,"1d3/1d3/1d3/4d6"},
{"yeti",'Y',30,{12,21,1},ISMEAN,_r,{10,10,10,_c},50,4,6,_p,"1d6/1d6"},
{"zombie",'Z',0,{5,14,1},ISMEAN,_r,{10,10,10,_c},7,2,8,_p,"1d8"},
{"anhkheg",'a',10,{7,16,1},ISMEAN,_r,{10,15,3,_c},20,3,2,_p,"3d6"},
{"giant beetle",'b',0,{9,18,1},ISMEAN,_r,{10,15,10,_c},30,5,3,_p,"4d4"},
{"cockatrice",'c',100,{8,17,0},0,_r,{10,10,10,_c},200,5,6,_p,"1d3"},
{"bone devil",'d',0,{27,99,1},ISMEAN,_r,{18,10,16,_c},8000,9,-1,_p,"3d4"},
{"elasmosaurus",'e',0,{28,99,1},ISMEAN,_r,{17,5,3,_c},4500,11,7,_p,"4d6"},
{"killer frog",'f',0,{3,8,1},ISMEAN,_r,{10,10,10,_c},4,2,8,_p,"1d3/1d3/1d4"},
{"green dragon",'g',50,{25,99,1},0,_r,{18,10,18,_c},7500,8,2,_p,"1d6/1d6/2d10"},
{"hell hound",'h',20,{10,19,1},ISMEAN,_r,{10,15,10,_c},30,4,4,_p,"1d10"},
{"imp",'i',20,{2,9,1},ISMEAN|ISREGEN,_r,{10,14,10,_c},6,2,2,_p,"1d4"},
{"jaguar",'j',0,{10,19,0},0,_r,{10,10,10,_c},25,7,6,_p,"1d3/1d3/1d8"},
{"koppleganger",'k',20,{8,17,1},ISMEAN,_r,{10,10,16,_c},35,4,5,_p,"1d12"},
{"lonchu",'l',15,{2,9,1},ISMEAN,_r,{10,4,18,_c},5,2,1,_p,"1d4/1d4"},
{"minotaur",'m',0,{12,21,1},ISMEAN,_r,{10,10,10,_c},40,6,6,_p,"2d4"},
{"neotyugh",'n',10,{14,23,1},ISMEAN,_r,{10,6,4,_c},50,6,3,_p,"1d8/1d8/2d2"},
{"ogre",'o',50,{7,16,1},0,_r,{20,10,10,_c},15,4,5,_p,"1d10"},
{"pseudo dragon",'p',50,{9,18,1},0,_r,{10,10,16,_c},20,4,2,_p,"2d3/1d6"},
{"quellit",'q',85,{30,99,1},0,_r,{17,10,10,_c},12500,17,0,_p,"2d10/2d6"},
{"rhynosphinx",'r',40,{26,99,0},0,_r,{19,6,18,_c},5000,10,-1,_p,"2d8/2d8"},
{"shadow",'s',15,{5,14,1},ISMEAN|ISREGEN|ISINVIS,_r,{10,17,18,_c},6,3,7,_p,"1d6"},
{"titanothere",'t',0,{19,99,0},0,_r,{17,6,3,_c},750,12,6,_p,"2d8"},
{"ulodyte",'u',10,{2,8,1},ISMEAN,_r,{10,10,10,_c},3,2,5,_p,"1d3/1d3"},
{"vrock",'v',0,{4,13,1},ISMEAN,_r,{10,10,10,_c},8,3,2,_p,"1d4/1d6"},
{"wuccubi",'w',0,{14,23,1},ISMEAN,_r,{10,10,10,_c},90,6,0,_p,"1d3/1d10"},
{"xonoclon",'x',0,{20,99,0},0,_r,{19,10,4,_c},1750,13,0,_p,"3d8"},
{"yeenoghu",'y',10,{15,24,1},ISMEAN,_r,{17,15,10,_c},250,7,1,_p,"3d6"},
{"zemure",'z',0,{1,6,1},ISMEAN|ISREGEN,_r,{10,10,10,_c},4,2,7,_p,"1d4"},
};

#undef _p		/* erase these definitions */
#undef _c
#undef _r

struct h_list helpstr[] = {
	'?',	"	prints help",
	'/',	"	identify object",
	'h',	"	left",
	'j',	"	down",
	'k',	"	up",
	'l',	"	right",
	'y',	"	up & left",
	'u',	"	up & right",
	'b',	"	down & left",
	'n',	"	down & right",
	'H',	"	run left",
	'J',	"	run down",
	'K',	"	run up",
	'L',	"	run right",
	'Y',	"	run up & left",
	'U',	"	run up & right",
	'B',	"	run down & left",
	'N',	"	run down & right",
	't',	"<dir>	throw something",
	'f',	"<dir>	forward until find something",
	'p',	"<dir>	zap a wand in a direction",
	'z',	"	zap a wand or staff",
	'>',	"(<)	go down(up) a staircase",
	's',	"	search for trap/secret door",
	' ',	"	(space) rest for a while",
	'i',	"	inventory pack",
	'I',	"	inventory single item",
	'q',	"	quaff potion",
	'r',	"	read a scroll",
	'e',	"	eat food",
	'w',	"	wield a weapon",
	'W',	"	wear armor",
	'T',	"	take armor off",
	'P',	"	put on ring",
	'R',	"	remove ring",
	'd',	"	drop object",
	'c',	"	call object",
	'o',	"	examine/set options",
	'a',	"	encumberence",
	'x',	"	dexterity & constitution",
	CTRL(L),"	redraw screen",
	CTRL(R),"	repeat last message",
	ESCAPE,	"	cancel command",
	'!',	"	shell escape",
	'S',	"	save game",
	'Q',	"	quit",
	0, 0
};

char *s_names[MAXSCROLLS];		/* Names of the scrolls */
char *p_colors[MAXPOTIONS];		/* Colors of the potions */
char *r_stones[MAXRINGS];		/* Stone settings of the rings */
char *ws_made[MAXSTICKS];		/* What sticks are made of */

struct magic_item things[NUMTHINGS] = {
	{ "potion",	257,	 5 },	/* potion */
	{ "scroll",	250,	30 },	/* scroll */
	{ "food",	185,	 7 },	/* food */
	{ "weapon",	 92,	 0 },	/* weapon */
	{ "armor",	 92,	 0 },	/* armor */
	{ "ring",	 62,	 5 },	/* ring */
	{ "stick",	 62,	 0 },	/* stick */
	{ "amulet",	 0,   -250 },	/* amulet */
};


struct init_armor armors[MAXARMORS] = {
	{ "leather armor",		17,  8,   5, 150 },
	{ "ring mail",			13,  7,  30, 250 },
	{ "studded leather armor",	13,  7, 200, 200 },
	{ "scale mail",			12,  6,   3, 400 },
	{ "padded armor",		10,  6, 250, 100 },
	{ "chain mail",			 9,  5,  75, 300 },
	{ "splint mail",		 9,  4,  80, 400 },
	{ "banded mail",		 9,  4,  90, 350 },
	{ "plate mail",		 	 5,  3, 400, 450 },
	{ "plate armor",		 3,  2, 650, 350 },
};

struct init_weps weaps[MAXWEAPONS] = {
    { "mace",		"2d4",  "1d3", NONE,  0, 100, 8 },
    { "long sword",	"1d10", "1d2", NONE,  0, 60, 15 },
    { "short bow",	"1d1",  "1d1", NONE,  0, 40, 75 },
    { "arrow",		"1d1",  "1d6", BOW,   ISMANY|ISMISL, 5, 1 },
    { "dagger",		"1d6",  "1d4", NONE,  ISMISL, 10, 2 },
    { "rock",		"1d2",  "1d4", SLING, ISMANY|ISMISL, 5, 1 },
    { "two-handed sword","3d6",  "1d2", NONE,  0, 250, 30 },
    { "sling",		"0d0",  "0d0", NONE,  0, 5, 1 },
    { "dart",		"1d1",  "1d3", NONE,  ISMANY|ISMISL, 5, 1 },
    { "crossbow",	"1d1",  "1d1", NONE,  0, 100, 15 },
    { "crossbow bolt",	"1d2", "1d10", CROSSBOW, ISMANY|ISMISL, 7, 1 },
    { "spear",		"1d8",  "1d6", NONE,  ISMISL, 50, 2 },
    { "trident",	"3d4",  "1d4", NONE,  0, 50, 25 },
    { "spetum",		"2d5",  "1d3", NONE,  0, 50, 12 },
    { "bardiche",	"3d3",  "1d2", NONE,  0, 125, 6 },
    { "pike",		"1d12", "1d8", NONE,  0, 80, 18 },
    { "bastard sword",	"2d7",  "1d2", NONE,  0, 100, 20 },
    { "halberd",	"2d6",  "1d3", NONE,  0, 175, 10 },
};


struct magic_item s_magic[MAXSCROLLS] = {
	{ "monster confusion",		 50, 200 },
	{ "magic mapping",		 55, 200 },
	{ "light",			 80, 100 },
	{ "hold monster",		 25, 200 },
	{ "sleep",			 40,  50 },
	{ "enchant armor",		 85, 175 },
	{ "identify",			215, 150 },
	{ "scare monster",		 40, 300 },
	{ "gold detection",		 30, 100 },
	{ "teleportation",		 70, 200 },
	{ "enchant weapon",		 90, 175 },
	{ "create monster",		 35,  75 },
	{ "remove curse",		 80, 100 },
	{ "aggravate monsters",		 10,  50 },
	{ "blank paper",		 10,  50 },
	{ "genocide",			  5, 350 },
	{ "item knowledge",		 15, 250 },
	{ "item protection",		 10, 250 },
	{ "demons curse",		  5,  25 },
	{ "transport",			 10, 100 },
	{ "enchantment",		  5, 300 },
	{ "gods blessing",		 10, 250 },
	{ "aquirement",			  5, 450 },
	{ "banishment",			  5,  25 },
	{ "recharge wand",		 15, 250 },
};

struct magic_item p_magic[MAXPOTIONS] = {
	{ "confusion",			 70,  50 },
	{ "paralysis",			 70,  50 },
	{ "poison",			 55,  50 },
	{ "gain strength",		130, 150 },
	{ "see invisible",		 25, 175 },
	{ "healing",			120, 130 },
	{ "monster detection",		 60, 120 },
	{ "magic detection",		 55, 105 },
	{ "raise level",		 25, 300 },
	{ "extra healing",		 55, 175 },
	{ "haste self",			 40, 200 },
	{ "restore strength",		130, 200 },
	{ "blindness",			 25,  50 },
	{ "thirst quenching",		 10,  50 },
	{ "increase dexterity",		 50, 175 },
	{ "etherealness",		 20, 150 },
	{ "increase wisdom",		 35, 175 },
	{ "regeneration",		 10, 175 },
	{ "super ability",		  5, 500 },
	{ "decrepedness",		  5,  25 },
	{ "invincibility",		  5, 500 },
};

struct magic_item r_magic[MAXRINGS] = {
	{ "protection",			 70, 200 },
	{ "strength",			 70, 200 },
	{ "sustain strength",		 50, 250 },
	{ "searching",			 70, 150 },
	{ "see invisible",		 80, 175 },
	{ "adornment",			 10, 100 },
	{ "aggravate monster",		 60, 100 },
	{ "agility",			 75, 250 },
	{ "increase damage",		 60, 250 },
	{ "regeneration",		 40, 250 },
	{ "digestion",			 60, 225 },
	{ "teleportation",		 60, 100 },
	{ "stealth",			 75, 200 },
	{ "speed",			 40, 225 },
	{ "find traps",			 20, 200 },
	{ "delusion",			 20, 100 },
	{ "sustain ability",		 10, 450 },
	{ "blindness",			 10,  50 },
	{ "lethargy",			 15,  75 },
	{ "ogre strength",		 10, 350 },
	{ "enfeeblement",		  5,  25 },
	{ "burden",			 10,  50 },
	{ "illumination",		 15, 100 },
	{ "fire protection",		  5, 225 },
	{ "wisdom",			 25, 200 },
	{ "dexterity",			 35, 200 },
};

struct magic_item ws_magic[MAXSTICKS] = {
	{ "light",			 95, 120 },
	{ "striking",			 75, 115 },
	{ "lightning",			 30, 200 },
	{ "fire",			 30, 200 },
	{ "cold",			 30, 200 },
	{ "polymorph",			 95, 210 },
	{ "magic missile",		 70, 170 },
	{ "haste monster",		 80,  50 },
	{ "slow monster",		 90, 220 },
	{ "drain life",			 80, 210 },
	{ "nothing",			 10,  70 },
	{ "teleport away",		 55, 140 },
	{ "teleport to",		 50,  60 },
	{ "cancellation",		 55, 130 },
	{ "sap life",			 20,  50 },
	{ "curing",			 25, 250 },
	{ "pyromania",			 15,  25 },
	{ "annihilate monster",		  5, 750 },
	{ "paralyze monster",		 10, 650 },
	{ "food absorption",		 10,  75 },
	{ "regenerate monster",		 15,  25 },
	{ "hide monster",		 10,  50 },
	{ "anti-matter",		  5,  25 },
	{ "clone monster",		 10,  10 },
	{ "confuse monster",		 15, 150 },
	{ "degenerate monster",		 15, 150 },
};

WINDOW *cw;		/* what the hero sees */
WINDOW *hw;		/* utility window */
WINDOW *mw;		/* monster window */
