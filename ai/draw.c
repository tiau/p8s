#include "draw.h"

void initDeckSans(struct deck* const restrict deck, const struct player* const restrict player, const struct deck* const restrict pile)
{
	{	assert(deck);
		assert(player);
		assert(player->n + 1 < DECKLEN);
		assert(pile);}

	size_t i, j;
	const size_t as = pile->n + player->n;
	card_t tp[as];

	memcpy(tp, pile->c, sizeof(card_t)*pile->n);
	memcpy(tp + pile->n, player->c, sizeof(card_t)*player->n);
	qsort(tp, as, sizeof(card_t), cmpcardt);
	deck->n = DECKLEN - as;
	for(i = 1, j = as; i < DECKLEN+1; i++)
		if(!bsearch(&i, tp, as, sizeof(card_t), cmpcardt))
			deck->c[j++] = i;
	cshuffle(deck->c, as, DECKLEN);
	deck->top = deck->c + as;
}

uint_fast32_t aiDraw(const struct aistate* const restrict as)
{
	struct player *p, tp;
	struct deck td;
	ssize_t cur;
	suit_t fs;
	card_t c;
	uint_fast32_t ret;

	{	assert(as);
		assert(as->gs);
		assert(as->pl);
		assert(as->pl->n);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);}

	ret = aiJudge(as);
	p = stateToPlayer(as->gs);
	cur = evalPlayer(p, as->gs->nplayers);

	initDeckSans(&td, p, &as->gs->pile);
	float hmb = 0.0f;
	const float ds = td.n;
	suit_t ns = getSuit(*as->gs->pile.top);
	card_t nv = getVal(*as->gs->pile.top);
	if(!as->gs->drew) {
		while(td.n--) {
			tp = *p;
			tp.c[tp.n++] = *td.top++;
			hmb += (evalPlayer(&tp, as->gs->nplayers) + 5 < cur);
		}

	/* We can't draw, so only consider passing if we can only make one play and
	 * that play ends in an eight and the next player has more than one card */
	} else if(as->pl->n == 1 &&
			  getVal(plistGet(as->pl, 0)->c[0]) == 8 &&
			  as->gs->players[(p - as->gs->players) % as->gs->nplayers].n > 1) {
		while(td.n--) {
			c = *td.top++;
			fs = getSuit(c);
			hmb += 0.15 + (fs == as->gs->eightSuit ||
					      (as->gs->eightSuit == Unknown && fs == ns) ||
					      getVal(c) == 8 ||
					      getVal(c) == nv);
		}
	}

	if(hmb/ds > 0.5f)
		MPACK(ret, as->pl->n);
#ifdef JUDGE_VERBOSE
	printf("%sdraw:%s\t%zu\t score \t%.2f\t%s\n", ANSI_CYAN, ANSI_DEFAULT, as->pl->n, hmb/ds, (as->gs->drew) ? "draw" : "pass");
#endif
	return ret;
}
