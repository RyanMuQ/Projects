#ifndef VANILLA_H
#define VANILLA_H

#include "PayOff3.h"													//The private payoff_ptr saves the pointer of the payoff class

class VanillaOption
{
public:
	VanillaOption(const PayOff& the_payoff, double maturity);			
	VanillaOption(const VanillaOption& original);
	VanillaOption& operator=(const VanillaOption& original);
	~VanillaOption();													//Destructor

	double get_maturity() const;										//Access the time to maturity
	double payoff(double spot) const;									//Access the payoff of option

private:
	double maturity_;													//Maturity date
	PayOff* payoff_ptr_;												//Pointer of a Payoff Class
};

#endif