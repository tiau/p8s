#include <math.h>
#include "defines.h"

#ifndef PROBABILITY_H
#define PROBABILITY_H

double se(const double p,
		  const size_t n1,
		  const size_t n2)
	__attribute__((hot,const));

double z(const double p1,
		 const double p2,
		 const size_t n1,
		 const size_t n2)
	__attribute__((hot,const));

double phi(double x)
	__attribute__((hot,const));

#endif /* PROBABILITY_H */
