/*
 * Functions for dealing with linked lists of goodies
 *
 * @(#)list.c	7.0	(Bell Labs)	10/08/82
 */

#include "rogue.h"
#include "rogue_ext.h"


/*
 * detach:
 *	Takes an item out of whatever linked list it might be in
 */


int discard (register struct linked_list *item);
extern void msg (const char *fmt, ...);
extern void debug (char *errstr);
extern int fatal (char *s);

_detach(register struct linked_list **list, register struct linked_list *item)
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

_attach(register struct linked_list **list, register struct linked_list *item)
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

_free_list(register struct linked_list **ptr)
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

discard(register struct linked_list *item)
{
#ifdef DEBUGLIST
    fprintf(stderr, "Discard 0x%x in 0x%x\n", item->l_data, item);
#endif
    FREE(item->l_data);
    FREE(item);
    total--;
}

/*
 * new_item
 *	get a new item with a specified size
 */

struct linked_list *
new_item(int size)
{
    register struct linked_list *item;

    if ((item = (struct linked_list *) new(sizeof *item)) == NULL)
	msg("Ran out of memory for header after %d items", total);
    if ((item->l_data = new(size)) == NULL)
	msg("Ran out of memory for data after %d items", total);
#ifdef DEBUGLIST
    fprintf(stderr, "Allocate 0x%x in 0x%x\n", item->l_data, item);
#endif
    item->l_next = item->l_prev = NULL;
#ifdef DEBUGLIST
    item->l_size = size;
#endif
    total++;
    return item;
}

char *
new(int size)
{
    register char *space = ALLOC(size);

    if (space == NULL) {
	sprintf(errbuf,"Rogue ran out of memory (%ld).",sbrk(0));
	debug(errbuf);
	fatal(errbuf);
    }
    return space;
}

#ifdef DEBUGLIST
void checksize(struct linked_list *list, int size)
{
    if (list->l_size != size) {
	fprintf(stderr, "Bad size for list %d != %d\n", list->l_size, size);
    }
}
#endif
