#include "BServantImp.h"
#include "BServer.h"
#include "servant/Application.h"
#include "servant/Communicator.h"

using namespace std;
using namespace kant;

//////////////////////////////////////////////////////
void BServantImp::initialize() {
  //initialize servant here:
  //...
  _pPrx = Application::getCommunicator()->stringToProxy<AServantPrx>(
    "TestApp.AServer.AServantObj@tcp -h 127.0.0.1 -p 9000 -t 10000");
}
//////////////////////////////////////////////////////
void BServantImp::destroy() {}

class AServantCoroCallback : public AServantCoroPrxCallback {
 public:
  virtual ~AServantCoroCallback() {}

  virtual void callback_testInt(kant::Int32 ret, kant::Int32 iOut) {
    _iRet = ret;
    _iOut = iOut;
  }
  virtual void callback_testInt_exception(kant::Int32 ret) { _iException = ret; }

  virtual void callback_testStr(kant::Int32 ret, const std::string &sOut) {
    _iRet = ret;
    _sOut = sOut;
  }
  virtual void callback_testStr_exception(kant::Int32 ret) { _iException = ret; }

 public:
  int _iException;
  int _iRet;
  int _iOut;
  string _sOut;
};
typedef std::shared_ptr<AServantCoroCallback> AServantCoroCallbackPtr;

int BServantImp::test(kant::KantCurrentPtr current) { return 0; }

kant::Int32 BServantImp::testCoroSerial(const std::string &sIn, std::string &sOut, kant::KantCurrentPtr current) {
  try {
    int iRet = -1;

    int iIn = 5;
    int iOut = 0;

    iRet = _pPrx->testInt(iIn, iOut);

    if (iRet == 0) {
      string sRet("");

      iRet = _pPrx->testStr(sIn, sRet);

      if (iRet == 0) {
        sOut = sRet;
      }
    }

    return iRet;
  } catch (exception &ex) {
    TLOGERROR("BServantImp::testCoroSerial exception:" << ex.what() << endl);
  }

  return -1;
}

kant::Int32 BServantImp::testCoroParallel(const std::string &sIn, std::string &sOut, kant::KantCurrentPtr current) {
  try {
    int iRet = -1;

    int iIn = 5;

    CoroParallelBasePtr sharedPtr = std::make_shared<CoroParallelBase>(2);

    AServantCoroCallbackPtr cb1 = std::make_shared<AServantCoroCallback>();
    cb1->setCoroParallelBasePtr(sharedPtr);
    _pPrx->coro_testInt(cb1, iIn);

    AServantCoroCallbackPtr cb2 = std::make_shared<AServantCoroCallback>();
    cb2->setCoroParallelBasePtr(sharedPtr);
    _pPrx->coro_testStr(cb2, sIn);

    coroWhenAll(sharedPtr);

    if (cb1->_iRet == 0 && cb2->_iRet == 0) {
      sOut = cb2->_sOut;
      iRet = 0;
    }

    return iRet;
  } catch (exception &ex) {
    TLOGERROR("BServantImp::testCoroParallel exception:" << ex.what() << endl);
  }

  return -1;
}
