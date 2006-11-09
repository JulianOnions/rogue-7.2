/*
 * Rogue definitions and variable declarations
 *
 * @(#)rogue.h	7.0	(Bell Labs)	10/08/82
 */

#include "curses.h"

#undef NULL
#define NULL	0
#define reg

/*
 * Maximum number of different things
 */

#define TYPETRAPS	8	/* max types of traps */
#define MAXROOMS	9	/* max rooms per level */
#define MAXTHINGS	9	/* max things on each level */
#define MAXOBJ		9	/* max goodies on each level */	
#define MAXPACK		23	/* max things this hero can carry */
#define MAXTRAPS	10	/* max traps per level */
#define MAXMONS		52	/* max available monsters */
#define MONRANGE	20	/* max # of monsters avail each level */
#define AMLEVEL		30	/* earliest level that amulet can appear */
#define MAXPURCH	4	/* max purchases in trading post */
#define MINABIL		3	/* minimum for any ability */
#define MAXSTR		24	/* maximum strength */
#define MAXOTHER	18	/* maximum wis, dex, con */

#define	NUMTHINGS	8	/* types of goodies for hero */
#define TYP_POTION	0
#define TYP_SCROLL	1
#define TYP_FOOD	2
#define TYP_WEAPON	3
#define TYP_ARMOR	4
#define TYP_RING	5
#define TYP_STICK	6
#define TYP_AMULET	7

#define FROMRING	2
/*
 * return values for get functions
 */

#define	NORM	0		/* normal exit */
#define	QUIT	1		/* quit option setting */
#define	MINUS	2		/* back up one option */


/*
 * All the fun defines
 */
#define next(ptr)	(*ptr).l_next
#define prev(ptr)	(*ptr).l_prev
#define ldata(ptr)	(*ptr).l_data
#define OBJPTR(what)	(struct object *)((*what).l_data)
#define THINGPTR(what)	(struct thing *)((*what).l_data)

#define inroom(rp, cp) (\
	(cp)->x <= (rp)->r_pos.x + ((rp)->r_max.x - 1) && \
	(rp)->r_pos.x <= (cp)->x && (cp)->y <= (rp)->r_pos.y + \
	((rp)->r_max.y - 1) && (rp)->r_pos.y <= (cp)->y)

#define unc(cp) (cp).y, (cp).x
#define cmov(xy) move((xy).y, (xy).x)
#define DISTANCE(y1,x1,y2,x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define when break;case
#define otherwise break;default
#define until(expr) while(!(expr))

#define ce(a, b) ((a).x == (b).x && (a).y == (b).y)
#define draw(window) wrefresh(window)

#define hero player.t_pos
#define pstats player.t_stats
#define pack player.t_pack

#define herowis() (getpwis(pstats.s_ef.a_wis))
#define herodex() (getpdex(pstats.s_ef.a_dex,FALSE))
#define herostr() (pstats.s_ef.a_str)
#define herocon() (pstats.s_ef.a_con)

#define attach(a,b) _attach(&a,b)
#define detach(a,b) _detach(&a,b)
#define free_list(a) _free_list(&a)
#define max(a, b) ((a) > (b) ? (a) : (b))

#define on(thing, flag) (((thing).t_flags & flag) != 0)
#define off(thing, flag) (((thing).t_flags & flag) == 0)

#ifndef BSD4_2
#define CTRL(ch) (ch & 0x1F)	/* 4.2 defines this in <sys/ttychars.h> */
#endif				/* which is included by <curses.h> */

#define ALLOC(x) malloc((unsigned int) x)
#define FREE(x) cfree((char *) x)
#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)
#define GOLDCALC (rnd(50 + 10 * level) + 2)
#define ISMULT(type) (type == POTION || type == SCROLL || type == FOOD)

#define newgrp() ++group
#define o_charges o_ac


/*
 * Things that appear on the screens
 */
#define PASSAGE		'#'
#define DOOR		'+'
#define FLOOR		'.'
#define PLAYER		'@'
#define POST		'^'
#define TRAPDOOR	'>'
#define ARROWTRAP	'{'
#define SLEEPTRAP	'$'
#define BEARTRAP	'}'
#define TELTRAP		'~'
#define DARTTRAP	'`'
#define POOL		'"'
#define SECRETDOOR	'&'
#define STAIRS		'%'
#define GOLD		'*'
#define POTION		'!'
#define SCROLL		'?'
#define MAGIC		'$'
#define FOOD		':'
#define WEAPON		')'
#define ARMOR		']'
#define AMULET		','
#define RING		'='
#define STICK		'/'
#define CALLABLE	-1


/*
 *	stuff to do with encumberence 
 */
#define NORMENCB 1500		/* normal encumberence */
#define SOMTHERE 5		/* something is in the way for dropping */
#define CANTDROP 6		/* cant drop it cause its cursed */
#define F_OK	 0		/* have plenty of food in stomach */
#define F_HUNGRY 1		/* player is hungry */
#define F_WEAK	 2		/* weak from lack of food */
#define F_FAINT	 3		/* fainting from lack of food */


/*
 * Various constants
 */
#define	PASSWD		"UWS1yX662sMUw"
#define SALT		"UW"
#define BEARTIME	3
#define SLEEPTIME	5
#define HEALTIME	30
#define HOLDTIME	2
#define STPOS		0
#define WANDERTIME	70
#define BEFORE		1
#define AFTER		2
#define HUHDURATION	20
#define SEEDURATION	850
#define HUNGERTIME	1300
#define MORETIME	150
#define STOMACHSIZE	2000
#define ESCAPE		27
#define LEFT		0
#define RIGHT		1
#define BOLT_LENGTH	6

#define STR		1
#define DEX		2
#define CON		3
#define WIS		4

/*
 * Save against things
 */
#define VS_POISON		00
#define VS_PARALYZATION		00
#define VS_DEATH		00
#define VS_PETRIFICATION	01
#define VS_BREATH		02
#define VS_MAGIC		03


/*
 * Various flag bits
 */
#define ISDARK		0000001		/* room is dark */
#define ISGONE		0000002		/* room is gone */
#define ISTREAS		0000004		/* room is a treasure room */

#define ISCURSED	0000001		/* object is cursed */
#define ISKNOW 		0000002		/* object is known */
#define ISPOST 		0000004		/* object is in a trading post */
#define ISPROT		0000020		/* object is protected somehow */
#define ISMISL 		0020000		/* object normally thrown in attacks */
#define ISMANY 		0040000		/* objects are found in a group (> 1) */

#define ISFOUND		0000010		/* trap is found */

#define ISSTUCK		0000001L	/* monster can't run (violet fungi) */
#define ISBLIND		0000001L	/* hero is blind */
#define ISPARA 		0000002L	/* monster is paralyzed */
#define ISRUN		0000004L	/* Hero & monsters are running */
#define ISINVINC	0000010L	/* player is invincible */
#define ISINVIS		0000020L	/* monster is invisible */
#define ISMEAN 		0000040L	/* monster is mean */
#define ISGREED		0000100L	/* monster is greedy */
#define ISBLOCK		0000200L	/* monster blocks paths */
#define ISHELD 		0000400L	/* hero is held fast */
#define ISHUH  		0001000L	/* hero | monster is confused */
#define ISREGEN		0002000L	/* monster is regenerative */
#define CANHUH 		0004000L	/* hero can confuse monsters */
#define CANSEE 		0010000L	/* hero can see invisible monsters */
#define ISCANC		0020000L	/* monsters special attacks canceled */
#define ISSLOW		0040000L	/* hero | monster is slow */
#define ISHASTE		0100000L	/* hero | monster is fast */
#define ISETHREAL	0200000L	/* hero is thin as air */

#define NONE		100		/* equal to 'd' (used for weaps) */

/*
 * Potion types
 */
#define P_CONFUSE	0		/* confusion */
#define P_PARALYZE	1		/* paralysis */
#define P_POISON	2		/* poison */
#define P_STRENGTH	3		/* gain strength */
#define P_SEEINVIS	4		/* see invisible */
#define P_HEALING	5		/* healing */
#define P_MFIND		6		/* monster detection */
#define P_TFIND		7		/* magic detection */
#define P_RAISE		8		/* raise level */
#define P_XHEAL		9		/* extra healing */
#define P_HASTE		10		/* haste self */
#define P_RESTORE	11		/* restore strength */
#define P_BLIND		12		/* blindness */
#define P_NOP		13		/* thirst quenching */
#define P_DEX		14		/* increase dexterity */
#define P_ETH		15		/* etherealness */
#define P_SMART		16		/* wisdom */
#define P_REGEN		17		/* regeneration */
#define P_SUPHERO	18		/* super ability */
#define P_DECREP	19		/* decrepedness */
#define P_INVINC	20		/* invicibility */
#define MAXPOTIONS	21		/* types of potions */


/*
 * Scroll types
 */
#define S_CONFUSE	0		/* monster confusion */
#define S_MAP		1		/* magic mapping */
#define S_LIGHT		2		/* light */
#define S_HOLD		3		/* hold monster */
#define S_SLEEP		4		/* sleep */
#define S_ARMOR		5		/* enchant armor */
#define S_IDENT		6		/* identify */
#define S_SCARE		7		/* scare monster */
#define S_GFIND		8		/* gold detection */
#define S_TELEP		9		/* teleportation */
#define S_ENCH		10		/* enchant weapon */
#define S_CREATE	11		/* create monster */
#define S_REMOVE	12		/* remove curse */
#define S_AGGR		13		/* aggravate monster */
#define S_NOP		14		/* blank paper */
#define S_GENOCIDE	15		/* genocide */
#define S_KNOWALL	16		/* item knowledge */
#define S_PROTECT	17		/* item protection */
#define S_DCURSE	18		/* demons curse */
#define S_DLEVEL	19		/* transport */
#define S_ALLENCH	20		/* enchantment */
#define S_BLESS		21		/* gods blessing */
#define S_MAKEIT	22		/* aquirement */
#define S_BAN		23		/* banishment */
#define S_CWAND		24		/* charge wands */
#define MAXSCROLLS	25		/* types of scrolls */


/*
 * Weapon types
 */
#define MACE		0		/* mace */
#define SWORD		1		/* long sword */
#define BOW		2		/* short bow */
#define ARROW		3		/* arrow */
#define DAGGER		4		/* dagger */
#define ROCK		5		/* rocks */
#define TWOSWORD	6		/* two-handed sword */
#define SLING		7		/* sling */
#define DART		8		/* darts */
#define CROSSBOW	9		/* crossbow */
#define BOLT		10		/* crossbow bolt */
#define SPEAR		11		/* spear */
#define TRIDENT		12		/* trident */
#define SPETUM		13		/* spetum */
#define BARDICHE	14 		/* bardiche */
#define PIKE		15		/* pike */
#define BASWORD		16		/* bastard sword */
#define HALBERD		17		/* halberd */
#define MAXWEAPONS	18		/* types of weapons */


/*
 * Armor types
 */
#define LEATHER		0		/* leather */
#define RINGMAIL	1		/* ring */
#define STUDDED		2		/* studded leather */
#define SCALE		3		/* scale */
#define PADDED		4		/* padded */
#define CHAIN		5		/* chain */
#define SPLINT		6		/* splint */
#define BANDED		7		/* banded */
#define PLATEMAIL	8		/* plate mail */
#define PLATEARMOR	9		/* plate armor */
#define MAXARMORS	10		/* types of armor */


/*
 * Ring types
 */
#define R_PROTECT	0		/* protection */
#define R_ADDSTR	1		/* add strength */
#define R_SUSTSTR	2		/* sustain strength */
#define R_SEARCH	3		/* searching */
#define R_SEEINVIS	4		/* see invisible */
#define R_NOP		5		/* adornment */
#define R_AGGR		6		/* aggravate monster */
#define R_ADDHIT	7		/* agility */
#define R_ADDDAM	8		/* increase damage */
#define R_REGEN		9		/* regeneration */
#define R_DIGEST	10		/* slow digestion */
#define R_TELEPORT	11		/* teleportation */
#define R_STEALTH	12		/* stealth */
#define R_SPEED		13		/* speed */
#define R_FTRAPS	14		/* find traps */
#define R_DELUS		15		/* delusion */
#define R_SUSAB		16		/* sustain ability */
#define R_BLIND		17		/* blindness */
#define R_SLOW		18		/* lethargy */
#define R_GIANT		19		/* ogre strength */
#define R_SAPEM		20		/* enfeeblement */
#define R_HEAVY		21		/* burden */
#define R_LIGHT		22		/* illumination */
#define R_BREATH	23		/* fire protection */
#define R_KNOW		24		/* wisdom */
#define R_DEX		25		/* dexterity */
#define MAXRINGS	26		/* types of rings */


/*
 * Rod/Wand/Staff types
 */

#define WS_LIGHT	0		/* light */
#define WS_HIT		1		/* striking */
#define WS_ELECT	2		/* lightning */
#define WS_FIRE		3		/* fire */
#define WS_COLD		4		/* cold */
#define WS_POLYMORPH	5		/* polymorph */
#define WS_MISSILE	6		/* magic missile */
#define WS_HASTE_M	7		/* haste monster */
#define WS_SLOW_M	8		/* slow monster */
#define WS_DRAIN	9		/* drain life */
#define WS_NOP		10		/* nothing */
#define WS_TELAWAY	11		/* teleport away */
#define WS_TELTO	12		/* teleport to */
#define WS_CANCEL	13		/* cancellation */
#define WS_SAPLIFE	14		/* sap life */
#define WS_CURE		15		/* curing */
#define WS_PYRO		16		/* pyromania */
#define WS_ANNIH	17		/* annihilate monster */
#define WS_PARZ		18		/* paralyze monster */
#define WS_HUNGER	19		/* food absorption */
#define WS_MREG		20		/* regenerate monster */
#define WS_MINVIS	21		/* hide monster */
#define WS_ANTIM	22		/* anti-matter */
#define WS_MOREMON	23		/* clone monster */
#define WS_CONFMON	24		/* confuse monster */
#define WS_MDEG		25		/* degenerate monster */
#define MAXSTICKS	26		/* max types of sticks */

#define MAXAMULETS	1		/* types of amulets */
#define MAXFOODS	1		/* types of food */


/*
 * Now we define the structures and types
 */


/*
 * Help list
 */
struct h_list {
	char h_ch;
	char *h_desc;
};


/*
 * Coordinate data type
 */
struct coord {
	int x;			/* column position */
	int y;			/* row position */
};

struct monlev {
	int l_lev;		/* lowest level for a monster */
	int h_lev;		/* highest level for a monster */
	bool d_wand;		/* TRUE if monster wanders */
};

/*
 * Linked list data type
 */
struct linked_list {
	struct linked_list *l_next;
	struct linked_list *l_prev;
	char *l_data;			/* Various structure pointers */
};


/*
 * Stuff about magic items
 */
#define mi_wght	mi_worth		/* weight of magic item */

struct magic_item {
	char *mi_name;			/* name of item */
	int mi_prob;			/* probability of getting item */
	int mi_worth;			/* worth of item */
};

/*
 * armor structure 
 */
struct init_armor {
	char *a_name;		/* name of armor */
	int  a_prob;		/* chance of getting armor */
	int  a_class;		/* normal armor class */
	int  a_worth;		/* worth of armor */
	int  a_wght;		/* weight of armor */
};

/*
 * weapon structure
 */
struct init_weps {
    char *w_name;		/* name of weapon */
    char *w_dam;		/* hit damage */
    char *w_hrl;		/* hurl damage */
    char w_launch;		/* need to launch it */
    int  w_flags;		/* flags */
    int  w_wght;		/* weight of weapon */
    int  w_worth;		/* worth of this weapon */
};


/*
 * Room structure
 */
struct room {
	struct coord r_pos;		/* Upper left corner */
	struct coord r_max;		/* Size of room */
	struct coord r_gold;		/* Where the gold is */
	int r_goldval;			/* How much the gold is worth */
	int r_flags;			/* Info about the room */
	int r_nexits;			/* Number of exits */
	struct coord r_exit[4];		/* Where the exits are */
};

/*
 * Array of all traps on this level
 */
struct trap {
	struct coord tr_pos;		/* Where trap is */
	char tr_type;			/* What kind of trap */
	int tr_flags;			/* Info about trap */
};

/*
 * structure for describing true abilities
 */
struct real {
	int a_str;			/* strength (3-24) */
	int a_dex;			/* dexterity (3-18) */
	int a_wis;			/* wisdom (3-18) */
	int a_con;			/* constitution (3-18) */
};

/*
 * Structure describing a fighting being
 */
struct stats {
	struct real s_re;		/* True ability */
	struct real s_ef;		/* Effective ability */
	long s_exp;			/* Experience */
	int s_lvl;			/* Level of mastery */
	int s_arm;			/* Armor class */
	int s_hpt;			/* Hit points */
	int s_pack;			/* current weight of his pack */
	int s_carry;			/* max weight he can carry */
	char *s_dmg;			/* String describing damage done */
};

/*
 * Structure for monsters and player
 */
struct thing {
	struct coord t_pos;		/* Position */
	bool t_turn;			/* If slow, is it a turn to move */
	char t_type;			/* What it is */
	char t_disguise;		/* What mimic looks like */
	char t_oldch;			/* Char that was where it was */
	struct coord *t_dest;		/* Where it is running to */
	long t_flags;			/* State word */
	struct stats t_stats;		/* Physical description */
	struct linked_list *t_pack;	/* What the thing is carrying */
};

/*
 * Array containing information on all the various types of mosnters
 */
struct monster {
	char *m_name;			/* What to call the monster */
	char m_show;			/* char that monster shows */
	short m_carry;			/* Probability of having an item */
	struct monlev m_lev;		/* level stuff */
	long m_flags;			/* Things about the monster */
	struct stats m_stats;		/* Initial stats */
};

/*
 * Structure for a thing that the rogue can carry
 */

struct object {
	int o_type;			/* What kind of object it is */
	struct coord o_pos;		/* Where it lives on the screen */
	char *o_text;			/* What it says if you read it */
	char o_launch;			/* What you need to launch it */
	char *o_damage;			/* Damage if used like sword */
	char *o_hurldmg;		/* Damage if thrown */
	int o_count;			/* Count for plural objects */
	int o_which;			/* Which object of a type it is */
	int o_hplus;			/* Plusses to hit */
	int o_dplus;			/* Plusses to damage */
	int o_ac;			/* Armor class or charges */
	int o_flags;			/* Information about objects */
	int o_group;			/* Group number for this object */
	int o_weight;			/* weight of this object */
};

extern WINDOW *cw, *hw, *mw;

#define LINLEN	80			/* length of buffers */

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

/* stty waits for output to finish & then flushes input */
#define flushout()	crmode();
