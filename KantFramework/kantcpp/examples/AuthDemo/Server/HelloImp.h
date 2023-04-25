#ifndef _SSLImp_H_
#define _SSLImp_H_

#include "servant/Application.h"
#include "Hello.h"

/**
 *
 *
 */
class HelloImp : public TestApp::Hello {
 public:
  /**
     *
     */
  virtual ~HelloImp() {}

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
  virtual int test(kant::KantCurrentPtr current) { return 0; };

  virtual int testHello(const std::string &sReq, std::string &sRsp, kant::KantCurrentPtr current);
};
/////////////////////////////////////////////////////
#endif
