#include <string.h>
#include <search.h>
#include "poker.h"
#include "defines.h"

#ifndef ENGINE_H
#define ENGINE_H

void cshuffle(card_t* const restrict cards,
			  const size_t s,
			  const size_t e)
	__attribute__((hot,nonnull));

void populateCIH(const struct gamestate* const restrict gs,
				 size_t* const restrict cih)
	__attribute__((hot,nonnull));

void initGameState(struct gamestate* const restrict gs,
				   const size_t nplayers,
				   const uint8_t* const restrict ai)
	__attribute__((nonnull,cold));

void cleanGameState(struct gamestate* const restrict gs)
	__attribute__((nonnull));

int getGameState(const struct gamestate* const restrict gs)
	__attribute__((pure,nonnull,cold));

bool isPlayLegal(const struct play* const restrict play)
	__attribute__((hot,pure,nonnull));

bool isCardOneLegal(const struct gamestate* const restrict gs,
					const struct play* const restrict play)
	__attribute__((hot,pure,nonnull));

void removeCards(struct player* const restrict player,
				 const struct play* const restrict play)
	__attribute__((hot,nonnull));

void makeMove(struct gamestate* const restrict gs,
			  const struct play* const restrict play)
	__attribute__((hot,nonnull));

bool playerDrawCard(struct gamestate* const gs,
					struct player* const player)
	__attribute__((hot,nonnull));

bool drawCard(struct gamestate* const restrict gs)
	__attribute__((nonnull));

__attribute__((hot,const)) inline static bool isMagicCard(card_t c)
{
	{	assert(c);
		assert(c <= DECKLEN);}

	c = getVal(c);
	return c == 2 || c == 3;
}

#endif /* ENGINE_H */
