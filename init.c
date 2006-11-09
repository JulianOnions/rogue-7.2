/*
 * initializate various things
 *
 * @(#)init.c	7.0	(Bell Labs)	10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include "rogue.ext"

static char *rainbow[] = {
	"red",		"blue",		"green",	"yellow",
	"black",	"brown",	"orange",	"pink",
	"purple",	"grey",		"white",	"silver",
	"gold",		"violet",	"clear",	"vermilion",
	"ecru",		"turquoise",	"magenta",	"amber",
	"topaz",	"plaid",	"tan",		"tangerine",
	"aquamarine",	"scarlet",	"khaki",	"crimson",
	"indigo",	"beige",	"lavender",	"saffron",
};
#define NCOLORS (sizeof rainbow / (sizeof (char *)))

static char *sylls[] = {
	"a", "ab", "ag", "aks", "ala", "an", "ankh", "app", "arg", "arze",
	"ash", "ban", "bar", "bat", "bek", "bie", "bin", "bit", "bjor",
	"blu", "bot", "bu", "byt", "comp", "con", "cos", "cre", "dalf",
	"dan", "den", "do", "e", "eep", "el", "eng", "er", "ere", "erk",
	"esh", "evs", "fa", "fid", "for", "fri", "fu", "gan", "gar",
	"glen", "gop", "gre", "ha", "he", "hyd", "i", "ing", "ion", "ip",
	"ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
	"man", "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur",
	"nej", "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od",
	"ood", "org", "orn", "ox", "oxy", "pay", "pet", "ple", "plu", "po",
	"pot","prok","re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol",
	"sa", "san", "sat", "see", "sef", "seh", "shu", "ski", "sna",
	"sne", "snik", "sno", "so", "sol", "sri", "sta", "sun", "ta",
	"tab", "tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
	"ulk", "um", "un", "uni", "ur", "val", "viv", "vly", "vom", "wah",
	"wed", "werg", "wex", "whon", "wun", "xo", "y", "yot", "yu",
	"zant", "zap", "zeb", "zim", "zok", "zon", "zum",
};
#define NSYLS (sizeof sylls / (sizeof (char *)))

static char *stones[] = {
	"agate",		"alexandrite",		"amethyst",
	"azurite",		"carnelian",		"chrysoberyl",
	"chrysoprase",		"citrine",		"diamond",
	"emerald",		"garnet",		"hematite",
	"jacinth",		"jade",			"kryptonite",
	"lapus lazuli",		"malachite",		"moonstone",
	"obsidian",		"olivine",		"onyx",
	"opal",			"pearl",		"peridot",
	"quartz",		"rhodochrosite",	"ruby",
	"sapphire",		"sardonyx",		"serpintine",
	"spinel",		"tiger eye",		"topaz",
	"tourmaline",		"turquoise",
};
#define NSTONES (sizeof stones / (sizeof (char *)))

static char *wood[] = {
	"avocado wood",	"balsa",	"banyan",	"birch",
	"cedar",	"cherry",	"cinnibar",	"dogwood",
	"driftwood",	"ebony",	"eucalyptus",	"hemlock",
	"ironwood",	"mahogany",	"manzanita",	"maple",
	"oak",		"pine",		"redwood",	"rosewood",
	"teak",		"walnut",	"zebra wood", 	"persimmon wood",
};
#define NWOOD (sizeof wood / (sizeof (char *)))

static char *metal[] = {
	"aluminium",	"bone",		"brass",	"bronze",
	"copper",	"chromium",	"iron",		"lead",
	"magnesium",	"pewter",	"platinum",	"steel",
	"tin",		"titanium",	"zinc",
};
#define NMETAL (sizeof metal / (sizeof (char *)))


/*
 * init_things:
 *	Initialize the probabilities for types of things
 */
init_things()
{
	reg struct magic_item *mp;

	for (mp = &things[1]; mp < &things[NUMTHINGS]; mp++)
		mp->mi_prob += (mp-1)->mi_prob;
	badcheck("things", things, NUMTHINGS);
}


/*
 * init_colors:
 *	Initialize the potion color scheme for this time
 */
init_colors()
{
	reg int i;
	reg char **str;

	for (i = 0; i < MAXPOTIONS; i++) {
		do {
		    str = &rainbow[rnd(NCOLORS)];
		} until (*str != (char *)NULL);
		p_colors[i] = *str;
		p_know[i] = FALSE;
		p_guess[i] = (char *)NULL;
		*str = (char *)NULL;
		if (i > 0)
			p_magic[i].mi_prob += p_magic[i-1].mi_prob;
	}
	badcheck("potions", p_magic, MAXPOTIONS);
}


/*
 * init_names:
 *	Generate the names of the various scrolls
 */
init_names()
{
	reg int nsyl;
	reg char *cp, *sp;
	reg int i, nwords;

	for (i = 0; i < MAXSCROLLS; i++) {
		cp = prbuf;
		nwords = rnd(3)+1;
		while(nwords--)	{
		    nsyl = rnd(3)+2;
		    while(nsyl--) {
			sp = sylls[rnd(NSYLS)];
			while(*sp)
			    *cp++ = *sp++;
		    }
		    *cp++ = ' ';
		}
		*--cp = NULL;
		s_names[i] = new(strlen(prbuf)+1);
		s_know[i] = FALSE;
		s_guess[i] = (char *)NULL;
		strcpy(s_names[i], prbuf);
		if (i > 0)
			s_magic[i].mi_prob += s_magic[i-1].mi_prob;
	}
	badcheck("scrolls", s_magic, MAXSCROLLS);
}

/*
 * init_stones:
 *	Initialize the ring stone setting scheme for this time
 */

init_stones()
{
	reg int i;
	reg char **str;

	for (i = 0; i < MAXRINGS; i++) {
		do {
		    str = &stones[rnd(NSTONES)];
		} until (*str != (char *)NULL);
		r_stones[i] = *str;
		r_know[i] = FALSE;
		r_guess[i] = (char *)NULL;
		*str = (char *)NULL;
		if (i > 0)
			r_magic[i].mi_prob += r_magic[i-1].mi_prob;
	}
	badcheck("rings", r_magic, MAXRINGS);
}

/*
 * init_materials:
 *	Initialize the construction materials for wands and staffs
 */

init_materials()
{
	reg int i;
	reg char **str;

	for (i = 0; i < MAXSTICKS; i++) {
		do {
		    if (rnd(100) > 50) {
			str = &metal[rnd(NMETAL)];
			if (*str != (char *)NULL)
			    ws_type[i] = "wand";
		    }
		    else {
			str = &wood[rnd(NWOOD)];
			if (*str != (char *)NULL)
			    ws_type[i] = "staff";
		    }
		} until (*str != (char *)NULL);
		ws_made[i] = *str;
		ws_know[i] = FALSE;
		ws_guess[i] = (char *)NULL;
		*str = (char *)NULL;
		if (i > 0)
			ws_magic[i].mi_prob += ws_magic[i-1].mi_prob;
	}
	badcheck("sticks", ws_magic, MAXSTICKS);
}

badcheck(name, magic, bound)
char *name;
reg struct magic_item *magic;
reg int bound;
{
	reg struct magic_item *end;

	if (magic[bound - 1].mi_prob == 1000)
		return;
	printf("\nBad percentages for %s:\n", name);
	for (end = &magic[bound]; magic < end; magic++)
		printf("%3d%% %s\n", magic->mi_prob, magic->mi_name);
	printf("[hit RETURN to continue]");
	fflush(stdout);
	while (getchar() != '\n')
		continue;
}


/*
 * init_player:
 *	roll up the rogue
 */

init_player()
{
	pstats.s_lvl = 1;
	pstats.s_exp = 0L;
	max_hp = pstats.s_hpt = pinit();	/* hit points */
	pstats.s_re.a_str = pinit();		/* strength */
	pstats.s_re.a_dex = pinit();		/* dexterity */
	pstats.s_re.a_wis = pinit();		/* wisdom */
	pstats.s_re.a_con = pinit();		/* constitution */
	pstats.s_ef = pstats.s_re;		/* effective = real */
	pstats.s_dmg = "1d4";
	pstats.s_arm = 10;
	pstats.s_carry = totalenc();
	pstats.s_pack = 0;
	pack = NULL;				/* empty pack so far */
	max_stats = pstats;
}


/*
 * pinit:
 *	Returns the best 3 of 4 on a 6-sided die
 */
pinit()
{
	int best[4];
	reg int i, min, minind, dicetot;

	for (i = 0 ; i < 4 ; i++) {
		best[i] = roll(1,6);	/* populate array */
	}
	min = best[0];			/* assume that 1st entry */
	minind = 0;			/* is the lowest */
	for (i = 1 ; i < 4 ; i++) {	/* find the lowest */
		if (best[i] < min) {	/* if < minimum then update */
			min = best[i];
			minind = i;	/* point to lowest value */
		}
	}
	dicetot = 0;			/* start with nothing */
	for (i = 0 ; i < 4 ; i++) {
		if (i != minind)	/* if not minimum, then add it */
			dicetot += best[i];
	}
	return(dicetot);		/* all done */
}
