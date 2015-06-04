#include "mimic.h"

// TODO: we could do a lot more than just stack the deck via magic cards and eights, people are also likely to be holding pairs, straights, flushes, etc.
void stackTheDeck(struct deck* const restrict deck, const size_t tch)
{
	size_t i, j, o, ds;
	size_t open[tch];
	card_t t;

	{	assert(deck);
		assert(tch);}

	if(tch < deck->n) {
		/* Find open slots for magic cards to go in the first tch cards */
		ds = DECKLEN - deck->n;
		for(i = ds, o = 0; i < (tch + ds) && i < (deck->n + ds); i++) {
			t = getVal(deck->c[i]);
			if(t != 2 && t != 3 && t != 8)
				open[o++] = i;
		}

		/* 10% chance to move magic cards to opponents' hands, 20% for eights */
		for(i = tch + ds, j = 0; i < deck->n && j < o; i++) {
			t = getVal(deck->c[i]);
			if(((t == 2 || t == 3) && rand() % 10 == 0) || (t == 8 && rand() % 5 == 0))
				cardSwap(&deck->c[i], &deck->c[open[j++]]);
		}
	}
}

void initStackedGameStateHypothetical(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	size_t i, tch = 0;

	{	assert(gs);
		assert(gs->nplayers >= MINPLRS && gs->nplayers <= MAXPLRS);
		assert(gs->players);
		assert(ogs);
		assert(ogs->nplayers);
		assert(ogs->players);}

	/* Calculate how many cards our opponents hold, for deck stacking */
	size_t cih[ogs->nplayers];
	populateCIH(ogs, cih);
	for(i = 0; i < ogs->nplayers-1; i++)
		tch += cih[i];

	initDeckSans(&gs->deck, stateToPlayer(ogs), &ogs->pile);
	gs->pile = ogs->pile;
	gs->pile.top = gs->pile.c + ogs->pile.n - 1;
	gs->players = calloc(ogs->nplayers, sizeof(struct player));
	assert(gs->players);
	gs->nplayers = ogs->nplayers;
	stackTheDeck(&gs->deck, tch);
	dealStateSans(gs, ogs);
	/* Cannot run a hypothetical game from before anything has happened */
	assert(gs->pile.n);
	gs->turn = 0;
	gs->eightSuit = ogs->eightSuit;
	gs->ai = NULL;
}

inline static float maxGrab(const float* const restrict arr, const size_t s)
{
	size_t i;
	float ret = -INFINITY;

	assert(arr);

	for(i = 0; i < s; i++)
		if(arr[i] > ret)
			ret = arr[i];

	return ret;
}

void* mimicThread(void* arg)
{
	size_t i, j = 0, lact;
	float maxw, wins = 0.0;
	const struct btmstate* const ar = arg;

	bool lost;
	float co;

	assert(ar);

	struct btmstate bs = { ar->as,
					       ar->trials,
					       ar->wins,
					       ar->emap,
					       ar->rbar,
					       ar->racebarr,
					       ar->actbarr,
					       ar->actbarr2,
					       ar->barrsize,
					       ar->race,
					       ar->idx,
					       ar->tott,
					       ar->active,
					       ar->maxTrials,
					       ar->play,
					       ar->aif,
					       ar->forces };
	{	assert(bs.tott);
		assert(bs.active);
		assert(bs.as->gs);
		assert(bs.as->gs->nplayers >= MINPLRS && bs.as->gs->nplayers <= MAXPLRS);
		assert(bs.racebarr);
		assert(bs.actbarr2);
		assert(bs.actbarr);
		assert(bs.maxTrials);
		assert(bs.rbar);
		assert(bs.race);
		assert(bs.barrsize);
		assert(bs.as);
		assert(bs.as->gs->players);}


	pthread_barrier_wait(bs.rbar);

	for(i = bs.barrsize; ; i+=bs.barrsize) {
		wins += playHypoGames(bs.barrsize, bs.play, bs.forces, bs.as, MIMICAIF, initStackedGameStateHypothetical);
		/* This (and all idx operations) are safe since we're the only thread that
		 * will use that particular index */
		bs.race[bs.idx] = wins;
		bs.barrsize += bs.barrsize / 16;
		pthread_barrier_wait(bs.racebarr);

		maxw = maxGrab(bs.race, bs.tott);
		co = 6.1*log(i+1)/pow(i+1, 1.1);
		lost = ((float)maxw/(i+1) - (float)wins/(i+1) > co);

		if(lost) {
			bs.race[bs.idx] = -INFINITY;
			__atomic_fetch_sub(bs.active, 1, __ATOMIC_RELAXED);
		} else if(unlikely(wins >= maxw)) {
			/* Since we could have a tied-for-best outcome, walk and find the
			 * first occuring tied-for-winning thread */
				for(j = 0; j < bs.tott; j++)
					if(bs.race[j] >= maxw)
						break;
		}

		pthread_barrier_wait(bs.actbarr);

		/* If we're the leader we need to hanDECKLENe the barriers */
		if(unlikely(wins >= maxw && j == bs.idx)) {
			/* Number of threads continuing to the next round */
			lact = *bs.active;
			assert(lact < 999);
			pthread_barrier_destroy(bs.racebarr);
			pthread_barrier_destroy(bs.actbarr);
			pthread_barrier_init(bs.racebarr, NULL, lact);
			pthread_barrier_init(bs.actbarr, NULL, lact);

			pthread_barrier_wait(bs.actbarr2);

			if(lact <= 1 || i >= bs.maxTrials)
				break;

			pthread_barrier_destroy(bs.actbarr2);
			pthread_barrier_init(bs.actbarr2, NULL, lact);

		/* Otherwise just wait for the leader */
		} else {
			pthread_barrier_wait(bs.actbarr2);
		}

		if(lost || i >= bs.maxTrials)
			break;
	}

	assert(sizeof(float) == 4);
	assert(sizeof(void*) >= 6); // This makes us compatible on 64-bit only
	unsigned char* g = (unsigned char*)&wins;
	return (void*)((i    << 32)						 | // LAZY!
				   (g[0] << 24 & 0x00000000FF000000) |
				   (g[1] << 16 & 0x0000000000FF0000) |
				   (g[2] << 8  & 0x000000000000FF00) |
				   (g[3]       & 0x00000000000000FF));
}

void hypoDpkg(const uint_fast64_t in, size_t* restrict per, float* restrict n)
{
	unsigned char rr[4];
	float* infp = (float*)&rr;
	
	*per = in>>32;
	rr[0] = (in & 0x00000000FF000000) >> 24;
	rr[1] = (in & 0x0000000000FF0000) >> 16;
	rr[2] = (in & 0x000000000000FF00) >> 8;
	rr[3] = (in & 0x00000000000000FF);
	*n = *infp;
}

static void mimic_hypoManage(struct btmstate* const restrict bs, const size_t idx, const suit_t forces, pthread_t* const restrict thread)
{
	assert(bs);
	assert(thread);

	bs->idx = idx;
	bs->forces = forces;

	if(pthread_create(thread, NULL, mimicThread, (void*)bs) != 0)
		fprintf(stderr, "%s: Cannot create thread %zu/%zu\n", __func__, idx, bs->tott);
	pthread_barrier_wait(bs->rbar);
}

uint_fast32_t aiMimic(const struct aistate* const restrict as)
{
	uint_fast32_t ret = 0;
	uint_fast64_t wins;
	float winsf;
	size_t i, numt, ra, active, j, wp = 0, per;
	float v, best = -INFINITY;
	pthread_barrier_t racebarr, actbarr, actbarr2;
	uint_fast32_t (*aif)(const struct aistate* const restrict as) = aiRandom;
	static const size_t cs = 40;	// Games between checking for fin
    static const size_t mg = 9999;	/* Upper bound on how many games might be played for a given move */

	pthread_barrier_t rbar;

	{	assert(as);
		assert(as->pl);
		assert(as->pl->n);
		assert(as->gs);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MINPLRS);
		assert(as->gs->players);}

	numt = eightMoveCount(as->pl);

	pthread_t threads[numt];
	float race[numt];
	size_t map[numt];

	pthread_barrier_init(&racebarr, NULL, numt);
	pthread_barrier_init(&actbarr, NULL, numt);
	pthread_barrier_init(&actbarr2, NULL, numt);
	pthread_barrier_init(&rbar, NULL, 2); 

	struct btmstate bs = { .as = as,
						   .tott = numt,
						   .active = &active,
						   .racebarr = &racebarr,
						   .actbarr2 = &actbarr2,
						   .actbarr = &actbarr,
						   .rbar = &rbar,
						   .maxTrials = mg,
						   .race = race,
						   .aif = aif,
						   .barrsize = cs };

	active = numt;
	ra = active;
	ESPACK(ret, UNKNOWN);
	for(i = 0; i < as->pl->n; i++) {
        bs.play = plistGet(as->pl, i);

		if(getVal(bs.play->c[bs.play->n-1]) == 8) {
			for(j = CLUBS; j <= SPADES; j++) {
				mimic_hypoManage(&bs, wp, j, &threads[wp]);
				map[wp++] = i;
			}
		} else {
			mimic_hypoManage(&bs, wp, UNKNOWN, &threads[wp]);
			map[wp++] = i;
		}
	}
	map[wp] = as->pl->n;
	bs.play = NULL;
	mimic_hypoManage(&bs, wp, UNKNOWN, &threads[wp]);

	pthread_barrier_destroy(&rbar);

	while(ra) {
		for(i = 0; i < numt; i++) {
			if(!threads[i])
				continue;
			if(pthread_tryjoin_np(threads[i], (void**)&wins) != 0) {
				usleep(5000);
				continue;
			}
			wp = map[i];
			ra--;
			threads[i] = 0;
			hypoDpkg(wins, &per, &winsf);
			v = 100.0*winsf/(float)per;
	
#ifdef MONTE_VERBOSE
			printf("%smimic:%s p %zu\t(%.1f%% :: %.1f)\t", ANSI_CYAN, ANSI_DEFAULT, i, 100.0*((float)per) / (float)mg, 100.0*(6.1*log(per)/pow((double)(per), 1.1)));
#endif
			if(v >= best) {
#ifdef MONTE_VERBOSE
				printf("%s", ANSI_WHITE);
#endif
				best = v;
				MPACK(ret, wp);
				ESPACK(ret, CLUBS);
				if(i && wp == map[i-1]) {
					ESPACK(ret, DIAMONDS);
					if(i > 1 && map[i-1] == map[i-2]) {
						ESPACK(ret, HEARTS);
						if(i > 2 && map[i-2] == map[i-3]) {
							ESPACK(ret, SPADES);
						}
					}
				}
			}

#ifdef MONTE_VERBOSE
			printf("%.1f%%%s\t", v, ANSI_DEFAULT);
			if(i + 1 == numt)
				printf("(%s)  ", ((as->gs->drew) ? "pass" : "draw"));
			else
				showPlay(plistGet(as->pl, wp));
			printf("%s %s\n", ANSI_BACK, ANSI_DEFAULT);
#endif
		}
	}

	pthread_barrier_destroy(&racebarr);
	pthread_barrier_destroy(&actbarr);
	pthread_barrier_destroy(&actbarr2);

	return ret;
}
