#include "BServant.h"
#include "servant/Communicator.h"
//#include "servant/CoroutineScheduler.h"
#include <iostream>

using namespace std;
using namespace Test;
using namespace kant;

//继承框架的协程类
class TestCoroutine : public KT_Coroutine {
 public:
  TestCoroutine(int iNum);

  ~TestCoroutine() {}

  void handle();

 private:
  int _num;
  Communicator _comm;
  BServantPrx _prx;
};

TestCoroutine::TestCoroutine(int iNum) : _num(iNum) {
  // _comm.setProperty("locator", "kant.kantregistry.QueryObj@tcp -h 10.208.139.242 -p 17890 -t 10000");
  _prx = _comm.stringToProxy<BServantPrx>("TestApp.BServer.BServantObj@tcp -h 127.0.0.1 -p 9200");
  // _comm.stringToProxy(_sObj, _prx);
}

void TestCoroutine::handle() {
  string sIn(32, 'a');
  string sOut("");
  unsigned long sum = 0;

  for (int i = 0; i < _num; i++) {
    try {
      int iRet = _prx->testCoroSerial(sIn, sOut);
      if (iRet == 0) {
        ++sum;
      }

      sOut = "";
      iRet = _prx->testCoroParallel(sIn, sOut);
      if (iRet == 0) {
        ++sum;
      }
    } catch (KT_Exception &e) {
      cout << "i: " << i << "exception: " << e.what() << endl;
    } catch (...) {
      cout << "i: " << i << "unknown exception." << endl;
    }
  }
  cout << "succ:" << sum << endl;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "usage: " << argv[0] << " CallTimes " << endl;
    return -1;
  }

  kant::Int32 iNum = KT_Common::strto<kant::Int32>(string(argv[1]));

  TestCoroutine testCoro(iNum);

  //start 10 co
  testCoro.setCoroInfo(10, 128, 128 * 1024);

  testCoro.start();

  testCoro.getThreadControl().join();

  return 0;
}
