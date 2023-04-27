#include "util/kt_logger.h"
#include <iostream>
#include <string.h>
#include <mutex>

namespace kant {

KT_RollLogger __global_logger_debug__;

static int init_global_debug_log() {
  __global_logger_debug__.modFlag(__global_logger_debug__.HAS_MTIME);
  return 0;
}
static int __global_logger_debug_init__ = init_global_debug_log();

bool KT_LoggerRoll::_bDyeingFlag = false;
KT_SpinLock KT_LoggerRoll::_mutexDyeing;
unordered_map<size_t, string> KT_LoggerRoll::_mapThreadID;

const string LogByDay::FORMAT = "%Y%m%d";
const string LogByHour::FORMAT = "%Y%m%d%H";
const string LogByMinute::FORMAT = "%Y%m%d%H%M";

void KT_LoggerRoll::setupThread(KT_LoggerThreadGroup *pThreadGroup) {
  assert(pThreadGroup != NULL);

  unSetupThread();

  std::lock_guard<std::mutex> lock(_mutex);

  _pThreadGroup = pThreadGroup;

  KT_LoggerRollPtr self = shared_from_this();

  _pThreadGroup->registerLogger(self);
}

void KT_LoggerRoll::unSetupThread() {
  std::lock_guard<std::mutex> lock(_mutex);

  if (_pThreadGroup != NULL) {
    _pThreadGroup->flush();

    KT_LoggerRollPtr self = shared_from_this();

    _pThreadGroup->unRegisterLogger(self);

    _pThreadGroup = NULL;
  }

  flush();
}

void KT_LoggerRoll::write(const pair<std::size_t, string> &buffer) {
  size_t ThreadID = 0;
  if (_bDyeingFlag) {
    KT_LockT<KT_SpinLock> lock(_mutexDyeing);

    if (_mapThreadID.find(KT_Thread::CURRENT_THREADID()) != _mapThreadID.end()) {
      ThreadID = KT_Thread::CURRENT_THREADID();
    }
  }

  if (_pThreadGroup) {
    _buffer.push_back(make_pair(ThreadID, buffer.second));
  } else {
    //同步记录日志
    deque<pair<size_t, string>> ds;
    ds.push_back(make_pair(ThreadID, buffer.second));
    roll(ds);
  }
}

void KT_LoggerRoll::flush() {
  KT_CasQueue<pair<size_t, string>>::queue_type qt;

  _buffer.swap(qt);

  if (!qt.empty()) {
    roll(qt);
  }
}

//////////////////////////////////////////////////////////////////
//
KT_LoggerThreadGroup::KT_LoggerThreadGroup() : _bTerminate(false), _thread(NULL) {}

KT_LoggerThreadGroup::~KT_LoggerThreadGroup() {
  terminate();
  /*挪到terminate函数中
	flush();

	{
		std::lock_guard<std::mutex> guard(_mutex);
		_bTerminate = true;
		_cond.notify_all();
	}

	if(_thread)
	{
		_thread->join();
		delete _thread;
		_thread = NULL;
	}
    */
}

void KT_LoggerThreadGroup::start(size_t iThreadNum) {
  if (_thread == NULL) {
    _thread = new std::thread(&KT_LoggerThreadGroup::run, this);
  }
}

void KT_LoggerThreadGroup::registerLogger(KT_LoggerRollPtr &l) {
  std::lock_guard<std::mutex> lock(_mutex);

  _logger.insert(l);
}

void KT_LoggerThreadGroup::unRegisterLogger(KT_LoggerRollPtr &l) {
  std::lock_guard<std::mutex> lock(_mutex);

  _logger.erase(l);
}

void KT_LoggerThreadGroup::terminate() {
  if (_bTerminate) return;

  flush();

  {
    std::unique_lock<std::mutex> lock(_mutex);
    _bTerminate = true;
    _cond.notify_all();
  }

  if (_thread) {
    _thread->join();
    delete _thread;
    _thread = NULL;
  }
}

void KT_LoggerThreadGroup::flush() {
  logger_set logger;

  {
    std::lock_guard<std::mutex> lock(_mutex);
    logger = _logger;
  }

  logger_set::iterator it = logger.begin();
  while (it != logger.end()) {
    try {
      it->get()->flush();
    } catch (exception &ex) {
      cerr << "[KT_LoggerThreadGroup::flush] log flush error:" << ex.what() << endl;
    } catch (...) {
    }
    ++it;
  }
}

void KT_LoggerThreadGroup::run() {
  while (!_bTerminate) {
    //100ms
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _cond.wait_for(lock, std::chrono::milliseconds(100));
    }

    flush();
  }
}

//////////////////////////////////////////////////////////////////////////////////

LoggerBuffer::LoggerBuffer() : _buffer(NULL), _buffer_len(0) {}

LoggerBuffer::LoggerBuffer(KT_LoggerRollPtr roll, size_t buffer_len)
  : _roll(roll), _buffer(NULL), _buffer_len(buffer_len) {
  //设置get buffer, 无效, 不适用
  setg(NULL, NULL, NULL);

  //设置put buffer
  if (_roll) {
    //分配空间
    _buffer = new char[_buffer_len];
    setp(_buffer, _buffer + _buffer_len);
  } else {
    setp(NULL, NULL);
    _buffer_len = 0;
  }
}

LoggerBuffer::~LoggerBuffer() {
  sync();
  if (_buffer) {
    delete[] _buffer;
  }
}

streamsize LoggerBuffer::xsputn(const char_type *s, streamsize n) {
  if (!_roll) {
    return n;
  }

  return std::basic_streambuf<char>::xsputn(s, n);
}

void LoggerBuffer::reserve(std::streamsize n) {
  if (n <= _buffer_len) {
    return;
  }

  //不超过最大大小
  if (n > MAX_BUFFER_LENGTH) {
    n = MAX_BUFFER_LENGTH;
  }

  int64_t len = pptr() - pbase();
  char_type *p = new char_type[n];
  memcpy(p, _buffer, len);
  delete[] _buffer;
  _buffer = p;
  _buffer_len = n;

  setp(_buffer, _buffer + _buffer_len);

  pbump((int)len);

  return;
}

std::basic_streambuf<char>::int_type LoggerBuffer::overflow(std::basic_streambuf<char>::int_type c) {
  if (!_roll) {
    return 0;
  }

  if (_buffer_len >= MAX_BUFFER_LENGTH) {
    sync();
  } else {
    reserve(_buffer_len * 2);
  }

  if (std::char_traits<char_type>::eq_int_type(c, std::char_traits<char_type>::eof())) {
    return std::char_traits<char_type>::not_eof(c);
  } else {
    return sputc(c);
  }
  return 0;
}

int LoggerBuffer::sync() {
  //有数据
  if (pptr() > pbase()) {
    std::streamsize len = pptr() - pbase();

    if (_roll) {
      //具体的写逻辑
      _roll->write(make_pair(KT_Thread::CURRENT_THREADID(), string(pbase(), len)));
    }

    //重新设置put缓冲区, pptr()重置到pbase()处
    setp(pbase(), epptr());
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
//

}  // namespace kant
