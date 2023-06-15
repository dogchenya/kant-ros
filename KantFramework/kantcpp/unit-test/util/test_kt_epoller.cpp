#include "util/kt_epoller.h"
#include "util/kt_common.h"
#include "util/kt_logger.h"
#include <thread>
#include <iostream>
#include <cmath>
#include "gtest/gtest.h"

using namespace kant;

class UtilEpollTest : public testing::Test {
 public:
  //添加日志
  static void SetUpTestCase() {
    //        cout<<"SetUpTestCase"<<endl;
  }
  static void TearDownTestCase() {
    //        cout<<"TearDownTestCase"<<endl;
  }
  virtual void SetUp()  //TEST跑之前会执行SetUp
  {
    //        cout<<"SetUp"<<endl;
  }
  virtual void TearDown()  //TEST跑完之后会执行TearDown
  {
    //        cout<<"TearDown"<<endl;
  }
};

TEST_F(UtilEpollTest, OUTEvents) {
  KT_Epoller epoller;

  epoller.create(1024);

  //用udp句柄, 方便唤醒, tcp句柄还得构建连接后才能唤醒
  KT_Socket fd;
  fd.createSocket(SOCK_DGRAM, AF_INET);

  shared_ptr<KT_Epoller::EpollInfo> epollInfo = epoller.createEpollInfo(fd.getfd());

  int out = 0;

  map<uint32_t, KT_Epoller::EpollInfo::EVENT_CALLBACK> callbacks;

  callbacks[EPOLLOUT] = [&](const shared_ptr<KT_Epoller::EpollInfo>& epollInfo) {
    ++out;
    return true;
  };

  epollInfo->registerCallback(callbacks, EPOLLIN | EPOLLOUT);

  std::thread start([&] {
    KT_Common::msleep(5);

    int i = 100;
    while (i--) {
      int l = out;
      epollInfo->mod(EPOLLOUT);
      KT_Common::msleep(5);
      ASSERT_TRUE(out - l == 1);
    }

    KT_Common::msleep(5);

    epoller.terminate();
  });

  epoller.loop(1000);

  //	delete epollInfo;

  start.join();
}

TEST_F(UtilEpollTest, close) {
  KT_Epoller epoller;

  epoller.create(1024);

  //用udp句柄, 方便唤醒, tcp句柄还得构建连接后才能唤醒
  KT_Socket fd;
  fd.createSocket(SOCK_DGRAM, AF_INET);

  bool c = false;

  {
    shared_ptr<KT_Epoller::EpollInfo> epollInfo = epoller.createEpollInfo(fd.getfd());

    map<uint32_t, KT_Epoller::EpollInfo::EVENT_CALLBACK> callbacks;

    callbacks[EPOLLOUT] = [&](const shared_ptr<KT_Epoller::EpollInfo>& epollInfo) { return false; };
    epollInfo->registerCallback(callbacks, 0);

    epollInfo->cookie(&c, [&](void* p) {
      ASSERT_TRUE(c == false);
      c = true;

      ASSERT_TRUE(&c == p);
    });

    std::thread start([&] {
      KT_Common::msleep(20);

      epollInfo->add(EPOLLOUT);

      KT_Common::msleep(100);

      epoller.terminate();
    });

    epoller.loop(1000);

    start.join();

    //		LOG_CONSOLE_DEBUG << epollInfo.use_count() << endl;
    epollInfo.reset();
  }

  ASSERT_TRUE(c);
}

TEST_F(UtilEpollTest, notify) {
  KT_Epoller epoller;

  epoller.create(1024);

  //用udp句柄, 方便唤醒, tcp句柄还得构建连接后才能唤醒
  KT_Socket fd;
  fd.createSocket(SOCK_DGRAM, AF_INET);

  bool idle = false;

  epoller.idle([&] { idle = true; });

  std::thread start([&] {
    KT_Common::msleep(5);

    epoller.notify();

    KT_Common::msleep(5);

    ASSERT_TRUE(idle);

    epoller.terminate();
  });

  epoller.loop(10000);

  start.join();
}

class EpollClass {
 public:
  ~EpollClass() {}
  void test() { _data.push_back(std::make_pair(TNOWMS, 0)); }

  vector<pair<int64_t, int64_t>> _data;
};

TEST_F(UtilEpollTest, testEpollTimer) {
  KT_Epoller epoller;

  epoller.create(1024);

  shared_ptr<EpollClass> tPtr = std::make_shared<EpollClass>();

  std::thread start([&] {
    epoller.postRepeated(100, false, std::bind(&EpollClass::test, tPtr.get()));
    KT_Common::msleep(1120);
    epoller.terminate();

    ASSERT_TRUE(tPtr->_data.size() >= 10);
    ASSERT_TRUE(tPtr->_data.size() <= 11);

    for (int i = 0; i < 9; i++) {
      //注意毫秒的精度问题
      ASSERT_TRUE(fabs(tPtr->_data[i + 1].first - tPtr->_data[i].first) <= 120);
    }
  });

  epoller.loop(200);
  start.join();
}

TEST_F(UtilEpollTest, testDelayTask) {
  KT_Timer timer;
  timer.startTimer(1);
  shared_ptr<EpollClass> tPtr = std::make_shared<EpollClass>();

  timer.postDelayed(50, std::bind(&EpollClass::test, tPtr.get()));
  timer.postDelayed(100, std::bind(&EpollClass::test, tPtr.get()));
  timer.postDelayed(200, std::bind(&EpollClass::test, tPtr.get()));
  KT_Common::msleep(306);
  ASSERT_TRUE(tPtr->_data.size() == 3);

  timer.stopTimer();
}

TEST_F(UtilEpollTest, testSharePtr) {
  KT_Timer timer;
  timer.startTimer(1);
  shared_ptr<EpollClass> tPtr = std::make_shared<EpollClass>();

  timer.postDelayed(50, std::bind(&EpollClass::test, tPtr));
  KT_Common::msleep(306);

  LOG_CONSOLE_DEBUG << tPtr->_data.size() << endl;
  ASSERT_TRUE(tPtr->_data.size() == 1);

  timer.stopTimer();
}

TEST_F(UtilEpollTest, testEpollSyncCallback) {
  KT_Epoller epoller;

  epoller.create(1024);

  shared_ptr<EpollClass> tPtr = std::make_shared<EpollClass>();

  std::thread start([&] {
    KT_Common::msleep(100);

    epoller.syncCallback([&]() { tPtr->test(); });

    ASSERT_TRUE(tPtr->_data.size() == 1);

    epoller.terminate();
  });

  epoller.loop(200);

  start.join();
}

TEST_F(UtilEpollTest, testEpollAsyncCallback) {
  KT_Epoller epoller;

  epoller.create(1024);

  shared_ptr<EpollClass> tPtr = std::make_shared<EpollClass>();

  std::thread start([&] {
    KT_Common::msleep(100);

    epoller.asyncCallback([&]() { tPtr->test(); });

    KT_Common::msleep(100);

    epoller.terminate();

    ASSERT_TRUE(tPtr->_data.size() == 1);
  });

  epoller.loop(200);

  start.join();
}