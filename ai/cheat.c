#include "cheat.h"

void initCheatGameStateHypothetical(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	assert(ogs);

	initGameStateHypoShared(gs, ogs);
	gs->deck = ogs->deck;
	gs->deck.top = gs->deck.c + (ogs->deck.top - ogs->deck.c);
	for(size_t i = 0; i < gs->nplayers; i++)
		gs->players[i] = ogs->players[i];
	gs->turn = ogs->turn;
}

uint_fast32_t aiCheat(const struct aistate* const restrict as)
{
	/* Since Cheat does no randomization, it needs to use a nondeterministic AI
	 * so as not to run into the case where it just plays the same game over
	 * and over.  Using aiRandom also allows it to more thoroughly explore its
	 * vastly constricted state space. */
	return pctmRun(as, initCheatGameStateHypothetical, aiRandom);
}
