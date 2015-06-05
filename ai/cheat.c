#include "cheat.h"

void initCheatGameStateHypothetical(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	{	assert(gs);
		assert(ogs);
		assert(ogs->players);
		assert(ogs->nplayers >= MINPLRS && ogs->nplayers <= MAXPLRS);}

	size_t i;

	gs->deck = ogs->deck;
	gs->deck.top = gs->deck.c + (ogs->deck.top - ogs->deck.c);
	cshuffle(gs->deck.c, 0, gs->deck.top - gs->deck.c);
	gs->pile = ogs->pile;
	gs->pile.top = gs->pile.c + ogs->pile.n - 1;
	gs->nplayers = ogs->nplayers;
	gs->players = calloc(gs->nplayers, sizeof(struct player));
	assert(gs->players);
	for(i = 0; i < gs->nplayers; i++)
		gs->players[i] = ogs->players[i];
	gs->turn = 0;
	gs->eightSuit = ogs->eightSuit;
	gs->drew = ogs->drew;
	gs->magic = false;
}

uint_fast32_t aiCheat(const struct aistate* const restrict as)
{

	/* Since Cheat doesn't randomize anything, it needs to use a
	 * nondeterministic AI as to not run into the case where it just plays the
	 * same game over and over with a cheating aiJudge, for instance.  Using
	 * aiRandom also allows it to more thoroughly explore its vastly
	 * constricted state space. */
	return pctmRun(as, initCheatGameStateHypothetical, aiRandom);
}
