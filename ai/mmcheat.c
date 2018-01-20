#include "mmcheat.h"

static struct gamestate* copyGameState(struct gamestate* const restrict ret, const struct gamestate* const restrict gs)
{
	{	assert(ret);
		assert(gs);}

	initCheatGameStateHypothetical(ret, gs);
	ret->turn = gs->turn;
	memcpy(ret->players, gs->players, gs->nplayers * sizeof(struct player));
	return ret;
}

static float minimax(const struct gamestate* const restrict gs, const uint8_t player, size_t depth, float alpha, float beta);

static float testNode(const struct aistate* const restrict as, uint8_t player, const uint32_t which, size_t depth, const float alpha, const float beta)
{
	{	assert(as);
		assert(as->pl);
		assert(as->gs);
		assert(as->gs->nplayers >= MINPLRS);
		assert(as->gs->nplayers <= MAXPLRS);
		assert(player < as->gs->nplayers);}
	float ret;
	struct gamestate is;
	bool eight;

	copyGameState(&is, as->gs);

	player++;
	if(which != as->pl->n) {
		glHandleMove(as, &is, 0, &eight, which);
		if(unlikely(glHandleMagic(&is, 0))) {
			player++;
		}
	} else {
		if(!is.drew) {
			drawCard(&is);
			player--;
			depth++;
		} else {
			is.turn++;
		}
		is.drew = !is.drew;
	}
	ret = minimax(&is, player % as->gs->nplayers, depth, alpha, beta);
	return ret;
}

static float minimax(const struct gamestate* const restrict gs, const uint8_t player, size_t depth, float alpha, float beta)
{
	size_t i;
	float v, bv;
	struct aistate as = { .gs = gs };

	for(i = 0; i < gs->nplayers; i++)
		if(!gs->players[i].n)
			return (i) ? -INFINITY : INFINITY;

	if(!--depth) {
		v = 0;
		for(i = 0; i < gs->nplayers; i++)
			v += (i ? 1.0 : -0.5) * evalPlayer(&gs->players[i], gs->nplayers);
		return v;
	}

	as.pl = getPotentials(gs, stateToPlayer(gs));
	if(!player) {
		bv = -INFINITY;
		for(i = 0; i <= as.pl->n; i++) {
			v = testNode(&as, player, i, depth, alpha, beta);
			if(v > bv) bv = v;
			alpha = max(alpha, v);
			if(beta <= alpha) break;
		}
	} else {
		bv = INFINITY;
		for(i = 0; i <= as.pl->n; i++) {
			v = testNode(&as, player, i, depth, alpha, beta);
			if(v < bv) bv = v;
			beta = min(beta, v);
			if(beta <= alpha) break;
		}
	}
	plistDel(as.pl);
	return bv;
}

uint_fast32_t aiMmCheat(const struct aistate* const restrict as)
{
	size_t i, best = 0;
	float v, bv = -INFINITY;
	uint_fast32_t ret = 0;

	for(i = 0; i <= as->pl->n; i++) {
		v = testNode(as, 0, i, MMDEPTH, -INFINITY, INFINITY);
#ifdef MONTE_VERBOSE
		printf("%sminmax:%s\t%zu\t%.0f\t", ANSI_CYAN, ANSI_DEFAULT, i, v);
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
	return ret;
}
