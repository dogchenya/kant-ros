
#ifndef _KT_SPIN_LOCK_H_
#define _KT_SPIN_LOCK_H_

#include "util/kt_platform.h"
#include <atomic>
#include <memory>

using namespace std;

namespace kant {

/**
 * 自旋锁
 * 不能阻塞wait, 只能快速加解锁, 适用于锁粒度非常小的情况, 减小线程切换的开销
 * 不支持trylock
 */
class UTIL_DLL_API KT_SpinLock {
 public:
  KT_SpinLock();
  virtual ~KT_SpinLock();

  void lock() const;
  bool tryLock() const;
  void unlock() const;

 private:
  KT_SpinLock(const KT_SpinLock&) = delete;
  KT_SpinLock(KT_SpinLock&&) = delete;
  KT_SpinLock& operator=(const KT_SpinLock&) = delete;
  KT_SpinLock& operator=(KT_SpinLock&&) = delete;

 private:
  mutable std::atomic_flag _flag;
};

}  // namespace kant
#endif
