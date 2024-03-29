﻿#pragma once

#include <string>
#include <vector>
#include "util/kt_network_buffer.h"

struct ssl_st;
typedef struct ssl_st SSL;

struct bio_st;
typedef struct bio_st BIO;

struct ssl_ctx_st;
typedef struct ssl_ctx_st SSL_CTX;

#define KANT_SSL 0
namespace kant {
#if KANT_SSL
/////////////////////////////////////////////////
/** 
 *@file   kt_openssl.h
 *@brief  OpenSsl wrapper
 * 
 */
/////////////////////////////////////////////////

/** 
 *@brief  OpenSsl wrapper
 */
class KT_OpenSSL {
 public:
  /**
    * @brief constructor.
    */
  KT_OpenSSL(SSL* ssl);

  /**
    * @brief deconstructor.
    */
  ~KT_OpenSSL();

  /**
     * ctx wrapper
     */
  struct CTX {
    CTX(SSL_CTX* x) : ctx(x) {}
    SSL_CTX* ctx;
  };

  /**
     * new ssl ctx
     * @param cafile
     * @param certfile
     * @param keyfile
     * @param verifyClient
     * @return
     */
  static shared_ptr<CTX> newCtx(const std::string& cafile, const std::string& certfile, const std::string& keyfile,
                                bool verifyClient, const string& ciphers);

  /**
	 * new ssl
	 * @param ctx
	 * @return
	 */
  static shared_ptr<KT_OpenSSL> newSSL(const std::shared_ptr<KT_OpenSSL::CTX>& ctx);

  static void getMemData(BIO* bio, KT_NetWorkBuffer& buf);
  static int doSSLRead(SSL* ssl, KT_NetWorkBuffer& out);

 protected:
  /**
    * @brief deny
    */
  KT_OpenSSL(const KT_OpenSSL&);
  void operator=(const KT_OpenSSL&);

  /**
	 * init openssl
	 */
  static void initialize();
  static bool _initialize;

 public:
  /** 
     * @brief release SSL
     */
  void release();

  /** 
     * @brief init SSL
     */
  void init(bool isServer);

  /**
     * @brief  is handshake finish
     * @return true: is handshake finish
     */
  bool isHandshaked() const;

  /**
     * get error message
     * @return
     */
  string getErrMsg() const;

  /**
     * @brief  get recv buffer
     */
  KT_NetWorkBuffer* recvBuffer() { return &_plainBuf; }

  /** 
     * @brief handshake
     * @return 0: succ, !=0: fail
     */
  int doHandshake(KT_NetWorkBuffer& out, const void* data = NULL, size_t size = 0);

  /** 
     * @brief encode data before send
     * @param data, data pointer
     * @param size, data size
     * @param out, out buffer
     * @return 0: succ, !=0: fail
     */
  int write(const char* data, size_t size, KT_NetWorkBuffer& out);

  /** 
     * @brief decode data before parse protocol
     * @param data  data pointer
     * @param size  data size
     * @param out   out buffer
     * @return 0: succ, !=0: fail
     */
  int read(const void* data, size_t size, KT_NetWorkBuffer& out);

  /**
     * set read buffer size
     * @param size
     */
  void setReadBufferSize(size_t size);

  /**
	 * set write buffer size
	 * @param size
	 */
  void setWriteBufferSize(size_t size);

  friend class KT_SSLManager;

 private:
  /**
     * ssl handle
     */
  SSL* _ssl;

  /**
     * is handeshake succ
     */
  bool _bHandshaked;

  /**
     * server/client
     */
  bool _isServer;

  /**
 	* ssl error code
 	*/
  int _err;

  /**
     * recv buff
     */
  KT_NetWorkBuffer _plainBuf;
};
#else
//未开启openssl得时候，定义一个空得对象保留指针占位符
class KT_OpenSSL {
 public:
  /**
     * @brief constructor.
     */
  KT_OpenSSL(SSL* ssl){};

  /**
     * @brief deconstructor.
     */
  ~KT_OpenSSL(){};

  struct CTX {
    CTX(SSL_CTX* x) : ctx(x) {}
    SSL_CTX* ctx;
  };
};

#endif

}  // namespace kant
