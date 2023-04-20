#include "util/kt_platform.h"
#include "util/kt_port.h"
#include "servant/KeepAliveNodeF.h"
#include "servant/RemoteLogger.h"
#include "servant/Communicator.h"

namespace kant {

void KeepAliveNodeFHelper::setNodeInfo(const CommunicatorPtr &comm, const string &obj, const string &app,
                                       const string &server) {
  _comm = comm;
  if (!obj.empty()) {
    _nodePrx = _comm->stringToProxy<ServerFPrx>(obj);
  }

  _si.application = app;
  _si.serverName = server;
  _si.pid = KT_Port::getpid();
}

void KeepAliveNodeFHelper::keepAlive(const string &adapter) {
  try {
    if (_nodePrx) {
      set<string> s;
      {
        KT_LockT<KT_ThreadMutex> lock(*this);

        _adapterSet.insert(adapter);

        //admin心跳来的时候才上报(减小上报次数)
        if (!adapter.empty() && adapter != "AdminAdapter") {
          return;
        }

        s.swap(_adapterSet);
      }
      ServerInfo si = _si;
      set<string>::const_iterator it = s.begin();
      while (it != s.end()) {
        si.adapter = *it;
        _nodePrx->async_keepAlive(NULL, si);
        ++it;
      }
    }
  } catch (exception &ex) {
    TLOGERROR("KeepAliveNodeFHelper::keepAlive error:" << ex.what() << endl);
  } catch (...) {
    TLOGERROR("KeepAliveNodeFHelper::keepAlive unknown error" << endl);
  }
}

void KeepAliveNodeFHelper::keepActiving() {
  try {
    if (_nodePrx) {
      _nodePrx->async_keepActiving(NULL, _si);
    }
  } catch (exception &ex) {
    TLOGERROR("[KeepAliveNodeFHelper::keepAlive error:" << ex.what() << "]" << endl);
  } catch (...) {
    TLOGERROR("[KeepAliveNodeFHelper::keepAlive unknown error]" << endl);
  }
}

void KeepAliveNodeFHelper::reportVersion(const string &version) {
  try {
    if (_nodePrx) {
      _nodePrx->async_reportVersion(NULL, _si.application, _si.serverName, version);
    }
  } catch (exception &ex) {
    TLOGERROR("[KeepAliveNodeFHelper::reportVersion error:" << ex.what() << "]" << endl);
  } catch (...) {
    TLOGERROR("[KeepAliveNodeFHelper::reportVersion unknown error"
              << "]" << endl);
  }
}

}  // namespace kant
