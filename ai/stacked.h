#include "monte.h"

/* The Stacked AI is the same as the Monte AI except that instead of just
 * randomly shuffling all the cards that it doesn't know about (opponents'
 * hands and the deck) it gives the opponent good hands per evalPlayer()
 * depending on how many cards they've recently drawn. */

#ifndef AI_STACKED_H
#define AI_STACKED_H

/* The max number of different deals to attempt. This is essentially a measure
 * of pessimism, the more deals the more likely Stacked is to think its
 * opponent has a good hand. 6 seems to be optimal against Monte or Cheat. */
static const size_t MaxDeals = 6;

/* What AI to call to play hypothetical games */
__attribute__((unused)) static uint_fast32_t (*StackedSub)(const struct aistate* const restrict) = aiDraw;

void initStackedGameStateHypothetical(struct gamestate* const restrict gs,
									  const struct gamestate* const restrict ogs)
	__attribute__((hot,nonnull));

uint_fast32_t aiStacked(const struct aistate* const restrict as)
	__attribute__((hot,nonnull));

__attribute__((hot,const,always_inline))
inline static float minf(const float x, const float y)
{
	return (x > y ? y : x);
}

__attribute__((hot,const,always_inline))
inline static float maxf(const float x, const float y)
{
	return (x > y ? x : y);
}

#endif /* AI_STACKED_H */
