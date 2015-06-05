#include "defines.h"
#include "ai/human.h"
#include "ai/first.h"
#include "ai/random.h"
#include "ai/shedder.h"
#include "ai/judge.h"
#include "ai/draw.h"
#include "ai/monte.h"
#include "ai/stacked.h"
#include "ai/cheat.h"

#ifndef AI_H
#define AI_H

/* Number of AIs, counting human */
#define NAI 9

static uint_fast32_t (*ais[NAI])(const struct aistate* const restrict) =
{
	&human,		// 0
	&aiFirst,	// 1
	&aiRandom,	// 2
	&aiShedder,	// 3
	&aiJudge,	// 4
	&aiDraw,	// 5
	&aiMonte,	// 6
	&aiStacked,	// 7
	&aiCheat	// 8
};

static const char *aiDesc[] =
{
	"human-controlled",
	"first move",
	"random move",
	"sheds most",
	"judges",
	"judges&draws",
	"monte carlo",
	"stacked monte carlo",
	"cheats"
};

#endif /* AI_H */
