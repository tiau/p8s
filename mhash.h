#include <string.h>
#include "defines.h"
#include "util.h"

#ifndef MHASH_H
#define MHASH_H

/* Move table struct, for telling 6C 6D 7D 7C = 6C 7D 6D 7C */
struct mTable {
	bool v[52];
	struct mTable* t[52];
};

/* N.B. Caller must freeMTable return value */
struct mTable* initMTable(void) __attribute__((malloc,hot,returns_nonnull));

void freeMTable(struct mTable* const restrict mt) __attribute__((nonnull,hot));

void addMove(struct mTable* const restrict mt,
			 const struct play* const restrict play)
	__attribute__((nonnull,hot));

bool lookupMove(const struct mTable* const restrict mt,
				const struct play* const restrict play)
	__attribute__((pure,nonnull,hot));

#endif /* MHASH_H */
