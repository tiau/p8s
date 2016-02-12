#include "human.h"

__attribute__((cold)) inline static uint_fast8_t humanCardHash(const struct play* const restrict play)
{
	size_t i;
	uint_fast8_t hash = 0;

	for(i = 0; i < play->n; i++)
		hash ^= play->c[i];

	return hash;
}

uint_fast32_t human(const struct aistate* const restrict as)
{
	uint_fast32_t tm = 0, ret = 0;
	size_t i, dc;
	struct player lp;
	const struct play* play;
	char b[5];
	uint_fast8_t th, lh = 255;

	{	assert(as);
		assert(as->gs);
		assert(as->pl);
		assert(as->gs->nplayers);
		assert(as->gs->players);}

	size_t cih[as->gs->nplayers];
	populateCIH(as->gs, cih);
	const struct player* const restrict player = stateToPlayer(as->gs);
	// TODO: fix this horrible hack, write some output methods, anything really
	printf("HAND %s", ANSI_WHITE);
	lp.n = player->n;
	memcpy(lp.c, player->c, player->n);
	orderHand(&lp);
	showHand(&lp);
	printf("#CARDS\t\t%s", ANSI_WHITE);
	for(i = 0, dc = 0; i < as->gs->nplayers-1; i++) {
		printf("%zu%s ", cih[i], (i == as->gs->nplayers-2) ? "" : ",");
		dc += cih[i];
	}
	printf("\n%sPILE ", ANSI_DEFAULT);
	showPile(&as->gs->pile);
	printf("%sDECK %s(%zu cards)%s\n", ANSI_DEFAULT, ANSI_WHITE, DECKLEN - as->gs->pile.n - player->n - dc, ANSI_DEFAULT);

	printf("MOVES%s\t%s", ANSI_WHITE, ANSI_DEFAULT);
	for(i = 0; i < as->pl->n; i++) {
		play = plistGet(as->pl, i);
		th = humanCardHash(play);
		if(lh != th)
			printf("\n\t");
		lh = th;
		printf("%zu: ", i);
		showPlay(play);
	}
	printf("\n");

	for(;;) {
		memset(b, 0, 5);
		printf("move -> ");
		if(!fgets(b, 4, stdin))
			continue;
		MPACK(ret, as->pl->n);
		if(*b == 'd')
			return ret;
		else if(*b == 'h')
			tm = aiStacked(as);
		else if(*b == 'H')
			tm = aiCheat(as);
		else if(*b == '?')
			tm = aiMonte(as);
		if(*b == 'h' || *b == 'H' || *b == '?') {
			tm = MUPACK(tm);
			if(tm != as->pl->n) {
				printf("AI recommends\t%s%zu%s :: ", ANSI_WHITE, tm, ANSI_DEFAULT);
				showPlay(plistGet(as->pl, tm));
				printf("%s \n", ANSI_BACK);
			}
			continue;
		}
		if(*b == 'Q') {
			exit(0);
		}
		size_t n = atoi(b);
		if(n >= as->pl->n || !isdigit(*b)) {
			printf("You must enter a play number or 'd'\n");
			continue;
		}
		MPACK(ret, n);
		play = plistGet(as->pl, n);
		if(getVal(play->c[play->n-1]) == 8) {
			printf("suit -> ");
			for(;;) {
				if(!fgets(b, 2, stdin))
					continue;
				switch(tolower(*b)) {
					case 's':
						ESPACK(ret, Spades);
						break;
					case 'h':
						ESPACK(ret, Hearts);
						break;
					case 'd':
						ESPACK(ret, Diamonds);
						break;
					case 'c':
						ESPACK(ret, Clubs);
						break;
					default:
						printf("You must enter a suit\n");
						continue;
				}
				break;
			}
		}
		break;
	}
	printf("\n");
	return ret;
}
