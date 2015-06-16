#include "p8.h"

__attribute__((cold,nonnull)) static void gameLoop(struct gamestate* const restrict gs, const uint8_t verbose)
{
	size_t i;
	const struct play* play;
	bool eightLastTurn = false;
	card_t tc;
	uint32_t ptm = 0;
	struct aistate as = { .gs = gs };

	assert(gs);
	assert(gs->players);
	
	gs->magic = false;
	gs->eightSuit = UNKNOWN;

	while(getGameState(gs)) {
		if(!eightLastTurn) gs->eightSuit = UNKNOWN;
		gs->drew = !gs->deck.n;

		if(verbose) {
			showGameState(gs);
			if(verbose > 1) {
				printf("Deck ");
				showDeck(&gs->deck);
				printf("Pile ");
				showPile(&gs->pile);
				for(i = 0; i < gs->nplayers; i++) {
					orderHand(&gs->players[i]);
					showHand(&gs->players[i]);
				}
			}
		}

		if(gs->magic) {
			tc = getVal(*gs->pile.top);
			gs->magic = false;

			if(tc == 2) {
				if(verbose) printf("%s2 played, drawing 2%s\n", ANSI_BLUE, ANSI_DEFAULT);
				drawCard(gs);
				drawCard(gs);
			} else if(tc == 3) {
				if(verbose) printf("%s3 played, skipping turn%s\n\n", ANSI_BLUE, ANSI_DEFAULT);
				gs->turn++;
				continue;
			}
		}

		for(;;) {
			as.pl = getPotentials(gs, stateToPlayer(gs));

			if(as.pl->n) {
				if(verbose > 2 && as.pl->n) {
					printf("Potentials %s(%zu)\t", ANSI_WHITE, as.pl->n);
					for(i = 0; i < as.pl->n; i++)
						showPlay(plistGet(as.pl, i));
					printf("%s \n", ANSI_BACK);
				}
				ptm = (*ais[gs->ai[gs->turn % gs->nplayers]])(&as);
			} else {
				MPACK(ptm, 0);
			}

			if(gs->drew || MUPACK(ptm) != as.pl->n)
				break;
			if(verbose) printf("%s%s drawing%s\n", ANSI_BLUE, as.pl->n ? "Voluntary" : "Forced", ANSI_DEFAULT);
			if(!drawCard(gs))
				if(verbose) printf("%sCould not draw card%s\n", ANSI_BLUE, ANSI_DEFAULT);
			gs->drew = true;
			plistDel(as.pl);
		}

		if(as.pl->n) {
			i = MUPACK(ptm);
			if(i != as.pl->n) {
				play = plistGet(as.pl, i);
				eightLastTurn = (getVal(play->c[play->n-1]) == 8);
				if(verbose) {
					printf("%sPlaying%s ", ANSI_BLUE, ANSI_DEFAULT);
					showPlay(play);
					printf("%s \n", ANSI_BACK);
				}
				makeMove(gs, play);
				if(eightLastTurn) {
					gs->eightSuit = ESUPACK(ptm);
					if(verbose) {
						printf("%sSuit is now%s ", ANSI_BLUE, ANSI_DEFAULT);
						showSuit(gs->eightSuit);
						printf("\n");
					}
				}
				gs->magic = isMagicCard(*gs->pile.top);
			} else {
				if(verbose) printf("%sVoluntary passing%s\n", ANSI_BLUE, ANSI_DEFAULT);
			}
		} else {
			if(!gs->drew) {
				if(verbose) printf("%sForced drawing%s\n", ANSI_BLUE, ANSI_DEFAULT);
				if(!drawCard(gs)) {
					if(verbose) printf("%sCould not draw, forced passing%s\n", ANSI_BLUE, ANSI_DEFAULT);
				}
			} else {
				if(verbose) printf("%sForced passing%s\n", ANSI_BLUE, ANSI_DEFAULT);
			}
		}
		plistDel(as.pl);
		gs->turn++;
		if(verbose) printf("\n");
	}
}

__attribute__((cold,noreturn)) static void sigintQueueClean(int sig)
{
	char pidstr[12];
	const pid_t pid = getpid();

	printf("\n");
	snprintf(pidstr, 12, "/p8s-%i", pid);
	mq_unlink(pidstr);
	exit(sig);
}

int main(int argc, char* argv[])
{
	size_t i, ngames = 1, nplayers = 0;
	uint8_t ai[MAXPLRS] = { 255 };
	uint8_t verbose = 0;
	int c, ti;

	assert(CPP * MAXPLRS < DECKLEN);

	opterr = 0;
	while((c = getopt(argc, argv, "m:p:g:v")) != -1) {
		switch(c) {
			case 'm':
				ti = 48;
				for(i = 0, nplayers = 0; i < strlen(optarg); i++) {
					if(isdigit(optarg[i])) {
						if(!(optarg[i] - ti)) {
							verbose++;
							ti++;
						}
						ai[nplayers++] = optarg[i] - 48;
					}
				}
				if(nplayers < MINPLRS || nplayers > MAXPLRS) {
					fprintf(stderr, "Player count (%zu) must be no less than %i and no greater than %i\n", nplayers, MINPLRS, MAXPLRS);
					return BADPNUM;
				}
				break;
			case 'g':
					ngames = atoi(optarg);
					if(ngames < 1) {
						fprintf(stderr, "Number of games (%zu) must be no less than 1\n", ngames);
						return BADGNUM;
					}
					break;
			case 'v':
					verbose++;
					break;
			case '?':
				if(optopt == 'p' || optopt == 'm' || optopt == 'n' || optopt == 'v')
					fprintf(stderr, "Option -%c requires an integer argument\n", optopt);
				else if(isprint(optopt))
					fprintf(stderr, "Unknown option '-%c'\n", optopt);
			default:
				fprintf(stderr, "getopt sadboosh %o\n", c);
				nplayers = 0;
		}
	}

	if(!nplayers || ai[0] == 255 || ai[nplayers-1] == 255) {
		fprintf(stderr, "Usage: %s -m nn[nn] [-g n] [-v]\nOptions:\n", 1+rindex(*argv, '/'));
		fprintf(stderr, "\t-m Modes (e.g. -m00, -m124)\n");
		fprintf(stderr, "\t-g Number of games to play\n");
		fprintf(stderr, "\t-v Verbose (specify multiple times for more verbosity, e.g. -vv)\n");
		fprintf(stderr, "\n\tAllowed modes are as follows:\n");
		for(i = 0; i < NAI; i++) fprintf(stderr, "\t\t%zu %s\n", i, aiDesc[i]);
		return BADARGS;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_sec + tv.tv_usec);
#ifdef NORANDOM
	srand(0);
#endif

	signal(SIGINT, sigintQueueClean);
#ifndef NDEBUG
	signal(SIGABRT, sigintQueueClean);
#endif

	struct gamestate gs;
	for(i = 0; i < ngames; i++) {
		initGameState(&gs, nplayers, ai);
		gameLoop(&gs, verbose);
		showGameState(&gs);
		cleanGameState(&gs);
	}

	return SUCCESS;
}
