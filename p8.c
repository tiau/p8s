#include "p8.h"

__attribute__((cold,noreturn)) static void sigintQueueClean(int sig)
{
	char pidstr[12];
	const pid_t pid = getpid();

	printf("\n");
	snprintf(pidstr, 12, "/p8s-%i", pid);
	mq_unlink(pidstr);
	exit(sig);
}

__attribute__((cold,nonnull)) static bool verifyHypoGame(const struct gamestate* const restrict gs, const size_t wp)
{
	size_t i, j, k, sz;

	assert(gs);

	/* Find repeated cards in pile / pile <-> players */
	for(i = 0; i < gs->pile.n; i++) {
		sz = gs->pile.n - i - 1;
		if(lfind(&gs->pile.c[i], gs->pile.c + i + 1, &sz, sizeof(card_t), cmpcardt)) {
			fprintf(stderr, "%s: Pile has repeat card: %i\n", __func__, gs->pile.c[i]);
			return false;
		}
		for(j = 0; j < wp; j++) {
			if(lfind(&gs->pile.c[i], gs->players[j].c, &gs->players[j].n, sizeof(card_t), cmpcardt)) {
				fprintf(stderr, "%s: Pile has repeat card (%i) with player %zu\n", __func__, gs->pile.c[i], j);
				return false;
			}
		}
	}
	/* Find repeated cards in player / player <-> player */
	for(i = 0; i < wp; i++) {
		for(j = 0; j < gs->players[i].n; j++) {
			sz = gs->players[i].n - j - 1;
			if(lfind(&gs->players[i].c[j], gs->players[i].c + j + 1, &sz, sizeof(card_t), cmpcardt)) {
				fprintf(stderr, "%s: Player %zu has repeat card: %i\n", __func__, wp, gs->players[i].c[j]);
				return false;
			}
			for(k = i + 1; k < wp; k++) {
				if(lfind(&gs->players[i].c[j], gs->players[k].c, &gs->players[k].n, sizeof(card_t), cmpcardt)) {
					fprintf(stderr, "%s: Player %zu has repeat card (%i) with player %zu\n", __func__, i, gs->players[i].c[j], k);
					return false;
				}
			}
		}
	}
	return true;
}

__attribute__((nonnull)) static void initGameStateHypoMain(struct gamestate* const restrict gs, struct gamestate* const restrict igs, const uint8_t* const restrict ai, const struct player* const restrict tplayer, const size_t nplayers, const size_t wp)
{
	size_t i, j;

	{	assert(gs);
		assert(igs);
		assert(ai);
		assert(tplayer);
		assert(nplayers);}

	gs->ai = ai;

	initDeckSans(&gs->deck, tplayer, &igs->pile);

	if(!igs->pile.n) {
		initPile(&igs->pile);
		*igs->pile.top = *gs->deck.top++;
		igs->pile.n++;
		gs->deck.n--;
	}

	initGameStateHypoShared(gs, igs);

	for(i = 0; i < nplayers; i++) {
		if(i < wp) {
			gs->players[i] = igs->players[i];
		} else {
			for(j = 0; j < CPP; j++)
				if(!playerDrawCard(gs, &gs->players[i]))
					assert(false);
		}
	}
}

int main(int argc, char* argv[])
{
	size_t i, ngames = 1, nplayers = 0;
	uint8_t ai[MAXPLRS] = { 255 };
	uint8_t verbose = 0;
	int c, ti;
	size_t wp = 0;
	struct gamestate igs;
	bool hypo = false, hv = false;
	uint_fast32_t (*aia[MAXPLRS])(const struct aistate* const restrict);

	assert(CPP * MAXPLRS < DECKLEN);

	igs.pile.n = 0;
	igs.players = calloc(MAXPLRS, sizeof(struct player));

	opterr = 0;
	while((c = getopt(argc, argv, "m:g:p:h:v")) != -1) {
		switch(c) {
			case 'm':
				ti = 48;
				for(i = 0, nplayers = 0; i < strlen(optarg); i++) {
					if(isdigit(optarg[i])) {
						if(!(optarg[i] - ti)) {
							verbose = (!verbose) ? 1 : verbose;
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
			case 'p':
					hypo = true;
					for(i = 0; i <= strlen(optarg)/3; i++) {
						igs.pile.c[i] = readCard(optarg + i*3);
					}
					igs.pile.n = i;
					igs.pile.top = igs.pile.c + i - 1;
					break;
			case 'h':
					hypo = true;
					if(wp >= MAXPLRS) {
						fprintf(stderr, "Only %i hands or fewer may be specified\n", MAXPLRS);
						return BADPNUM;
					}
					for(i = 0; i <= strlen(optarg)/3; i++) {
						igs.players[wp].c[i] = readCard(optarg + i*3);
					}
					igs.players[wp++].n = i;
					break;
			case 'v':
					if(!verbose || hv)
						verbose++;
					hv = true;
					break;
			case '?':
				if(optopt == 'm' || optopt == 'g' || optopt == 'v')
					fprintf(stderr, "Option -%c requires an integer argument\n", optopt);
				else if(isprint(optopt))
					fprintf(stderr, "Unknown option '-%c'\n", optopt);
			default:
				nplayers = 0;
		}
	}

	if(!nplayers || ai[0] == 255 || nplayers < wp || !verifyHypoGame(&igs, wp)) {
		fprintf(stderr, "Usage: %s -m nn[nn] [-g n] [-v] [-p cards] [-h cards] [-d cards]\nOptions:\n", 1+rindex(*argv, '/'));
		fprintf(stderr, "\t-m Modes (e.g. -m00, -m124)\n");
		fprintf(stderr, "\t-g Number of games to play\n");
		fprintf(stderr, "\t-v Verbose (specify multiple times for more verbosity, e.g. -vv)\n");
		fprintf(stderr, "\t-p Specific pile to use, (e.g. AH or \"AH 7H 7D 0D\")\n");
		fprintf(stderr, "\t-h Specific hand to use, give multiple hands with more -h's\n");
		fprintf(stderr, "\n\tAllowed modes are as follows:\n");
		for(i = 0; i < NAI; i++) fprintf(stderr, "\t\t%zu %s\n", i, aiDesc[i]);
		return BADARGS;
	}

	size_t victories[nplayers];
	memset(victories, 0, nplayers * sizeof(size_t));

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

	for(i = 0; i < nplayers; i++)
		aia[i] = ais[ai[i]];

	struct player tplayer;
	if(hypo) {
		size_t j;

		igs.nplayers = nplayers;
		igs.eightSuit = Unknown;

		tplayer.n = 0;
		for(i = 0; i < wp; i++)
			for(j = 0; j < igs.players[i].n; j++)
				tplayer.c[tplayer.n++] = igs.players[i].c[j];
	}

	struct gamestate gs;
	for(i = 0; i < ngames; i++) {
		if(hypo)
			initGameStateHypoMain(&gs, &igs, ai, &tplayer, nplayers, wp);
		else
			initGameState(&gs, nplayers, ai);

		gameLoop(&gs, verbose, false, false, aia);
		showGameState(&gs);
		victories[(gs.turn - (!getGameState(&gs) ? 1 : 0)) % gs.nplayers]++;
		cleanGameState(&gs);
	}

	if(ngames > 1) {
		printf("\n");
		for(i = 0; i < nplayers; i++)
			printf("Player %zu won %zu games (%.1f%%)\n", i, victories[i], 100.0*victories[i]/ngames);
	}

	free(igs.players);
	return SUCCESS;
}
