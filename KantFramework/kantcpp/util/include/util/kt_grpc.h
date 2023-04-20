#ifndef __KT_GRPC_PROTOCOL_H__
#define __KT_GRPC_PROTOCOL_H__

#include <unordered_map>
#include "util/kt_network_buffer.h"
#include "util/kt_spin_lock.h"
#include "util/kt_http2.h"
#include "util/kt_epoll_server.h"

namespace kant {
void addGrpcPrefix(string &body, bool compressed);
bool RemoveGrpcPrefix(string &body, bool *compressed);

class KT_GrpcServer : public KT_Http2Server {
 public:
  /**
	 * constructor
	 */
  KT_GrpcServer();

  /**
     * deconstructor
     */
  ~KT_GrpcServer();

  /**
	 *
	 * @param context
	 * @param out
	 * @return
	 */
  int encodeResponse(const shared_ptr<Http2Context> &context, std::string gStatus, vector<char> &out);

  /**
	 *
	 * @param context
	 * @param status
	 * @param body
	 * @return
	 */
  void packGrpcResponse(shared_ptr<KT_GrpcServer::Http2Context> &context, const int status, const string &body);

  static shared_ptr<KT_GrpcServer> getHttp2(uint32_t uid);
  static void addHttp2(uint32_t uid, const shared_ptr<KT_GrpcServer> &ptr);
  static void delHttp2(uint32_t uid);
  static KT_NetWorkBuffer::PACKET_TYPE parseGrpc(KT_NetWorkBuffer &in, vector<char> &out);

 protected:
  static KT_SpinLock _mutex;
  static unordered_map<int32_t, shared_ptr<KT_GrpcServer>> _http2;
};

/////////////////////////////////////////////////////////////////////////////////

class KT_GrpcClient : public KT_Http2Client {
 public:
  /**
	 * constructor
	 */
  KT_GrpcClient();

  /**
     * deconstructor
     */
  ~KT_GrpcClient();

  /**
     * parse response
     * @param in
     */
  KT_NetWorkBuffer::PACKET_TYPE parseResponse(KT_NetWorkBuffer &in, pair<int, shared_ptr<KT_HttpResponse>> &out);

  //    int submit(const string &method, const string &path, const map<string, string> &header, const vector<char> &buff);
  int submit(const KT_HttpRequest &request);
  /**
	 * @brief response
	 */
  std::unordered_map<int, shared_ptr<KT_HttpResponse>> &responses() { return _responses; }

  /** 
     * @brief response finished
     */
  std::unordered_map<int, shared_ptr<KT_HttpResponse>> &doneResponses() { return _doneResponses; }

 private:
  /**
     * 收到的响应
	 * Responses received
     */
  std::unordered_map<int, shared_ptr<KT_HttpResponse>> _responses;

  /**
     * 收到的完整响应
	 * Complete response received
     */
  std::unordered_map<int, shared_ptr<KT_HttpResponse>> _doneResponses;
};

}  // namespace kant

#endif  //__KT_GRPC_PROTOCOL_H__