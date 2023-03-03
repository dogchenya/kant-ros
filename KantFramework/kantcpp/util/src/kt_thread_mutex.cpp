#include "util/kt_thread_mutex.h"
#include "util/kt_common.h"
#include <string.h>
#include <iostream>
#include <cassert>

namespace kant {

KT_ThreadMutex::KT_ThreadMutex() {}

KT_ThreadMutex::~KT_ThreadMutex() {}

void KT_ThreadMutex::lock() const { _mutex.lock(); }

bool KT_ThreadMutex::tryLock() const { return _mutex.try_lock(); }

void KT_ThreadMutex::unlock() const { _mutex.unlock(); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
KT_ThreadRecMutex::KT_ThreadRecMutex() {}

KT_ThreadRecMutex::~KT_ThreadRecMutex() {}

void KT_ThreadRecMutex::lock() const { _mutex.lock(); }

void KT_ThreadRecMutex::unlock() const { _mutex.unlock(); }

bool KT_ThreadRecMutex::tryLock() const { return _mutex.try_lock(); }

}  // namespace kant
