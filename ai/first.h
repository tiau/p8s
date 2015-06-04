#include "../defines.h"

/* The first AI for pokereights ever created by anyone!  What fun.  This AI
 * just selects the first play from the given playlist.  If it has to decide
 * which suit should follow an 8 it always chooses clubs. */

#ifndef AI_FIRST_H
#define AI_FIRST_H

uint_fast32_t aiFirst(const struct aistate* const restrict as)
	__attribute__((const,nonnull));

#endif /* AI_FIRST_H */
