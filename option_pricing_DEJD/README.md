# Application of Double Exponential Jump Diffusion for Option Pricing

## Directories
```C++
option_pricing_DEJD
├─DEJDsimulation  // codes
├─inputs
├─log         // log for maximizing MLE iteration
├─outputs
└─reports
```

## Abstract
When pricing for options, an essential issue is the accuracy in estimating the price of the underlying security. Observed that the best-known Black-Scholes model is unable to explain several economic facts when modeling stock price dynamics, the resulting option price under this model is not conclusive. This report focus on pricing European options using an advanced model and implement it using C++. In particular, we reproduce the asymmetric leptokurtic feature of stock return and simulate the price for European options by double exponential jump diffusion (DEJD) model. We also put an effort in parameter estimation through maximum likelihood estimation (MLE) using SPY adjusted close price. <br>
*(Option Pricing; double exponential jump diffusion; asymmetric leptokurtic; MLE)*

## Results

For more details, see [./results.xlsx](https://github.com/RyanMuQ/Projects/blob/master/option_pricing_DEJD/results.xlsx).

#### Black-Scholes model vs. DEJD model

![Aaron Swartz](https://github.com/RyanMuQ/Projects/blob/master/option_pricing_DEJD/result.PNG)
