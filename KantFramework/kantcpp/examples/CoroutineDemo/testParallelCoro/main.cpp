#include "BServant.h"
#include "servant/Communicator.h"
//#include "servant/CoroutineScheduler.h"
#include <iostream>

using namespace std;
using namespace Test;
using namespace kant;

class BServantCoroCallback : public BServantCoroPrxCallback {
 public:
  virtual ~BServantCoroCallback() {}

  virtual void callback_testCoroSerial(kant::Int32 ret, const std::string &sOut)  // override
  {
    _iRet = ret;
    _sOut = sOut;
  }
  virtual void callback_testCoroSerial_exception(kant::Int32 ret)  // override
  {
    _iException = ret;
  }

  virtual void callback_testCoroParallel(kant::Int32 ret, const std::string &sOut)  // override
  {
    _iRet = ret;
    _sOut = sOut;
  }

  virtual void callback_testCoroParallel_exception(kant::Int32 ret)  // override
  {
    _iException = ret;
  }

 public:
  int _iException;
  int _iRet;
  int _iOut;
  string _sOut;
};
typedef std::shared_ptr<BServantCoroCallback> BServantCoroCallbackPtr;

////////////////////////////////////////////
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
  unsigned long sum = 0;

  for (int i = 0; i < _num; i++) {
    try {
      CoroParallelBasePtr sharedPtr = std::make_shared<CoroParallelBase>(2);

      BServantCoroCallbackPtr cb1 = std::make_shared<BServantCoroCallback>();
      cb1->setCoroParallelBasePtr(sharedPtr);
      _prx->coro_testCoroSerial(cb1, sIn);

      BServantCoroCallbackPtr cb2 = std::make_shared<BServantCoroCallback>();
      cb2->setCoroParallelBasePtr(sharedPtr);
      _prx->coro_testCoroParallel(cb2, sIn);

      coroWhenAll(sharedPtr);

      // cout << "ret1:" << cb1->_sOut << "|ret2:" << cb2->_sOut << endl;

      if (cb1->_iRet == 0 && cb2->_iRet == 0 && cb1->_iException == 0 && cb2->_iException == 0) {
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

  testCoro.setCoroInfo(10, 128, 128 * 1024);

  testCoro.start();

  testCoro.getThreadControl().join();

  return 0;
}
