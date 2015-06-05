#include "stacked.h"

// TODO: we could do a lot more than just stack the deck via magic cards and eights, people are also likely to be holding pairs, straights, flushes, etc.
void stackTheDeck(struct deck* const restrict deck, const size_t tch)
{
	size_t i, j, o, ds;
	size_t open[tch];
	card_t t;

	{	assert(deck);
		assert(tch);}

	if(tch < deck->n) {
		/* Find open slots for magic cards to go in the first tch cards */
		ds = DECKLEN - deck->n;
		for(i = ds, o = 0; i < (tch + ds) && i < (deck->n + ds); i++) {
			t = getVal(deck->c[i]);
			if(t != 2 && t != 3 && t != 8)
				open[o++] = i;
		}

		/* 10% chance to move magic cards to opponents' hands, 20% for eights */
		for(i = tch + ds, j = 0; i < deck->n && j < o; i++) {
			t = getVal(deck->c[i]);
			if(((t == 2 || t == 3) && rand() % 10 == 0) || (t == 8 && rand() % 5 == 0))
				cardSwap(&deck->c[i], &deck->c[open[j++]]);
		}
	}
}

void initStackedGameStateHypothetical(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	size_t i, tch = 0;

	{	assert(gs);
		assert(ogs);
		assert(ogs->nplayers >= MINPLRS && ogs->nplayers <= MAXPLRS);
		assert(ogs->players);}

	/* Calculate how many cards our opponents hold, for deck stacking */
	size_t cih[ogs->nplayers];
	populateCIH(ogs, cih);
	for(i = 0; i < ogs->nplayers-1; i++)
		tch += cih[i];

	initDeckSans(&gs->deck, stateToPlayer(ogs), &ogs->pile);
	gs->pile = ogs->pile;
	gs->pile.top = gs->pile.c + ogs->pile.n - 1;
	gs->players = calloc(ogs->nplayers, sizeof(struct player));
	assert(gs->players);
	gs->nplayers = ogs->nplayers;
	stackTheDeck(&gs->deck, tch);
	dealStateSans(gs, ogs);
	/* Cannot run a hypothetical game from before anything has happened */
	assert(gs->pile.n);
	gs->turn = 0;
	gs->eightSuit = ogs->eightSuit;
	gs->drew = ogs->drew;
	gs->magic = false;
}

uint_fast32_t aiStacked(const struct aistate* const restrict as)
{
	return pctmRun(as, initStackedGameStateHypothetical, STACKEDAIF);
}
