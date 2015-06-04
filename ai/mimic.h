#include "monte.h"
#include "random.h"

/* The Stacked AI is the same as the Monte AI except that instead of just
 * randomly shuffling all the cards that it doesn't know about (opponent's
 * hands and the deck) it tends to put magic cards (2s, 3s, 8s) into its
 * opponents hands, as smart players will likely be holding these cards. */

#ifndef AI_MIMIC_H
#define AI_MIMIC_H

/* What AI to call to play hypothetical games */
#define MIMICAIF aiJudge

struct btmstate {
	const struct aistate* const as;
	size_t* const trials;
	size_t* const wins;
	const size_t* const emap;
	pthread_barrier_t* const rbar;
	pthread_barrier_t* const racebarr;
	pthread_barrier_t* const actbarr;
	pthread_barrier_t* const actbarr2;
	size_t barrsize;
	float* const race;
	size_t idx;
	const size_t tott;
	size_t* const active;
	const size_t maxTrials;
	const struct play* play;
	uint_fast32_t (* const aif)(const struct aistate* const restrict);
	suit_t forces;
};

void stackTheDeck(struct deck* const restrict deck,
				  const size_t tch)
	__attribute__((hot,nonnull));
    
void initStackedGameStateHypothetical(struct gamestate* const restrict gs,
									  const struct gamestate* const restrict ogs)
	__attribute__((hot,nonnull));

void* mimicThread(void* arg) __attribute__((hot,nonnull));

void hypoDpkg(const uint_fast64_t in, size_t* restrict per, float* restrict n) __attribute__((hot,nonnull));
    
uint_fast32_t aiMimic(const struct aistate* const restrict as)
	__attribute__((hot,pure,nonnull));

#endif /* AI_MIMIC_H */
