#include "plist.h"

struct plist* plistNew()
{
	struct plist* ret;
	ret = malloc(sizeof(struct plist));
	assert(ret);
	ret->next = NULL;
	ret->sz = 0;
	ret->n = 0;
	return ret;
}

void plistAdd(struct plist* const pl, const struct play* const restrict play)
{
	struct plist *t, *ret;

	{	assert(pl);
		assert(play);}

	for(t = pl; t->next && t->sz == PLNODESIZE; t = t->next)
		t->n++;
	t->n++;

	if(t->sz == PLNODESIZE) {
		/* All slots full, need to create a new node */
		ret = plistNew();
		ret->plays[0] = *play;
		ret->sz = 1;
		ret->n = 1;
		t->next = ret;
	} else {
		t->plays[t->sz++] = *play;
	}
}

const struct play* plistGet(const struct plist* const pl, const size_t i)
{
	size_t j;
	const struct plist* t = pl;
	
	{	assert(pl);
		assert(i < pl->n);}
	
	for(j = 0; j + PLNODESIZE <= i; j += PLNODESIZE)
		t = t->next;
	assert(i - j <= t->sz);
	return &t->plays[i-j];
}

void plistDel(struct plist* head)
{
	struct plist* n;

	assert(head);

	do {
		n = head->next;
		free(head);
		head = n;
	} while(head);
}
