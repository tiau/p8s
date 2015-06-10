#include "../defines.h"
#include "../plist.h"

/* The random AI selects a random play to make.  If that play ends on an 8 it
 * selects a random suit to come next. */

#ifndef AI_RANDOM_H
#define AI_RANDOM_H

uint_fast32_t aiRandom(const struct aistate* const restrict as)
	__attribute__((nonnull));

#endif /* AI_RANDOM_H */
