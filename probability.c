#include "probability.h"

double se(const double p, const size_t n1, const size_t n2)
{
	return sqrt(p / (n1+n2) * (1-p / (n1+n2)) * (1.0/n1 + 1.0/n2));
}

double z(const double p1, const double p2, const size_t n1, const size_t n2)
{
	return (p1/n1 - p2/n2) / se(p1+p2, n1, n2);
}

double phi(double x)
{
	static const double a1 =  0.254829592;
	static const double a2 = -0.284496736;
	static const double a3 =  1.421413741;
	static const double a4 = -1.453152027;
	static const double a5 =  1.061405429;
	static const double p  =  0.3275911;

	const int sign = (x < 0) ? -1 : 1;
	x = fabs(x)/sqrt(2.0);
	const double t = 1.0/(1.0 + p*x);
	const double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);
	return 0.5*(1.0 + sign*y);
}
