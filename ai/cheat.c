#include "cheat.h"

void initCheatGameStateHypothetical(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	size_t i, j;

	{	assert(gs);
		assert(ogs);}

	initGameStateHypoShared(gs, ogs);
	gs->deck = ogs->deck;
	gs->deck.top = gs->deck.c + (ogs->deck.top - ogs->deck.c);
	/* This makes it so we (Cheat) are always gs->players[0] */
	for(i = ogs->turn % gs->nplayers, j = 0; j < gs->nplayers; i = (i + 1) % gs->nplayers, j++)
		gs->players[j] = ogs->players[i];
}

uint_fast32_t aiCheat(const struct aistate* const restrict as)
{
	/* Since Cheat does no randomization, it needs to use a nondeterministic AI
	 * so as not to just play the same game over and over.  Using Random also
	 * lets it more thoroughly explore its vastly constricted state space. */
	return pctmRun(as, initCheatGameStateHypothetical, aiRandom, __func__);
}
