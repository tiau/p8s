#include "stacked.h"

__attribute__((hot,nonnull)) static void copyGameState(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	{	assert(gs);
		assert(ogs);}

	memcpy(gs->deck.c, ogs->deck.c, DECKLEN * sizeof(card_t));
	memcpy(gs->players, ogs->players, ogs->nplayers * sizeof(struct player));
	memcpy(gs->draws, ogs->draws, ogs->nplayers * sizeof(uint_fast16_t));
}

/* TODO: tune this; ideas: player's handsize, more than the immediate next player, better function + maxdeals */
__attribute__((hot,nonnull,pure)) static uint_fast8_t scoreDraws(const struct gamestate* const restrict gs)
{
	int_fast8_t deals = MAXDEALS;
	size_t i;

	assert(gs);

	for(i = 0; i < 8; i++)
		deals -= ((gs->draws[gs->turn + 1 % gs->nplayers] >> i) & 3) / (i / 4 + 1);

	if(unlikely(deals < 2))
		deals = 2;

	return deals;
}

void initStackedGameStateHypothetical(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	size_t i;
	uint_fast8_t deals;
	int_fast16_t score, bscore = 32767;
	struct gamestate bgs;

	{	assert(ogs);
		assert(ogs->nplayers >= MINPLRS && ogs->nplayers <= MAXPLRS);}

	initGameStateHypoShared(&bgs, ogs);
	deals = scoreDraws(ogs);

	for(i = 0; ; i++) {
		initGameStateHypoShared(gs, ogs);
		initDeckSans(&gs->deck, stateToPlayer(ogs), &ogs->pile);
		dealStateSans(gs, ogs);
		score = evalPlayer(&gs->players[gs->turn + 1 % gs->nplayers], gs->nplayers);
		if(score < bscore) {
			copyGameState(&bgs, gs);
			bscore = score;
		}
		if(i > deals)
			break;
		cleanGameState(gs);
	}

	copyGameState(gs, &bgs);
	cleanGameState(&bgs);
}

uint_fast32_t aiStacked(const struct aistate* const restrict as)
{
	return pctmRun(as, initStackedGameStateHypothetical, STACKEDAIF, __func__);
}
