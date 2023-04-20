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
#include "util/kt_des.h"
#include "util/kt_sha.h"
#include "util/kt_md5.h"
#include "servant/Application.h"
#include "servant/AuthLogic.h"
#include <iostream>
#include <cassert>
#include "tup/tup.h"

namespace kant {

AUTH_STATE processAuthReqHelper(const BasicAuthPackage& pkg, const BasicAuthInfo& info) {
  // 明文:objName, accessKey, time, hashMethod
  // 密文:use TmpKey to enc secret1;
  // and tmpKey = sha1(secret2 | timestamp);
  if (pkg.sObjName != info.sObjName) return AUTH_WRONG_OBJ;

  if (pkg.sAccessKey != info.sAccessKey) return AUTH_WRONG_AK;

  time_t now = TNOW;
  const int range = 60 * 60;
  if (!(pkg.iTime > (now - range) && pkg.iTime < (now + range))) return AUTH_WRONG_TIME;

  if (pkg.sHashMethod != "sha1") return AUTH_NOT_SUPPORT_ENC;

  // 用secret1 = sha1(password); secret2 = sha1(secret1);
  // 1.client create TmpKey use timestamp and secret2;
  // 2.client use TmpKey to enc secret1;
  // 3.server use TmpKey same as client, to dec secret1;
  // 4.server got secret1, then sha1(secret1), to compare secret2;
  // 下面这个是123456的两次sha1值
  //assert (info.sHashSecretKey2 == "69c5fcebaa65b560eaf06c3fbeb481ae44b8d618");

  string tmpKey;
  string hash2;
  {
    string hash1 = KT_SHA::sha1str(info.sHashSecretKey2.data(), info.sHashSecretKey2.size());
    hash2 = KT_SHA::sha1str(hash1.data(), hash1.size());
    string tmp = hash2;
    const char* pt = (const char*)&pkg.iTime;
    for (size_t i = 0; i < sizeof pkg.iTime; ++i) {
      tmp[i] |= pt[i];
    }

    tmpKey = KT_MD5::md5str(tmp);
  }

  string secret1;
  {
    try {
      secret1 = KT_Des::decrypt3(tmpKey.data(), pkg.sSignature.data(), pkg.sSignature.size());
    } catch (const KT_DES_Exception&) {
      return AUTH_DEC_FAIL;
    }
  }

  // 4.server got secret1, then sha1(secret1), to compare secret2;
  string clientSecret2 = KT_SHA::sha1str(secret1.data(), secret1.size());
  if (clientSecret2.size() != hash2.size() || !std::equal(clientSecret2.begin(), clientSecret2.end(), hash2.begin())) {
    return AUTH_ERROR;
  }

  return AUTH_SUCC;
}

// 只需要传入 expect 的objname；
// 内部根据obj查找access账号集
AUTH_STATE defaultProcessAuthReq(const char* request, size_t len, KT_EpollServer::BindAdapterPtr& adapter,
                                 const string& expectObj) {
  if (len <= 20) return AUTH_PROTO_ERR;

  BasicAuthPackage pkg;
  KantInputStream<BufferReader> is;
  is.setBuffer(request, len);
  try {
    pkg.readFrom(is);
  } catch (...) {
    return AUTH_PROTO_ERR;
  }

  //    KT_EpollServer::BindAdapterPtr bap = Application::getEpollServer()->getBindAdapter(expectObj);
  //    if (!bap)
  //        return AUTH_WRONG_OBJ;

  BasicAuthInfo info;
  string expectServantName = expectObj;
  info.sObjName = expectServantName;
  info.sAccessKey = pkg.sAccessKey;
  info.sHashSecretKey2 = adapter->getSk(info.sAccessKey);
  if (info.sHashSecretKey2.empty()) return AUTH_WRONG_AK;

  return processAuthReqHelper(pkg, info);
}

AUTH_STATE defaultProcessAuthReq(const string& request, KT_EpollServer::BindAdapterPtr& adapter,
                                 const string& expectObj) {
  return defaultProcessAuthReq(request.data(), request.size(), adapter, expectObj);
}

vector<char> defaultCreateAuthReq(const BasicAuthInfo& info /*, const string& hashMethod*/) {
  // 明文:objName, accessKey, time, hashMethod
  // 密文:use TmpKey to enc secret1;
  KantOutputStream<BufferWriterVector> os;
  BasicAuthPackage pkg;
  pkg.sObjName = info.sObjName;
  pkg.sAccessKey = info.sAccessKey;
  pkg.iTime = TNOW;

  string secret1 = KT_SHA::sha1str(info.sSecretKey.data(), info.sSecretKey.size());
  string secret2 = KT_SHA::sha1str(secret1.data(), secret1.size());

  // create tmpKey
  string tmpKey;
  {
    string tmp = secret2;
    const char* pt = (const char*)&pkg.iTime;
    for (size_t i = 0; i < sizeof pkg.iTime; ++i) {
      tmp[i] |= pt[i];
    }
    // 保证key是16字节
    tmpKey = KT_MD5::md5str(tmp);
  }

  pkg.sSignature = KT_Des::encrypt3(tmpKey.data(), secret1.data(), secret1.size());

  pkg.writeTo(os);

  return os.getByteBuffer();
}

pair<KT_NetWorkBuffer::PACKET_TYPE, shared_ptr<KT_NetWorkBuffer::Buffer>> serverVerifyAuthCallback(
  KT_NetWorkBuffer& buff, KT_Transceiver* trans, weak_ptr<KT_EpollServer::BindAdapter> adapterPtr,
  const string& expectObj) {
  shared_ptr<KT_NetWorkBuffer::Buffer> data = std::make_shared<KT_NetWorkBuffer::Buffer>();

  auto adapter = adapterPtr.lock();
  if (!adapter) {
    data->addBuffer("adapter release");
    return std::make_pair(KT_NetWorkBuffer::PACKET_ERR, data);
  }

  // got auth request
  RequestPacket request;

  if (adapter->isKantProtocol()) {
    KantInputStream<BufferReader> is;

    is.setBuffer(buff.mergeBuffers(), buff.getBufferLength());

    try {
      request.readFrom(is);
    } catch (...) {
      adapter->getEpollServer()->error("serverVerifyCallback protocol decode error, close connection.");
      return std::make_pair(KT_NetWorkBuffer::PACKET_ERR, data);
    }
  } else {
    request.sBuffer = buff.getBuffers();
  }

  AUTH_STATE newstate = kant::defaultProcessAuthReq(request.sBuffer.data(), request.sBuffer.size(), adapter, expectObj);
  std::string out = kant::etos((kant::AUTH_STATE)newstate);

  if (newstate != AUTH_SUCC) {
    // 验证错误,断开连接
    adapter->getEpollServer()->error("authProcess failed with new state [" + out + "]");
    return std::make_pair(KT_NetWorkBuffer::PACKET_ERR, data);
  }

  buff.clearBuffers();

  string host;
  uint16_t port;
  KT_Socket::parseAddr(trans->getClientAddr(), host, port);

  adapter->getEpollServer()->info(host + ":" + KT_Common::tostr(port) + ", auth response succ: [" + out + "]");

  if (adapter->isKantProtocol()) {
    KantOutputStream<BufferWriter> os;

    ResponsePacket response;
    response.iVersion = KANTVERSION;
    response.iRequestId = request.iRequestId;
    response.iMessageType = request.iMessageType;
    response.cPacketType = request.cPacketType;
    response.iRet = 0;
    response.sBuffer.assign(out.begin(), out.end());

    kant::Int32 iHeaderLen = 0;

    os.writeBuf((const char*)&iHeaderLen, sizeof(iHeaderLen));

    response.writeTo(os);

    iHeaderLen = htonl((int)(os.getLength()));

    memcpy((void*)os.getBuffer(), (const char*)&iHeaderLen, sizeof(iHeaderLen));

    data = ProxyProtocol::toBuffer(os);
  } else {
    data->addBuffer(out.c_str(), out.size());
  }

  return std::make_pair(KT_NetWorkBuffer::PACKET_FULL, data);
}

}  // namespace kant
