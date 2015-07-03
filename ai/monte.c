# include "monte.h"

void dealStateSans(struct gamestate* const restrict gs, const struct gamestate* const restrict ogs)
{
	size_t i, j;

	{	assert(gs);
		assert(gs->players);
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
		assert(ogs->players);
		assert(ogs->nplayers >= MINPLRS && ogs->nplayers <= MAXPLRS);}

	gs->pile = ogs->pile;
	gs->pile.top = gs->pile.c + ogs->pile.n - 1;
	gs->players = calloc(ogs->nplayers, sizeof(struct player));
	assert(gs->players);
	gs->nplayers = ogs->nplayers;
	gs->turn = 0;
	gs->eightSuit = ogs->eightSuit;
	gs->drew = ogs->drew;
	gs->magic = false;
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
	float grv;		// Win/lose distance
	bool e;			// Whether we're testing a suit
	struct gamestate gs;
	bool magic = likely(forces == UNKNOWN && gtp);
	uint_fast32_t (*aia[MAXPLRS])(const struct aistate* const restrict);

	{	assert(ngames);
		assert(forces <= UNKNOWN);
		assert(as);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);}

	for(i = 0; i < as->gs->nplayers; i++)
		aia[i] = aif;

	for(i = 0; i < ngames; i++) {
		(*initgs)(&gs, as->gs);

		if(likely((bool)gtp)) {	// If we have a play to make, make it
			makeMove(&gs, gtp);
			gs.turn++;
			magic = (magic ? isMagicCard(*gs.pile.top) : false);
		} else {
			if(!as->gs->drew)	// If we have no play but haven't drawn, draw
				drawCard(&gs);
			else				// Otherwise we have no play and have drawn, pass
				gs.turn++;
		}

		gs.eightSuit = forces;
		if(likely(forces == UNKNOWN)) {	// If we aren't testing a suit
			if(likely(gtp != NULL))		// ...we either have a play
				e = unlikely(getVal(gtp->c[gtp->n-1]) == 8);
			else						// ...or draw/pass
				e = false;
		} else {						// Otherwise we're testing a suit
			e = true;
		}

		// TODO use silly magic about win distance to select better plays?
		grv = (unlikely(!gs.players[0].n) ? 1.0 : gameLoop(&gs, false, e, magic, aia));
		ret += (grv > 0.0);

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
			for(j = CLUBS; j <= SPADES; j++)
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

#if HAPPYHELGRIND
	if(dead[rplay])
#else
	/* We don't lock for trials here since it's not really important if we get
	 * a lower-than-actual number; we'd waste more time locking and the worst
	 * outcome is that we play a few more games than we needed to. */
	if(dead[rplay] || s->trials[rplay] >= MAXGAMES)
#endif
		return;

	snprintf(buf, BUFSZ, "p%4zu %4zu %1u %4zu", play, rplay, suit, hmg);
	mq_send(*s->mq, buf, 18, 1);
}

__attribute__((nonnull,hot)) static void controlThread(const struct pctmstate* const restrict s, const size_t ncpu)
{
	size_t i, j, k, m;
	const struct play* restrict play;
	const size_t nump = s->nplays;
	size_t hmg = 20;	// How many games to play the first round
	bool dead[nump];
	float scores[nump];
	float best, ath, co;
	size_t tt, btn;
	size_t act = nump;
	size_t times = 1;

	memset(dead, 0, sizeof(bool)*nump);

	while(act) {
		if(act > 1) {
			/* Send enough games to saturate our threads before checking */
			times = (act < ncpu) ? ceilf((float)ncpu / (float)act) : 1;
			for(k = 0; k < times; k++) {
				for(i = 0, m = 0; i < s->as->pl->n; i++) {
					play = plistGet(s->as->pl, i);
					if(unlikely(getVal(play->c[play->n-1]) == 8)) {
						for(j = CLUBS; j <= SPADES; j++)
							sendPlay(s, i, m++, j, hmg, dead);
					} else {
						sendPlay(s, i, m++, UNKNOWN, hmg, dead);
					}
				}
				sendPlay(s, i, m, UNKNOWN, hmg, dead);
			}
			hmg += hmg / 8;
			usleep(4000);
		}

		pthread_rwlock_rdlock(s->rwl);
		best = -1.0;
		btn = 0;
		for(j = 0; j < nump; j++) {
			if(dead[j])
				continue;
			tt = s->trials[j];
			if((scores[j] = (float)s->wins[j] / (float)tt) > best) {
				best = scores[j];
				btn = tt;
			}
		}

		for(j = 0; j < nump; j++) {
			if(dead[j])
				continue;
			tt = s->trials[j];
			ath = min(tt, btn);
			if(ath < 5)
				continue;
			// TODO tune these floats
			co = 6.1*log(ath)/pow((double)ath, 1.1);
			if(best - scores[j] > co || tt > MAXGAMES || act == 1) {
				dead[j] = true;
				act--;
#ifdef MONTE_VERBOSE
				printf("%smonte:%s  %zu\t(%.1f%% :: %.1f / %.1f)\t%.1f%%\t", ANSI_CYAN, ANSI_DEFAULT, j, 100.0*((float)tt) / (float)MAXGAMES, 100.0* (best - scores[j]), 100.0*co, 100.0*scores[j]);
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
	const struct play* gtp;
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
			 * cccc 4 bytes, how many games to play
			 * 0	1 null byte */
				wp = atoi(tbuf+1);
				bucket = atoi(tbuf+6);
				forces = (suit_t)tbuf[11] - 48;
				ngames = atoi(tbuf+13);
				ti = ngames;
				gtp = unlikely(wp == s->as->pl->n) ? NULL : plistGet(s->as->pl, wp);
				tt = playHypoGames(ngames, gtp, forces, s->as, s->aif, s->initgs);
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

static uint_fast32_t findBest(const struct pctmstate* const restrict s)
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
			ESPACK(ret, CLUBS);
			if(unlikely(i && m == s->emap[i-1])) {
				ESPACK(ret, DIAMONDS);
				if(i > 1 && s->emap[i-1] == s->emap[i-2]) {
					ESPACK(ret, HEARTS);
					if(i > 2 && s->emap[i-2] == s->emap[i-3]) {
						ESPACK(ret, SPADES);
					}
				}
			}
		}
	}
	return ret;
}

uint_fast32_t pctmRun(const struct aistate* const restrict as, void (*initgs)(struct gamestate* const restrict, const struct gamestate* const restrict), uint_fast32_t (*aif)(const struct aistate* const restrict))
{
	{	assert(as);
		assert(as->gs);
		assert(as->pl);
		assert(as->pl->n);
		assert(as->gs->nplayers >= MINPLRS && as->gs->nplayers <= MAXPLRS);}

	/* How many moves we have counting 8-ending plays as 4-in-1 and draw/pass
	 * as a move.  emap / initMoveMap() allow us to map from one number to the
	 * play number and eight suit that we are expected to return. */
	const size_t nump = eightMoveCount(as->pl);
	const size_t ncpu = sysconf(_SC_NPROCESSORS_ONLN) + 1;
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
	launchThreads(threads, ncpu, monteThread, &pcs);
	controlThread(&pcs, ncpu);
	threadsDone(ncpu, threads, &mq, mqname);
	pthread_rwlock_destroy(&rwl);
	return findBest(&pcs);
}

uint_fast32_t aiMonte(const struct aistate* const restrict as)
{
	return pctmRun(as, initGameStateHypothetical, MONTEAIF);
}
