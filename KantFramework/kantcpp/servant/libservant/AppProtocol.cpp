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

#include "util/kt_epoll_server.h"
#include "util/kt_http.h"
#include "util/kt_grpc.h"
#include "servant/AppProtocol.h"
// #include "servant/KT_Transceiver.h"
#include "servant/AdapterProxy.h"
#include "servant/RemoteLogger.h"
#include "tup/Kant.h"
#include <iostream>

#if KANT_HTTP2
#include "util/kt_http2.h"
#endif

namespace kant {

class KT_Transceiver;

shared_ptr<KT_NetWorkBuffer::Buffer> ProxyProtocol::toBuffer(KantOutputStream<BufferWriter> &os) {
  shared_ptr<KT_NetWorkBuffer::Buffer> buff = std::make_shared<KT_NetWorkBuffer::Buffer>();

  buff->replaceBuffer(os.getBuffer(), os.getLength());
  os._buf = NULL;
  os._buf_len = 0;
  os._len = 0;

  return buff;
}

shared_ptr<KT_NetWorkBuffer::Buffer> ProxyProtocol::kantRequest(RequestPacket &request, KT_Transceiver *) {
  KantOutputStream<BufferWriter> os;

  int iHeaderLen = 0;

  //	先预留4个字节长度
  os.writeBuf((const char *)&iHeaderLen, sizeof(iHeaderLen));

  request.writeTo(os);

  assert(os.getLength() >= 4);

  iHeaderLen = htonl((int)(os.getLength()));

  memcpy((void *)os.getBuffer(), (const char *)&iHeaderLen, sizeof(iHeaderLen));

  return toBuffer(os);
}

////////////////////////////////////////////////////////////////////////////////////

shared_ptr<KT_NetWorkBuffer::Buffer> ProxyProtocol::http1Request(kant::RequestPacket &request, KT_Transceiver *trans) {
  shared_ptr<KT_HttpRequest> &data = *(shared_ptr<KT_HttpRequest> *)request.sBuffer.data();

  shared_ptr<KT_NetWorkBuffer::Buffer> buff = std::make_shared<KT_NetWorkBuffer::Buffer>();

  if (!data->hasHeader("Host")) {
    data->setHost(trans->getEndpoint().getHost());
  }
  data->encode(buff);

  data.reset();

  return buff;
}

KT_NetWorkBuffer::PACKET_TYPE ProxyProtocol::http1Response(KT_NetWorkBuffer &in, ResponsePacket &rsp) {
  shared_ptr<KT_HttpResponse> *context = (shared_ptr<KT_HttpResponse> *)(in.getContextData());

  if (!context) {
    context = new shared_ptr<KT_HttpResponse>();
    *context = std::make_shared<KT_HttpResponse>();
    in.setContextData(context, [](KT_NetWorkBuffer *nb) {
      shared_ptr<KT_HttpResponse> *p = (shared_ptr<KT_HttpResponse> *)(nb->getContextData());
      if (!p) {
        nb->setContextData(NULL);
        delete p;
      }
    });
  }

  if ((*context)->incrementDecode(in)) {
    rsp.sBuffer.resize(sizeof(shared_ptr<KT_HttpResponse>));

    shared_ptr<KT_HttpResponse> &data = *(shared_ptr<KT_HttpResponse> *)rsp.sBuffer.data();

    data = *context;

    auto ret = KT_NetWorkBuffer::PACKET_FULL;
    if (data->checkHeader("Connection", "keep-alive") ||
        (data->getVersion() == "HTTP/1.1" && !data->hasHeader("Connection"))) {
      ret = KT_NetWorkBuffer::PACKET_FULL;
    } else {
      ret = KT_NetWorkBuffer::PACKET_FULL_CLOSE;
    }

    //收取到完整的包, 把当前包释放掉, 下次新包来会新建context
    (*context) = NULL;
    delete context;
    in.setContextData(NULL);

    return ret;
  }

  return KT_NetWorkBuffer::PACKET_LESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#if KANT_HTTP2

// ENCODE function, called by network thread
shared_ptr<KT_NetWorkBuffer::Buffer> ProxyProtocol::http2Request(RequestPacket &request, KT_Transceiver *trans) {
  KT_Http2Client *session = (KT_Http2Client *)trans->getSendBuffer().getContextData();
  if (session == NULL) {
    session = new KT_Http2Client();

    trans->getSendBuffer().setContextData(session, [=](KT_NetWorkBuffer *) { delete session; });

    session->settings(3000);
  }

  shared_ptr<KT_HttpRequest> *data = (shared_ptr<KT_HttpRequest> *)request.sBuffer.data();

  request.iRequestId = session->submit(*(*data).get());

  //这里把智能指针释放一次
  (*data).reset();

  shared_ptr<KT_NetWorkBuffer::Buffer> buff = std::make_shared<KT_NetWorkBuffer::Buffer>();

  if (request.iRequestId < 0) {
    TLOGERROR("http2Request::Fatal submit error: " << session->getErrMsg() << endl);
    return buff;
  }

  vector<char> out;
  session->swap(out);

  buff->addBuffer(out);

  return buff;
}

KT_NetWorkBuffer::PACKET_TYPE ProxyProtocol::http2Response(KT_NetWorkBuffer &in, ResponsePacket &rsp) {
  KT_Http2Client *session =
    (KT_Http2Client *)((KT_Transceiver *)(in.getConnection()))->getSendBuffer().getContextData();

  pair<int, shared_ptr<KT_HttpResponse>> out;
  KT_NetWorkBuffer::PACKET_TYPE flag = session->parseResponse(in, out);

  if (flag == KT_NetWorkBuffer::PACKET_FULL) {
    rsp.iRequestId = out.first;

    rsp.sBuffer.resize(sizeof(shared_ptr<KT_HttpResponse>));

    //这里智能指针有一次+1, 后面要自己reset掉
    *(shared_ptr<KT_HttpResponse> *)rsp.sBuffer.data() = out.second;
  }

  return flag;
}

// ENCODE function, called by network thread
shared_ptr<KT_NetWorkBuffer::Buffer> ProxyProtocol::grpcRequest(RequestPacket &request, KT_Transceiver *trans) {
  KT_GrpcClient *session = (KT_GrpcClient *)trans->getSendBuffer().getContextData();
  if (session == NULL) {
    session = new KT_GrpcClient();

    trans->getSendBuffer().setContextData(session, [=](KT_NetWorkBuffer *) { delete session; });

    session->settings(3000);
  }

  if (session->buffer().size() != 0) {
    //直接发送裸得应答数据，业务层一般不直接使用，仅仅tcp支持
    trans->getSendBuffer().addBuffer(session->buffer());

    trans->doRequest();

    //		auto data = trans->getSendBuffer().getBufferPointer();
    //		int iRet = trans->send(data.first, (uint32_t) data.second, 0);
    //		trans->getSendBuffer().moveHeader(iRet);
    session->buffer().clear();
  }

  shared_ptr<KT_HttpRequest> *data = (shared_ptr<KT_HttpRequest> *)request.sBuffer.data();

  request.iRequestId = session->submit(*(*data).get());

  //这里把智能指针释放一次
  (*data).reset();

  shared_ptr<KT_NetWorkBuffer::Buffer> buff = std::make_shared<KT_NetWorkBuffer::Buffer>();

  if (request.iRequestId < 0) {
    TLOGERROR("http2Request::Fatal submit error: " << session->getErrMsg() << endl);
    return buff;
  }

  //	cout << "http2Request id:" << request.iRequestId << endl;

  vector<char> out;
  session->swap(out);

  buff->addBuffer(out);

  return buff;
}

KT_NetWorkBuffer::PACKET_TYPE ProxyProtocol::grpcResponse(KT_NetWorkBuffer &in, ResponsePacket &rsp) {
  KT_GrpcClient *session = (KT_GrpcClient *)((KT_Transceiver *)(in.getConnection()))->getSendBuffer().getContextData();

  pair<int, shared_ptr<KT_HttpResponse>> out;
  KT_NetWorkBuffer::PACKET_TYPE flag = session->parseResponse(in, out);

  if (flag == KT_NetWorkBuffer::PACKET_FULL) {
    rsp.iRequestId = out.first;

    rsp.sBuffer.resize(sizeof(shared_ptr<KT_HttpResponse>));

    //这里智能指针有一次+1, 后面要自己reset掉
    *(shared_ptr<KT_HttpResponse> *)rsp.sBuffer.data() = out.second;
  }

  return flag;
}

#endif
}  // namespace kant
