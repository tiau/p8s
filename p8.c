#include "p8.h"

__attribute__((nonnull)) static void swapSt(size_t* a, size_t* b)
{
	size_t t = *a;
	*a = *b;
	*b = t;
}

__attribute__((nonnull)) static void swapAi(uint_fast32_t (**a)(const struct aistate* const restrict), uint_fast32_t (**b)(const struct aistate* const restrict))
{
	uint_fast32_t (*t)(const struct aistate* const restrict) = *a;
	*a = *b;
	*b = t;
}

__attribute__((nonnull)) static void reverseSt(size_t* a, size_t* b)
{
	while(--b > a) swapSt(a++, b);
}

__attribute__((nonnull)) static void reverseAi(uint_fast32_t (**a)(const struct aistate* const restrict), uint_fast32_t (**b)(const struct aistate* const restrict))
{
	while(--b > a) swapAi(a++, b);
}

__attribute__((nonnull)) static void rotateSt(size_t* const restrict ar, const size_t s, const size_t by)
{
	reverseSt(ar, ar + s);
	reverseSt(ar, ar + by);
	reverseSt(ar + by, ar + s);
}

__attribute__((nonnull)) static void rotateAi(uint_fast32_t (**ar)(const struct aistate* const restrict), const size_t s, const size_t by)
{
	reverseAi(ar, ar + s);
	reverseAi(ar, ar + by);
	reverseAi(ar + by, ar + s);
}

__attribute__((cold)) static void showStats(void)
{
	size_t i, j;

	rotateSt(successes, nplayers, nplayers - offset);
	printf("\n");
	for(i = 0; i < nplayers; i++)
		printf("Player %zu won %zu games (%.1f%%)\n", i, successes[i], 100.0*successes[i]/ngames);
	if(ngames > 9) {
		printf("\n");
		for(i = 0; i < nplayers; i++)
			for(j = i + 1;  j < nplayers; j++)
				printf("Player %zu is better than player %zu with %.2f%% certainty\n", i, j, 100*phi(z(successes[i], successes[j], ngames, ngames)));
	}
}

__attribute__((cold,noreturn)) static void sigintQueueClean(int sig)
{
	char pidstr[12];
	const pid_t pid = getpid();

	printf("\n");
	snprintf(pidstr, 12, "/p8s-%i", pid);
	mq_unlink(pidstr);
	if(successes) {
		showStats();
		free(successes);
	}
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

__attribute__((hot,nonnull)) static void initGameStateHypoMain(struct gamestate* const restrict gs, struct gamestate* const restrict igs, uint_fast32_t (* const ai[MAXPLRS])(const struct aistate* const restrict), const struct player* const restrict tplayer, const size_t wp)
{
	size_t i, j;

	{	assert(gs);
		assert(igs);
		assert(ai);
		assert(tplayer);
		assert(nplayers);}

	memcpy(gs->ai, ai, nplayers * sizeof(uint_fast32_t (* const)(const struct aistate* const restrict)));

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

__attribute__((hot,nonnull)) static void runGames(const bool hypo, struct gamestate* const restrict igs, const size_t wp, const bool rot, const uint8_t verbose, uint_fast32_t (*ai[MAXPLRS])(const struct aistate* const restrict))
{
	size_t i, j;
	struct player tplayer;
	struct gamestate gs;

	if(hypo) {
		igs->nplayers = nplayers;
		igs->eightSuit = Unknown;

		tplayer.n = 0;
		for(i = 0; i < wp; i++)
			for(j = 0; j < igs->players[i].n; j++)
				tplayer.c[tplayer.n++] = igs->players[i].c[j];
	}

	i = ngames;
	ngames = 0;
	for(; ngames < i; ngames++) {
		if(hypo)
			initGameStateHypoMain(&gs, igs, ai, &tplayer, wp);
		else
			initGameState(&gs, nplayers, ai);

		gameLoop(&gs, verbose, false, offset);
		showGameState(&gs, offset);
		successes[(gs.turn - (!getGameState(&gs) ? 1 : 0)) % gs.nplayers]++;
		cleanGameState(&gs);
		if(rot) {
			rotateAi(ai, nplayers, 1);
			rotateSt(successes, nplayers, 1);
			offset = (offset + 1) % nplayers;
		}
	}

	if(ngames > 1)
		showStats();
}

int main(int argc, char* argv[])
{
	size_t i, wp = 0;
	uint8_t verbose = 0;
	int c, ti;
	struct gamestate igs;
	bool hypo = false, hv = false;
	bool rot = false;
	uint_fast32_t (*ai[MAXPLRS])(const struct aistate* const restrict) = { NULL };

	assert(CPP * MAXPLRS < DECKLEN);

	igs.pile.n = 0;
	igs.drew = false;
	memset(igs.players, 0, MAXPLRS * sizeof(struct player));
	memset(igs.draws, 0, MAXPLRS * sizeof(uint_fast64_t));

	opterr = 0;
	while((c = getopt(argc, argv, "m:g:p:h:vr")) != -1) {
		switch(c) {
			case 'm':
				ti = '0';
				for(i = 0, nplayers = 0; i < strlen(optarg); i++) {
					if(isdigit(optarg[i])) {
						if(!(optarg[i] - ti)) {
							verbose = (!verbose) ? 1 : verbose;
							ti++;
						}
						ai[nplayers++] = ais[optarg[i] - '0'];
					}
				}
				if(nplayers < MINPLRS || nplayers > MAXPLRS) {
					fprintf(stderr, "Player count (%zu) must be no less than %i and no greater than %i\n", nplayers, MINPLRS, MAXPLRS);
					return BADARGS;
				}
				break;
			case 'g':
					ngames = atoi(optarg);
					if(ngames < 1) {
						fprintf(stderr, "Number of games (%zu) must be no less than 1\n", ngames);
						return BADARGS;
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
						return BADARGS;
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
			case 'r':
					rot = true;
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

	if(!nplayers || ai[0] == NULL || nplayers < wp || !verifyHypoGame(&igs, wp)) {
		fprintf(stderr, "Usage: %s -m nn[nn] [-g n] [-v] [-p cards] [-h cards]\nOptions:\n", 1+rindex(*argv, '/'));
		fprintf(stderr, "\t-m Modes (e.g. -m00, -m124)\n");
		fprintf(stderr, "\t-g Number of games to play\n");
		fprintf(stderr, "\t-r Rotate player positions each game (hides signal from positional advantage)\n");
		fprintf(stderr, "\t-v Verbose (specify multiple times for more verbosity, e.g. -vv)\n");
		fprintf(stderr, "\t-p Specific pile to use, (e.g. AH or \"AH 7H 7D 0D\")\n");
		fprintf(stderr, "\t-h Specific hand to use, give multiple hands with more -h's\n");
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
	signal(SIGUSR1, (void(*)(int))showStats);
#ifndef NDEBUG
	signal(SIGABRT, sigintQueueClean);
#endif

	successes = calloc(nplayers, sizeof(size_t));
	runGames(hypo, &igs, wp, rot, verbose, ai);
	free(successes);

	return SUCCESS;
}
