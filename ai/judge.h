#include "shedder.h"

/* The Judge AI looks at both what cards it might play and what cards would be
 * left in its hand to determine a good play to make.  The heuristics aren't
 * very complicated.  For plays: penalize 2s, 3s and 8s wasted in the middle.
 * For the its hand: give a bonus for any type of poker hands, 2s, 3s and
 * especially 8s. */

#ifndef AI_JUDGE_H
#define AI_JUDGE_H

int_fast16_t evalPlayer(const struct player* const restrict p,
						const size_t nplayers,
						const size_t cn)
	__attribute__((hot,pure,nonnull));

int_fast16_t evalPlay(const struct play* const restrict p,
					  const size_t nplayers,
					  const size_t* const cih,
					  const suit_t bs)
	__attribute__((hot,pure,nonnull));

uint_fast32_t aiJudge(const struct aistate* const restrict as)
	__attribute__((hot,pure,nonnull));

#endif /* AI_JUDGE_H */
