#include "io.h"

void showSuit(const suit_t c)
{
	switch(c) {
		case 0:
			printf("%sC%s", ANSI_GRAY, ANSI_DEFAULT);
			break;
		case 1:
			printf("%sD%s", ANSI_RED, ANSI_DEFAULT);
			break;
		case 2:
			printf("%sH%s", ANSI_RED, ANSI_DEFAULT);
			break;
		case 3:
			printf("%sS%s", ANSI_GRAY, ANSI_DEFAULT);
			break;
		case 4:
			printf("%sU%s", ANSI_YELLOW, ANSI_DEFAULT);
			break;
		default:
			assert(false);
	}
}

void showCard(const card_t c)
{
	const suit_t s = getSuit(c);
	const unsigned int cv = getVal(c);

	printf("%s", (!s || s == 3) ? ANSI_GRAY : ANSI_RED);
	if(cv != 1 && cv < 11) {
		printf("%u", cv);
	} else {
		switch(cv) {
			case 1: printf("%c", 'A'); break;
			case 13: printf("%c", 'K'); break;
			case 12: printf("%c", 'Q'); break;
			case 11: printf("%c", 'J'); break;
		}
	}
	showSuit(s);
	printf(", ");
}

void showPlay(const struct play* const restrict play)
{
	size_t i;

	assert(play);
	assert(play->n);

	for(i = 0; i < play->n; i++)
		showCard(play->c[i]);
	printf("%s | ", ANSI_BACK);
}

void showHand(const struct player* const restrict player)
{
	size_t i;

	assert(player);

	printf("%s(%zu cards)\t", ANSI_WHITE, player->n);
	for(i = 0; i < player->n; i++)
		showCard(player->c[i]);
	printf("%s \n", ANSI_BACK);
}

void showDeck(const struct deck* const deck)
{
	const card_t* c;

	assert(deck);

	printf("%s(%zu cards)\t", ANSI_WHITE, deck->n);
	for(c = deck->top; c < deck->n + deck->top; c++)
		showCard(*c);
	printf("%s \n", ANSI_BACK);
}

void showPile(const struct deck* const pile)
{
	const card_t* c;

	assert(pile);

	printf("%s(%zu cards)\t", ANSI_WHITE, pile->n);
	for(c = pile->c; c <= pile->top; c++)
		showCard(*c);
	printf("%s%s*%s\n", ANSI_BACK, ANSI_WHITE, ANSI_DEFAULT);
}

void showGameState(const struct gamestate* const restrict gs)
{
	assert(gs);

	switch(getGameState(gs)) {
		case CONCLUDED:
			printf("%sConcluded", ANSI_DRED);
			break;
		case INPROGRESS:
			printf("%sIn progress", ANSI_GREEN);
			break;
		case NOTSTARTED:
			printf("%sNot started", ANSI_YELLOW);
			break;
		default:
			assert(false);
	}
	printf(", %sTurn %s%zu%s (player %s%zu%s)\n", ANSI_DEFAULT, ANSI_WHITE, gs->turn, ANSI_DEFAULT, ANSI_WHITE, (gs->turn - (!getGameState(gs) ? 1 : 0)) % gs->nplayers, ANSI_DEFAULT);
}
