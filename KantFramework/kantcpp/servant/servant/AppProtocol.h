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

#ifndef _KANT_PROTOCOL_H_
#define _KANT_PROTOCOL_H_

#include <string>
#include <memory>
#include <map>
#include <vector>
#include "tup/RequestF.h"
#include "tup/tup.h"
#include "servant/BaseF.h"
#include "util/kt_network_buffer.h"

using namespace std;
using namespace tup;

namespace kant {

class KT_Transceiver;

#define KANT_NET_MIN_PACKAGE_SIZE 5
#define KANT_NET_MAX_PACKAGE_SIZE 1024 * 1024 * 10

template <typename T>
T net2host(T len) {
  switch (sizeof(T)) {
    case sizeof(uint8_t):
      return len;
    case sizeof(uint16_t):
      return ntohs(len);
    case sizeof(uint32_t):
      return ntohl(len);
  }
  assert(true);
  return 0;
}

class KT_Transceiver;

//////////////////////////////////////////////////////////////////////
/**
 * 协议解析
 */
class AppProtocol {
 public:
  /**
     * 解析协议
     * @param in, 目前的buffer
     * @param out, 一个完整的包
     *
     * @return int, 0表示没有接收完全, 1表示收到一个完整包
     */
  static KT_NetWorkBuffer::PACKET_TYPE parse(KT_NetWorkBuffer &in, vector<char> &out) {
    return KT_NetWorkBuffer::parseBinary4<KANT_NET_MIN_PACKAGE_SIZE, KANT_NET_MAX_PACKAGE_SIZE>(in, out);
  }

  /**
     *
     * @param T
     * @param offset
     * @param netorder
     * @param in
     * @param out
     * @return int
     */
  template <size_t offset, typename T, bool netorder>
  static KT_NetWorkBuffer::PACKET_TYPE parseStream(KT_NetWorkBuffer &in, vector<char> &out) {
    size_t len = offset + sizeof(T);

    if (in.getBufferLength() < len) {
      return KT_NetWorkBuffer::PACKET_LESS;
    }

    string header;
    in.getHeader(len, header);

    assert(header.size() == len);

    T iHeaderLen = 0;

    ::memcpy(&iHeaderLen, header.c_str() + offset, sizeof(T));

    if (netorder) {
      iHeaderLen = net2host<T>(iHeaderLen);
    }

    //长度保护一下
    if (iHeaderLen < (T)(len) || (uint32_t)iHeaderLen > KANT_NET_MAX_PACKAGE_SIZE) {
      return KT_NetWorkBuffer::PACKET_ERR;
    }

    if (in.getBufferLength() < (uint32_t)iHeaderLen) {
      return KT_NetWorkBuffer::PACKET_LESS;
    }

    in.getHeader(iHeaderLen, out);

    assert(out.size() == iHeaderLen);

    in.moveHeader(iHeaderLen);

    return KT_NetWorkBuffer::PACKET_FULL;
  }
};

//typedef std::function<vector<char>(RequestPacket&, KT_Transceiver *)> request_protocol;

typedef std::function<shared_ptr<KT_NetWorkBuffer::Buffer>(RequestPacket &, KT_Transceiver *)> request_protocol;

typedef std::function<KT_NetWorkBuffer::PACKET_TYPE(KT_NetWorkBuffer &, ResponsePacket &)> response_protocol;

//////////////////////////////////////////////////////////////////////
/**
 * 客户端自定义协议设置
 */
class ProxyProtocol {
 public:
  /**
     * 构造
     */
  ProxyProtocol() : requestFunc(streamRequest) {}

  /**
     * 将KantOutputStream<BufferWriter>换成shared_ptr<KT_NetWorkBuffer::Buffer>, 且中间没有内存copy
     * 注意: 转换后os无效了, 数据被置换到shared_ptr<KT_NetWorkBuffer::Buffer>
     */
  static shared_ptr<KT_NetWorkBuffer::Buffer> toBuffer(KantOutputStream<BufferWriter> &os);
  static shared_ptr<KT_NetWorkBuffer::Buffer> http1Request(kant::RequestPacket &request, KT_Transceiver *);
  static KT_NetWorkBuffer::PACKET_TYPE http1Response(KT_NetWorkBuffer &in, ResponsePacket &done);

#if KANT_HTTP2

  // ENCODE function, called by network thread
  static shared_ptr<KT_NetWorkBuffer::Buffer> http2Request(kant::RequestPacket &request, KT_Transceiver *);

  // DECODE function, called by network thread
  static KT_NetWorkBuffer::PACKET_TYPE http2Response(KT_NetWorkBuffer &in, ResponsePacket &done);

  // ENCODE function, called by network thread
  static shared_ptr<KT_NetWorkBuffer::Buffer> grpcRequest(kant::RequestPacket &request, KT_Transceiver *);

  // DECODE function, called by network thread
  static KT_NetWorkBuffer::PACKET_TYPE grpcResponse(KT_NetWorkBuffer &in, ResponsePacket &done);
#endif

  /**
     * 普通二进制请求包
     * @param request
     * @param buff
     */
  static shared_ptr<KT_NetWorkBuffer::Buffer> streamRequest(RequestPacket &request, KT_Transceiver *) {
    shared_ptr<KT_NetWorkBuffer::Buffer> buff = std::make_shared<KT_NetWorkBuffer::Buffer>();
    buff->addBuffer(request.sBuffer);
    return buff;
  }

  /**
     * 普通二进制包普kant请求包
     * @param request
     * @param buff
     */
  template <size_t offset, typename T, bool netorder, size_t idOffset, typename K, bool idNetorder,
            size_t packetMaxSize>
  static size_t streamResponse(KT_NetWorkBuffer &in, ResponsePacket &rsp) {
    size_t len = offset + sizeof(T);

    if (in.getBufferLength() < len) {
      return KT_NetWorkBuffer::PACKET_LESS;
    }

    string header;
    in.getHeader(len, header);

    assert(header.size() == len);

    T iHeaderLen = 0;

    ::memcpy(&iHeaderLen, header.c_str() + offset, sizeof(T));

    if (netorder) {
      iHeaderLen = net2host<T>(iHeaderLen);
    }

    //做一下保护
    size_t sizeHeaderLen = static_cast<size_t>(iHeaderLen);
    if (sizeHeaderLen > packetMaxSize || iHeaderLen == 0) {
      throw KantDecodeException("packet length too long or zero,len:(" + KT_Common::tostr(packetMaxSize) + ")" +
                                KT_Common::tostr(iHeaderLen));
    }

    //包没有接收全
    if (in.getBufferLength() < (uint32_t)iHeaderLen) {
      return KT_NetWorkBuffer::PACKET_LESS;
    }

    //把buffer读取到
    in.getHeader(iHeaderLen, rsp.sBuffer);

    K requestId;

    vector<char> tmp;
    in.moveHeader(idOffset);
    in.getHeader(sizeof(K), tmp);

    if (idNetorder) {
      requestId = net2host<K>(*(K *)tmp.data());
    }

    rsp.iRequestId = static_cast<uint32_t>(requestId);

    return KT_NetWorkBuffer::PACKET_FULL;
  }

  template <size_t offset, typename T, bool netorder, size_t idOffset, typename K, bool idNetorder>
  static size_t streamResponse(KT_NetWorkBuffer &in, ResponsePacket &done) {
    return streamResponse<offset, T, netorder, idOffset, K, idNetorder, KANT_NET_MAX_PACKAGE_SIZE>(in, done);
  }

  /**
     * wup响应包(wup的响应会放在ResponsePacket的buffer中)
     * @param request
     * @param buff
     */
  static KT_NetWorkBuffer::PACKET_TYPE tupResponse(KT_NetWorkBuffer &in, ResponsePacket &done) {
    return tupResponseLen<KANT_NET_MIN_PACKAGE_SIZE, KANT_NET_MAX_PACKAGE_SIZE>(in, done);
  }

  template <uint32_t iMinLength, uint32_t iMaxLength>
  static KT_NetWorkBuffer::PACKET_TYPE tupResponseLen(KT_NetWorkBuffer &in, ResponsePacket &rsp) {
    uint32_t len = (uint32_t)in.getBufferLength();

    //收到的字节数太少, 还需要继续接收
    if (len < sizeof(uint32_t)) return KT_NetWorkBuffer::PACKET_LESS;

    //获取包总体长度
    uint32_t iHeaderLen = in.getValueOf4();

    //做一下保护,长度大于10M
    if (iHeaderLen < iMinLength || iHeaderLen > iMaxLength) {
      throw KantDecodeException("packet length too long or too short,len:" + KT_Common::tostr(iHeaderLen));
    }

    //包没有接收全
    if (len < iHeaderLen) {
      //看看包头是否正确
      static const uint32_t head = 20;

      if (len >= head) {
        string buffer;
        in.getHeader(head, buffer);

        KantInputStream<BufferReader> is;

        is.setBuffer(buffer.c_str() + sizeof(kant::Int32), head);

        //tup回来是requestpackage
        RequestPacket rsp;

        is.read(rsp.iVersion, 1, true);

        if (rsp.iVersion != TUPVERSION) {
          throw KantDecodeException("version not correct, version:" + KT_Common::tostr(rsp.iVersion));
        }

        is.read(rsp.cPacketType, 2, true);

        if (rsp.cPacketType != KANTNORMAL) {
          throw KantDecodeException("packettype not correct, packettype:" + KT_Common::tostr((int)rsp.cPacketType));
        }

        is.read(rsp.iMessageType, 3, true);
        is.read(rsp.iRequestId, 4, true);
      }
      return KT_NetWorkBuffer::PACKET_LESS;
    } else {
      //buffer包括4个字节长度
      vector<char> buffer;
      buffer.resize(iHeaderLen);

      in.getHeader(iHeaderLen, buffer);
      KantInputStream<BufferReader> is;

      is.setBuffer(buffer.data() + sizeof(kant::Int32), buffer.size() - sizeof(kant::Int32));

      //TUP的响应包其实也是返回包
      RequestPacket req;
      req.readFrom(is);

      if (req.iVersion != TUPVERSION) {
        throw KantDecodeException("version not correct, version:" + KT_Common::tostr(req.iVersion));
      }

      if (req.cPacketType != KANTNORMAL) {
        throw KantDecodeException("packettype not correct, packettype:" + KT_Common::tostr((int)req.cPacketType));
      }

      rsp.cPacketType = req.cPacketType;
      rsp.iMessageType = req.iMessageType;
      rsp.iRequestId = req.iRequestId;
      rsp.iVersion = req.iVersion;
      rsp.context = req.context;
      //tup的响应包直接放入到sBuffer里面
      rsp.sBuffer = buffer;

      in.moveHeader(iHeaderLen);
    }

    return KT_NetWorkBuffer::PACKET_FULL;
  }
  /**
	* wup响应包(wup的响应会放在ResponsePacket的buffer中)
	* @param request
	* @param buff
	*/
  static KT_NetWorkBuffer::PACKET_TYPE jsonResponse(KT_NetWorkBuffer &in, ResponsePacket &done) {
    return jsonResponseLen<KANT_NET_MIN_PACKAGE_SIZE, KANT_NET_MAX_PACKAGE_SIZE>(in, done);
  }

  template <uint32_t iMinLength, uint32_t iMaxLength>
  static KT_NetWorkBuffer::PACKET_TYPE jsonResponseLen(KT_NetWorkBuffer &in, ResponsePacket &rsp) {
    uint32_t len = (uint32_t)in.getBufferLength();

    //收到的字节数太少, 还需要继续接收
    if (len < sizeof(uint32_t)) return KT_NetWorkBuffer::PACKET_LESS;

    //获取包总体长度
    uint32_t iHeaderLen = in.getValueOf4();

    //做一下保护,长度大于10M
    if (iHeaderLen < iMinLength || iHeaderLen > iMaxLength) {
      throw KantDecodeException("packet length too long or too short,len:" + KT_Common::tostr(iHeaderLen));
    }

    //包没有接收全
    if (len < iHeaderLen) {
      //看看包头是否正确
      static const uint32_t head = 20;

      if (len >= head) {
        string buffer;
        in.getHeader(head, buffer);

        KantInputStream<BufferReader> is;
        is.setBuffer(buffer.c_str() + sizeof(kant::Int32), head);

        is.read(rsp.iVersion, 1, false);

        if (rsp.iVersion != JSONVERSION) {
          throw KantDecodeException("json version not correct, version:" + KT_Common::tostr(rsp.iVersion));
        }

        is.read(rsp.cPacketType, 2, false);

        if (rsp.cPacketType != KANTNORMAL) {
          throw KantDecodeException("packettype not correct, packettype:" + KT_Common::tostr((int)rsp.cPacketType));
        }

        is.read(rsp.iMessageType, 3, false);
        is.read(rsp.iRequestId, 4, false);
        is.read(rsp.iRet, 5, false);

        if (rsp.iRet < KANTSERVERUNKNOWNERR) {
          throw KantDecodeException("response value not correct, value:" + KT_Common::tostr(rsp.iRet));
        }
      }

      return KT_NetWorkBuffer::PACKET_LESS;
    } else {
      vector<char> buffer;
      bool ret = in.parseBufferOf4(buffer, iMinLength, iMaxLength);
      if (!ret) {
        throw KantDecodeException("parse buffer exception");
      }

      KantInputStream<BufferReader> is;
      is.setBuffer(buffer.data(), buffer.size());

      rsp.readFrom(is);

      if (rsp.iVersion != JSONVERSION) {
        throw KantDecodeException("json version not correct, version:" + KT_Common::tostr(rsp.iVersion));
      }

      if (rsp.cPacketType != KANTNORMAL) {
        throw KantDecodeException("packettype not correct, packettype:" + KT_Common::tostr((int)rsp.cPacketType));
      }

      if (rsp.iRet < KANTSERVERUNKNOWNERR) {
        throw KantDecodeException("response value not correct, value:" + KT_Common::tostr(rsp.iRet));
      }
    }

    return KT_NetWorkBuffer::PACKET_FULL;
  }

 public:
  /**
     * kant请求包
     * @param request
     * @param buff
     */
  static shared_ptr<KT_NetWorkBuffer::Buffer> kantRequest(RequestPacket &request, KT_Transceiver *);

  /**
     * kant响应包解析
     * @param recvBuffer
     * @param done
     */
  static KT_NetWorkBuffer::PACKET_TYPE kantResponse(KT_NetWorkBuffer &in, ResponsePacket &done) {
    return kantResponseLen<KANT_NET_MIN_PACKAGE_SIZE, KANT_NET_MAX_PACKAGE_SIZE>(in, done);
  }

  template <uint32_t iMinLength, uint32_t iMaxLength>
  static KT_NetWorkBuffer::PACKET_TYPE kantResponseLen(KT_NetWorkBuffer &in, ResponsePacket &rsp) {
    uint32_t len = (uint32_t)in.getBufferLength();

    //收到的字节数太少, 还需要继续接收
    if (len < sizeof(uint32_t)) return KT_NetWorkBuffer::PACKET_LESS;

    //获取包总体长度
    uint32_t iHeaderLen = in.getValueOf4();

    //做一下保护,长度大于10M
    if (iHeaderLen < iMinLength || iHeaderLen > iMaxLength) {
      throw KantDecodeException("packet length too long or too short,len:" + KT_Common::tostr(iHeaderLen));
    }

    //包没有接收全
    if (len < iHeaderLen) {
      //看看包头是否正确
      static const uint32_t head = 20;

      if (len >= head) {
        string buffer;
        in.getHeader(head, buffer);

        KantInputStream<BufferReader> is;
        is.setBuffer(buffer.c_str() + sizeof(kant::Int32), head);

        // ResponsePacket rsp;
        is.read(rsp.iVersion, 1, false);

        if (rsp.iVersion != KANTVERSION) {
          throw KantDecodeException("version not correct, version:" + KT_Common::tostr(rsp.iVersion));
        }

        is.read(rsp.cPacketType, 2, false);

        if (rsp.cPacketType != KANTNORMAL) {
          throw KantDecodeException("packettype not correct, packettype:" + KT_Common::tostr((int)rsp.cPacketType));
        }

        is.read(rsp.iRequestId, 3, false);
        is.read(rsp.iMessageType, 4, false);
        is.read(rsp.iRet, 5, false);

        if (rsp.iRet < KANTSERVERUNKNOWNERR) {
          throw KantDecodeException("response value not correct, value:" + KT_Common::tostr(rsp.iRet));
        }
      }

      return KT_NetWorkBuffer::PACKET_LESS;
    } else {
      vector<char> buffer;
      auto ret = in.parseBufferOf4(buffer, iMinLength, iMaxLength);
      if (ret == KT_NetWorkBuffer::PACKET_LESS) {
        throw KantDecodeException("parse buffer exception");
      }

      KantInputStream<BufferReader> is;
      is.setBuffer(buffer.data(), buffer.size());

      rsp.readFrom(is);

      if (rsp.iVersion != KANTVERSION) {
        throw KantDecodeException("version not correct, version:" + KT_Common::tostr(rsp.iVersion));
      }

      if (rsp.cPacketType != KANTNORMAL) {
        throw KantDecodeException("packettype not correct, packettype:" + KT_Common::tostr((int)rsp.cPacketType));
      }

      if (rsp.iRet < KANTSERVERUNKNOWNERR) {
        throw KantDecodeException("response value not correct, value:" + KT_Common::tostr(rsp.iRet));
      }
    }

    return KT_NetWorkBuffer::PACKET_FULL;
  }

  /**
	  * kant各种协议响应包解析
	  * @param recvBuffer
	  * @param done
	  */
  static KT_NetWorkBuffer::PACKET_TYPE totalResponse(KT_NetWorkBuffer &in, ResponsePacket &done) {
    return totalResponseLen<KANT_NET_MIN_PACKAGE_SIZE, KANT_NET_MAX_PACKAGE_SIZE>(in, done);
  }

  template <uint32_t iMinLength, uint32_t iMaxLength>
  static KT_NetWorkBuffer::PACKET_TYPE totalResponseLen(KT_NetWorkBuffer &in, ResponsePacket &rsp) {
    uint32_t len = (uint32_t)in.getBufferLength();

    //收到的字节数太少, 还需要继续接收
    if (len < sizeof(uint32_t)) return KT_NetWorkBuffer::PACKET_LESS;

    //获取包总体长度
    uint32_t iHeaderLen = in.getValueOf4();

    //做一下保护,长度大于10M
    if (iHeaderLen < iMinLength || iHeaderLen > iMaxLength) {
      throw KantDecodeException("packet length too long or too short,len:" + KT_Common::tostr(iHeaderLen));
    }

    //包没有接收全
    if (len < iHeaderLen) {
      //看看包头是否正确
      static const uint32_t head = 18;

      if (len >= head) {
        string buffer;
        in.getHeader(head, buffer);

        KantInputStream<BufferReader> is;
        is.setBuffer(buffer.c_str() + sizeof(kant::Int32), head);

        // ResponsePacket rsp;
        is.read(rsp.iVersion, 1, false);

        if (rsp.iVersion != KANTVERSION && rsp.iVersion != TUPVERSION && rsp.iVersion != JSONVERSION) {
          throw KantDecodeException("version not correct, version:" + KT_Common::tostr(rsp.iVersion));
        }

        is.read(rsp.cPacketType, 2, false);

        if (rsp.cPacketType != KANTNORMAL) {
          throw KantDecodeException("packettype not correct, packettype:" + KT_Common::tostr((int)rsp.cPacketType));
        }

        is.read(rsp.iRequestId, 3, false);
        is.read(rsp.iMessageType, 4, false);
        is.read(rsp.iRet, 5, false);

        if (rsp.iRet < KANTSERVERUNKNOWNERR) {
          throw KantDecodeException("response value not correct, value:" + KT_Common::tostr(rsp.iRet));
        }
      }

      return KT_NetWorkBuffer::PACKET_LESS;
    } else {
      //看看包头是否正确
      string buffer;
      in.getHeader(iHeaderLen, buffer);

      KantInputStream<BufferReader> is;
      is.setBuffer(buffer.c_str() + sizeof(kant::Int32), iHeaderLen - sizeof(kant::Int32));

      is.read(rsp.iVersion, 1, false);

      if (rsp.iVersion == TUPVERSION) {
        //buffer包括4个字节长度
        vector<char> buffer;
        buffer.resize(iHeaderLen);

        in.getHeader(iHeaderLen, buffer);
        KantInputStream<BufferReader> is;

        is.setBuffer(buffer.data() + sizeof(kant::Int32), buffer.size() - sizeof(kant::Int32));

        //TUP的响应包其实也是返回包
        RequestPacket req;
        req.readFrom(is);

        if (req.cPacketType != KANTNORMAL) {
          throw KantDecodeException("packettype not correct, packettype:" + KT_Common::tostr((int)req.cPacketType));
        }

        rsp.cPacketType = req.cPacketType;
        rsp.iMessageType = req.iMessageType;
        rsp.iRequestId = req.iRequestId;
        rsp.iVersion = req.iVersion;
        rsp.context = req.context;
        //tup的响应包直接放入到sBuffer里面
        rsp.sBuffer = buffer;

        in.moveHeader(iHeaderLen);
      } else if (rsp.iVersion == KANTVERSION || rsp.iVersion == JSONVERSION) {
        vector<char> buffer;
        bool ret = in.parseBufferOf4(buffer, iMinLength, iMaxLength);
        if (!ret) {
          throw KantDecodeException("parse buffer exception");
        }

        KantInputStream<BufferReader> is;
        is.setBuffer(buffer.data(), buffer.size());

        rsp.readFrom(is);

        if (rsp.cPacketType != KANTNORMAL) {
          throw KantDecodeException("packettype not correct, packettype:" + KT_Common::tostr((int)rsp.cPacketType));
        }

        if (rsp.iRet < KANTSERVERUNKNOWNERR) {
          throw KantDecodeException("response value not correct, value:" + KT_Common::tostr(rsp.iRet));
        }
      } else {
        throw KantDecodeException("===>version not correct, version:" + KT_Common::tostr(rsp.iVersion));
      }
    }

    return KT_NetWorkBuffer::PACKET_FULL;
  }

 public:
  request_protocol requestFunc;

  response_protocol responseFunc;
};

//////////////////////////////////////////////////////////////////////
}  // namespace kant
#endif
