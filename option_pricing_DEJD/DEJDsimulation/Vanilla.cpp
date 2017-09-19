#include "Vanilla.h"

//consructor
VanillaOption::VanillaOption(const PayOff& the_payoff,
	double maturity)
{
	maturity_ = maturity;
	payoff_ptr_ = the_payoff.clone();
}

//Constructor of copy
VanillaOption::VanillaOption(const VanillaOption& original)
{
	maturity_ = original.maturity_;
	payoff_ptr_ = original.payoff_ptr_->clone();
}

//Constructor using "="
VanillaOption& VanillaOption::operator=(const VanillaOption& original)
{
	if (this != &original)
	{
		maturity_ = original.maturity_;
		delete payoff_ptr_;
		payoff_ptr_ = original.payoff_ptr_->clone();
	}

	return *this;
}

//Destructor
VanillaOption::~VanillaOption()
{
	delete payoff_ptr_;
}

//Access to maturity date
double VanillaOption::get_maturity() const
{
	return maturity_;
}

//Access to payoff given spot price
double VanillaOption::payoff(double spot) const
{
	return (*payoff_ptr_)(spot);
}

