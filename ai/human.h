#include "stacked.h"
#include "cheat.h"

/* The Human AI isn't an AI at all.  It asks the user what play it should make.
 * The user must first enter a number to select which play to make.  If said
 * play ends with an 8 they must enter the first letter of (s)pades, (h)earts,
 * (d)iamonds or (c)lubs to select which suit must follow.
 *
 * '?',  'h' and 'H' query monte, stacked and cheat, respectively, and output
 * the play they would select. */

#ifndef HUMAN_H
#define HUMAN_H

uint_fast32_t human(const struct aistate* const restrict as)
	__attribute__((nonnull,cold));

#endif /* HUMAN_H */
