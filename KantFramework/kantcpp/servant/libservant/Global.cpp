#include "servant/Global.h"
#include "servant/BaseF.h"
#include "servant/RemoteLogger.h"

namespace kant {

//////////////////////////////////////////////////////////////////////////
void KantException::throwException(int ret, const string& desc) {
  switch (ret) {
    case kant::KANTSERVERSUCCESS:
      break;
    case KANTSERVERDECODEERR:
      throw KantServerDecodeException("server decode exception: ret:" + KT_Common::tostr(ret) + " msg:" + desc);
      break;
    case KANTSERVERENCODEERR:
      throw KantServerEncodeException("server encode exception: ret:" + KT_Common::tostr(ret) + " msg:" + desc);
      break;
    case KANTSERVERNOFUNCERR:
      throw KantServerNoFuncException("server function mismatch exception: ret:" + KT_Common::tostr(ret) +
                                      " msg:" + desc);
      break;
    case KANTSERVERNOSERVANTERR:
      throw KantServerNoServantException("server servant mismatch exception: ret:" + KT_Common::tostr(ret) +
                                         " msg:" + desc);
      break;
    case KANTSERVERQUEUETIMEOUT:
      throw KantServerQueueTimeoutException("server queue timeout exception: ret:" + KT_Common::tostr(ret) +
                                            " msg:" + desc);
      break;
    case KANTPROXYCONNECTERR:
      throw KantServerConnectionException("server connection lost: ret:" + KT_Common::tostr(ret) + " msg:" + desc);
      break;
    case KANTINVOKETIMEOUT:
      throw KantServerInvokeTimeoutException(desc);
    default:
      throw KantServerUnknownException("server unknown exception: ret:" + KT_Common::tostr(ret) + " msg:" + desc);
  }
}
////////////////////////////////////////////////////////////////////////////
}  // namespace kant
