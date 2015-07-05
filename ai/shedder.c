#include "shedder.h"

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
	uint_fast32_t ret = 0;
	size_t i, best = 0;
	const struct play* play;

	{	assert(as);
		assert(as->gs);
		assert(as->pl);
		assert(as->pl->n);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);}

	MPACK(ret, as->pl->n);
	ESPACK(ret, freqSuit(stateToPlayer(as->gs)));
	for(i = 0; i < as->pl->n; i++) {
		play = plistGet(as->pl, i);
		if(play->n > best) {
			MPACK(ret, i);
			best = play->n;
		}
	}
	return ret;
}
