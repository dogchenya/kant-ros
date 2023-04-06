#include "util/kt_coroutine.h"
#include "util/kt_platform.h"
#include "util/kt_logger.h"

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <assert.h>
#include "util/kt_timeprovider.h"

namespace kant {

#if TARGET_PLATFORM_WINDOWS

// x86_64
// test x86_64 before i386 because icc might
// define __i686__ for x86_64 too
#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_M_X64) || \
  defined(_M_AMD64)

// Windows seams not to provide a constant or function
// telling the minimal stacksize
#define MIN_STACKSIZE 8 * 1024
#else
#define MIN_STACKSIZE 4 * 1024
#endif

void system_info_(SYSTEM_INFO *si) { ::GetSystemInfo(si); }

SYSTEM_INFO system_info() {
  static SYSTEM_INFO si;
  static std::once_flag flag;
  std::call_once(flag, static_cast<void (*)(SYSTEM_INFO *)>(system_info_), &si);
  return si;
}

std::size_t pagesize() { return static_cast<std::size_t>(system_info().dwPageSize); }

// Windows seams not to provide a limit for the stacksize
// libcoco uses 32k+4k bytes as minimum
bool stack_traits::is_unbounded() { return true; }

std::size_t stack_traits::page_size() { return pagesize(); }

std::size_t stack_traits::default_size() { return 128 * 1024; }
// because Windows seams not to provide a limit for minimum stacksize
std::size_t stack_traits::minimum_size() { return MIN_STACKSIZE; }

// because Windows seams not to provide a limit for maximum stacksize
// maximum_size() can never be called (pre-condition ! is_unbounded() )
std::size_t stack_traits::maximum_size() {
  assert(!is_unbounded());
  return 1 * 1024 * 1024 * 1024;  // 1GB
}

stack_context stack_traits::allocate(size_t size_) {
  // calculate how many pages are required
  const std::size_t pages(static_cast<std::size_t>(std::ceil(static_cast<float>(size_) / stack_traits::page_size())));
  // add one page at bottom that will be used as guard-page
  const std::size_t size__ = (pages + 1) * stack_traits::page_size();

  void *vp = ::VirtualAlloc(0, size__, MEM_COMMIT, PAGE_READWRITE);
  if (!vp) throw std::bad_alloc();

  DWORD old_options;
  const BOOL result =
    ::VirtualProtect(vp, stack_traits::page_size(), PAGE_READWRITE | PAGE_GUARD /*PAGE_NOACCESS*/, &old_options);
  assert(FALSE != result);

  stack_context sctx;
  sctx.size = size__;
  sctx.sp = static_cast<char *>(vp) + sctx.size;
  return sctx;
}

void stack_traits::deallocate(stack_context &sctx) {
  assert(sctx.sp);

  void *vp = static_cast<char *>(sctx.sp) - sctx.size;
  ::VirtualFree(vp, 0, MEM_RELEASE);
}

#else

// 128kb recommended stack size
// # define MINSIGSTKSZ (131072)

void pagesize_(std::size_t *size) {
  // conform to POSIX.1-2001
  *size = ::sysconf(_SC_PAGESIZE);
}

void stacksize_limit_(rlimit *limit) {
  // conforming to POSIX.1-2001
  ::getrlimit(RLIMIT_STACK, limit);
}

std::size_t pagesize() {
  static std::size_t size = 0;
  static std::once_flag flag;
  std::call_once(flag, pagesize_, &size);
  return size;
}

rlimit stacksize_limit() {
  static rlimit limit;
  static std::once_flag flag;
  std::call_once(flag, stacksize_limit_, &limit);
  return limit;
}

bool stack_traits::is_unbounded() { return RLIM_INFINITY == stacksize_limit().rlim_max; }

std::size_t stack_traits::page_size() { return pagesize(); }

std::size_t stack_traits::default_size() { return 128 * 1024; }

std::size_t stack_traits::minimum_size() { return MINSIGSTKSZ; }

std::size_t stack_traits::maximum_size() {
  assert(!is_unbounded());
  return static_cast<std::size_t>(stacksize_limit().rlim_max);
}

stack_context stack_traits::allocate(std::size_t size_) {
  // calculate how many pages are required
  const std::size_t pages(static_cast<std::size_t>(std::ceil(static_cast<float>(size_) / stack_traits::page_size())));
  // add one page at bottom that will be used as guard-page
  const std::size_t size__ = (pages + 1) * stack_traits::page_size();

  // conform to POSIX.4 (POSIX.1b-1993, _POSIX_C_SOURCE=199309L)
#if defined(MAP_ANON)
  void *vp = ::mmap(0, size__, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
#else
  void *vp = ::mmap(0, size__, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
  if (MAP_FAILED == vp) throw std::bad_alloc();

  // conforming to POSIX.1-2001
  const int result(::mprotect(vp, stack_traits::page_size(), PROT_NONE));
  assert(0 == result);

  stack_context sctx;
  sctx.size = size__;
  sctx.sp = static_cast<char *>(vp) + sctx.size;

  return sctx;
}

void stack_traits::deallocate(stack_context &sctx) {
  assert(sctx.sp);

  void *vp = static_cast<char *>(sctx.sp) - sctx.size;
  // conform to POSIX.4 (POSIX.1b-1993, _POSIX_C_SOURCE=199309L)
  ::munmap(vp, sctx.size);
}

#endif

KT_CoroutineInfo::KT_CoroutineInfo() : _prev(NULL), _next(NULL), _scheduler(NULL), _uid(0), _eStatus(CORO_FREE) {}

KT_CoroutineInfo::KT_CoroutineInfo(KT_CoroutineScheduler *scheduler, uint32_t iUid, stack_context stack_ctx)
  : _prev(NULL), _next(NULL), _scheduler(scheduler), _uid(iUid), _eStatus(CORO_FREE), _stack_ctx(stack_ctx) {}

KT_CoroutineInfo::~KT_CoroutineInfo() {}

void KT_CoroutineInfo::setStackContext(stack_context stack_ctx) { _stack_ctx = stack_ctx; }

void KT_CoroutineInfo::registerFunc(const std::function<void()> &callback) {
  _callback = callback;

  _init_func.coroFunc = KT_CoroutineInfo::corotineProc;

  _init_func.args = this;

  fcontext_t ctx = make_fcontext(_stack_ctx.sp, _stack_ctx.size, KT_CoroutineInfo::corotineEntry);

  transfer_t tf = jump_fcontext(ctx, this);

  //实际的ctx
  this->setCtx(tf.fctx);
}

void KT_CoroutineInfo::corotineEntry(transfer_t tf) {
  KT_CoroutineInfo *coro = static_cast<KT_CoroutineInfo *>(tf.data);

  auto func = coro->_init_func.coroFunc;
  void *args = coro->_init_func.args;

  transfer_t t = jump_fcontext(tf.fctx, NULL);

  //拿到自己的协程堆栈, 当前协程结束以后, 好跳转到main
  coro->_scheduler->setMainCtx(t.fctx);

  //再跳转到具体函数
  func(args, t);
}

void KT_CoroutineInfo::corotineProc(void *args, transfer_t t) {
  KT_CoroutineInfo *coro = (KT_CoroutineInfo *)args;

  try {
    //执行具体业务代码
    coro->_callback();
  } catch (std::exception &ex) {
    cerr << "KT_CoroutineInfo::corotineProc exception:" << ex.what() << endl;
  } catch (...) {
    cerr << "KT_CoroutineInfo::corotineProc unknown exception." << endl;
  }

  KT_CoroutineScheduler *scheduler = coro->getScheduler();
  scheduler->decUsedSize();
  scheduler->moveToFreeList(coro);

  //当前业务执行完, 会跳到main
  scheduler->switchCoro(&(scheduler->getMainCoroutine()));
}

///////////////////////////////////////////////////////////////////////////////////////////

thread_local shared_ptr<KT_CoroutineScheduler> KT_CoroutineScheduler::g_scheduler;

const shared_ptr<KT_CoroutineScheduler> &KT_CoroutineScheduler::create() {
  if (!g_scheduler) {
    g_scheduler = std::make_shared<KT_CoroutineScheduler>();
  }

  return g_scheduler;
}

const shared_ptr<KT_CoroutineScheduler> &KT_CoroutineScheduler::scheduler() { return g_scheduler; }

void KT_CoroutineScheduler::reset() { g_scheduler.reset(); }

KT_CoroutineScheduler::KT_CoroutineScheduler()
  : _currentSize(0), _usedSize(0), _uniqId(0), _currentCoro(NULL), _all_coro(NULL) {
  // LOG_CONSOLE_DEBUG << endl;

  _epoller = new KT_Epoller();

  _epoller->create(10240);
}

KT_CoroutineScheduler::~KT_CoroutineScheduler() {
  // LOG_CONSOLE_DEBUG << endl;
  if (_epoller) {
    delete _epoller;
    _epoller = NULL;
  }
}

void KT_CoroutineScheduler::createCoroutineInfo(size_t poolSize) {
  if (_all_coro != NULL) {
    delete[] _all_coro;
  }

  _all_coro = new KT_CoroutineInfo *[_poolSize + 1];
  for (size_t i = 0; i <= _poolSize; ++i) {
    //id=0不使用, 给mainCoro来使用!
    _all_coro[i] = NULL;
  }
}

void KT_CoroutineScheduler::setPoolStackSize(uint32_t iPoolSize, size_t iStackSize) {
  _poolSize = iPoolSize;
  _stackSize = iStackSize;
}

void KT_CoroutineScheduler::init() {
  _usedSize = 0;
  _uniqId = 0;

  if (_poolSize <= 100) {
    _currentSize = _poolSize;
  } else {
    _currentSize = 100;
  }

  createCoroutineInfo(_poolSize);

  KT_CoroutineInfo::CoroutineHeadInit(&_active);
  KT_CoroutineInfo::CoroutineHeadInit(&_avail);
  KT_CoroutineInfo::CoroutineHeadInit(&_inactive);
  KT_CoroutineInfo::CoroutineHeadInit(&_timeout);
  KT_CoroutineInfo::CoroutineHeadInit(&_free);

  int iSucc = 0;

  for (size_t i = 0; i < _currentSize; ++i) {
    //iId=0不使用, 给mainCoro使用!!!!
    uint32_t iId = generateId();

    assert(iId != 0);

    stack_context s_ctx = stack_traits::allocate(_stackSize);

    KT_CoroutineInfo *coro = new KT_CoroutineInfo(this, iId, s_ctx);

    _all_coro[iId] = coro;

    KT_CoroutineInfo::CoroutineAddTail(coro, &_free);

    ++iSucc;
  }

  _currentSize = iSucc;

  _mainCoro.setUid(0);
  _mainCoro.setStatus(KT_CoroutineInfo::CORO_FREE);

  _currentCoro = &_mainCoro;
}

int KT_CoroutineScheduler::increaseCoroPoolSize() {
  if (_poolSize <= _currentSize) return -1;

  int iInc = ((_poolSize - _currentSize) > 100) ? 100 : (_poolSize - _currentSize);

  for (int i = 0; i < iInc; ++i) {
    uint32_t iId = generateId();
    stack_context s_ctx = stack_traits::allocate(_stackSize);

    KT_CoroutineInfo *coro = new KT_CoroutineInfo(this, iId, s_ctx);

    _all_coro[iId] = coro;

    KT_CoroutineInfo::CoroutineAddTail(coro, &_free);
  }

  _currentSize += iInc;

  return 0;
}

uint32_t KT_CoroutineScheduler::go(const std::function<void()> &callback) {
  if (!_all_coro) {
    init();
  }

  if (_usedSize >= _currentSize || KT_CoroutineInfo::CoroutineHeadEmpty(&_free)) {
    int iRet = increaseCoroPoolSize();

    if (iRet != 0) return 0;
  }

  KT_CoroutineInfo *coro = _free._next;
  assert(coro != NULL);

  KT_CoroutineInfo::CoroutineDel(coro);

  _usedSize++;

  coro->setStatus(KT_CoroutineInfo::CORO_AVAIL);

  KT_CoroutineInfo::CoroutineAddTail(coro, &_avail);

  coro->registerFunc(callback);

  return coro->getUid();
}

bool KT_CoroutineScheduler::full() {
  if (_usedSize >= _currentSize || KT_CoroutineInfo::CoroutineHeadEmpty(&_free)) {
    if (_poolSize <= _currentSize) return true;
  }

  return false;
}

void KT_CoroutineScheduler::notify() {
  assert(_epoller);

  _epoller->notify();
}

void KT_CoroutineScheduler::run() {
  if (!_all_coro) {
    init();
  }

  _ready = true;

  while (!_epoller->isTerminate()) {
    if (_activeCoroQueue.empty() && KT_CoroutineInfo::CoroutineHeadEmpty(&_avail) &&
        KT_CoroutineInfo::CoroutineHeadEmpty(&_active)) {
      _epoller->done(1000);
    }

    //唤醒需要激活的协程
    wakeup();

    //唤醒sleep的协程
    wakeupbytimeout();

    //唤醒yield的协程
    wakeupbyself();

    int iLoop = 100;

    //执行active协程, 每次执行100个, 避免占满cpu
    while (iLoop > 0 && !KT_CoroutineInfo::CoroutineHeadEmpty(&_active)) {
      KT_CoroutineInfo *coro = _active._next;

      assert(coro != NULL);

      switchCoro(coro);

      --iLoop;
    }

    //检查yield的线程, 执行
    if (!KT_CoroutineInfo::CoroutineHeadEmpty(&_avail)) {
      KT_CoroutineInfo *coro = _avail._next;

      assert(coro != NULL);

      switchCoro(coro);
    }

    //没有任何可执行的写成了, 直接退出!
    if (_usedSize == 0 && _noCoroutineCallback) {
      _noCoroutineCallback(this);
    }
  }

  destroy();

  _ready = false;
}

void KT_CoroutineScheduler::yield(bool bFlag) {
  //主协程不允许yield
  if (_currentCoro->getUid() == 0) {
    return;
  }

  if (bFlag) {
    _needActiveCoroId.push_back(_currentCoro->getUid());
  }

  moveToInactive(_currentCoro);
  switchCoro(&_mainCoro);
}

void KT_CoroutineScheduler::sleep(int iSleepTime) {
  //主协程不允许sleep
  if (_currentCoro->getUid() == 0) return;

  int64_t iNow = TNOWMS;
  int64_t iTimeout = iNow + (iSleepTime >= 0 ? iSleepTime : -iSleepTime);

  _timeoutCoroId.insert(make_pair(iTimeout, _currentCoro->getUid()));

  moveToTimeout(_currentCoro);

  _epoller->postAtTime(iTimeout, []() {});

  switchCoro(&_mainCoro);
}

void KT_CoroutineScheduler::wakeupbyself() {
  if (!_needActiveCoroId.empty() && !_epoller->isTerminate()) {
    list<uint32_t>::iterator it = _needActiveCoroId.begin();
    while (it != _needActiveCoroId.end()) {
      KT_CoroutineInfo *coro = _all_coro[*it];

      assert(coro != NULL);

      moveToAvail(coro);

      ++it;
    }
    _needActiveCoroId.clear();
  }
}

void KT_CoroutineScheduler::put(uint32_t iCoroId) {
  if (!_epoller->isTerminate()) {
    _activeCoroQueue.push_back(iCoroId);

    _epoller->notify();
  }
}

void KT_CoroutineScheduler::wakeup() {
  if (!_activeCoroQueue.empty() && !_epoller->isTerminate()) {
    deque<uint32_t> coroIds;

    _activeCoroQueue.swap(coroIds);

    auto it = coroIds.begin();

    auto itEnd = coroIds.end();

    while (it != itEnd) {
      KT_CoroutineInfo *coro = _all_coro[*it];

      assert(coro != NULL);

      moveToActive(coro);

      ++it;
    }
  }
}

void KT_CoroutineScheduler::wakeupbytimeout() {
  if (!_timeoutCoroId.empty() && !_epoller->isTerminate()) {
    int64_t iNow = TNOWMS;
    while (true) {
      multimap<int64_t, uint32_t>::iterator it = _timeoutCoroId.begin();

      if (it == _timeoutCoroId.end() || it->first > iNow) break;

      KT_CoroutineInfo *coro = _all_coro[it->second];

      assert(coro != NULL);

      moveToActive(coro);

      _timeoutCoroId.erase(it);
    }
  }
}

void KT_CoroutineScheduler::terminate() {
  assert(_epoller);

  _epoller->terminate();
}

uint32_t KT_CoroutineScheduler::generateId() {
  uint32_t i = ++_uniqId;
  if (i == 0) {
    i = ++_uniqId;
  }

  assert(i <= _poolSize);

  return i;
}

void KT_CoroutineScheduler::switchCoro(KT_CoroutineInfo *to) {
  //跳转到to协程
  _currentCoro = to;

  transfer_t t = jump_fcontext(to->getCtx(), NULL);

  //并保存协程堆栈
  to->setCtx(t.fctx);
}

void KT_CoroutineScheduler::moveToActive(KT_CoroutineInfo *coro) {
  if (coro->getStatus() == KT_CoroutineInfo::CORO_INACTIVE || coro->getStatus() == KT_CoroutineInfo::CORO_TIMEOUT) {
    KT_CoroutineInfo::CoroutineDel(coro);
    coro->setStatus(KT_CoroutineInfo::CORO_ACTIVE);
    KT_CoroutineInfo::CoroutineAddTail(coro, &_active);
  } else {
    assert(false);
  }
}

void KT_CoroutineScheduler::moveToAvail(KT_CoroutineInfo *coro) {
  if (coro->getStatus() == KT_CoroutineInfo::CORO_INACTIVE) {
    KT_CoroutineInfo::CoroutineDel(coro);
    coro->setStatus(KT_CoroutineInfo::CORO_AVAIL);
    KT_CoroutineInfo::CoroutineAddTail(coro, &_avail);
  } else {
    assert(false);
  }
}

void KT_CoroutineScheduler::moveToInactive(KT_CoroutineInfo *coro) {
  if (coro->getStatus() == KT_CoroutineInfo::CORO_ACTIVE || coro->getStatus() == KT_CoroutineInfo::CORO_AVAIL) {
    KT_CoroutineInfo::CoroutineDel(coro);
    coro->setStatus(KT_CoroutineInfo::CORO_INACTIVE);
    KT_CoroutineInfo::CoroutineAddTail(coro, &_inactive);
  } else {
    assert(false);
  }
}

void KT_CoroutineScheduler::moveToTimeout(KT_CoroutineInfo *coro) {
  if (coro->getStatus() == KT_CoroutineInfo::CORO_ACTIVE || coro->getStatus() == KT_CoroutineInfo::CORO_AVAIL) {
    KT_CoroutineInfo::CoroutineDel(coro);
    coro->setStatus(KT_CoroutineInfo::CORO_TIMEOUT);
    KT_CoroutineInfo::CoroutineAddTail(coro, &_timeout);
  } else {
    assert(false);
  }
}

void KT_CoroutineScheduler::moveToFreeList(KT_CoroutineInfo *coro) {
  if (coro->getStatus() != KT_CoroutineInfo::CORO_FREE) {
    KT_CoroutineInfo::CoroutineDel(coro);
    coro->setStatus(KT_CoroutineInfo::CORO_FREE);
    KT_CoroutineInfo::CoroutineAddTail(coro, &_free);
  } else {
    assert(false);
  }
}

void KT_CoroutineScheduler::destroy() {
  if (_all_coro) {
    //id=0是保留不用的, 给mainCoro作为id用
    assert(_all_coro[0] == NULL);

    for (size_t i = 1; i <= _poolSize; i++) {
      if (_all_coro[i]) {
        stack_traits::deallocate(_all_coro[i]->getStackContext());
        delete _all_coro[i];
        _all_coro[i] = NULL;
      }
    }
    delete[] _all_coro;
    _all_coro = NULL;
  }
}

/////////////////////////////////////////////////////////
KT_Coroutine::KT_Coroutine() : _coroSched(NULL), _num(1), _maxNum(128), _stackSize(128 * 1024) {}

KT_Coroutine::~KT_Coroutine() {
  if (isAlive()) {
    terminate();

    getThreadControl().join();
  }
}

void KT_Coroutine::setCoroInfo(uint32_t iNum, uint32_t iMaxNum, size_t iStackSize) {
  _maxNum = (iMaxNum > 0 ? iMaxNum : 1);
  _num = (iNum > 0 ? (iNum <= _maxNum ? iNum : _maxNum) : 1);
  _stackSize = (iStackSize >= pagesize() ? iStackSize : pagesize());
}

void KT_Coroutine::run() {
  _coroSched = KT_CoroutineScheduler::create();

  initialize();

  handleCoro();

  destroy();
}

void KT_Coroutine::terminate() {
  if (_coroSched) {
    _coroSched->terminate();
  }
}

void KT_Coroutine::handleCoro() {
  _coroSched->setPoolStackSize(_maxNum, _stackSize);

  _coroSched->setNoCoroutineCallback([&](KT_CoroutineScheduler *scheduler) { scheduler->terminate(); });

  //把协程创建出来
  for (uint32_t i = 0; i < _num; ++i) {
    _coroSched->go(std::bind(&KT_Coroutine::coroEntry, this));
  }

  _coroSched->run();
}

void KT_Coroutine::coroEntry(KT_Coroutine *pCoro) { pCoro->handle(); }

uint32_t KT_Coroutine::go(const std::function<void()> &coroFunc) { return _coroSched->go(coroFunc); }

void KT_Coroutine::yield() { _coroSched->yield(); }

void KT_Coroutine::sleep(int millseconds) { _coroSched->sleep(millseconds); }
}  // namespace kant