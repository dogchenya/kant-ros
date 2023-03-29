#include "util/kt_thread_pool.h"
#include "util/kt_common.h"

#include <iostream>

namespace kant {
KT_ThreadPool::KT_ThreadPool() : _threadNum(1), _bTerminate(true) {}

KT_ThreadPool::~KT_ThreadPool() { stop(); }

void KT_ThreadPool::init(size_t num) {
  std::unique_lock<std::mutex> lock(_mutex);
  if (!_threads.empty()) {
    throw KT_ThreadPool_Exception("[KT_ThreadPool::init] thread pool has start!");
  }

  _threadNum = num;
}

void KT_ThreadPool::start() {
  std::unique_lock<std::mutex> lock(_mutex);
  if (!_threads.empty()) {
    throw KT_ThreadPool_Exception("[KT_ThreadPool::init] thread pool has start!");
  }

  _bTerminate = false;

  for (size_t i = 0; i < _threadNum; i++) {
    _threads.push_back(new std::thread(&KT_ThreadPool::run, this));
  }
}

void KT_ThreadPool::stop() {
  if (_bTerminate) {
    return;
  }

  {
    std::unique_lock<std::mutex> lock(_mutex);

    _bTerminate = true;

    _condition.notify_all();
  }

  for (size_t i = 0; i < _threads.size(); i++) {
    if (_threads[i]->joinable()) {
      _threads[i]->join();
    }

    delete _threads[i];
    _threads[i] = NULL;
  }

  std::unique_lock<std::mutex> lock(_mutex);
  _threads.clear();
}

bool KT_ThreadPool::get(TaskFuncPtr& task) {
  std::unique_lock<std::mutex> lock(_mutex);

  if (_tasks.empty()) {
    _condition.wait(lock, [this] { return _bTerminate || !_tasks.empty(); });
  }

  if (_bTerminate) return false;

  if (!_tasks.empty()) {
    task = std::move(_tasks.front());

    _tasks.pop();

    return true;
  }

  return false;
}

void KT_ThreadPool::run() {
  //调用处理部分
  while (!isTerminate()) {
    TaskFuncPtr task;
    bool ok = get(task);

    if (ok) {
      ++_atomic;

      try {
        if (task->_expireTime != 0 && task->_expireTime < TNOWMS) {
          //超时任务，是否需要处理?
        } else {
          task->_func();
        }
      } catch (...) {
      }

      --_atomic;

      //任务都执行完毕了
      std::unique_lock<std::mutex> lock(_mutex);
      if (_atomic == 0 && _tasks.empty()) {
        _condition.notify_all();
      }
    }
  }
}

bool KT_ThreadPool::waitForAllDone(int millsecond) {
  std::unique_lock<std::mutex> lock(_mutex);

  if (_tasks.empty()) return true;

  if (millsecond < 0) {
    _condition.wait(lock, [this] { return _tasks.empty(); });
    return true;
  } else {
    return _condition.wait_for(lock, std::chrono::milliseconds(millsecond), [this] { return _tasks.empty(); });
  }
}

///////////////////////////////////////////////////////////////////////////////////////

KT_ThreadPoolHash::KT_ThreadPoolHash() {}

KT_ThreadPoolHash::~KT_ThreadPoolHash() {}

void KT_ThreadPoolHash::init(size_t num) {
  for (size_t i = 0; i < num; i++) {
    KT_ThreadPool* p = new KT_ThreadPool();
    p->init(1);
    _pools.push_back(p);
  }
}

KT_ThreadPool* KT_ThreadPoolHash::getThread(size_t index) {
  if (_pools.empty() || (index + 1) > _pools.size()) {
    return nullptr;
  }
  return _pools[index];
}

KT_ThreadPool* KT_ThreadPoolHash::selectThread(const string& hashkey) {
  if (_pools.empty()) {
    return nullptr;
  }
  std::hash<string> hash_fu;
  size_t pos = hash_fu(hashkey) % _pools.size();
  return _pools[pos];
}

void KT_ThreadPoolHash::stop() {
  for (size_t i = 0; i < _pools.size(); i++) {
    _pools[i]->stop();
    delete _pools[i];
  }
  _pools.clear();
}

void KT_ThreadPoolHash::start() {
  for (size_t i = 0; i < _pools.size(); i++) {
    _pools[i]->start();
  }
}
}  // namespace kant
