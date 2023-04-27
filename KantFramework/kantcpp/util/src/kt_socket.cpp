#include "util/kt_platform.h"
#include "util/kt_clientsocket.h"
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#ifndef TARGET_PLATFORM_ANDROID
#include <ifaddrs.h>
#endif
#include <sys/ioctl.h>
#include <net/if.h>
#ifndef TARGET_PLATFORM_IOS
#include <net/if_arp.h>
#endif
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#else
#include <WS2tcpip.h>

#define bzero(b, len) (memset((b), '\0', (len)))
#endif

#include <cerrno>
#include <cassert>
#include "util/kt_port.h"
#include "util/kt_socket.h"
namespace kant {

KT_Socket::KT_Socket() : _sock(INVALID_SOCKET), _bOwner(true), _iDomain(AF_INET) {}

KT_Socket::~KT_Socket() {
  if (_bOwner) {
    close();
  }
}

void KT_Socket::init(int fd, bool bOwner, int iDomain) {
  if (_bOwner) {
    close();
  }

  _sock = fd;
  _bOwner = bOwner;
  _iDomain = iDomain;
}

void KT_Socket::createSocket(int iSocketType, int iDomain) {
  // assert(iSocketType == SOCK_STREAM || iSocketType == SOCK_DGRAM);
  close();

  _iDomain = iDomain;
  _sock = socket(iDomain, iSocketType, 0);

  if (_sock < 0) {
    _sock = INVALID_SOCKET;
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::createSocket] create socket error");
    // throw KT_Socket_Exception("[KT_Socket::createSocket] create socket error! :" + string(strerror(errno)));
  } else {
    ignoreSigPipe();
  }
}

void KT_Socket::getPeerName(string &sPeerAddress, uint16_t &iPeerPort) const {
  assert(_iDomain == AF_INET || _iDomain == AF_INET6);

  char sAddr[INET6_ADDRSTRLEN] = "\0";
  struct sockaddr_in stPeer4;
  struct sockaddr_in6 stPeer6;
  struct sockaddr *stPeer = (AF_INET6 == _iDomain) ? (struct sockaddr *)&stPeer6 : (struct sockaddr *)&stPeer4;
  socklen_t iPeerLen = (AF_INET6 == _iDomain) ? sizeof(stPeer6) : sizeof(stPeer4);

  bzero(stPeer, iPeerLen);
  getPeerName(stPeer, iPeerLen);
  inet_ntop(_iDomain, (AF_INET6 == _iDomain) ? (void *)&stPeer6.sin6_addr : (void *)&stPeer4.sin_addr, sAddr,
            sizeof(sAddr));
  sPeerAddress = sAddr;
  iPeerPort = (AF_INET6 == _iDomain) ? ntohs(stPeer6.sin6_port) : ntohs(stPeer4.sin_port);
}

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS

void KT_Socket::getPeerName(string &sPathName) const {
  assert(_iDomain == AF_LOCAL);

  struct sockaddr_un stSock;
  bzero(&stSock, sizeof(struct sockaddr_un));
  SOCKET_LEN_TYPE iSockLen = sizeof(stSock);
  getPeerName((struct sockaddr *)&stSock, iSockLen);
  sPathName = stSock.sun_path;
}

void KT_Socket::getSockName(string &sPathName) const {
  assert(_iDomain == AF_LOCAL);

  struct sockaddr_un stSock;
  bzero(&stSock, sizeof(struct sockaddr_un));
  SOCKET_LEN_TYPE iSockLen = sizeof(stSock);
  getSockName((struct sockaddr *)&stSock, iSockLen);

  sPathName = stSock.sun_path;
}

void KT_Socket::bind(const char *sPathName) {
  assert(_iDomain == AF_LOCAL);

  unlink(sPathName);

  struct sockaddr_un stBindAddr;
  memset(&stBindAddr, 0x00, sizeof(stBindAddr));
  stBindAddr.sun_family = _iDomain;
  strncpy(stBindAddr.sun_path, sPathName, sizeof(stBindAddr.sun_path));

  try {
    bind((struct sockaddr *)&stBindAddr, sizeof(stBindAddr));
  } catch (const std::exception &e) {
    std::cerr << e.what() << ", " << sPathName << endl;
    throw e;
  }
}

void KT_Socket::connect(const char *sPathName) {
  int ret = connectNoThrow(sPathName);
  if (ret < 0) {
    THROW_EXCEPTION_SYSCODE(KT_SocketConnect_Exception, "[KT_Socket::connect] connect error");
  }
}

int KT_Socket::connectNoThrow(const char *sPathName) {
  assert(_iDomain == AF_LOCAL);

  struct sockaddr_un stServerAddr;
  memset(&stServerAddr, 0x00, sizeof(stServerAddr));
  stServerAddr.sun_family = _iDomain;
  strncpy(stServerAddr.sun_path, sPathName, sizeof(stServerAddr.sun_path));

  return connect((struct sockaddr *)&stServerAddr, sizeof(stServerAddr));
}

#endif

void KT_Socket::getPeerName(struct sockaddr *pstPeerAddr, SOCKET_LEN_TYPE &iPeerLen) const {
  if (getpeername(_sock, pstPeerAddr, &iPeerLen) < 0) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::getPeerName] getpeername error");
  }
}

void KT_Socket::getSockName(string &sSockAddress, uint16_t &iSockPort) const {
  assert(_iDomain == AF_INET || _iDomain == AF_INET6);

  char sAddr[INET6_ADDRSTRLEN] = "\0";
  struct sockaddr_in6 in6;
  struct sockaddr_in in4;
  struct sockaddr *in = (AF_INET6 == _iDomain) ? (struct sockaddr *)&in6 : (struct sockaddr *)&in4;
  socklen_t len = (AF_INET6 == _iDomain) ? sizeof(in6) : sizeof(in4);

  bzero(in, len);
  getSockName(in, len);
  inet_ntop(_iDomain, (AF_INET6 == _iDomain) ? (void *)&in6.sin6_addr : (void *)&in4.sin_addr, sAddr, sizeof(sAddr));
  sSockAddress = sAddr;
  iSockPort = (AF_INET6 == _iDomain) ? ntohs(in6.sin6_port) : ntohs(in4.sin_port);
}

void KT_Socket::getSockName(struct sockaddr *pstSockAddr, SOCKET_LEN_TYPE &iSockLen) const {
  if (getsockname(_sock, pstSockAddr, &iSockLen) < 0) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::getSockName] getsockname error");
  }
}

SOCKET_TYPE KT_Socket::accept(KT_Socket &tcSock, struct sockaddr *pstSockAddr, SOCKET_LEN_TYPE &iSockLen) {
  assert(tcSock._sock == INVALID_SOCKET);

  SOCKET_TYPE ifd;

#if TARGET_PLATFORM_WINDOWS
  ifd = ::accept(_sock, pstSockAddr, &iSockLen);
#else
  while ((ifd = ::accept(_sock, pstSockAddr, &iSockLen)) < 0 && errno == EINTR)
    ;
#endif

  tcSock._sock = ifd;
  tcSock._iDomain = _iDomain;

  return tcSock._sock;
}

void KT_Socket::parseAddr(const string &sAddr, struct in_addr &stSinAddr) {
  int iRet = inet_pton(AF_INET, sAddr.c_str(), &stSinAddr);
  if (iRet < 0) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::parseAddr] inet_pton(" + sAddr + ") error");
  }
#if TARGET_PLATFORM_LINUX
  else if (iRet == 0) {
    struct hostent stHostent;
    struct hostent *pstHostent;
    char buf[2048] = "\0";
    int iError;

    gethostbyname_r(sAddr.c_str(), &stHostent, buf, sizeof(buf), &pstHostent, &iError);

    if (pstHostent == NULL) {
      THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::parseAddr] gethostbyname_r(" + sAddr + ") error");

      // throw KT_Socket_Exception("[KT_Socket::parseAddr] gethostbyname_r(" + sAddr + ") error", KT_Exception::getSystemCode());
    } else {
      stSinAddr = *(struct in_addr *)pstHostent->h_addr;
    }
  }
#else
  else if (iRet == 0) {
    struct addrinfo *ailist;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int err = getaddrinfo(sAddr.c_str(), NULL, &hints, &ailist);
    if (err != 0) {
      THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::parseAddr] getaddrinfo(" + sAddr + ") error");
    } else {
      for (addrinfo *aip = ailist; aip != NULL; aip = aip->ai_next) {
        if (aip->ai_family == AF_INET) {
          stSinAddr = ((struct sockaddr_in *)aip->ai_addr)->sin_addr;
          break;
        }
      }

      freeaddrinfo(ailist);
    }
  }
#endif
}

void KT_Socket::parseAddr(const string &host, struct in6_addr &stSinAddr) {
  int iRet = inet_pton(AF_INET6, host.c_str(), &stSinAddr);
  if (iRet < 0) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::parseAddr] inet_pton(" + host + ") error");
  } else if (iRet == 0) {
    struct addrinfo *info = 0;
    int retry = 5;

    struct addrinfo hints = {0};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    int rs = 0;
    do {
      rs = getaddrinfo(host.c_str(), 0, &hints, &info);
    } while (info == 0 && rs == EAI_AGAIN && --retry >= 0);

    if (rs != 0) {
      ostringstream os;
      os << "DNSException ex:(" << KT_Exception::parseError(KT_Exception::getSystemCode()) << ")" << rs << ":" << host
         << ":" << __FILE__ << ":" << __LINE__;
      if (info != NULL) {
        freeaddrinfo(info);
      }
      throw KT_Socket_Exception(os.str());
    }

    assert(info != NULL);

    memcpy(&stSinAddr, info->ai_addr, sizeof(stSinAddr));

    freeaddrinfo(info);
  }
}

void KT_Socket::parseAddr(const addr_type &addr, string &host, uint16_t &port) {
  int iDomain;
  sockaddr_in6 *addr6 = nullptr;
  sockaddr_in *addr4 = nullptr;
  if (addr.second == sizeof(sizeof(struct sockaddr_in6))) {
    iDomain = AF_INET6;
    addr6 = (sockaddr_in6 *)addr.first.get();
  } else {
    iDomain = AF_INET;
    addr4 = (sockaddr_in *)addr.first.get();
  }

  char sAddr[INET6_ADDRSTRLEN] = "\0";
  inet_ntop(iDomain, (AF_INET6 == iDomain) ? (void *)&(addr6->sin6_addr) : (void *)&addr4->sin_addr, sAddr,
            sizeof(sAddr));
  host = sAddr;
  port = (AF_INET6 == iDomain) ? ntohs(addr6->sin6_port) : ntohs(addr4->sin_port);
}

KT_Socket::addr_type KT_Socket::createSockAddr(const char *str) {
  KT_Socket::addr_type addr;

  if (KT_Socket::addressIsIPv6(str)) {
    addr.first.reset((sockaddr *)new sockaddr_in6());
    addr.second = sizeof(struct sockaddr_in6);
  } else {
    addr.first.reset((sockaddr *)new sockaddr_in());
    addr.second = sizeof(struct sockaddr_in);
  }

  return addr;
}

KT_Socket::addr_type KT_Socket::createSockAddr(const KT_Endpoint &ep) {
  KT_Socket::addr_type addr;
#if !TARGET_PLATFORM_WINDOWS
  if (ep.isUnixLocal()) {
    addr.first.reset((sockaddr *)new sockaddr_un());
    addr.second = sizeof(struct sockaddr_un);
  } else
#endif

    if (KT_Socket::addressIsIPv6(ep.getHost())) {
    addr.first.reset((sockaddr *)new sockaddr_in6());
    addr.second = sizeof(struct sockaddr_in6);
  } else {
    addr.first.reset((sockaddr *)new sockaddr_in());
    addr.second = sizeof(struct sockaddr_in);
  }

  return addr;
}

void KT_Socket::parseAddrWithPort(const string &host, int port, struct sockaddr_in &addr) {
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (host == "" || host == "0.0.0.0" || host == "*") {
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    parseAddr(host, addr.sin_addr);
  }
}

#if !TARGET_PLATFORM_WINDOWS

void KT_Socket::parseUnixLocalAddr(const char *sPathName, struct sockaddr_un &addr) {
  memset(&addr, 0x00, sizeof(addr));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, sPathName, sizeof(addr.sun_path));
}

#endif

void KT_Socket::parseAddrWithPort(const string &host, int port, struct sockaddr_in6 &addr) {
  memset(&addr, 0, sizeof(struct sockaddr_in6));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port);
  if (host == "") {
    addr.sin6_addr = in6addr_any;
  } else {
    parseAddr(host, addr.sin6_addr);
  }
}

void KT_Socket::bind(const string &sServerAddr, int port) {
  assert(_iDomain == AF_INET || _iDomain == AF_INET6);

  struct sockaddr_in6 bindAddr6;
  struct sockaddr_in bindAddr4;
  struct sockaddr *bindAddr = (AF_INET6 == _iDomain) ? (struct sockaddr *)&bindAddr6 : (struct sockaddr *)&bindAddr4;
  socklen_t len = (AF_INET6 == _iDomain) ? sizeof(bindAddr6) : sizeof(bindAddr4);

  bzero(bindAddr, len);

  if (AF_INET6 == _iDomain) {
    parseAddrWithPort(sServerAddr, port, bindAddr6);
  } else {
    parseAddrWithPort(sServerAddr, port, bindAddr4);
  }

  try {
    bind(bindAddr, len);
  } catch (KT_Socket_Exception &e) {
    std::cerr << e.what() << ", " << sServerAddr << ":" << port << endl;
    throw e;
  }
}

void KT_Socket::bind(const struct sockaddr *pstBindAddr, SOCKET_LEN_TYPE iAddrLen) {
  //如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间
  int iReuseAddr = 1;

  setSockOpt(SO_REUSEADDR, (const void *)&iReuseAddr, sizeof(int), SOL_SOCKET);

  if (::bind(_sock, pstBindAddr, iAddrLen) < 0) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::bind] bind error");
  }
}

void KT_Socket::close() {
  if (_sock != INVALID_SOCKET) {
    KT_Port::closeSocket(_sock);
    // ::close(_sock);
    _sock = INVALID_SOCKET;
  }
}

int KT_Socket::connectNoThrow(const string &sServerAddr, uint16_t port) {
  assert(_iDomain == AF_INET || _iDomain == AF_INET6);

  if (sServerAddr == "") {
    throw KT_Socket_Exception("[KT_Socket::connect] server address is empty error!");
  }

  struct sockaddr_in6 serverAddr6;
  struct sockaddr_in serverAddr4;
  struct sockaddr *serverAddr =
    (AF_INET6 == _iDomain) ? (struct sockaddr *)&serverAddr6 : (struct sockaddr *)&serverAddr4;
  socklen_t len = (AF_INET6 == _iDomain) ? sizeof(serverAddr6) : sizeof(serverAddr4);

  bzero(serverAddr, len);

  if (AF_INET6 == _iDomain) {
    serverAddr6.sin6_family = _iDomain;
    parseAddr(sServerAddr, serverAddr6.sin6_addr);
    serverAddr6.sin6_port = htons(port);
  } else {
    serverAddr4.sin_family = _iDomain;
    parseAddr(sServerAddr, serverAddr4.sin_addr);
    serverAddr4.sin_port = htons(port);
  }

  return connect(serverAddr, len);
}

int KT_Socket::connectNoThrow(const struct sockaddr *addr) {
  assert(_iDomain == AF_INET || _iDomain == AF_INET6);

  return connect(addr, (AF_INET6 == _iDomain) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in));
}

void KT_Socket::connect(const string &sServerAddr, uint16_t port) {
  int ret = connectNoThrow(sServerAddr, port);

  if (ret < 0) {
    THROW_EXCEPTION_SYSCODE(KT_SocketConnect_Exception, "[KT_Socket::connect] connect error");
  }
}

int KT_Socket::connect(const struct sockaddr *pstServerAddr, SOCKET_LEN_TYPE serverLen) {
  return ::connect(_sock, pstServerAddr, serverLen);
}

void KT_Socket::listen(int iConnBackLog) {
  if (::listen(_sock, iConnBackLog) < 0) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::listen] listen error");
  }
}

int KT_Socket::recv(void *pvBuf, size_t iLen, int iFlag) { return ::recv(_sock, (char *)pvBuf, (int)iLen, iFlag); }

int KT_Socket::send(const void *pvBuf, size_t iLen, int iFlag) {
  return ::send(_sock, (char *)pvBuf, (int)iLen, iFlag);
}

int KT_Socket::recvfrom(void *pvBuf, size_t iLen, string &sFromAddr, uint16_t &iFromPort, int iFlags) {
  int iBytes;
  struct sockaddr_in6 stFromAddr6;
  struct sockaddr_in stFromAddr4;
  struct sockaddr *stFromAddr =
    (AF_INET6 == _iDomain) ? (struct sockaddr *)&stFromAddr6 : (struct sockaddr *)&stFromAddr4;
  socklen_t iFromLen = (AF_INET6 == _iDomain) ? sizeof(stFromAddr6) : sizeof(stFromAddr4);

  bzero(stFromAddr, iFromLen);
  iBytes = recvfrom(pvBuf, iLen, stFromAddr, iFromLen, iFlags);
  if (iBytes >= 0) {
    char sAddr[INET6_ADDRSTRLEN] = "\0";
    inet_ntop(_iDomain, (AF_INET6 == _iDomain) ? (void *)&stFromAddr6.sin6_addr : (void *)&stFromAddr4.sin_addr, sAddr,
              sizeof(sAddr));
    sFromAddr = sAddr;
    iFromPort = (AF_INET6 == _iDomain) ? ntohs(stFromAddr6.sin6_port) : ntohs(stFromAddr4.sin_port);
  }
  return iBytes;
}

int KT_Socket::recvfrom(void *pvBuf, size_t iLen, struct sockaddr *pstFromAddr, SOCKET_LEN_TYPE &iFromLen, int iFlags) {
  return ::recvfrom(_sock, (char *)pvBuf, (int)iLen, iFlags, pstFromAddr, &iFromLen);
}

int KT_Socket::sendto(const void *pvBuf, size_t iLen, const string &sToAddr, uint16_t port, int iFlags) {
  struct sockaddr_in6 toAddr6;
  struct sockaddr_in toAddr4;
  struct sockaddr *toAddr = (AF_INET6 == _iDomain) ? (struct sockaddr *)&toAddr6 : (struct sockaddr *)&toAddr4;
  socklen_t len = (AF_INET6 == _iDomain) ? sizeof(toAddr6) : sizeof(toAddr4);

  bzero(toAddr, len);
  if (AF_INET6 == _iDomain) {
    toAddr6.sin6_family = _iDomain;

    if (sToAddr == "") {
      //toAddr.sin6_addr = in6addr_linklocal_allrouters;
    } else {
      parseAddr(sToAddr, toAddr6.sin6_addr);
    }
    toAddr6.sin6_port = htons(port);
  } else {
    toAddr4.sin_family = _iDomain;

    if (sToAddr == "") {
      toAddr4.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    } else {
      parseAddr(sToAddr, toAddr4.sin_addr);
    }

    toAddr4.sin_port = htons(port);
  }

  return sendto(pvBuf, iLen, toAddr, len, iFlags);
}

int KT_Socket::sendto(const void *pvBuf, size_t iLen, struct sockaddr *pstToAddr, SOCKET_LEN_TYPE iToLen, int iFlags) {
  return ::sendto(_sock, (char *)pvBuf, (int)iLen, iFlags, pstToAddr, iToLen);
}

void KT_Socket::shutdown(int iHow) {
  if (::shutdown(_sock, iHow) < 0) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::shutdown] shutdown error");
  }
}

void KT_Socket::setblock(bool bBlock) {
  assert(_sock != INVALID_SOCKET);

  setblock(_sock, bBlock);
}

int KT_Socket::setSockOpt(int opt, const void *pvOptVal, SOCKET_LEN_TYPE optLen, int level) {
  return setsockopt(_sock, level, opt, (const char *)pvOptVal, optLen);
}

int KT_Socket::getSockOpt(int opt, void *pvOptVal, SOCKET_LEN_TYPE &optLen, int level) const {
  return getsockopt(_sock, level, opt, (char *)pvOptVal, &optLen);
}

void KT_Socket::setNoCloseWait() {
  linger stLinger;
  stLinger.l_onoff = 1;   //在close socket调用后, 但是还有数据没发送完毕的时候容许逗留
  stLinger.l_linger = 0;  //容许逗留的时间为0秒

  if (setSockOpt(SO_LINGER, (const void *)&stLinger, sizeof(linger), SOL_SOCKET) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setNoCloseWait] error");
  }
}

void KT_Socket::setReuseAddr() {
  int iReuseAddr = 1;

  if (setSockOpt(SO_REUSEADDR, (const void *)&iReuseAddr, sizeof(int), SOL_SOCKET) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setReuseAddr] error");
    // throw KT_Socket_Exception("[KT_Socket::setNoCloseWait] error", KT_Exception::getSystemCode());
  }
}

void KT_Socket::setCloseWait(int delay) {
  linger stLinger;
  stLinger.l_onoff = 1;       //在close socket调用后, 但是还有数据没发送完毕的时候容许逗留
  stLinger.l_linger = delay;  //容许逗留的时间为delay秒

  if (setSockOpt(SO_LINGER, (const void *)&stLinger, sizeof(linger), SOL_SOCKET) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setCloseWait] error");

    // throw KT_Socket_Exception("[KT_Socket::setCloseWait] error", KT_Exception::getSystemCode());
  }
}

void KT_Socket::setCloseWaitDefault() {
  linger stLinger;
  stLinger.l_onoff = 0;
  stLinger.l_linger = 0;

  if (setSockOpt(SO_LINGER, (const void *)&stLinger, sizeof(linger), SOL_SOCKET) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setCloseWaitDefault] error");
  }
}

void KT_Socket::setTcpNoDelay() {
  int flag = 1;

  if (setSockOpt(TCP_NODELAY, (char *)&flag, int(sizeof(int)), IPPROTO_TCP) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setTcpNoDelay] error");
  }
}

void KT_Socket::setKeepAlive() {
  int flag = 1;
  if (setSockOpt(SO_KEEPALIVE, (char *)&flag, int(sizeof(int)), SOL_SOCKET) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setKeepAlive] error");
  }
}

void KT_Socket::setSendBufferSize(int sz) {
  if (setSockOpt(SO_SNDBUF, (char *)&sz, int(sizeof(int)), SOL_SOCKET) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setKeepsetSendBufferSizeAlive] error");
  }
}

int KT_Socket::getSendBufferSize() const {
  int sz;
  SOCKET_LEN_TYPE len = sizeof(sz);
  if (getSockOpt(SO_SNDBUF, (void *)&sz, len, SOL_SOCKET) == -1 || len != sizeof(sz)) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::getSendBufferSize] error");
  }

  return sz;
}

void KT_Socket::setRecvBufferSize(int sz) {
  if (setSockOpt(SO_RCVBUF, (char *)&sz, int(sizeof(int)), SOL_SOCKET) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setRecvBufferSize] error");
  }
}

int KT_Socket::getRecvBufferSize() const {
  int sz;
  SOCKET_LEN_TYPE len = sizeof(sz);
  if (getSockOpt(SO_RCVBUF, (void *)&sz, len, SOL_SOCKET) == -1 || len != sizeof(sz)) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::getRecvBufferSize] error");
  }

  return sz;
}

void KT_Socket::ignoreSigPipe() {
#ifdef TARGET_PLATFORM_IOS
  int set = 1;

  if (setSockOpt(SO_NOSIGPIPE, (char *)&set, int(sizeof(int)), SOL_SOCKET) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::ignoreSigPipe] error");
  }
#endif
}

void KT_Socket::setblock(SOCKET_TYPE fd, bool bBlock) {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS

  int val = 0;

  if ((val = fcntl(fd, F_GETFL, 0)) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setblock] fcntl [F_GETFL] error");
    // throw KT_Socket_Exception("[KT_Socket::setblock] fcntl [F_GETFL] error", KT_Exception::getSystemCode());
  }

  if (!bBlock) {
    val |= O_NONBLOCK;
  } else {
    val &= ~O_NONBLOCK;
  }

  if (fcntl(fd, F_SETFL, val) == -1) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setblock] fcntl [F_SETFL] error");
  }
#else
  unsigned long ul = bBlock ? 0 : 1;

  int ret;
  ret = ioctlsocket(fd, FIONBIO, (unsigned long *)&ul);
  if (ret == SOCKET_ERROR) {
    THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::setblock] ioctlsocket [FIONBIO] error");
  }

#endif
}

void KT_Socket::createPipe(int fds[2], bool bBlock) {
  int tcp1, tcp2;
  sockaddr_in name;
  memset(&name, 0, sizeof(name));
  name.sin_family = AF_INET;
  name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  SOCKET_LEN_TYPE namelen = sizeof(name);
  tcp1 = tcp2 = -1;
  int tcp = socket(AF_INET, SOCK_STREAM, 0);
  if (tcp == -1) {
    goto clean;
  }
  if (::bind(tcp, (sockaddr *)&name, namelen) == -1) {
    goto clean;
  }
  if (::listen(tcp, 5) == -1) {
    goto clean;
  }
  if (getsockname(tcp, (sockaddr *)&name, &namelen) == -1) {
    goto clean;
  }
  tcp1 = socket(AF_INET, SOCK_STREAM, 0);
  if (tcp1 == -1) {
    goto clean;
  }
  if (-1 == ::connect(tcp1, (sockaddr *)&name, namelen)) {
    goto clean;
  }
  tcp2 = ::accept(tcp, (sockaddr *)&name, &namelen);
  if (tcp2 == -1) {
    goto clean;
  }

  fds[0] = tcp1;
  fds[1] = tcp2;

  try {
#ifdef TARGET_PLATFORM_IOS
    int set = 1;
    setsockopt(tcp1, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
    setsockopt(tcp2, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif

    setblock(fds[0], bBlock);
    setblock(fds[1], bBlock);
  } catch (...) {
    goto clean;
  }

  // #if TARGET_PLATFORM_LINUX||TARGET_PLATFORM_IOS
  // #define closesocket ::close
  // #endif
  if (KT_Port::closeSocket(tcp) == -1) {
    goto clean;
  }

  return;

clean:
  if (tcp != -1) {
    KT_Port::closeSocket(tcp);
  }
  if (tcp2 != -1) {
    KT_Port::closeSocket(tcp2);
  }
  if (tcp1 != -1) {
    KT_Port::closeSocket(tcp1);
  }
  // #if TARGET_PLATFORM_LINUX||TARGET_PLATFORM_IOS
  // #undef closesocket
  // #endif
  THROW_EXCEPTION_SYSCODE(KT_Socket_Exception, "[KT_Socket::createPipe] error");
}

#if TARGET_PLATFORM_LINUX
vector<string> KT_Socket::getLocalHosts(int domain) {
  vector<string> result;
  KT_Socket ts;
  ts.createSocket(SOCK_STREAM, domain);

  int cmd = SIOCGIFCONF;

  struct ifconf ifc;

  int numaddrs = 10;

  int old_ifc_len = 0;

  while (true) {
    int bufsize = numaddrs * static_cast<int>(sizeof(struct ifreq));
    ifc.ifc_len = bufsize;
    ifc.ifc_buf = (char *)malloc(bufsize);
    int rs = ioctl(ts.getfd(), cmd, &ifc);

    if (rs == -1) {
      free(ifc.ifc_buf);
      throw KT_Socket_Exception("[KT_Socket::getLocalHosts] ioctl error", errno);
    } else if (ifc.ifc_len == old_ifc_len) {
      break;
    } else {
      old_ifc_len = ifc.ifc_len;
    }

    numaddrs += 10;
    free(ifc.ifc_buf);
  }

  numaddrs = ifc.ifc_len / static_cast<int>(sizeof(struct ifreq));
  struct ifreq *ifr = ifc.ifc_req;
  for (int i = 0; i < numaddrs; ++i) {
    if (ifr[i].ifr_addr.sa_family == AF_INET) {
      struct sockaddr_in *addr = reinterpret_cast<struct sockaddr_in *>(&ifr[i].ifr_addr);
      if (addr->sin_addr.s_addr != 0) {
        char sAddr[INET_ADDRSTRLEN] = "\0";
        inet_ntop(AF_INET, &(*addr).sin_addr, sAddr, sizeof(sAddr));
        result.push_back(sAddr);
      }
    } else if (ifr[i].ifr_addr.sa_family == AF_INET6) {
      struct sockaddr_in6 *addr = reinterpret_cast<struct sockaddr_in6 *>(&ifr[i].ifr_addr);
      if (!memcmp(&addr->sin6_addr, &in6addr_any, sizeof(addr->sin6_addr))) {
        char sAddr[INET6_ADDRSTRLEN] = "\0";
        inet_ntop(AF_INET6, &(*addr).sin6_addr, sAddr, sizeof(sAddr));
        result.push_back(sAddr);
      }
    }
  }

  free(ifc.ifc_buf);

  return result;
}
#elif TARGET_PLATFORM_IOS
vector<string> KT_Socket::getLocalHosts(int domain) {
  vector<string> hosts;
  char local[255] = {0};
  gethostname(local, sizeof(local));
  hostent *ph = gethostbyname(local);
  if (ph == NULL) {
    return hosts;
  }

  in_addr addr;
  if (ph->h_addrtype == AF_INET) {
    int i = 0;
    while (ph->h_addr_list[i] != 0) {
      addr.s_addr = *(u_long *)ph->h_addr_list[i++];
      hosts.emplace_back(inet_ntoa(addr));
    }
  } else {
    // unsupport AF_INET6  ...
    return hosts;
  }
  return hosts;
}
#endif

#if TARGET_PLATFORM_WINDOWS
vector<string> KT_Socket::getLocalHosts(int domain) {
  vector<string> hosts;
  WORD wVersionRequested = MAKEWORD(2, 2);

  WSADATA wsaData;
  if (WSAStartup(wVersionRequested, &wsaData) != 0) {
    return hosts;
  }

  char local[255] = {0};
  gethostname(local, sizeof(local));
  hostent *ph = gethostbyname(local);
  if (ph == NULL) {
    return hosts;
  }

  in_addr addr;
  if (ph->h_addrtype == AF_INET) {
    int i = 0;
    while (ph->h_addr_list[i] != 0) {
      addr.s_addr = *(u_long *)ph->h_addr_list[i++];
      hosts.emplace_back(inet_ntoa(addr));
    }
  } else {
    // unsupport AF_INET6  ...
    return hosts;
  }
  WSACleanup();
  return hosts;
}

#endif

bool KT_Socket::isPending() {
#if TARGET_PLATFORM_WINDOWS
  return KT_Exception::getSystemCode() == WSAEWOULDBLOCK;
#else
  return KT_Exception::getSystemCode() == EAGAIN;
#endif
}

bool KT_Socket::isInProgress() {
#if TARGET_PLATFORM_WINDOWS
  return KT_Exception::getSystemCode() == WSAEWOULDBLOCK;
#else
  return KT_Exception::getSystemCode() == EINPROGRESS;
#endif
}

}  // namespace kant
