/*
 * This file contains misc functions for dealing with armor
 *
 * @(#)armor.c	7.0	(Bell Labs)	10/08/82
 */

#include "rogue.h"
#include "rogue.ext"

/*
 * wear:
 *	The player wants to wear something, so let the hero try
 */
wear()
{
    struct linked_list *item;
    struct object *obj;

	if (cur_armor != NULL) {
		msg("You are already wearing some");
		after = FALSE;
		return;
	}
	if ((item = get_item("wear", ARMOR)) == NULL)
		return;
	obj = OBJPTR(item);
	if (obj->o_type != ARMOR) {
		msg("You can't wear that.");
		return;
	}
	waste_time();
	msg("Wearing %s.", armors[obj->o_which].a_name);
	cur_armor = obj;
	setoflg(obj,ISKNOW);
	nochange = FALSE;
}


/*
 * take_off:
 *	Get the armor off of the players back
 */
take_off()
{
	reg struct object *obj;

	if ((obj = cur_armor) == NULL) {
		msg("Not wearing any armor");
		return;
	}
	if (!dropcheck(cur_armor))
		return;
	cur_armor = NULL;
	msg("Was wearing %c) %s",pack_char(obj),inv_name(obj,TRUE));
	nochange = FALSE;
}


/*
 * waste_time:
 *	Do nothing but let other things happen
 */
waste_time()
{
	if (inwhgt)		/* if from wghtchk, then done */
	     return;
	do_daemons(BEFORE);
	do_fuses(BEFORE);
	do_daemons(AFTER);
	do_fuses(AFTER);
}
