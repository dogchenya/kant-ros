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
  virtual int test(kant::KantCurrentPtr current);

  kant::Int32 testInt(kant::Int32 iIn, kant::Int32 &iOut, kant::KantCurrentPtr current);

  kant::Int32 testStr(const std::string &sIn, std::string &sOut, kant::KantCurrentPtr current);

 private:
};
/////////////////////////////////////////////////////
#endif
