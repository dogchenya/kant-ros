#include "servant/EndpointInfo.h"
#include "servant/RemoteLogger.h"
//#include "servant/NetworkUtil.h"
#include "util/kt_socket.h"

namespace kant {

EndpointInfo::EndpointInfo() {
  _setDivision.clear();
  // memset(&_addr,0,sizeof(_addr));
}

EndpointInfo::EndpointInfo(const KT_Endpoint &ep, const string &setDivision)
  : _ep(ep),
    _setDivision(setDivision)
// , _addressSucc(false)
{
  _cmpDesc = createCompareDesc();
  _desc = createDesc();
}

EndpointInfo::EndpointInfo(const EndpointF &ep) : _setDivision(ep.setId) {
  _ep.setHost(ep.host);
  _ep.setPort(ep.port);
  _ep.setTimeout(ep.timeout);
  _ep.setType((KT_Endpoint::EType)ep.istcp);
  _ep.setGrid(ep.grid);
  _ep.setQos(ep.qos);
  _ep.setWeight(ep.weight);
  _ep.setWeightType(ep.weightType);

  _ep.setAuthType((KT_Endpoint::AUTH_TYPE)ep.authType);

  _cmpDesc = createCompareDesc();
  _desc = createDesc();
}

string EndpointInfo::createCompareDesc() {
  stringstream ss;
  ss << _ep.getType() << ":" << _ep.getHost() << ":" << _ep.getPort();

  return ss.str();
}

///////////////////////////////////////////////////////////
}  // namespace kant
