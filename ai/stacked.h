#include "monte.h"

/* The Stacked AI is the same as the Monte AI except that instead of just
 * randomly shuffling all the cards that it doesn't know about (opponent's
 * hands and the deck) it tends to put magic cards (2s, 3s, 8s) into its
 * opponents hands, as smart players will likely be holding these cards. */

#ifndef AI_STACKED_H
#define AI_STACKED_H

/* What AI to call to play hypothetical games */
#define STACKEDAIF aiJudge

void stackTheDeck(struct deck* const restrict deck,
				  const size_t tch)
	__attribute__((hot,nonnull));
    
void initStackedGameStateHypothetical(struct gamestate* const restrict gs,
									  const struct gamestate* const restrict ogs)
	__attribute__((hot,nonnull));

uint_fast32_t aiStacked(const struct aistate* const restrict as)
	__attribute__((hot,pure,nonnull));

#endif /* AI_STACKED_H */
