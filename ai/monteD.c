#include "monteD.h"

uint_fast32_t aiMonteD(const struct aistate* const restrict as)
{
	return pctmRun(as, initGameStateHypothetical, aiDraw);
}
