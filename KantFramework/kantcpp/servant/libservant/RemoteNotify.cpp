#include "servant/RemoteNotify.h"
#include "servant/Communicator.h"
#include "servant/RemoteLogger.h"
#include "servant/Application.h"

namespace kant {

int RemoteNotify::setNotifyInfo(const CommunicatorPtr &comm, const string &obj, const string &app,
                                const string &serverName, const string &sSetName, const string &nodeName) {
  _comm = comm;
  if (!obj.empty()) {
    _notifyPrx = _comm->stringToProxy<NotifyPrx>(obj);
    _notifyPrx->kant_timeout(500);
  }

  _setName = sSetName;
  _app = app;
  _serverName = serverName;
  _nodeName = nodeName;
  return 0;
}

void RemoteNotify::report(const string &sResult, bool bSync) {
  try {
    if (_notifyPrx) {
      ReportInfo info;
      info.eType = REPORT;
      info.sApp = _app;
      info.sServer = _serverName;
      info.sSet = _setName;
      info.sThreadId = KT_Common::tostr(std::this_thread::get_id());
      info.sMessage = sResult;
      info.sNodeName = _nodeName;
      if (!bSync) {
        _notifyPrx->async_reportNotifyInfo(NULL, info, ServerConfig::Context);
      } else {
        _notifyPrx->reportNotifyInfo(info, ServerConfig::Context);
      }
    }
  } catch (exception &ex) {
    TLOGERROR("[RemoteNotify::report error:" << ex.what() << "]" << endl);
  } catch (...) {
    TLOGERROR("[RemoteNotify::report unknown error"
              << "]" << endl);
  }
}

void RemoteNotify::notify(NOTIFYLEVEL level, const string &sMessage) {
  try {
    if (_notifyPrx) {
      ReportInfo info;
      info.eType = NOTIFY;
      info.sApp = _app;
      info.sServer = _serverName;
      info.sSet = _setName;
      info.sThreadId = KT_Common::tostr(std::this_thread::get_id());
      info.sMessage = sMessage;
      info.eLevel = level;
      info.sNodeName = _nodeName;
      _notifyPrx->async_reportNotifyInfo(NULL, info, ServerConfig::Context);
    }
  } catch (exception &ex) {
    TLOGERROR("[RemoteNotify::notify error:" << ex.what() << "]" << endl);
  } catch (...) {
    TLOGERROR("[RemoteNotify::notify unknown error"
              << "]" << endl);
  }
}

void RemoteNotify::report(const string &sMessage, const string &app, const string &serverName,
                          const string &sNodeName) {
  try {
    if (_notifyPrx) {
      ReportInfo info;
      info.eType = REPORT;
      info.sApp = app;
      info.sServer = serverName;
      info.sSet = "";
      info.sMessage = sMessage;
      info.sNodeName = sNodeName;
      _notifyPrx->async_reportNotifyInfo(NULL, info, ServerConfig::Context);
    }
  } catch (exception &ex) {
    TLOGERROR("[RemoteNotify::notify error:" << ex.what() << "]" << endl);
  } catch (...) {
    TLOGERROR("[RemoteNotify::notify unknown error"
              << "]" << endl);
  }
}

}  // namespace kant
