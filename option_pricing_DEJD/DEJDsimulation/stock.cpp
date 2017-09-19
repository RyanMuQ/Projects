#include "stock.h"
#include <random>
#include <vector>
#include <time.h>

//Constructor which send in the parameter from the map at the same time
Stock::Stock(const Parameters& para, int nGrid):interest_rate_( para["interest_rate"]),
sigma_ (para["sigma"]), eta_left_ ( para["eta_left"]), eta_right_ (para["eta_right"]),
lambda_ (para["lambda"]), prob_ (para["prob"]), nGrid_ (nGrid) 
{
}

//A normal generator which is used for brownian motion term
double Stock::generate_Random_Normal()
{
	std::random_device rd;
	unsigned int my_seed = rd();
	std::mt19937_64 rng(my_seed);
	std::normal_distribution<double> norm(0, 1);

	double rnorm = norm(rng);
	return rnorm;
}

//A poisson generator for simulating the frequency of the jump
int Stock::simulate_Poisson(double t)
{
	std::random_device rd;
	unsigned int my_seed = rd();
	std::mt19937_64 rng(my_seed);
	std::uniform_real_distribution<double> uni(0, 1);	//Generate a normal(0, 1) random number 

	double L = exp(-lambda_ * t);
	int k = 0;
	double p = 1;
	while (p >= L)										//When the random number pass the test
	{
		k++;											//We succesffully take one step further
		p = p * uni(rng);								//And prepare for next step
	}
	return k - 1;										//The number of steps is poisson-distributed
}

//A double exponential generator for simulating the range of the jump, the inverse distribution function is pre-calculated
double Stock::simulate_Double_Exponential()
{
	std::random_device rd;
	unsigned int my_seed = rd();
	std::mt19937_64 rng(my_seed);
	std::uniform_real_distribution<double> uni(0, 1);	//Generate a normal(0, 1) random number for inverse distribution function

	double u = uni(rng);								//This uni(0, 1) is for inverse distribution function
	double indicator = uni(rng);						//This uni(0, 1) is for test of up/down jump
	double X = 0;										//Jump range
	if (indicator < prob_)								//Up jump
		X = -1 / eta_right_*log(1 - u);		//Plug into up jump inverse distribution function
	else if (indicator > prob_)							//Down jump
		X = 1 / eta_left_*log(u);			//Plug into down jump inverse distribution function
	return X;											//Return the jump range
}

//One step stock price generator
double Stock::simulate_BS(double maturity, double S0)	
{
	double spot_price;
  bm_term_ = sqrt(maturity) * generate_Random_Normal();				//Generate the brownian motion term
	spot_price = S0 * exp((interest_rate_ - 0.5*sigma_*sigma_)*maturity + sigma_*bm_term_);							//Spot price's dynamic is a geometric brownian motion
	return spot_price;
}

//One step jump diffusion model generator
double Stock::simulate_DEJD(double maturity, double S0)	
{
	std::random_device rd;
	unsigned int my_seed = rd();
	std::mt19937_64 rng(my_seed);
	std::normal_distribution<double> norm(0, 1);

	//Directly using formula for S(T)
	double spot_price;
	double zeta = (prob_*eta_right_) / (eta_right_ - 1) + ((1 - prob_)*eta_left_) / (eta_left_ + 1) - 1;			//Compute zeta given formula
	int Nt = Stock::simulate_Poisson(maturity);					//Simulate the number of jumps
	std::vector<double> Yt(Nt);
	double sum_Yi = 0;
	for (int i = 0; i < Nt; i++)
	{
		Yt[i] = simulate_Double_Exponential();			//For each jump, simulate the jump range
		sum_Yi = sum_Yi + Yt[i];						//Sum up all the jumps
	}
  bm_term_ = sqrt(maturity) * generate_Random_Normal();				//Generate the brownian motion term
	//spot_price = S0 * exp((interest_rate_ - 0.5*sigma_*sigma_)*maturity + sigma_*bm_term_ + sum_Yi);	//Simulate the jump diffusion stock price
	spot_price = S0 * exp((interest_rate_ - 0.5*sigma_*sigma_ - lambda_*zeta)*maturity + sigma_*bm_term_ + sum_Yi);//这一行和上一行分别是加了zeta和不加zeta的公式，你看着用
	double test = S0 * exp((interest_rate_ - 0.5*sigma_*sigma_ - lambda_*zeta)*maturity + sigma_*bm_term_);
	return spot_price;
}