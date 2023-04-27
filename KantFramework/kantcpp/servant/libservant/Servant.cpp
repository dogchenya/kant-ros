#include "servant/Servant.h"
#include "servant/BaseF.h"
#include "servant/Application.h"
#include "servant/AppCache.h"
#include "servant/RemoteLogger.h"

#include <cerrno>

namespace kant {

thread_local shared_ptr<CallbackThreadData> CallbackThreadData::g_sp;

Servant::Servant() : _handle(NULL) {}

Servant::~Servant() {}

void Servant::setName(const string& name) { _name = name; }

string Servant::getName() const { return _name; }

void Servant::setApplication(Application* application) {
  _application = application;

  setNotifyObserver(application->getNotifyObserver());
}

Application* Servant::getApplication() const { return _application; }

void Servant::setHandle(KT_EpollServer::Handle* handle) { _handle = handle; }

KT_EpollServer::Handle* Servant::getHandle() { return _handle; }

int Servant::dispatch(CurrentPtr current, vector<char>& buffer) {
  int ret = KANTSERVERUNKNOWNERR;

  if (current->getFuncName() == "kant_ping") {
    TLOGKANT("[Servant::dispatch] kant_ping ok from [" << current->getIp() << ":" << current->getPort() << "]" << endl);

    ret = KANTSERVERSUCCESS;
  } else if (!current->getBindAdapter()->isKantProtocol()) {
    KT_LockT<KT_ThreadRecMutex> lock(*this);

    ret = doRequest(current, buffer);
  } else {
    KT_LockT<KT_ThreadRecMutex> lock(*this);

    ret = onDispatch(current, buffer);
  }
  return ret;
}

KT_CasQueue<ReqMessagePtr>& Servant::getResponseQueue() { return _asyncResponseQueue; }

///////////////////////////////////////////////////////////////////////////
ServantCallback::ServantCallback(const string& type, const ServantPtr& servant, const CurrentPtr& current)
  : _servant(servant), _current(current) {
  ServantProxyCallback::setType(type);
}

int ServantCallback::onDispatch(ReqMessagePtr msg) {
  _servant->getResponseQueue().push_back(msg);

  _servant->getHandle()->notifyFilter();

  return 0;
}

const ServantPtr& ServantCallback::getServant() { return _servant; }

const CurrentPtr& ServantCallback::getCurrent() { return _current; }

///////////////////////////////////////////////////////////////////////////
CallbackThreadData::CallbackThreadData() : _contextValid(false) {}

// void CallbackThreadData::destructor(void* p)
// {
//     CallbackThreadData * pCbtd = (CallbackThreadData*)p;
//     if(pCbtd)
//         delete pCbtd;
// }

CallbackThreadData* CallbackThreadData::getData() {
  if (!g_sp) {
    g_sp.reset(new CallbackThreadData());
  }
  return g_sp.get();
}

void CallbackThreadData::setResponseContext(const map<std::string, std::string>& context) {
  _contextValid = true;
  if (context.empty()) {
    _responseContext.clear();
  } else {
    _responseContext = context;
  }
}

map<std::string, std::string>& CallbackThreadData::getResponseContext() { return _responseContext; }

void CallbackThreadData::delResponseContext() {
  _contextValid = false;
  _responseContext.clear();
}

////////////////////////////////////////////////////////////////////////
}  // namespace kant
