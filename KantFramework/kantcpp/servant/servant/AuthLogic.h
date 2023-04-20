#include "servant/AuthF.h"
#include "util/kt_epoll_server.h"

namespace kant {
/**
 * server:默认生成鉴权请求方法
 */
pair<KT_NetWorkBuffer::PACKET_TYPE, shared_ptr<KT_NetWorkBuffer::Buffer>> serverVerifyAuthCallback(
  KT_NetWorkBuffer &, KT_Transceiver *, weak_ptr<KT_EpollServer::BindAdapter> adapter, const string &expectObj);

/**
 * client:默认生成鉴权请求方法
 */
vector<char> defaultCreateAuthReq(const BasicAuthInfo &info);

}  // namespace kant
