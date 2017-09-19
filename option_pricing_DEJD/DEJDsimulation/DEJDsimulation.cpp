#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <ctime>
#include "MLE.h"
#include "parameters.h"
#include "stock.h"
#include "timeseries.h"

#define YEARS_DATA 1

const std::string addr = "C:\\Users\\ryanw\\Desktop\\MATH512\\Project_1\\DEJDsimulation";

void findParas() {
  Parameters para;
  para = MLE(YEARS_DATA, addr);
  switch (YEARS_DATA) {
  case 1: para.to_csv(addr + "\\parameters_1_year_2017.csv"); break;
  case 2: para.to_csv(addr + "\\parameters_1_year.csv"); break;
  case 4: para.to_csv(addr + "\\parameters_4_year.csv"); break;
  case 16: para.to_csv(addr + "\\parameters.csv"); break;
  default: std::cout << "ERROR: Wrong YEARS_DATA" << std::endl; throw "";
  }
}

void MonteCarlo() {
  std::map<std::string, double> M;														//Build a map to save parameters
  M["interest_rate"] = 0.00000545;															//Input exotic parameters
  M["sigma"] = 0.000225238;
  M["eta_left"] = 313.242;
  M["eta_right"] = 213.216;
  M["lambda"] = 1.24551;
  M["prob"] = 0.217218;

  Parameters para(M);																	//Save parameters into the map
  Stock jump(para);

  M["interest_rate"] = 0.0000247199;															//Input exotic parameters
  M["sigma"] = 0.00563322;

  Parameters para2(M);																	//Save parameters into the map
  Stock usual(para2);

  double maturity = 300;
  double S0 = 100;
  int nSamples = 10000;

  double usual_result = 0;
  double jump_result = 0;

  std::ofstream outfile;
  outfile.open(addr + "\\R\\test_ALF.csv");
  outfile << "usual" << "," << "jump" << std::endl;
  for (int i = 1; i <= nSamples; i++)
  {
    double usual_res = usual.simulate_BS(maturity, S0);
    double jump_res = jump.simulate_DEJD(maturity, S0);
    usual_result += usual_res;
    jump_result += jump_res;
    outfile << usual_res << "," << jump_res << std::endl;
  }
  outfile.close();

  std::cout << "usual stock price:" << usual_result / nSamples << std::endl;
  std::cout << "jump stock price:" << jump_result / nSamples << std::endl;
}

void findALF() {
  std::map<std::string, double> M;														// Build a map to save parameters
  M["interest_rate"] = 0.15 / 250;														// Input exotic parameters
  M["sigma"] = 0.2 / sqrt(250.0);
  M["eta_left"] = 25;
  M["eta_right"] = 50;
  M["lambda"] = 10.0 / 250;
  M["prob"] = 0.3;

  Parameters para(M);																	// Save parameters into the map
  Stock jump(para);

  double maturity = 1;
  double S0 = 100;
  int nSamples = 10000;

  std::vector<std::string> rowname(nSamples);
  std::vector<double> jumps(nSamples);
  std::vector<double> usuals(nSamples);

  for (int i = 0; i < nSamples; i++)
    rowname[i] = std::to_string(i);

  for (int i = 0; i < nSamples; i++) {
    double res = jump.simulate_DEJD(maturity, S0);
    jumps[i] = std::log(res / S0);
    if (i % (nSamples / 10) == 0) std::cout << '.';
  }
  std::cout << std::endl;

  TimeSeries ts_jump("jump", rowname, jumps);

  para.add("interest_rate", ts_jump.mean() / maturity);
  para.add("sigma", ts_jump.stdev() / std::sqrt(maturity));

  Stock usual(para);

  for (int i = 0; i < nSamples; i++) {
    double res = usual.simulate_BS(maturity, S0);
    usuals[i] = std::log(res / S0);
    if (i % (nSamples / 10) == 0) std::cout << '.';
  }
  std::cout << std::endl;

  TimeSeries ts_usual("usual", rowname, usuals);

  std::cout << ts_jump.head(6) << std::endl;
  std::cout << "mean: " << ts_jump.mean() << " stdev: " << ts_jump.stdev() << std::endl << std::endl;
  std::cout << ts_usual.head(6) << std::endl;
  std::cout << "mean: " << ts_usual.mean() << " stdev: " << ts_usual.stdev() << std::endl << std::endl;

  std::cout << "KURT: " << ts_jump.kurt() << " vs. " << ts_usual.kurt() << std::endl;
  std::cout << "SKEW: " << ts_jump.skew() << " vs. " << ts_usual.skew() << std::endl;

  ts_jump.write_csv(addr + "\\stock_price_jump.csv");
  ts_usual.write_csv(addr + "\\stock_price_usual.csv");
}

int main()
{
  clock_t start, finish;
  double totaltime;
  start = clock();

  findALF();

  finish = clock();
  totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
  std::cout << std::endl << "The processed time of this program is " << totaltime << " seconds." << std::endl;

  return 0;
}

