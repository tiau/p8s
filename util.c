#include "util.h"

int cmpcardt(const void* const c1, const void* const c2)
{
	{	assert(c1);
		assert(c2);}
	return *((const card_t* const)c1) - *((const card_t* const)c2);
}
