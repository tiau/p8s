#include "monteF.h"

uint_fast32_t aiMonteF(const struct aistate* const restrict as)
{
	return pctmRun(as, initGameStateHypothetical, aiFirst);
}
