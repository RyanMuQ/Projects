#include "timeseries.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

using namespace std;

double to_double(string s) {
  istringstream st(s);
  double token;
  st >> token;
  return token;
}

TimeSeries::TimeSeries() {}

TimeSeries::TimeSeries(string name_) : name(name_){}

TimeSeries::TimeSeries(string name_, const vector<string>& rowname_, const vector<double> d_) :
  name(name_), rowname(rowname_), d(d_)
{
  mean_ = get_mean();
  stdev_ = get_stdev();
}

ostream& operator << (ostream& output, const TimeSeries& ts)
{
  for (int i = 0; i < ts.rowname.size(); i++) {
    output << setw(10) << ts.rowname[i] << setw(16) << fixed << setprecision(6) << ts.d[i] << endl;
  }
  output << "nrows: " << ts.rowname.size() << "\t" << ts.name  << endl;
  return output;
}

void TimeSeries::read_csv(string addr) {
  ifstream fin;
  fin.open(addr);

  string token;
  while (getline(fin, token, ',')) {
    rowname.push_back(token);
    getline(fin, token, '\n');
    d.push_back(to_double(token));
  }

  fin.close();

  mean_ = get_mean();
  stdev_ = get_stdev();
}

void TimeSeries::write_csv(string addr, char sep) const {
  ofstream fout;
  fout.open(addr);

  for (int i = 0; i < rowname.size(); i++) {
    fout << rowname[i] << sep << d[i] << std::endl;
  }

  fout.close();
}

double TimeSeries::operator [](size_t ind) const {
  return d.at(ind);
}

size_t TimeSeries::len() const {
  return rowname.size();
}

double TimeSeries::get_mean() const {
  double ret = 0;
  for (int i = 0; i < rowname.size(); i++)
    ret += d[i];
  return ret / rowname.size();
}

double TimeSeries::get_stdev() const {
  double ret = 0;
  double m = get_mean();
  for (int i = 0; i < rowname.size(); i++)
    ret += pow(d[i] - m, 2);
  return sqrt(ret / (rowname.size() - 1));
}

double TimeSeries::mean() const {
  return mean_;
}

double TimeSeries::stdev() const {
  return stdev_;
}


double TimeSeries::kurt() const { // O(n)
  int n = rowname.size();
  double ret = 0.0;
  for (int i = 0; i < n; i++) {
    ret += pow((d[i] - mean_) / stdev_, 4);
  }
  return double(n) * (n + 1) / (n - 1) / (n - 2) / (n - 3) * ret - 3.0 * (n - 1) * (n - 1) / (n - 2) / (n - 3);
}

double TimeSeries::skew() const { // O(n)
  int n = rowname.size();
  double ret = 0.0;
  for (int i = 0; i < n; i++) {
    ret += pow((d[i] - mean_) / stdev_, 3);
  }
  return double(n) / (n - 1) / (n - 2) * ret;
}

TimeSeries TimeSeries::head(const int len=6) const {
  vector<string> new_row(len);
  vector<double> new_d(len);
  for (int i = 0; i < len; i++) {
    new_row[i] = rowname[i];
    new_d[i] = d[i];
  }
  return *(new TimeSeries(name + "_head(" + to_string(len) + ")", new_row, new_d));
}

vector<double> TimeSeries::to_vector() const {
  return d;
}