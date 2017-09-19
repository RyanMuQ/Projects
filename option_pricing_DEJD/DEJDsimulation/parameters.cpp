#include "parameters.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

std::ostream& operator << (std::ostream& output, const Parameters& para)
{
  for (auto const& ent : para.M) {
    output << std::setw(13) << ent.first << " = " << ent.second << std::endl;
  }
  return output;
}

Parameters::Parameters() {}

Parameters::Parameters(std::map<std::string, double> M_) : M(M_) {}

void Parameters::add(std::string key, double val) {
  M[key] = val;
}

double Parameters::operator [] (std::string parameter_name) const
{
  return M.at(parameter_name);
}

void Parameters::to_csv(std::string filename, char sep) const
{
  std::ofstream fout;
  fout.open(filename);

  for (auto const& ent : M) {
    fout << ent.first << sep << ent.second << std::endl;
  }

  fout.close();
}