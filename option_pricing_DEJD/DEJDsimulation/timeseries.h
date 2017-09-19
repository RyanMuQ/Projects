#ifndef _TIMESERIES_
#define _TIMESERIES_

#include <vector>
#include <string>

using namespace std;

class TimeSeries {

private:

  string name;
  vector<string> rowname;
  vector<double> d;
  double stdev_, mean_;
  double get_stdev() const;
  double get_mean() const;
  friend ostream& operator << (ostream& output, const TimeSeries& ts);  // output

public:

  int days;

  TimeSeries();
  TimeSeries(string name_);
  TimeSeries(string name_, const vector<string>& rowname_, const vector<double> d_);

  double operator [](size_t ind) const;

  void read_csv(string addr);
  void write_csv(string addr, char sep = ',') const;
  size_t len() const;
  double stdev() const;
  double mean() const;
  double kurt() const; // O(n)
  double skew() const; // O(n)
  TimeSeries head(const int len) const;
  vector<double> to_vector() const;
};

#endif
