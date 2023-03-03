#include "util/kt_thread_cond.h"
#include "util/kt_timeprovider.h"
#include <string.h>
#include <cassert>
#include <iostream>

using namespace std;

namespace kant {

KT_ThreadCond::KT_ThreadCond() {}

KT_ThreadCond::~KT_ThreadCond() {}

void KT_ThreadCond::signal() { _cond.notify_one(); }

void KT_ThreadCond::broadcast() { _cond.notify_all(); }

}  // namespace kant
