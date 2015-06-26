#include "mhash.h"

__attribute__((pure,nonnull,hot)) static uint_fast16_t mHash(const struct play* const restrict play)
{
	size_t i;
	uint_fast16_t hash = 0;

	hash = play->c[play->n-1];
	hash <<= 8;
	for(i = 0; i < play->n-1; i++)
		hash ^= play->c[i];

	return hash % MT_SIZE;
}

__attribute__((pure,nonnull,hot)) static bool isEqualEnough(const struct play* const restrict p1, const struct play* const restrict p2)
{
	size_t i, j;
	size_t matches = 0;

	{	assert(p1);
		assert(p2);}

	if(p1->n != p2->n || p1->c[p1->n-1] != p2->c[p1->n-1])
		return false;

	for(i = 0; i < p1->n - 1; i++) {
		for(j = 0; j < p1->n - 1; j++) {
			if(p1->c[i] == p2->c[j]) {
				matches++;
				break;
			}
		}
		if(matches != i + 1) return false;
	}

	return true;
}

__attribute__((pure,nonnull,hot)) static const struct play* nextSlot(const struct mTable* const restrict mt, const struct play* const restrict play)
{
	const struct play *t, *tt;

	{	assert(mt);
		assert(play);}

	tt = &mt->table[mHash(play)];
	t = tt;
	while(t->n) {
		if(isEqualEnough(t, play)) // Play is already present
			return NULL;
		t = (t - mt->table > MT_SIZE) ? mt->table : t + 1;
		if(unlikely(t == tt)) {
			fprintf(stderr, "%s: Move hash table is full.\n", __func__);
			return NULL;
		}
	}
	return t;
}

void initMoveTable(struct mTable* const restrict mt)
{
	assert(mt);

	memset(mt->table, 0, MT_SIZE * sizeof(struct play));
	mt->n = 0;
}

void addMove(struct mTable* const restrict mt, const struct play* const restrict play)
{
	{	assert(mt);
		assert(play);}

	struct play* const t = (struct play*)nextSlot(mt, play);
	if(t) *t = *play;
}

bool lookupMove(const struct mTable* const restrict mt, const struct play* const restrict play)
{
	{	assert(mt);
		assert(play);}

	return nextSlot(mt, play) == NULL;
}
