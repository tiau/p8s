#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <mqueue.h>
#include "draw.h"

/* The Monte (Carlo) AI runs a whole bunch of games for each possible play it
 * might make.  One of the simpler AIs is used for these games.  Each game is
 * different as the AI shuffles all the cards that could be in the deck and its
 * opponents hands together and then deals out the right number of cards to its
 * opponents.  Eventually a single play appears so much better than the others
 * that it is selected as the play to make, or the AI plays so many games that
 * it gives up on telling which of the remaining plays is better and just makes
 * the best play found so far. */

#ifndef AI_MONTE_H
#define AI_MONTE_H

/* Changing this to true will prevent helgrind (drd doesn't seem to care) from
 * complaining about sendPlay() and monteThread() having data races.  They do,
 * but not in a way that matters. */
#define HAPPYHELGRIND false

/* Number of bytes sent per pipe message */
#define BUFSZ 18

/* What AI to call to play hypothetical games */
#define MONTEAIF aiJudge

/* The probability that apparently worse move W is actually better than best
 * apparent move B at which we cut off W from further computation */
#define PIBM 0.01

/* Maximum number of hypothetical games to play for a each possible move */
#define MAXGAMES 9999

/* Extra state for producer-consumer thread model */
struct pctmstate {
	const struct aistate* const as;
	const size_t nplays;		// How many different plays we're considering
	size_t* const trials;		// How many trials we've run for each play
	size_t* const wins;			// ... wins ...
	const size_t* const emap;	// Map for 8-ending plays, see pctmRun()
	const mqd_t* const mq;
	pthread_rwlock_t* const rwl;
	uint_fast32_t (*aif)(const struct aistate* const restrict);
	void (*initgs)(struct gamestate* const restrict, const struct gamestate* const restrict);
};

void dealStateSans(struct gamestate* const restrict gs,
				   const struct gamestate* const restrict ogs)
	__attribute__((hot,nonnull));

/* N.B. Caller must free gs->players */
void initGameStateHypoShared(struct gamestate* const restrict gs,
							 const struct gamestate* const restrict ogs)
	__attribute__((hot,nonnull));

uint_fast32_t pctmRun(const struct aistate* const restrict as,
					  void (*initgs)(struct gamestate* const restrict, const struct gamestate* const restrict),
					  uint_fast32_t (*aif)(const struct aistate* const restrict))
	__attribute__((hot,nonnull));

uint_fast32_t aiMonte(const struct aistate* const restrict as)
	__attribute__((hot,pure,nonnull));

#endif /* AI_MONTE_H */
