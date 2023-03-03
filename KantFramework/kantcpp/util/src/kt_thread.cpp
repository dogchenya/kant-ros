#include "util/kt_thread.h"
#include "util/kt_port.h"
//#include "util/kt_coroutine.h"
#include "util/kt_common.h"
#include <sstream>
#include <cerrno>
#include <cassert>

namespace kant {

KT_ThreadControl::KT_ThreadControl(std::thread *th) : _th(th) { assert(_th != NULL); }

void KT_ThreadControl::join() {
  if (std::this_thread::get_id() == _th->get_id()) {
    throw KT_ThreadThreadControl_Exception("[KT_ThreadControl::join] can't be called in the same thread");
  }

  if (_th->joinable()) {
    _th->join();
  }
}

void KT_ThreadControl::detach() { _th->detach(); }

std::thread::id KT_ThreadControl::id() const { return _th->get_id(); }

void KT_ThreadControl::sleep(int64_t millsecond) { std::this_thread::sleep_for(std::chrono::milliseconds(millsecond)); }

void KT_ThreadControl::yield() { std::this_thread::yield(); }

KT_Thread::KT_Thread(const string &threadName) : _threadName(threadName), _running(false), _th(NULL) {}

KT_Thread::~KT_Thread() {
  if (_th != NULL) {
    //如果资源没有被detach或者被join，则自己释放
    if (_th->joinable()) {
      _th->detach();
    }

    delete _th;
    _th = NULL;
  }
}

void KT_Thread::setThreadName(const string &threadName) { _threadName = threadName; }

class RunningClosure {
 public:
  RunningClosure(KT_Thread *pThread) : _pThread(pThread) { pThread->_running = true; }

  ~RunningClosure() {
    if (!_pThread->getScheduler()) {
      //非协程模式
      _pThread->_running = false;
    }
  }

 protected:
  KT_Thread *_pThread;
};

void KT_Thread::threadEntry(KT_Thread *pThread) {
  RunningClosure r(pThread);

  {
    KT_ThreadLock::Lock sync(pThread->_lock);
    pThread->_lock.notifyAll();
  }

  try {
    pThread->run();
  } catch (exception &ex) {
    cerr << std::this_thread::get_id() << "|" << ex.what() << endl;
    throw ex;
  } catch (...) {
    throw;
  }
}

KT_ThreadControl KT_Thread::start() {
  KT_ThreadLock::Lock sync(_lock);

  if (_running) {
    throw KT_ThreadThreadControl_Exception("[KT_Thread::start] thread has start");
  }

  try {
    _th = new std::thread(&KT_Thread::threadEntry, this);

  } catch (...) {
    throw KT_ThreadThreadControl_Exception("[KT_Thread::start] thread start error");
  }

  _lock.wait();

  return KT_ThreadControl(_th);
}

//void KT_Thread::coroutineEntry(KT_Thread *pThread, uint32_t iPoolSize, size_t iStackSize, bool autoQuit) {
//  pThread->_scheduler = TC_CoroutineScheduler::create();
//
//  pThread->_scheduler->setPoolStackSize(iPoolSize, iStackSize);
//
//  if (autoQuit) {
//    pThread->_scheduler->setNoCoroutineCallback([](TC_CoroutineScheduler *scheduler) { scheduler->terminate(); });
//  }
//
//  pThread->_scheduler->go(std::bind(KT_Thread::threadEntry, pThread));
//
//  {
//    KT_ThreadLock::Lock sync(pThread->_lock);
//    pThread->_lock.notifyAll();
//  }
//
//  pThread->_scheduler->run();
//
//  pThread->_running = false;
//
//  pThread->_scheduler.reset();
//  TC_CoroutineScheduler::reset();
//}
//
//KT_ThreadControl KT_Thread::startCoroutine(uint32_t iPoolSize, size_t iStackSize, bool autoQuit) {
//  KT_ThreadLock::Lock sync(_lock);
//
//  if (_running) {
//    throw KT_ThreadThreadControl_Exception("[KT_Thread::startCoroutine] thread has start");
//  }
//
//  try {
//    _th = new std::thread(&KT_Thread::coroutineEntry, this, iPoolSize, iStackSize, autoQuit);
//  } catch (...) {
//    throw KT_ThreadThreadControl_Exception("[KT_Thread::startCoroutine] thread start error");
//  }
//
//  _lock.wait();
//
//  return KT_ThreadControl(_th);
//}

void KT_Thread::join() {
  if (!_th) {
    return;
  }
  if (std::this_thread::get_id() == _th->get_id()) {
    throw KT_ThreadThreadControl_Exception("[KT_Thread::join] can't be called in the same thread");
  }

  if (_th->joinable()) {
    _th->join();
  }
}

bool KT_Thread::joinable() {
  if (!_th) {
    return false;
  }

  return _th->joinable();
}

void KT_Thread::detach() { _th->detach(); }

KT_ThreadControl KT_Thread::getThreadControl() { return KT_ThreadControl(_th); }

bool KT_Thread::isAlive() const { return _running; }

size_t KT_Thread::CURRENT_THREADID() {
  static thread_local size_t threadId = 0;
  if (threadId == 0) {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    threadId = strtol(ss.str().c_str(), NULL, 0);
  }
  return threadId;
}

}  // namespace kant
