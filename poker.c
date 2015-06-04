#include "poker.h"

bool isPair(const struct play* const restrict p)
{
	{	assert(p);
		assert(p->n == 2);}

	return getVal(p->c[0]) == getVal(p->c[1]);
}

bool isThreeOAK(const struct play* const restrict p)
{
	card_t t;

	{	assert(p);
		assert(p->n == 3);}

	t = getVal(p->c[0]);
	return getVal(p->c[1]) == t &&
		   getVal(p->c[2]) == t;
}

bool isTwoPair(const struct play* const restrict p)
{
	card_t t1, t2, t3, t4;

	{	assert(p);
		assert(p->n == 4);}

	t1 = getVal(p->c[0]);
	t2 = getVal(p->c[1]);
	t3 = getVal(p->c[2]);
	t4 = getVal(p->c[3]);
	return (t1 == t2 && t3 == t4) ||
		   (t1 == t3 && t2 == t4) ||
		   (t1 == t4 && t2 == t3);
}

bool isFourOAK(const struct play* const restrict p)
{
	card_t t;

	{	assert(p);
		assert(p->n == 4);}

	t = getVal(p->c[0]);
	return getVal(p->c[1]) == t &&
		   getVal(p->c[2]) == t &&
		   getVal(p->c[3]) == t;
}

bool isFlush(const struct play* const restrict p)
{
	suit_t t;

	{	assert(p);
		assert(p->n == 5);}

	t = getSuit(p->c[0]);
	return getSuit(p->c[1]) == t &&
		   getSuit(p->c[2]) == t &&
		   getSuit(p->c[3]) == t &&
		   getSuit(p->c[4]) == t;
}

bool isFullHouse(const struct play* const restrict p)
{
	card_t seen[(DECKLEN/4)+1] = { 0 };
	card_t t, i, j;

	{	assert(p);
		assert(p->n == 5);}

	t = getVal(p->c[0]);
	seen[t]++;
	i = t;
	t = getVal(p->c[1]);
	seen[t]++;
	j = t;
	t = getVal(p->c[2]);
	seen[t]++;
	if(i == j)
		j = t;
	t = getVal(p->c[3]);
	seen[t]++;
	if(i == j)
		j = t;
	seen[getVal(p->c[4])]++;
	return (seen[i] == 3 && seen[j] == 2) ||
		   (seen[i] == 2 && seen[j] == 3);
}

bool isStraight(const struct play* const restrict p)
{
	bool seen[(DECKLEN/4)+1] = { 0 };
	size_t i;
	card_t t, cmin = DECKLEN/4-1;

	{	assert(p);
		assert(p->n == 5);}

	for(i = 0; i < 5; i++) {
		t = getVal(p->c[i]);
		if(seen[t])
			return false;
		seen[t] = true;
		if(t < cmin)
			cmin = t;
	}

	const bool *bp = (cmin == 1 && seen[13]) ? &seen[9] : &seen[cmin];
	if(!*++bp) return false; // Manual loop unrolling ftw!
	if(!*++bp) return false;
	if(!*++bp) return false;
	if(!*++bp) return false;
	return true;
}
