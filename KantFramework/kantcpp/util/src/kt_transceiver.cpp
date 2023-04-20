#include "util/kt_transceiver.h"
#include "util/kt_logger.h"
#if KANT_SSL
#include "util/kt_openssl.h"
#endif
#include <sstream>

namespace kant {

class CloseClourse {
 public:
  CloseClourse(KT_Transceiver* trans, KT_Transceiver::CloseReason reason, const string& err)
    : _trans(trans), _reason(reason), _err(err) {}

  ~CloseClourse() { _trans->tcpClose(false, _reason, _err); }

 protected:
  KT_Transceiver* _trans;
  KT_Transceiver::CloseReason _reason;
  string _err;
};

#define THROW_ERROR(x, r, y)       \
  {                                \
    CloseClourse c(this, r, y);    \
    THROW_EXCEPTION_SYSCODE(x, y); \
  }

static const int BUFFER_SIZE = 16 * 1024;
uint64_t KT_Transceiver::LONG_NETWORK_TRANS_TIME = 1;
///////////////////////////////////////////////////////////////////////

int KT_Transceiver::createSocket(bool udp, bool isLocal, bool isIpv6) {
#if TARGET_PLATFORM_WINDOWS
  int domain = (isIpv6 ? PF_INET6 : PF_INET);
#else
  int domain = isLocal ? PF_LOCAL : (isIpv6 ? PF_INET6 : PF_INET);
#endif

  int type = udp ? SOCK_DGRAM : SOCK_STREAM;

  KT_Socket s;
  s.createSocket(type, domain);

  if (!udp) {
    if (!isLocal) {
      s.setTcpNoDelay();
      s.setKeepAlive();
      s.setNoCloseWait();
    }
  } else {
    s.setRecvBufferSize(512 * 1024);
    s.setSendBufferSize(512 * 1024);
  }

  s.setOwner(false);
  s.setblock(false);
  return s.getfd();
}

bool KT_Transceiver::doConnect(int fd, const struct sockaddr* addr, socklen_t len) {
  bool bConnected = false;

  int iRet = ::connect(fd, addr, len);

  if (iRet == 0) {
    bConnected = true;
  } else if (!KT_Socket::isInProgress()) {
    THROW_ERROR(KT_Transceiver_Exception, CR_Connect,
                "connect error, " + _desc);  //, KT_Exception::getSystemCode());
  }

  //	LOG_CONSOLE_DEBUG << bConnected << endl;

  return bConnected;
}

KT_Transceiver::KT_Transceiver(KT_Epoller* epoller, const KT_Endpoint& ep)
  : _epoller(epoller),
    _ep(ep),
    _desc(ep.toString()),
    _fd(-1),
    _connStatus(eUnconnected),
    _sendBuffer(this),
    _recvBuffer(this),
    _authState(eAuthInit) {
  // LOG_CONSOLE_DEBUG << endl;
  if (ep.isUdp()) {
    _pRecvBuffer = std::make_shared<KT_NetWorkBuffer::Buffer>();
    _nRecvBufferSize = DEFAULT_RECV_BUFFERSIZE;
    _pRecvBuffer->alloc(_nRecvBufferSize);
  }

  //    _serverAddr = KT_Socket::createSockAddr(_ep.getHost().c_str());
  _serverAddr = KT_Socket::createSockAddr(_ep);
}

KT_Transceiver::~KT_Transceiver() {
  if (!isValid()) return;

  if (_ep.isTcp()) {
    tcpClose(true, CR_DECONSTRUCTOR, "");
  } else {
    udpClose();
  }
}

void KT_Transceiver::initializeClient(const oncreate_callback& oncreate, const onclose_callback& onclose,
                                      const onconnect_callback& onconnect, const onrequest_callback& onrequest,
                                      const onparser_callback& onparser, const onopenssl_callback& onopenssl,
                                      const oncompletepackage_callback& onfinish,
                                      const oncompletenetwork_callback& onfinishAll) {
  _isServer = false;

  _createSocketCallback = oncreate;

  _onConnectCallback = onconnect;

  _onRequestCallback = onrequest;

  _onCloseCallback = onclose;

  _onParserCallback = onparser;

  _onCompletePackageCallback = onfinish;

  _onOpensslCallback = onopenssl;

  _onCompleteNetworkCallback = onfinishAll;
}

void KT_Transceiver::initializeServer(const onclose_callback& onclose, const onrequest_callback& onrequest,
                                      const onparser_callback& onparser, const onopenssl_callback& onopenssl,
                                      const oncompletepackage_callback& onfinish,
                                      const oncompletenetwork_callback& onfinishAll) {
  _isServer = true;

  _connStatus = eConnected;

  _onRequestCallback = onrequest;

  _onCloseCallback = onclose;

  _onParserCallback = onparser;

  _onCompletePackageCallback = onfinish;

  _onOpensslCallback = onopenssl;

  _onCompleteNetworkCallback = onfinishAll;

#if KANT_SSL
  if (isSSL()) {
    _openssl = _onOpensslCallback(this);
    if (!_openssl) {
      THROW_ERROR(KT_Transceiver_Exception, CR_SSL,
                  "[KT_Transceiver::initializeServer create '" + _desc + "' ssl client error]");
    }

    _openssl->init(true);

    _openssl->recvBuffer()->setConnection(this);

    int ret = _openssl->doHandshake(_sendBuffer);
    if (ret != 0) {
      THROW_ERROR(
        KT_Transceiver_Exception, CR_SSL_HANDSHAKE,
        "[KT_Transceiver::initializeServer create '" + _desc + "' ssl client error: " + _openssl->getErrMsg() + "]");
    }

    // send the encrypt data from write buffer
    if (!_sendBuffer.empty()) {
      doRequest();
    }
  }
#endif
}

void KT_Transceiver::setClientAuthCallback(const onclientsendauth_callback& onsendauth,
                                           const onclientverifyauth_callback& onverifyauth) {
  _onClientSendAuthCallback = onsendauth;

  _onClientVerifyAuthCallback = onverifyauth;
}

void KT_Transceiver::setServerAuthCallback(const onserververifyauth_callback& onverifyauth) {
  _onServerVerifyAuthCallback = onverifyauth;
}

void KT_Transceiver::setBindAddr(const char* host) {
  if (_isServer) {
    THROW_ERROR(KT_Transceiver_Exception, CR_Type, "setBindAddr(" + string(host) + ") only use in client, " + _desc);
  }
  _bindAddr = KT_Socket::createSockAddr(host);
}

void KT_Transceiver::setBindAddr(const KT_Socket::addr_type& bindAddr) {
  if (_isServer) {
    THROW_ERROR(KT_Transceiver_Exception, CR_Type, "setBindAddr only use in client, " + _desc);
  }
  _bindAddr = bindAddr;
}

shared_ptr<KT_Epoller::EpollInfo> KT_Transceiver::bindFd(int fd) {
  if (!_isServer) {
    THROW_ERROR(KT_Transceiver_Exception, CR_Type, "client should not call bindFd, " + _desc);
  }
  _connStatus = eConnected;

  _fd = fd;

  //设置套接口选项
  for (size_t i = 0; i < _socketOpts.size(); ++i) {
    setsockopt(_fd, _socketOpts[i].level, _socketOpts[i].optname, (const char*)_socketOpts[i].optval,
               _socketOpts[i].optlen);
  }

  _clientAddr = KT_Socket::createSockAddr(_ep.getHost().c_str());

  getpeername(_fd, _clientAddr.first.get(), &_clientAddr.second);

  _epollInfo = _epoller->createEpollInfo(_fd);

  return _epollInfo;
}

void KT_Transceiver::setUdpRecvBuffer(size_t nSize) {
  _nRecvBufferSize = nSize;
  _pRecvBuffer->alloc(_nRecvBufferSize);
}

void KT_Transceiver::checkConnect() {
  //检查连接是否有错误
  if (isConnecting()) {
    int iVal = 0;
    SOCKET_LEN_TYPE iLen = static_cast<SOCKET_LEN_TYPE>(sizeof(int));
    int ret = ::getsockopt(_fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&iVal), &iLen);

    if (ret < 0 || iVal) {
      string err = KT_Exception::parseError(iVal);
      THROW_ERROR(KT_Transceiver_Exception, CR_Connect, "connect " + _desc + " error:" + err);
    }

    _clientAddr = KT_Socket::createSockAddr(_ep.getHost().c_str());

    getpeername(_fd, _clientAddr.first.get(), &_clientAddr.second);

    if (_bindAddr.first) {
      //如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间
      int iReuseAddr = 1;

      setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&iReuseAddr, sizeof(int));

      ::bind(_fd, _bindAddr.first.get(), _bindAddr.second);
    }
    setConnected();
  }
}

void KT_Transceiver::parseConnectAddress() {
#if !TARGET_PLATFORM_WINDOWS
  if (isUnixLocal()) {
    KT_Socket::parseUnixLocalAddr(getConnectEndpoint().getHost().c_str(), *(sockaddr_un*)_serverAddr.first.get());
  } else
#endif
  {
    if (isConnectIPv6()) {
      KT_Socket::parseAddrWithPort(getConnectEndpoint().getHost(), getConnectEndpoint().getPort(),
                                   *(sockaddr_in6*)_serverAddr.first.get());
    } else {
      KT_Socket::parseAddrWithPort(getConnectEndpoint().getHost(), getConnectEndpoint().getPort(),
                                   *(sockaddr_in*)_serverAddr.first.get());
    }
  }
}

bool KT_Transceiver::isSSL() const { return _ep.isSSL(); }

void KT_Transceiver::connect() {
  if (_isServer) {
    THROW_ERROR(KT_Transceiver_Exception, CR_Type, "server should not call connect, " + _desc);
  }

  if (isValid()) {
    return;
  }

  if (_connStatus == eConnecting || _connStatus == eConnected) {
    return;
  }

  if (_ep.isUdp()) {
    _fd = createSocket(true, isUnixLocal(), isConnectIPv6());

    _connStatus = eConnected;

    _epollInfo = _epoller->createEpollInfo(_fd);

    _proxyInfo = _createSocketCallback(this);
    if (_proxyInfo) {
      _desc = _proxyInfo->getEndpoint().toString();
    }

    //每次连接前都重新解析一下地址, 避免dns变了!
    parseConnectAddress();
  } else {
    _fd = createSocket(false, isUnixLocal(), isConnectIPv6());

    _isConnTimeout = false;

    _epollInfo = _epoller->createEpollInfo(_fd);

    _connTimerId = _epoller->postDelayed(_connTimeout, std::bind(&KT_Transceiver::checkConnectTimeout, this));

    _proxyInfo = _createSocketCallback(this);
    if (_proxyInfo) {
      _desc = _proxyInfo->getEndpoint().toString();
    }

    //每次连接前都重新解析一下地址, 避免dns变了!
    parseConnectAddress();

    bool bConnected = doConnect(_fd, _serverAddr.first.get(), _serverAddr.second);
    if (bConnected) {
      setConnected();
    } else {
      _connStatus = KT_Transceiver::eConnecting;
    }
  }

  //设置套接口选项
  for (size_t i = 0; i < _socketOpts.size(); ++i) {
    setsockopt(_fd, _socketOpts[i].level, _socketOpts[i].optname, (const char*)_socketOpts[i].optval,
               _socketOpts[i].optlen);
  }
}

void KT_Transceiver::checkConnectTimeout() {
  if (_connStatus != eConnected) {
    _isConnTimeout = true;
    THROW_ERROR(KT_Transceiver_Exception, CR_ConnectTimeout, "connect timeout, " + _desc);
  }
}

void KT_Transceiver::setConnected() {
  if (_isServer) {
    THROW_ERROR(KT_Transceiver_Exception, CR_Type, "server should not call setConnected, " + _desc);
  }
  _connStatus = eConnected;

  if (_proxyInfo) {
    connectProxy();
  } else {
    onSetConnected();
  }
}

void KT_Transceiver::onSetConnected() {
  if (_isServer) {
    THROW_ERROR(KT_Transceiver_Exception, CR_Type, "server should not call onSetConnected, " + _desc);
  }
  onConnect();

  _onConnectCallback(this);

  if (!isSSL()) {
    doAuthReq();
  }
}

void KT_Transceiver::onConnect() {
  if (_isServer) {
    THROW_ERROR(KT_Transceiver_Exception, CR_Type, "server should not call onConnect, " + _desc);
  }

  _epoller->erase(_connTimerId);
  _connTimerId = 0;
#if KANT_SSL
  if (isSSL()) {
    _openssl = _onOpensslCallback(this);
    if (!_openssl) {
      close();
      return;
    }

    _openssl->init(false);

    _openssl->setReadBufferSize(1024 * 8);
    _openssl->setWriteBufferSize(1024 * 8);

    _openssl->recvBuffer()->setConnection(this);

    int ret = _openssl->doHandshake(_sendBuffer);
    if (ret != 0) {
      THROW_ERROR(KT_Transceiver_Exception, CR_SSL,
                  "ssl hande shake failed, " + _desc + ", error:" + _openssl->getErrMsg());
    }

    // send the encrypt data from write buffer
    if (!_sendBuffer.empty()) {
      doRequest();
    }

    return;
  }
#endif
}

void KT_Transceiver::doAuthReq() {
  if (_ep.getAuthType() == KT_Endpoint::AUTH_TYPENONE) {
    _authState = eAuthSucc;
    _onRequestCallback(this);
  } else {
    //如果是客户端, 则主动发起鉴权请求
    shared_ptr<KT_NetWorkBuffer::Buffer> buff = _onClientSendAuthCallback(this);

#if KANT_SSL
    if (this->isSSL()) {
      int ret = _openssl->write(buff->buffer(), (uint32_t)buff->length(), _sendBuffer);
      if (ret != 0) {
        THROW_ERROR(KT_Transceiver_Exception, CR_SSL,
                    "ssl write failed, " + _desc + ", error:" + _openssl->getErrMsg());
        return;
      }
    } else {
      _sendBuffer.addBuffer(buff);
    }

#else
    _sendBuffer.addBuffer(buff);
#endif

    doRequest();
  }
}

void KT_Transceiver::connectProxy() {
  assert(_proxyInfo);

  vector<char> buff;

  bool succ = _proxyInfo->sendProxyPacket(buff, _ep);
  if (!succ) {
    THROW_ERROR(KT_Transceiver_Exception, CR_PROXY_SEND,
                "connect to proxy, " + _desc + ", error:" + _proxyInfo->getErrMsg());
  }
  _sendBuffer.addBuffer(buff);

  doRequest();
}

int KT_Transceiver::doCheckProxy(const char* buff, size_t length) {
  if (!_proxyInfo || _proxyInfo->isSuccess()) return 0;

  bool succ = _proxyInfo->recvProxyPacket(buff, length);
  if (!succ) {
    THROW_ERROR(KT_Transceiver_Exception, CR_PROXY_RECV,
                "connect to proxy, " + _desc + ", error:" + _proxyInfo->getErrMsg());
  }

  if (!_proxyInfo->isSuccess()) {
    connectProxy();
  } else {
    onSetConnected();
  }

  return 1;
}

void KT_Transceiver::udpClose() {
  if (_ep.isUdp()) {
    _epoller->releaseEpollInfo(_epollInfo);

    _epollInfo.reset();

    KT_Port::closeSocket(_fd);

    _fd = -1;

    _connStatus = eUnconnected;

    _sendBuffer.clearBuffers();

    _recvBuffer.clearBuffers();
  }
}

void KT_Transceiver::close() {
  //	LOG_CONSOLE_DEBUG << this << endl;
  if (!isValid()) return;

  if (_ep.isTcp()) {
    tcpClose(false, CR_ACTIVE, "active call");
  } else {
    udpClose();
  }
}

void KT_Transceiver::tcpClose(bool deconstructor, CloseReason reason, const string& err) {
  if (_ep.isTcp() && isValid()) {
#if KANT_SSL
    if (_openssl) {
      _openssl->release();
      _openssl.reset();
    }
#endif

    //LOG_CONSOLE_DEBUG << this << ", " << _fd << ", " << reason << ", " << err << ", " << deconstructor << endl;

    if (_connTimerId != 0) {
      _epoller->erase(_connTimerId);
      _connTimerId = 0;
    }

    _epoller->releaseEpollInfo(_epollInfo);

    _epollInfo.reset();

    KT_Port::closeSocket(_fd);

    _fd = -1;

    _connStatus = eUnconnected;

    _sendBuffer.clearBuffers();

    _recvBuffer.clearBuffers();

    _authState = eAuthInit;

    if (!deconstructor) {
      //注意必须放在最后, 主要避免_onCloseCallback里面析构了链接, 从而导致又进入tcpClose
      //放在最后就不会有问题了, 因为不会再进入这个函数
      _onCloseCallback(this, reason, err);
    }
  }
}

void KT_Transceiver::doRequest() {
  if (!isValid()) return;

  checkConnect();

  //buf不为空,先发送buffer的内容
  while (!_sendBuffer.empty()) {
    auto data = _sendBuffer.getBufferPointer();
    assert(data.first != NULL && data.second != 0);

    int iRet = this->send(data.first, (uint32_t)data.second, 0);

    if (iRet <= 0) {
      return;
    }

    _sendBuffer.moveHeader(iRet);
  }

  if (_sendBuffer.empty()) {
    _onRequestCallback(this);
  }
}

KT_Transceiver::ReturnStatus KT_Transceiver::sendRequest(const shared_ptr<KT_NetWorkBuffer::Buffer>& buff,
                                                         const KT_Socket::addr_type& addr) {
  //	LOG_CONSOLE_DEBUG << buff->length() << endl;

  //空数据 直接返回成功
  if (buff->empty()) {
    return eRetOk;
  }

  // assert(_sendBuffer.empty());
  //buf不为空, 表示之前的数据还没发送完, 直接返回失败, 等buffer可写了,epoll会通知写事件
  if (!_sendBuffer.empty()) {
    //不应该运行到这里
    return eRetNotSend;
  }

  if (eConnected != _connStatus) {
    return eRetNotSend;
  }

  if (_proxyInfo && !_proxyInfo->isSuccess()) {
    return eRetNotSend;
  }

  if (_ep.isTcp() && _ep.getAuthType() == KT_Endpoint::AUTH_TYPELOCAL && _authState != eAuthSucc) {
#if KANT_SSL
    if (isSSL() && !_openssl) {
      return eRetNotSend;
    }
#endif
    return eRetNotSend;  // 需要鉴权但还没通过，不能发送非认证消息
  }

#if KANT_SSL
  // 握手数据已加密,直接发送，会话数据需加密
  if (isSSL()) {
    if (!_openssl->isHandshaked()) {
      return eRetNotSend;
    }

    int ret = _openssl->write(buff->buffer(), (uint32_t)buff->length(), _sendBuffer);
    if (ret != 0) {
      close();
      return eRetError;
    }

    buff->clear();
  } else {
    _sendBuffer.addBuffer(buff);
  }
#else
  _sendBuffer.addBuffer(buff);
#endif

  //	LOG_CONSOLE_DEBUG << _sendBuffer.getBufferLength() << endl;

  _lastAddr = addr;
  do {
    auto data = _sendBuffer.getBufferPointer();

    int iRet = this->send(data.first, (uint32_t)data.second, 0);
    if (iRet < 0) {
      if (!isValid()) {
        _sendBuffer.clearBuffers();
        return eRetError;
      } else {
        return eRetFull;
      }
    }

    _sendBuffer.moveHeader(iRet);
    //		assert(iRet != 0);
  } while (!_sendBuffer.empty());

  return eRetOk;
}

void KT_Transceiver::doAuthCheck(KT_NetWorkBuffer* buff) {
  if (!buff->empty() && _ep.isTcp() && _ep.getAuthType() == KT_Endpoint::AUTH_TYPELOCAL && _authState != eAuthSucc) {
    KT_NetWorkBuffer::PACKET_TYPE type;

    if (_isServer) {
      //验证鉴权
      auto ret = _onServerVerifyAuthCallback(*buff, this);

      type = ret.first;

      if (type == KT_NetWorkBuffer::PACKET_FULL) {
        _authState = eAuthSucc;
        //服务器端, 鉴权通过, 可以响应包
        sendRequest(ret.second, _serverAddr);
      }
    } else {
      type = _onClientVerifyAuthCallback(*buff, this);

      if (type == KT_NetWorkBuffer::PACKET_FULL) {
        _authState = eAuthSucc;
        //客户端, 鉴权通过可以发送业务包了
        _onRequestCallback(this);
      }
    }

    if (type == KT_NetWorkBuffer::PACKET_ERR) {
      THROW_ERROR(KT_Transceiver_Exception, CR_PROTOCOL, "[KT_Transceiver::doProtocolAnalysis, auth error]");
    }
  }
}

int KT_Transceiver::doProtocolAnalysis(KT_NetWorkBuffer* buff) {
  doAuthCheck(buff);

  KT_NetWorkBuffer::PACKET_TYPE ret;

  int packetCount = 0;

  int ioriginal = 0;
  int isurplus = 0;
  try {
    do {
      ioriginal = buff->getBuffers().size();
      ret = _onParserCallback(*buff, this);
      isurplus = buff->getBuffers().size();

      if (ret == KT_NetWorkBuffer::PACKET_FULL || ret == KT_NetWorkBuffer::PACKET_FULL_CLOSE) {
        ++packetCount;
      }
      if (_onCompletePackageCallback) {
        //收到一个完整的包
        _onCompletePackageCallback(this);
      }

      if (ret == KT_NetWorkBuffer::PACKET_FULL_CLOSE) {
        //full close模式下, 需要关闭连接
        tcpClose(false, CR_PROTOCOL, "protocol full close");
      }

      // 当收到完整包时，解析完包后，buffer没movehead，则报错
      if (ret == KT_NetWorkBuffer::PACKET_FULL && ioriginal == isurplus) {
        ret = KT_NetWorkBuffer::PACKET_FULL_CLOSE;
        string err = "parser buffer movehead error, " + _desc;
        tcpClose(false, CR_PROTOCOL, err);  // 这个地方会将连接关闭，为了方便后期问题定位
        throw KT_Transceiver_Exception(err);
      }

    } while (ret == KT_NetWorkBuffer::PACKET_FULL);

    if (_onCompleteNetworkCallback) {
      try {
        _onCompleteNetworkCallback(this);
      } catch (...) {
      }
    }
  } catch (exception& ex) {
    if (_onCompleteNetworkCallback) {
      try {
        _onCompleteNetworkCallback(this);
      } catch (...) {
      }
    }
    THROW_ERROR(KT_Transceiver_Exception, CR_PROTOCOL, "parser decode error:" + string(ex.what()) + "]");
  } catch (...) {
    if (_onCompleteNetworkCallback) {
      try {
        _onCompleteNetworkCallback(this);
      } catch (...) {
      }
    }
    THROW_ERROR(KT_Transceiver_Exception, CR_PROTOCOL, "parser decode error");
  }

  if (ret == KT_NetWorkBuffer::PACKET_ERR) {
    string err = "parser decode error, " + _desc;
    tcpClose(false, CR_PROTOCOL, err);
    throw KT_Transceiver_Exception(err);
  }

  return packetCount;
}

//////////////////////////////////////////////////////////
KT_TCPTransceiver::KT_TCPTransceiver(KT_Epoller* epoller, const KT_Endpoint& ep) : KT_Transceiver(epoller, ep) {
  assert(epoller);
}

//不同的内存分配机制
#if 0
bool KT_TCPTransceiver::doResponse()
{
checkConnect();

int iRet = 0;
int64_t now = TNOWMS;

//	int packetCount = 0;
do
{
	char buff[BUFFER_SIZE];

	if ((iRet = this->recv((void*)buff, BUFFER_SIZE, 0)) > 0)
	{
		int check = doCheckProxy(buff, iRet);
		if(check != 0)
		{
			_recvBuffer.clearBuffers();
			return true;
		}

		_recvBuffer.addBuffer(buff, iRet);

		//解析协议
		doProtocolAnalysis(&_recvBuffer);
		//收包太多了, 中断一下, 释放线程给send等
		if (TNOWMS - now >= LONG_NETWORK_TRANS_TIME && isValid())
		{
			_epollInfo->mod(EPOLLIN | EPOLLOUT);
			break;
		}

		//接收的数据小于buffer大小, 内核会再次通知你
		if(iRet < BUFFER_SIZE)
		{
			break;
		}
	}
}
while (iRet>0);

if(iRet == 0)
{
	tcpClose(false, CR_PEER_CLOSE, "peer close connection");
}

return iRet != 0;
}

#else

bool KT_TCPTransceiver::doResponse() {
  checkConnect();

  int iRet = 0;
  int64_t now = TNOWMS;

  //		int packetCount = 0;
  do {
    auto data = _recvBuffer.getOrCreateBuffer(BUFFER_SIZE / 8, BUFFER_SIZE);

    uint32_t left = (uint32_t)data->left();

    if ((iRet = this->recv((void*)data->free(), left, 0)) > 0) {
      int check = doCheckProxy(data->free(), iRet);
      if (check != 0) {
        _recvBuffer.clearBuffers();
        return true;
      }

      data->addWriteIdx(iRet);

      _recvBuffer.addLength(iRet);

      //解析协议
      doProtocolAnalysis(&_recvBuffer);

      //收包太多了, 中断一下, 释放线程给send等
      if (TNOWMS - now >= LONG_NETWORK_TRANS_TIME && isValid()) {
        _epollInfo->mod(EPOLLIN | EPOLLOUT);
        break;
      }

      //接收的数据小于buffer大小, 内核会再次通知你
      if (iRet < (int)left) {
        break;
      }
    }
  } while (iRet > 0);

  if (iRet == 0) {
    tcpClose(false, CR_PEER_CLOSE, "peer close connection");
  }

  return iRet != 0;
}

#endif

int KT_TCPTransceiver::send(const void* buf, uint32_t len, uint32_t flag) {
  //只有是连接状态才能收发数据
  if (eConnected != _connStatus) {
    return -1;
  }

  int iRet = ::send(_fd, (const char*)buf, len, flag);
  //    LOG_CONSOLE_DEBUG << this << ", send, fd:" << _fd << ", " << _desc << ", iRet:" << iRet << ", len:" << len << endl;

  if (iRet < 0 && !KT_Socket::isPending()) {
    THROW_ERROR(KT_Transceiver_Exception, CR_SEND,
                "KT_TCPTransceiver::send, " + _desc + ", fd:" + KT_Common::tostr(_fd));
  }

#if TARGET_PLATFORM_WINDOWS
  if (iRet < 0 && KT_Socket::isPending()) {
    _epollInfo->mod(EPOLLIN | EPOLLOUT);
  }
#endif

  return iRet;
}

int KT_TCPTransceiver::recv(void* buf, uint32_t len, uint32_t flag) {
  //只有是连接状态才能收发数据
  if (eConnected != _connStatus) return -1;

  int iRet = ::recv(_fd, (char*)buf, len, flag);

  //	 LOG_CONSOLE_DEBUG << this << ", recv, fd:" << _fd << ", " << _desc << ", iRet:" << iRet << endl;

  if ((iRet < 0 && !KT_Socket::isPending())) {
    int nerr = KT_Exception::getSystemCode();
    string err = "recv error, errno:" + KT_Common::tostr(nerr) + "," + KT_Exception::parseError(nerr);
    THROW_ERROR(KT_Transceiver_Exception, CR_RECV, err + ", " + _desc + ", fd:" + KT_Common::tostr(_fd));
  }

#if TARGET_PLATFORM_WINDOWS
  if (iRet < 0 && KT_Socket::isPending()) {
    _epollInfo->mod(EPOLLIN | EPOLLOUT);
  }
#endif

  return iRet;
}
/////////////////////////////////////////////////////////////////
#if KANT_SSL

KT_SSLTransceiver::KT_SSLTransceiver(KT_Epoller* epoller, const KT_Endpoint& ep) : KT_TCPTransceiver(epoller, ep) {}

#if 0

bool KT_SSLTransceiver::doResponse()
{
	checkConnect();

	int iRet = 0;
	int64_t now = TNOWMS;

//		int packetCount = 0;
	do
	{
		char buff[BUFFER_SIZE] = {0x00};
		if ((iRet = this->recv(buff, BUFFER_SIZE, 0)) > 0)
		{
			int check = doCheckProxy(buff, iRet);
			if(check != 0)
			{
				return true;
			}

			const bool preHandshake = _openssl->isHandshaked();

			int ret = _openssl->read(buff, iRet, _sendBuffer);
			if (ret != 0)
			{
//            	LOG_CONSOLE_DEBUG << "ret:" << ret << ", " << _openssl->getErrMsg() << endl;
				THROW_ERROR(KT_Transceiver_Exception, CR_SSL, "[KT_SSLTransceiver::doResponse, SSL_read handshake failed: " + _desc + ", info: " + _openssl->getErrMsg() + "]");
			}
			else if(!_sendBuffer.empty())
			{
//				LOG_CONSOLE_DEBUG << "[Transceiver::doResponse SSL_read prehandshake:" << preHandshake << ", handshake:" << _openssl->isHandshaked() << ", send handshake len:" << _sendBuffer.getBufferLength() << endl;
				int ret = doRequest();

				if(ret < 0)
				{
					// doRequest失败 close fd
					if (!isValid())
					{
						THROW_ERROR(KT_Transceiver_Exception, CR_SSL, "[KT_SSLTransceiver::doResponse, ssl doRequest failed: " + _desc + ", info: " + _openssl->getErrMsg() + "]");
					}
					else
					{
						return true;
					}
				}
			}

//			LOG_CONSOLE_DEBUG << "recv length:" << iRet << ", preHandshake:" << preHandshake << endl;

			if (!_openssl->isHandshaked())
			{
//				LOG_CONSOLE_DEBUG << "[Transceiver::doResponse not handshake, prehandshake:" << preHandshake << ", handshake:" << _openssl->isHandshaked() << endl;
				return true;
			}

			if (!preHandshake)
			{
				if(_isServer)
				{
					_onRequestCallback(this);
				}
				else
				{
					//握手完毕, 客户端直接发送鉴权请求
					doAuthReq();
					// doAuthReq失败，会close fd, 这里判断下是否还有效
					if (!isValid())
					{
						THROW_ERROR(KT_Transceiver_Exception, CR_SSL,
								"[KT_SSLTransceiver::doResponse, doAuthReq failed: " + _desc + ", info: " +
								_openssl->getErrMsg() + "]");
					}
					else
					{
//						LOG_CONSOLE_DEBUG << "[Transceiver::doResponse prehandshake:" << preHandshake << ", handshake:" << _openssl->isHandshaked() << endl;
					}
				}
			}

			KT_NetWorkBuffer *rbuf = _openssl->recvBuffer();

			//解析协议
			doProtocolAnalysis(rbuf);

			//收包太多了, 中断一下, 释放线程给send等
			if (TNOWMS - now >= LONG_NETWORK_TRANS_TIME && isValid())
			{
				_epollInfo->mod(EPOLLIN | EPOLLOUT);
				break;
			}

			//接收的数据小于buffer大小, 内核会再次通知你
			if(iRet < BUFFER_SIZE)
			{
				break;
			}
		}
	}
	while (iRet>0);

	if(iRet == 0)
	{
		tcpClose(false, CR_PEER_CLOSE, "peer close connection");
	}

	return iRet != 0;
}

#else

bool KT_SSLTransceiver::doResponse() {
  checkConnect();

  int iRet = 0;

  int64_t now = TNOWMS;

  do {
    auto data = _recvBuffer.getOrCreateBuffer(BUFFER_SIZE / 8, BUFFER_SIZE);

    uint32_t left = (uint32_t)data->left();

    if ((iRet = this->recv((void*)data->free(), left, 0)) > 0) {
      int check = doCheckProxy(data->free(), iRet);

      if (check != 0) {
        return true;
      }

      const bool preHandshake = _openssl->isHandshaked();

      int ret = _openssl->read(data->free(), iRet, _sendBuffer);

      if (ret != 0) {
        //            	LOG_CONSOLE_DEBUG << "ret:" << ret << ", " << _openssl->getErrMsg() << endl;
        THROW_ERROR(KT_Transceiver_Exception, CR_SSL,
                    "[KT_SSLTransceiver::doResponse, SSL_read handshake failed: " + _desc +
                      ", info: " + _openssl->getErrMsg() + "]");
      } else if (!_sendBuffer.empty()) {
        doRequest();
      }

      if (!_openssl->isHandshaked()) {
        //				LOG_CONSOLE_DEBUG << "[Transceiver::doResponse not handshake, prehandshake:" << preHandshake << ", handshake:" << _openssl->isHandshaked() << endl;
        return true;
      }

      if (!preHandshake) {
        if (_isServer) {
          _onRequestCallback(this);
        } else {
          //握手完毕, 客户端直接发送鉴权请求
          doAuthReq();
          // doAuthReq失败，会close fd, 这里判断下是否还有效
          if (!isValid()) {
            THROW_ERROR(
              KT_Transceiver_Exception, CR_SSL,
              "[KT_SSLTransceiver::doResponse, doAuthReq failed: " + _desc + ", info: " + _openssl->getErrMsg() + "]");
          } else {
            //						LOG_CONSOLE_DEBUG << "[Transceiver::doResponse prehandshake:" << preHandshake << ", handshake:" << _openssl->isHandshaked() << endl;
          }
        }
      }

      KT_NetWorkBuffer* rbuf = _openssl->recvBuffer();

      //解析协议
      doProtocolAnalysis(rbuf);

      //收包太多了, 中断一下, 释放线程给send等
      if (TNOWMS - now >= LONG_NETWORK_TRANS_TIME && isValid()) {
        _epollInfo->mod(EPOLLIN | EPOLLOUT);
        break;
      }

      //接收的数据小于buffer大小, 内核会再次通知你
      if (iRet < left) {
        break;
      }
    }
  } while (iRet > 0);

  if (iRet == 0) {
    tcpClose(false, CR_PEER_CLOSE, "peer close connection");
  }

  return iRet != 0;
}

#endif
#endif

/////////////////////////////////////////////////////////////////
KT_UDPTransceiver::KT_UDPTransceiver(KT_Epoller* epoller, const KT_Endpoint& ep) : KT_Transceiver(epoller, ep) {}

KT_UDPTransceiver::~KT_UDPTransceiver() {}

bool KT_UDPTransceiver::doResponse() {
  checkConnect();

  int iRet = 0;
  int64_t now = TNOWMS;
  do {
    _recvBuffer.clearBuffers();

    auto data = _recvBuffer.getOrCreateBuffer(_nRecvBufferSize, _nRecvBufferSize);

    uint32_t left = (uint32_t)data->left();

    if ((iRet = this->recv((void*)data->free(), left, 0)) > 0) {
      data->addWriteIdx(iRet);
      _recvBuffer.addLength(iRet);

      //解析协议
      doProtocolAnalysis(&_recvBuffer);
      //收包太多了, 中断一下, 释放线程给send等
      if (TNOWMS - now >= LONG_NETWORK_TRANS_TIME && isValid()) {
        _epollInfo->mod(EPOLLIN | EPOLLOUT);
        break;
      }
    }
  } while (iRet > 0);

  return iRet != 0;
}

int KT_UDPTransceiver::send(const void* buf, uint32_t len, uint32_t flag) {
  if (!isValid()) return -1;

  int iRet = 0;
  if (_isServer) {
    iRet = ::sendto(_fd, (const char*)buf, len, flag, _lastAddr.first.get(), _lastAddr.second);
  } else {
    iRet = ::sendto(_fd, (const char*)buf, len, flag, _serverAddr.first.get(), _serverAddr.second);
  }

  if (iRet > 0) {
    //udp只发一次 发送一半也算全部发送成功
    return len;
  }

  if (iRet < 0 && KT_Socket::isPending()) {
    //EAGAIN, 认为没有发送
    return 0;
  }

  return iRet;
}

int KT_UDPTransceiver::recv(void* buf, uint32_t len, uint32_t flag) {
  if (!isValid()) return -1;

  _clientAddr = KT_Socket::createSockAddr(_ep.getHost().c_str());

  int iRet = ::recvfrom(_fd, (char*)buf, len, flag, _clientAddr.first.get(),
                        &_clientAddr.second);  //need check from_ip & port

  if (!_isServer) {
    //客户端才会关闭连接, 会重建socket, 服务端不会
    if (iRet < 0 && !KT_Socket::isPending()) {
      THROW_ERROR(KT_Transceiver_Exception, CR_RECV,
                  "KT_UDPTransceiver::udp recv, " + _desc + ", fd:" + KT_Common::tostr(_fd));
      return 0;
    }
  }

  return iRet;
}

/////////////////////////////////////////////////////////////////
}  // namespace kant
