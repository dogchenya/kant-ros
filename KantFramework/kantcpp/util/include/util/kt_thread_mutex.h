#ifndef _KT_THREAD_MUTEX_H_
#define _KT_THREAD_MUTEX_H_

#include "util/kt_lock.h"
#include "util/kt_platform.h"
#include <mutex>
#include <atomic>

namespace kant {
/////////////////////////////////////////////////
/** 
 * @file tc_thread_mutex.h 
 * @brief 线程锁互斥类(底层直接封装了c++11, 从而跨平台兼容)
 *  
 * @author jarodruan@upchina.com  
 */

/////////////////////////////////////////////////
class KT_ThreadCond;

/**
* @brief 线程锁 . 
*  
* 不可重复加锁，即同一个线程不可以重复加锁 
*  
* 通常不直接使用，和TC_Monitor配合使用，即TC_ThreadLock; 
*/
class UTIL_DLL_API KT_ThreadMutex {
 public:
  KT_ThreadMutex();
  virtual ~KT_ThreadMutex();

  /**
     * @brief 加锁
     */
  void lock() const;

  /**
     * @brief 尝试锁
     * 
     * @return bool
     */
  bool tryLock() const;

  /**
     * @brief 解锁
     */
  void unlock() const;

 protected:
  // noncopyable
  KT_ThreadMutex(const KT_ThreadMutex&) = delete;
  void operator=(const KT_ThreadMutex&) = delete;

  friend class KT_ThreadCond;

 protected:
  mutable std::mutex _mutex;
};

/**
* @brief 线程锁类. 
*  
* 采用线程库实现
**/
class UTIL_DLL_API KT_ThreadRecMutex {
 public:
  /**
    * @brief 构造函数
    */
  KT_ThreadRecMutex();

  /**
    * @brief 析够函数
    */
  virtual ~KT_ThreadRecMutex();

  /**
	* @brief 锁, 调用pthread_mutex_lock. 
	*  
    */
  void lock() const;

  /**
	* @brief 解锁, pthread_mutex_unlock. 
	*  
    */
  void unlock() const;

  /**
	* @brief 尝试锁, 失败抛出异常. 
	*  
    * return : true, 成功锁; false 其他线程已经锁了
    */
  bool tryLock() const;

 protected:
  /**
     * @brief 友元类
     */
  friend class KT_ThreadCond;

 private:
  /**
    锁对象
    */
  mutable recursive_mutex _mutex;
};

}  // namespace kant
#endif
