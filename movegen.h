#include "defines.h"
#include "mhash.h"
#include "plist.h"
#include "poker.h"

#ifndef MOVEGEN_H
#define MOVEGEN_H

/* N.B. Caller must plistDel() return value */
struct plist* getPotentials(const struct gamestate* const restrict gs,
							const struct player* const restrict player)
	__attribute__((malloc,hot,nonnull,returns_nonnull));

bool isPlayLegal(const struct play* const restrict play)
	__attribute__((hot,pure,nonnull));


__attribute__((hot,pure,nonnull))
static int cmpcardt(const void* const c1, const void* const c2)
{
	{	assert(c1);
		assert(c2);}
	return *((const card_t* const)c1) - *((const card_t* const)c2);
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

__attribute__((hot,pure,nonnull))
inline static bool isCardOneLegal(const struct gamestate* const restrict gs, const struct play* const restrict play)
{
	{	assert(gs);
		assert(play);
		assert(play->n);}

	const card_t v = getVal(*play->c);
	return ((getSuit(*play->c) == ((gs->eightSuit == UNKNOWN) ? getSuit(*gs->pile.top) : gs->eightSuit)) ||
			(v == getVal(*gs->pile.top)) ||
			(v == 8));
}

#endif /* MOVEGEN_H */
