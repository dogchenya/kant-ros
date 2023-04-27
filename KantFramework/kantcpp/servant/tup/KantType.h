#ifndef _KANT_TYPE_H_
#define _KANT_TYPE_H_

#ifdef __linux__
#include <netinet/in.h>
#elif _WIN32
#include <WinSock2.h>
#endif

// #include <netinet/in.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "util/kt_platform.h"

namespace kant {
/////////////////////////////////////////////////////////////////////////////////////
typedef bool Bool;
typedef char Char;
typedef short Short;
typedef float Float;
typedef double Double;
typedef int Int32;
struct KantStructBase;

typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned int UInt32;

#if __WORDSIZE == 64
typedef long Int64;
#else
typedef long long Int64;
#endif

#ifndef KANT_MAX_STRING_LENGTH
#define KANT_MAX_STRING_LENGTH (100 * 1024 * 1024)
#endif
/*
#define kant__bswap_constant_32(x) \
    ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |           \
    (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define kant__bswap_constant_64(x) \
    ((((x) & 0xff00000000000000ull) >> 56)                   \
    | (((x) & 0x00ff000000000000ull) >> 40)                     \
    | (((x) & 0x0000ff0000000000ull) >> 24)                     \
    | (((x) & 0x000000ff00000000ull) >> 8)                      \
    | (((x) & 0x00000000ff000000ull) << 8)                      \
    | (((x) & 0x0000000000ff0000ull) << 24)                     \
    | (((x) & 0x000000000000ff00ull) << 40)                     \
    | (((x) & 0x00000000000000ffull) << 56))
*/
#if (defined(__APPLE__) || defined(_WIN32))
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif
#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 4321
#endif
#ifndef __BYTE_ORDER
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define kant_ntohll(x) (x)
#define kant_htonll(x) (x)
#define kant_ntohf(x) (x)
#define kant_htonf(x) (x)
#define kant_ntohd(x) (x)
#define kant_htond(x) (x)
#else
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define kant_ntohll(x) kant_htonll(x)
//#     define kant_htonll(x)    kant__bswap_constant_64(x)
namespace detail {
union bswap_helper {
  Int64 i64;
  Int32 i32[2];
};
}  // namespace detail
inline Int64 kant_htonll(Int64 x) {
  detail::bswap_helper h;
  h.i64 = x;
  Int32 tmp = htonl(h.i32[1]);
  h.i32[1] = htonl(h.i32[0]);
  h.i32[0] = tmp;
  return h.i64;
}
inline Float kant_ntohf(Float x) {
  union {
    Float f;
    Int32 i32;
  } helper;

  helper.f = x;
  helper.i32 = htonl(helper.i32);

  return helper.f;
}
#define kant_htonf(x) kant_ntohf(x)
inline Double kant_ntohd(Double x) {
  union {
    Double d;
    Int64 i64;
  } helper;

  helper.d = x;
  helper.i64 = kant_htonll(helper.i64);

  return helper.d;
}
#define kant_htond(x) kant_ntohd(x)
#endif
#endif

//type2name
template <typename T>
struct KantClass {
  static std::string name() { return T::className(); }
};
template <>
struct KantClass<kant::Bool> {
  static std::string name() { return "bool"; }
};
template <>
struct KantClass<kant::Char> {
  static std::string name() { return "char"; }
};
template <>
struct KantClass<kant::Short> {
  static std::string name() { return "short"; }
};
template <>
struct KantClass<kant::Float> {
  static std::string name() { return "float"; }
};
template <>
struct KantClass<kant::Double> {
  static std::string name() { return "double"; }
};
template <>
struct KantClass<kant::Int32> {
  static std::string name() { return "int32"; }
};
template <>
struct KantClass<kant::Int64> {
  static std::string name() { return "int64"; }
};
template <>
struct KantClass<kant::UInt8> {
  static std::string name() { return "short"; }
};
template <>
struct KantClass<kant::UInt16> {
  static std::string name() { return "int32"; }
};
template <>
struct KantClass<kant::UInt32> {
  static std::string name() { return "int64"; }
};
template <>
struct KantClass<std::string> {
  static std::string name() { return "string"; }
};
template <typename T>
struct KantClass<std::vector<T>> {
  static std::string name() { return std::string("list<") + KantClass<T>::name() + ">"; }
};
template <typename T, typename U>
struct KantClass<std::map<T, U>> {
  static std::string name() { return std::string("map<") + KantClass<T>::name() + "," + KantClass<U>::name() + ">"; }
};

namespace detail {
// is_convertible
template <int N>
struct type_of_size {
  char elements[N];
};

typedef type_of_size<1> yes_type;
typedef type_of_size<2> no_type;

namespace meta_detail {
struct any_conversion {
  template <typename T>
  any_conversion(const volatile T&);
  template <typename T>
  any_conversion(T&);
};

template <typename T>
struct conversion_checker {
  static no_type _m_check(any_conversion...);
  static yes_type _m_check(T, int);
};
}  // namespace meta_detail

template <typename From, typename To>
class is_convertible {
  static From _m_from;

 public:
  enum { value = sizeof(meta_detail::conversion_checker<To>::_m_check(_m_from, 0)) == sizeof(yes_type) };
};

template <typename T>
struct type2type {
  typedef T type;
};

template <typename T, typename U>
struct is_same_type {
  enum { value = is_convertible<type2type<T>, type2type<U>>::value };
};

// enable_if
template <bool B, class T = void>
struct enable_if_c {
  typedef T type;
};

template <class T>
struct enable_if_c<false, T> {};

template <class Cond, class T = void>
struct enable_if : public enable_if_c<Cond::value, T> {};

template <bool B, class T = void>
struct disable_if_c {
  typedef T type;
};

template <class T>
struct disable_if_c<true, T> {};

template <class Cond, class T = void>
struct disable_if : public disable_if_c<Cond::value, T> {};
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace kant
#endif
