#include "judge.h"

/* The Draw AI is the same as the Judge AI except that it also considers the
 * possibility of drawing or passing depending on its chances of getting a good
 * card upon drawing or not wasting an 8 upon passing. */

#ifndef AI_DRAW_H
#define AI_DRAW_H

void initDeckSans(struct deck* const restrict deck,
				  const struct player* const restrict player,
				  const struct deck* const restrict pile)
	__attribute__((hot,nonnull));

uint_fast32_t aiDraw(const struct aistate* const restrict as)
	__attribute__((hot,pure,nonnull));

#endif /* AI_DRAW_H */
