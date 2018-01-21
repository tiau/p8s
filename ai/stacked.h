#include "monte.h"

/* The Stacked AI is the same as the Monte AI except that instead of just
 * randomly shuffling all the cards that it doesn't know about (opponents'
 * hands and the deck) it gives the opponent good hands per evalPlayer()
 * depending on how many cards they've recently drawn. */

#ifndef AI_STACKED_H
#define AI_STACKED_H

/* The max number of different deals to attempt */
#define MAXDEALS 8

/* What AI to call to play hypothetical games */
#define STACKEDAIF aiJudge

void initStackedGameStateHypothetical(struct gamestate* const restrict gs,
									  const struct gamestate* const restrict ogs)
	__attribute__((hot,nonnull));

uint_fast32_t aiStacked(const struct aistate* const restrict as)
	__attribute__((hot,nonnull));

#endif /* AI_STACKED_H */
