#include <string.h>
#include "defines.h"

#ifndef MHASH_H
#define MHASH_H

/* Maximum number of moves that can be stored in a move hash table */
#define MT_SIZE 2047

/* Move table struct, for telling 6C 6D 7D 7C = 6C 7D 6D 7C */
struct mTable {
	struct play table[MT_SIZE];
	uint_fast16_t n;
};

void initMTable(struct mTable* const restrict mt) __attribute__((nonnull,hot));

void addMove(struct mTable* const restrict mt, const struct play* const restrict play) __attribute__((nonnull,hot));

bool lookupMove(const struct mTable* const restrict mt, const struct play* const restrict play) __attribute__((hot,pure,nonnull));

#endif /* MHASH_H */
