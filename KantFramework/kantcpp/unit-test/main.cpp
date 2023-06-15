#include <cassert>
#include <iostream>
#include <vector>
#include "util/kt_common.h"
#include "util/kt_logger.h"
#include "gtest/gtest.h"

using namespace std;
using namespace kant;

int main(int argc, char** argv) {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  kant::KT_Common::ignorePipe();
#endif

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}