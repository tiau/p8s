#include "monte.h"
#include "random.h"

/* The Cheat AI is the same as the Monte AI except that it doesn't shuffle its
 * opponents' hands into the deck and then deal them new cards.  Instead it
 * leaves all the hands as they are.  It also doesn't shuffle the deck.  In
 * short, it cheats, a lot. */

#ifndef AI_CHEAT_H
#define AI_CHEAT_H

/* N.B. Caller must free gs->players */
void initCheatGameStateHypothetical(struct gamestate* const restrict gs,
									const struct gamestate* const restrict ogs)
	__attribute__((hot,nonnull));

uint_fast32_t aiCheat(const struct aistate* const restrict as)
	__attribute__((hot,nonnull));

#endif /* AI_CHEAT_H */
