/*
 * Functions for dealing with potions
 *
 * @(#)potions.c	7.0	(Bell Labs)	10/8/82
 */

#include "rogue.h"
#include "rogue.ext"

/*
 * quaff:
 *	Let the hero drink a potion
 */
quaff()
{
    reg struct object *obj;
    reg struct linked_list *item, *titem;
    reg struct thing *th;
    reg int wh;
    char buf[LINLEN];

    /*
     * Make certain that it is somethings that we want to drink
     */

    if ((item = get_item("quaff", POTION)) == NULL)
	return;

    obj = OBJPTR(item);
    if (obj->o_type != POTION) {
	msg("That's undrinkable!");
	return;
    }
    wh = obj->o_which;

    del_pack(item);		/* get rid of it */

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch(wh) {
	case P_CONFUSE:
	    if (pl_on(ISINVINC))
		msg("You remain level-headed.");
	    else {
		chg_abil(WIS,-1,TRUE);		/* confuse his mind */
		if (pl_off(ISHUH)) {
		    msg("Wait, what's going on here. Huh? What? Who?");
		    if (pl_on(ISHUH))
			lengthen(unconfuse,rnd(8)+HUHDURATION);
		    else
			fuse(unconfuse,TRUE,rnd(8)+HUHDURATION,AFTER);
		    player.t_flags |= ISHUH;
		}
	    }
	    p_know[P_CONFUSE] = TRUE;
	when P_POISON:
	    if(pl_off(ISINVINC) && !iswearing(R_SUSTSTR) &&
	      !iswearing(R_SUSAB)) {
		chg_abil(CON,-1,TRUE);		
		chg_abil(STR,-(rnd(3)+1),TRUE);
		msg("You feel very sick now.");
	    }
	    else
		msg("You feel momentarily sick");
	    p_know[P_POISON] = TRUE;
	when P_HEALING:
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 4)) > max_hp)
		pstats.s_hpt = ++max_hp;
	    msg("You begin to feel better.");
	    if (!iswearing(R_SLOW))
		notslow(FALSE);		/* stop sluggishness (from imp) */
	    sight(FALSE);
	    p_know[P_HEALING] = TRUE;
	when P_STRENGTH:
	    msg("You feel stronger, now.  What bulging muscles!");
	    chg_abil(STR,1,TRUE);
	    p_know[P_STRENGTH] = TRUE;
	when P_MFIND:
	    /*
	     * Potion of monster detection - find all monters
	     */
	    if (mlist != NULL) {
		dispmons();
		mpos = 0;
		msg("You begin to sense the presence of monsters--More--");
		p_know[P_MFIND] = TRUE;
		wait_for(' ');
		msg("");		/* clear line */
	    }
	    else
		msg("You have a strange feeling for a moment, then it passes.");
	when P_TFIND:
	    /*
	     * Potion of magic detection.  Show the potions and scrolls
	     */
	    if (lvl_obj != NULL) {
		struct linked_list *mobj;
		struct object *tp;
		bool show;

		show = FALSE;
		wclear(hw);
		for (mobj = lvl_obj; mobj != NULL; mobj = next(mobj)) {
		    tp = OBJPTR(mobj);
		    if (is_magic(tp)) {
			show = TRUE;
			mvwaddch(hw, tp->o_pos.y, tp->o_pos.x, MAGIC);
		    }
		}
		for(titem = mlist; titem != NULL; titem = next(titem)) {
		    reg struct linked_list *pitem;

		    th = THINGPTR(titem);
		    for(pitem=th->t_pack;pitem!=NULL;pitem=next(pitem)) {
			if (is_magic(ldata(pitem))) {
			    show = TRUE;
			    mvwaddch(hw,th->t_pos.y, th->t_pos.x, MAGIC);
			}
		    }
		}
		if(show) {
		    msg("You begin to sense the presence of magic.");
		    overlay(hw,cw);
		    p_know[P_TFIND] = TRUE;
		    break;
		}
	    }
	   msg("You have a strange feeling for a moment, then it passes.");
	when P_PARALYZE:
	    if (pl_on(ISINVINC))
		msg("You feel numb for a moment.");
	    else {
		msg("You can't move.");
		no_command = HOLDTIME;
	    }
	    p_know[P_PARALYZE] = TRUE;
	when P_SEEINVIS: {
	    int invlen = roll(40,20);
	    msg("This potion tastes like %s juice.", fruit);
	    if (pl_off(CANSEE)) {
		player.t_flags |= CANSEE;
		fuse(unsee, TRUE, invlen, AFTER);
		light(&hero);
	    }
	    else
		lengthen(unsee, invlen);
	    sight(FALSE);
	}
	when P_RAISE:
	    msg("You suddenly feel much more skillful");
	    p_know[P_RAISE] = TRUE;
	    chg_abil(DEX,1,TRUE);		/* increase dexterity */
	    chg_abil(WIS,1,TRUE);		/* increase wisdom */
	    chg_abil(CON,1,TRUE);		/* increase constitution */
	    raise_level();
	when P_XHEAL:
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 8)) > max_hp)
		pstats.s_hpt = ++max_hp;
	    chg_abil(CON,1,TRUE);
	    msg("You begin to feel much better.");
	    p_know[P_XHEAL] = TRUE;
	    if (!iswearing(R_SLOW))
		notslow(FALSE);		/* stop sluggishness (from imp) */
	    player.t_flags &= ~ISHUH;	/* no more confusion */
	    extinguish(unconfuse);
	    sight(FALSE);
	when P_HASTE:
	    add_haste(TRUE);
	    msg("You feel yourself moving much faster.");
	    p_know[P_HASTE] = TRUE;
	when P_INVINC: {
	    int time = rnd(400) + 350;
	    msg("You feel invincible.");
	    if (player.t_flags & ISINVINC)
		lengthen(notinvinc,time);
	    else
		fuse(notinvinc,TRUE,time,AFTER);
	    player.t_flags |= ISINVINC;
	    p_know[P_INVINC] = TRUE;
	}
	when P_SMART:
	    msg("You feel more perceptive.");
	    p_know[P_SMART] = TRUE;
	    chg_abil(WIS,1,TRUE);
	when P_RESTORE:
	    msg("Hey, this tastes great. It make you feel warm all over");
	    pstats.s_re = max_stats.s_re;
	    pstats.s_ef = max_stats.s_re;
	    ringabil();				/* add in rings */
	    updpack(TRUE);			/* update weight */
	    p_know[P_RESTORE] = TRUE;
	when P_BLIND:
	    if (pl_on(ISINVINC))
		msg("The light dims for a moment.");
	    else {
		chg_abil(WIS,-1,TRUE);
		msg("A cloak of darkness falls around you.");
		if (pl_off(ISBLIND)) {
		    player.t_flags |= ISBLIND;
		    fuse(sight, TRUE, rnd(400) + 450, AFTER);
		    look(FALSE);
		}
	    }
	    p_know[P_BLIND] = TRUE;
	when P_ETH: {
	    int ethlen = roll(40,20);
	    msg("You feel more vaporous.");
	    if(pl_on(ISETHREAL))
		lengthen(noteth,ethlen);
	    else
		fuse(noteth,TRUE,ethlen, AFTER);
	    player.t_flags |= ISETHREAL;
	    p_know[P_ETH] = TRUE;
	}
	when P_NOP:
	    msg("This potion tastes extremely dull.");
	when P_DEX:
	    chg_abil(DEX,1,TRUE);		/* increase dexterity */
	    p_know[P_DEX] = TRUE;
	    msg("You feel much more agile.");
	when P_REGEN: {
	    int reglen = rnd(450) + 450;
	    if (pl_on(ISREGEN))
		lengthen(notregen,reglen);
	    else
		fuse(notregen,TRUE,reglen,AFTER);
	    player.t_flags |= ISREGEN;
	    msg("You feel yourself improved.");
	    p_know[P_REGEN] = TRUE;
	}
	when P_DECREP:
	case P_SUPHERO: {
	    int howmuch = rnd(3) + 2;
	    if (wh == P_DECREP) {
		if (iswearing(R_SUSAB) || pl_on(ISINVINC)) {
		    msg("You feel momentarily woozy.");
		    howmuch = 0;
		}
		else {
		    msg("You feel crippled.");
		    howmuch = -howmuch;
		    if (!iswearing(R_SUSTSTR))
			chg_abil(STR,howmuch,TRUE);
		}
	    }
	    else {
		msg("You feel invigorated.");
		chg_abil(STR,howmuch,TRUE);
	    }
	    chg_abil(CON,howmuch,TRUE);
	    chg_abil(DEX,howmuch,TRUE);
	    chg_abil(WIS,howmuch,TRUE);		/* change abilities */
	    p_know[wh] = TRUE;
	}
	otherwise:
	    msg("What an odd tasting potion!");
	    return;
    }
    nochange = FALSE;
    if (p_know[wh] && p_guess[wh]) {
	FREE(p_guess[wh]);
	p_guess[wh] = NULL;
    }
    else if(!p_know[wh] && p_guess[wh] == NULL) {
	buf[0] = NULL;
	msg("Call it: ");
	if (get_str(buf, cw) == NORM) {
	    p_guess[wh] = new(strlen(buf) + 1);
	    strcpy(p_guess[wh], buf);
	}
    }
}
