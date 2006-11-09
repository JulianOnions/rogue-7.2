/*
 * Functions for dealing with linked lists of goodies
 *
 * @(#)list.c	7.0	(Bell Labs)	10/08/82
 */

#include "rogue.h"
#include "rogue.ext"

/*
 * detach:
 *	Takes an item out of whatever linked list it might be in
 */

_detach(list, item)
register struct linked_list **list, *item;
{
    if (*list == item)
	*list = next(item);
    if (prev(item) != NULL) item->l_prev->l_next = next(item);
    if (next(item) != NULL) item->l_next->l_prev = prev(item);
    item->l_next = NULL;
    item->l_prev = NULL;
}

/*
 * _attach:
 *	add an item to the head of a list
 */

_attach(list, item)
register struct linked_list **list, *item;
{
    if (*list != NULL)
    {
	item->l_next = *list;
	(*list)->l_prev = item;
	item->l_prev = NULL;
    }
    else
    {
	item->l_next = NULL;
	item->l_prev = NULL;
    }

    *list = item;
}

/*
 * _free_list:
 *	Throw the whole blamed thing away
 */

_free_list(ptr)
register struct linked_list **ptr;
{
    register struct linked_list *item;

    while (*ptr != NULL){
	item = *ptr;
	*ptr = next(item);
	discard(item);
    }
}

/*
 * discard:
 *	free up an item
 */

discard(item)
register struct linked_list *item;
{
    FREE(item->l_data);
    FREE(item);
    total--;
}

/*
 * new_item
 *	get a new item with a specified size
 */

struct linked_list *
new_item(size)
int size;
{
    register struct linked_list *item;

    if ((item = (struct linked_list *) new(sizeof *item)) == NULL)
	msg("Ran out of memory for header after %d items", total);
    if ((item->l_data = new(size)) == NULL)
	msg("Ran out of memory for data after %d items", total);
    item->l_next = item->l_prev = NULL;
    total++;
    return item;
}

char *
new(size)
int size;
{
    register char *space = ALLOC(size);

    if (space == NULL) {
	sprintf(errbuf,"Rogue ran out of memory (%ld).",sbrk(0));
	debug(errbuf);
	fatal(errbuf);
    }
    return space;
}
