﻿#ifndef _KT_MONITOR_H_
#define _KT_MONITOR_H_

#include "util/kt_thread_mutex.h"
#include "util/kt_thread_cond.h"

namespace kant {
/////////////////////////////////////////////////
/** 
 * @file tc_monitor.h 
 * @brief 锁监控器类.
 * @brief Lock Monitor Class.
 *  
 * @author ruanshudong@qq.com
 */
/////////////////////////////////////////////////
/**
 * @brief 线程锁监控模板类.
 * @brief Thread Lock Monitoring Template Class.
 * 通常线程锁，都通过该类来使用，而不是直接用KT_ThreadMutex、KT_ThreadRecMutex 
 * Typically thread locks are used through this class, rather than directly using KT_ThreadMutex,KT_ThreadRecMutex
 *  
 * 该类将KT_ThreadMutex/KT_ThreadRecMutex 与KT_ThreadCond结合起来； 
 * This class combines KT_ThreadMutex/KT_ThreadRecMutex with KT_ThreadCond;
 */
template <class T, class P>
class KT_Monitor {
 public:
  /**
     * @brief 定义锁控制对象
     * @brief Define lock control objects
     */
  typedef KT_LockT<KT_Monitor<T, P>> Lock;
  typedef KT_TryLockT<KT_Monitor<T, P>> TryLock;

  /**
     * @brief 构造函数
     * @brief Constructor
     */
  KT_Monitor() : _nnotify(0) {}

  /**
     * @brief 析构
     * @brief Destructor
     */
  virtual ~KT_Monitor() {}

  /**
     * @brief 锁
     * @brief Lock
     */
  void lock() const {
    _mutex.lock();
    _nnotify = 0;
  }

  /**
     * @brief 解锁, 根据上锁的次数通知
     * @brief Unlock, notified according to the number of locks
     */
  void unlock() const {
    notifyImpl(_nnotify);
    _mutex.unlock();
  }

  /**
     * @brief 尝试锁.
     * @brief Attempt to lock.
     *
     * @return bool
     */
  bool tryLock() const {
    bool result = _mutex.tryLock();
    if (result) {
      _nnotify = 0;
    }
    return result;
  }

  /**
     * @brief 等待,当前调用线程在锁上等待，直到事件通知，
     * @brief Wait, the current calling thread waits on the lock until event notification,
     */
  void wait() const {
    // _cond.wait(_mutex);

    notifyImpl(_nnotify);

    try {
      _cond.wait(_mutex);
    } catch (...) {
      _nnotify = 0;
      throw;
    }

    _nnotify = 0;
  }

  /**
	 * @brief 等待时间,当前调用线程在锁上等待，直到超时或有事件通知
     * @brief Waiting time, current calling thread waiting on lock until timeout or event notification
	 *  
	 * @param millsecond 等待时间
     * @param millsecond waiting time
     * @return           false:超时了, ture:有事件来了
     * @return           false: timeout, ture: there's an event coming
     */
  bool timedWait(int millsecond) const {
    // return _cond.timedWait(_mutex, millsecond);

    notifyImpl(_nnotify);

    bool rc;

    try {
      rc = _cond.timedWait(_mutex, millsecond);
    } catch (...) {
      _nnotify = 0;
      throw;
    }

    _nnotify = 0;
    return rc;
  }

  /**
	 * @brief 通知某一个线程醒来 
     * @brief Notify a thread to wake up
	 *  
	 * 通知等待在该锁上某一个线程醒过来 ,调用该函数之前必须加锁, 
     * Notifies that a thread waiting on the lock must wake up and be locked before calling the function.
	 *  
	 * 在解锁的时候才真正通知 
     * Real notification when unlocked
     */
  void notify() {
    // _cond.signal();
    if (_nnotify != -1) {
      ++_nnotify;
    }
  }

  /**
	 * @brief 通知等待在该锁上的所有线程醒过来，
     * @brief Notify all threads waiting on this lock to wake up.
	 * 注意调用该函数时必须已经获得锁.
     * Note that the lock must have been acquired when calling this function.
	 *  
	 * 该函数调用前之必须加锁, 在解锁的时候才真正通知 
     * This function must be locked before it is called, and will not be notified until it is unlocked.
     */
  void notifyAll() {
    // _cond.broadcast();
    _nnotify = -1;
  }

 protected:
  /**
	 * @brief 通知实现. 
     * @brief Notification implementation
	 *  
     * @param nnotify 上锁的次数
     * @param nnotify Number of Locks
     */
  void notifyImpl(int nnotify) const {
    if (nnotify != 0) {
      if (nnotify == -1) {
        _cond.broadcast();
        return;
      } else {
        while (nnotify > 0) {
          _cond.signal();
          --nnotify;
        }
      }
    }
  }

 private:
  /** 
	  * @brief noncopyable
	  */
  KT_Monitor(const KT_Monitor&);
  void operator=(const KT_Monitor&);

 protected:
  /**
	 * 上锁的次数
     * Number of Locks
	 */
  mutable int _nnotify;
  mutable P _cond;
  T _mutex;
};

/**
 * 普通线程锁
 * Normal Thread Lock
 */
typedef KT_Monitor<KT_ThreadMutex, KT_ThreadCond> KT_ThreadLock;

/**
 * 循环锁(一个线程可以加多次锁)
 * Circular locks (a thread can lock multiple times)
 */
typedef KT_Monitor<KT_ThreadRecMutex, KT_ThreadCond> KT_ThreadRecLock;

}  // namespace kant
#endif
