#include <string.h>
#include <math.h>
#include "defines.h"

#ifndef MHASH_H
#define MHASH_H

/* Move table struct, for telling 6C 6D 7D 7C = 6C 7D 6D 7C */
struct mTable {
	struct play* table;
	uint_fast16_t s;
};

void initMoveTable(struct mTable* const restrict mt, size_t hint) __attribute__((nonnull,hot));

void addMove(struct mTable* const restrict mt, const struct play* const restrict play) __attribute__((nonnull,hot));

bool lookupMove(const struct mTable* const restrict mt, const struct play* const restrict play) __attribute__((hot,pure,nonnull));

__attribute__((nonnull,hot)) static inline void freeMoveTable(struct mTable* const restrict mt)
{
	assert(mt);
	free(mt->table);
}

#endif /* MHASH_H */
