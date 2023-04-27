#include <cerrno>
#include <iostream>
#include "util/kt_clientsocket.h"
#include "util/kt_epoller.h"
#include "util/kt_common.h"
#include "util/kt_socket.h"

namespace kant {

KT_Endpoint::KT_Endpoint() {
  _host = "0.0.0.0";
  _port = 0;
  _timeout = 3000;
  _type = TCP;
  _grid = 0;
  _qos = 0;
  _weight = -1;
  _weighttype = 0;
  _authType = AUTH_TYPENONE;
  _isIPv6 = KT_Socket::addressIsIPv6(_host);
}

void KT_Endpoint::init(const string &host, int port, int timeout, EType type, int grid, int qos, int weight,
                       unsigned int weighttype, AUTH_TYPE authType) {
  _host = host;
  _port = port;
  _timeout = timeout;
  _type = type;
  _grid = grid;
  _qos = qos;
  if (weighttype == 0) {
    _weight = -1;
    _weighttype = 0;
  } else {
    if (weight == -1) {
      weight = 100;
    }
    _weight = (weight > 100 ? 100 : weight);
    _weighttype = weighttype;
  }

  _authType = authType;

  _isIPv6 = KT_Socket::addressIsIPv6(_host);
}

void KT_Endpoint::parse(const string &str) {
  _grid = 0;
  _qos = 0;
  _weight = -1;
  _weighttype = 0;
  _authType = AUTH_TYPENONE;

  const string delim = " \t\n\r";

  string::size_type beg;
  string::size_type end = 0;

  beg = str.find_first_not_of(delim, end);
  if (beg == string::npos) {
    throw KT_EndpointParse_Exception("KT_Endpoint::parse error : " + str);
  }

  end = str.find_first_of(delim, beg);
  if (end == string::npos) {
    end = str.length();
  }

  string desc = str.substr(beg, end - beg);
  if (desc == "tcp") {
    _type = TCP;
  } else if (desc == "ssl") {
    _type = SSL;
  } else if (desc == "udp") {
    _type = UDP;
  } else {
    throw KT_EndpointParse_Exception("KT_Endpoint::parse tcp or udp or ssl error : " + str);
  }

  desc = str.substr(end);
  end = 0;
  while (true) {
    beg = desc.find_first_not_of(delim, end);
    if (beg == string::npos) {
      break;
    }

    end = desc.find_first_of(delim, beg);
    if (end == string::npos) {
      end = desc.length();
    }

    string option = desc.substr(beg, end - beg);
    if (option.length() != 2 || option[0] != '-') {
      throw KT_EndpointParse_Exception("KT_Endpoint::parse error : " + str);
    }

    string argument;
    string::size_type argumentBeg = desc.find_first_not_of(delim, end);
    if (argumentBeg != string::npos && desc[argumentBeg] != '-') {
      beg = argumentBeg;
      end = desc.find_first_of(delim, beg);
      if (end == string::npos) {
        end = desc.length();
      }
      argument = desc.substr(beg, end - beg);
    }

    switch (option[1]) {
      case 'h': {
        if (argument.empty()) {
          throw KT_EndpointParse_Exception("KT_Endpoint::parse -h error : " + str);
        }
        const_cast<string &>(_host) = argument;
        break;
      }
      case 'p': {
        istringstream p(argument);
        if (!(p >> const_cast<int &>(_port)) || !p.eof() || _port < 0 || _port > 65535) {
          throw KT_EndpointParse_Exception("KT_Endpoint::parse -p error : " + str);
        }
        break;
      }
      case 't': {
        istringstream t(argument);
        if (!(t >> const_cast<int &>(_timeout)) || !t.eof()) {
          throw KT_EndpointParse_Exception("KT_Endpoint::parse -t error : " + str);
        }
        break;
      }
      case 'g': {
        istringstream t(argument);
        if (!(t >> const_cast<int &>(_grid)) || !t.eof()) {
          throw KT_EndpointParse_Exception("KT_Endpoint::parse -g error : " + str);
        }
        break;
      }
      case 'q': {
        istringstream t(argument);
        if (!(t >> const_cast<int &>(_qos)) || !t.eof()) {
          throw KT_EndpointParse_Exception("KT_Endpoint::parse -q error : " + str);
        }
        break;
      }
      case 'w': {
        istringstream t(argument);
        if (!(t >> const_cast<int &>(_weight)) || !t.eof()) {
          throw KT_EndpointParse_Exception("KT_Endpoint::parse -w error : " + str);
        }
        break;
      }
      case 'v': {
        istringstream t(argument);
        if (!(t >> const_cast<unsigned int &>(_weighttype)) || !t.eof()) {
          throw KT_EndpointParse_Exception("KT_Endpoint::parse -v error : " + str);
        }
        break;
      }
      // auth type
      case 'e': {
        int v = 0;
        istringstream p(argument);

        if (!(p >> const_cast<int &>(v)) || !p.eof() || (v != AUTH_TYPENONE && v != AUTH_TYPELOCAL)) {
          throw KT_EndpointParse_Exception("KT_Endpoint::parse -e error : " + str);
        }

        _authType = (AUTH_TYPE)v;
        break;
      }
      default: {
        ///throw KT_EndpointParse_Exception("KT_Endpoint::parse error : " + str);
      }
    }
  }

  if (_weighttype != 0) {
    if (_weight == -1) {
      _weight = 100;
    }

    _weight = (_weight > 100 ? 100 : _weight);
  }

  if (_host.empty()) {
    throw KT_EndpointParse_Exception("KT_Endpoint::parse error : host must not be empty: " + str);
  } else if (_host == "*") {
    const_cast<string &>(_host) = "0.0.0.0";
  }
  _isIPv6 = KT_Socket::addressIsIPv6(_host);

  // if (_authType < 0)
  //     _authType = 0;
  // else if (_authType > 0)
  //     _authType = 1;
}

vector<string> KT_Endpoint::sepEndpoint(const string &sEndpoints) {
  vector<string> vEndpoints;
  bool flag = false;
  string::size_type startPos = 0;
  string::size_type sepPos = string::npos;
  for (string::size_type pos = 0; pos < sEndpoints.size(); pos++) {
    if (sEndpoints[pos] == ':' && !flag) {
      sepPos = pos;
      flag = true;
    } else if (flag) {
      if (sEndpoints[pos] == ' ') {
        continue;
      }

      if (KT_Port::strncasecmp("tcp", (sEndpoints.c_str() + pos), 3) == 0 ||
          KT_Port::strncasecmp("udp", (sEndpoints.c_str() + pos), 3) == 0 ||
          KT_Port::strncasecmp("ssl", (sEndpoints.c_str() + pos), 3) == 0) {
        string ep = KT_Common::trim(string(sEndpoints.c_str() + startPos, sepPos - startPos));
        if (!ep.empty()) {
          vEndpoints.push_back(ep);
        }
        startPos = pos;
      }

      flag = false;
    }
  }

  string ep = sEndpoints.substr(startPos, sepPos - startPos);

  if (!ep.empty()) {
    vEndpoints.push_back(ep);
  }

  return vEndpoints;
}

/*************************************KT_TCPClient**************************************/

KT_ClientSocket::KT_ClientSocket() : _port(0), _timeout(3000) {}

KT_ClientSocket::~KT_ClientSocket() {
  if (_epoller) {
    _epoller->close();
    delete _epoller;
    _epoller = NULL;
  }
}

void KT_ClientSocket::init(const string &sIp, int iPort, int iTimeout) {
  if (!_epoller) {
    _epoller = new KT_Epoller();
    _epoller->create(10, false);
    _epoller->enableET(false);
  }

  _socket.close();
  _ip = sIp;
  _port = iPort;
  _timeout = iTimeout;
  _isIPv6 = KT_Socket::addressIsIPv6(sIp);
}

void KT_ClientSocket::close() { _socket.close(); }

/*************************************KT_TCPClient**************************************/

#define LEN_MAXRECV 8196

int KT_TCPClient::checkSocket() {
  if (!_socket.isValid()) {
    try {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS

      _socket.createSocket(SOCK_STREAM, _port ? (_isIPv6 ? AF_INET6 : AF_INET) : AF_LOCAL);
      //设置非阻塞模式
      _socket.setblock(false);
      _socket.setNoCloseWait();
      _socket.setKeepAlive();
      _socket.setTcpNoDelay();

      _epoller->add(_socket.getfd(), 0, EPOLLOUT | EPOLLIN);

      int iRet;

      if (_port == 0) {
        iRet = _socket.connectNoThrow(_ip.c_str());
      } else {
        iRet = _socket.connectNoThrow(_ip, _port);
      }

      if (iRet < 0 && !KT_Socket::isInProgress()) {
        _socket.close();
        return EM_CONNECT;
      }
      int iRetCode = _epoller->wait(_timeout);

      if (iRetCode < 0) {
        _socket.close();
        return EM_SELECT;
      } else if (iRetCode == 0) {
        _socket.close();
        return EM_TIMEOUT;
      } else {
        for (int i = 0; i < iRetCode; ++i) {
          const epoll_event &ev = _epoller->get(i);

          if (KT_Epoller::errorEvent(ev)) {
            _socket.close();
            return EM_CONNECT;
          } else if (KT_Epoller::writeEvent(ev)) {
            int iVal = 0;
            socklen_t iLen = static_cast<socklen_t>(sizeof(int));
            if (_socket.getSockOpt(SO_ERROR, reinterpret_cast<char *>(&iVal), iLen) == -1 || iVal) {
              _socket.close();
              return EM_CONNECT;
            }
          }
        }
      }
      //设置为阻塞模式
      _socket.setblock(true);

#else
      _socket.createSocket(SOCK_STREAM, _isIPv6 ? AF_INET6 : AF_INET);
      //设置非阻塞模式
      _socket.setblock(false);
      _socket.setNoCloseWait();

      _epoller->add(_socket.getfd(), 0, EPOLLOUT | EPOLLIN);

      int iRet = _socket.connectNoThrow(_ip, _port);

      if (iRet < 0 && !KT_Socket::isInProgress()) {
        _socket.close();
        return EM_CONNECT;
      }
      int iRetCode = _epoller->wait(_timeout);
      if (iRetCode < 0) {
        _socket.close();
        return EM_SELECT;
      } else if (iRetCode == 0) {
        _socket.close();
        return EM_TIMEOUT;
      }

      //设置为阻塞模式
      _socket.setblock(true);
#endif
    } catch (KT_Socket_Exception &ex) {
      _socket.close();
      return EM_SOCKET;
    }
  }
  return EM_SUCCESS;
}

int KT_TCPClient::send(const char *sSendBuffer, size_t iSendLen) {
  int iRet = checkSocket();
  if (iRet < 0) {
    return iRet;
  }

  iRet = _socket.send(sSendBuffer, iSendLen);
  if (iRet < 0) {
    _socket.close();
    return EM_SEND;
  }

  return EM_SUCCESS;
}

int KT_TCPClient::recv(char *sRecvBuffer, size_t &iRecvLen) {
  int iRet = checkSocket();
  if (iRet < 0) {
    return iRet;
  }

  _epoller->mod(_socket.getfd(), 0, EPOLLIN);

  int iRetCode = _epoller->wait(_timeout);

  if (iRetCode < 0) {
    _socket.close();
    return EM_SELECT;
  } else if (iRetCode == 0) {
    _socket.close();
    return EM_TIMEOUT;
  }

  const epoll_event &ev = _epoller->get(0);
  if (KT_Epoller::errorEvent(ev)) {
    _socket.close();
    return EM_CLOSE;
  }
#if TARGET_PLATFORM_IOS || TARGET_PLATFORM_LINUX
  else
#else
  else if (KT_Epoller::readEvent(ev))
#endif
  {
    // DEBUG_COST("before recv");

    int iLen = _socket.recv((void *)sRecvBuffer, iRecvLen);
    if (iLen < 0) {
      _socket.close();
      return EM_RECV;
    } else if (iLen == 0) {
      _socket.close();
      return EM_CLOSE;
    }

    iRecvLen = iLen;
    return EM_SUCCESS;
  }

  return EM_SELECT;
}

int KT_TCPClient::recvBySep(string &sRecvBuffer, const string &sSep) {
  sRecvBuffer.clear();

  int iRet = checkSocket();
  if (iRet < 0) {
    return iRet;
  }

  bool succ = false;
  while (!succ) {
    int iRetCode = _epoller->wait(_timeout);

    if (iRetCode < 0) {
      _socket.close();
      return EM_SELECT;
    } else if (iRetCode == 0) {
      _socket.close();
      return EM_TIMEOUT;
    }

    const epoll_event &ev = _epoller->get(0);
    if (KT_Epoller::errorEvent(ev)) {
      _socket.close();
      return EM_CLOSE;
    }
#if TARGET_PLATFORM_IOS
    else
#else
    else if (KT_Epoller::readEvent(ev))
#endif
    {
      char buffer[LEN_MAXRECV] = "\0";

      int len = _socket.recv((void *)&buffer, sizeof(buffer));
      if (len < 0 && !KT_Socket::isPending()) {
        _socket.close();
        return EM_RECV;
      } else if (len == 0) {
        _socket.close();
        return EM_CLOSE;
      }

      if (len > 0) {
        sRecvBuffer.append(buffer, len);
      }

      if (sRecvBuffer.length() >= sSep.length() &&
          sRecvBuffer.compare(sRecvBuffer.length() - sSep.length(), sSep.length(), sSep) == 0) {
        break;
      }
    }
  }

  return EM_SUCCESS;
}

int KT_TCPClient::recvAll(string &sRecvBuffer) {
  sRecvBuffer.clear();

  int iRet = checkSocket();
  if (iRet < 0) {
    return iRet;
  }

  _epoller->mod(_socket.getfd(), 0, EPOLLIN);

  while (true) {
    int iRetCode = _epoller->wait(_timeout);
    if (iRetCode < 0) {
      _socket.close();
      return EM_SELECT;
    } else if (iRetCode == 0) {
      _socket.close();
      return EM_TIMEOUT;
    }

    const epoll_event &ev = _epoller->get(0);
    if (KT_Epoller::errorEvent(ev)) {
      _socket.close();
      return EM_CLOSE;
    }
#if TARGET_PLATFORM_IOS
    else
#else
    else if (KT_Epoller::readEvent(ev))
#endif
    {
      char sTmpBuffer[LEN_MAXRECV] = "\0";

      int len = _socket.recv((void *)sTmpBuffer, LEN_MAXRECV);
      if (len < 0) {
        _socket.close();
        return EM_RECV;
      } else if (len == 0) {
        _socket.close();
        return EM_SUCCESS;
      }

      sRecvBuffer.append(sTmpBuffer, len);
    }
  }

  return EM_SUCCESS;
}

int KT_TCPClient::recvLength(char *sRecvBuffer, size_t iRecvLen) {
  int iRet = checkSocket();
  if (iRet < 0) {
    return iRet;
  }

  size_t iRecvLeft = iRecvLen;
  iRecvLen = 0;

  _epoller->mod(_socket.getfd(), 0, EPOLLIN);

  while (iRecvLeft != 0) {
    int iRetCode = _epoller->wait(_timeout);
    if (iRetCode < 0) {
      _socket.close();
      return EM_SELECT;
    } else if (iRetCode == 0) {
      _socket.close();
      return EM_TIMEOUT;
    }

    const epoll_event &ev = _epoller->get(0);
    if (KT_Epoller::errorEvent(ev)) {
      _socket.close();
      return EM_CLOSE;
    }
#if TARGET_PLATFORM_IOS
    else
#else
    else if (KT_Epoller::readEvent(ev))
#endif
    {
      int len = _socket.recv((void *)(sRecvBuffer + iRecvLen), iRecvLeft);
      if (len < 0) {
        _socket.close();
        return EM_RECV;
      } else if (len == 0) {
        _socket.close();
        return EM_CLOSE;
      }

      iRecvLeft -= len;
      iRecvLen += len;
    }
  }

  return EM_SUCCESS;
}

int KT_TCPClient::sendRecv(const char *sSendBuffer, size_t iSendLen, char *sRecvBuffer, size_t &iRecvLen) {
  int iRet = send(sSendBuffer, iSendLen);
  if (iRet != EM_SUCCESS) {
    return iRet;
  }

  return recv(sRecvBuffer, iRecvLen);
}

int KT_TCPClient::sendRecvBySep(const char *sSendBuffer, size_t iSendLen, string &sRecvBuffer, const string &sSep) {
  int iRet = send(sSendBuffer, iSendLen);
  if (iRet != EM_SUCCESS) {
    return iRet;
  }

  return recvBySep(sRecvBuffer, sSep);
}

int KT_TCPClient::sendRecvLine(const char *sSendBuffer, size_t iSendLen, string &sRecvBuffer) {
  return sendRecvBySep(sSendBuffer, iSendLen, sRecvBuffer, "\r\n");
}

int KT_TCPClient::sendRecvAll(const char *sSendBuffer, size_t iSendLen, string &sRecvBuffer) {
  int iRet = send(sSendBuffer, iSendLen);
  if (iRet != EM_SUCCESS) {
    return iRet;
  }

  return recvAll(sRecvBuffer);
}

/*************************************KT_UDPClient**************************************/

int KT_UDPClient::checkSocket() {
  if (!_socket.isValid()) {
    try {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
      _socket.createSocket(SOCK_DGRAM, _port ? (_isIPv6 ? AF_INET6 : AF_INET) : AF_LOCAL);
#else
      _socket.createSocket(SOCK_DGRAM, _isIPv6 ? AF_INET6 : AF_INET);
#endif
    } catch (KT_Socket_Exception &ex) {
      _socket.close();
      return EM_SOCKET;
    }

    _epoller->add(_socket.getfd(), 0, EPOLLIN);

    try {
#if TARGET_PLATFORM_LINUX

      if (_port == 0) {
        _socket.connect(_ip.c_str());
        if (_port == 0) {
          _socket.bind(_ip.c_str());
        }
      } else
#endif
      {
        _socket.connect(_ip, _port);
      }
    } catch (KT_SocketConnect_Exception &ex) {
      _socket.close();
      return EM_CONNECT;
    } catch (KT_Socket_Exception &ex) {
      _socket.close();
      return EM_SOCKET;
    }
  }
  return EM_SUCCESS;
}

int KT_UDPClient::send(const char *sSendBuffer, size_t iSendLen) {
  int iRet = checkSocket();
  if (iRet < 0) {
    return iRet;
  }

  iRet = _socket.send(sSendBuffer, iSendLen);
  if (iRet < 0) {
    return EM_SEND;
  }

  return EM_SUCCESS;
}

int KT_UDPClient::recv(char *sRecvBuffer, size_t &iRecvLen) {
  string sTmpIp;
  uint16_t iTmpPort;

  return recv(sRecvBuffer, iRecvLen, sTmpIp, iTmpPort);
}

int KT_UDPClient::recv(char *sRecvBuffer, size_t &iRecvLen, string &sRemoteIp, uint16_t &iRemotePort) {
  int iRet = checkSocket();
  if (iRet < 0) {
    return iRet;
  }

  int iRetCode = _epoller->wait(_timeout);
  if (iRetCode < 0) {
    return EM_SELECT;
  } else if (iRetCode == 0) {
    return EM_TIMEOUT;
  }

  const epoll_event &ev = _epoller->get(0);
  if (KT_Epoller::errorEvent(ev)) {
    _socket.close();
    return EM_CLOSE;
  }
#if TARGET_PLATFORM_IOS
  else
#else
  else if (KT_Epoller::readEvent(ev))
#endif
  {
    iRet = _socket.recvfrom(sRecvBuffer, iRecvLen, sRemoteIp, iRemotePort);
    if (iRet < 0) {
      return EM_SEND;
    }

    iRecvLen = iRet;
    return EM_SUCCESS;
  }

  return EM_SELECT;
}

int KT_UDPClient::sendRecv(const char *sSendBuffer, size_t iSendLen, char *sRecvBuffer, size_t &iRecvLen) {
  int iRet = send(sSendBuffer, iSendLen);
  if (iRet != EM_SUCCESS) {
    return iRet;
  }

  return recv(sRecvBuffer, iRecvLen);
}

int KT_UDPClient::sendRecv(const char *sSendBuffer, size_t iSendLen, char *sRecvBuffer, size_t &iRecvLen,
                           string &sRemoteIp, uint16_t &iRemotePort) {
  int iRet = send(sSendBuffer, iSendLen);
  if (iRet != EM_SUCCESS) {
    return iRet;
  }

  return recv(sRecvBuffer, iRecvLen, sRemoteIp, iRemotePort);
}

}  // namespace kant
