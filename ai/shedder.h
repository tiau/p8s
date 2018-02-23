#include "../engine.h"

/* The shedder AI selects the first play it comes across that drops the maximum
 * number of cards it can drop on a given turn. For example, it will play a
 * flush before a four of a kind, and a four of a kind before a pair. After
 * playing an 8 it selects the suit that was most populous in its hand. */

#ifndef AI_SHEDDER_H
#define AI_SHEDDER_H

suit_t freqSuit(const struct player* const restrict player)
	__attribute__((hot,pure,nonnull));

uint_fast32_t aiShedder(const struct aistate* const restrict as)
	__attribute__((hot,nonnull));

#endif /* AI_SHEDDER_H */
