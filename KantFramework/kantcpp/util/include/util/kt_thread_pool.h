#pragma once

#include "util/kt_platform.h"
#include "util/kt_thread.h"
#include "util/kt_timeprovider.h"
#include <future>
#include <functional>
#include <iostream>
#include <queue>
#include <set>
#include <vector>
#include <mutex>
#include <utility>

using namespace std;

namespace kant {
/////////////////////////////////////////////////
/**
 * @file kt_thread_pool.h
 * @brief 线程池类,采用c++11来实现了
 * 使用说明:
 * KT_ThreadPool tpool;
 * tpool.init(5);   //初始化线程池线程数
 * //启动线程有两种方式
 * //第一种, 直接启动
 * tpool.start();
 * //第二种, 启动时指定初始化函数, 比如定义函数
 * void testFunction(int i)
 * {
 *     cout << i << endl;
 * }
 * tpool.start(testFunction, 5);    //start的第一函数是std::bind返回的函数(std::function), 后面跟参数
 * //将任务丢到线程池中
 * tpool.exec(testFunction, 10);    //参数和start相同
 * //等待线程池结束, 有两种方式:
 * //第一种等待线程池中无任务
 * tpool.waitForAllDone(1000);      //参数<0时, 表示无限等待(注意有人调用stop也会推出)
 * //第二种等待外部有人调用线程池的stop函数
 * tpool.waitForStop(1000);
 * //此时: 外部需要结束线程池是调用
 * tpool.stop();
 * 注意:
 * KT_ThreadPool::exec执行任务返回的是个future, 因此可以通过future异步获取结果, 比如:
 * int testInt(int i)
 * {
 *     return i;
 * }
 * auto f = tpool.exec(testInt, 5);
 * cout << f.get() << endl;   //当testInt在线程池中执行后, f.get()会返回数值5
 *
 * class Test
 * {
 * public:
 *     int test(int i);
 * };
 * Test t;
 * auto f = tpool.exec(std::bind(&Test::test, &t, std::placeholders::_1), 10);
 * //返回的future对象, 可以检查是否执行
 * cout << f.get() << endl;
 * @author  jarodruan@upchina.com
 */
/////////////////////////////////////////////////
/**
* @brief 线程异常
*/
struct KT_ThreadPool_Exception : public KT_Exception {
  KT_ThreadPool_Exception(const string& buffer) : KT_Exception(buffer){};
  KT_ThreadPool_Exception(const string& buffer, bool err) : KT_Exception(buffer, err){};
  ~KT_ThreadPool_Exception() throw(){};
};

/**
* @brief 用通线程池类(采用c++11实现)
*
* 使用方式说明:
* 具体示例代码请参见:examples/util/example_kt_thread_pool.cpp
*/

class KT_ThreadPool {
 protected:
  struct TaskFunc {
    TaskFunc(uint64_t expireTime) : _expireTime(expireTime) {}

    std::function<void()> _func;
    uint64_t _expireTime = 0;  //超时的绝对时间
  };
  typedef shared_ptr<TaskFunc> TaskFuncPtr;

 public:
  /**
    * @brief 构造函数
    *
    */
  KT_ThreadPool();

  /**
    * @brief 析构, 会停止所有线程
    */
  virtual ~KT_ThreadPool();

  /**
    * @brief 初始化.
    *
    * @param num 工作线程个数
    */
  void init(size_t num);

  /**
    * @brief 获取线程个数.
    *
    * @return size_t 线程个数
    */
  size_t getThreadNum() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _threads.size();
  }

  /**
    * @brief 获取当前线程池的任务数
    *
    * @return size_t 线程池的任务数
    */
  size_t getJobNum() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _tasks.size();
  }

  /**
    * @brief 停止所有线程, 会等待所有线程结束
    */
  void stop();

  /**
    * @brief 启动所有线程
    */
  void start();

  /**
    * @brief 用线程池启用任务(F是function, Args是参数)
    *
    * @param ParentFunctor
    * @param tf
    * @return 返回任务的future对象, 可以通过这个对象来获取返回值
    */
  template <class F, class... Args>
  auto exec(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    return exec(0, f, args...);
  }

  /**
    * @brief 用线程池启用任务(F是function, Args是参数)
    *
    * @param 超时时间 ，单位ms (为0时不做超时控制) ；若任务超时，此任务将被丢弃
    * @param bind function
    * @return 返回任务的future对象, 可以通过这个对象来获取返回值
    */
  template <class F, class... Args>
  auto exec(int64_t timeoutMs, F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    int64_t expireTime = (timeoutMs == 0 ? 0 : TNOWMS + timeoutMs);
    //定义返回值类型
    using RetType = decltype(f(args...));
    auto task =
      std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    TaskFuncPtr fPtr = std::make_shared<TaskFunc>(expireTime);
    fPtr->_func = [task] { (*task)(); };

    std::unique_lock<std::mutex> lock(_mutex);
    _tasks.push(fPtr);
    _condition.notify_one();

    return task->get_future();
  }

  /**
     * @brief 等待当前任务队列中, 所有task全部结束(队列无任务).
     *
     * @param millsecond 等待的时间(ms), -1:永远等待
     * @return           true, 所有工作都处理完毕 
     *                   false,超时退出
     */
  bool waitForAllDone(int millsecond = -1);

  /**
    * @brief 线程池是否退出
    */
  bool isTerminate() { return _bTerminate; }

 protected:
  /**
    * @brief 获取任务
    *
    * @return TaskFuncPtr
    */
  bool get(TaskFuncPtr& task);

  /**
    * @brief 线程运行态
    */
  void run();

 protected:
  /**
    * 任务队列
    */
  queue<TaskFuncPtr> _tasks;

  std::mutex _mutex;

  /**
    * 工作线程
    */
  std::vector<std::thread*> _threads;

  size_t _threadNum;

  std::condition_variable _condition;

  bool _bTerminate;

  std::atomic<int> _atomic{0};
};

//根据keyhash的线程池
class KT_ThreadPoolHash {
 public:
  KT_ThreadPoolHash();
  virtual ~KT_ThreadPoolHash();

  KT_ThreadPoolHash(const KT_ThreadPoolHash&) = delete;
  KT_ThreadPoolHash& operator=(const KT_ThreadPoolHash&) = delete;

  /**
    * @brief 初始化.
    *
    * @param num 工作线程个数
    */
  void init(size_t num);

  /**
    * @brief 停止所有线程, 会等待所有线程结束
    */
  void stop();

  /**
        * @brief 启动所有线程
        */
  void start();

  /**
    * @brief 用线程池启用任务(F是function, Args是参数)
    *
    * @param hashkey 根据次key保证将同样的任务hash到同样的队列中处理
    * @param tf
    * @return 返回任务的future对象, 可以通过这个对象来获取返回值
    */
  template <class F, class... Args>
  auto exec(const string& hashkey, F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    return exec(hashkey, 0, f, args...);
  }

  /**
    * @brief 用线程池启用任务(F是function, Args是参数)
    * @param 超时时间 ，单位ms (为0时不做超时控制) ；若任务超时，此任务将被丢弃
    * @param hashkey 根据次key保证将同样的任务hash到同样的队列中处理
    * @param tf
    * @return 返回任务的future对象, 可以通过这个对象来获取返回值
    */
  template <class F, class... Args>
  auto exec(const string& hashkey, int64_t timeoutMs, F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    KT_ThreadPool* thread = selectThread(hashkey);
    if (thread) {
      return thread->exec(timeoutMs, f, args...);
    } else {
      throw KT_ThreadPool_Exception("[KT_ThreadPoolHash::start] no worker thread!");
    }
  }

  KT_ThreadPool* getThread(size_t index);
  size_t size() { return _pools.size(); }

 protected:
  KT_ThreadPool* selectThread(const string& hashkey);

 private:
  vector<KT_ThreadPool*> _pools;
};

}  // namespace kant