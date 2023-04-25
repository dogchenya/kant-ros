#include <iostream>
#include "servant/Communicator.h"
#include "Hello.h"
#include "util/kt_option.h"

using namespace std;
using namespace kant;
using namespace TestApp;

Communicator* _comm;

static string helloObj = "TestApp.AuthServer.AuthObj@tcp -h 127.0.0.1 -p 9016 -e 1";

struct Param {
  int count;
  string call;
  int thread;
  int buffersize;
  int netthread;

  HelloPrx pPrx;
};

Param param;
std::atomic<int> callback_count(0);

struct HelloCallback : public HelloPrxCallback {
  HelloCallback(int64_t t, int i, int c) : start(t), cur(i), count(c) {}

  //call back
  virtual void callback_testHello(int ret, const string& r) {
    assert(ret == 0);
    callback_count++;

    if (cur == count - 1) {
      int64_t cost = KT_Common::now2us() - start;
      cout << "callback_testHello count:" << count << ", " << cost << " us, avg:" << 1. * cost / count << "us" << endl;
    }
  }

  virtual void callback_testHello_exception(kant::Int32 ret) { cout << "callback exception:" << ret << endl; }

  int64_t start;
  int cur;
  int count;
};

void syncCall(int c) {
  string buffer(param.buffersize, 'a');

  int64_t t = KT_Common::now2us();
  //发起远程调用
  for (int i = 0; i < c; ++i) {
    string r;

    try {
      param.pPrx->testHello(buffer, r);
    } catch (exception& e) {
      cout << "exception:" << e.what() << endl;
    }
    ++callback_count;
  }

  int64_t cost = KT_Common::now2us() - t;
  cout << "syncCall total:" << cost << "us, avg:" << 1. * cost / c << "us" << endl;
}

void asyncCall(int c) {
  int64_t t = KT_Common::now2us();

  string buffer(param.buffersize, 'a');

  //发起远程调用
  for (int i = 0; i < c; ++i) {
    HelloPrxCallbackPtr p = std::make_shared<HelloCallback>(t, i, c);

    try {
      param.pPrx->async_testHello(p, buffer);
    } catch (exception& e) {
      cout << "exception:" << e.what() << endl;
    }
  }

  int64_t cost = KT_Common::now2us() - t;
  cout << "asyncCall send:" << cost << "us, avg:" << 1. * cost / c << "us" << endl;
}

int main(int argc, char* argv[]) {
  try {
    if (argc < 6) {
      cout << "Usage:" << argv[0]
           << "--config=conf --count=1000 --call=[sync|async] --thread=1 --buffersize=1000 --netthread=1" << endl;

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

    KT_Config conf;
    conf.parseFile(option.getValue("config"));
    _comm->setProperty(conf);

    //        LocalRollLogger::getInstance()->logger()->setLogLevel(6);

    _comm->setProperty("sendqueuelimit", "1000000");
    _comm->setProperty("asyncqueuecap", "1000000");

    _comm->setProperty("netthread", KT_Common::tostr(param.netthread));

    param.pPrx = _comm->stringToProxy<HelloPrx>(helloObj);

    param.pPrx->kant_connect_timeout(5000);
    param.pPrx->kant_async_timeout(60 * 1000);
    param.pPrx->kant_ping();

    int64_t start = KT_Common::now2us();

    std::function<void(int)> func;

    if (param.call == "sync") {
      func = syncCall;
    } else if (param.call == "async") {
      func = asyncCall;
    } else {
      cout << "no func, exits" << endl;
      exit(0);
    }

    vector<std::thread*> vt;
    for (int i = 0; i < param.thread; i++) {
      vt.push_back(new std::thread(func, param.count));
    }

    std::thread print([&] {
      while (callback_count != param.count * param.thread) {
        cout << "Auth:" << param.call << " : ----------finish count:" << callback_count << endl;
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
    cout << "Auth:" << param.call << " ----------finish count:" << callback_count << endl;
  } catch (exception& ex) {
    cout << ex.what() << endl;
  }
  cout << "main return." << endl;

  return 0;
}
