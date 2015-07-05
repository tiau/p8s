#include "monteS.h"

uint_fast32_t aiMonteS(const struct aistate* const restrict as)
{
	return pctmRun(as, initGameStateHypothetical, aiShedder);
}
