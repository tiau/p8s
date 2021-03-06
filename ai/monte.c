# include "monte.h"

void dealStateSans(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	size_t i, j;

	{	assert(gs);
		assert(gs->nplayers == ogs->nplayers);}

	gs->players[0] = *stateToPlayer(ogs);

	size_t cih[ogs->nplayers];
	populateCIH(ogs, cih);
	for(i = 1; i < ogs->nplayers; i++)
		for(j = 0; j < cih[i-1]; j++)
			if(!playerDrawCard(gs, &gs->players[i]))
				assert(false);

	{	assert(gs->players[0].n);
		assert(gs->players[0].n + 1 < DECKLEN);
		assert(gs->players[1].n);
		assert(gs->players[1].n + 1 < DECKLEN);}
}

void initGameStateHypoShared(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	{	assert(gs);
		assert(ogs->pile.n);
		assert(ogs->nplayers >= MINPLRS && ogs->nplayers <= MAXPLRS);}

	gs->pile = ogs->pile;
	gs->pile.top = gs->pile.c + ogs->pile.n - 1;
	gs->nplayers = ogs->nplayers;
	memset(gs->players, 0, sizeof(struct player) * gs->nplayers);
	gs->turn = 0;
	gs->eightSuit = ogs->eightSuit;
	gs->drew = ogs->drew;
	gs->magic = false;
	memcpy(gs->draws, ogs->draws, ogs->nplayers * sizeof(draw_t));
}

__attribute__((hot,nonnull)) static void initGameStateHypothetical(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	assert(ogs);

	initGameStateHypoShared(gs, ogs);
	initDeckSans(&gs->deck, stateToPlayer(ogs), &ogs->pile);
	dealStateSans(gs, ogs);
}

__attribute__((hot,nonnull(5,6))) static size_t playHypoGames(const size_t ngames, const struct play* const restrict gtp, const suit_t forces, const struct aistate* const restrict as, uint_fast32_t (*aif)(const struct aistate* const restrict), void (*initgs)(struct gamestate* const restrict, const struct gamestate* const restrict))
{
	size_t i, ret = 0;
	bool e; // Whether we're testing a suit
	struct gamestate gs;
	bool magic = likely(forces == Unknown && gtp);

	{	assert(ngames);
		assert(forces <= Unknown);
		assert(as);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);}

	for(i = 0; i < as->gs->nplayers; i++)
		gs.ai[i] = aif;

	for(i = 0; i < ngames; i++) {
		(*initgs)(&gs, as->gs);

		if(likely((bool)gtp)) {	// If we have a play to make, make it
			makeMove(&gs, gtp);
			gs.turn++;
			gs.magic = (magic ? isMagicCard(*gs.pile.top) : false);
		} else {
			if(!as->gs->drew)	// If we have no play but haven't drawn, draw
				drawCard(&gs);
			else				// Otherwise we have no play and have drawn, pass
				gs.turn++;
		}

		gs.eightSuit = forces;
		if(likely(forces == Unknown)) {	// If we aren't testing a suit
			if(likely(gtp != NULL))		// ...we either have a play
				e = unlikely(getVal(gtp->c[gtp->n-1]) == 8);
			else						// ...or draw/pass
				e = false;
		} else {						// Otherwise we're testing a suit
			e = true;
		}

		if(unlikely(!gs.players[0].n)) {
			ret = MaxGames;
			cleanGameState(&gs);
			break;
		}
		// TODO use silly magic about win distance to select better plays?
		ret += (gameLoop(&gs, false, e, 0) > 0.0);

		cleanGameState(&gs);
	}

	return ret;
}

__attribute__((hot,pure,nonnull)) static size_t eightMoveCount(const struct plist* const restrict pl)
{
	size_t i, nump;
	const struct play* restrict play;

	assert(pl);

	for(i = 0, nump = pl->n + 1; i < pl->n; i++) {
		play = plistGet(pl, i);
		if(unlikely(getVal(play->c[play->n-1]) == 8))
			nump += 3;
	}
	return nump;
}

static void initMsgQueue(mqd_t* const restrict mq, char* const restrict mqname, const size_t ncpu)
{
	struct mq_attr mqa;
	const pid_t pid = getpid();	// PID for MQ name so multiple simultaneous p8s don't clash

	{	assert(mq);
		assert(mqname);
		assert(ncpu);}

	mqa.mq_flags = 0;
	mqa.mq_maxmsg = ncpu;
	mqa.mq_msgsize = BUFSZ;
	snprintf(mqname, 12, "/p8s-%i", pid);
	if((*mq = mq_open(mqname, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, &mqa)) == -1)
		fprintf(stderr, "%s: Cannot open message queue %s\n", __func__, mqname);
}

static void initMoveMap(size_t* const restrict emap, const struct plist* const pl)
{
	size_t i, j, m;
	const struct play* play;

	{	assert(emap);
		assert(pl);}

	for(i = 0, m = 0; i < pl->n; i++) {
		play = plistGet(pl, i);
		if(unlikely(getVal(play->c[play->n-1]) == 8))
			for(j = Clubs; j <= Spades; j++)
				emap[m++] = i;
		else
			emap[m++] = i;
	}
	emap[m] = pl->n;
}

static void launchThreads(pthread_t* const restrict threads, const size_t ncpu, void* (*gtf)(void*), struct pctmstate* const restrict s)
{
	size_t i;

	{	assert(threads);
		assert(ncpu);
		assert(gtf);
		assert(s);}

	for(i = 0; i < ncpu; i++)
		if(pthread_create(&threads[i], NULL, gtf, (void*)s) != 0)
			fprintf(stderr, "%s: Cannot create game thread %zu of %zu\n", __func__, i, ncpu);
}

static void sendPlay(const struct pctmstate* const restrict s, const size_t play, const size_t rplay, const suit_t suit, const size_t hmg, const bool* const restrict dead)
{
	static char buf[BUFSZ];

	assert(s);
	assert(dead);

	if(dead[rplay])
		return;

	snprintf(buf, BUFSZ, "p%4zu %4zu %1u %4zu", play, rplay, suit, hmg);
	mq_send(*s->mq, buf, 18, 1);
}

__attribute__((nonnull,hot)) static void controlThread(const struct pctmstate* const restrict s, const size_t ncpu, const char* const restrict name)
{
	size_t i, j, k, m;
	const struct play* restrict play;
	const size_t nump = s->nplays;
	size_t hmg = MinGames;
	bool dead[nump];
	float scores[nump];
	float best, ath;
	size_t tt, btn;
	size_t act = nump;
	size_t times = 1;
	float phiv;

	memset(dead, 0, sizeof(bool)*nump);

	while(act) {
		if(act > 1) {
			/* Send enough games to saturate our threads before checking */
			times = (act < ncpu) ? ceilf((float)ncpu / (float)act) : 1;
			for(k = 0; k < times; k++) {
				for(i = 0, m = 0; i < s->as->pl->n; i++) {
					play = plistGet(s->as->pl, i);
					if(unlikely(getVal(play->c[play->n-1]) == 8)) {
						for(j = Clubs; j <= Spades; j++)
							sendPlay(s, i, m++, j, hmg, dead);
					} else {
						sendPlay(s, i, m++, Unknown, hmg, dead);
					}
				}
				sendPlay(s, i, m, Unknown, hmg, dead);
			}
			hmg += hmg / 8;
		}

		size_t bwins = 0;
		pthread_rwlock_rdlock(s->rwl);
		best = -1.0;
		btn = 0;
		for(j = 0; j < nump; j++) {
			if(dead[j])
				continue;
			tt = s->trials[j];
			if((scores[j] = (float)s->wins[j] / (float)tt) > best) {
				bwins = s->wins[j];
				best = scores[j];
				btn = tt;
			}
		}

		for(j = 0; j < nump; j++) {
			if(dead[j])
				continue;
			tt = s->trials[j];
			ath = min(tt, btn);
			if(ath < MinGames)
				continue;
			phiv = phi(z(s->wins[j], bwins, tt, btn));
			if(phiv < pCutOff || unlikely(tt >= MaxGames || act == 1)) {
				dead[j] = true;
				act--;
#ifdef MONTE_VERBOSE
				printf("%s%s:%s  %zu\t(%.1f%%\t%.1f%%)\t%.1f%%\t", ANSI_CYAN, name, ANSI_DEFAULT, j, 100*phiv, 100.0*((float)tt) / (float)MaxGames, 100.0*scores[j]);
				if(j+1 == nump)
					printf("(%s)  ", ((s->as->gs->drew) ? "pass" : "draw"));
				else
					showPlay(plistGet(s->as->pl, s->emap[j]));
				printf("%s %s\n", ANSI_BACK, ANSI_DEFAULT);
#endif
			}
		}
		pthread_rwlock_unlock(s->rwl);
	}
}

__attribute__((nonnull,hot)) static void* monteThread(void* arg)
{
	char tbuf[BUFSZ];
	const struct play* gtp; // The play to make (NULL to draw/pass)
	size_t bucket;			// Which wins/trials bucket to use
	size_t wp;				// Which play to make
	size_t ngames;			// How many games to play
	suit_t forces;			// Suit to force for 8-ending plays
	ssize_t br;				// Bytes read via message queue
	struct pctmstate* s = (struct pctmstate*)arg;
	size_t tt, ti;

	{	assert(s);
		assert(s->as);
		assert(s->as->gs);
		assert(s->as->pl);
		assert(s->mq);
		assert(s->wins);
		assert(s->trials);
		assert(s->emap);
		assert(s->as->gs->nplayers >= MINPLRS && s->as->gs->nplayers <= MAXPLRS);
		assert(s->aif);
		assert(s->initgs);}

	for(bool done = false; !done;) {
		if(unlikely((br = mq_receive(*s->mq, tbuf, BUFSZ, NULL)) == -1)) {
			fprintf(stderr, "%s: Could not receive message\n", __func__);
			usleep(50000);
			continue;
		}

		switch(tbuf[0]) {
			case 'p':
			/* Message format specification for p
			 * paaaa bbbb c dddd0
			 * where
			 * aaaa 4 bytes, which play to make
			 * bbbb 4 bytes, which bucket to use
			 * c	1 bytes, the suit for 8-ending plays
			 * dddd 4 bytes, how many games to play
			 * 0	1 null byte */
				wp = atoi(tbuf+1);
				bucket = atoi(tbuf+6);
				forces = (suit_t)tbuf[11] - 48;
				ngames = atoi(tbuf+13);
				ti = ngames;
				gtp = unlikely(wp == s->as->pl->n) ? NULL : plistGet(s->as->pl, wp);
				tt = playHypoGames(ngames, gtp, forces, s->as, s->aif, s->initgs);
				ti = max(ti, tt);
				pthread_rwlock_wrlock(s->rwl);
				s->wins[bucket] += tt;
				s->trials[bucket] += ti;
				pthread_rwlock_unlock(s->rwl);
				break;
			/* Quit message is always 'q' then a null byte */
			case 'q':
				done = true;
				break;
			default:
				assert(false);
				break;
		}
	}
	return NULL;
}

static void threadsDone(const size_t nthr, pthread_t* const restrict threads, mqd_t* const restrict mq, const char* const restrict mqname)
{
	size_t i;

	{	assert(threads);
		assert(mq);
		assert(mqname);}

	for(i = 0; i < nthr; i++)
		if(unlikely(mq_send(*mq, "q\0", 2, 2) == -1))
			fprintf(stderr, "%s: Failed to send quit message %zu, retrying\n", __func__, i--);

	for(i = 0; i < nthr; i++)
		if(unlikely(pthread_join(threads[i], NULL)))
			fprintf(stderr, "%s Could not join thread %zu\n", __func__, i);

	if(unlikely(mq_close(*mq)))
		fprintf(stderr, "%s: Failed to close message queue\n", __func__);

	if(unlikely(mq_unlink(mqname)))
		fprintf(stderr, "%s: Failed to unlink message queue\n", __func__);
}

__attribute__((pure)) static uint_fast32_t findBest(const struct pctmstate* const restrict s)
{
	size_t i, m;
	uint_fast32_t ret = 0;
	float ft, bs = 0.0; 

	assert(s);

	MPACK(ret, s->as->pl->n);
	for(i = 0; i < s->nplays; i++) {
		m = s->emap[i];
		ft = (float)s->wins[i] / (float)s->trials[i];
		if(ft > bs && s->trials[i]) {
			bs = ft;
			MPACK(ret, m);
			ESPACK(ret, Clubs);
			if(unlikely(i && m == s->emap[i-1])) {
				ESPACK(ret, Diamonds);
				if(i > 1 && s->emap[i-1] == s->emap[i-2]) {
					ESPACK(ret, Hearts);
					if(i > 2 && s->emap[i-2] == s->emap[i-3]) {
						ESPACK(ret, Spades);
					}
				}
			}
		}
	}
	return ret;
}

uint_fast32_t pctmRun(const struct aistate* const restrict as, void (*initgs)(struct gamestate* const restrict, const struct gamestate* const restrict), uint_fast32_t (*aif)(const struct aistate* const restrict), const char* const restrict name)
{
	{	assert(as);
		assert(as->gs);
		assert(as->pl);
		assert(as->pl->n);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);
		assert(name);}

	/* How many moves we have counting 8-ending plays as 4-in-1 and draw/pass
	 * as a move.  emap / initMoveMap() allow us to map from one number to the
	 * play number and eight suit that we are expected to return. */
	const size_t nump = eightMoveCount(as->pl);
	const size_t ncpu = sysconf(_SC_NPROCESSORS_ONLN) / 2; // Assume 2 threads per core
	size_t emap[nump];
	size_t wins[nump];
	size_t trials[nump];
	pthread_rwlock_t rwl;
	char mqname[12];
	pthread_t threads[ncpu];
	mqd_t mq;

	memset(wins, 0, sizeof(size_t)*nump);
	memset(trials, 0, sizeof(size_t)*nump);
	pthread_rwlock_init(&rwl, NULL);
	initMsgQueue(&mq, mqname, ncpu);
	initMoveMap(emap, as->pl);
	struct pctmstate pcs = { as, nump, trials, wins, emap, &mq, &rwl, aif, initgs };
#ifdef BENCHMARK
	struct timeval tval_before, tval_after, tval_result;
	gettimeofday(&tval_before, NULL);
#endif
	launchThreads(threads, ncpu, monteThread, &pcs);
	controlThread(&pcs, ncpu, name);
	threadsDone(ncpu, threads, &mq, mqname);
#ifdef BENCHMARK
	gettimeofday(&tval_after, NULL);
	timersub(&tval_after, &tval_before, &tval_result);
	size_t s = 0;
	for(size_t i = 0; i < nump; i++)
		s += trials[i];
	printf("%zu games in %ld.%06ld seconds, %f games/sec\n", s, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec, (float)s/((float)tval_result.tv_sec+(float)tval_result.tv_usec/1000000));
#endif
	pthread_rwlock_destroy(&rwl);
	return findBest(&pcs);
}

uint_fast32_t aiMonte(const struct aistate* const restrict as)
{
	return pctmRun(as, initGameStateHypothetical, MonteSub, __func__);
}
