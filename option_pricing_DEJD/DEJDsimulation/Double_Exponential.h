#ifndef DOUBLEEXPO_H
#define DOUBLEEXPO_H

#include "parameters.h"

class Double_Exponential
{
private:
	double eta_left_, eta_right_, prob_;
public:
	Double_Exponential(); // do nothing
	Double_Exponential(const Parameters& para);
	double simulate_Double_Exponential();
};

#endif
