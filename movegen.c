#include "movegen.h"

__attribute__((nonnull,hot,pure)) static bool oneSuit(const struct play* const restrict play)
{
	size_t i;
	suit_t s;

	assert(play->n >= 2);

	s = getSuit(play->c[0]);
	for(i = 1; i < play->n; i++)
		if(getSuit(play->c[i]) != s)
			return false;
	return true;
}

__attribute__((hot,nonnull,pure)) static bool isStraightish(const struct play* const restrict play)
{
	bool seen[(DECKLEN/4)+1] = { 0 };
	size_t i;
	card_t t, cmin = DECKLEN/4-1;

	{	assert(play);
		assert(play->n >= 3);
		assert(play->n <= MAXCIP);}

	for(i = 0; i < play->n; i++) {
		t = getVal(play->c[i]);
		if(seen[t])
			return false;
		seen[t] = true;
		if(t < cmin)
			cmin = t;
	}

	const bool *bp = (cmin == 1 && seen[13]) ? &seen[9] : &seen[cmin];
	if(!*++bp) return false;
	if(!*++bp) return false;
	if(play->n > 3) {
		if(!*++bp) return false;
		if(play->n > 4)
			if(!*++bp) return false;
	}
	return true;
}

__attribute__((nonnull,hot,pure)) static bool isLegalish(const struct gamestate* const restrict gs, const struct play* const restrict play)
{
	int_fast8_t d1, d2, d3;

	switch(play->n) {
		case 0:
			return true;
		case 1:
			return isCardOneLegal(gs, play);
		case 2:
			/* We can't rule out any moves at size 2; we could have two seemingly
			 * unrelated cards destined to a two-pair or full house down the line. */
			return true;
		case 4:
			d3 = getVal(play->c[0]) - getVal(play->c[3]);
			/* Fall-through */
		case 3:
			d2 = getVal(play->c[0]) - getVal(play->c[2]);
			d1 = getVal(play->c[0]) - getVal(play->c[1]);
			if(play->n == 3)
				return !d1 || !d2 || oneSuit(play) || isStraightish(play);
			return (!d1 && !d2) || (!d1 && !d3) || (!d2 && !d3) || oneSuit(play) || isStraightish(play);
		case 5:
			/* 5 card plays are immediately checked by isMoveLegal() */
			return true;
	}
	assert(false);
	return false; // Fixes warning, we never get here
}

__attribute__((nonnull,hot)) static void addLegals(const struct play* const restrict play, struct plist* restrict pl, struct mTable* const restrict mt)
{
	if(!lookupMove(mt, play)) {
		addMove(mt, play);
		plistAdd(pl, play);
	}
}

__attribute__((nonnull,hot)) static void movesTree(const struct gamestate* const restrict gs, struct plist* const restrict pl, struct mTable* const restrict mt, const struct play* const restrict play, const struct player* const restrict pool)
{
	size_t i;

	assert(play->n <= MAXCIP);

	if(isPlayLegal(play))
		addLegals(play, pl, mt);

	if(play->n == MAXCIP || pool->n == 0)
		return;

	struct play t1 = *play;
	struct player t2 = *pool;
	t1.n++;
	t2.n--;
	for(i = 0; i < t2.n + 1; i++) {
		t1.c[t1.n-1] = t2.c[i];
		cardSwap(&t2.c[i], &t2.c[t2.n]);
		if(isLegalish(gs, &t1))
			movesTree(gs, pl, mt, &t1, &t2);
		cardSwap(&t2.c[i], &t2.c[t2.n]);
	}
}

struct plist* getPotentials(const struct gamestate* const restrict gs, const struct player* const restrict player)
{
	struct plist* restrict pl;
	struct mTable mt;
	struct play play;

	{	assert(gs);
		assert(player);
		assert(player->n);}

	pl = plistNew();
	initMoveTable(&mt, player->n);
	play.n = 0;
	movesTree(gs, pl, &mt, &play, player);
	freeMoveTable(&mt);

	return pl;
}

bool isPlayLegal(const struct play* const restrict play)
{
	assert(play);

	switch(play->n) {
		case 0:
			return false;
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
