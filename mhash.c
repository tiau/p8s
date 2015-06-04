#include "mhash.h"

struct mTable* initMTable(void)
{
	return calloc(1, sizeof(struct mTable));
}

void freeMTable(struct mTable* const restrict mt)
{
	size_t i;

	assert(mt);

	for(i = 0; i < 52; i++)
		if(mt->t[i])
			freeMTable(mt->t[i]);

	free(mt);
}

__attribute__((nonnull,hot)) inline static void addMTable(struct mTable* const restrict mt, const card_t b)
{
	{	assert(mt);
		assert(b > 0 && b <= DECKLEN);}

	if(!mt->t[b]) {
		mt->t[b] = initMTable();
		assert(mt->t[b]);
	}
}

__attribute__((nonnull,hot)) inline static void handleEights(bool (* const restrict v)[52], card_t c)
{
	size_t i;

	assert(v);

	if(getVal(c) == 8)
		for(i = 8; i <= 47; i += 13)
			(*v)[i] = true;
}

void addMove(struct mTable* const restrict mt, const struct play* const restrict play)
{
	struct play l;

	{	assert(mt);
		assert(play);
		assert(play->n <= MAXCIP);}

	if(unlikely(play->n == 1))
		return;

	l.n = play->n - 1;
	memcpy(l.c, play->c, sizeof(card_t)*play->n);
	orderPlay(&l);

	switch(play->n) {
		case 2:
			mt->v[l.c[0]] = true;
			handleEights(&mt->v, l.c[0]);
			break;
		case 3:
			addMTable(mt, l.c[0]);
			mt->t[l.c[0]]->v[l.c[1]] = true;
			handleEights(&mt->t[l.c[0]]->v, l.c[1]);
			break;
		case 4:
			addMTable(mt, l.c[0]);
			addMTable(mt->t[l.c[0]], l.c[1]);
			mt->t[l.c[0]]->t[l.c[1]]->v[l.c[2]] = true;
			handleEights(&mt->t[l.c[0]]->t[l.c[1]]->v, l.c[2]);
			break;
		case 5:
			addMTable(mt, l.c[0]);
			addMTable(mt->t[l.c[0]], l.c[1]);
			addMTable(mt->t[l.c[0]]->t[l.c[1]], l.c[2]);
			mt->t[l.c[0]]->t[l.c[1]]->t[l.c[2]]->v[l.c[3]] = true;
			handleEights(&mt->t[l.c[0]]->t[l.c[1]]->t[l.c[2]]->v, l.c[3]);
			break;
	}
}

bool lookupMove(const struct mTable* const restrict mt, const struct play* const restrict play)
{
	struct play l;

	{	assert(mt);
		assert(play);
		assert(play->n >= 1 && play->n <= MAXCIP);}

	if(unlikely(play->n == 1))
		return false;

	l.n = play->n - 1;
	memcpy(l.c, play->c, sizeof(card_t)*play->n);
	orderPlay(&l);

	if(play->n >= 3)
		if(!mt->t[l.c[0]])
			return false;
	if(play->n >= 4)
		if(!mt->t[l.c[0]]->t[l.c[1]])
			return false;
	if(play->n >= 5)
		if(!mt->t[l.c[0]]->t[l.c[1]]->t[l.c[2]])
			return false;

	switch(play->n) {
		case 2:
			return mt->v[l.c[0]];
		case 3:
			return mt->t[l.c[0]]->v[l.c[1]];
		case 4:
			return mt->t[l.c[0]]->t[l.c[1]]->v[l.c[2]];
		case 5:
			return mt->t[l.c[0]]->t[l.c[1]]->t[l.c[2]]->v[l.c[3]];
		default:
			assert(false);
	}
	assert(false);
	return false; // Fixes warning, we never get here
}
