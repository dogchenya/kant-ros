#ifndef _KT_TIME_PROVIDER_H_
#define _KT_TIME_PROVIDER_H_

#include <string.h>
#include "util/kt_platform.h"
#include "util/kt_common.h"
#include "util/kt_thread.h"
//#include "util/kt_autoptr.h"

namespace kant {
#define TNOW KT_TimeProvider::getInstance()->getNow()
#define TNOWMS KT_TimeProvider::getInstance()->getNowMs()

/////////////////////////////////////////////////
/**
 * @file kt_timeprovider.h
 * @brief 秒级、毫秒级时间提供类(linux有独立线程来获取时间, windows直接使用获取时间函数KT_Common::gettimeofday())
 *
 * @author jarodruan@upchina.com
 */
/////////////////////////////////////////////////
class KT_TimeProvider;

/**
 * @brief 提供秒级别的时间
 */
class UTIL_DLL_API KT_TimeProvider : public KT_Thread {
 public:
  /**
     * @brief 获取实例.
     *
     * @return TimeProvider&
     */
  static KT_TimeProvider* getInstance();

  /**
     * @brief 构造函数
     */
  KT_TimeProvider() : _use_tsc(true), _cpu_cycle(0), _buf_idx(0) {
    memset(_t, 0, sizeof(_t));
    memset(_tsc, 0, sizeof(_tsc));

    struct timeval tv;
    KT_Common::gettimeofday(tv);
    _t[0] = tv;
    _t[1] = tv;
  }

  /**
     * @brief 析构，停止线程
     */
  ~KT_TimeProvider();

  /**
     * @brief 获取时间.
     *
     * @return time_t 当前时间
     */
  time_t getNow() { return _t[_buf_idx].tv_sec; }

  /**
     * @brief 获取时间.
     *
     * @para timeval
     * @return void
     */
  void getNow(timeval* tv);

  /**
     * @brief 获取ms时间.
     *
     * @para timeval
     * @return void
     */
  uint64_t getNowMs();

 protected:
  static KT_TimeProvider* g_tp;

 protected:
  /**
	* @brief 运行
	*/
  virtual void run();

  uint64_t GetCycleCount();

  /**
	* @brief 获取cpu主频.
	*
	* @return float cpu主频
	*/
  // double cpuMHz();

  void setTsc(timeval& tt);

  void addTimeOffset(timeval& tt, const int& idx);

  void terminate();

 private:
  bool _use_tsc;

  double _cpu_cycle;

  volatile int _buf_idx;

  timeval _t[2];

  uint64_t _tsc[2];

  bool _terminate = false;
};

}  // namespace kant

#endif
