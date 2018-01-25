#include "judge.h"

/* The Draw AI is the same as the Judge AI except that it also considers the
 * possibility of drawing or passing depending on its chances of getting a good
 * card upon drawing or leaving the next player in a worse situation upon
 * passing. It's very slightly better than Judge in direct play. */

#ifndef AI_DRAW_H
#define AI_DRAW_H

/* How much better a given hand must be than our current hand to be considered
 * worth trying to draw for. Higher numbers make us less likely to draw. */
static const float DrawThreshold = 3.2;

/* The weightings on how much suit matching and value matching matter when
 * considering passing. Generally suits are more important since there're only
 * 4 of them to 13 values. Higher 'Play' numbers make us more likely to pass.
 * Higher 'Pass' numbers make us more likely to play. */
static const float PlaySuitMult = 2.0f;
static const float PlayValMult = 1.0f;
static const float PassSuitMult = 1.5f;
static const float PassValMult = 0.8f;

void initDeckSans(struct deck* const restrict deck,
				  const struct player* const restrict player,
				  const struct deck* const restrict pile)
	__attribute__((hot,nonnull));

uint_fast32_t aiDraw(const struct aistate* const restrict as)
	__attribute__((hot,pure,nonnull));

#endif /* AI_DRAW_H */
