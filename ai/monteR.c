#include "monteR.h"

uint_fast32_t aiMonteR(const struct aistate* const restrict as)
{
	return pctmRun(as, initGameStateHypothetical, aiRandom);
}
