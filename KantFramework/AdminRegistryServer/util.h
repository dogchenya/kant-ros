#ifndef __UTIL_H_
#define __UTIL_H_

#include "util/kt_common.h"
#include "servant/RemoteLogger.h"

#define FILE_FUN __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "|"
#define FILE_FUN_STR \
  KT_Common::tostr(__FILE__) + ":" + KT_Common::tostr(__FUNCTION__) + ":" + KT_Common::tostr(__LINE__) + "|"

#endif
