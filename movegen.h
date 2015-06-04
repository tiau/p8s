#include "defines.h"
#include "engine.h"
#include "mhash.h"
#include "plist.h"

#ifndef MOVEGEN_H
#define MOVEGEN_H

/* N.B. Caller must plistDel() return value */
struct plist* getPotentials(const struct gamestate* const restrict gs,
							const struct player* const restrict player)
	__attribute__((malloc,hot,nonnull,returns_nonnull));

#endif /* MOVEGEN_H */
