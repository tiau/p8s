#include "random.h"

uint_fast32_t aiRandom(const struct aistate* const restrict as)
{
	{	assert(as);
		assert(as->gs);
		assert(as->pl);
		assert(as->pl->n);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);}

	uint_fast32_t ret = 0;
	ESPACK(ret, rand() % UNKNOWN);	// fyi: UNKNOWN is a suit
	MPACK(ret, rand() % as->pl->n);	// No chance to draw/pass
	return ret;
}
