/**
 * Tencent is pleased to support the open source community by making Kant available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#include "servant/ServantProxy.h"
#include "servant/ServantHandle.h"
#include "servant/StatReport.h"
#include "servant/Application.h"
#include "servant/BaseF.h"
#include "servant/CommunicatorEpoll.h"
#include "servant/EndpointManager.h"
#include "servant/Message.h"
#include "servant/ObjectProxy.h"
#include "servant/RemoteLogger.h"

namespace kant {

shared_ptr<ServantProxyThreadData::Immortal> ServantProxyThreadData::g_immortal;

thread_local shared_ptr<ServantProxyThreadData> ServantProxyThreadData::g_sp;
unsigned int ServantProxyThreadData::_traceParamMaxLen = 1;  // 默认1K

///////////////////////////////////////////////////////////////
SeqManager::SeqManager(uint16_t iNum) {
  assert(iNum < MAX_UNSIGN_SHORT);
  assert(iNum > 0);
  _p = NULL;
  _p = new SeqInfo[iNum];
  assert(_p);

  //0xff表示结束
  _free = 0;
  _freeTail = iNum - 1;

  for (uint16_t i = 0; i < (uint16_t)iNum; i++) {
    _p[i].free = true;
    _p[i].next = i + 1;
  }
  _p[iNum - 1].next = MAX_UNSIGN_SHORT;
  _num = iNum;
}

SeqManager ::~SeqManager() {
  if (_p) {
    delete[] _p;
    _p = NULL;
  }
}

uint16_t SeqManager::get() {
  KT_LockT<KT_SpinLock> lock(_mutex);

  assert(_free != MAX_UNSIGN_SHORT);

  assert(_p[_free].free);

  uint16_t buf = _free;

  if (_free == _freeTail) {
    assert(_p[buf].next == MAX_UNSIGN_SHORT);
    _freeTail = MAX_UNSIGN_SHORT;
  }

  _free = _p[buf].next;

  _p[buf].free = false;

  return buf;
}

void SeqManager::del(uint16_t iSeq) {
  KT_LockT<KT_SpinLock> lock(_mutex);

  assert(iSeq < _num);
  assert(!_p[iSeq].free);

  _p[iSeq].next = MAX_UNSIGN_SHORT;
  if (MAX_UNSIGN_SHORT == _freeTail) {
    _free = iSeq;
  } else {
    _p[_freeTail].next = iSeq;
  }
  _p[iSeq].free = true;
  _freeTail = iSeq;
}

///////////////////////////////////////////////////////////////

ServantProxyThreadData::Immortal::Immortal() { _pSeq.reset(new SeqManager(MAX_CLIENT_NOTIFYEVENT_NUM)); }

ServantProxyThreadData::Immortal::~Immortal() { _pSeq.reset(); }

void ServantProxyThreadData::Immortal::add(ServantProxyThreadData *data) {
  KT_LockT<KT_ThreadMutex> lock(_mutex);
  _sp_list.insert(data);
}

void ServantProxyThreadData::Immortal::erase(ServantProxyThreadData *data) {
  KT_LockT<KT_ThreadMutex> lock(_mutex);
  _sp_list.erase(data);
}

unordered_set<ServantProxyThreadData *> ServantProxyThreadData::Immortal::getList() {
  KT_LockT<KT_ThreadMutex> lock(_mutex);
  return _sp_list;
}

void ServantProxyThreadData::Immortal::erase(Communicator *comm) {
  KT_LockT<KT_ThreadMutex> lock(_mutex);

  for (auto it : _sp_list) {
    (*it).erase(comm);
  }
}

ServantProxyThreadData::ServantProxyThreadData() : _reqQNo(0) {
  //     LOG_CONSOLE_DEBUG << endl;

  _sp_immortal = g_immortal;

  _reqQNo = _sp_immortal->getSeqManager()->get();

  _sp_immortal->add(this);
}

ServantProxyThreadData::~ServantProxyThreadData() {
  //     LOG_CONSOLE_DEBUG << endl;
  try {
    //		KT_LockT<KT_SpinLock> lock(_mutex);

    //先释放公有的网络通信器的信息
    for (auto it = _communicatorEpollInfo.begin(); it != _communicatorEpollInfo.end(); ++it) {
      for (auto &e : it->second->_info) {
        shared_ptr<CommunicatorEpoll> ce = e._communicatorEpoll.lock();
        if (ce) {
          ce->notifyServantThreadQuit(_reqQNo);
        }
      }
    }

    for (auto it = _schedCommunicatorEpollInfo.begin(); it != _schedCommunicatorEpollInfo.end(); ++it) {
      it->second->_communicator->eraseSchedCommunicatorEpoll(_reqQNo);
    }

  } catch (...) {
  }

  _sp_immortal->erase(this);

  _sp_immortal->getSeqManager()->del(_reqQNo);

  _sched.reset();

  _sp_immortal.reset();
}

ServantProxyThreadData *ServantProxyThreadData::getData() {
  static std::once_flag flag;
  std::call_once(flag, []() { g_immortal = std::make_shared<Immortal>(); });

  if (!g_sp) {
    g_sp = std::make_shared<ServantProxyThreadData>();
  }
  return g_sp.get();
}

void ServantProxyThreadData::deconstructor(Communicator *communicator) {
  if (g_immortal.use_count() > 0) {
    g_immortal->erase(communicator);
  }
}

void ServantProxyThreadData::erase(Communicator *communicator) {
  KT_LockT<KT_ThreadMutex> lock(_mutex);

  _communicatorEpollInfo.erase(communicator);
  _schedCommunicatorEpollInfo.erase(communicator);
}

shared_ptr<ServantProxyThreadData::CommunicatorEpollInfo> ServantProxyThreadData::getCommunicatorEpollInfo(
  Communicator *communicator) {
  KT_LockT<KT_ThreadMutex> lock(_mutex);

  auto it = _communicatorEpollInfo.find(communicator);

  if (it != _communicatorEpollInfo.end()) {
    return it->second;
  }

  auto info = std::make_shared<CommunicatorEpollInfo>();

  _communicatorEpollInfo.insert(std::make_pair(communicator, info));

  return info;
}

shared_ptr<ServantProxyThreadData::SchedCommunicatorEpollInfo> ServantProxyThreadData::getSchedCommunicatorEpollInfo(
  Communicator *communicator) {
  KT_LockT<KT_ThreadMutex> lock(_mutex);

  auto it = _schedCommunicatorEpollInfo.find(communicator);

  if (it != _schedCommunicatorEpollInfo.end()) {
    return it->second;
  }

  auto info = std::make_shared<SchedCommunicatorEpollInfo>();

  _schedCommunicatorEpollInfo.insert(std::make_pair(communicator, info));

  return info;
}

ThreadPrivateData ServantProxyThreadData::move() {
  ThreadPrivateData data = _data;

  //hash每次调用完成都要清掉，不用透传
  _data._hash = false;
  _data._conHash = false;
  _data._timeout = 0;

  return data;
}

shared_ptr<ServantProxyThreadData::CommunicatorEpollInfo> ServantProxyThreadData::addCommunicatorEpoll(
  const shared_ptr<CommunicatorEpoll> &ce) {
  auto q = std::make_shared<ReqInfoQueue>(ce->getNoSendQueueLimit());

  ServantProxyThreadData::CommunicatorEpollReqQueueInfo epollReqQueueInfo;
  epollReqQueueInfo._reqQueue = q;
  epollReqQueueInfo._communicatorEpoll = ce;

  auto info = getCommunicatorEpollInfo(ce->getCommunicator());

  info->_info.push_back(epollReqQueueInfo);

  ce->initNotify(this->_reqQNo, q);

  return info;
}

///////////////////////////////////////////////////////////////////////////////////////////

ServantProxyCallback::ServantProxyCallback() : _bNetThreadProcess(false) {}

int ServantProxyCallback::dispatch(ReqMessagePtr msg) { return onDispatch(msg); }

///////////////////////////////////////////////////////////////////////////////////////////

int HttpServantProxyCallback::onDispatch(ReqMessagePtr msg) {
  if (msg->response->iRet != KANTSERVERSUCCESS) {
    return onDispatchException(msg->request, *msg->response);
  }

  return onDispatchResponse(msg->request, *msg->response);
}

HttpServantProxyCallback::HttpServantProxyCallback(const HttpCallbackPtr &cb) : _httpCb(cb) {}

int HttpServantProxyCallback::onDispatchException(const RequestPacket &request, const ResponsePacket &response) {
  if (_httpCb) return _httpCb->onHttpResponseException(response.iRet);

  return 0;
}

int HttpServantProxyCallback::onDispatchResponse(const RequestPacket &request, const ResponsePacket &response) {
  assert(response.sBuffer.size() == sizeof(shared_ptr<KT_HttpResponse>));

  shared_ptr<KT_HttpResponse> rsp = *(shared_ptr<KT_HttpResponse> *)(response.sBuffer.data());

  if (_httpCb) return _httpCb->onHttpResponse(rsp);

  return 0;
}

///////////////////////////////////////////////////////////////
void coroWhenAll(const CoroParallelBasePtr &ptr) {
  if (!ptr->checkAllReqSend()) {
    TLOGERROR("[coroWhenAll use coro invoke interface's num not equal ptr set value]" << endl);
    throw KantUseCoroException("use coro invoke interface's num not equal ptr set value");
  }

  //线程私有数据
  ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();
  assert(pSptd != NULL);

  if (!pSptd->_sched) {
    TLOGERROR("[coroWhenAll no open coroutine mode]" << endl);
    throw KantUseCoroException("coroWhenAll not open coroutine mode");
  }

  pSptd->_sched->yield(false);

  vector<ReqMessage *> vMsg = ptr->getAllReqMessage();

  for (size_t i = 0; i < vMsg.size(); ++i) {
    ReqMessagePtr msgPtr = std::shared_ptr<ReqMessage>(vMsg[i]);
    vMsg[i]->callback->dispatch(msgPtr);
  }
}

///////////////////////////////////////////////////////////////
string ServantProxy::STATUS_DYED_KEY = "STATUS_DYED_KEY";

string ServantProxy::STATUS_GRID_KEY = "STATUS_GRID_KEY";

string ServantProxy::STATUS_RESULT_CODE = "STATUS_RESULT_CODE";

string ServantProxy::STATUS_RESULT_DESC = "STATUS_RESULT_DESC";

string ServantProxy::STATUS_SETNAME_VALUE = "STATUS_SETNAME_VALUE";

string ServantProxy::STATUS_TRACE_KEY = "STATUS_TRACE_KEY";

////////////////////////////////////
ServantProxy::ServantProxy(Communicator *pCommunicator, const string &name, const string &setName)
  : _communicator(pCommunicator),
    _syncTimeout(DEFAULT_SYNCTIMEOUT),
    _asyncTimeout(DEFAULT_ASYNCTIMEOUT),
    _id(0),
    _masterFlag(false),
    _minTimeout(100) {
  _proxyProtocol.requestFunc = ProxyProtocol::kantRequest;
  _proxyProtocol.responseFunc = ProxyProtocol::kantResponse;

  //在每个公有网络线程对象中创建ObjectProxy
  for (size_t i = 0; i < _communicator->getCommunicatorEpollNum(); ++i) {
    _communicator->getCommunicatorEpoll(i)->createObjectProxy(this, name, setName);
  }

  //用第一个ObjectProxy返回数据
  _objectProxy = this->getObjectProxy(0);

  _endpointInfo.reset(new EndpointManagerThread(_communicator, _objectProxy->name()));

  _minTimeout = pCommunicator->getMinTimeout();
  if (_minTimeout < 1) {
    _minTimeout = 1;
  }
}

void ServantProxy::kant_initialize(bool rootServant) {
  //等ServantProxy完全创建完以后, 再创建Object
  for (size_t i = 0; i < _communicator->getCommunicatorEpollNum(); ++i) {
    this->getObjectProxy(i)->initialize(rootServant);
  }
}

ServantProxy::~ServantProxy() {}

const string &ServantProxy::kant_name() const { return _objectProxy->name(); }

string ServantProxy::kant_full_name() const {
  string name = _objectProxy->name();

  if (!_objectProxy->hash().empty()) {
    name = name + "#" + _objectProxy->hash();
  }
  if (!_objectProxy->address().empty()) {
    name = name + "@" + _objectProxy->address();
  }

  return name;
}

const string &ServantProxy::kant_setName() const { return _objectProxy->getInvokeSetName(); }

ObjectProxy *ServantProxy::getObjectProxy(size_t netThreadSeq) {
  return _communicator->getCommunicatorEpoll(netThreadSeq)->servantToObjectProxy(this);
}

void ServantProxy::forEachObject(std::function<void(ObjectProxy *)> func) {
  for (size_t i = 0; i < _communicator->getCommunicatorEpollNum(); ++i) {
    ObjectProxy *objectProxy = _communicator->getCommunicatorEpoll(i)->servantToObjectProxy(this);
    assert(objectProxy != NULL);

    try {
      func(objectProxy);
    } catch (...) {
    }
  }
}

void ServantProxy::kant_reconnect(int second) {
  forEachObject([=](ObjectProxy *o) { o->reconnect(second); });
}

KT_Endpoint ServantProxy::kant_invoke_endpoint() {
  ServantProxyThreadData *td = ServantProxyThreadData::getData();

  if (td) {
    return KT_Endpoint(td->_data._szHost);
  }
  return KT_Endpoint();
}

void ServantProxy::kant_set_proxy(ServantProxy::SERVANT_PROXY type, const KT_Endpoint &ep, const string &user,
                                  const string &pass) {
  _proxyBaseInfo = std::make_shared<KT_ProxyInfo::ProxyBaseInfo>();
  _proxyBaseInfo->ep = ep;
  _proxyBaseInfo->user = user;
  _proxyBaseInfo->pass = pass;

  switch (type) {
    case PROXY_SOCK4:
      _proxyBaseInfo->type = KT_ProxyInfo::eProxy_Type_Sock4;
      break;
    case PROXY_SOCK5:
      _proxyBaseInfo->type = KT_ProxyInfo::eProxy_Type_Sock5;
      break;
    case PROXY_HTTP:
      _proxyBaseInfo->type = KT_ProxyInfo::eProxy_Type_Http;
      break;
    default:
      assert(false);
  }
}

void ServantProxy::kant_timeout(int msecond) {
  //保护，超时时间不能小于_minTimeout毫秒
  _syncTimeout = (msecond < _minTimeout) ? _minTimeout : msecond;
}

int ServantProxy::kant_timeout() const { return _syncTimeout; }

int ServantProxy::kant_connect_timeout() const { return _connTimeout; }

void ServantProxy::kant_connect_timeout(int conTimeout) {
  if (conTimeout < 100) {
    conTimeout = 100;
  }

  _connTimeout = conTimeout;
}

void ServantProxy::kant_async_timeout(int msecond) {
  KT_LockT<KT_ThreadMutex> lock(*this);
  //保护，超时时间不能小于_minTimeout毫秒
  _asyncTimeout = (msecond < _minTimeout) ? _minTimeout : msecond;
}

int ServantProxy::kant_async_timeout() const { return _asyncTimeout; }

void ServantProxy::kant_set_custom_callback(ServantProxy::custom_callback callback) { _callback = callback; }

void ServantProxy::kant_enable_callback_hash() { _callbackHash = true; }

void ServantProxy::kant_connection_serial(int connectionSerial) {
  assert(!_rootPrx);
  _connectionSerial = connectionSerial;
}

int ServantProxy::kant_connection_serial() const {
  if (_rootPrx) {
    return _rootPrx->kant_connection_serial();
  }

  return _connectionSerial;
}

void ServantProxy::kant_set_protocol(SERVANT_PROTOCOL protocol, int connectionSerial) {
  ProxyProtocol proto;

  switch (protocol) {
    case PROTOCOL_HTTP1:
      proto.requestFunc = ProxyProtocol::http1Request;
      proto.responseFunc = ProxyProtocol::http1Response;

      if (connectionSerial <= 0) connectionSerial = DEFAULT_CONNECTION_SERIAL;
      break;
#if KANT_HTTP2
    case PROTOCOL_HTTP2:
      proto.requestFunc = ProxyProtocol::http2Request;
      proto.responseFunc = ProxyProtocol::http2Response;
      connectionSerial = 0;
      break;
    case PROTOCOL_GRPC:
      proto.requestFunc = ProxyProtocol::grpcRequest;
      proto.responseFunc = ProxyProtocol::grpcResponse;
      connectionSerial = 0;
      break;
#endif
    case PROTOCOL_KANT:
    default:
      proto.requestFunc = ProxyProtocol::kantRequest;
      proto.responseFunc = ProxyProtocol::kantResponse;
      break;
  }
  kant_set_protocol(proto, connectionSerial);
}

void ServantProxy::kant_set_protocol(const ProxyProtocol &protocol, int connectionSerial) {
  KT_LockT<KT_ThreadMutex> lock(*this);

  _proxyProtocol = protocol;

  _connectionSerial = connectionSerial;
}

const ProxyProtocol &ServantProxy::kant_get_protocol() const { return _proxyProtocol; }

vector<ServantProxy::SocketOpt> ServantProxy::kant_get_sockopt() const {
  KT_LockT<KT_ThreadMutex> lock(*this);

  return _socketOpts;
}

void ServantProxy::kant_set_sockopt(int level, int optname, const void *optval, SOCKET_LEN_TYPE optlen) {
  KT_LockT<KT_ThreadMutex> lock(*this);

  SocketOpt socketOpt;

  socketOpt.level = level;
  socketOpt.optname = optname;
  socketOpt.optval = optval;
  socketOpt.optlen = optlen;

  _socketOpts.push_back(socketOpt);
}

void ServantProxy::kant_set_check_timeout(const CheckTimeoutInfo &checkTimeoutInfo) {
  KT_LockT<KT_ThreadMutex> lock(*this);

  _checkTimeoutInfo = checkTimeoutInfo;
}

CheckTimeoutInfo ServantProxy::kant_get_check_timeout() {
  KT_LockT<KT_ThreadMutex> lock(*this);

  return _checkTimeoutInfo;
}

void ServantProxy::kant_ping() {
  map<string, string> m;

  map<string, string> s;

  KantOutputStream<BufferWriterVector> os;

  kant_invoke(KANTNORMAL, "kant_ping", os, m, s);
}

void ServantProxy::kant_async_ping() {
  map<string, string> m;
  map<string, string> s;

  KantOutputStream<BufferWriterVector> os;

  kant_invoke_async(KANTONEWAY, "kant_ping", os, m, s, NULL);
}

ServantProxy *ServantProxy::kant_hash(int64_t key) {
  ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();

  assert(pSptd != NULL);

  pSptd->_data._hash = true;
  pSptd->_data._hashCode = key;

  return this;
}

//ServantProxy* ServantProxy::kant_direct()
//{
//	ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();
//
//	pSptd->_direct = true;
//	return this;
//}

ServantProxy *ServantProxy::kant_consistent_hash(int64_t key) {
  ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();

  assert(pSptd != NULL);

  pSptd->_data._hash = true;
  pSptd->_data._conHash = true;
  pSptd->_data._hashCode = key;

  return this;
}

ServantProxy *ServantProxy::kant_open_trace(bool traceParams) {
  ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();

  assert(pSptd != NULL);
  if (traceParams) {
    pSptd->openTrace(15);
  } else {
    pSptd->openTrace();
  }

  return this;
}

void ServantProxy::kant_clear_hash() {}

ServantProxy *ServantProxy::kant_set_timeout(int msecond) {
  ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();
  assert(pSptd != NULL);

  //     pSptd->_hasTimeout = true;
  pSptd->_data._timeout = msecond;

  return this;
}

uint32_t ServantProxy::kant_gen_requestid() {
  uint32_t i = ++_id;
  if (i == 0) {
    i = ++_id;
  }

  return i;
}

void ServantProxy::kant_set_push_callback(const ServantProxyCallbackPtr &cb) { _pushCallback = cb; }

ServantProxyCallbackPtr ServantProxy::kant_get_push_callback() { return _pushCallback; }

void ServantProxy::invoke(ReqMessage *msg, bool bCoroAsync) {
  //线程私有数据
  ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();
  assert(pSptd != NULL);

  //协程调用方式, 启用协程
  if (bCoroAsync && KT_CoroutineScheduler::scheduler() && !pSptd->_sched) {
    pSptd->_sched = KT_CoroutineScheduler::scheduler();
  }

  msg->data = pSptd->move();

  // 调用链追踪透传
  msg->bTraceCall = pSptd->_traceCall;
  msg->sTraceKey = pSptd->getTraceKey(true);

  if (msg->data._dyeing) {
    TLOGKANT("[ServantProxy::invoke, set dyeing, key=" << pSptd->_data._dyeingKey << endl);
  }
  msg->data._cookie = pSptd->_data._cookie;

  // #ifdef KANT_OPENTRACKING
  //     msg->trackInfoMap = pSptd->_trackInfoMap;
  // #endif
  //
  //设置超时时间
  msg->request.iTimeout = (ReqMessage::SYNC_CALL == msg->eType) ? _syncTimeout : _asyncTimeout;

  //判断是否针对接口级设置超时
  if (msg->data._timeout > 0) {
    msg->request.iTimeout = msg->data._timeout;
  } else {
    msg->request.iTimeout = (ReqMessage::SYNC_CALL == msg->eType) ? _syncTimeout : _asyncTimeout;
  }

  shared_ptr<ReqInfoQueue> pReqQ;

  //选择网络线程
  selectNetThreadInfo(pSptd, msg->pObjectProxy, pReqQ);

  //如果是按set规则调用
  if (msg->pObjectProxy && msg->pObjectProxy->isInvokeBySet()) {
    SET_MSG_TYPE(msg->request.iMessageType, KANTMESSAGETYPESETNAME);
    msg->request.status[ServantProxy::STATUS_SETNAME_VALUE] = msg->pObjectProxy->getInvokeSetName();

    TLOGKANT("[ServantProxy::invoke, " << msg->request.sServantName << ", invoke with set,"
                                       << msg->pObjectProxy->getInvokeSetName() << "]" << endl);
  }

  assert(msg->pMonitor == NULL);
  if (msg->eType == ReqMessage::SYNC_CALL) {
    if (pSptd->_sched) {
      msg->sched = pSptd->_sched;
      msg->iCoroId = pSptd->_sched->getCoroutineId();
    } else {
      //同步调用 new 一个ReqMonitor
      msg->pMonitor = new ReqMonitor(msg);
    }
  } else if (msg->eType == ReqMessage::ASYNC_CALL) {
    //是否是协程的并行请求
    if (bCoroAsync) {
      if (pSptd->_sched) {
        CoroParallelBasePtr coroPtr = msg->callback->getCoroParallelBasePtr();
        if (coroPtr) {
          coroPtr->incReqCount();

          msg->sched = pSptd->_sched;
          msg->iCoroId = pSptd->_sched->getCoroutineId();
        } else {
          TLOGERROR("[ServantProxy::invoke use coroutine's callback not set CoroParallelBasePtr]" << endl);
          delete msg;
          msg = NULL;
          throw KantUseCoroException("use coroutine's callback not set CoroParallelBasePtr");
        }
      } else {
        TLOGERROR("[ServantProxy::invoke coroutine mode invoke not open]" << endl);

        delete msg;
        msg = NULL;
        throw KantUseCoroException("coroutine mode invoke not open");
      }
    }
  }

  //通知网络线程
  bool bEmpty = false;
  bool bSync = (msg->eType == ReqMessage::SYNC_CALL);

  if (!pReqQ->push_back(msg, bEmpty)) {
    TLOGERROR("[ServantProxy::invoke msgQueue push_back error thread seq:" << pSptd->_reqQNo << "]" << endl);

    delete msg;
    msg = NULL;

    throw KantClientQueueException("client queue full");
  }

  if (msg->sched) {
    //		LOG_CONSOLE_DEBUG << "in sched handle: " << this << ", " << msg->request.sServantName << endl;

    //协程中, 直接发包了
    msg->pObjectProxy->getCommunicatorEpoll()->handle(pSptd->_reqQNo);
  } else {
    msg->pObjectProxy->getCommunicatorEpoll()->notify(pSptd->_reqQNo);
  }

  if (bSync) {
    if (!msg->sched) {
      assert(msg->pMonitor);

      msg->pMonitor->wait();

      if (!msg->pMonitor->bMonitorFin) {
        TLOGERROR("[ServantProxy::invoke communicator terminate]" << endl);
        throw KantCommunicatorException("communicator terminate");
      }
    } else {
      assert(!msg->sched->isMainCoroutine());
      msg->sched->yield(false);
    }

    //判断eStatus来判断状态
    //        assert(msg->eStatus != ReqMessage::REQ_REQ);

    if (msg->adapter) {
      pSptd->_data._szHost = msg->adapter->endpoint().desc();
    }

    if (msg->eStatus == ReqMessage::REQ_RSP && msg->response->iRet == KANTSERVERSUCCESS) {
      //成功
      return;
    }

    ostringstream os;
    if (msg->eStatus == ReqMessage::REQ_TIME) {
      //超时
      os << "[ServantProxy::invoke timeout:" << msg->request.iTimeout;
    } else {
      os << "[ServantProxy::invoke errno:" << msg->response->iRet << ",info:" << msg->response->sResultDesc;
    }

    os << ",servant:" << msg->pObjectProxy->name() << ",func:" << msg->request.sFuncName;

    if (msg->adapter) os << ",adapter:" << msg->adapter->endpoint().desc();

    os << ",reqid:" << msg->request.iRequestId << "]";

    if (msg->eStatus == ReqMessage::REQ_TIME) {
      //超时
      delete msg;
      msg = NULL;

      throw KantSyncCallTimeoutException(os.str());
    }

    //异常调用
    int ret = msg->response->iRet;

    delete msg;
    msg = NULL;

    KantException::throwException(ret, os.str());
  }
}

//////////////////////////////////////////////////////////////////
void ServantProxy::kant_invoke_async(char cPacketType, const string &sFuncName,
                                     KantOutputStream<BufferWriterVector> &buf, const map<string, string> &context,
                                     const map<string, string> &status, const ServantProxyCallbackPtr &callback,
                                     bool bCoro) {
  ReqMessage *msg = new ReqMessage();

  msg->init(callback ? ReqMessage::ASYNC_CALL : ReqMessage::ONE_WAY, this);
  msg->callback = callback;

  msg->request.iVersion = KANTVERSION;
  msg->request.cPacketType = (callback ? cPacketType : KANTONEWAY);
  msg->request.sFuncName = sFuncName;
  msg->request.sServantName = _objectProxy->name();

  buf.swap(msg->request.sBuffer);

  msg->request.context = context;
  msg->request.status = status;
  msg->request.iTimeout = _asyncTimeout;

  //    // 在RequestPacket中的context设置主调信息
  //    if(_masterFlag)
  //    {
  //        msg->request.context.insert(std::make_pair(KANT_MASTER_KEY,ClientConfig::ModuleName)); //KANT_MASTER_KEY  clientConfig.ModuleName
  //    }

  checkDye(msg->request);
  checkTrace(msg->request);
  checkCookie(msg->request);
  servant_invoke(msg, bCoro);
}

//////////////////////////////////////////////////////////////////
void ServantProxy::kant_invoke_async(char cPacketType, const string &sFuncName, const vector<char> &buf,
                                     const map<string, string> &context, const map<string, string> &status,
                                     const ServantProxyCallbackPtr &callback, bool bCoro) {
  ReqMessage *msg = new ReqMessage();

  msg->init(callback ? ReqMessage::ASYNC_CALL : ReqMessage::ONE_WAY, this);
  msg->callback = callback;

  msg->request.iVersion = KANTVERSION;
  msg->request.cPacketType = (callback ? cPacketType : KANTONEWAY);
  msg->request.sFuncName = sFuncName;
  msg->request.sServantName = _objectProxy->name();
  msg->request.sBuffer = buf;
  msg->request.context = context;
  msg->request.status = status;
  msg->request.iTimeout = _asyncTimeout;

  //    // 在RequestPacket中的context设置主调信息
  //    if(_masterFlag)
  //    {
  //        msg->request.context.insert(std::make_pair(KANT_MASTER_KEY,ClientConfig::ModuleName)); //KANT_MASTER_KEY  clientConfig.ModuleName
  //    }

  checkDye(msg->request);
  checkTrace(msg->request);
  checkCookie(msg->request);
  servant_invoke(msg, bCoro);
}

shared_ptr<ResponsePacket> ServantProxy::kant_invoke(char cPacketType, const string &sFuncName, const vector<char> &buf,
                                                     const map<string, string> &context,
                                                     const map<string, string> &status)
//   ResponsePacket& rsp)
{
  ReqMessage *msg = new ReqMessage();

  msg->init(ReqMessage::SYNC_CALL, this);

  msg->request.iVersion = KANTVERSION;
  msg->request.cPacketType = cPacketType;
  msg->request.sFuncName = sFuncName;
  msg->request.sServantName = _objectProxy->name();

  msg->request.sBuffer = buf;
  msg->request.context = context;
  msg->request.status = status;
  msg->request.iTimeout = _syncTimeout;

  //    // 在RequestPacket中的context设置主调信息
  //    if(_masterFlag)
  //    {
  //        msg->request.context.insert(std::make_pair(KANT_MASTER_KEY,ClientConfig::ModuleName));
  //    }

  checkDye(msg->request);
  checkTrace(msg->request);
  checkCookie(msg->request);
  servant_invoke(msg, false);

  shared_ptr<ResponsePacket> rsp = msg->response;
  // rsp = msg->response;

  delete msg;
  msg = NULL;

  return rsp;
}

shared_ptr<ResponsePacket> ServantProxy::kant_invoke(char cPacketType, const string &sFuncName,
                                                     KantOutputStream<BufferWriterVector> &buf,
                                                     const map<string, string> &context,
                                                     const map<string, string> &status)
//   ResponsePacket& rsp)
{
  ReqMessage *msg = new ReqMessage();

  msg->init(ReqMessage::SYNC_CALL, this);

  msg->request.iVersion = KANTVERSION;
  msg->request.cPacketType = cPacketType;
  msg->request.sFuncName = sFuncName;
  msg->request.sServantName = _objectProxy->name();

  buf.swap(msg->request.sBuffer);
  msg->request.context = context;
  msg->request.status = status;
  msg->request.iTimeout = _syncTimeout;

  //    // 在RequestPacket中的context设置主调信息
  //    if(_masterFlag)
  //    {
  //        msg->request.context.insert(std::make_pair(KANT_MASTER_KEY,ClientConfig::ModuleName));
  //    }

  checkDye(msg->request);
  checkTrace(msg->request);
  checkCookie(msg->request);
  servant_invoke(msg, false);

  shared_ptr<ResponsePacket> rsp = msg->response;
  // rsp = msg->response;

  delete msg;
  msg = NULL;

  return rsp;
}
//////////////////////////////////////////////////////////////////////////////
//服务端是非kant协议，通过rpc_call调用
void ServantProxy::rpc_call(uint32_t iRequestId, const string &sFuncName, const char *buff, uint32_t len,
                            ResponsePacket &rsp) {
  ReqMessage *msg = new ReqMessage();

  msg->init(ReqMessage::SYNC_CALL, this);
  msg->bFromRpc = true;
  msg->request.sServantName = _objectProxy->name();
  msg->request.sFuncName = sFuncName;
  msg->request.iRequestId = iRequestId;

  msg->request.sBuffer.assign(buff, buff + len);

  servant_invoke(msg, false);

  rsp = *msg->response.get();

  delete msg;
  msg = NULL;
}

void ServantProxy::rpc_call_async(uint32_t iRequestId, const string &sFuncName, const char *buff, uint32_t len,
                                  const ServantProxyCallbackPtr &callback, bool bCoro) {
  ReqMessage *msg = new ReqMessage();

  msg->init(callback ? ReqMessage::ASYNC_CALL : ReqMessage::ONE_WAY, this);

  msg->bFromRpc = true;
  msg->callback = callback;
  msg->request.sServantName = _objectProxy->name();
  msg->request.sFuncName = sFuncName;

  msg->request.iRequestId = iRequestId;

  msg->request.sBuffer.assign(buff, buff + len);

  servant_invoke(msg, bCoro);
}

ServantPrx ServantProxy::getServantPrx(ReqMessage *msg) {
  if (_servantId == 0) {
    std::lock_guard<std::mutex> m(_servantMutex);

    if (_servantId == 0 && _servantList.empty()) {
      for (int i = 1; i < _connectionSerial; ++i) {
        string obj = kant_name() + "#" + KT_Common::tostr(i);
        if (!_objectProxy->address().empty()) {
          obj += "@" + _objectProxy->address();
        }

        ServantPrx prx = _communicator->stringToProxy<ServantPrx>(obj, this->kant_setName(), false);
        prx->kant_set_protocol(kant_get_protocol());
        prx->kant_connect_timeout(kant_connect_timeout());
        prx->kant_timeout(kant_timeout());
        prx->kant_async_timeout(kant_async_timeout());

        prx->_rootPrx = this->shared_from_this();

        _servantList.push_back(prx);
      }
    }
  }

  if (_connectionSerial != 0) {
    assert(int(_servantList.size() + 1) == _connectionSerial);
  }

  int id = _servantId++ % (_servantList.size() + 1);

  if (id == 0) {
    return this->shared_from_this();
  }

  return _servantList[(id - 1)];
}

void ServantProxy::travelObjectProxys(ServantProxy *prx, function<void(ObjectProxy *)> f) {
  vector<ObjectProxy *> objectProxys;

  size_t num = _communicator->getCommunicatorEpollNum();

  for (size_t i = 0; i < num; ++i) {
    auto ce = _communicator->getCommunicatorEpoll(i);

    ObjectProxy *objectProxy = ce->servantToObjectProxy(prx);

    if (objectProxy) {
      f(objectProxy);
    }
  }

  //协程通信器也需要
  _communicator->forEachSchedCommunicatorEpoll([&](const shared_ptr<CommunicatorEpoll> &ce) {
    ObjectProxy *objectProxy = ce->servantToObjectProxy(prx);

    if (objectProxy) {
      f(objectProxy);
    }
  });
}

vector<ObjectProxy *> ServantProxy::getObjectProxys() {
  vector<ObjectProxy *> objectProxys;

  //更新子servant proxy的地址
  for (size_t i = 0; i < _servantList.size(); i++) {
    ServantProxy *prx = _servantList[i].get();

    travelObjectProxys(prx, [&](ObjectProxy *op) { objectProxys.push_back(op); });
  }

  travelObjectProxys(this, [&](ObjectProxy *op) { objectProxys.push_back(op); });

  return objectProxys;
}

void ServantProxy::kant_update_endpoints(const set<EndpointInfo> &active, const set<EndpointInfo> &inactive) {
  onNotifyEndpoints(NULL, active, inactive);
}

void ServantProxy::onNotifyEndpoints(CommunicatorEpoll *communicatorEpoll, const set<EndpointInfo> &active,
                                     const set<EndpointInfo> &inactive) {
  //更新子servant proxy的地址
  for (size_t i = 0; i < _servantList.size(); i++) {
    ServantProxy *prx = _servantList[i].get();

    travelObjectProxys(prx, [&](ObjectProxy *op) {
      if (op->getEndpointManager()) {
        op->getEndpointManager()->updateEndpointsOutter(active, inactive);
      }
    });
  }

  travelObjectProxys(this, [&](ObjectProxy *op) {
    if (op->getEndpointManager()) {
      op->getEndpointManager()->updateEndpointsOutter(active, inactive);
    }
  });
}

void ServantProxy::onSetInactive(const EndpointInfo &ep) {
  if (!_rootPrx) return;

  for (size_t i = 0; i < _rootPrx->_servantList.size(); i++) {
    ServantPrx &prx = _rootPrx->_servantList[i];

    prx->forEachObject([&](ObjectProxy *o) { o->onSetInactive(ep); });
  }
}

int ServantProxy::servant_invoke(ReqMessage *msg, bool bCoroAsync) {
  ServantPrx prx = getServantPrx(msg);

  if (msg->callback) {
    msg->callback->setServantPrx(prx);
  }

  prx->invoke(msg, bCoroAsync);

  return 0;
}

void ServantProxy::http_call(const string &funcName, shared_ptr<KT_HttpRequest> &request,
                             shared_ptr<KT_HttpResponse> &response) {
  if (_connectionSerial <= 0) {
    _connectionSerial = DEFAULT_CONNECTION_SERIAL;
  }

  ReqMessage *msg = new ReqMessage();

  msg->init(ReqMessage::SYNC_CALL, this);

  msg->bFromRpc = true;

  msg->request.sServantName = _objectProxy->name();
  msg->request.sFuncName = funcName;

  msg->request.sBuffer.resize(sizeof(shared_ptr<KT_HttpRequest>));

  msg->deconstructor = [msg] {
    shared_ptr<KT_HttpRequest> &data = *(shared_ptr<KT_HttpRequest> *)(msg->request.sBuffer.data());
    data.reset();

    if (!msg->response->sBuffer.empty()) {
      shared_ptr<KT_HttpResponse> &rsp = *(shared_ptr<KT_HttpResponse> *)(msg->response->sBuffer.data());
      //主动reset一次
      rsp.reset();

      msg->response->sBuffer.clear();
    }
  };

  shared_ptr<KT_HttpRequest> &data = *(shared_ptr<KT_HttpRequest> *)(msg->request.sBuffer.data());

  data = request;

  servant_invoke(msg, false);

  response = *(shared_ptr<KT_HttpResponse> *)(msg->response->sBuffer.data());

  delete msg;
  msg = NULL;
}

void ServantProxy::http_call_async(const string &funcName, shared_ptr<KT_HttpRequest> &request,
                                   const HttpCallbackPtr &cb, bool bCoro) {
  if (_connectionSerial <= 0) {
    _connectionSerial = DEFAULT_CONNECTION_SERIAL;
  }

  ReqMessage *msg = new ReqMessage();

  msg->init(ReqMessage::ASYNC_CALL, this);

  msg->bFromRpc = true;

  msg->request.sServantName = _objectProxy->name();
  msg->request.sFuncName = funcName;

  msg->request.sBuffer.resize(sizeof(shared_ptr<KT_HttpRequest>));

  msg->deconstructor = [msg] {
    shared_ptr<KT_HttpRequest> &data = *(shared_ptr<KT_HttpRequest> *)(msg->request.sBuffer.data());
    data.reset();

    if (!msg->response->sBuffer.empty()) {
      shared_ptr<KT_HttpResponse> &rsp = *(shared_ptr<KT_HttpResponse> *)(msg->response->sBuffer.data());
      //主动reset一次
      rsp.reset();

      msg->response->sBuffer.clear();
    }
  };

  *(shared_ptr<KT_HttpRequest> *)(msg->request.sBuffer.data()) = request;

  ServantProxyCallbackPtr callback = std::make_shared<HttpServantProxyCallback>(cb);

  msg->callback = callback;

  servant_invoke(msg, bCoro);
}

//
//选取一个网络线程对应的信息
void ServantProxy::selectNetThreadInfo(ServantProxyThreadData *pSptd, ObjectProxy *&pObjProxy,
                                       shared_ptr<ReqInfoQueue> &pReqQ) {
  if (pSptd->_sched && pSptd->_communicatorEpoll == NULL) {
    //处于业务线程中, 且当前业务线程是以协程模式启动;
    auto schedCommunicatorEpollInfo = pSptd->getSchedCommunicatorEpollInfo(_communicator);

    shared_ptr<CommunicatorEpoll> ce;

    if (!schedCommunicatorEpollInfo->_communicator) {
      //当前协程没有关联过私有网络通信器, 需要新建!
      pReqQ = std::make_shared<ReqInfoQueue>(_communicator->getCommunicatorEpoll(0)->getNoSendQueueLimit());
      ce = _communicator->createSchedCommunicatorEpoll(pSptd->_reqQNo, pReqQ);

      schedCommunicatorEpollInfo->_communicator = _communicator;
      schedCommunicatorEpollInfo->_info._reqQueue = pReqQ;
      schedCommunicatorEpollInfo->_info._communicatorEpoll = ce;

      pObjProxy = ce->createObjectProxy(this, this->kant_full_name(), this->kant_setName());

      pObjProxy->initialize(true);
    } else {
      //网络通信器已经初始化过, 直接获取对象
      pReqQ = schedCommunicatorEpollInfo->_info._reqQueue.lock();

      ce = schedCommunicatorEpollInfo->_info._communicatorEpoll.lock();
      if (ce && pReqQ) {
        pObjProxy = ce->hasObjectProxy(this->kant_full_name(), this->kant_setName());

        //创建对应的ObjectProxy
        if (!pObjProxy) {
          pObjProxy = ce->createObjectProxy(this, this->kant_full_name(), this->kant_setName());

          pObjProxy->initialize(true);
        }
      } else {
        //				assert(false);
        throw KantCommunicatorException("communicator may deconstruct");
      }
    }
  } else {
    if (pSptd->_communicatorEpoll) {
      auto info = pSptd->getCommunicatorEpollInfo(_communicator);

      assert(info->_info.size() == 1);
      assert(info->_info[0]._communicatorEpoll.lock().get() == pSptd->_communicatorEpoll);

      pObjProxy = pSptd->_communicatorEpoll->servantToObjectProxy(this);
      pReqQ = info->_info[0]._reqQueue.lock();

    } else {
      //处于普通线程中, 判断当前线程是否关联过网络通信器, 没关联, 则关联所有的公有通信器, 即创建和公有网络通信器的队列!
      auto communicatorEpollInfo = pSptd->getCommunicatorEpollInfo(_communicator);

      //当前线程没有关联业务通信器, 需要关联
      if (!communicatorEpollInfo->_communicator) {
        communicatorEpollInfo->_communicator = _communicator;
        //为每个网络线程都创建一个队列
        for (size_t i = 0; i < _communicator->getCommunicatorEpollNum(); ++i) {
          shared_ptr<CommunicatorEpoll> ce = _communicator->getCommunicatorEpoll(i);

          pSptd->addCommunicatorEpoll(ce);
        }
      }

      assert(communicatorEpollInfo->_netSeq < _communicator->getCommunicatorEpollNum());

      //循环使用下一个网络线程发送数据
      auto ce = communicatorEpollInfo->_info[communicatorEpollInfo->_netSeq]._communicatorEpoll.lock();
      if (ce) {
        pObjProxy = ce->servantToObjectProxy(this);
        pReqQ = communicatorEpollInfo->_info[communicatorEpollInfo->_netSeq]._reqQueue.lock();

        communicatorEpollInfo->_netSeq++;

        if (communicatorEpollInfo->_netSeq == _communicator->getCommunicatorEpollNum())
          communicatorEpollInfo->_netSeq = 0;
      }
    }

    if (!pReqQ) {
      //队列已经析构, 说明通信器已经释放了!
      throw KantCommunicatorException("communicator has deconstructed");
    }
  }
}

void ServantProxy::checkDye(RequestPacket &req) {
  //线程私有数据
  ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();
  assert(pSptd != NULL);
  if (pSptd && pSptd->_data._dyeing) {
    SET_MSG_TYPE(req.iMessageType, KANTMESSAGETYPEDYED);

    req.status[ServantProxy::STATUS_DYED_KEY] = pSptd->_data._dyeingKey;
  }
}

void ServantProxy::checkTrace(RequestPacket &req) {
  //线程私有数据
  ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();
  assert(pSptd != NULL);
  if (pSptd && pSptd->_traceCall) {
    SET_MSG_TYPE(req.iMessageType, kant::KANTMESSAGETYPETRACE);

    req.status[ServantProxy::STATUS_TRACE_KEY] = pSptd->getTraceKey();
  }
}

void ServantProxy::checkCookie(RequestPacket &req) {
  //线程私有数据
  ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();
  assert(pSptd != NULL);

  std::for_each(pSptd->_data._cookie.begin(), pSptd->_data._cookie.end(),
                [&](map<string, string>::value_type &p) { req.status.insert(make_pair(p.first, p.second)); });
}

void ServantProxy::kant_endpoints(vector<EndpointInfo> &activeEndPoint, vector<EndpointInfo> &inactiveEndPoint) {
  _endpointInfo->getEndpoint(activeEndPoint, inactiveEndPoint);
}

void ServantProxy::kant_endpointsAll(vector<EndpointInfo> &activeEndPoint, vector<EndpointInfo> &inactiveEndPoint) {
  _endpointInfo->getEndpointByAll(activeEndPoint, inactiveEndPoint);
}

void ServantProxy::kant_endpointsBySet(const string &sName, vector<EndpointInfo> &activeEndPoint,
                                       vector<EndpointInfo> &inactiveEndPoint) {
  _endpointInfo->getEndpointBySet(sName, activeEndPoint, inactiveEndPoint);
}

void ServantProxy::kant_endpointsByStation(const string &sName, vector<EndpointInfo> &activeEndPoint,
                                           vector<EndpointInfo> &inactiveEndPoint) {
  _endpointInfo->getEndpointByStation(sName, activeEndPoint, inactiveEndPoint);
}

vector<KT_Endpoint> ServantProxy::kant_endpoints(const std::string &sStation) {
  vector<KT_Endpoint> activeEndPoint;
  vector<KT_Endpoint> inactiveEndPoint;
  _endpointInfo->getTCEndpoint(activeEndPoint, inactiveEndPoint);
  return activeEndPoint;
}

void ServantProxy::kant_endpoints(const std::string &sStation, vector<KT_Endpoint> &vecActive,
                                  vector<KT_Endpoint> &vecInactive) {
  _endpointInfo->getTCEndpointByStation(sStation, vecActive, vecInactive);
}

vector<KT_Endpoint> ServantProxy::getEndpoint() {
  vector<KT_Endpoint> activeEndPoint;
  vector<KT_Endpoint> inactiveEndPoint;
  _endpointInfo->getTCEndpoint(activeEndPoint, inactiveEndPoint);
  return activeEndPoint;
}

vector<KT_Endpoint> ServantProxy::getEndpoint4All() {
  vector<KT_Endpoint> activeEndPoint;
  vector<KT_Endpoint> inactiveEndPoint;
  _endpointInfo->getTCEndpointByAll(activeEndPoint, inactiveEndPoint);
  return activeEndPoint;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace kant
