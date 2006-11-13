/*
 * All the fighting gets done here
 *
 * @(#)fight.c	7.0	(Bell Labs) 	10/08/82
 */

#include <ctype.h>
#include "rogue.h"
#include "rogue_ext.h"


long e_levels[] = {
    10L,20L,40L,80L,160L,320L,640L,1280L,2560L,5120L,10240L,20480L,
    40920L, 81920L, 163840L, 327680L, 655360L, 1310720L, 2621440L, 0L };

/*
 * fight:
 *	The player attacks the monster.
 */

static char fbuf[6];


extern int look (NCURSES_BOOL wakeup);
extern void msg (const char *fmt, ...);
extern int runto (struct coord *runner, struct coord *spot);
extern int mon_index (char whichmon);
int roll_em (struct stats *att, struct stats *def, struct object *weap, NCURSES_BOOL hurl);
int thunk (struct object *weap, char *mname);
int hit (char *er);
int killed (struct linked_list *item, NCURSES_BOOL pr);
int bounce (struct object *weap, char *mname);
int miss (char *er);
extern int death (char monst);
extern int o_on (struct object *what, int bit);
int save (int which);
extern int iswearing (int ring);
extern int chg_abil (register int what, register int amt, register int how);
extern int chg_hpt (int howmany, NCURSES_BOOL alsomax, char whichmon);
extern int roll (int number, int sides);
int removelist (struct coord *mp, struct linked_list *item);
int is_magic (struct object *obj);
extern int _detach (register struct linked_list **list, register struct linked_list *item);
extern int discard (register struct linked_list *item);
extern int updpack (int getmax);
extern int fuse (int (*func) (/* ??? */), int arg, int time, int type);
extern int lengthen (int (*func) (/* ??? */), int xtime);
extern int getpcon (int econ);
extern int isring (int hand, int ring);
extern int getpdex (int edex, NCURSES_BOOL heave);
extern int str_plus (int str);
extern void debug (char *errstr);
extern int add_dam (int str);
extern int getpwis (int ewis);
extern void addmsg (const char *fmt, ...);
extern int fallpos (struct coord *pos, struct coord *newpos, NCURSES_BOOL passages);
extern int light (struct coord *cp);
extern int fall (struct linked_list *item, NCURSES_BOOL pr);

fight(struct coord *mp, char mn, struct object *weap, NCURSES_BOOL thrown)
{

    reg struct thing *tp;
    reg struct linked_list *item;
    reg bool did_hit = TRUE;

    /*
     * Find the monster we want to fight
     */

    if(pl_on(ISETHREAL))	/* cant fight when ethereal */
	return 0;

    if ((item = find_mons(mp->y, mp->x)) == NULL) {
	mvaddch(mp->y, mp->x, FLOOR);
	look(FALSE);
	msg("That monster must have been an illusion.");
	return 0;
    }
    tp = THINGPTR(item);
    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    quiet = 0;
    isfight = TRUE;
    runto(mp, &hero);
    /*
     * Let him know it was really a mimic (if it was one).
     */
    if(tp->t_type == 'M' && tp->t_disguise != 'M' && pl_off(ISBLIND)) {
	msg("Wait! That's a mimic!");
	tp->t_disguise = 'M';
	did_hit = thrown;
    }
    if (did_hit) {
	reg char *mname;

	did_hit = FALSE;
	if (pl_on(ISBLIND))
	    mname = "it";
	else
	    mname = monsters[mon_index(mn)].m_name;
	if (roll_em(&pstats, &tp->t_stats, weap, thrown)) {
	    did_hit = TRUE;
	    if (thrown)
		thunk(weap, mname);
	    else
		hit(NULL);
	    if (pl_on(CANHUH)) {
		msg("Your hands stop glowing red");
		msg("The %s appears confused.", mname);
		tp->t_flags |= ISHUH;
		player.t_flags &= ~CANHUH;
	    }
	    if (tp->t_stats.s_hpt <= 0)
		killed(item, TRUE);
	}
	else
	    if (thrown)
		bounce(weap, mname);
	    else
		miss(NULL);
    }
    count = 0;
    return did_hit;
}


/*
 * attack:
 *	The monster attacks the player
 */
attack(struct thing *mp)
{
    reg char *mname;
    char *prname(char *who, NCURSES_BOOL upper);

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */

    if(pl_on(ISETHREAL))	/* ehtereal players cant be hit */
	return;
    if(mp->t_flags & ISPARA)	/* paralyzed monsters */
	return;
    running = FALSE;
    quiet = 0;
    isfight = TRUE;
    if (mp->t_type == 'M' && pl_off(ISBLIND))
	mp->t_disguise = 'M';
    if (pl_on(ISBLIND))
	mname = "it";
    else
	mname = monsters[mon_index(mp->t_type)].m_name;
    if (roll_em(&mp->t_stats, &pstats, NULL, FALSE)) {
	if (pl_on(ISINVINC)) {
		msg("%s does not harm you",prname(mname,TRUE));
	} else {
	nochange = FALSE;
	if (mp->t_type != 'E')
	    hit(mname);
	if (pstats.s_hpt <= 0)
	    death(mp->t_type);	/* Bye bye life ... */
	if (off(*mp, ISCANC))
	    switch (mp->t_type) {
		case 'R':
		    /*
		     * If a rust monster hits, you lose armor
		     */
		    if(cur_armor != NULL && o_on(cur_armor,ISPROT))
			break;		/* if protected */
		    if(cur_armor != NULL && cur_armor->o_ac < 9 &&
			cur_armor->o_which != LEATHER &&
			cur_armor->o_which != PADDED) {
			msg("Your armor weakens");
			cur_armor->o_ac++;
		    }
		when 'E':
		    /*
		     * The gaze of the floating eye hypnotizes you
		     */
		    if (pl_off(ISBLIND) && no_command <= 0) {
			no_command = rnd(15) + 10;
			msg("You are transfixed");
		    }
		when 'Q':
			if(!save(VS_POISON) && !iswearing(R_SUSAB)) {
				chg_abil(DEX, -1, TRUE);
				msg("You feel less agile.");
			}
		when 'A':
		    if (!save(VS_POISON) && herostr() > MINABIL)
			if(!iswearing(R_SUSTSTR) && !iswearing(R_SUSAB)) {
			    chg_abil(STR, -1, TRUE);
			    msg("A sting has weakened you");
			}
			else
				msg("Sting has no effect");
		when 'W':
		    if (rnd(100) < 15 && !iswearing(R_SUSAB)) {
			if (pstats.s_exp <= 0)
			    death('W');		/* All levels gone */
			msg("You suddenly feel weaker.");
			if (--pstats.s_lvl == 0) {
				pstats.s_exp = 0;
				pstats.s_lvl = 1;
			}
			else
				pstats.s_exp = e_levels[pstats.s_lvl-1]+1;
			chg_hpt(-roll(1,10),TRUE,'W');
		    }
		when 'F':
		    player.t_flags |= ISHELD;
		    sprintf(fbuf, "%dd1", ++fung_hit);
		    monsters[mon_index('F')].m_stats.s_dmg = fbuf;
		when 'L': {
		    reg long lastpurse;

		    lastpurse = purse;
		    purse -= GOLDCALC;
		    if (!save(VS_MAGIC))
			purse -= GOLDCALC + GOLDCALC + GOLDCALC + GOLDCALC;
		    if (purse < 0)
			purse = 0;
		    if (purse != lastpurse)
			msg("Your purse feels lighter");
		    removelist(&mp->t_pos,find_mons(mp->t_pos.y,mp->t_pos.x));
		}
		when 'N': {
		    reg struct linked_list *list, *steal;
		    reg struct object *obj;
		    reg int nobj;

		    /*
		     * Nymph's steal a magic item, look through the pack
		     * and pick out one we like.
		     */
		    steal = NULL;
		    for(nobj=0,list = pack;list != NULL;list=next(list)) {
			obj = OBJPTR(list);
			if (obj != cur_armor && obj != cur_weapon &&
			    is_magic(obj) && rnd(++nobj) == 0)
				steal = list;
		    }
		    if (steal != NULL) {
			reg struct object *sobj;

			sobj = OBJPTR(steal);
			if (o_on(sobj,ISPROT))
			    break;
			removelist(&mp->t_pos,
			       find_mons(mp->t_pos.y,mp->t_pos.x));
			if (sobj->o_count > 1 && sobj->o_group == 0) {
				reg int oc;
				oc = sobj->o_count--;
				sobj->o_count = 1;
				msg("She stole %s!", inv_name(sobj, TRUE));
				sobj->o_count = oc;
				}
			else {
				msg("She stole %s!", inv_name(sobj, TRUE));
				detach(pack, steal);
				discard(steal);
				if(sobj == cur_ring[LEFT])
					cur_ring[LEFT] = NULL;
				else if(sobj == cur_ring[RIGHT])
					cur_ring[RIGHT] = NULL;
			}
			inpack--;
			updpack(FALSE);
		    }
		}
		when 'c': if(!save(VS_PETRIFICATION)) {
		    msg("Your body begins to solidify");
		    msg("You are turned to stone !!! --More--");
		    wait_for(' ');
		    death('c');
		}
		when 'd': if(rnd(100) < 50) player.t_flags |= ISHELD;
			if(!save(VS_POISON)) {
			    if(iswearing(R_SUSAB) || iswearing(R_SUSTSTR))
				msg("Sting has no effect.");
			    else {
				int fewer, ostr;
				fewer = roll(1,4);
				ostr = herostr();
				chg_abil(STR,-fewer,TRUE);
				if (herostr() < ostr) {
				    fewer = ostr - herostr();
				    fuse(rchg_str,fewer-1,10,AFTER);
				}
				msg("You feel weaker now.");
			    }
		}
		when 'g': if(!save(VS_BREATH)) {
		    if(iswearing(R_BREATH))
			msg("The fire has no effect.");
		    else {
			msg("You feel singed.");
			chg_hpt(-roll(1,8),FALSE,'g');
		    }
		}
		when 'h': if(!save(VS_BREATH)) {
		    if(iswearing(R_BREATH))
			msg("The flames do you no harm.");
		    else {
			msg("You are seared.");
			chg_hpt(-roll(1,4),FALSE,'h');
		    }
		}
		when 'p': if(!save(VS_POISON)) {
		    if (!iswearing(R_SUSTSTR) && !iswearing(R_SUSAB)) {
			msg("You are gnawed.");
			chg_abil(STR,-1,TRUE);
		    }
		}
		when 'u': if(!save(VS_POISON) && herostr() > MINABIL) {
		    if(!iswearing(R_SUSTSTR) && !iswearing(R_SUSAB)) {
			msg("You are bitten.");
			chg_abil(STR,-1,TRUE);
			fuse(rchg_str,1,roll(5,10),AFTER);
 		    }
		}
		when 'w': if(!save(VS_POISON) && !iswearing(R_SUSAB)) {
			msg("You feel devitalized.");
			chg_hpt(-1,TRUE,'w');
		}
		when 'i': if(!save(VS_PARALYZATION)) {
		    if(!iswearing(R_SUSAB)) {
			if(pl_on(ISSLOW))
			    lengthen(notslow,roll(3,10));
			else {
			    msg("You feel impaired.");
			    player.t_flags |= ISSLOW;
			    fuse(notslow,TRUE,roll(5,10),AFTER);
			}
		    }
		}
		otherwise:
		    break;
	    }
	}
    }
    else if (mp->t_type != 'E') {
	if (mp->t_type == 'F') {
	    pstats.s_hpt -= fung_hit;
	    if (pstats.s_hpt <= 0)
		death(mp->t_type);
	}
	miss(mname);
    }
    /*
     * Check to see if this is a regenerating monster and let it heal if
     * it is.
     */

    if (on(*mp, ISREGEN) && rnd(100) < 40)
	mp->t_stats.s_hpt++;
    flushout();			/* flush type ahead */
    count = 0;
}


/*
 * swing:
 *	returns true if the swing hits
 */
swing(int at_lvl, int op_arm, int wplus)
{
    reg int res = rnd(20)+1;
    reg int need = (21 - at_lvl) - op_arm;

    return (res + wplus >= need);
}


/*
 * check_level:
 *	Check to see if the guy has gone up a level.
 */
check_level(void)
{
    reg int i, add;

    for (i = 0; e_levels[i] != 0; i++)
	if (e_levels[i] > pstats.s_exp)
	    break;
    i++;
    if (i > pstats.s_lvl) {
	add = roll(i-pstats.s_lvl,10) + getpcon(pstats.s_ef.a_con);
	max_hp += add;
	if ((pstats.s_hpt += add) > max_hp)
	    pstats.s_hpt = max_hp;
	msg("Welcome to level %d", i);
    }
    pstats.s_lvl = i;
}


/*
 * roll_em:
 *	Roll several attacks
 */
roll_em(struct stats *att, struct stats *def, struct object *weap, NCURSES_BOOL hurl)
{
    reg char *cp;
    reg int ndice, nsides, def_arm;
    reg bool did_hit = FALSE;
    reg int prop_hplus, prop_dplus;
    char *mindex(char *cp, char c);

    prop_hplus = prop_dplus = 0;
    if (weap == NULL) {
	cp = att->s_dmg;
    }
    else if (hurl) {
	if (o_on(weap,ISMISL) && cur_weapon != NULL &&
	  cur_weapon->o_which == weap->o_launch) {
	    cp = weap->o_hurldmg;
	    prop_hplus = cur_weapon->o_hplus;
	    prop_dplus = cur_weapon->o_dplus;
	}
	else
	    cp = (o_on(weap,ISMISL) ? weap->o_damage : weap->o_hurldmg);
    }
    else {
	cp = weap->o_damage;
	/*
	 * Drain a staff of striking
	 */
	if (weap->o_type == STICK && weap->o_which == WS_HIT
	    && weap->o_charges == 0) {
		weap->o_damage = "0d0";
		weap->o_hplus = weap->o_dplus = 0;
	}
    }
    for (;;) {
	int damage;
	int hplus = prop_hplus + (weap == NULL ? 0 : weap->o_hplus);
	int dplus = prop_dplus + (weap == NULL ? 0 : weap->o_dplus);

	if (att == &pstats && weap == cur_weapon) {
	    if (isring(LEFT, R_ADDDAM))
		dplus += cur_ring[LEFT]->o_ac;
	    else if (isring(LEFT, R_ADDHIT))
		hplus += cur_ring[LEFT]->o_ac;
	    if (isring(RIGHT, R_ADDDAM))
		dplus += cur_ring[RIGHT]->o_ac;
	    else if (isring(RIGHT, R_ADDHIT))
		hplus += cur_ring[RIGHT]->o_ac;
	}
	ndice = atoi(cp);
	if ((cp = mindex(cp, 'd')) == NULL)
	    break;
	nsides = atoi(++cp);
	if (def == &pstats) {
	    if (cur_armor != NULL)
		def_arm = cur_armor->o_ac;
	    else
		def_arm = def->s_arm;
	    if (isring(LEFT, R_PROTECT))
		def_arm -= cur_ring[LEFT]->o_ac;
	    if (isring(RIGHT, R_PROTECT))
		def_arm -= cur_ring[RIGHT]->o_ac;
	}
	else
	    def_arm = def->s_arm;
	if(hurl)
	    hplus += getpdex(att->s_ef.a_dex,TRUE);
	if (swing(att->s_lvl, def_arm + getpdex(def->s_ef.a_dex,FALSE),
	  hplus + str_plus(att->s_ef.a_str))) {
	    reg int proll;

	    proll = roll(ndice, nsides);
	    if (ndice + nsides > 0 && proll < 1) {
		sprintf(errbuf,"Damage for %dd%d came out %d.",
		   ndice, nsides, proll);
		debug(errbuf);
	    }
	    damage = dplus + proll + add_dam(att->s_ef.a_str);
	    if (pl_off(ISINVINC) || def != &pstats)
		def->s_hpt = def->s_hpt - max(0, damage);
	    did_hit = TRUE;
	}
	if ((cp = mindex(cp, '/')) == NULL)
	    break;
	cp++;
    }
    return did_hit;
}


/*
 * mindex:
 *	Look for char 'c' in string pointed to by 'cp'
 */
char *
mindex(char *cp, char c)
{
	reg int i;

	for (i=0;i<3;i++) {
		if (*cp != c)  cp++;
	}
 	if (*cp == c) return (cp);
	else return (NULL);
}


/*
 * prname:
 *	The print name of a combatant
 */
char *
prname(char *who, NCURSES_BOOL upper)
{
    static char tbuf[LINLEN];

    *tbuf = '\0';
    if (who == 0)
	strcpy(tbuf, "you"); 
    else if (pl_on(ISBLIND))
	strcpy(tbuf, "it");
    else {
	strcpy(tbuf, "the ");
	strcat(tbuf, who);
    }
    if (upper)
	*tbuf = toupper(*tbuf);
    return tbuf;
}

/*
 * hit:
 *	Print a message to indicate a succesful hit
 */
hit(char *er)
{
    msg("%s hit.",prname(er, TRUE));
}


/*
 * miss:
 *	Print a message to indicate a poor swing
 */
miss(char *er)
{
    msg("%s miss%s.",prname(er, TRUE),(er == 0 ? "":"es"));
}


/*
 * save_throw:
 *	See if a creature saves against something
 */
save_throw(int which, struct thing *tp)
{
    reg int need;

    need = 14 + which - (tp->t_stats.s_lvl / 2) -
	   getpwis(tp->t_stats.s_ef.a_wis);
    return (roll(1, 20) >= need);
}


/*
 * save:
 *	See if he saves against various nasty things
 */
save(int which)
{
    return save_throw(which, &player);
}

/*
 * raise_level:
 *	The guy just magically went up a level.
 */
raise_level(void)
{
	pstats.s_exp = e_levels[pstats.s_lvl-1] + 1L;
	check_level();
}


/*
 * thunk:
 *	A missile hits a monster
 */
thunk(struct object *weap, char *mname)
{
	if(weap->o_type == WEAPON)
	    msg("The %s hits the %s", weaps[weap->o_which].w_name,mname);
	else
	    msg("You hit the %s.", mname);
}


/*
 * bounce:
 *	A missile misses a monster
 */
bounce(struct object *weap, char *mname)
{
    if (weap->o_type == WEAPON)
	msg("The %s misses the %s", weaps[weap->o_which].w_name,mname);
    else
	msg("You missed the %s.", mname);
}


/*
 * remove:
 *	Remove a monster from the screen
 */
removelist(struct coord *mp, struct linked_list *item)
{
    mvwaddch(mw, mp->y, mp->x, ' ');
    mvwaddch(cw, mp->y, mp->x, (THINGPTR(item))->t_oldch);
    detach(mlist, item);
    discard(item);
}


/*
 * is_magic:
 *	Returns true if an object radiates magic
 */
is_magic(struct object *obj)
{
    switch (obj->o_type) {
	case ARMOR:
	    return obj->o_ac != armors[obj->o_which].a_class;
	case WEAPON:
	    return obj->o_hplus != 0 || obj->o_dplus != 0;
	case POTION:
	case SCROLL:
	case STICK:
	case RING:
	case AMULET:
	    return TRUE;
    }
    return FALSE;
}


/*
 * killed:
 *	Called to put a monster to death
 */
killed(struct linked_list *item, NCURSES_BOOL pr)
{
    reg struct thing *tp;
    reg struct linked_list *pitem, *nexti;
    int notfight(int fromfuse);

    nochange = FALSE;
    tp = THINGPTR(item);
    if (pr) {
	addmsg("Defeated ");
	if (pl_on(ISBLIND))
	    msg("it.");
	else
	    msg("%s.", monsters[mon_index(tp->t_type)].m_name);
    }
    pstats.s_exp += tp->t_stats.s_exp;
    fuse(notfight,TRUE,1,AFTER);		/* 1 turn delay */
    /*
     * Do adjustments if he went up a level
     */
    check_level();
    /*
     * If the monster was a violet fungi, un-hold him
     */
    switch (tp->t_type) {
	case 'F':
	case 'd':
	    player.t_flags &= ~ISHELD;
	    if (tp->t_type == 'F') {
		fung_hit = 0;
		monsters[mon_index('F')].m_stats.s_dmg = "000d0";
	    }
	when 'L': {
	    reg struct room *rp;

	    if ((rp = roomin(&tp->t_pos)) == NULL)
		break;
	    if(rp->r_goldval != 0 ||
	      fallpos(&tp->t_pos,&rp->r_gold,FALSE)) {
		rp->r_goldval += GOLDCALC;
		if (!save_throw(VS_MAGIC,tp))
		    rp->r_goldval += GOLDCALC + GOLDCALC + GOLDCALC
				   + GOLDCALC + GOLDCALC;
		mvwaddch(stdscr, rp->r_gold.y, rp->r_gold.x, GOLD);
		if (!(rp->r_flags & ISDARK)) {
		    light(&hero);
		    mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
	    }
	}
    }
    /*
     * Empty the monsters pack
     */
    pitem = tp->t_pack;
    while (pitem != NULL) {
	reg struct object *obj;

	nexti = next(tp->t_pack);
	obj = OBJPTR(pitem);
	obj->o_pos = tp->t_pos;
	detach(tp->t_pack, pitem);
	fall(pitem, FALSE);
	pitem = nexti;
    }
    /*
     * Get rid of the monster.
     */
    removelist(&tp->t_pos, item);
}
