﻿#ifndef _KANT_DISPLAYER_H_
#define _KANT_DISPLAYER_H_

//#include <netinet/in.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <limits.h>

//支持iphone
#ifdef __APPLE__
#include "KantType.h"
#else
#include "tup/KantType.h"
#endif

namespace kant {
//////////////////////////////////////////////////////////////////////
/// 用于输出
class KantDisplayer {
  std::ostream& _os;
  int _level;

  void ps(const char* fieldName) {
    for (int i = 0; i < _level; ++i) _os << '\t';
    if (fieldName != NULL) _os << fieldName << ": ";
  }

 public:
  explicit KantDisplayer(std::ostream& os, int level = 0) : _os(os), _level(level) {}

  KantDisplayer& display(Bool b, const char* fieldName) {
    ps(fieldName);
    _os << (b ? 'T' : 'F') << std::endl;
    return *this;
  }

  KantDisplayer& display(Char n, const char* fieldName) {
    ps(fieldName);
    _os << n << std::endl;
    return *this;
  }

  KantDisplayer& display(UInt8 n, const char* fieldName) {
    ps(fieldName);
    _os << n << std::endl;
    return *this;
  }

  KantDisplayer& display(Short n, const char* fieldName) {
    ps(fieldName);
    _os << n << std::endl;
    return *this;
  }

  KantDisplayer& display(UInt16 n, const char* fieldName) {
    ps(fieldName);
    _os << n << std::endl;
    return *this;
  }

  KantDisplayer& display(Int32 n, const char* fieldName) {
    ps(fieldName);
    _os << n << std::endl;
    return *this;
  }

  KantDisplayer& display(UInt32 n, const char* fieldName) {
    ps(fieldName);
    _os << n << std::endl;
    return *this;
  }

  KantDisplayer& display(Int64 n, const char* fieldName) {
    ps(fieldName);
    _os << n << std::endl;
    return *this;
  }

  KantDisplayer& display(Float n, const char* fieldName) {
    ps(fieldName);
    _os << n << std::endl;
    return *this;
  }

  KantDisplayer& display(Double n, const char* fieldName) {
    ps(fieldName);
    _os << n << std::endl;
    return *this;
  }

  KantDisplayer& display(const std::string& s, const char* fieldName) {
    ps(fieldName);
    _os << s << std::endl;
    return *this;
  }

  KantDisplayer& display(const char* s, const size_t len, const char* fieldName) {
    ps(fieldName);
    for (unsigned i = 0; i < len; i++) {
      _os << s[i];
    }
    _os << std::endl;
    return *this;
  }

  template <typename K, typename V, typename Cmp, typename Alloc>
  KantDisplayer& display(const std::map<K, V, Cmp, Alloc>& m, const char* fieldName) {
    ps(fieldName);
    if (m.empty()) {
      _os << m.size() << ", {}" << std::endl;
      return *this;
    }
    _os << m.size() << ", {" << std::endl;
    KantDisplayer jd1(_os, _level + 1);
    KantDisplayer jd(_os, _level + 2);
    typedef typename std::map<K, V, Cmp, Alloc>::const_iterator IT;
    IT f = m.begin(), l = m.end();
    for (; f != l; ++f) {
      jd1.display('(', NULL);
      jd.display(f->first, NULL);
      jd.display(f->second, NULL);
      jd1.display(')', NULL);
    }
    display('}', NULL);
    return *this;
  }

  template <typename T, typename Alloc>
  KantDisplayer& display(const std::vector<T, Alloc>& v, const char* fieldName) {
    ps(fieldName);
    if (v.empty()) {
      _os << v.size() << ", []" << std::endl;
      return *this;
    }
    _os << v.size() << ", [" << std::endl;
    KantDisplayer jd(_os, _level + 1);
    typedef typename std::vector<T, Alloc>::const_iterator IT;
    IT f = v.begin(), l = v.end();
    for (; f != l; ++f) jd.display(*f, NULL);
    display(']', NULL);
    return *this;
  }

  template <typename T>
  KantDisplayer& display(const T* v, const size_t len, const char* fieldName) {
    ps(fieldName);
    if (len == 0) {
      _os << len << ", []" << std::endl;
      return *this;
    }
    _os << len << ", [" << std::endl;
    KantDisplayer jd(_os, _level + 1);
    for (size_t i = 0; i < len; ++i) jd.display(v[i], NULL);
    display(']', NULL);
    return *this;
  }

  template <typename T>
  KantDisplayer& display(
    const T& v, const char* fieldName,
    typename detail::disable_if<detail::is_convertible<T*, KantStructBase*>, void***>::type dummy = 0) {
    return display((Int32)v, fieldName);
  }

  template <typename T>
  KantDisplayer& display(
    const T& v, const char* fieldName,
    typename detail::enable_if<detail::is_convertible<T*, KantStructBase*>, void***>::type dummy = 0) {
    display('{', fieldName);
    v.display(_os, _level + 1);
    display('}', NULL);
    return *this;
  }

  KantDisplayer& displaySimple(Bool b, bool bSep) {
    _os << (b ? 'T' : 'F') << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(Char n, bool bSep) {
    _os << n << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(UInt8 n, bool bSep) {
    _os << n << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(Short n, bool bSep) {
    _os << n << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(UInt16 n, bool bSep) {
    _os << n << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(Int32 n, bool bSep) {
    _os << n << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(UInt32 n, bool bSep) {
    _os << n << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(Int64 n, bool bSep) {
    _os << n << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(Float n, bool bSep) {
    _os << n << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(Double n, bool bSep) {
    _os << n << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(const std::string& n, bool bSep) {
    _os << n << (bSep ? "|" : "");
    return *this;
  }

  KantDisplayer& displaySimple(const char* n, const size_t len, bool bSep) {
    for (unsigned i = 0; i < len; i++) {
      _os << n[i];
    }
    _os << (bSep ? "|" : "");
    return *this;
  }

  template <typename K, typename V, typename Cmp, typename Alloc>
  KantDisplayer& displaySimple(const std::map<K, V, Cmp, Alloc>& m, bool bSep) {
    if (m.empty()) {
      _os << m.size() << "{}";
      return *this;
    }
    _os << m.size() << "{";
    KantDisplayer jd1(_os, _level + 1);
    KantDisplayer jd(_os, _level + 2);
    typedef typename std::map<K, V, Cmp, Alloc>::const_iterator IT;
    IT f = m.begin(), l = m.end();
    for (; f != l; ++f) {
      if (f != m.begin()) _os << ',';
      jd.displaySimple(f->first, true);
      jd.displaySimple(f->second, false);
    }
    _os << '}' << (bSep ? "|" : "");
    return *this;
  }

  template <typename T, typename Alloc>
  KantDisplayer& displaySimple(const std::vector<T, Alloc>& v, bool bSep) {
    if (v.empty()) {
      _os << v.size() << "{}";
      return *this;
    }
    _os << v.size() << '{';
    KantDisplayer jd(_os, _level + 1);
    typedef typename std::vector<T, Alloc>::const_iterator IT;
    IT f = v.begin(), l = v.end();
    for (; f != l; ++f) {
      if (f != v.begin()) _os << "|";
      jd.displaySimple(*f, false);
    }
    _os << '}' << (bSep ? "|" : "");
    return *this;
  }

  template <typename T>
  KantDisplayer& displaySimple(const T* v, const size_t len, bool bSep) {
    if (len == 0) {
      _os << len << "{}";
      return *this;
    }
    _os << len << '{';
    KantDisplayer jd(_os, _level + 1);
    for (size_t i = 0; i < len; ++i) {
      if (i != 0) _os << "|";
      jd.displaySimple(v[i], false);
    }
    _os << '}' << (bSep ? "|" : "");
    return *this;
  }

  template <typename T>
  KantDisplayer& displaySimple(
    const T& v, bool bSep,
    typename detail::disable_if<detail::is_convertible<T*, KantStructBase*>, void***>::type dummy = 0) {
    return displaySimple((Int32)v, bSep);
  }

  template <typename T>
  KantDisplayer& displaySimple(
    const T& v, bool bSep,
    typename detail::enable_if<detail::is_convertible<T*, KantStructBase*>, void***>::type dummy = 0) {
    _os << "{";
    v.displaySimple(_os, _level + 1);
    _os << "}" << (bSep ? "|" : "");
    return *this;
  }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace kant
#endif
