#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/time.h>
#include "draw.h"

/* The Monte (Carlo) AI runs a whole bunch of games for each possible play it
 * might make. One of the simpler AIs is used for these games. Each game is
 * different as the AI shuffles all the cards that could be in the deck and its
 * opponents hands together and then deals out the right number of cards to its
 * opponents. Eventually a single play appears so much better than the others
 * that it's selected as the play to make, or the AI plays so many games that
 * it gives up on determining which of the remaining plays is better and just
 * makes the best play found so far. */

#ifndef AI_MONTE_H
#define AI_MONTE_H

/* What AI to call to play hypothetical games */
__attribute__((unused)) static uint_fast32_t (*MonteSub)(const struct aistate* const restrict) = aiDraw;

/* The probability that apparently worse move W is actually better than best
 * apparent move B at which we cut off W from further computation. */
static const float pCutOff = 0.04;

/* Maximum number of hypothetical games to play for a each possible move */
static const size_t MaxGames = 200;

/* Minimum number of games to play with each move before ruling it out */
static const size_t MinGames = 8;

/* Number of bytes sent per message queue message */
#define BUFSZ 18

/* Extra state for producer-consumer thread model */
struct pctmstate {
	const struct aistate* const as;
	const size_t nplays;		// How many different plays we're considering
	size_t* const trials;		// How many trials we've run for each play
	size_t* const wins;			// How many wins we've got for each play
	const size_t* const emap;	// Map for 8-ending plays, see pctmRun()
	const mqd_t* const mq;
	pthread_rwlock_t* const rwl;
	uint_fast32_t (*aif)(const struct aistate* const restrict);
	void (*initgs)(struct gamestate* const restrict, const struct gamestate* const restrict);
};

void dealStateSans(struct gamestate* const restrict gs,
				   const struct gamestate* const restrict ogs)
	__attribute__((hot,nonnull));

void initGameStateHypoShared(struct gamestate* const restrict gs,
							 const struct gamestate* const restrict ogs)
	__attribute__((hot,nonnull));

uint_fast32_t pctmRun(const struct aistate* const restrict as,
					  void (*initgs)(struct gamestate* const restrict, const struct gamestate* const restrict),
					  uint_fast32_t (*aif)(const struct aistate* const restrict),
					  const char* const restrict name)
	__attribute__((hot,nonnull));

uint_fast32_t aiMonte(const struct aistate* const restrict as)
	__attribute__((hot,nonnull));

#endif /* AI_MONTE_H */
