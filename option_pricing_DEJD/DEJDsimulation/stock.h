#ifndef STOCK_H
#define STOCK_H

#include "parameters.h"

class Stock {
private:
  double interest_rate_, sigma_, eta_left_, eta_right_, lambda_, prob_;	//Parameters used for generating spot prices
  double bm_term_;														//Brownian motion term
  int nGrid_;															//Step of simulation
public:
  Stock(const Parameters& para, int nGrid=100);							//Constructor
  double generate_Random_Normal();										//Normal generator
  int simulate_Poisson(double t);												//Poisson generator
  double simulate_Double_Exponential();									//Double exponential generator
  double simulate_BS(double maturity, double S0);						//Simulate stock price by one step 
  double simulate_DEJD(double maturity, double S0);						//Simulate stock price using double expenotial jump diffusion model
};

#endif
