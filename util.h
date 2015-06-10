#include <math.h>
#include "defines.h"

#ifndef UTIL_H
#define UTIL_H

__attribute__((hot,pure,nonnull)) static int cmpcardt(const void* const c1, const void* const c2)
{
	{	assert(c1);
		assert(c2);}
	return *((const card_t* const)c1) - *((const card_t* const)c2);
}

__attribute__((hot,const,always_inline))
inline static suit_t getSuit(const card_t c)
{
	/* Lookup tables are much faster than computation for this and getVal */
	static const suit_t vals[] = {UNKNOWN,
								  0,0,0,0,0,0,0,0,0,0,0,0,0,
								  1,1,1,1,1,1,1,1,1,1,1,1,1,
								  2,2,2,2,2,2,2,2,2,2,2,2,2,
								  3,3,3,3,3,3,3,3,3,3,3,3,3};
	assert((c-1)/(DECKLEN/4) == vals[c]);
	return vals[c];
}

__attribute__((hot,const,always_inline))
inline static card_t getVal(const card_t c)
{
	static const card_t vals[] = {0,
								  1,2,3,4,5,6,7,8,9,10,11,12,13,
								  1,2,3,4,5,6,7,8,9,10,11,12,13,
								  1,2,3,4,5,6,7,8,9,10,11,12,13,
								  1,2,3,4,5,6,7,8,9,10,11,12,13};
	assert(((c-1) % (DECKLEN/4)) + 1 == vals[c]);
	return vals[c];
}

__attribute__((hot,const,always_inline))
inline static size_t min(const size_t x, const size_t y)
{
	return (x > y ? y : x);
}

__attribute__((hot,const,always_inline))
inline static size_t max(const size_t x, const size_t y)
{
	return (x > y ? x : y);
}

__attribute__((hot,nonnull))
inline static void cardSwap(card_t* const c1, card_t* const c2)
{
	{	assert(c1);
		assert(c2);
		assert(*c1);
		assert(*c2);}
	card_t t;
	t = *c1;
	*c1 = *c2;
	*c2 = t;
}

__attribute__((cold,nonnull))
inline static void orderHand(struct player* const restrict player)
{
	{	assert(player);
		assert(player->n);}
	qsort(player->c, player->n, sizeof(card_t), cmpcardt);
}

__attribute__((hot,nonnull))
inline static void orderPlay(struct play* const restrict play)
{
	{	assert(play);
		assert(play->n);}
	qsort(play->c, play->n, sizeof(card_t), cmpcardt);
}

__attribute__((nonnull,returns_nonnull))
inline static struct player* stateToPlayer(const struct gamestate* const restrict gs)
{
	assert(gs);
	return &gs->players[gs->turn % gs->nplayers];
}

#endif /* UTIL_H */
