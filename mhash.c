#include "mhash.h"

__attribute__((pure,nonnull,hot)) static uint_fast16_t mHash(const struct play* const restrict play, const uint_fast16_t s)
{
	size_t i;
	uint_fast16_t hash;

	{	assert(play);
		assert(play->n && play->n <= MAXCIP);}

	/* Ternarys to treat all 8s as 8C (all 8s are the same) */
	hash = (getVal(play->c[play->n-1]) == 8) ? 8 : play->c[play->n-1];
	hash <<= 8;
	for(i = 0; i < play->n-1; i++)
		hash ^= (getVal(play->c[i]) == 8) ? 8 : play->c[i];

	return hash % s;
}

__attribute__((pure,nonnull,hot)) static bool isEqualEnough(const struct play* const restrict p1, const struct play* const restrict p2)
{
	{	assert(p1);
		assert(p2);
		assert(p1->n && p1->n <= MAXCIP);}

	/* Plays may not be equiv because:
	 * (1) They have different numbers of cards
	 * (2) The card they end with differs (provided they aren't both eights) */
	if(p1->n != p2->n || (p1->c[p1->n-1] != p2->c[p2->n-1] &&
	   (getVal(p1->c[p1->n-1]) != 8 || getVal(p2->c[p2->n-1]) != 8)))
		return false;

	/* (3) Their non-end cards do not all match (w/ exception for 8s) */
	size_t i, j;
	size_t matches = 0;
	for(i = 0; i < p1->n - 1; i++) {
		for(j = 0; j < p1->n - 1; j++) {
			if(p1->c[i] == p2->c[j] || (getVal(p1->c[i]) == 8 && getVal(p2->c[j]) == 8)) {
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
	{	assert(mt);
		assert(play);
		assert(play->n && play->n <= MAXCIP);}

	const struct play* const start = &mt->table[mHash(play, mt->s)];
	const struct play* t = start;
	while(t->n) {
		if(isEqualEnough(t, play)) // Play is already present
			return NULL;
		t = ((size_t)(t - mt->table) >= mt->s) ? mt->table : t + 1;
		if(unlikely(t == start)) {
			fprintf(stderr, "%s: Move hash table (%zu slots) is full.\n", __func__, mt->s);
			return NULL;
		}
	}
	return t;
}

void initMoveTable(struct mTable* const restrict mt, size_t hint)
{
	const uint_fast16_t s = min(4096, powf(2, hint+1));
	mt->table = calloc(s, sizeof(struct play));
	mt->s = s - 1;
}

void addMove(struct mTable* const restrict mt, const struct play* const restrict play)
{
	{	assert(mt);
		assert(play);
		assert(play->n);
		assert(play->n <= MAXCIP);}
	struct play* const t = (struct play*)nextSlot(mt, play);
	if(t) *t = *play;
}

bool lookupMove(const struct mTable* const restrict mt, const struct play* const restrict play)
{
	{	assert(mt);
		assert(play);
		assert(play->n);
		assert(play->n <= MAXCIP);}
	return nextSlot(mt, play) == NULL;
}
