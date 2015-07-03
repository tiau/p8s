#include <string.h>
#include <search.h>
#include "io.h"
#include "movegen.h"

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

float gameLoop(struct gamestate* const restrict gs,
			  const uint8_t verbose,
			  bool eight,
			  bool magic,
			  uint_fast32_t (*aif[MAXPLRS])(const struct aistate* const restrict))
	__attribute__((hot,nonnull));

__attribute__((nonnull,hot))
inline static void initPile(struct deck* const restrict pile)
{
	assert(pile);

	memset(pile->c, 0, DECKLEN);
	pile->top = pile->c;
	pile->n = 0;
}

__attribute__((hot,const))
inline static bool isMagicCard(card_t c)
{
	{	assert(c);
		assert(c <= DECKLEN);}

	c = getVal(c);
	return c == 2 || c == 3;
}

__attribute__((cold,nonnull))
inline static void orderHand(struct player* const restrict player)
{
	{	assert(player);
		assert(player->n);}
	qsort(player->c, player->n, sizeof(card_t), cmpcardt);
}

__attribute__((nonnull,returns_nonnull))
inline static struct player* stateToPlayer(const struct gamestate* const restrict gs)
{
	assert(gs);
	return &gs->players[gs->turn % gs->nplayers];
}

#endif /* ENGINE_H */
