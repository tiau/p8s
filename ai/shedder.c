#include "shedder.h"
#include "stacked.h"
#include "random.h"

suit_t freqSuit(const struct player* const restrict player)
{
	size_t i, most;
	suit_t ret = Clubs;
	size_t ns[4] = { 0 };

	assert(player);

	for(i = 0, most = 0; i < player->n; i++)
		if(getVal(player->c[i]) != 8)
			ns[getSuit(player->c[i])]++;

	for(i = Clubs; i <= Spades; i++) {
		if(ns[i] > most) {
			most = ns[i];
			ret = i;
		}
	}

	return ret;
}

uint_fast32_t aiShedder(const struct aistate* const restrict as)
{
	return pctmRun(as, initStackedGameStateHypothetical, aiRandom, __func__);
}
