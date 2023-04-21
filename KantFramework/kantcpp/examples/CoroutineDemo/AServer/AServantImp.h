#ifndef _AServantImp_H_
#define _AServantImp_H_

#include "servant/Application.h"
#include "AServant.h"

/**
 *
 *
 */
class AServantImp : public Test::AServant {
 public:
  /**
	 *
	 */
  virtual ~AServantImp() {}

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
  virtual int test(tars::KantCurrentPtr current);

  tars::Int32 testInt(tars::Int32 iIn, tars::Int32 &iOut, tars::KantCurrentPtr current);

  tars::Int32 testStr(const std::string &sIn, std::string &sOut, tars::KantCurrentPtr current);

 private:
};
/////////////////////////////////////////////////////
#endif
