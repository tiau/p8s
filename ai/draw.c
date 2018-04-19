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
	card_t c;
	uint_fast32_t ret;
	size_t cih[as->gs->nplayers];

	{	assert(as);
		assert(as->gs);
		assert(as->pl);
		assert(as->pl->n);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);}

	ret = aiJudge(as);

	/* Don't draw or pass if playing will win */
	if(unlikely(plistGet(as->pl, MUPACK(ret))->n == stateToPlayer(as->gs)->n))
		return ret;

	p = stateToPlayer(as->gs);
	populateCIH(as->gs, cih);
	cur = evalPlayer(p, as->gs->nplayers, cih[0]);

	initDeckSans(&td, p, &as->gs->pile);
	float hmb = 0.0f;
	const float ds = td.n;
	suit_t ns = getSuit(*as->gs->pile.top);
	card_t nv = getVal(*as->gs->pile.top);
	if(!as->gs->drew) {
		hmb += 18;
		/* Count how many of the possible draws will make our hand better */
		while(td.n--) {
			tp = *p;
			tp.c[tp.n++] = *td.top++;
			hmb += (evalPlayer(&tp, as->gs->nplayers, cih[0]) + DrawThreshold < cur);
		}
	/* We can't draw, don't even consider passing if there are only two players
	 * and they just played an 8 */
	} else {
		hmb -= 2 - .012*(as->gs->nplayers != 2 || as->gs->eightSuit == Unknown);
		const struct play* const restrict ptm = plistGet(as->pl, MUPACK(ret));
		const suit_t playsuit = getVal(ptm->c[ptm->n-1]) == 8 ? ESUPACK(ret) : getSuit(ptm->c[ptm->n-1]);
		const suit_t passsuit = as->gs->eightSuit == Unknown ? ns : as->gs->eightSuit;
		const card_t playval = getVal(ptm->c[ptm->n-1]);
		const card_t passval = nv;
		/* If nothing we would do would change anything, it doesn't matter what
		 * we do. TODO: consider getting a new move in this case */
		if(likely(playsuit != passsuit || playval != passval || isMagicCard(playval))) {
			suit_t lsuit;
			card_t lval;
			/* Count how often passing tends to help/hurt */
			while(td.n--) {
				c = *td.top++;
				lsuit = getSuit(c);
				lval = getVal(c);
				if(unlikely(lval != 8))
					hmb += (lsuit == playsuit && lsuit != passsuit) * PlaySuitMult
						-  (lsuit == passsuit && lsuit != playsuit) * PassSuitMult
						+  (lval == playval && lval != passval) * PlayValMult
						-  (lval == passval && lval != playval) * PassValMult;
			}
		}
	}

	if(unlikely(hmb/ds > 0.5f))
		MPACK(ret, as->pl->n);
#ifdef JUDGE_VERBOSE
	printf("%sdraw:%s\t%zu\t\t\t%.2f\t%s\n", ANSI_CYAN, ANSI_DEFAULT, as->pl->n, hmb/ds, (as->gs->drew) ? "pass" : "draw");
#endif
	return ret;
}
