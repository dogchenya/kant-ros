#include "util/kt_coroutine.h"

using namespace kant;
class MyCoroutine : public KT_Coroutine {
 protected:
  void handle() {
    ++_count;

    this->go(std::bind(&MyCoroutine::co_test, this));
  }

  void co_test() {
    ++_count;
    ;
  }

 public:
  static atomic<int> _count;
};

atomic<int> MyCoroutine::_count{0};

int main(int argc, char* argv[]) {
  MyCoroutine::_count = 0;

  MyCoroutine co;

  co.setCoroInfo(10, 200, 128 * 1024);

  co.start();

  co.join();

  return 1;
}