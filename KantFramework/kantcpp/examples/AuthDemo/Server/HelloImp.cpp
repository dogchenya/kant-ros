#include "HelloImp.h"
#include "servant/Application.h"

using namespace std;

//////////////////////////////////////////////////////
void HelloImp::initialize() {
  //initialize servant here:
  //...
}

//////////////////////////////////////////////////////
void HelloImp::destroy() {
  //destroy servant here:
  //...
}

int HelloImp::testHello(const std::string &sReq, std::string &sRsp, kant::KantCurrentPtr current) {
  //    TLOGDEBUG("HelloImp::testHellosReq:"<<sReq<<endl);
  //    cout << sReq << endl;
  sRsp = sReq;
  return 0;
}
