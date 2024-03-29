﻿
#include "util/kt_spin_lock.h"
#include "util/kt_common.h"

#include <thread>
#include <iostream>
#include <cassert>
using namespace std;

#define TRYS_COUNT 10
#define TRYS_SLEEP 1
namespace kant {

KT_SpinLock::KT_SpinLock() { _flag.clear(std::memory_order_release); }

KT_SpinLock::~KT_SpinLock() {}

void KT_SpinLock::lock() const {
  for (size_t i = 1; _flag.test_and_set(std::memory_order_acquire); i++) {
    if (i % TRYS_COUNT == 0) {
      KT_Common::msleep(TRYS_SLEEP);
    } else {
      std::this_thread::yield();
    }
  }
}

void KT_SpinLock::unlock() const { _flag.clear(std::memory_order_release); }

bool KT_SpinLock::tryLock() const {
  int trys = TRYS_COUNT;
  for (; trys > 0 && _flag.test_and_set(std::memory_order_acquire); --trys) {
    std::this_thread::yield();
  }

  if (trys > 0) return true;

  return false;
}

}  // namespace kant
