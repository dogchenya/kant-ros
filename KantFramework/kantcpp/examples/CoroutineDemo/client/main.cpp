﻿#include "BServant.h"
#include "servant/Communicator.h"
#include "util/kt_thread.h"
#include "util/kt_option.h"
#include <iostream>

using namespace std;
using namespace Test;
using namespace kant;

Communicator* _comm;

static string coroObj = "TestApp.BServer.BServantObj@tcp -h 127.0.0.1 -p 9200";

struct Param {
  int count;
  string call;
  int thread;
  int buffersize;
  int netthread;

  BServantPrx pPrx;
};

Param param;
std::atomic<int> callback_count(0);

void serialCall(int c) {
  string buffer(param.buffersize, 'a');

  int64_t t = KT_Common::now2us();
  //发起远程调用
  for (int i = 0; i < c; ++i) {
    string r;

    try {
      param.pPrx->testCoroSerial(buffer, r);
    } catch (exception& e) {
      cout << "exception:" << e.what() << endl;
    }
    ++callback_count;
  }

  int64_t cost = KT_Common::now2us() - t;
  cout << "serialCall  total:" << cost << "us, avg:" << 1. * cost / c << "us" << endl;
}

void parallelCall(int c) {
  string buffer(param.buffersize, 'a');

  int64_t t = KT_Common::now2us();
  //发起远程调用
  for (int i = 0; i < c; ++i) {
    string r;

    try {
      param.pPrx->testCoroParallel(buffer, r);
    } catch (exception& e) {
      cout << "exception:" << e.what() << endl;
    }
    ++callback_count;
  }

  int64_t cost = KT_Common::now2us() - t;
  cout << "parallelCall  total:" << cost << "us, avg:" << 1. * cost / c << "us" << endl;
}
//
//class Test1
//{
//public:
//    Test1();
//
//    ~Test1();
//
//    void  queryResult(int iFlag, int iExecuteNum);
//
//private:
//    Communicator    _comm;
//    BServantPrx     _prx;
//};
//
//Test1::Test1()
//{
//    // _comm.setProperty("locator", "kant.kantregistry.QueryObj@tcp -h 10.208.139.242 -p 17890 -t 10000");
//    // _comm.setProperty("stat", "kant.kantstat.StatObj");
//    _prx = _comm.stringToProxy<BServantPrx>("TestApp.BServer.BServantObj@tcp -h 127.0.0.1 -p 9200");
//}
//
//Test1::~Test1()
//{
//
//}
//
//void Test1::queryResult(int iFlag, int iExecuteNum)
//{
//    string sIn(10,'a');
//    string sOut("");
//
//    time_t t = KT_Common::now2us();
//
//    for(int i=0; i<iExecuteNum; i++)
//    {
//        // sOut = "";
//        try
//        {
//            int ret = -1;
//            if(iFlag == 0)
//            {
//                ret = _prx->testCoroSerial(sIn, sOut);
//            }
//            else
//            {
//                ret = _prx->testCoroParallel(sIn, sOut);
//            }
//
//            assert(sIn == sOut);
//            // cout << ret << ", " << sIn << ", " << sOut << endl;
//        }
//        catch(KT_Exception &e)
//        {
//            cout << "pthread id: " << std::this_thread::get_id() << "id: " << i << "exception: " << e.what() << endl;
//        }
//        catch(...)
//        {
//            cout << "pthread id: " << std::this_thread::get_id() << "id: " << i << "unknown exception." << endl;
//        }
//    }
//
//    int64_t cost = KT_Common::now2us() - t;
//    cout << "syncCall total:" << cost << "us, avg:" << 1.*cost/iExecuteNum << "us" << endl;
//}

int main(int argc, char* argv[]) {
  try {
    if (argc < 5) {
      cout << "Usage:" << argv[0] << " --count=1000 --call=[serial|parallel] --thread=1 --buffersize=1000 --netthread=1"
           << endl;

      return 0;
    }

    KT_Option option;
    option.decode(argc, argv);

    param.count = KT_Common::strto<int>(option.getValue("count"));
    if (param.count <= 0) param.count = 1000;
    param.buffersize = KT_Common::strto<int>(option.getValue("buffersize"));
    if (param.buffersize <= 0) param.buffersize = 1000;
    param.call = option.getValue("call");
    if (param.call.empty()) param.call = "sync";
    param.thread = KT_Common::strto<int>(option.getValue("thread"));
    if (param.thread <= 0) param.thread = 1;
    param.netthread = KT_Common::strto<int>(option.getValue("netthread"));
    if (param.netthread <= 0) param.netthread = 1;

    _comm = new Communicator();

    param.pPrx = _comm->stringToProxy<BServantPrx>(coroObj);

    //        LocalRollLogger::getInstance()->logger()->setLogLevel(6);

    _comm->setProperty("sendqueuelimit", "1000000");
    _comm->setProperty("asyncqueuecap", "1000000");

    _comm->setProperty("netthread", KT_Common::tostr(param.netthread));

    int64_t start = KT_Common::now2us();

    std::function<void(int)> func;

    if (param.call == "serial") {
      func = serialCall;
    } else if (param.call == "parallel") {
      func = parallelCall;
    }

    param.pPrx->kant_connect_timeout(5000);
    param.pPrx->kant_async_timeout(60 * 1000);

    vector<std::thread*> vt;
    for (int i = 0; i < param.thread; i++) {
      vt.push_back(new std::thread(func, param.count));
    }

    std::thread print([&] {
      while (callback_count != param.count * param.thread) {
        cout << "coro:" << param.call << ": ----------finish count:" << callback_count << endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
      };
    });

    for (size_t i = 0; i < vt.size(); i++) {
      vt[i]->join();
      delete vt[i];
    }

    cout << "(pid:" << std::this_thread::get_id() << ")"
         << "(count:" << param.count << ")"
         << "(use ms:" << (KT_Common::now2us() - start) / 1000 << ")" << endl;

    while (callback_count != param.count * param.thread) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    print.join();
    cout << "----------finish count:" << callback_count << endl;
  } catch (exception& ex) {
    cout << ex.what() << endl;
  }
  cout << "main return." << endl;

  return 0;
}
