#include "engine.h"

void populateCIH(const struct gamestate* const restrict gs, size_t* const restrict cih)
{
	size_t i, j;

	{	assert(gs);
		assert(cih);}

	/* The player after the current player always occupies the first slot */
	for(i = (gs->turn + 1) % gs->nplayers,	j = 0; j < gs->nplayers; i = (i + 1) % gs->nplayers, j++)
		cih[j] = (i != (gs->turn % gs->nplayers)) ? gs->players[i].n : 0;

	assert(cih[0]);
}

void cshuffle(card_t* const restrict cards, const size_t s, const size_t e)
{
	size_t i;

	{	assert(cards);
		assert(s <= e);}

	for(i = s; i < e; i++)
		cardSwap(&cards[i], &cards[(rand() % (e-s)) + s]);
}

__attribute__((nonnull,hot)) inline static void shuffle(struct deck* const restrict deck)
{
	assert(deck);

	cshuffle(deck->c, 0, deck->n);
	deck->top = deck->c;
}

__attribute__((nonnull,hot)) inline static void initDeck(struct deck* const restrict deck)
{
	size_t i;

	assert(deck);

	deck->n = DECKLEN;
	for(i = 1; i < DECKLEN+1; i++)
		deck->c[i-1] = i;
	shuffle(deck);
}

__attribute__((nonnull,hot)) inline static void deal(struct gamestate* const restrict gs)
{
	size_t i, p;

	{	assert(gs);
		assert(gs->players);}

	for(i = 0; i < gs->nplayers * CPP; i++) {
		p = i % gs->nplayers;
		gs->players[p].c[gs->players[p].n++] = *gs->deck.top++;
		gs->deck.n--;
	}
}

void initGameState(struct gamestate* const restrict gs, const size_t nplayers, uint_fast32_t (* const ai[MAXPLRS])(const struct aistate* const restrict))
{
	{	assert(gs);
		assert(nplayers >= MINPLRS && nplayers <= MAXPLRS);}

	initDeck(&gs->deck);
	initPile(&gs->pile);
	gs->players = calloc(nplayers, sizeof(struct player));
	assert(gs->players);
	gs->nplayers = nplayers;
	deal(gs);
	*gs->pile.top = *gs->deck.top++;
	gs->pile.n++;
	gs->deck.n--;
	gs->turn = 0;
	gs->eightSuit = Unknown;
	gs->magic = false;
	gs->drew = false;
	memcpy(gs->ai, ai, nplayers * sizeof(uint_fast32_t (*const)(const struct aistate* const restrict)));
}

void cleanGameState(struct gamestate* const restrict gs)
{
	free(gs->players);
	gs->deck.top = gs->deck.c;
	gs->pile.top = gs->pile.c;
	gs->deck.n = 0;
	gs->pile.n = 0;
}

__attribute__((nonnull,hot)) static void removeCard(struct player* const restrict player, const card_t c)
{
	card_t* i;

	{	assert(player);
		assert(player->n);
		assert(player->n + 1 < DECKLEN);
		assert(c);}

	i = (card_t*)lfind(&c, player->c, &player->n, sizeof(card_t), cmpcardt);
	assert(i);
	if(likely(i < &player->c[player->n--]))
		*i = player->c[player->n];
}

void removeCards(struct player* const restrict player, const struct play* const restrict play)
{
	size_t i;

	{	assert(player);
		assert(play);
		assert(play->n);}

	for(i = 0; i < play->n; i++)
		removeCard(player, play->c[i]);
}

void makeMove(struct gamestate* const restrict gs, const struct play* const restrict play)
{
	size_t i;

	{	assert(gs);
		assert(play);
		assert(isPlayLegal(play));
		assert(isCardOneLegal(gs, play));
		assert(gs->pile.n + play->n < DECKLEN);}

	for(i = 0; i < play->n; i++) {
		removeCard(stateToPlayer(gs), play->c[i]);
		*++gs->pile.top = play->c[i];
		gs->pile.n++;
	}
}

bool playerDrawCard(struct gamestate* const gs, struct player* const player)
{
	{	assert(gs);
		assert(player);
		assert(player->n + 1 < DECKLEN);
		assert(gs->deck.n);
		assert((size_t)(gs->deck.top - gs->deck.c) == DECKLEN - gs->deck.n);}

	if(!gs->deck.n)
		return false;

	player->c[player->n++] = *gs->deck.top++;
	gs->deck.n--;
	return true;
}

bool drawCard(struct gamestate* const restrict gs)
{
	assert(gs);
	const bool r = playerDrawCard(gs, stateToPlayer(gs));

	/* If we run out of draw cards, shuffle all but the top of the pile back
	 * into the deck */
	if(!gs->deck.n && gs->pile.n > 1) {
		cardSwap(gs->pile.c, &gs->pile.c[gs->pile.n-1]);
		memcpy(gs->deck.c + (DECKLEN - gs->pile.n+1), gs->pile.c + 1, sizeof(card_t)*(gs->pile.n - 1));
		gs->pile.top = gs->pile.c;
		gs->deck.n = gs->pile.n - 1;
		gs->deck.top = gs->deck.c + (DECKLEN - gs->deck.n);
		gs->pile.n = 1;
		cshuffle(gs->deck.c, DECKLEN - gs->deck.n, DECKLEN);
	}

	return r;
}

__attribute__((nonnull,cold)) inline static void glShowStatus(struct gamestate* const restrict gs, const uint8_t verbose, const size_t offset)
{
	if(unlikely(verbose)) {
		showGameState(gs, offset);
		if(verbose > 1) {
			printf("Deck ");
			showDeck(&gs->deck);
			printf("Pile ");
			showPile(&gs->pile);
			for(size_t i = 0; i < gs->nplayers; i++) {
				orderHand(&gs->players[i]);
				showHand(&gs->players[i]);
			}
		}
	}
}

bool glHandleMagic(struct gamestate* const restrict gs, const uint8_t verbose)
{
	card_t tc;

	if(unlikely(gs->magic)) {
		tc = getVal(*gs->pile.top);
		gs->magic = false;

		if(tc == 2) {
			drawCard(gs);
			drawCard(gs);
			if(unlikely(verbose)) {
				struct player* player = stateToPlayer(gs);
				printf("%s2 played, drew:%s ", ANSI_BLUE, ANSI_DEFAULT);
				showCard(player->c[player->n-2]);
				showCard(player->c[player->n-1]);
				printf("%s \n", ANSI_BACK);
			}
		} else if(tc == 3) {
			if(unlikely(verbose)) printf("%s3 played, skipping turn%s\n\n", ANSI_BLUE, ANSI_DEFAULT);
			gs->turn++;
			return true;
		}
	}
	return false;
}

uint32_t glEvalMoves(struct aistate* const restrict as, struct gamestate* const gs, const uint8_t verbose)
{
	uint32_t ret = 0;

	for(;;) {
		as->pl = getPotentials(gs, stateToPlayer(gs));

		if(as->pl->n) {
			if(unlikely(verbose > 2 && as->pl->n)) {
				printf("Potentials %s(%zu)\t", ANSI_WHITE, as->pl->n);
				for(size_t i = 0; i < as->pl->n; i++)
					showPlay(plistGet(as->pl, i));
				printf("%s \n", ANSI_BACK);
			}
			ret = (*gs->ai[gs->turn % gs->nplayers])(as);
		} else {
			MPACK(ret, 0);
		}

		if(gs->drew || MUPACK(ret) != as->pl->n)
			break;
		if(unlikely(verbose)) printf("%s%s drawing%s\n", ANSI_BLUE, as->pl->n ? "Voluntary" : "Forced", ANSI_DEFAULT);
		if(unlikely(!drawCard(gs)))
			if(unlikely(verbose)) printf("%sCould not draw card%s\n", ANSI_BLUE, ANSI_DEFAULT);
		gs->drew = true;
		plistDel(as->pl);
	}
	return ret;
}

void glHandleMove(const struct aistate* const restrict as, struct gamestate* const gs, const uint8_t verbose, bool* const restrict eight, uint32_t ptm)
{
	if(as->pl->n) {
		size_t move = MUPACK(ptm);
		if(move != as->pl->n) {
			const struct play* play = plistGet(as->pl, move);
			*eight = (getVal(play->c[play->n-1]) == 8);
			if(unlikely(verbose)) {
				printf("%sPlaying%s ", ANSI_BLUE, ANSI_DEFAULT);
				showPlay(play);
				printf("%s \n", ANSI_BACK);
			}
			makeMove(gs, play);
			if(unlikely(*eight)) {
				gs->eightSuit = ESUPACK(ptm);
				if(unlikely(verbose)) {
					printf("%sSuit is now%s ", ANSI_BLUE, ANSI_DEFAULT);
					showSuit(gs->eightSuit);
					printf("\n");
				}
			}
			gs->magic = isMagicCard(*gs->pile.top);
		} else {
			if(unlikely(verbose)) printf("%sVoluntary passing%s\n", ANSI_BLUE, ANSI_DEFAULT);
		}
	} else {
		if(!gs->drew) {
			if(unlikely(verbose)) printf("%sForced drawing%s\n", ANSI_BLUE, ANSI_DEFAULT);
			if(unlikely(!drawCard(gs))) {
				if(unlikely(verbose)) printf("%sCould not draw, forced passing%s\n", ANSI_BLUE, ANSI_DEFAULT);
			}
		} else {
			if(unlikely(verbose)) printf("%sForced passing%s\n", ANSI_BLUE, ANSI_DEFAULT);
		}
	}
	if(unlikely(verbose)) printf("\n");
	gs->turn++;
	gs->drew = false;
}

float gameLoop(struct gamestate* const restrict gs, const uint8_t verbose, bool eight, const size_t offset)
{
	uint32_t ptm;
	struct aistate as = { .gs = gs };

	{	assert(gs);
		assert(gs->players);}

	while(getGameState(gs)) {
		if(likely(!eight)) gs->eightSuit = Unknown;
		glShowStatus(gs, verbose, offset);
		if(glHandleMagic(gs, verbose))
			continue;
		ptm = glEvalMoves(&as, gs, verbose);
		glHandleMove(&as, gs, verbose, &eight, ptm);
		plistDel(as.pl);
	}
	return (((gs->turn - 1) % gs->nplayers) ? -1.0 * gs->turn : gs->turn);
}
