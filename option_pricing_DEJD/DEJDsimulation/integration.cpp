#include "integration.h"
#include "Parameters.h"
#include <cmath>
#include <iostream>

double integrate(int n, double(*f)(double, void*), void* para, double a, double b) {

  double ret = 0.0;
  double step = 5.0;
  double coeff = 1e-6;

  if (std::isinf(a) && std::isinf(b))
    return integrate(n, f, para, a, 0) + integrate(n, f, para, 0, b);

  if (std::isinf(a)) {
    double th = 1.0;
    for (double now = b; std::fabs(th) * 2 > coeff * (ret + ret + th); now -= step) {
      th = integrate(n, f, para, now - step, now);
      ret += th;
    }
    return ret;
  }

  if (std::isinf(b)) {
    double th = 1.0;
    for (double now = a; std::fabs(th) * 2 > coeff * (ret + ret + th); now += step) {
      th = integrate(n, f, para, now, now + step);
      ret += th;
    }
    return ret;
  }

  ret = gauss_legendre(n, f, para, a, b);
  return ret;
}