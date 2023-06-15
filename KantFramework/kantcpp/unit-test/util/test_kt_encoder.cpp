//
// Created by jarod on 2020/2/20.
//

#include "util/kt_encoder.h"
#include "gtest/gtest.h"
#include "util/kt_common.h"

using namespace kant;

class UtilEncodeTest : public testing::Test {
 public:
  //添加日志
  static void SetUpTestCase() {}
  static void TearDownTestCase() {}
  virtual void SetUp()  //TEST跑之前会执行SetUp
  {}
  virtual void TearDown()  //TEST跑完之后会执行TearDown
  {}
};

TEST_F(UtilEncodeTest, encode)  //此时使用的是TEST_F宏
{
  string utf8 = "我们的祖国;";
  string gbk;

  gbk = KT_Encoder::utf82gbk(utf8);  //, KT_Encoder::ICONV_NORMAL);

  string tmpUtf8 = KT_Encoder::gbk2utf8(gbk);  //, KT_Encoder::ICONV_NORMAL);

  ASSERT_TRUE(utf8 == tmpUtf8);
}
