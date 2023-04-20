﻿#include "servant/PropertyReport.h"
#include "util/kt_common.h"

namespace kant {

string PropertyReport::sum::get() {
  string s = KT_Common::tostr(_d);

  clear();

  return s;
}

string PropertyReport::avg::get() {
  if (_count == 0) {
    return "0";
  }
  string s = KT_Common::tostr(static_cast<double>(_sum) / _count);

  clear();

  return s;
}

PropertyReport::distr::distr(const vector<int>& range) {
  _range = range;

  std::sort(_range.begin(), _range.end());

  _range.erase(unique(_range.begin(), _range.end()), _range.end());

  _result.resize(_range.size());
}

void PropertyReport::distr::set(int o) {
  vector<int>::iterator it = std::upper_bound(_range.begin(), _range.end(), o);

  if (it != _range.end()) {
    size_t n = it - _range.begin();

    ++_result[n];
  }
}

string PropertyReport::distr::get() {
  string s = "";

  for (unsigned i = 0; i < _range.size(); ++i) {
    if (i != 0) {
      s += ",";
    }
    s = s + KT_Common::tostr(_range[i]) + "|" + KT_Common::tostr(_result[i]);
  }
  for (unsigned i = 0; i < _result.size(); ++i) {
    _result[i] = 0;
  }
  return s;
}

string PropertyReport::max::get() {
  string s = KT_Common::tostr(_d);

  clear();

  return s;
}

string PropertyReport::min::get() {
  string s = KT_Common::tostr(_d);

  clear();

  return s;
}

void PropertyReport::min::set(int o) {
  //非0最小值
  if (_d == 0 || (_d > o && o != 0)) {
    _d = o;
  }
}

string PropertyReport::count::get() {
  string s = KT_Common::tostr(_d);

  clear();

  return s;
}
///////////////////////////////////////////////
}  // namespace kant
