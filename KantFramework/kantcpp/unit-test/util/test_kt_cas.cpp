#include "util/kt_common.h"
#include "util/kt_cas_queue.h"
#include "util/kt_thread.h"
//#include "util/kt_autoptr.h"
#include "util/kt_spin_lock.h"
#include "util/kt_thread_queue.h"
#include "gtest/gtest.h"

#include <mutex>
#include <iostream>

using namespace kant;

class UtilCasTest : public testing::Test {
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

TEST_F(UtilCasTest, casLock) {
  size_t i = 0;
  size_t count = 1000000;

  KT_SpinLock mutex;
  std::thread add([&] {
    int j = count;
    while (j--) {
      KT_LockT<KT_SpinLock> lock(mutex);
      ++i;
    }
  });

  std::thread del([&] {
    int j = count;
    while (j--) {
      KT_LockT<KT_SpinLock> lock(mutex);
      --i;
    }
  });

  add.join();
  del.join();
  ASSERT_TRUE(i == 0);
}

bool g_terminate = false;
KT_CasQueue<int64_t> data_cas;
KT_ThreadQueue<int64_t> data_queue;

class WriteThread : public KT_Thread {
 protected:
  virtual void run() {
    for (int64_t i = 0; i < 100000; i++) {
      data_cas.push_back(i);
    }
  }
};
using WriteThreadPtr = std::shared_ptr<WriteThread>;

class ReadThread : public KT_Thread {
 public:
  ReadThread() {}

  virtual void run() {
    int64_t last = -1;
    while (!g_terminate) {
      int64_t i;
      if (data_cas.pop_front(i)) {
        ASSERT_TRUE(i - last == 1);
        last = i;

        if (i == 100000 - 1) {
          g_terminate = true;
        }
      }
    }
  }
};
using ReadThreadPtr = std::shared_ptr<ReadThread>;

TEST_F(UtilCasTest, casQueue) {
  WriteThreadPtr w = std::make_shared<WriteThread>();
  ReadThreadPtr r = std::make_shared<ReadThread>();

  w->start();
  r->start();

  w->join();
  r->join();
}

template <typename Q>
void start(int w, int r, int sleeps, Q &queue_data) {
  g_terminate = false;

  atomic<int64_t> writeIndex{0};
  vector<std::thread *> wthreads;
  for (int i = 0; i < w; i++) {
    wthreads.push_back(new std::thread([&] {
      while (!g_terminate) {
        queue_data.push_back(++writeIndex);
      }
    }));
  }

  int64_t readIndex = 0;

  vector<std::thread *> rthreads;
  for (int i = 0; i < r; i++) {
    rthreads.push_back(new std::thread([&] {
      do {
        int64_t j;
        if (queue_data.pop_front(j)) {
          readIndex = j;
        } else {
          if (sleeps > 0) {
            KT_Common::msleep(sleeps);
          }
        }

      } while (!g_terminate);
    }));
  }

  std::thread print([&] {
    int64_t lastReadIndex = 0;
    int64_t lastWriteIndex = 0;
    while (!g_terminate) {
      cout << "size:" << data_queue.size() << ", write/read index:" << writeIndex / 10000. << "/" << readIndex / 10000.
           << ", " << (writeIndex - lastWriteIndex) / 10000. << ", " << (readIndex - lastReadIndex) / 10000. << endl;
      lastReadIndex = readIndex;
      lastWriteIndex = writeIndex;
      KT_Common::sleep(1);
    }
  });

  std::thread t([&] {
    KT_Common::sleep(10);
    g_terminate = true;
  });
  t.join();
  print.join();
  for (size_t i = 0; i < wthreads.size(); i++) {
    wthreads[i]->join();
    delete wthreads[i];
  }
  for (size_t i = 0; i < rthreads.size(); i++) {
    rthreads[i]->join();
    delete rthreads[i];
  }
}

int wThread = 1;
int rThread = 4;
int sleepms = 0;

TEST_F(UtilCasTest, queueBatch) {
  cout << "threadQueueBatch-------------------------------------------" << endl;
  start<KT_ThreadQueue<int64_t>>(wThread, rThread, sleepms, data_queue);

  cout << "casQueueBatch-------------------------------------------" << endl;
  start<KT_CasQueue<int64_t>>(wThread, rThread, sleepms, data_cas);
}