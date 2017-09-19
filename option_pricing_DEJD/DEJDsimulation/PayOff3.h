#ifndef PAYOFF3_H
#define PAYOFF3_H

// Virtual class PayOff
class PayOff
{
public:
	virtual double operator()(double spot) const = 0;		//Virtual function
	virtual PayOff * clone() const = 0;						//Virtual fucntion
private:
};

class PayOffCall : public PayOff
{
public:
	PayOffCall(double strike);

	virtual double operator()(double spot) const;			//Inherit function which realize its own payoff function for call
	virtual ~PayOffCall() {}								//Destructor
	virtual PayOff * clone() const;							//Inherit function which realize clone

private:
	double strike_;
};

class PayOffPut : public PayOff
{
public:
	PayOffPut(double strike);

	virtual double operator()(double spot) const;			//Inherit function which realize its own payoff function for put
	virtual ~PayOffPut() {}									//Destructor
	virtual PayOff * clone() const;							//Inherit function which realize clone

private:
	double strike_;
};

#endif