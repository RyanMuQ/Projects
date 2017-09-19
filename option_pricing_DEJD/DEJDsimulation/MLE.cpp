#include <cmath>
#include <iostream>
#include <map>
#include "gauss_legendre/gauss_legendre.h"
#include "Parameters.h"
#include "MLE.h"
#include "timeseries.h"
#include "dlib/optimization.h"

/*
  Here we do optimation using dlib/optimization.h.
  http://dlib.net/optimization.html#find_min_bobyqa
  Numerical integration is needed to calculate rosen function (likelihood function).
  http://www.holoborodko.com/pavel/numerical-methods/numerical-integration/
  Spectial thanks for authors.
*/

#ifndef PI
	#define PI 3.1415926535897932384626433832795028841971693993751
#endif

#ifndef E
  #define E 2.71828182846
#endif

#define N_ORDER 20 // Polynomial degrees while integrating
#define DAYS_IN_ONE_YEAR 254

#define CONSIDER_MN true

const double epsilon = 1e-2;

using namespace dlib;

typedef matrix<double, 0, 1> column_vector;

TimeSeries ts;

inline int round2int(double x) {
  if (x < 0) return -int(-x + epsilon);
  else return int(x + epsilon);
}

// calculate x^p where p is non-negative integer
inline double smart_pow(double x, double p) { 
  double ret = 1, now = x;
  int p_ = round2int(p);
  if (p_ < 0) { std::cout << "p_ < 0" << std::endl; throw ""; }
  for (int i = 1; i <= p_; i <<= 1, now*=now) {
    if (i & p_) ret *= now;
  }
  return ret;
}

// Calculate n! using Stirling's approximation
inline double factorial(double n)
{
  int n_ = round2int(n);
  if (n_ <= 10) {
    double ret = 1;
    for (int i = 2; i <= n_; ret *= i, i++);
    return ret;
  }
  return std::sqrt(2 * PI * n) * smart_pow(n / E, n);
}

double P(double n, double lambda) {
  return std::exp(-lambda) * smart_pow(lambda, n) / factorial(n);
}

double I_t1(double x, void* data)
{
  const Parameters& p = *((Parameters*)data);
  return 1 / smart_pow(-x, p["n"] + 1) * smart_pow(1 / p["t"] - 1 / x, p["m"] - 1) *
    std::exp(p["eta_u"] * (1 / x - 1 / p["t"]) + p["eta_d"] / x);
}

double I_t2(double x, void* data)
{
  const Parameters& p = *((Parameters*)data);
  return 1 / smart_pow(-x, p["n"] + 1) * smart_pow(p["t"] - 1 / x, p["m"] - 1) *
    std::exp(p["eta_u"] * (1 / x - p["t"]) + p["eta_d"] / x);
}

double I_t3(double x, void* data)
{
  const Parameters& p = *((Parameters*)data);
  return smart_pow(-x, p["n"] - 1) * smart_pow(p["t"] - x, p["m"] - 1) *
    std::exp(p["eta_u"] * (x - p["t"]) + p["eta_d"] * x);
}

double I_t4(double x, void* data)
{
  const Parameters& p = *((Parameters*)data);
  return smart_pow(-x, p["n"] - 1) * smart_pow(1 / p["t"] - x, p["m"] - 1) *
    std::exp(p["eta_u"] * (x - 1 / p["t"]) + p["eta_d"] * x);
}

double g_0n(double x, void* data)
{
  const Parameters& p = *((Parameters*)data);
  return smart_pow(-x, p["n"] - 1) *
    std::exp(p["eta_d"] * x - smart_pow(p["r"] - x - p["mu"] * p["s"] + p["sigma"] * p["sigma"] / 2. * p["s"], 2)
      / (2. * p["sigma"] * p["sigma"] * p["s"]));
}

double h_0n(double x, void* data)
{
  const Parameters& p = *((Parameters*)data);
  return 1.0 / smart_pow(-x, p["n"] + 1) *
    std::exp(p["eta_d"] / x - smart_pow(p["r"] - 1. / x - p["mu"] * p["s"] + p["sigma"] * p["sigma"] / 2. * p["s"], 2)
      / (2. * p["sigma"] * p["sigma"] * p["s"]));
}

double g_m0(double x, void* data)
{
  const Parameters& p = *((Parameters*)data);
  return smart_pow(x, p["m"] - 1) *
    std::exp(- p["eta_u"] * x - smart_pow(p["r"] - x - p["mu"] * p["s"] + p["sigma"] * p["sigma"] / 2. * p["s"], 2)
      / (2. * p["sigma"] * p["sigma"] * p["s"]));
}

double h_m0(double x, void* data)
{
  const Parameters& p = *((Parameters*)data);
  return 1.0 / smart_pow(x, p["m"] + 1) *
    std::exp(-p["eta_u"] / x - smart_pow(p["r"] - 1. / x - p["mu"] * p["s"] + p["sigma"] * p["sigma"] / 2. * p["s"], 2)
      / (2. * p["sigma"] * p["sigma"] * p["s"]));
}

double h_mn(double x, void* data) {
  const Parameters& p = *((Parameters*)data);
  return std::exp(-smart_pow(p["r"] - 1 / p["t"] - p["mu"] * p["s"] + p["sigma"] * p["sigma"] / 2. * p["s"], 2)
    / (2 * p["sigma"] * p["sigma"] * p["s"])) / (p["t"] * p["t"]);
}

double g_mn(double t, void* data) {
  Parameters p = *((Parameters*)data);
  return std::exp(-smart_pow(p["r"] - p["t"] - p["mu"] * p["s"] + p["sigma"] * p["sigma"] / 2. * p["s"], 2)
    / (2 * p["sigma"] * p["sigma"] * p["s"]));
}

double f_00(double r, void* data) {
  const Parameters& p = *((Parameters*)data);
  return 1.0 / (std::sqrt(PI * 2 * p["s"]) * p["sigma"]) *
    std::exp(-smart_pow(p["r"] - p["mu"] * p["s"] + p["sigma"] * p["sigma"] / 2. * p["s"], 2)
      / (2. * p["sigma"] * p["sigma"] * p["s"]));
}

double f_0n(double r, void* data) {
  Parameters& p = *((Parameters*)data);
  double integral_g_0n = gauss_legendre(N_ORDER, g_0n, &p, -1, 0);
  double integral_h_0n = gauss_legendre(N_ORDER, h_0n, &p, -1, 0);
  return smart_pow(p["eta_d"], p["n"]) / (factorial(p["n"] - 1) * std::sqrt(2 * PI * p["s"]) * p["sigma"])
    * (integral_g_0n + integral_h_0n);
}

double f_m0(double r, void* data) {
  Parameters& p = *((Parameters*)data);
  double integral_g_m0 = gauss_legendre(N_ORDER, g_m0, &p, 0, 1);
  double integral_h_m0 = gauss_legendre(N_ORDER, h_m0, &p, 0, 1);
  return smart_pow(p["eta_u"], p["m"]) / (factorial(p["m"] - 1) * std::sqrt(2 * PI * p["s"]) * p["sigma"])
    * (integral_g_m0 + integral_h_m0);
}

double f_mn_1(double t, void* data) {
  Parameters p;
  p = *((Parameters*)data);
  p.add("t", t);
  return h_mn(t, &p) * gauss_legendre(N_ORDER, I_t1, &p, p["t"], 0);
}

double f_mn_2(double t, void* data) {
  Parameters p;
  p = *((Parameters*)data);
  p.add("t", t);
  return g_mn(t, &p) * gauss_legendre(N_ORDER, I_t2, &p, 1 / p["t"], 0);
}

double f_mn_3(double t, void* data) {
  Parameters p;
  p = *((Parameters*)data);
  p.add("t", t);
  return g_mn(t, &p) * (
    gauss_legendre(N_ORDER, I_t2, &p, -1, 0) +
    gauss_legendre(N_ORDER, I_t3, &p, -1, 0));
}

double f_mn_4(double t, void* data) {
  Parameters p;
  p = *((Parameters*)data);
  p.add("t", t);
  return h_mn(t, &p) * (
    gauss_legendre(N_ORDER, I_t1, &p, -1, 0) +
    gauss_legendre(N_ORDER, I_t4, &p, -1, 0));
}

double f_mn(double r, void* data) {
  Parameters p;
  p = *((Parameters*)data);
  p.add("r", r);
  double pp = smart_pow(p["eta_u"], p["m"]) * smart_pow(p["eta_d"], p["n"])
    / (factorial(p["m"] - 1) * factorial(p["n"] - 1) * std::sqrt(2 * PI * p["s"]) * p["sigma"]);
  return pp * (
    gauss_legendre(N_ORDER, f_mn_1, &p, -1, 0) +
    gauss_legendre(N_ORDER, f_mn_2, &p, -1, 0) +
    gauss_legendre(N_ORDER, f_mn_3, &p, 0, 1) +
    gauss_legendre(N_ORDER, f_mn_4, &p, 0, 1));
}

double f(double r, void* data) {
  /*////////////////////////
  clock_t start, finish;
  double totaltime;
  start = clock();
  *//////////////////////////
  double coeff = epsilon * 0.1;
  Parameters p;
  p = *((Parameters*)data);
  p.add("r", r);
  double ret = 0;

  ret = std::exp(-(p["lambda_u"] + p["lambda_d"])) * f_00(r, &p);

  for (int n = 1; n <= 20; n++) {
    p.add("n", n);
    double th = std::exp(-p["lambda_u"]) * P(n, p["lambda_d"]) * f_0n(r, &p);
    ret += th;
    if (2 * fabs(th) < coeff * (ret + ret - th)) break;
  }

  for (int m = 1; m <= 20; m++) {
    p.add("m", m);
    double th = std::exp(-p["lambda_d"]) * P(m, p["lambda_u"]) * f_m0(r, &p);
    ret += th;
    if (2 * fabs(th) < coeff * (ret + ret - th)) break;
  }
  
  if (CONSIDER_MN) {
    for (int m = 1; m <= 20; m++) {
      p.add("m", m);
      bool last_loop = true;
      for (int n = 1; n <= 20; n++) {
        p.add("n", n);
        double th = P(n, p["lambda_d"]) * P(m, p["lambda_u"]) * f_mn(r, &p);
        ret += th;
        if (2 * fabs(th) < coeff * (ret + ret - th)) break;
        last_loop = false;
      }
      if (last_loop) break;
    }
  }
  /*/////////////////////////
  finish = clock();
  totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
  std::cout << "The processed time of this program is " << totaltime << " seconds." << std::endl;
  *//////////////////////////
  return ret;
}

int iteration_count;

double rosen(column_vector x) {
  std::map<std::string, double> M;
  M["mu"] = x(4) / 100;
  M["sigma"] = x(5) / 10;
  M["eta_u"] = x(0) * 1000;
  M["eta_d"] = x(1) * 1000;
  M["lambda_u"] = x(2);
  M["lambda_d"] = x(3);
  M["s"] = ts.days;
  Parameters para(M);

  std::cout << para;

  double ret = 0;
  for (int i = 0; i < ts.len(); i++) {
    ret += std::log(f(ts[i], &para));
  }

  std::cout << "ret[" << ++iteration_count << "] = " << ret << std::endl << std::endl;
  return ret;
}

Parameters MLE(const int YEARS_DATA, const std::string addr)
{
  ts = *(new TimeSeries("daily_returns"));
  switch (YEARS_DATA) {
  case 1: ts.read_csv(addr + "\\returns_1_year_2017.csv"); break;
  case 2: ts.read_csv(addr + "\\returns_1_year.csv"); break;
  case 4: ts.read_csv(addr + "\\returns_4_year.csv"); break;
  case 16: ts.read_csv(addr + "\\returns.csv"); break;
  default: std::cout << "ERROR: Wrong YEARS_DATA" << std::endl; throw "";
  }
  ts.days = ts.len();
  
  std::cout << "Sample mean              = " << ts.mean() << std::endl;
  std::cout << "Sample standard variance = " << ts.stdev() << std::endl << std::endl;

  column_vector para(6), lower(6), upper(6);
  // (x0, x1, x2, x3, x4, x5) = (eta_1 / 1000, eta_2 / 1000, lambda_1, lambda_2, mu * 100, sigma * 10)
  para  = 0.2, 0.2, 0.5, 0.5, ts.mean() * 100, ts.stdev() * 10;
  lower = 0.001, 0.001, 0.001, 0.001, -1., 0.001;
  upper = 1., 1., 1., 1., 1., 1.;

  iteration_count = 0;
  find_max_bobyqa(rosen,
                  para,            // starting point
                  9,               // number of interpolation points
                  lower,           // lower bound constraint
                  upper,           // upper bound constraint
                  0.4,             // initial trust region radius
                  epsilon * 1e-2,  // stopping trust region radius
                  1000);           // max number of objective function evaluations

  std::cout << para << std::endl;

  std::map<std::string, double> M;
  M["interest_rate"] = para(4) / 100;
  M["sigma"] = para(5) / 10;
  M["eta_left"] = para(1) * 1000;             // eta_left = eta_u
  M["eta_right"] = para(0) * 1000;            // eta_right = eta_d
  M["lambda"] = para(2) + para(3);            // lambda = lambda_u + lambda_d
  M["prob"] = para(2) / (para(2) + para(3));  // p = lambda_u / lambda
  M["s"] = ts.days;
  M["sample_mu"] = ts.mean();
  M["sample_SD"] = ts.stdev();

  return Parameters(M);
}
