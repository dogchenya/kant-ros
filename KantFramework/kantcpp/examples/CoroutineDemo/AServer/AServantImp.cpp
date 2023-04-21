#include "AServantImp.h"
#include "AServer.h"

using namespace std;

//////////////////////////////////////////////////////
void AServantImp::initialize() {
  //initialize servant here:
  //...
}

//////////////////////////////////////////////////////
void AServantImp::destroy() {
  //destroy servant here:
  //...
}

int AServantImp::test(kant::KantCurrentPtr current) { return 0; }

kant::Int32 AServantImp::testInt(kant::Int32 iIn, kant::Int32 &iOut, kant::KantCurrentPtr current) {
  iOut = iIn;

  return 0;
}

kant::Int32 AServantImp::testStr(const std::string &sIn, std::string &sOut, kant::KantCurrentPtr current) {
  sOut = sIn;

  return 0;
}
