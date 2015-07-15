#include "mmcheat.h"

/* N.B. Caller must free ret->players */
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

static float testChild(const struct aistate* const restrict as, const uint8_t player, const uint32_t which, const size_t depth, const float alpha, const float beta)
{
	{	assert(as);
		assert(as->pl);
		assert(as->gs);
		assert(as->gs->nplayers >= MINPLRS);
		assert(as->gs->nplayers <= MAXPLRS);
		assert(as->gs->players);}
	float ret;
	struct gamestate is;
	bool eight;
	uint8_t nextplayer = (player + 1) % as->gs->nplayers;

	copyGameState(&is, as->gs);

	if(likely(which != as->pl->n)) {
		glHandleMove(as, &is, 0, &eight, which);
		if(glHandleMagic(&is, 0)) {
			nextplayer = (player + 2) % as->gs->nplayers;
		}
	} else {
		if(!is.drew) {
			drawCard(&is);
			nextplayer = player;
			is.drew = true;
		} else {
			is.turn++;
			is.drew = false;
		}
	}
	ret = minimax(&is, nextplayer, depth, alpha, beta);
	free(is.players);
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
		v = ((player) ? -1.0 : 1.0) * -evalPlayer(stateToPlayer(gs), gs->nplayers);
		return v;
	}

	as.pl = getPotentials(gs, stateToPlayer(gs));
	if(!player) {
		bv = -INFINITY;
		for(i = 0; i <= as.pl->n; i++) {
			v = testChild(&as, player, i, depth, alpha, beta);
			if(v > bv) bv = v;
			alpha = max(alpha, v);
			if(beta <= alpha) break;
		}
	} else {
		bv = INFINITY;
		for(i = 0; i <= as.pl->n; i++) {
			v = testChild(&as, player, i, depth, alpha, beta);
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
		v = testChild(as, 0, i, 6, -INFINITY, INFINITY);
		printf("%sminmax:%s\t%zu\t%.0f\t", ANSI_CYAN, ANSI_DEFAULT, i, v);
		if(i != as->pl->n) {
			showPlay(plistGet(as->pl, i));
			printf("%s %s\n", ANSI_BACK, ANSI_DEFAULT);
		} else {
			printf("\n");
		}
		if(v > bv || (v >= bv && i != as->pl->n)) {
			bv = v;
			best = i;
		}
	}

	MPACK(ret, best);
	ESPACK(ret, Clubs);
	return ret;
}
