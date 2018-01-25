#include "cheat.h"

/* The MinMax Cheat AI, like the Cheat AI, cheats a lot. It uses minmax with
 * alpha-beta pruning instead of monte carlo to evaluate potential moves. 
 * It's better than the regular cheat AI. */

#ifndef AI_MMCHEAT_H
#define AI_MMCHEAT_H

/* How many plies we search before evaluating */
static const size_t MaxPlies = 7;

/* The hand size at which we cease considering drawing more cards. Letting this
 * be unbounded results in the AI drawing so many cards that its evaluation to
 * MaxPlies plies becomes too slow. */
static const size_t MaxCards = CPP + 3;

uint_fast32_t aiMmCheat(const struct aistate* const restrict as)
	__attribute__((hot,pure,nonnull));

#endif /* AI_MMCHEAT_H */
