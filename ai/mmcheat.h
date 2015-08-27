#include "cheat.h"

#ifndef AI_MMCHEAT_H
#define AI_MMCHEAT_H

/* How many plies we search before evaluating */
#define MMDEPTH 4

uint_fast32_t aiMmCheat(const struct aistate* const restrict as)
	__attribute__((hot,pure,nonnull));

#endif /* AI_MMCHEAT_H */
