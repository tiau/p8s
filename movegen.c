#include "movegen.h"

__attribute__((nonnull,hot)) inline static void cardsRev(card_t* const restrict c, size_t s, size_t e)
{
	{	assert(c);
		assert(s <= e);}

	while(e > s)
		cardSwap(&c[e--], &c[s++]);
}

__attribute__((pure,nonnull,hot)) static ssize_t permuteIndex(const struct play* const restrict play)
{
	ssize_t i;

	{	assert(play);
		assert(play->n > 1 && play->n <= MAXCIP);}

	for(i = play->n - 2; i > -1; i--)
		if(play->c[i] < play->c[i+1])
			return i;
	return -1;
}

__attribute__((nonnull,hot)) static void addLegals(const struct gamestate* const restrict gs, const struct play* const restrict play, struct plist* restrict pl, struct mTable* const restrict mt)
{
	if(isCardOneLegal(gs, play) && !lookupMove(mt, play)) {
		addMove(mt, play);
		plistAdd(pl, play);
	}
}

__attribute__((nonnull,hot)) static void permuteGuts(const struct gamestate* const restrict gs, const struct player* const restrict player, struct plist* restrict pl, struct mTable* const restrict mt, struct play* const restrict play)
{
	size_t i;
	ssize_t j;

	qsort(play->c, play->n, sizeof(card_t), cmpcardt);
	addLegals(gs, play, pl, mt);
	while((j = permuteIndex(play)) != -1) {
		for(i = play->n - 1; i < play->n && play->c[i] < play->c[j]; i--);
		cardSwap(&play->c[j], &play->c[i]);
		cardsRev(play->c, j + 1, play->n - 1);
		addLegals(gs, play, pl, mt);
	}
}

__attribute__((nonnull,hot)) static void permutator(const size_t k, const struct gamestate* const restrict gs, const struct player* const restrict player, struct plist* restrict pl, ssize_t* restrict is, struct mTable* const restrict mt)
{
	struct play play;
	size_t i;

	play.n = k;
	for(i = 0; i < k; i++)
		play.c[i] = player->c[is[i+1]];

	if(isPlayLegal(&play)) {
		permuteGuts(gs, player, pl, mt, &play);
	}
}

__attribute__((nonnull,hot)) static void recurserator(const size_t d, const size_t k, const struct gamestate* const restrict gs, const struct player* const restrict player, struct plist* restrict pl, ssize_t* restrict is, struct mTable* const restrict mt)
{
	{	assert(d < DECKLEN);
		assert(player);
		assert(pl);
		assert(is);}

	if(!d) {
		/* If we only have 1 card then there's only 1 permutation */
		if(unlikely(k == 1)) {
			struct play play;
			play.n = 1;
			play.c[0] = player->c[is[1]];
			if(isCardOneLegal(gs, &play))
				plistAdd(pl, &play);
		} else {
			permutator(k, gs, player, pl, is, mt);
		}
	} else {
		assert((ssize_t)player->n - (ssize_t)d > -1);
		for(is[d] = is[d+1] + 1; (size_t)is[d] < player->n - d + 1; is[d]++)
			recurserator(d-1, k, gs, player, pl, is, mt);
	}
}

__attribute__((nonnull,hot)) static void nComboPermute(const struct gamestate* const restrict gs, const struct player* const restrict player, const size_t k, struct plist* const restrict pl, struct mTable* const restrict mt)
{
	ssize_t is[k + 2];
	size_t i;

	{	assert(player);
		assert(k < DECKLEN);}

	/* Initialize array of indicies (i, j, k, et cetera) */
	for(i = 0; i < k + 2; i++)
		is[i] = k - i;

	/* Generate all k-permutations of all k-combinations of the player's hand */
	recurserator(k, k, gs, player, pl, is, mt);
}

struct plist* getPotentials(const struct gamestate* const restrict gs, const struct player* const restrict player)
{
	size_t i;
	struct plist* restrict pl;
	struct mTable mt;

	{	assert(gs);
		assert(player);}

	pl = plistNew();
	initMoveTable(&mt, player->n);
	for(i = 1; i <= min(player->n, MAXCIP); i++)
		nComboPermute(gs, player, i, pl, &mt);
	freeMoveTable(&mt);

	return pl;
}

bool isPlayLegal(const struct play* const restrict play)
{
	{	assert(play);
		assert(play->n);}

	switch(play->n) {
		case 1:
			return true;
		case 2:
			return isPair(play);
		case 3:
			return isThreeOAK(play);
		case 4:
			return isTwoPair(play) ||
				   isFourOAK(play);
		case 5:
			return isFlush(play) ||		// Straights are more common, but flushes
				   isStraight(play) ||	// are faster to check
				   isFullHouse(play);
	}
	assert(false);
	return false; // Fixes warning, we never get here
}
