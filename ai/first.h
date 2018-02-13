#include "../defines.h"
#include "shedder.h"

/* The first AI for pokereights ever created by anyone! What fun. This AI
 * just selects the first play from the given playlist. If it has to decide
 * which suit should follow an 8 it always chooses clubs. */

#ifndef AI_FIRST_H
#define AI_FIRST_H

int_fast16_t evalPlayerf(const struct player* const restrict player, const size_t nplayers, const size_t cn);

uint_fast32_t aiFirst(const struct aistate* const restrict as)
	__attribute__((const,nonnull));

#endif /* AI_FIRST_H */
