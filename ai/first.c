#include "first.h"

uint_fast32_t aiFirst(const struct aistate* const restrict as __attribute__((unused)))
{
	{	assert(as);
		assert(as->gs);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);}

	uint_fast32_t ret = 0; // This assignment fixes a warning
	MPACK(ret, 0);
	ESPACK(ret, Clubs);
	return ret;
}
