#ifndef _KT_THREAD_COND_H_
#define _KT_THREAD_COND_H_

#include <cerrno>
#include <iostream>
#include <condition_variable>
#include "util/kt_platform.h"
#include "util/kt_ex.h"

namespace kant {
/////////////////////////////////////////////////
/**
 * @file tc_thread_cond.h 
 * @brief 线程锁以及条件变量类(兼容TARS4.x版本, 底层直接封装了c++11, 从而跨平台兼容)
 *  
 * @author  jarodruan@upchina.com
 */
/////////////////////////////////////////////////
class KT_ThreadMutex;

/**
 *  @brief 线程信号条件类, 所有锁可以在上面等待信号发生
 *  
 *  和KT_ThreadMutex、KT_ThreadRecMutex配合使用,
 *  
 *  通常不直接使用，而是使用TC_ThreadLock/TC_ThreadRecLock;
 */
class KT_ThreadCond {
 public:
  /**
     *  @brief 构造函数
     */
  KT_ThreadCond();

  /**
     *  @brief 析构函数
     */
  ~KT_ThreadCond();

  /**
     *  @brief 发送信号, 等待在该条件上的一个线程会醒
     */
  void signal();

  /**
     *  @brief 等待在该条件的所有线程都会醒
     */
  void broadcast();

  /**
	 *  @brief 无限制等待.
	 *  
     * @param M
     */
  template <typename Mutex>
  void wait(const Mutex& mutex) const {
    _cond.wait(mutex._mutex);
  }

  /**
	 * @brief 等待时间. 
	 *  
	 * @param M 
     * @return bool, false表示超时, true:表示有事件来了
     */
  template <typename Mutex>
  bool timedWait(const Mutex& mutex, int millsecond) const {
    if (_cond.wait_for(mutex._mutex, std::chrono::milliseconds(millsecond)) == std::cv_status::timeout) {
      return false;
    }
    return true;
  }

 protected:
  // Not implemented; prevents accidental use.
  KT_ThreadCond(const KT_ThreadCond&);
  KT_ThreadCond& operator=(const KT_ThreadCond&);

 private:
  /**
     * 线程条件
     */
  mutable std::condition_variable_any _cond;
};

}  // namespace kant

#endif
