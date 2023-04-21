#ifndef _BServantImp_H_
#define _BServantImp_H_

#include "servant/Application.h"
#include "BServant.h"
#include "AServant.h"

using namespace Test;

/**
 *
 *
 */
class BServantImp : public Test::BServant {
 public:
  /**
	 *
	 */
  virtual ~BServantImp() {}

  /**
	 *
	 */
  virtual void initialize();

  /**
	 *
	 */
  virtual void destroy();

  /**
	 *
	 */
  virtual int test(kant::KantCurrentPtr current);

  kant::Int32 testCoroSerial(const std::string& sIn, std::string& sOut, kant::KantCurrentPtr current);

  kant::Int32 testCoroParallel(const std::string& sIn, std::string& sOut, kant::KantCurrentPtr current);

 private:
  AServantPrx _pPrx;
};
/////////////////////////////////////////////////////
#endif
