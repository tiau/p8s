#include <math.h>
#include "defines.h"

#ifndef PROBABILITY_H
#define PROBABILITY_H

__attribute__((hot,const)) inline static double se(const double p, const size_t n1, const size_t n2)
{
	return sqrt(p / (n1+n2) * (1-p / (n1+n2)) * (1.0/n1 + 1.0/n2));
}

__attribute__((hot,const)) inline static double z(const double p1, const double p2, const size_t n1, const size_t n2)
{
	return (p1/n1 - p2/n2) / se(p1+p2, n1, n2);
}

__attribute__((hot,const)) inline static double phi(double x)
{
	return .5*erfc(-x/sqrt(2));
}

#endif /* PROBABILITY_H */
