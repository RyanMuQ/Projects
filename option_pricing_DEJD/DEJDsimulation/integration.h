#ifndef _INTEGRATION_
#define _INTEGRATION_

#include "gauss_legendre/gauss_legendre.h"
#include "Parameters.h"

double integrate(int n, double(*f)(double, void*), void* para, double a, double b);

#endif
