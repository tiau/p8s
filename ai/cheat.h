#include "monte.h"

/* The Cheat AI is the same as the Monte AI except that it doesn't shuffle its
 * opponent's hands into the deck and then deal them new cards.  Instead it
 * leaves all the hands as they are; which is to say, it cheats. */

#ifndef AI_CHEAT_H
#define AI_CHEAT_H

uint_fast32_t aiCheat(const struct aistate* const restrict as)
	__attribute__((hot,pure,nonnull));

#endif /* AI_CHEAT_H */
