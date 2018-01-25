#include "mmcheat.h"

__attribute__((hot,nonnull)) static struct gamestate* copyGameState(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	{	assert(gs);
		assert(ogs);}

	*gs = *ogs;
	gs->deck.top = gs->deck.c + (ogs->deck.top - ogs->deck.c);
	gs->pile.top = gs->pile.c + (ogs->pile.top - ogs->pile.c);
	return gs;
}

__attribute__((hot,nonnull,pure)) static float minimax(const struct gamestate* const restrict gs, size_t depth, float alpha, float beta);

__attribute__((hot,nonnull,pure)) static float testNode(const struct aistate* const restrict as, uint32_t which, size_t depth, const float alpha, const float beta)
{
	struct gamestate is;
	bool eight;

	{	assert(as);
		assert(as->pl);
		assert(as->gs);
		assert(as->gs->nplayers >= MINPLRS);
		assert(as->gs->nplayers <= MAXPLRS);}

	copyGameState(&is, as->gs);
	if(likely(which != as->pl->n)) {
		/* TODO: Handle 8s instead of always having their given suit follow */
		ESPACK(which, getSuit(plistGet(as->pl, which)->c[plistGet(as->pl, which)->n-1]));
		glHandleMove(as, &is, 0, &eight, which);
		if(unlikely(glHandleMagic(&is, 0)))
			is.turn++;
	} else if(!is.drew) {
		is.drew = true;
		if(likely(drawCard(&is)))
			is.turn--;
	}
	is.turn++;
	return minimax(&is, depth, alpha, beta);
}

static float minimax(const struct gamestate* const restrict gs, size_t depth, float alpha, float beta)
{
	size_t i;
	float v;
	struct aistate as = { .gs = gs };
	uint_fast8_t considerDraw;

	for(i = 0; i < gs->nplayers; i++)
		if(!gs->players[i].n)
			return i ? -INFINITY : INFINITY;

	/* TODO: evaluate all plays here, instead of just the players' hands */
	if(!--depth) {
		v = 0.0;
		for(i = 0; i < gs->nplayers; i++)
			v += (i ? 0.1 : -1.0) * evalPlayer(&gs->players[i], gs->nplayers);
		return v;
	}

	considerDraw = (gs->drew || stateToPlayer(gs)->n < MaxCards);

	as.pl = getPotentials(gs, stateToPlayer(gs));
	if(!(gs->turn % gs->nplayers)) {
		v = -INFINITY;
		for(i = 0; i < as.pl->n + considerDraw; i++) {
			v = maxf(v, testNode(&as, i, depth, alpha, beta));
			alpha = maxf(alpha, v);
			if(beta <= alpha) break;
		}
	} else {
		v = INFINITY;
		for(i = 0; i < as.pl->n + considerDraw; i++) {
			v = minf(v, testNode(&as, i, depth, alpha, beta));
			beta = minf(beta, v);
			if(beta <= alpha) break;
		}
	}
	plistDel(as.pl);
	return v;
}

uint_fast32_t aiMmCheat(const struct aistate* const restrict as)
{
	size_t i, best = 0;
	float v, bv = -INFINITY;
	uint_fast32_t ret = 0;
	struct gamestate igs;
	struct aistate ias = { .gs = &igs };
	const uint_fast8_t considerDraw = (as->gs->drew || stateToPlayer(as->gs)->n < MaxCards);

	initCheatGameStateHypothetical(&igs, as->gs);
	ias.pl = as->pl;

	for(i = 0; i < as->pl->n + considerDraw; i++) {
		v = testNode(&ias, i, MaxPlies, -INFINITY, INFINITY);
#ifdef MONTE_VERBOSE
		printf("%sminmax:%s\t%zu\t%.1f\t", ANSI_CYAN, ANSI_DEFAULT, i, v);
		if(i != as->pl->n) {
			showPlay(plistGet(as->pl, i));
			printf("%s %s", ANSI_BACK, ANSI_DEFAULT);
		}
		printf("\n");
#endif
		/* Plays are ordered by size; this selects the quickest way to win */
		if(v > bv || (v >= bv && i != as->pl->n)) {
			bv = v;
			best = i;
		}
	}

	MPACK(ret, best);
	ESPACK(ret, Clubs);
	if(best != as->pl->n)
		ESPACK(ret, getSuit(plistGet(as->pl, best)->c[plistGet(as->pl, best)->n-1]));
	return ret;
}
