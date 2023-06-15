#include "util/kt_http_async.h"
#include "util/kt_common.h"
#include "util/kt_timeprovider.h"

namespace kant {

KT_HttpAsync::AsyncRequest::~AsyncRequest() {}

void KT_HttpAsync::AsyncRequest::initialize(KT_Epoller *epoller, const KT_Endpoint &ep, KT_HttpRequest &stHttpRequest,
                                            RequestCallbackPtr &callbackPtr) {
  _callbackPtr = callbackPtr;

#if KANT_SSL
  if (ep.isSSL()) {
    _trans.reset(new KT_SSLTransceiver(epoller, ep));
  } else {
    _trans.reset(new KT_TCPTransceiver(epoller, ep));
  }
#else
  _trans.reset(new KT_TCPTransceiver(epoller, ep));
#endif
  _buff = std::make_shared<KT_NetWorkBuffer::Buffer>();

  stHttpRequest.encode(_buff);

  _trans->initializeClient(
    std::bind(&AsyncRequest::onCreateCallback, this, std::placeholders::_1),
    std::bind(&AsyncRequest::onCloseCallback, this, std::placeholders::_1, std::placeholders::_2,
              std::placeholders::_3),
    std::bind(&AsyncRequest::onConnectCallback, this, std::placeholders::_1),
    std::bind(&AsyncRequest::onRequestCallback, this, std::placeholders::_1),
    std::bind(&AsyncRequest::onParserCallback, this, std::placeholders::_1, std::placeholders::_2),
    std::bind(&AsyncRequest::onOpensslCallback, this, std::placeholders::_1));
}

shared_ptr<KT_ProxyInfo> KT_HttpAsync::AsyncRequest::onCreateCallback(KT_Transceiver *trans) {
  _pHttpAsync->addFd(this);

  return NULL;
}

std::shared_ptr<KT_OpenSSL> KT_HttpAsync::AsyncRequest::onOpensslCallback(KT_Transceiver *trans) {
#if KANT_SSL
  if (trans->isSSL()) {
    if (!_pHttpAsync->getCtx()) {
      _ctx = KT_OpenSSL::newCtx("", "", "", false, "");
    } else {
      _ctx = _pHttpAsync->getCtx();
    }
    return KT_OpenSSL::newSSL(_ctx);
  }
  return NULL;
#else
  return NULL;
#endif
}

void KT_HttpAsync::AsyncRequest::onCloseCallback(KT_Transceiver *trans, KT_Transceiver::CloseReason reason,
                                                 const string &err) {
  if (reason == KT_Transceiver::CloseReason::CR_PEER_CLOSE) {
    //服务器端主动关闭的, 对于http而言, 如果没有content-length的情况下, 则认为是成功
    if (_callbackPtr) {
      if (_stHttpResp.isHeadComplete() && !_stHttpResp.hasHeader("Content-Length") && !_stHttpResp.isChunked()) try {
          _callbackPtr->onSucc(_stHttpResp);
        } catch (...) {
        }
    }
  }

  try {
    if (_callbackPtr) _callbackPtr->onClose();
  } catch (...) {
  }

  if (_pHttpAsync) {
    _pHttpAsync->erase(_iUniqId);
  }
}

void KT_HttpAsync::AsyncRequest::onConnectCallback(KT_Transceiver *trans) {
  //	LOG_CONSOLE_DEBUG << endl;
}

void KT_HttpAsync::AsyncRequest::onRequestCallback(KT_Transceiver *trans) {
  //	LOG_CONSOLE_DEBUG << _buff->length() << endl;

  if (!_buff->empty()) {
    auto iRet = trans->sendRequest(_buff);

    if (iRet == KT_Transceiver::eRetError) {
      doException(RequestCallback::Failed_Request, getError("request error"));
    }
  }
}

KT_NetWorkBuffer::PACKET_TYPE KT_HttpAsync::AsyncRequest::onParserCallback(KT_NetWorkBuffer &buff,
                                                                           KT_Transceiver *trans) {
  if (buff.empty()) {
    return KT_NetWorkBuffer::PACKET_LESS;
  }

  //	LOG_CONSOLE_DEBUG << buff.getBuffer()->buffer() << endl;
  //增量decode
  bool ret = _stHttpResp.incrementDecode(buff);

  //	LOG_CONSOLE_DEBUG << _stHttpResp.getContent() << endl;

  //有头部数据了
  if (_callbackPtr && !_stHttpResp.getHeaders().empty()) {
    bool bContinue = _callbackPtr->onContinue(_stHttpResp);
    if (!bContinue) {
      doException(RequestCallback::Failed_Interrupt, getError("receive interrupt"));
      return KT_NetWorkBuffer::PACKET_ERR;
    }
  }

  //数据接收完毕
  if (ret) {
    try {
      if (_callbackPtr) _callbackPtr->onSucc(_stHttpResp);
    } catch (...) {
    }

    return KT_NetWorkBuffer::PACKET_FULL_CLOSE;
  }

  return KT_NetWorkBuffer::PACKET_LESS;
}

void KT_HttpAsync::AsyncRequest::setBindAddr(const KT_Socket::addr_type &bindAddr) { _trans->setBindAddr(bindAddr); }

void KT_HttpAsync::AsyncRequest::timeout() {
  if (_pHttpAsync) _pHttpAsync->assertThreadId();

  if (hasConnected())
    doException(RequestCallback::Failed_Timeout, "timeout error.");
  else
    doException(RequestCallback::Failed_ConnectTimeout, "connect timeout error.");
}

string KT_HttpAsync::AsyncRequest::getError(const string &sDefault) const {
  int ret = KT_Exception::getSystemCode();
  if (ret != 0) {
    return sDefault + ", ret:" + KT_Common::tostr(ret) + ", msg:" + KT_Exception::parseError(ret);
  }

  return sDefault + ", ret:" + KT_Common::tostr(ret);
}

void KT_HttpAsync::AsyncRequest::doException(RequestCallback::FAILED_CODE ret, const string &e) {
  try {
    if (_callbackPtr) _callbackPtr->onFailed(ret, e);
  } catch (...) {
  }
}

///////////////////////////////////////////////////////////////////////////
//#define H64(x) (((uint64_t)x) << 32)

KT_HttpAsync::KT_HttpAsync()  //: _terminate(false)
{
  _data = new http_queue_type(10000);

  _epoller.create(10240);
}

KT_HttpAsync::~KT_HttpAsync() {
  terminate();

  delete _data;
}

void KT_HttpAsync::start() {
  _tpool.init(1);
  _tpool.start();

  _tpool.exec(std::bind(&KT_HttpAsync::run, this));
}

void KT_HttpAsync::waitForAllDone(int millsecond) {
  time_t now = TNOW;

  while (!_data->empty()) {
    if (millsecond < 0) {
      KT_ThreadLock::Lock lock(*this);
      timedWait(100);
      continue;
    }

    {
      //等待100ms
      KT_ThreadLock::Lock lock(*this);
      timedWait(100);
    }

    if ((TNOW - now) >= (millsecond / 1000)) break;
  }

  terminate();
}

void KT_HttpAsync::erase(uint32_t uniqId) { _erases.push_back(uniqId); }

void KT_HttpAsync::terminate() {
  _epoller.terminate();

  _tpool.waitForAllDone();
}

void KT_HttpAsync::timeout(AsyncRequestPtr &ptr) {
  if (ptr->isValid()) {
    ptr->timeout();
  }
}

void KT_HttpAsync::doAsyncRequest(KT_HttpRequest &stHttpRequest, RequestCallbackPtr &callbackPtr,
                                  const KT_Endpoint &ep) {
  AsyncRequestPtr req = std::make_shared<AsyncRequest>();

  req->initialize(&_epoller, ep, stHttpRequest, callbackPtr);

  if (_bindAddr.first) {
    req->setBindAddr(_bindAddr);
  }

  uint32_t uniqId = _data->generateId();

  req->setUniqId(uniqId);

  req->setHttpAsync(this);

  _data->push(req, uniqId);

  {
    std::lock_guard<std::mutex> lock(_mutex);
    _events.push_back(uniqId);
  }

  _epoller.notify();
}

void KT_HttpAsync::doAsyncRequest(KT_HttpRequest &stHttpRequest, RequestCallbackPtr &callbackPtr, bool bUseProxy) {
  KT_Endpoint ep;

  if (bUseProxy && _proxyEp) {
    ep = *this->_proxyEp.get();
  } else {
    string sHost;
    uint32_t iPort;
    stHttpRequest.getHostPort(sHost, iPort);
    ep.setHost(sHost);
    ep.setPort(iPort);

    if (KT_Port::strcmp(stHttpRequest.getURL().getScheme().c_str(), "https") == 0) {
      ep.setType(KT_Endpoint::SSL);
    }
  }

  doAsyncRequest(stHttpRequest, callbackPtr, ep);
}

void KT_HttpAsync::doAsyncRequest(KT_HttpRequest &stHttpRequest, RequestCallbackPtr &callbackPtr, const string &addr) {
  vector<string> v = KT_Common::sepstr<string>(addr, ":");

  if (v.size() < 2) {
    throw KT_HttpAsync_Exception("[KT_HttpAsync::doAsyncRequest] addr is error:" + addr);
  }

  KT_Endpoint ep;

  ep.setHost(v[0]);
  ep.setPort(KT_Common::strto<uint16_t>(v[1]));

  if (KT_Port::strcmp(stHttpRequest.getURL().getScheme().c_str(), "https") == 0) {
    ep.setType(KT_Endpoint::SSL);
  }

  doAsyncRequest(stHttpRequest, callbackPtr, ep);
}

void KT_HttpAsync::setBindAddr(const char *sBindAddr) { _bindAddr = KT_Socket::createSockAddr(sBindAddr); }

void KT_HttpAsync::setProxyAddr(const KT_Endpoint &ep) {
  _proxyEp.reset(new KT_Endpoint());

  *_proxyEp.get() = ep;
}

void KT_HttpAsync::setProxyAddr(const char *sProxyAddr) {
  vector<string> v = KT_Common::sepstr<string>(sProxyAddr, ":");

  if (v.size() < 2) {
    throw KT_HttpAsync_Exception("[KT_HttpAsync::setProxyAddr] addr is error:" + string(sProxyAddr));
  }

  KT_Endpoint ep;
  ep.setHost(v[0]);
  ep.setPort(KT_Common::strto<uint16_t>(v[1]));

  return setProxyAddr(ep);
}

void KT_HttpAsync::setProxyAddr(const char *sHost, uint16_t iPort) {
  KT_Endpoint ep;
  ep.setHost(sHost);
  ep.setPort(iPort);

  return setProxyAddr(ep);
}

bool KT_HttpAsync::handleCloseImp(const shared_ptr<KT_Epoller::EpollInfo> &data) {
  AsyncRequest *asyncRequest = (AsyncRequest *)data->cookie();

  asyncRequest->doException(RequestCallback::Failed_Net, asyncRequest->getError("epoller error"));

  asyncRequest->trans()->close();

  return false;
}

bool KT_HttpAsync::handleInputImp(const shared_ptr<KT_Epoller::EpollInfo> &data) {
  AsyncRequest *asyncRequest = (AsyncRequest *)data->cookie();

  try {
    asyncRequest->trans()->doResponse();
  } catch (const std::exception &e) {
    asyncRequest->doException(RequestCallback::Failed_Net, e.what());
    return false;
  }

  return true;
}

bool KT_HttpAsync::handleOutputImp(const shared_ptr<KT_Epoller::EpollInfo> &data) {
  AsyncRequest *asyncRequest = (AsyncRequest *)data->cookie();

  try {
    asyncRequest->trans()->doRequest();
  } catch (const std::exception &e) {
    asyncRequest->doException(RequestCallback::Failed_Net, e.what());
    return false;
  }

  return true;
}

void KT_HttpAsync::addFd(AsyncRequest *asyncRequest) {
  shared_ptr<KT_Epoller::EpollInfo> epollInfo = asyncRequest->trans()->getEpollInfo();

  epollInfo->cookie(asyncRequest);

  map<uint32_t, KT_Epoller::EpollInfo::EVENT_CALLBACK> callbacks;

  callbacks[EPOLLIN] = std::bind(&KT_HttpAsync::handleInputImp, this, std::placeholders::_1);
  callbacks[EPOLLOUT] = std::bind(&KT_HttpAsync::handleOutputImp, this, std::placeholders::_1);
  callbacks[EPOLLERR] = std::bind(&KT_HttpAsync::handleCloseImp, this, std::placeholders::_1);

  epollInfo->registerCallback(callbacks, EPOLLIN | EPOLLOUT);
}

void KT_HttpAsync::run() {
  _threadId = std::this_thread::get_id();

  KT_TimeoutQueue<AsyncRequestPtr>::data_functor df(&KT_HttpAsync::timeout);

  _epoller.postRepeated(100, false, [&]() { _data->timeout(df); });

  _epoller.idle([&] {
    deque<uint64_t> events;

    {
      std::lock_guard<std::mutex> lock(_mutex);
      _events.swap(events);
    }

    for (auto data : events) {
      uint32_t uniqId = (uint32_t)data;

      AsyncRequestPtr ptr = _data->getAndRefresh(uniqId);
      if (!ptr) {
        continue;
      }

      try {
        ptr->trans()->connect();
      } catch (exception &ex) {
        ptr->doException(RequestCallback::Failed_Connect, ex.what());
      }
    }

    for (auto it : _erases) {
      _data->erase(it);
    }
    _erases.clear();
  });

  _epoller.loop();
}

}  // namespace kant
