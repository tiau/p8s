#include "defines.h"

#ifndef PLIST_H
#define PLIST_H

/* Number of elements to store in each node */
#define PLNODESIZE 50

struct plist {
	size_t sz;	// Number of elements in this bucket
	size_t n;	// Number of elements in this and all next buckets
	struct play plays[PLNODESIZE];
	struct plist* next;
};

struct plist* plistNew(void)
	__attribute__((hot,malloc,returns_nonnull));

void plistAdd(struct plist* const pl, const struct play* const restrict play)
	__attribute__((hot,nonnull));

const struct play* plistGet(const struct plist* const pl, const size_t i)
	__attribute__((hot,nonnull,pure,returns_nonnull));

void plistDel(struct plist* head)
	__attribute__((nonnull));

#endif /* PLIST_H */
