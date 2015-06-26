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

void initGameState(struct gamestate* const restrict gs, const size_t nplayers, const uint8_t* const restrict ai)
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
	gs->eightSuit = UNKNOWN;
	gs->magic = false;
	gs->ai = ai;
}

void cleanGameState(struct gamestate* const restrict gs)
{
	free(gs->players);
	gs->deck.top = gs->deck.c;
	gs->pile.top = gs->pile.c;
	gs->deck.n = 0;
	gs->pile.n = 0;
}

int getGameState(const struct gamestate* const restrict gs)
{
	assert(gs);

	if(gs->nplayers >= MINPLRS && gs->nplayers <= MAXPLRS) {
		size_t i;
		for(i = 0; i < gs->nplayers; i++)
			if(!gs->players[i].n)
				return CONCLUDED;
		return INPROGRESS;
	}
	return NOTSTARTED;
}

bool isPlayLegal(const struct play* const restrict play)
{
	{	assert(play);
		assert(play->n);}

	switch(play->n) {
		case 1:
			return true;
		case 2:
			return isPair(play);
		case 3:
			return isThreeOAK(play);
		case 4:
			return isTwoPair(play) ||
				   isFourOAK(play);
		case 5:
			return isFlush(play) ||		// Straights are more common, but flushes
				   isStraight(play) ||	// are faster to check
				   isFullHouse(play);
		default:
			assert(false);
	}
	assert(false);
	return false; // Fixes warning, we never get here
}

bool isCardOneLegal(const struct gamestate* const restrict gs, const struct play* const restrict play)
{
	card_t v;

	{	assert(gs);
		assert(play);
		assert(play->n);}

	v = getVal(*play->c);
	return ((getSuit(*play->c) == ((gs->eightSuit == UNKNOWN) ? getSuit(*gs->pile.top) : gs->eightSuit)) ||
			(v == getVal(*gs->pile.top)) ||
			(v == 8));
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
