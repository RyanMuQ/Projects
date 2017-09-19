#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <map>
#include <string>

class Parameters {
private:
  // double interest_rate_, sigma_, eta_left_, eta_right_, lambda_, prob_;
  std::map<std::string, double> M;                                                  // all parameters are stored here
  friend std::ostream& operator << (std::ostream& output, const Parameters& para);  // output
public:
  Parameters(); // do nothing
  Parameters(std::map<std::string,double> M_);
  void add(std::string key, double val);
  double operator [] (std::string parameter_name) const;                            // return the value of the parameter requested
  void to_csv(std::string filename, char sep = ',') const;                          // save parameters as .csv file
};

#endif
