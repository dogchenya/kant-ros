#ifndef __KT_HTTP2_H__
#define __KT_HTTP2_H__

#include "util/kt_http.h"
#include "util/kt_spin_lock.h"
#include "util/kt_network_buffer.h"

typedef struct nghttp2_session nghttp2_session;

namespace kant {

class KT_Http2 {
 public:
  /**
	 * constructor
	 */
  KT_Http2();

  /**
	 * deconstructor
	 */
  virtual ~KT_Http2();

  /**
	 * data pack
	 */
  struct DataPack {
    DataPack(const char *data, size_t length) : _dataBuf(data), _length(length) {}

    const char *_dataBuf;
    size_t _length;
    size_t _readPos = 0;
  };

  /**
	 * @brief setting
	 */
  int settings(unsigned int maxCurrentStreams = 2000);

  /**
	 * @brief  buffer
	 */
  vector<char> &buffer() { return _buff; }

  /**
	 * buffer
	 * @return
	 */
  const vector<char> &buffer() const { return _buff; }

  /**
	 * swap buff
	 * @param buff
	 */
  void swap(vector<char> &buff) { _buff.swap(buff); }

  /**
	 * insert buff
	 * @param buff
	 */
  void insertBuff(const char *buff, size_t length) { _buff.insert(_buff.end(), buff, buff + length); }

  /**
	 * @brief  session
	 */
  nghttp2_session *session() const { return _session; }

  /**
	 *
	 * @return
	 */
  const char *getErrMsg();

 protected:
  /**
	 * error code
	 */
  int _err = 0;

  /**
	 * session
	 */
  nghttp2_session *_session;

  /**
	 * data buff
	 */
  vector<char> _buff;
};

class KT_Http2Server : public KT_Http2 {
 public:
  /**
	 * constructor
	 */
  KT_Http2Server();

  /**
     * deconstructor
     */
  ~KT_Http2Server();

  /**
     * context
     */
  struct Http2Context {
    Http2Context(int32_t id) : reqId(id) {}

    int32_t reqId;
    KT_HttpRequest request;
    KT_HttpResponse response;
  };

  /**
	 * parse all request
	 * @param request
	 * @param vector<std::shared_ptr<KT_HttpRequest>>
	 * @return
	 */
  vector<shared_ptr<Http2Context>> decodeRequest();

  /**
	 *
	 * @param reqid
	 * @param response
	 * @param out
	 * @return
	 */
  int encodeResponse(const shared_ptr<Http2Context> &context, vector<char> &out);

  /**
     * http2
     * @param in
     * @param out
     * @return
     */
  KT_NetWorkBuffer::PACKET_TYPE parse(KT_NetWorkBuffer &in, vector<char> &out);

  void onHeaderCallback(int32_t streamId);
  void onHeaderCallback(int32_t streamId, const string &skey, const string &svalue);
  void onFrameRecvCallback(int32_t streamId);
  void onDataChunkRecvCallback(int32_t streamId, const char *data, size_t len);
  void onStreamCloseCallback(int32_t streamId);

 protected:
  shared_ptr<Http2Context> getContext(int32_t streamId);

  void deleteContext(int32_t streamId);

 protected:
  KT_ThreadMutex _nghttpLock;

  unordered_map<int32_t, shared_ptr<Http2Context>> _context;

  vector<shared_ptr<Http2Context>> _contextFinished;

  vector<char> _reqout;
};

/////////////////////////////////////////////////////////////////////////////////

class KT_Http2Client : public KT_Http2 {
 public:
  /**
	 * constructor
	 */
  KT_Http2Client();

  /**
     * deconstructor
     */
  ~KT_Http2Client();

  /**
     * parse response
     * @param in
     */
  KT_NetWorkBuffer::PACKET_TYPE parseResponse(KT_NetWorkBuffer &in, pair<int, shared_ptr<KT_HttpResponse>> &out);

  /**
     * submit, get net buffer to send
     * @param method
     * @param path
     * @param header
     * @param buff
     * @return
     */
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

//#endif

#endif
