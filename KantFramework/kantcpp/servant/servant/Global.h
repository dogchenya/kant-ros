#ifndef _KANT_GLOBAL_H_
#define _KANT_GLOBAL_H_

#include <inttypes.h>
#include <iostream>
#include <sstream>
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <memory>
#include "util/kt_platform.h"
#include "util/kt_clientsocket.h"
//#include "util/kt_autoptr.h"
#include "util/kt_common.h"
#include "util/kt_logger.h"
#include "util/kt_thread_mutex.h"
#include "util/kt_coroutine.h"
#include "tup/RequestF.h"
#include "servant/BaseF.h"

using namespace std;

namespace kant {
//////////////////////////////////////////////////////////////

const size_t MAX_CLIENT_THREAD_NUM = 2048;  //客户端最大网络线程数(线程模型: 网络线程数, 协程模型: 业务线程+网络线程)
const size_t MAX_CLIENT_ASYNCTHREAD_NUM = 1024;  //客户端每个网络线程拥有的最大异步线程数
const size_t MAX_CLIENT_NOTIFYEVENT_NUM = 2048;  //客户端每个网络线程拥有的最大通知事件的数目

//////////////////////////////////////////////////////////////
class Communicator;
class AdapterProxy;
class ServantProxy;
class ServantProxyCallback;
class ObjectProxy;
class Current;
class FDReactor;
class KT_Transceiver;
class StatFProxy;
class StatReport;
class ServantProxyFactory;
class AsyncProcThread;
class LocalRollLogger;
class RemoteConfig;
class RemoteTimeLogger;
class RemoteNotify;

typedef std::shared_ptr<Communicator> CommunicatorPtr;
typedef std::shared_ptr<ServantProxy> ServantPrx;
typedef std::shared_ptr<ServantProxyCallback> ServantProxyCallbackPtr;
typedef std::shared_ptr<ObjectProxy> ObjectPrx;
typedef std::shared_ptr<Current> CurrentPtr;
typedef std::shared_ptr<StatFProxy> StatFPrx;
typedef std::shared_ptr<StatReport> StatReportPtr;
typedef std::shared_ptr<FDReactor> FDReactorPtr;
typedef std::shared_ptr<AsyncProcThread> AsyncProcThreadPtr;

typedef CurrentPtr KantCurrentPtr;
typedef RemoteConfig KantRemoteConfig;
typedef RemoteNotify KantRemoteNotify;
typedef LocalRollLogger KantRollLogger;
typedef RemoteTimeLogger KantTimeLogger;

//////////////////////////////////////////////////////////////
/**
 * 定义KANT网络调用的异常基类
 */
struct KantException : public KT_Exception {
  KantException(const string &buffer) : KT_Exception(buffer){};
  KantException(const string &buffer, int err) : KT_Exception(buffer, err){};
  ~KantException() throw(){};
  /**
     * 根据返回值抛出异常
     * @param ret
     * @param desc
     */
  static void throwException(int ret, const string &desc = "");
};

////////////////////////////////////////////////////////////////
// 定义网络异常

/**
 * Server编码异常
 */
struct KantServerEncodeException : public KantException {
  KantServerEncodeException(const string &buffer) : KantException(buffer){};
  ~KantServerEncodeException() throw(){};
};
/**
 * Server解码异常
 */
struct KantServerDecodeException : public KantException {
  KantServerDecodeException(const string &buffer) : KantException(buffer){};
  ~KantServerDecodeException() throw(){};
};

/**
 * Server无函数异常
 */
struct KantServerNoFuncException : public KantException {
  KantServerNoFuncException(const string &buffer) : KantException(buffer){};
  ~KantServerNoFuncException() throw(){};
};

/**
 * Server无对象异常
 */
struct KantServerNoServantException : public KantException {
  KantServerNoServantException(const string &buffer) : KantException(buffer){};
  ~KantServerNoServantException() throw(){};
};
/**
 * 消息在服务端队列超时
 */
struct KantServerQueueTimeoutException : public KantException {
  KantServerQueueTimeoutException(const string &buffer) : KantException(buffer){};
  ~KantServerQueueTimeoutException() throw(){};
};
/**
 * 连接异常
 */
struct KantServerConnectionException : public KantException {
  KantServerConnectionException(const string &buffer) : KantException(buffer){};
  ~KantServerConnectionException() throw(){};
};
/**
 * 调用超时(连接都没有成功建立)
 */
struct KantServerInvokeTimeoutException : public KantException {
  KantServerInvokeTimeoutException(const string &buffer) : KantException(buffer){};
  ~KantServerInvokeTimeoutException() throw(){};
};
/**
 * 服务端返回的未知值
 */
struct KantServerUnknownException : public KantException {
  KantServerUnknownException(const string &buffer) : KantException(buffer){};
  ~KantServerUnknownException() throw(){};
};

/**
 * 同步调用超时异常
 */
struct KantSyncCallTimeoutException : public KantException {
  KantSyncCallTimeoutException(const string &buffer) : KantException(buffer){};
  ~KantSyncCallTimeoutException() throw(){};
};

/**
 * 访问 Registry 错误
 */
struct KantRegistryException : public KantException {
  KantRegistryException(const string &buffer) : KantException(buffer){};
  ~KantRegistryException() throw(){};
};

/**
 * 客户端队列满了
 */
struct KantClientQueueException : public KantException {
  KantClientQueueException(const string &buffer) : KantException(buffer){};
  ~KantClientQueueException() throw(){};
};

/**
 * 业务线程调用协程并行请求接口时，抛出的异常
 */
struct KantUseCoroException : public KantException {
  KantUseCoroException(const string &buffer) : KantException(buffer){};
  ~KantUseCoroException() throw(){};
};

/**
 * 通信器析构了
 */
struct KantCommunicatorException : public KantException {
  KantCommunicatorException(const string &buffer) : KantException(buffer){};
  ~KantCommunicatorException() throw(){};
};
///////////////////////////////////////////////////////////////////
}  // namespace kant
#endif
