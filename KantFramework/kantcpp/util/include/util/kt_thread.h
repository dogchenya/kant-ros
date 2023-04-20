#ifndef _KT_THREAD_H_
#define _KT_THREAD_H_

#include <sys/types.h>
#include <signal.h>
// #include <pthread.h>
#include <thread>
#include "util/kt_ex.h"
#include "util/kt_monitor.h"

namespace kant {
/////////////////////////////////////////////////
/** 
 * @file kt_thread.h 
 * @brief  线程类(底层直接封装了c++11 thread, 从而跨平台兼容)
 *
 * 使用说明:
 * - KT_Thread定义一个线程, 继承KT_Thread, 实现run方法, 调用start即可启动线程
 * - 注意KT_Thread也可以定义一个协程, 继承KT_Thread, 实现run方法, 调用startCoroutine方法, 即可进入协程执行, 注意startCoroutine定义了线程退出的方式
 * >- 一种是: 所有协程都执行完毕以后, 自动退出
 * >- 一种是: 不自动退出, 除非显示调用KT_CoroutineScheduler::terminate方法
 */
/////////////////////////////////////////////////

/**
 * @brief  线程控制异常类
 */
struct KT_ThreadThreadControl_Exception : public KT_Exception {
  KT_ThreadThreadControl_Exception(const string &buffer) : KT_Exception(buffer){};
  ~KT_ThreadThreadControl_Exception() throw(){};
};

class KT_CoroutineScheduler;

/**
 * @brief  线程控制类
 */
class KT_ThreadControl {
 public:
  /**
	 * @brief  构造, 表示当前运行的线程，
	 *  		join和detach在不能在该对象上调用
	*/
  explicit KT_ThreadControl(std::thread *th);

  /**
     * @brief  等待当前线程结束, 不能在当前线程上调用
     */
  void join();

  /**
     * @brief  detach, 不能在当前线程上调用
     */
  void detach();

  /**
     * @brief  获取当前线程id.
     *
     * @return pthread_t当前线程id
     */
  std::thread::id id() const;

  /**
	 * @brief  休息ms时间. 
	 *  
     * @param millsecond 休息的时间，具体ms数字
     */
  static void sleep(int64_t millsecond);

  /**
     * @brief  交出当前线程控制权
     */
  static void yield();

 private:
  std::thread *_th;
};

/**
 *
 */
class KT_Runable {
 public:
  virtual ~KT_Runable(){};
  virtual void run() = 0;
};

/**
 * @brief 线程基类. 
 * 线程基类，所有自定义线程继承于该类，同时实现run接口即可, 
 *  
 * 可以通过KT_ThreadControl管理线程。
 */
class KT_Thread : public KT_Runable {
 public:
  /**
     * @brief  构造函数
	 */
  KT_Thread(const string &threadName = "");

  /**
     * @brief  析构函数
	 */
  virtual ~KT_Thread();

  /**
	 * 设置线程名称(一般调试方便)
	 * @param threadName
	 */
  void setThreadName(const string &threadName);

  /**
     * @brief  线程运行
	 */
  KT_ThreadControl start();

  /**
     * @brief  启动, 并处于协程中, 当前线程进入tc_coroutine的run, 当前对象执行KT_Thread::run
     * @param iPoolSize 协程池个数
     * @param iStackSize 协程堆栈大小
     * @param autoQuit 自动退出
     * 注意: autoQuit=true时, 当前所有协程执行完毕, 则线程调度退出(线程也退出了) 
     *      autoQuit=false时, 协程调度器不主动退出, 除非调用协程调度器的terminate
	 */
  KT_ThreadControl startCoroutine(uint32_t iPoolSize, size_t iStackSize, bool autoQuit = false);

  /**
     * @brief  获取线程控制类.
     *
     * @return ThreadControl
     */
  KT_ThreadControl getThreadControl();

  /**
     * @brief  等待当前线程结束, 不能在当前线程上调用
     * 可以不再使用KT_ThreadControl类
     */
  void join();

  /**
     * 是否可以join
     * @return
     */
  bool joinable();

  /**
     * @brief  detach, 不能在当前线程上调用
     * 可以不再使用KT_ThreadControl类
     */
  void detach();

  /**
     * @brief  线程是否存活.
     *
     * @return bool 存活返回true，否则返回false
     */
  bool isAlive() const;

  /**
     * @brief  获取线程id.
	 *
	 * @return std::thread::id 线程id
	 */
  std::thread::id id() { return _th->get_id(); }

  /**
     * @brief  获取线程
	 *
	 * @return std::thread::id 线程id
	 */
  std::thread *getThread() { return _th; }

  /**
     * 协程模式下, 获取调度器, 非协程模式下则为NULL
     * @return
     */
  const shared_ptr<KT_CoroutineScheduler> &getScheduler() { return _scheduler; }

  /**
     * @brief  获取当前线程ID, 用size_t返回
	 *
	 * @return 
	 */
  static size_t CURRENT_THREADID();

 protected:
  /**
	 * @brief  静态函数, 线程入口. 
	 *  
	 * @param pThread 线程对象
	 */
  static void threadEntry(KT_Thread *pThread);

  /**
	 * @brief  静态函数, 协程入口. 
	 *  
	 * @param pThread 线程对象
	 */
  static void coroutineEntry(KT_Thread *pThread, uint32_t iPoolSize, size_t iStackSize, bool autoQuit);

  /**
     * @brief  运行
	 */
  virtual void run() = 0;

  friend class RunningClosure;

 protected:
  /**
     * 线程名称
     */
  string _threadName;

  /**
     * 是否在运行
     */
  bool _running;

  /**
     * 线程锁
     */
  KT_ThreadLock _lock;

  //当前线程
  std::thread *_th;

  /**
     * 协程调度器
     */
  shared_ptr<KT_CoroutineScheduler> _scheduler;
};

}  // namespace kant
#endif
