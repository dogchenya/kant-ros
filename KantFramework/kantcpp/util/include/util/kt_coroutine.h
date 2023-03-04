#ifndef _KT_COROUTINES_H_
#define _KT_COROUTINES_H_

#include <cstddef>
#include <list>
#include <set>
#include <deque>
#include <map>
#include <functional>

#include "util/kt_fcontext.h"
#include "util/kt_thread_queue.h"
#include "util/kt_monitor.h"
#include "util/kt_thread.h"
//#include "util/tc_epoller.h"

using namespace std;
namespace kant {

//Э���쳣��
struct KT_CoroutineException : public KT_Exception {
  KT_CoroutineException(const string &buffer) : KT_Exception(buffer) {}
  KT_CoroutineException(const string &buffer, int err) : KT_Exception(buffer, err) {}
  ~KT_CoroutineException() throw() {}
};

/////////////////////////////////////////////
/**
 * Э��ʹ�õ�ջ������Ϣ
 */
struct stack_context {
  std::size_t size;
  void *sp;

  stack_context() : size(0), sp(0) {}
};

/////////////////////////////////////////////
/**
 * Э��ʹ�õ�ջ������
 */
struct stack_traits {
  static bool is_unbounded();

  static std::size_t page_size();

  static std::size_t default_size();

  static std::size_t minimum_size();

  static std::size_t maximum_size();

  static stack_context allocate(std::size_t);

  static void deallocate(stack_context &);
};

class KT_CoroutineScheduler;
///////////////////////////////////////////
/**
 * Э����Ϣ��
 * ������Э�̵Ļ�����Ϣ, ����Э�����������ʽ��֯��һ��
 */
class KT_CoroutineInfo {
 public:
  /**
    * Э�̵�״̬��Ϣ
    */
  enum CORO_STATUS {
    CORO_FREE = 0,
    CORO_ACTIVE = 1,
    CORO_AVAIL = 2,
    CORO_INACTIVE = 3,
    CORO_TIMEOUT = 4,
  };

  /////////////////////////////////////////////
  /*
   * Э���ڲ�ʹ�õĺ���
   */
  struct CoroutineFunc {
    std::function<void(void *, transfer_t)> coroFunc;
    void *args;
  };

  /**
     * �����ʼ��
     */
  static inline void CoroutineHeadInit(KT_CoroutineInfo *coro) {
    coro->_next = coro;
    coro->_prev = coro;
  }

  /**
     * �����Ƿ�Ϊ��
     */
  static inline bool CoroutineHeadEmpty(KT_CoroutineInfo *coro_head) { return coro_head->_next == coro_head; }

  /**
     * ����
     */
  static inline void __CoroutineAdd(KT_CoroutineInfo *coro, KT_CoroutineInfo *prev, KT_CoroutineInfo *next) {
    next->_prev = coro;
    coro->_next = next;
    coro->_prev = prev;
    prev->_next = coro;
  }

  /**
     * ����ͷ��
     */
  static inline void CoroutineAdd(KT_CoroutineInfo *new_coro, KT_CoroutineInfo *coro_head) {
    __CoroutineAdd(new_coro, coro_head, coro_head->_next);
  }

  /**
     * ����β��
     */
  static inline void CoroutineAddTail(KT_CoroutineInfo *new_coro, KT_CoroutineInfo *coro_head) {
    __CoroutineAdd(new_coro, coro_head->_prev, coro_head);
  }

  /**
     * ɾ��
     */
  static inline void __CoroutineDel(KT_CoroutineInfo *prev, KT_CoroutineInfo *next) {
    next->_prev = prev;
    prev->_next = next;
  }

  /**
     * ɾ��
     */
  static inline void CoroutineDel(KT_CoroutineInfo *coro) {
    __CoroutineDel(coro->_prev, coro->_next);
    coro->_next = NULL;
    coro->_prev = NULL;
  }

  /**
     * ��һ�������ƶ�������һ������ͷ��
     */
  static inline void CoroutineMove(KT_CoroutineInfo *coro, KT_CoroutineInfo *coro_head) {
    CoroutineDel(coro);
    CoroutineAdd(coro, coro_head);
  }

  /**
     * ��һ�������ƶ�������һ������β��
     */
  static inline void CoroutineMoveTail(KT_CoroutineInfo *coro, KT_CoroutineInfo *coro_head) {
    CoroutineDel(coro);
    CoroutineAddTail(coro, coro_head);
  }

 protected:
  //Э�̵���ں���
  static void corotineEntry(transfer_t q);

  //��Э����ִ��ʵ���߼�����ں���
  static void corotineProc(void *args, transfer_t t);

 public:
  /**
     * ���캯��
     */
  KT_CoroutineInfo();

  /**
     * ���캯��
     */
  KT_CoroutineInfo(KT_CoroutineScheduler *scheduler, uint32_t iUid, stack_context stack_ctx);

  /**
     * ��������
     */
  ~KT_CoroutineInfo();

  /**
     * ע��Э��ʵ�ʵĴ�����
     */
  void registerFunc(const std::function<void()> &callback);

  /**
     * ����Э�̵��ڴ�ռ�
     */
  void setStackContext(stack_context stack_ctx);

  /**
     * ��ȡЭ�̵��ڴ�ռ�
     */
  inline stack_context &getStackContext() { return _stack_ctx; }

  /**
     * ��ȡЭ�������ĵ�����
     */
  inline KT_CoroutineScheduler *getScheduler() { return _scheduler; }

  /**
     * ��ȡЭ�̵ı�־
     */
  inline uint32_t getUid() const { return _uid; }

  /**
     * ����Э�̵ı�־
     */
  inline void setUid(uint32_t iUid) { _uid = iUid; }

  /**
     * ��ȡЭ�̵�״̬
     */
  inline CORO_STATUS getStatus() const { return _eStatus; }

  /**
     * ����Э�̵�״̬
     */
  inline void setStatus(CORO_STATUS status) { _eStatus = status; }

  /**
     * ��ȡЭ��������������
     */
  inline fcontext_t getCtx() const { return _ctx; }
  inline void setCtx(fcontext_t ctx) { _ctx = ctx; }

 public:
  /*
   * ˫������ָ��
   */
  KT_CoroutineInfo *_prev;
  KT_CoroutineInfo *_next;

 private:
  /*
     * Э�������ĵ�����
     */
  KT_CoroutineScheduler *_scheduler;

  /*I
     * Э�̵ı�ʶ
     */
  uint32_t _uid;

  /*
     * Э�̵�״̬
     */
  CORO_STATUS _eStatus;

  /*
     * Э�̵��ڴ�ռ�
     */
  stack_context _stack_ctx;

  /*
     * ����Э�̺�Э�����ڵ�������
     */
  fcontext_t _ctx = NULL;

  /*
     * Э�̳�ʼ��������ں���
     */
  CoroutineFunc _init_func;

  /*
     * Э�̾���ִ�к���
     */
  std::function<void()> _callback;
};

///////////////////////////////////////////
/**
 * Э�̵�����
 */
class KT_CoroutineScheduler {
 protected:
  static thread_local shared_ptr<KT_CoroutineScheduler> g_scheduler;

 public:
  /**
     * ���û��, �򴴽�(�߳�˽�б���, ÿ���߳���һ��)
     */
  static const shared_ptr<KT_CoroutineScheduler> &create();

  /**
     * ��ȡscheduler, û���򷵻�null, (�߳�˽�б���, ÿ���߳���һ��)
     */
  static const shared_ptr<KT_CoroutineScheduler> &scheduler();

  /**
     * �ͷ�Э�̵�����
     */
  static void reset();

  /**
     * ���캯��(ÿ���߳������һ��)
     */
  KT_CoroutineScheduler();

  /**
     * ��������
     */
  ~KT_CoroutineScheduler();

  /**
     * ��ʼ��Э�̳صĴ�С���Լ�Э�̵Ķ�ջ��С
     */
  void setPoolStackSize(uint32_t iPoolSize, size_t iStackSize);

  /**
     * ����Э��
     */
  uint32_t go(const std::function<void()> &callback);

  /**
     * ֪ͨѭ���ѹ���
     */
  void notify();

  /**
     * ����Э�̵���(û�л�ԾЭ�̻�����, ������epoll��)
     */
  void run();

  /**
	 * �Ѿ�����������
	 * @return
	 */
  bool isReady() const { return _ready; }

  /**
     * ��ǰЭ�̷�������ִ��
     * @param bFlag: true, ���Զ�����(�ȵ��´�Э�̵���, �����ټ��ǰ�߳�), false: �����Զ�����, �����Լ����ȸ�Э��(����put����������)
     */
  void yield(bool bFlag = true);

  /**
     * ��ǰЭ������iSleepTimeʱ��(��λ:����)��Ȼ��ᱻ���Ѽ���ִ��
     */
  void sleep(int millseconds);

  /**
     * ������Ҫ���ѵ�Э��, ��Э�̷��뵽��������, ���ϻᱻ����������
     */
  void put(uint32_t iCoroId);

  /**
     * Э���л�
     */
  void switchCoro(KT_CoroutineInfo *to);

  /**
     * ֹͣ
     */
  void terminate();

  /**
     * ��Դ����
     */
  void destroy();

  /**
     * Э�̵����Ƿ��Ѿ�����
     * @return
     */
  bool isTerminate() const {
    //return _epoller->isTerminate();
  }

  /**
     * Э���Ƿ�������
     * @return
     */
  bool full();

  /**
     * ��ȡ����Э����Ŀ
     */
  inline uint32_t getPoolSize() { return _poolSize; }

  /**
     * ��ȡ��ǰ�Ѿ�������Э����Ŀ
     */
  inline uint32_t getCurrentSize() { return _currentSize; }

  /**
     * ��ȡ������Ӧ������Э����Ŀ
     */
  inline size_t getResponseCoroSize() { return _activeCoroQueue.size(); }

  /**
     * ��ȡ�����Ͽ��е�Э����Ŀ
     */
  inline uint32_t getFreeSize() { return _poolSize - _usedSize; }

  /**
     * ��������ʹ�õ�Э����Ŀ
     */
  inline void decUsedSize() { --_usedSize; }

  /**
     * ��������ʹ�õ�Э����Ŀ
     */
  inline void incUsedSize() { ++_usedSize; }

  /**
     * �Ƿ�����Э����
     */
  inline bool isMainCoroutine() { return _currentCoro->getUid() == 0; }

  /**
     * �������е���Э��
     */
  inline KT_CoroutineInfo &getMainCoroutine() { return _mainCoro; }

  /**
     * ������Э��
     */
  inline void setMainCtx(fcontext_t ctx) { _mainCoro.setCtx(ctx); }

  /**
     * ��ǰЭ�̵ı�ʶId
     */
  inline uint32_t getCoroutineId() { return _currentCoro->getUid(); }

  /**
     * ���õ�ǰ����Э��ִ�����ʱ�Ļص�
     */
  inline void setNoCoroutineCallback(std::function<void(KT_CoroutineScheduler *)> noCoroutineCallback) {
    _noCoroutineCallback = noCoroutineCallback;
  }

  friend class KT_CoroutineInfo;

 protected:
  /**
	 * ��ʼ��
	 */
  void init();

  /**
	 * �ͷ�����Э����Դ
	 */
  void createCoroutineInfo(size_t poolSize);

  /**
     * ����Э��id
     */
  uint32_t generateId();

  /**
     * ����Э�̳صĴ�С
     */
  int increaseCoroPoolSize();

  /**
     * ������Ҫ���е�Э��
     */
  void wakeup();

  /**
     * �����Լ��������е�Э��
     */
  void wakeupbyself();

  /**
     * �������ߵ�Э��
     */
  void wakeupbytimeout();

  /**
     * �ŵ�active��Э��������
     */
  void moveToActive(KT_CoroutineInfo *coro);

  /**
     * �ŵ�avail��Э��������
     */
  void moveToAvail(KT_CoroutineInfo *coro);

  /**
     * �ŵ�inactive��Э��������
     */
  void moveToInactive(KT_CoroutineInfo *coro);

  /**
     * �ŵ���ʱ�ȴ���Э��������
     */
  void moveToTimeout(KT_CoroutineInfo *coro);

  /**
     * �ŵ����е�Э��������
     */
  void moveToFreeList(KT_CoroutineInfo *coro);

 private:
  /*
     * Э�̳صĴ�С
     */
  uint32_t _poolSize = 1000;

  /*
     * Э�̵�ջ�ռ��С
     */
  size_t _stackSize = 128 * 1024;

  /*
     * ��ǰ�Ѿ�������Э����
     */
  uint32_t _currentSize;

  /*
     * ����ʹ�õ�Э����
     */
  uint32_t _usedSize;

  /*
     * ����Э��Id�ı���
     */
  uint32_t _uniqId;

  /*
     * ��Э��
     */
  KT_CoroutineInfo _mainCoro;

  /*
     * ��ǰ���е�Э��
     */
  KT_CoroutineInfo *_currentCoro;

  /*
     * �������Э�̵�����ָ��
     */
  KT_CoroutineInfo **_all_coro = NULL;

  /*
     * ��Ծ��Э������
     */
  KT_CoroutineInfo _active;

  /*
     * ���õ�Э������
     */
  KT_CoroutineInfo _avail;

  /*
     * ����Ծ��Э������
     */
  KT_CoroutineInfo _inactive;

  /*
     * ��ʱ��Э������
     */
  KT_CoroutineInfo _timeout;

  /*
     * ���е�Э������
     */
  KT_CoroutineInfo _free;

  /*
     * ��Ҫ�����Э�̶��У������߳�ʹ�ã���������ȴ������Э��
     */
  deque<uint32_t> _activeCoroQueue;

  /*
	 * ��Ҫ�����Э�̶��У����߳�ʹ��
	 */
  list<uint32_t> _needActiveCoroId;

  /*
     * ��ų�ʱ��Э��
     */
  multimap<int64_t, uint32_t> _timeoutCoroId;

  /**
     * epoller
     */
  //KT_Epoller *_epoller = NULL;

  /**
     * ��Э�̶�������Ϻ�Ļص�
     */
  std::function<void(KT_CoroutineScheduler *)> _noCoroutineCallback;

  /**
     * �Ƿ�����������
     */
  bool _ready = false;
};

/**
 * ���߳̽��а�װ��Э���࣬��Ҫ�������Լ�����߳���ʹ��Э��,
 * ʹ�÷�ʽ:
 * 1 ҵ����Լ̳������
 * 2 ʵ��handleCoroutine����(Э�̾���ִ�д���), ������������������������������Э��
 * 3 ����start����, �����߳�, ͬʱ�ᴴ��iNum��Э��, ��������������iPoolSize��Э��ͬʱ����
 * 4 terminate����
 */
class KT_Coroutine : public KT_Thread {
 public:
  /**
     * ���캯��
     */
  KT_Coroutine();

  /**
     * ��������
     */
  virtual ~KT_Coroutine();

  /**
     * ��ʼ��
     * @iNum, ��ʾͬʱ���������ٸ�Э�̣������ж��ٸ�coroFunc���е�Э��
     * @iPoolSize����ʾ����̵߳�������������Э�̸���
     * @iStackSize��Э�̵�ջ��С
     */
  void setCoroInfo(uint32_t iNum, uint32_t iPoolSize, size_t iStackSize);

  /**
     * ����Э�̣����Ѿ�������Э����ʹ��
     * ����ֵΪЭ�̵�id������0����ʾ�ɹ�����С�ڵ���0����ʾʧ��
     */
  uint32_t go(const std::function<void()> &coroFunc);

  /**
     * ��ǰЭ���Լ�����ִ��,���Զ�������������
     * ���Ѿ�������Э����ʹ��
     */
  void yield();

  /**
     * ��ǰЭ������iSleepTimeʱ��(��λ:����)��ʱ�䵽�ˣ����Զ�������������
     * ���Ѿ�������Э����ʹ��
     */
  void sleep(int millseconds);

  /**
     * ��ȡ���õ����Э�̵���Ŀ
     */
  uint32_t getMaxCoroNum() { return _maxNum; }

  /**
     * ��ȡ����ʱ�����õ�Э�̵���Ŀ
     */
  uint32_t getCoroNum() { return _num; }

  /**
     * ����Э�̵�ջ��С
     */
  size_t getCoroStackSize() { return _stackSize; }

  /**
     * ֹͣ
     */
  void terminate();

 protected:
  /**
     * �̴߳�����
     */
  virtual void run();

  /**
     *  ��̬����, Э�����. 
     */
  static void coroEntry(KT_Coroutine *pCoro);

  /**
     * Э�����еĺ���������_num����Ŀ��������_num���������
     */
  virtual void handle() = 0;

 protected:
  /**
     * �߳��Ѿ�����, ����Э�̴���ǰ����
     */
  virtual void initialize() {}

  /**
     * ����Э��ֹͣ����֮���߳��˳�֮ǰʱ����
     */
  virtual void destroy() {}

  /**
     * ����Ĵ����߼�
     */
  virtual void handleCoro();

 protected:
  shared_ptr<KT_CoroutineScheduler> _coroSched;
  uint32_t _num;
  uint32_t _maxNum;
  size_t _stackSize;
};

}  // namespace kant

#endif _KT_COROUTINES_H_
