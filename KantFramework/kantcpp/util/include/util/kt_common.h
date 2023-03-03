#ifndef _KT_COMMON_H_
#define _KT_COMMON_H_

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "util/kt_platform.h"

using namespace std;

#if TARGET_PLATFORM_WINDOWS

#ifndef ssize_t
#define ssize_t __int64
#endif

#endif

namespace kant {
/////////////////////////////////////////////////
/**
 * @file kt_common.h
 * @brief  ������,���Ǿ�̬����,�ṩһЩ���õĺ��� .
 * @brief  Helper Class. There're all static functions in this which provides
 * some commonly used functions
 *
 */
/////////////////////////////////////////////////

/**
 * @brief  ���������࣬�ṩ��һЩ�ǳ������ĺ���ʹ��.
 * @brief  Basic Utility Class. Some basic functions are provided.
 *
 * ��Щ���������Ծ�̬�����ṩ�� �������¼��ֺ���:
 * These functions are provided as static functions.It includes the following
 * functions:
 *
 * Trim�ຯ��,��Сдת������,�ָ��ַ���������ֱ�ӷָ��ַ�����
 * Trim class functions, case conversion functions, delimited string functions
 * (directly delimited strings, numbers, etc.),
 *
 * ���ֵȣ�,ʱ����غ���,�ַ���ת������,�������ַ�����ת����,
 * time-dependent functions, string conversion functions, binary string
 * conversion functions,
 *
 * �滻�ַ�������,Ipƥ�亯��,�ж�һ�����Ƿ���������
 * replacement string functions, IP matching functions, determining whether a
 * number is a prime number, etc.
 */
class UTIL_DLL_API KT_Common {
 public:
  static const float _EPSILON_FLOAT;
  static const double _EPSILON_DOUBLE;
  static const int64_t ONE_DAY_MS = 24 * 3600 * 1000L;
  static const int64_t ONE_HOUR_MS = 1 * 3600 * 1000L;
  static const int64_t ONE_MIN_MS = 60 * 1000L;
  static const int64_t ONE_DAY_SEC = 86400;

  /**
   * @brief  ��ƽ̨sleep
   * @brief  Cross Platform Sleep
   */
  static void sleep(uint32_t sec);
  static void msleep(uint32_t ms);

  /**
   * @brief  �������Ƚ�,double Ĭ��ȡ6λ���ȣ�floatĬ��6λ����
   * @brief  Floating Number Comparison, double defaults to be 6-bit precision,
   * and float defaults to be 6-bit precision as well.
   */
  static bool equal(double x, double y, double epsilon = _EPSILON_DOUBLE);
  static bool equal(double x, double y, float epsilon);

  static bool equal(float x, float y, float epsilon = _EPSILON_FLOAT);
  static bool equal(float x, float y, double epsilon);

  /**
   * @brief  vector double ���ֳ����ȽϺ���
   * @brief  vector double, comparison functions for various scenarios
   */
  static bool equal(const vector<double> &vx, const vector<double> &vy, double epsilon = _EPSILON_DOUBLE);
  static bool equal(const vector<double> &vx, const vector<double> &vy, float epsilon);
  static bool equal(const vector<float> &vx, const vector<float> &vy, float epsilon = _EPSILON_FLOAT);
  static bool equal(const vector<float> &vx, const vector<float> &vy, double epsilon);

  static bool equal(const set<double> &vx, const set<double> &vy, double epsilon = _EPSILON_DOUBLE);
  static bool equal(const set<double> &vx, const set<double> &vy, float epsilon);
  static bool equal(const set<float> &vx, const set<float> &vy, float epsilon = _EPSILON_FLOAT);
  static bool equal(const set<float> &vx, const set<float> &vy, double epsilon);

  static bool equal(const unordered_set<double> &vx, const unordered_set<double> &vy, double epsilon = _EPSILON_DOUBLE);
  static bool equal(const unordered_set<double> &vx, const unordered_set<double> &vy, float epsilon);
  static bool equal(const unordered_set<float> &vx, const unordered_set<float> &vy, float epsilon = _EPSILON_FLOAT);
  static bool equal(const unordered_set<float> &vx, const unordered_set<float> &vy, double epsilon);

  /**
   * @brief  map�����key����valueΪdouble/float�ֶΣ����ô�ģ�庯���Ƚ�
   * @brief  In map, if the key or value is the double/float field, use this
   * template function to compare.
   */
  template <typename V, typename E>
  static bool equal(const V &x, const V &y, E eps);
  template <typename K, typename V, typename D, typename A, typename E = double>
  static bool equal(const map<K, V, D, A> &mx, const map<K, V, D, A> &my, E epsilon = _EPSILON_DOUBLE);
  template <typename K, typename V, typename D, typename A, typename E = double>
  static bool equal(const unordered_map<K, V, D, A> &mx, const unordered_map<K, V, D, A> &my,
                    E epsilon = _EPSILON_DOUBLE);

  /**
   * �̶��������ַ���, ������������ʽ��(Ĭ�������)
   * Fixed width filled string for output alignment format (default
   * right-padding)
   * @param s
   * @param c
   * @param n
   * @return
   */
  static string outfill(const string &s, char pad = ' ', size_t n = 50, bool rightPad = true) {
    if (n <= s.length()) return s;

    if (rightPad) return (s + string((n - s.length()), pad));

    return (string((n - s.length()), pad) + s);
  }

  /**
   * @brief  ȥ��ͷ���Լ�β�����ַ����ַ���.
   * @brief  Remove the head and the tail characters or strings
   *
   * @param sStr    �����ַ���
   * @param sStr    input string
   * @param s       ��Ҫȥ�����ַ�
   * @param s       the characters which need to be removed
   * @param bChar   ���Ϊtrue, ��ȥ��s��ÿ���ַ�; ���Ϊfalse, ��ȥ��s�ַ���
   * @param bChar   bool : true, Remove each character in 's'; false, remove the
   * s String
   * @return string ����ȥ������ַ���
   * @return string Return the removed string
   */
  static string trim(const string &sStr, const string &s = " \r\n\t", bool bChar = true);

  /**
   * @brief  ȥ����ߵ��ַ����ַ���.
   * @brief  Remove left character or string
   *
   * @param sStr    �����ַ���
   * @param sStr    input string
   * @param s       ��Ҫȥ�����ַ�
   * @param s       the characters which need to be removed
   * @param bChar   ���Ϊtrue, ��ȥ��s��ÿ���ַ�; ���Ϊfalse, ��ȥ��s�ַ���
   * @param bChar   bool : true, Remove each character in 's'; false, remove the
   * s String
   * @return string ����ȥ������ַ���
   * @return string Return the removed string
   */
  static string trimleft(const string &sStr, const string &s = " \r\n\t", bool bChar = true);

  /**
   * @brief  ȥ���ұߵ��ַ����ַ���.
   * @brief  Remove right character or string
   *
   * @param sStr    �����ַ���
   * @param sStr    input string
   * @param s       ��Ҫȥ�����ַ�
   * @param s       the characters which need to be removed
   * @param bChar   ���Ϊtrue, ��ȥ��s��ÿ���ַ�; ���Ϊfalse, ��ȥ��s�ַ���
   * @param bChar   bool : true, Remove each character in 's'; false, remove the
   * s String
   * @return string ����ȥ������ַ���
   * @return string Return the removed string
   */
  static string trimright(const string &sStr, const string &s = " \r\n\t", bool bChar = true);

  /**
   * @brief  �ַ���ת����Сд.
   * @brief  Convert string to lowercase.
   *
   * @param sString  �ַ���
   * @param sString  String
   * @return string  ת������ַ���
   * @return string  the converted string
   */
  static string lower(const string &sString);

  /**
   * @brief  �ַ���ת���ɴ�д.
   * @brief  Convert string to uppercase.
   *
   * @param sString  �ַ���
   * @param sString  string
   * @return string  ת����Ĵ�д���ַ���
   * @return string  the converted string
   */
  static string upper(const string &sString);

  /**
   * @brief  �ַ����Ƿ������ֵ�.
   * @brief  Whether strings are all numbers or not.
   *
   * @param sString  �ַ���
   * @param sString  string
   * @return bool    �Ƿ�������
   * @return bool    whether number or not
   */
  static bool isdigit(const string &sInput);

  /**
   * @brief  �ַ���ת����ʱ��ṹ.
   * @brief  Convert string to time structure.
   *
   * @param sString  �ַ���ʱ���ʽ
   * @param sString  string Time Format
   * @param sFormat  ��ʽ
   * @param sFormat  format
   * @param stTm     ʱ��ṹ
   * @param stTm     time structure
   * @return         0: �ɹ�, -1:ʧ��
   * @return         0: success, -1: fail
   */
  static int str2tm(const string &sString, const string &sFormat, struct tm &stTm);

  /**
   * @brief GMT��ʽ��ʱ��ת��Ϊʱ��ṹ
   * @brief Conversion of time into time structure in GMT format
   *
   * eg.Sat, 06 Feb 2010 09:29:29 GMT, %a, %d %b %Y %H:%M:%S GMT
   *
   * ������mktime����time_t, ����ע��ʱ�� ������mktime(&stTm)
   * You can replace time_t with mktime. Be careful about the time zones, and it
   * can be used with mktime (&stTm)
   *
   * - timezone���ɱ��ص���(time(NULL)ֵ��ͬ) timezone��ϵͳ�� ,
   * - timezone changes costs to local seconds (time (NULL) values are the
   * same). Timezones is systematic.
   *
   * ��Ҫextern long timezone;
   * need extern long timezone;
   *
   * @param sString  GMT��ʽ��ʱ�䣬����ʱ��
   * @param sString  time in GMT format
   * @param stTm     ת�����ʱ��ṹ
   * @param stTm     converted Time Structure
   * @return         0: �ɹ�, -1:ʧ��
   * @return         0: success, -1: fail
   */
  static int strgmt2tm(const string &sString, struct tm &stTm);

  /**
   * @brief  ��ʽ�����ַ���ʱ��תΪʱ���.
   * @brief  Format time string to timestamp
   *
   * @param sString  ��ʽ�����ַ���ʱ�䣬����ʱ��
   * @param sString  format time string
   * @param sFormat  ��ʽ�����ַ���ʱ��ĸ�ʽ��Ĭ��Ϊ���ո�ʽ
   * @param sFormat  format of formatted string time
   * @return time_t  ת�����ʱ���
   * @return time_t  the converted timestamp
   */
  static time_t str2time(const string &sString, const string &sFormat = "%Y%m%d%H%M%S");

  /**
   * @brief  ʱ��ת�����ַ���.
   * @brief  Convert time into string.
   *
   * @param stTm     ʱ��ṹ
   * @param stTm     time structure
   * @param sFormat  ��Ҫת����Ŀ���ʽ��Ĭ��Ϊ���ո�ʽ
   * @param sFormat  Target format to be converted, default to compact format
   * @return string  ת�����ʱ���ַ���
   * @return string  converted time string
   */
  static string tm2str(const struct tm &stTm, const string &sFormat = "%Y%m%d%H%M%S");

  /**
   * @brief  ʱ��ת�����ַ���.
   * @brief  Convert time into string
   *
   * @param t        ʱ��ṹ
   * @param t        time structure
   * @param sFormat  ��Ҫת����Ŀ���ʽ��Ĭ��Ϊ���ո�ʽ
   * @param sFormat  Target format to be converted, default to compact format
   * @return string  ת�����ʱ���ַ���
   * @return string  converted time string
   */
  static string tm2str(const time_t &t, const string &sFormat = "%Y%m%d%H%M%S");

  /**
   * @brief  ʱ��ת��tm.
   * @brief  Convert time into tm.
   *
   * @param t        ʱ��ṹ��UTCʱ���
   * @param t        time structure
   */
  static void tm2time(const time_t &t, struct tm &tt);

  /**
   * @brief  time_tת����tm(����ϵͳ��localtime_r, �������!!!)
   * @brief  Convert time_t to tm (Don't use system's localtime_r. The function
   * will be slowed down.)
   *
   * @param t        ʱ��ṹ��UTCʱ���
   * @param t        time structure
   * @param sFormat  ��Ҫת����Ŀ���ʽ��Ĭ��Ϊ���ո�ʽ
   * @param sFormat  Target format to be converted, default to compact format
   * @return string  ת�����ʱ���ַ���
   * @return string  converted time string
   */
  static void tm2tm(const time_t &t, struct tm &stTm);

  /**
   * @brief  ��ȡ��ǰ����ͺ���
   * @brief  Get the current seconds and milliseconds
   *
   * @param t        ʱ��ṹ
   * @param t        time structure
   */
  static int gettimeofday(struct timeval &tv);

  /**
   * @brief  ��ǰʱ��ת���ɽ��ո�ʽ�ַ���
   * @brief  Convert current time to compact string
   * @param sFormat ��ʽ��Ĭ��Ϊ���ո�ʽ
   * @param sFormat the format, default to compact format
   * @return string ת�����ʱ���ַ���
   * @return string converted time string
   */
  static string now2str(const string &sFormat = "%Y%m%d%H%M%S");

  /**
   * @brief  ����ʱ��, һ�����ڵ������
   * @return string ת�����ʱ���ַ���
   */
  static string now2msstr();

  /**
   * @brief ����ʱ��ת�ַ�������
   * @return string ת�����ʱ���ַ���
   */
  static string ms2str(int64_t ms);
  /**
   * @brief  ʱ��ת����GMT�ַ�����GMT��ʽ:Fri, 12 Jan 2001 18:18:18 GMT
   * @brief  Convert time into GMT string, GMT��ʽ:Fri, 12 Jan 2001 18:18:18 GMT
   * @param stTm    ʱ��ṹ
   * @param stTm    time structure
   * @return string GMT��ʽ��ʱ���ַ���
   * @return string time string in GMT format
   */
  static string tm2GMTstr(const struct tm &stTm);

  /**
   * @brief  ʱ��ת����GMT�ַ�����GMT��ʽ:Fri, 12 Jan 2001 18:18:18 GMT
   * @brief  Convert time into GMT string, GMT��ʽ:Fri, 12 Jan 2001 18:18:18 GMT
   * @param stTm    ʱ��ṹ
   * @param stTm    time structure
   * @return string GMT��ʽ��ʱ���ַ���
   * @return string time string in GMT format
   */
  static string tm2GMTstr(const time_t &t);

  /**
   * @brief  ��ǰʱ��ת����GMT�ַ�����GMT��ʽ:Fri, 12 Jan 2001 18:18:18 GMT
   * @brief  Convert current time into GMT string, GMT��ʽ:Fri, 12 Jan 2001
   * 18:18:18 GMT
   * @return string GMT��ʽ��ʱ���ַ���
   * @return string time string in GMT format
   */
  static string now2GMTstr();

  /**
   * @brief  ��ǰ������(������)ת�����ַ���(%Y%m%d).
   * @brief  Get current date(yearmonthday) and convert it into string (%Y%m%d).
   *
   * @return string (%Y%m%d)��ʽ��ʱ���ַ���
   * @return string time string in (%Y%m%d) format
   */
  static string nowdate2str();

  /**
   * @brief  ��ǰ��ʱ��(ʱ����)ת�����ַ���(%H%M%S).
   * @brief  Get current time(hourminutesecond) and convert it into string
   * (%H%M%S).
   *
   * @return string (%H%M%S)��ʽ��ʱ���ַ���
   * @return string time string in (%H%M%S) format
   */
  static string nowtime2str();

  /**
   * @brief  ��ȡ��ǰʱ��ĵĺ�����.
   * @brief  Get the value of milliseconds of current time.
   *
   * @return int64_t ��ǰʱ��ĵĺ�����
   * @return int64_t current milliseconds of this time
   */
  static int64_t now2ms();

  /**
   * @brief  ȡ����ǰʱ���΢��.
   * @brief  Take out microseconds of current time.
   *
   * @return int64_t ȡ����ǰʱ���΢��
   * @return int64_t Take out microseconds of current time.
   */
  static int64_t now2us();

  /**
   * @brief  �ַ���ת����T�ͣ����T����ֵ����, ���strΪ��,��TΪ0.
   * @brief  Convert string to type T. if T is a numeric type and STR is empty,
   * then T values 0.
   *
   * @param sStr  Ҫת�����ַ���
   * @param sStr  the string needs to be converted
   * @return T    T������
   * @return T    the type of type T
   */
  template <typename T>
  static T strto(const string &sStr);

  /**
   * @brief  �ַ���ת����T��.
   * @brief  Convert string to type T
   *
   * @param sStr      Ҫת�����ַ���
   * @param sStr      the string needs to be converted
   * @param sDefault  ȱʡֵ
   * @param sDefault  default value
   * @return T        ת�����T����
   * @return T        the converted type of type T
   */
  template <typename T>
  static T strto(const string &sStr, const string &sDefault);

  /**
   * @brief  �����ַ���,�÷ָ����ŷָ�,������vector��
   * @brief  Parse string, separate with separator, and save in vector
   *
   * ����: |a|b||c|
   * Example: |a|b||c|
   *
   * ���withEmpty=trueʱ, ����|�ָ�Ϊ:"","a", "b", "", "c", ""
   * If 'withEmpty=true' then use '|' to separate it into "","a", "b", "", "c",
   * "".
   *
   * ���withEmpty=falseʱ, ����|�ָ�Ϊ:"a", "b", "c"
   * If 'withEmpty=false' then use '|' to separate it into "a", "b", "c".
   *
   * ���T����Ϊint����ֵ����, ��ָ����ַ���Ϊ"", ��ǿ��ת��Ϊ0
   * If the T type is a numeric type such as int, the delimited string is' ',
   * then it is forced to 0.
   *
   * @param sStr      �����ַ���
   * @param sStr      input string
   * @param sSep      �ָ��ַ���(ÿ���ַ�����Ϊ�ָ���)
   * @param sSep      the separator string (each character counts as a
   * separator)
   * @param withEmpty true����յ�Ҳ��һ��Ԫ��, falseʱ�յĹ���
   * @param withEmpty bool: true, represented that empty is also an element ;
   * false, filter empty ones.
   * @return          ��������ַ�vector
   * @return          parsed character: vector
   */
  template <typename T>
  static vector<T> sepstr(const string &sStr, const string &sSep, bool withEmpty = false);

  /**
   * @brief T��ת�����ַ�����ֻҪT�ܹ�ʹ��ostream������<<����,�����Ա��ú���֧��
   * @brief Convert T-type to string. As long as T can use ostream object with
   * << to overload, it can be supported by this function.
   *
   * @param t Ҫת��������
   * @param t the data needs to be converted
   * @return  ת������ַ���
   * @return  the converted string
   */
  template <typename T>
  inline static string tostr(const T &t) {
    ostringstream sBuffer;
    sBuffer << t;
    return sBuffer.str();
  }

  /**
   * @brief  vectorת����string.
   * @brief  Convert vector to string.
   *
   * @param t Ҫת����vector�͵�����
   * @param t data which need to be convertes to vector type
   * @return  ת������ַ���
   * @return  the converted string
   */
  template <typename T>
  static string tostr(const vector<T> &t);

  /**
   * @brief  ��map���Ϊ�ַ���.
   * @brief  export map as string
   *
   * @param map<K, V, D, A>  Ҫת����map����
   * @param map<K, V, D, A>  the map object needs to be converted
   * @return                    string ������ַ���
   * @return                    output string
   */
  template <typename K, typename V, typename D, typename A>
  static string tostr(const map<K, V, D, A> &t);

  /**
   * @brief  map���Ϊ�ַ���.
   * @brief  export map as string
   *
   * @param multimap<K, V, D, A>  map����
   * @param multimap<K, V, D, A>  the map object needs to be converted
   * @return                      ������ַ���
   * @return                      output string
   */
  template <typename K, typename V, typename D, typename A>
  static string tostr(const multimap<K, V, D, A> &t);

  /**
   * @brief  ��map���Ϊ�ַ���.
   * @brief  export map as string
   *
   * @param map<K, V, D, A>  Ҫת����map����
   * @param map<K, V, D, A>  the map object needs to be converted
   * @return                    string ������ַ���
   * @return                    output string
   */
  template <typename K, typename V, typename D, typename P, typename A>
  static string tostr(const unordered_map<K, V, D, P, A> &t);

  /**
   * @brief  pair ת��Ϊ�ַ�������֤map�ȹ�ϵ��������ֱ����tostr�����
   * @brief  Convert pair to string, ensure that the relationship containers
   * such as map can output directly with tostr.
   * @param pair<F, S> pair����
   * @param pair<F, S> object pair
   * @return           ������ַ���
   * @return           output string
   */
  template <typename F, typename S>
  static string tostr(const pair<F, S> &itPair);

  /**
   * @brief  container ת�����ַ���.
   * @brief  Convert container to string
   *
   * @param iFirst  ������
   * @param iFirst  iterator
   * @param iLast   ������
   * @param iLast   iterator
   * @param sSep    ����Ԫ��֮��ķָ���
   * @param sSep    the separator between two elements
   * @return        ת������ַ���
   * @return        the converted string
   */
  template <typename InputIter>
  static string tostr(InputIter iFirst, InputIter iLast, const string &sSep = "|");

  /**
   * @brief  ����������ת�����ַ���.
   * @brief  Convert binary data t0 string
   *
   * @param buf     ������buffer
   * @param buf     binary buffer
   * @param len     buffer����
   * @param len     buffer length
   * @param sSep    �ָ���
   * @param sSep    separator
   * @param lines   ���ٸ��ֽڻ�һ��, Ĭ��0��ʾ������
   * @param lines   The max number of bytes for oneline.By default, 0 means no
   * new line.
   * @return        ת������ַ���
   * @return        the converted string
   */
  static string bin2str(const void *buf, size_t len, const string &sSep = "", size_t lines = 0);

  /**
   * @brief  ����������ת�����ַ���.
   * @brief  Convert binary data t0 string
   *
   * @param sBinData  ����������
   * @param sBinData  binary data
   * @param sSep     �ָ���
   * @param sSep     separator
   * @param lines    ���ٸ��ֽڻ�һ��, Ĭ��0��ʾ������
   * @param lines    The max number of bytes for oneline.By default, 0 means no
   * new line.
   * @return         ת������ַ���
   * @return         the converted string
   */
  static string bin2str(const string &sBinData, const string &sSep = "", size_t lines = 0);

  /**
   * @brief   �ַ���ת���ɶ�����.
   * @brief   Convert string to binary
   *
   * @param psAsciiData �ַ���
   * @param psAsciiData string
   * @param sBinData    ����������
   * @param sBinData    binary data
   * @param iBinSize    ��Ҫת�����ַ�������
   * @param iBinSize    the length of the string which needs to be converted.
   * @return            ת�����ȣ�С�ڵ���0���ʾʧ��
   * @return            Conversion length, less than or equal to 0 means failure
   */
  static int str2bin(const char *psAsciiData, unsigned char *sBinData, int iBinSize);

  /**
   * @brief  �ַ���ת���ɶ�����.
   * @brief  convert string to binary
   *
   * @param sBinData  Ҫת�����ַ���
   * @param sBinData  the string needs to be converted
   * @param sSep      �ָ���
   * @param sSep      separator
   * @param lines     ���ٸ��ֽڻ�һ��, Ĭ��0��ʾ������
   * @param lines     The max number of bytes for oneline.By default, 0 means no
   * new line.
   * @return          ת����Ķ���������
   * @return          the converted binary data
   */
  static string str2bin(const string &sBinData, const string &sSep = "", size_t lines = 0);

  /**
   * @brief  �滻�ַ���.
   * @brief  replace string
   *
   * @param sString  �����ַ���
   * @param sString  input string
   * @param sSrc     ԭ�ַ���
   * @param sSrc     the original string
   * @param sDest    Ŀ���ַ���
   * @param sDest    the target string
   * @return string  �滻����ַ���
   * @return string  the converted string
   */
  static string replace(const string &sString, const string &sSrc, const string &sDest);

  /**
   * @brief  �����滻�ַ���.
   * @brief  Batch replace string.
   *
   * @param sString  �����ַ���
   * @param sString  input string
   * @param mSrcDest  map<ԭ�ַ���,Ŀ���ַ���>
   * @param mSrcDest  map<original,target>
   * @return string  �滻����ַ���
   * @return string  the converted string
   */
  static string replace(const string &sString, const map<string, string> &mSrcDest);

  /**
   * @brief ƥ����.�ָ����ַ�����pat��*�����ͨ���������ǿյ��κ��ַ���
   * sΪ��, ����false ��patΪ��, ����true
   * @brief Match string separated by '.' And '*' in pat represents wildcard
   * which represents any string that is not empty. If s is empty, return false.
   * If pat is empty, return true.
   * @param s    ��ͨ�ַ���
   * @param s    normal string
   * @param pat  ��*��ƥ����ַ���������ƥ��ip��ַ
   * @param pat  string matched with * to match IP address
   * @return     �Ƿ�ƥ��ɹ�
   * @return     whether they matches or not
   */
  static bool matchPeriod(const string &s, const string &pat);

  /**
   * �ж���ͬһ�����ڵ�ʱ���Ƿ����
   * @param lastDate ����:%Y%m%d��ʽ
   * @param date ����:%Y%m%d��ʽ
   * @param period: W(��)/M(��)/Q(��)/S(����)/Y(��)
   * @return
   */
  static bool matchPeriod(int lastDate, int date, const std::string &period);

  /**
   * @brief  ƥ����.�ָ����ַ���.
   * @brief  Match strings separated by '.'
   *
   * @param s   ��ͨ�ַ���
   * @param s   normal string
   * @param pat vector�е�ÿ��Ԫ�ض��Ǵ�*��ƥ����ַ���������ƥ��ip��ַ
   * @param pat each elment in this vector means string matched with * to match
   * IP address
   * @return    �Ƿ�ƥ��ɹ�
   * @return    whether they matches or not
   */
  static bool matchPeriod(const string &s, const vector<string> &pat);

  /**
   * @brief  �ж�һ�����Ƿ�Ϊ����.
   * @brief  Determine whether a number is prime or not
   * @param n  ��Ҫ���жϵ�����
   * @param n  the data needs to be determined
   * @return   true������������false��ʾ������
   * @return   true for prime , false for non prime
   */
  static bool isPrimeNumber(size_t n);

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS

  /**
   * @brief  daemon
   */
  static void daemon();

  /**
   * @brief  ���Թܵ��쳣
   * @brief  Ignore pipe exceptions
   */
  static void ignorePipe();

  /**
   * @brief  ���ɻ���16�����ַ��������
   * @brief  Generating random strings based on hexadecimal characters.
   * @param p            �洢����ַ���
   * @param p            store random string
   * @param len          �ַ�����С
   * @param len          string length
   */
  static void getRandomHexChars(char *p, unsigned int len);

#endif

  /**
   * @brief  ��һ��string����ת��һ���ֽ� .
   * @brief  Convert a string type to a byte .
   *
   * @param sWhat Ҫת�����ַ���
   * @param sWhat the string which needs to be converted
   * @return char    ת������ֽ�
   * @return char    the converted byte
   */
  static char x2c(const string &sWhat);

  /**
   * @brief  ��С�ַ��������ֽ�����֧��K, M, G���� ����: 1K, 3M, 4G, 4.5M, 2.3G
   * @brief  The string can be changed into bytes. It supports two kinds of K, M
   * and G, such as 1K, 3M, 4G, 4.5M and 2.3G
   * @param s            Ҫת�����ַ���
   * @param s            the string which needs to be converted
   * @param iDefaultSize ��ʽ����ʱ, ȱʡ�Ĵ�С
   * @param iDefaultSize the default size in case of format error
   * @return             �ֽ���
   * @return             Bytes
   */
  static size_t toSize(const string &s, size_t iDefaultSize);

  /**
   * @brief  ��ȡ��������.
   * @brief  Get machine name
   * @return string    ��������ʧ���Ƿ��ؿ�
   * @return string    machine name. if failed returns null
   */
  static string getHostName();

  //��һ������, sDate��:%Y%m%d
  static string nextDate(const string &sDate);
  //��һ������, sDate��:%Y%m%d
  static string prevDate(const string &sDate);
  //��һ������, iDate��:%Y%m%d
  static int nextDate(int iDate);
  //��һ������, iDate��:%Y%m%d
  static int prevDate(int iDate);

  //��һ���·� sMonth��:%Y%m
  static string nextMonth(const string &sMonth);
  //��һ���·� sMonth��:%Y%m
  static string prevMonth(const string &sMonth);
  //��һ����� sYear��:%Y
  static string nextYear(const string &sYear);
  //��һ����� sYear��:%Y
  static string prevYear(const string &sYear);

  //�뵽����%Y%m%d
  static int secondsToDateInt(time_t seconds);
  //�뵽����%Y%m%d
  static string secondsToDateString(time_t seconds);

  //�뵽��һ%Y%m%d
  static string secondsToMondayString(time_t seconds);
  //�뵽�� %Y-%m
  static string secondsToMonthString(time_t seconds);

  //����(%Y%m%d)������
  static int64_t dateToMs(const string &sDate);

  static int64_t dateToSecond(const string &sDate);

  static int dateTo(const string &sDate, const string &sPeriod);

  //����(%Y%m%d)���ܼ�
  static int dateToWeekday(const string &sDate);

  //����(%Y%m%d)���ڼ���
  static int dateToWeek(const string &sDate);

  //����(%Y%m%d)��x��
  static int dateToMonth(const string &sDate);

  //����(%Y%m%d)��x����
  static int dateToSeason(const string &sDate);

  //����(%Y%m%d)�����°���
  static int dateToHalfYear(const string &sDate);

  //����(%Y%m%d)��x��
  static int dateToYear(const string &sDate);

  // MS���ַ��� %Y%m%d-%H%M%S-xxx
  static string msToTimeString(int64_t ms);
  //�ַ�����MS %Y%m%d-%H%M%S-xxx��ת��ʧ�ܷ���0
  static int64_t timeStringToMs(const string &timeStr);

  // MS ��ȡ������(%Y%m%d)��ʱ���ַ���(%H%M%S)��MS�ַ���(xxx)
  static bool getSectionFromMs(int64_t ms, string &date, string &time, string &mstick);
  // MS ��ȡ������(%Y%m%d)��ʧ�ܷ���"00000000"
  static string getDateFromMs(int64_t ms);
  // MS ��ȡ��ʱ��(%H%M%S)��ʧ�ܷ���"000000"
  static string getTimeFromMs(int64_t ms);

  //���뻻�ɵ��������
  static int msToNowSeconds(int64_t ms);
  static int nowDaySeconds();
  //���ɵ��쿪ʼ�ĺ���
  static int64_t msToNowMs(int64_t ms);

  static int64_t us();

  //��ǰ��Сʱ������������ 0 ~ (86400-1),hour 0~23 ;min 0~59
  static int64_t timeToDaySec(int64_t hour, int64_t min);

  //��ȡ��һ��֪ͨ�ľ���ʱ�䣬clockSecΪ���컻������
  static int64_t getNextAbsClockMs(int64_t clockSec);

  static int nextDate(int iDate, int offset);
  static int prevDate(int iDate, int offset);
  // ȡ�������ڵ������е����һ��
  static int lastDate(int iDate, const char period = 'D');
  // ��ȡ�����б��ں�periodƥ�������
  // period: ('W', 1) ('M', -2), ('Q', 5) // ȡÿ��/ÿ��/ÿ���ȵڼ���
  // (�������ǵ����ؼ���)
  static int getMatchPeriodDays(const std::vector<int> &days, const std::pair<string, int> &period,
                                std::vector<int> &matchDays);
};

namespace p {
template <typename D>
struct strto1 {
  D operator()(const string &sStr) {
    string s = "0";

    if (!sStr.empty()) {
      s = sStr;
    }

    istringstream sBuffer(s);

    D t;
    sBuffer >> t;

    return t;
  }
};

template <>
struct strto1<char> {
  char operator()(const string &sStr) {
    if (!sStr.empty()) {
      return sStr[0];
    }
    return 0;
  }
};

template <>
struct strto1<unsigned char> {
  unsigned char operator()(const string &sStr) {
    if (!sStr.empty()) {
      return (unsigned char)sStr[0];
    }
    return 0;
  }
};

template <>
struct strto1<short> {
  short operator()(const string &sStr) {
    if (!sStr.empty()) {
      if (sStr.find("0x") == 0) {
        return (short)::strtol(sStr.c_str(), NULL, 16);
      } else {
        return atoi(sStr.c_str());
      }
    }
    return 0;
  }
};

template <>
struct strto1<unsigned short> {
  unsigned short operator()(const string &sStr) {
    if (!sStr.empty()) {
      if (sStr.find("0x") == 0) {
        return (unsigned short)::strtoul(sStr.c_str(), NULL, 16);
      } else {
        return (unsigned short)strtoul(sStr.c_str(), NULL, 10);
      }
    }
    return 0;
  }
};

template <>
struct strto1<int> {
  int operator()(const string &sStr) {
    if (!sStr.empty()) {
      if (sStr.find("0x") == 0) {
        return ::strtol(sStr.c_str(), NULL, 16);
      } else {
        return atoi(sStr.c_str());
      }
    }
    return 0;
  }
};

template <>
struct strto1<unsigned int> {
  unsigned int operator()(const string &sStr) {
    if (!sStr.empty()) {
      if (sStr.find("0x") == 0) {
        return ::strtoul(sStr.c_str(), NULL, 16);
      } else {
        return strtoul(sStr.c_str(), NULL, 10);
      }
    }
    return 0;
  }
};

template <>
struct strto1<long> {
  long operator()(const string &sStr) {
    if (!sStr.empty()) {
      if (sStr.find("0x") == 0) {
        return ::strtol(sStr.c_str(), NULL, 16);
      } else {
        return atol(sStr.c_str());
      }
    }
    return 0;
  }
};

template <>
struct strto1<long long> {
  long long operator()(const string &sStr) {
    if (!sStr.empty()) {
      if (sStr.find("0x") == 0) {
        return ::strtoll(sStr.c_str(), NULL, 16);
      } else {
        return atoll(sStr.c_str());
      }
    }
    return 0;
  }
};

template <>
struct strto1<unsigned long> {
  unsigned long operator()(const string &sStr) {
    if (!sStr.empty()) {
      if (sStr.find("0x") == 0) {
        return ::strtoul(sStr.c_str(), NULL, 16);
      } else {
        return strtoul(sStr.c_str(), NULL, 10);
      }
    }
    return 0;
  }
};

template <>
struct strto1<float> {
  float operator()(const string &sStr) {
    if (!sStr.empty()) {
      return (float)atof(sStr.c_str());
    }
    return 0;
  }
};

template <>
struct strto1<double> {
  double operator()(const string &sStr) {
    if (!sStr.empty()) {
      return atof(sStr.c_str());
    }
    return 0;
  }
};

// for enum
template <typename D>
struct strto2 {
  D operator()(const string &sStr) {
    istringstream sBuffer(sStr);
    int i;
    sBuffer >> i;

    return (D)i;
  }
};

// for class
template <typename D>
struct strto3 {
  D operator()(const string &sStr) {
    istringstream sBuffer(sStr);
    D t;

    sBuffer >> t;

    return t;
  }
};

// for string
template <>
struct strto3<string> {
  const string &operator()(const string &sStr) { return sStr; }
};

}  // namespace p

template <typename T>
T KT_Common::strto(const string &sStr) {
  using strto_enum_type = typename std::conditional<std::is_enum<T>::value, p::strto2<T>, p::strto3<T>>::type;

  using strto_type = typename std::conditional<std::is_arithmetic<T>::value, p::strto1<T>, strto_enum_type>::type;

  return strto_type()(sStr);
}

template <typename T>
T KT_Common::strto(const string &sStr, const string &sDefault) {
  string s;

  if (!sStr.empty()) {
    s = sStr;
  } else {
    s = sDefault;
  }

  return strto<T>(s);
}

template <typename T>
vector<T> KT_Common::sepstr(const string &sStr, const string &sSep, bool withEmpty) {
  vector<T> vt;

  string::size_type pos = 0;
  string::size_type pos1 = 0;

  while (true) {
    string s;
    pos1 = sStr.find_first_of(sSep, pos);
    if (pos1 == string::npos) {
      if (pos + 1 <= sStr.length()) {
        s = sStr.substr(pos);
      }
    } else if (pos1 == pos) {
      s = "";
    } else {
      s = sStr.substr(pos, pos1 - pos);
      pos = pos1;
    }

    if (withEmpty) {
      vt.push_back(std::move(strto<T>(s)));
    } else {
      if (!s.empty()) {
        vt.push_back(std::move(strto<T>(s)));
      }
    }

    if (pos1 == string::npos) {
      break;
    }

    pos++;
  }

  return vt;
}

template <>
string KT_Common::tostr<bool>(const bool &t);

template <>
string KT_Common::tostr<char>(const char &t);

template <>
string KT_Common::tostr<unsigned char>(const unsigned char &t);

template <>
string KT_Common::tostr<short>(const short &t);

template <>
string KT_Common::tostr<unsigned short>(const unsigned short &t);

template <>
string KT_Common::tostr<int>(const int &t);

template <>
string KT_Common::tostr<unsigned int>(const unsigned int &t);

template <>
string KT_Common::tostr<long>(const long &t);

template <>
string KT_Common::tostr<long long>(const long long &t);

template <>
string KT_Common::tostr<unsigned long>(const unsigned long &t);

template <>
string KT_Common::tostr<float>(const float &t);

template <>
string KT_Common::tostr<double>(const double &t);

template <>
string KT_Common::tostr<long double>(const long double &t);

template <>
string KT_Common::tostr<std::string>(const std::string &t);

template <typename T>
string KT_Common::tostr(const vector<T> &t) {
  string s;
  for (size_t i = 0; i < t.size(); i++) {
    s += tostr(t[i]);
    s += " ";
  }
  return s;
}

template <typename K, typename V, typename D, typename A>
string KT_Common::tostr(const map<K, V, D, A> &t) {
  string sBuffer;
  typename map<K, V, D, A>::const_iterator it = t.begin();
  while (it != t.end()) {
    sBuffer += " [";
    sBuffer += tostr(it->first);
    sBuffer += "]=[";
    sBuffer += tostr(it->second);
    sBuffer += "] ";
    ++it;
  }
  return sBuffer;
}

template <typename K, typename V, typename D, typename A>
string KT_Common::tostr(const multimap<K, V, D, A> &t) {
  string sBuffer;
  typename multimap<K, V, D, A>::const_iterator it = t.begin();
  while (it != t.end()) {
    sBuffer += " [";
    sBuffer += tostr(it->first);
    sBuffer += "]=[";
    sBuffer += tostr(it->second);
    sBuffer += "] ";
    ++it;
  }
  return sBuffer;
}

template <typename K, typename V, typename D, typename P, typename A>
string KT_Common::tostr(const unordered_map<K, V, D, P, A> &t) {
  string sBuffer;
  typename unordered_map<K, V, D, P, A>::const_iterator it = t.begin();
  while (it != t.end()) {
    sBuffer += " [";
    sBuffer += tostr(it->first);
    sBuffer += "]=[";
    sBuffer += tostr(it->second);
    sBuffer += "] ";
    ++it;
  }
  return sBuffer;
}

template <typename F, typename S>
string KT_Common::tostr(const pair<F, S> &itPair) {
  string sBuffer;
  sBuffer += "[";
  sBuffer += tostr(itPair.first);
  sBuffer += "]=[";
  sBuffer += tostr(itPair.second);
  sBuffer += "]";
  return sBuffer;
}

template <typename InputIter>
string KT_Common::tostr(InputIter iFirst, InputIter iLast, const string &sSep) {
  string sBuffer;
  InputIter it = iFirst;

  while (it != iLast) {
    sBuffer += tostr(*it);
    ++it;

    if (it != iLast) {
      sBuffer += sSep;
    } else {
      break;
    }
  }

  return sBuffer;
}

template <typename V, typename E>
bool KT_Common::equal(const V &x, const V &y, E eps) {
  return x == y;
}

template <typename K, typename V, typename D, typename A, typename E>
bool KT_Common::equal(const map<K, V, D, A> &mx, const map<K, V, D, A> &my, E epsilon) {
  auto first1 = mx.begin();
  auto last1 = mx.end();
  auto first2 = my.begin();
  auto last2 = my.end();

  if (distance(first1, last1) != distance(first2, last2)) {
    return false;
  }

  bool doubleKey = (std::is_same<K, double>::value || std::is_same<K, float>::value);
  bool doubleValue = (std::is_same<V, double>::value || std::is_same<V, float>::value);

  for (; first2 != last2; ++first1, ++first2) {
    if (doubleKey) {
      if (!KT_Common::equal(first1->first, first2->first, epsilon)) {
        return false;
      }
    } else {
      if (first1->first != first2->first) {
        return false;
      }
    }

    if (doubleValue) {
      if (!KT_Common::equal(first1->second, first2->second, epsilon)) {
        return false;
      }
    } else {
      if (first1->second != first2->second) {
        return false;
      }
    }
  }
  return true;
}

template <typename K, typename V, typename D, typename A, typename E>
bool KT_Common::equal(const unordered_map<K, V, D, A> &mx, const unordered_map<K, V, D, A> &my, E epsilon) {
  auto first1 = mx.begin();
  auto last1 = mx.end();
  auto first2 = my.begin();
  auto last2 = my.end();

  if (distance(first1, last1) != distance(first2, last2)) {
    return false;
  }

  bool doubleKey = (std::is_same<K, double>::value || std::is_same<K, float>::value);
  bool doubleValue = (std::is_same<V, double>::value || std::is_same<V, float>::value);

  for (; first2 != last2; ++first1, ++first2) {
    if (doubleKey) {
      if (!KT_Common::equal(first1->first, first2->first, epsilon)) {
        return false;
      }
    } else {
      if (first1->first != first2->first) {
        return false;
      }
    }

    if (doubleValue) {
      if (!KT_Common::equal(first1->second, first2->second, epsilon)) {
        return false;
      }
    } else {
      if (first1->second != first2->second) {
        return false;
      }
    }
  }
  return true;
}

#if TARGET_PLATFORM_WINDOWS
#define __filename__(x) (strrchr(x, '\\') ? strrchr(x, '\\') + 1 : x)
#define FILE_FUNC_LINE "[" << __filename__(__FILE__) << "::" << __FUNCTION__ << "::" << __LINE__ << "]"
#else
#define FILE_FUNC_LINE "[" << __FILE__ << "::" << __FUNCTION__ << "::" << __LINE__ << "]"
#endif

}  // namespace kant

#endif
