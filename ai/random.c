#include "random.h"

uint_fast32_t aiRandom(const struct aistate* const restrict as)
{
	{	assert(as);
		assert(as->gs);
		assert(as->pl);
		assert(as->pl->n);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);}

	uint_fast32_t ret = 0;
	ESPACK(ret, rand() % Unknown);	// Unknown is a suit
	/* No chance to draw/pass; it makes the AI much worse if allowed and not
 	 * strongly discouraged. */
	MPACK(ret, rand() % as->pl->n);
	return ret;
}
