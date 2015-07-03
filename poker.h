#include "defines.h"

#ifndef POKER_H
#define POKER_H

bool isPair(const struct play* const restrict p)
	__attribute__((hot,pure,nonnull));

bool isThreeOAK(const struct play* const restrict p)
	__attribute__((hot,pure,nonnull));

bool isTwoPair(const struct play* const restrict p)
	__attribute__((hot,pure,nonnull));

bool isFourOAK(const struct play* const restrict p)
	__attribute__((hot,pure,nonnull));

bool isFlush(const struct play* const restrict p)
	__attribute__((hot,pure,nonnull));

bool isFullHouse(const struct play* const restrict p)
	__attribute__((hot,pure,nonnull));

bool isStraight(const struct play* const restrict p)
	__attribute__((hot,pure,nonnull));

#endif /* POKER_H */
