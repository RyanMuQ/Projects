#include "PayOff3.h"
#include <algorithm>
using std::max;

PayOffCall::PayOffCall(double strike) : strike_(strike)
{
}

double PayOffCall::operator()(double spot) const
{
	return max(spot - strike_, 0.0);						//Calculate the call options payoff
}

PayOff* PayOffCall::clone() const
{
	return new PayOffCall(*this);							//Return the pointer
}

PayOffPut::PayOffPut(double strike) : strike_(strike)
{
}

double PayOffPut::operator()(double spot) const
{
	return max(strike_ - spot, 0.0);						//Calculate the put payoff
}

PayOff* PayOffPut::clone() const
{
	return new PayOffPut(*this);							//Return the pointer
}
