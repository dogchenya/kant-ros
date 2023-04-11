#ifndef _KANT_H_
#define _KANT_H_

#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <functional>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <set>

#if defined _WIN32 || defined _WIN64
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

//支持iphone
#ifdef __APPLE__
#include "KantType.h"
#elif defined ANDROID  // android
#include "KantType.h"
#else
#include "tup/KantType.h"
#endif

#ifndef kant_likely
#if defined(__GNUC__) && __GNUC__ >= 4
#define kant_likely(x) (__builtin_expect(!!(x), 1))
#else
#define kant_likely(x) (x)
#endif
#endif

#ifndef kant_unlikely
#if defined(__GNUC__) && __GNUC__ >= 4
#define kant_unlikely(x) (__builtin_expect(!!(x), 0))
#else
#define kant_unlikely(x) (x)
#endif
#endif

//数据头类型
#define KantHeadeChar 0
#define KantHeadeShort 1
#define KantHeadeInt32 2
#define KantHeadeInt64 3
#define KantHeadeFloat 4
#define KantHeadeDouble 5
#define KantHeadeString1 6
#define KantHeadeString4 7
#define KantHeadeMap 8
#define KantHeadeList 9
#define KantHeadeStructBegin 10
#define KantHeadeStructEnd 11
#define KantHeadeZeroTag 12
#define KantHeadeSimpleList 13

//////////////////////////////////////////////////////////////////
//// 保留接口版本Kant宏定义
//编码相应的宏

#define KantReserveBuf(os, len)               \
  do {                                        \
    if (kant_likely((os)._buf_len < (len))) { \
      size_t len1 = (len) << 1;               \
      if (len1 < 128) len1 = 128;             \
      (os)._buf = (os)._reserve(os, len1);    \
      (os)._buf_len = (len1);                 \
    }                                         \
  } while (0)

#define KantWriteToHead(os, type, tag)                              \
  do {                                                              \
    if (kant_likely((tag) < 15)) {                                  \
      KantWriteUInt8TTypeBuf(os, (type) + ((tag) << 4), (os)._len); \
    } else {                                                        \
      KantWriteUInt8TTypeBuf(os, (type) + (240), (os)._len);        \
      KantWriteUInt8TTypeBuf(os, (tag), (os)._len);                 \
    }                                                               \
  } while (0)

#define KantWriteCharTypeBuf(os, val, osLen)    \
  do {                                          \
    KantReserveBuf(os, (osLen) + sizeof(Char)); \
    (*(Char*)((os)._buf + (osLen))) = (val);    \
    (osLen) += sizeof(Char);                    \
  } while (0)

#define KantWriteInt32TypeBuf(os, val, osLen)    \
  do {                                           \
    KantReserveBuf(os, (osLen) + sizeof(Int32)); \
    (*(Int32*)((os)._buf + (osLen))) = (val);    \
    (osLen) += sizeof(Int32);                    \
  } while (0)

#define KantWriteInt64TypeBuf(os, val, osLen)    \
  do {                                           \
    KantReserveBuf(os, (osLen) + sizeof(Int64)); \
    (*(Int64*)((os)._buf + (osLen))) = (val);    \
    (osLen) += sizeof(Int64);                    \
  } while (0)

#define KantWriteFloatTypeBuf(os, val, osLen)    \
  do {                                           \
    KantReserveBuf(os, (osLen) + sizeof(Float)); \
    (*(Float*)((os)._buf + (osLen))) = (val);    \
    (osLen) += sizeof(Float);                    \
  } while (0)

#define KantWriteDoubleTypeBuf(os, val, osLen)    \
  do {                                            \
    KantReserveBuf(os, (osLen) + sizeof(Double)); \
    (*(Double*)((os)._buf + (osLen))) = (val);    \
    (osLen) += sizeof(Double);                    \
  } while (0)

#define KantWriteUInt32TTypeBuf(os, val, osLen)     \
  do {                                              \
    KantReserveBuf(os, (osLen) + sizeof(uint32_t)); \
    (*(uint32_t*)((os)._buf + (osLen))) = (val);    \
    (osLen) += sizeof(uint32_t);                    \
  } while (0)

#define KantWriteUInt8TTypeBuf(os, val, osLen)     \
  do {                                             \
    KantReserveBuf(os, (osLen) + sizeof(uint8_t)); \
    (*(uint8_t*)((os)._buf + (osLen))) = (val);    \
    (osLen) += sizeof(uint8_t);                    \
  } while (0)

#define KantWriteUIntTypeBuf(os, val, osLen)            \
  do {                                                  \
    KantReserveBuf(os, (osLen) + sizeof(unsigned int)); \
    (*(unsigned int*)((os)._buf + (osLen))) = (val);    \
    (osLen) += sizeof(unsigned int);                    \
  } while (0)

#define KantWriteShortTypeBuf(os, val, osLen)    \
  do {                                           \
    KantReserveBuf(os, (osLen) + sizeof(Short)); \
    (*(Short*)((os)._buf + (osLen))) = (val);    \
    (osLen) += sizeof(Short);                    \
  } while (0)

#define KantWriteTypeBuf(os, buf, len)                        \
  do {                                                        \
    KantReserveBuf(os, (os)._len + (len));                    \
    memcpy((os)._buf + (os)._len, (const void*)(buf), (len)); \
    (os)._len += (len);                                       \
  } while (0)

//解码相应的宏
#define KantPeekTypeBuf(is, buf, offset, type)                                                     \
  do {                                                                                             \
    if (kant_unlikely((is)._cur + (offset) + sizeof(type) > (is)._buf_len)) {                      \
      char s[64];                                                                                  \
      snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)((is)._buf_len)); \
      throw KantDecodeException(s);                                                                \
    }                                                                                              \
    (buf) = (*((type*)((is)._buf + (is)._cur + (offset))));                                        \
  } while (0)

#define KantPeekTypeBufNoTag(is, offset, type)                                                     \
  do {                                                                                             \
    if (kant_unlikely((is)._cur + (offset) + sizeof(type) > (is)._buf_len)) {                      \
      char s[64];                                                                                  \
      snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)((is)._buf_len)); \
      throw KantDecodeException(s);                                                                \
    }                                                                                              \
  } while (0)

#define KantReadCharTypeBuf(is, buf)   \
  do {                                 \
    KantPeekTypeBuf(is, buf, 0, Char); \
    (is)._cur += sizeof(Char);         \
  } while (0)

#define KantReadShortTypeBuf(is, buf)   \
  do {                                  \
    KantPeekTypeBuf(is, buf, 0, Short); \
    (is)._cur += sizeof(Short);         \
  } while (0)

#define KantReadInt32TypeBuf(is, buf)   \
  do {                                  \
    KantPeekTypeBuf(is, buf, 0, Int32); \
    (is)._cur += sizeof(Int32);         \
  } while (0)

#define KantReadInt64TypeBuf(is, buf)   \
  do {                                  \
    KantPeekTypeBuf(is, buf, 0, Int64); \
    (is)._cur += sizeof(Int64);         \
  } while (0)

#define KantReadFloatTypeBuf(is, buf)   \
  do {                                  \
    KantPeekTypeBuf(is, buf, 0, Float); \
    (is)._cur += sizeof(Float);         \
  } while (0)

#define KantReadDoubleTypeBuf(is, buf)   \
  do {                                   \
    KantPeekTypeBuf(is, buf, 0, Double); \
    (is)._cur += sizeof(Double);         \
  } while (0)

#define KantReadTypeBuf(is, buf, type) \
  do {                                 \
    KantPeekTypeBuf(is, buf, 0, type); \
    (is)._cur += sizeof(type);         \
  } while (0)

#define KantReadHeadSkip(is, len) \
  do {                            \
    (is)._cur += (len);           \
  } while (0)

#define KantPeekFromHead(is, type, tag, n)    \
  do {                                        \
    (n) = 1;                                  \
    uint8_t typeTag, tmpTag;                  \
    KantPeekTypeBuf(is, typeTag, 0, uint8_t); \
    tmpTag = typeTag >> 4;                    \
    (type) = (typeTag & 0x0F);                \
    if (kant_unlikely(tmpTag == 15)) {        \
      KantPeekTypeBuf(is, tag, 1, uint8_t);   \
      (n) += 1;                               \
    } else {                                  \
      (tag) = tmpTag;                         \
    }                                         \
  } while (0)

#define readFromHead(is, type, tag)     \
  do {                                  \
    size_t n = 0;                       \
    KantPeekFromHead(is, type, tag, n); \
    KantReadHeadSkip(is, n);            \
  } while (0)

#define KantPeekFromHeadNoTag(is, type, n)    \
  do {                                        \
    (n) = 1;                                  \
    uint8_t typeTag, tmpTag;                  \
    KantPeekTypeBuf(is, typeTag, 0, uint8_t); \
    tmpTag = typeTag >> 4;                    \
    (type) = (typeTag & 0x0F);                \
    if (kant_unlikely(tmpTag == 15)) {        \
      KantPeekTypeBufNoTag(is, 1, uint8_t);   \
      (n) += 1;                               \
    }                                         \
  } while (0)

#define readFromHeadNoTag(is, type)     \
  do {                                  \
    size_t n = 0;                       \
    KantPeekFromHeadNoTag(is, type, n); \
    KantReadHeadSkip(is, n);            \
  } while (0)

#define KantPeekBuf(is, buf, len, offset)                                                          \
  do {                                                                                             \
    if (kant_unlikely((is)._cur + (offset) + (len) > (is)._buf_len)) {                             \
      char s[64];                                                                                  \
      snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)((is)._buf_len)); \
      throw KantDecodeException(s);                                                                \
    }                                                                                              \
    ::memcpy(buf, (is)._buf + (is)._cur + (offset), (len));                                        \
  } while (0)

#define KantReadBuf(is, buf, len) \
  do {                            \
    KantPeekBuf(is, buf, len, 0); \
    (is)._cur += (len);           \
  } while (0)

#define KantReadStringBuf(is, str, len)                                                            \
  do {                                                                                             \
    if (kant_unlikely((is)._cur + (len) > (is)._buf_len)) {                                        \
      char s[64];                                                                                  \
      snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)((is)._buf_len)); \
      throw KantDecodeException(s);                                                                \
    }                                                                                              \
    str.assign((is)._buf + (is)._cur, (is)._buf + (is)._cur + (len));                              \
    (is)._cur += len;                                                                              \
  } while (0)

#define KantSkipToTag(flag, tag, retHeadType, retHeadTag)                             \
  do {                                                                                \
    try {                                                                             \
      uint8_t nextHeadType, nextHeadTag;                                              \
      while (!ReaderT::hasEnd()) {                                                    \
        size_t len = 0;                                                               \
        KantPeekFromHead(*this, nextHeadType, nextHeadTag, len);                      \
        if (kant_unlikely(nextHeadType == KantHeadeStructEnd || tag < nextHeadTag)) { \
          break;                                                                      \
        }                                                                             \
        if (tag == nextHeadTag) {                                                     \
          (retHeadType) = nextHeadType;                                               \
          (retHeadTag) = nextHeadTag;                                                 \
          KantReadHeadSkip(*this, len);                                               \
          (flag) = true;                                                              \
          break;                                                                      \
        }                                                                             \
        KantReadHeadSkip(*this, len);                                                 \
        skipField(nextHeadType);                                                      \
      }                                                                               \
    } catch (KantDecodeException & e) {                                               \
    }                                                                                 \
  } while (0)

namespace kant {
//////////////////////////////////////////////////////////////////
struct KantStructBase {
 protected:
  KantStructBase() {}

  ~KantStructBase() {}
};

struct KantProtoException : public std::runtime_error {
  KantProtoException(const std::string& s) : std::runtime_error(s) {}
};

struct KantEncodeException : public KantProtoException {
  KantEncodeException(const std::string& s) : KantProtoException(s) {}
};

struct KantDecodeException : public KantProtoException {
  KantDecodeException(const std::string& s) : KantProtoException(s) {}
};

struct KantDecodeMismatch : public KantDecodeException {
  KantDecodeMismatch(const std::string& s) : KantDecodeException(s) {}
};

struct KantDecodeRequireNotExist : public KantDecodeException {
  KantDecodeRequireNotExist(const std::string& s) : KantDecodeException(s) {}
};

struct KantDecodeInvalidValue : public KantDecodeException {
  KantDecodeInvalidValue(const std::string& s) : KantDecodeException(s) {}
};

struct KantNotEnoughBuff : public KantProtoException {
  KantNotEnoughBuff(const std::string& s) : KantProtoException(s) {}
};

//////////////////////////////////////////////////////////////////
namespace {
/// 数据头信息的封装，包括类型和tag
class DataHead {
  uint8_t _type;
  uint8_t _tag;

 public:
  enum {
    eChar = 0,
    eShort = 1,
    eInt32 = 2,
    eInt64 = 3,
    eFloat = 4,
    eDouble = 5,
    eString1 = 6,
    eString4 = 7,
    eMap = 8,
    eList = 9,
    eStructBegin = 10,
    eStructEnd = 11,
    eZeroTag = 12,
    eSimpleList = 13,
  };

#pragma pack(1)
  struct helper {
    uint8_t type : 4;
    uint8_t tag : 4;
  };
#pragma pack()
 public:
  DataHead() : _type(0), _tag(0) {}
  DataHead(uint8_t type, uint8_t tag) : _type(type), _tag(tag) {}

  uint8_t getTag() const { return _tag; }
  void setTag(uint8_t t) { _tag = t; }
  uint8_t getType() const { return _type; }
  void setType(uint8_t t) { _type = t; }

  /// 读取数据头信息
  template <typename InputStreamT>
  void readFrom(InputStreamT& is) {
    size_t n = peekFrom(is);
    is.skip(n);
  }

  /// 读取头信息，但不前移流的偏移量
  template <typename InputStreamT>
  size_t peekFrom(InputStreamT& is) {
    helper h;
    size_t n = sizeof(h);
    is.peekBuf(&h, sizeof(h));
    _type = h.type;
    if (h.tag == 15) {
      is.peekBuf(&_tag, sizeof(_tag), sizeof(h));
      n += sizeof(_tag);
    } else {
      _tag = h.tag;
    }
    return n;
  }

  /// 写入数据头信息
  template <typename OutputStreamT>
  void writeTo(OutputStreamT& os) {
    writeTo(os, _type, _tag);
  }

  /// 写入数据头信息
  template <typename OutputStreamT>
  static void writeTo(OutputStreamT& os, uint8_t type, uint8_t tag) {
    helper h;
    h.type = type;
    if (tag < 15) {
      h.tag = tag;
      os.writeBuf((const char*)&h, sizeof(h));
    } else {
      h.tag = 15;
      os.writeBuf((const char*)&h, sizeof(h));
      os.writeBuf((const char*)&tag, sizeof(tag));
    }
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////
/// 缓冲区读取器封装
class BufferReader {
 private:
  BufferReader(const BufferReader&);

  BufferReader& operator=(const BufferReader&);

 public:
  const char* _buf;  ///< 缓冲区
  size_t _buf_len;   ///< 缓冲区长度
  size_t _cur;       ///< 当前位置

 public:
  BufferReader() : _buf(NULL), _buf_len(0), _cur(0) {}

  void reset() { _cur = 0; }

  /// 读取缓存
  void readBuf(void* buf, size_t len) {
    if (len <= _buf_len && (_cur + len) <= _buf_len) {
      peekBuf(buf, len);
      _cur += len;
    } else {
      char s[64];
      snprintf(s, sizeof(s), "buffer overflow when skip, over %u.", (uint32_t)_buf_len);
      throw KantDecodeException(s);
    }
  }

  /// 读取缓存，但不改变偏移量
  void peekBuf(void* buf, size_t len, size_t offset = 0) {
    if (_cur + offset + len > _buf_len) {
      char s[64];
      snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)_buf_len);
      throw KantDecodeException(s);
    }
    ::memcpy(buf, _buf + _cur + offset, len);
  }

  /// 读取缓存 for vector<char>
  template <typename Alloc>
  void readBuf(std::vector<Char, Alloc>& v, size_t len) {
    if (len <= _buf_len && (_cur + len) <= _buf_len) {
      peekBuf(v, len);
      _cur += len;
    } else {
      char s[64];
      snprintf(s, sizeof(s), "buffer overflow when skip, over %u.", (uint32_t)_buf_len);
      throw KantDecodeException(s);
    }
  }

  /// 读取缓存，但不改变偏移量 for vector<char>
  template <typename Alloc>
  void peekBuf(std::vector<Char, Alloc>& v, size_t len, size_t offset = 0) {
    if (_cur + offset + len > _buf_len) {
      char s[64];
      snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)_buf_len);
      throw KantDecodeException(s);
    }

    const char* begin = _buf + _cur + offset;
    v.assign(begin, begin + len);
  }

  /// 跳过len个字节
  void skip(size_t len) {
    if (len <= _buf_len && (_cur + len) <= _buf_len) {
      _cur += len;
    } else {
      char s[64];
      snprintf(s, sizeof(s), "buffer overflow when skip, over %u.", (uint32_t)_buf_len);
      throw KantDecodeException(s);
    }
  }

  /// 设置缓存
  void setBuffer(const char* buf, size_t len) {
    _buf = buf;
    _buf_len = len;
    _cur = 0;
  }

  /// 设置缓存
  template <typename Alloc>
  void setBuffer(const std::vector<char, Alloc>& buf) {
    _buf = buf.data();
    _buf_len = buf.size();
    _cur = 0;
  }

  /**
	 * 判断是否已经到BUF的末尾
	 */
  bool hasEnd() {
    if (_cur > _buf_len) {
      char s[64];
      snprintf(s, sizeof(s), "buffer overflow when skip, over %u.", (uint32_t)_buf_len);
      throw KantDecodeException(s);
    }
    return _cur >= _buf_len;
  }
  size_t tellp() const { return _cur; }
  const char* base() const { return _buf; }
  size_t size() const { return _buf_len; }
};

//当kant文件中含有指针型类型的数据用MapBufferReader读取
//在读数据时利用MapBufferReader提前分配的内存 减少运行过程中频繁内存分配
//结构中定义byte指针类型，指针用*来定义，如下：
//byte *m;
//指针类型使用时需要MapBufferReader提前设定预分配内存块setMapBuffer()，
//指针需要内存时通过偏移指向预分配内存块，减少解码过程中的内存申请
class MapBufferReader : public BufferReader {
 private:
  MapBufferReader(const MapBufferReader&);

  MapBufferReader& operator=(const MapBufferReader&);

 public:
  MapBufferReader() : _buf_m(NULL), _buf_len_m(0), _cur_m(0) {}

  void reset() {
    _cur_m = 0;
    BufferReader::reset();
  }

  char* cur() {
    if (kant_unlikely(_buf_m == NULL)) {
      char s[64];
      snprintf(s, sizeof(s), "MapBufferReader's buff not set,_buf = null");
      throw KantDecodeException(s);
    }
    return _buf_m + _cur_m;
  }

  size_t left() { return _buf_len_m - _cur_m; }

  /// 跳过len个字节
  void mapBufferSkip(size_t len) {
    if (kant_unlikely(_cur_m + len > _buf_len_m)) {
      char s[64];
      snprintf(s, sizeof(s), "MapBufferReader's buffer overflow when peekBuf, over %u.", (uint32_t)_buf_len_m);
      throw KantDecodeException(s);
    }
    _cur_m += len;
  }

  /// 设置缓存
  void setMapBuffer(char* buf, size_t len) {
    _buf_m = buf;
    _buf_len_m = len;
    _cur_m = 0;
  }

  /// 设置缓存
  template <typename Alloc>
  void setMapBuffer(std::vector<char, Alloc>& buf) {
    _buf_m = buf.data();
    _buf_len_m = buf.size();
    _cur_m = 0;
  }

 public:
  char* _buf_m;       ///< 缓冲区
  size_t _buf_len_m;  ///< 缓冲区长度
  size_t _cur_m;      ///< 当前位置
};

//////////////////////////////////////////////////////////////////
/// 缓冲区写入器封装
class BufferWriter {
 public:
  char* _buf;
  size_t _len;
  size_t _buf_len;
  std::function<char*(BufferWriter&, size_t)> _reserve = BufferWriter::reserve;  //扩展空间

  static char* reserve(BufferWriter& os, size_t len) {
    char* p = new char[(len)];
    memcpy(p, (os)._buf, (os)._len);
    delete[](os)._buf;
    return p;
  }

 private:
  BufferWriter(const BufferWriter& bw);
  BufferWriter& operator=(const BufferWriter& buf);

 public:
  BufferWriter() : _buf(NULL), _len(0), _buf_len(0) {}
  ~BufferWriter() { delete[] _buf; }

  void reset() { _len = 0; }
  void writeBuf(const char* buf, size_t len) {
    KantReserveBuf(*this, _len + len);
    memcpy(_buf + _len, buf, len);
    _len += len;
  }
  std::vector<char> getByteBuffer() const { return std::vector<char>(_buf, _buf + _len); }
  const char* getBuffer() const { return _buf; }
  size_t getLength() const { return _len; }
  void swap(std::vector<char>& v) { v.assign(_buf, _buf + _len); }
  void swap(std::string& v) { v.assign(_buf, _len); }
  void swap(BufferWriter& buf) {
    std::swap(_buf, buf._buf);
    std::swap(_buf_len, buf._buf_len);
    std::swap(_len, buf._len);
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// 实际buffer是std::string
/// 可以swap, 把buffer交换出来, 避免一次内存copy
class BufferWriterString {
 protected:
  mutable std::string _buffer;
  char* _buf;
  size_t _len;
  size_t _buf_len;
  std::function<char*(BufferWriterString&, size_t)> _reserve;

 private:
  //不让copy 复制
  BufferWriterString(const BufferWriterString&);
  BufferWriterString& operator=(const BufferWriterString& buf);

 public:
  BufferWriterString() : _buf(NULL), _len(0), _buf_len(0) {
#ifndef GEN_PYTHON_MASK
    //内存分配器
    _reserve = [](BufferWriterString& os, size_t len) {
      os._buffer.resize(len);
      return (char*)os._buffer.data();
    };
#endif
  }

  ~BufferWriterString() {}

  void reset() { _len = 0; }

  void writeBuf(const char* buf, size_t len) {
    KantReserveBuf(*this, _len + len);
    memcpy(_buf + _len, buf, len);
    _len += len;
  }

  const std::string& getByteBuffer() const {
    _buffer.resize(_len);
    return _buffer;
  }
  std::string& getByteBuffer() {
    _buffer.resize(_len);
    return _buffer;
  }
  const char* getBuffer() const { return _buf; }
  size_t getLength() const { return _len; }
  void swap(std::string& v) {
    _buffer.resize(_len);
    v.swap(_buffer);
    _buf = NULL;
    _buf_len = 0;
    _len = 0;
  }
  void swap(std::vector<char>& v) {
    _buffer.resize(_len);
    v.assign(_buffer.c_str(), _buffer.c_str() + _buffer.size());
    _buf = NULL;
    _buf_len = 0;
    _len = 0;
  }
  void swap(BufferWriterString& buf) {
    buf._buffer.swap(_buffer);
    std::swap(_buf, buf._buf);
    std::swap(_buf_len, buf._buf_len);
    std::swap(_len, buf._len);
  }
};

/// 实际buffer是std::vector<char>
/// 可以swap, 把buffer交换出来, 避免一次内存copy
class BufferWriterVector {
 protected:
  mutable std::vector<char> _buffer;
  char* _buf;
  size_t _len;
  size_t _buf_len;
  std::function<char*(BufferWriterVector&, size_t)> _reserve;

 private:
  //不让copy 复制
  BufferWriterVector(const BufferWriterVector&);
  BufferWriterVector& operator=(const BufferWriterVector& buf);

 public:
  BufferWriterVector() : _buf(NULL), _len(0), _buf_len(0) {
#ifndef GEN_PYTHON_MASK
    //内存分配器
    _reserve = [](BufferWriterVector& os, size_t len) {
      os._buffer.resize(len);
      return os._buffer.data();
    };
#endif
  }

  ~BufferWriterVector() {}

  void reset() { _len = 0; }

  void writeBuf(const char* buf, size_t len) {
    KantReserveBuf(*this, _len + len);
    memcpy(_buf + _len, buf, len);
    _len += len;
  }

  const std::vector<char>& getByteBuffer() const {
    _buffer.resize(_len);
    return _buffer;
  }
  std::vector<char>& getByteBuffer() {
    _buffer.resize(_len);
    return _buffer;
  }
  const char* getBuffer() const { return _buf; }
  size_t getLength() const { return _len; }
  void swap(std::string& v) {
    _buffer.resize(_len);
    v.assign(_buffer.data(), _buffer.size());
    _buf = NULL;
    _buf_len = 0;
    _len = 0;
  }
  void swap(std::vector<char>& v) {
    _buffer.resize(_len);
    v.swap(_buffer);
    _buf = NULL;
    _buf_len = 0;
    _len = 0;
  }
  void swap(BufferWriterVector& buf) {
    buf._buffer.swap(_buffer);
    std::swap(_buf, buf._buf);
    std::swap(_buf_len, buf._buf_len);
    std::swap(_len, buf._len);
  }
};

//////////////////////////////////////////////////////////////////
template <typename ReaderT = BufferReader>
class KantInputStream : public ReaderT {
 public:
  /// 跳到指定标签的元素前
  bool skipToTag(uint8_t tag) {
    try {
      uint8_t headType = 0, headTag = 0;
      while (!ReaderT::hasEnd()) {
        size_t len = 0;
        KantPeekFromHead(*this, headType, headTag, len);
        if (tag <= headTag || headType == KantHeadeStructEnd)
          return headType == KantHeadeStructEnd ? false : (tag == headTag);
        KantReadHeadSkip(*this, len);
        skipField(headType);
      }
    } catch (KantDecodeException& e) {
    }
    return false;
  }

  /// 跳到当前结构的结束
  void skipToStructEnd() {
    uint8_t headType = 0;
    do {
      readFromHeadNoTag(*this, headType);
      skipField(headType);
    } while (headType != KantHeadeStructEnd);
  }

  /// 跳过一个字段
  void skipField() {
    uint8_t headType = 0;
    readFromHeadNoTag(*this, headType);
    skipField(headType);
  }

  /// 跳过一个字段，不包含头信息
  void skipField(uint8_t type) {
    switch (type) {
      case KantHeadeChar:
        KantReadHeadSkip(*this, sizeof(Char));
        break;
      case KantHeadeShort:
        KantReadHeadSkip(*this, sizeof(Short));
        break;
      case KantHeadeInt32:
        KantReadHeadSkip(*this, sizeof(Int32));
        break;
      case KantHeadeInt64:
        KantReadHeadSkip(*this, sizeof(Int64));
        break;
      case KantHeadeFloat:
        KantReadHeadSkip(*this, sizeof(Float));
        break;
      case KantHeadeDouble:
        KantReadHeadSkip(*this, sizeof(Double));
        break;
      case KantHeadeString1: {
        size_t len = 0;
        KantReadTypeBuf(*this, len, uint8_t);
        KantReadHeadSkip(*this, len);
      } break;
      case KantHeadeString4: {
        uint32_t len = 0;
        KantReadTypeBuf(*this, len, uint32_t);
        len = ntohl((uint32_t)len);
        KantReadHeadSkip(*this, len);
      } break;
      case KantHeadeMap: {
        UInt32 size = 0;
        read(size, 0);
        for (UInt32 i = 0; i < size * 2; ++i) skipField();
      } break;
      case KantHeadeList: {
        UInt32 size = 0;
        read(size, 0);
        for (UInt32 i = 0; i < size; ++i) skipField();
      } break;
      case KantHeadeSimpleList: {
        uint8_t headType = 0, headTag = 0;
        readFromHead(*this, headType, headTag);
        if (kant_unlikely(headType != KantHeadeChar)) {
          char s[64];
          snprintf(s, sizeof(s), "skipField with invalid type, type value: %d, %d, %d.", type, headType, headTag);
          throw KantDecodeMismatch(s);
        }
        UInt32 size = 0;
        read(size, 0);
        KantReadHeadSkip(*this, size);
      } break;
      case KantHeadeStructBegin:
        skipToStructEnd();
        break;
      case KantHeadeStructEnd:
      case KantHeadeZeroTag:
        break;
      default: {
        char s[64];
        snprintf(s, sizeof(s), "skipField with invalid type, type value:%d.", type);
        throw KantDecodeMismatch(s);
      }
    }
  }

  /// 读取一个指定类型的数据（基本类型）
  template <typename T>
  inline T readByType() {
    T n;
    this->readBuf(&n, sizeof(n));
    return n;
  }
  void readUnknown(std::string& sUnkown, uint8_t tag) {
    size_t start = ReaderT::tellp();
    size_t last = start;
    try {
      uint8_t lasttag = tag;
      DataHead h;
      while (!ReaderT::hasEnd()) {
        size_t len = h.peekFrom(*this);
        if (h.getTag() <= lasttag) {
          break;
        }
        lasttag = h.getTag();
        this->skip(len);
        skipField(h.getType());
        last = ReaderT::tellp();  //记录下最后一次正常到达的位置
      }
    } catch (...)  //
    {
      throw;
    }
    std::string s(ReaderT::base() + start, last - start);
    sUnkown = s;
    return;
  }
  friend class XmlProxyCallback;

  void read(Bool& b, uint8_t tag, bool isRequire = true) {
    Char c = b;
    read(c, tag, isRequire);
    b = c ? true : false;
    if (tag) {
    }  //avoid compiler warning
  }

  void read(Char& c, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeZeroTag:
          c = 0;
          break;
        case KantHeadeChar:
          KantReadTypeBuf(*this, c, Char);
          break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'Char' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d.", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  void read(UInt8& n, uint8_t tag, bool isRequire = true) {
    Short i = (Short)n;
    read(i, tag, isRequire);
    n = (UInt8)i;
  }

  void read(Short& n, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeZeroTag:
          n = 0;
          break;
        case KantHeadeChar:
          KantReadTypeBuf(*this, n, Char);
          break;
        case KantHeadeShort:
          KantReadTypeBuf(*this, n, Short);
          n = ntohs(n);
          break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'Short' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  void read(UInt16& n, uint8_t tag, bool isRequire = true) {
    Int32 i = (Int32)n;
    read(i, tag, isRequire);
    n = (UInt16)i;
  }

  void read(Int32& n, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 1, headTag = 1;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeZeroTag:
          n = 0;
          break;
        case KantHeadeChar:
          KantReadTypeBuf(*this, n, Char);
          break;
        case KantHeadeShort:
          KantReadTypeBuf(*this, n, Short);
          n = (Short)ntohs(n);
          break;
        case KantHeadeInt32:
          KantReadTypeBuf(*this, n, Int32);
          n = ntohl(n);
          break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'Int32' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d headType: %d, headTag: %d", tag, headType, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  void read(UInt32& n, uint8_t tag, bool isRequire = true) {
    Int64 i = (Int64)n;
    read(i, tag, isRequire);
    n = (UInt32)i;
  }

  void read(Int64& n, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeZeroTag:
          n = 0;
          break;
        case KantHeadeChar:
          KantReadTypeBuf(*this, n, Char);
          break;
        case KantHeadeShort:
          KantReadTypeBuf(*this, n, Short);
          n = (Short)ntohs(n);
          break;
        case KantHeadeInt32:
          KantReadTypeBuf(*this, n, Int32);
          n = (Int32)ntohl(n);
          break;
        case KantHeadeInt64:
          KantReadTypeBuf(*this, n, Int64);
          n = kant_ntohll(n);
          break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'Int64' type mismatch, tag: %d, headTag: %c, get type: %c.", tag, headTag,
                   headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  void read(Float& n, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeZeroTag:
          n = 0;
          break;
        case KantHeadeFloat:
          KantReadTypeBuf(*this, n, float);
          n = kant_ntohf(n);
          break;
        default: {
          char s[96];
          snprintf(s, sizeof(s), "read 'Float' type mismatch, tag: %d, get type: %d, headTag: %d", tag, headType,
                   headTag);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  void read(Double& n, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeZeroTag:
          n = 0;
          break;
        case KantHeadeFloat:
          KantReadTypeBuf(*this, n, float);
          n = kant_ntohf(n);
          break;
        case KantHeadeDouble:
          KantReadTypeBuf(*this, n, double);
          n = kant_ntohd(n);
          break;
        default: {
          char s[96];
          snprintf(s, sizeof(s), "read 'Double' type mismatch, tag: %d, get type: %d, headType: %d.", tag, headType,
                   headTag);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  void read(std::string& s, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      uint32_t strLength = 0;
      switch (headType) {
        case KantHeadeString1: {
          KantReadTypeBuf(*this, strLength, uint8_t);
        } break;
        case KantHeadeString4: {
          KantReadTypeBuf(*this, strLength, uint32_t);
          strLength = ntohl(strLength);
          if (kant_unlikely(strLength > TARS_MAX_STRING_LENGTH)) {
            char s[128];
            snprintf(s, sizeof(s), "invalid string size, tag: %d, size: %d, headTag: %d", tag, strLength, headTag);
            throw KantDecodeInvalidValue(s);
          }
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'string' type mismatch, tag: %d, get type: %d, tag: %d.", tag, headType,
                   headTag);
          throw KantDecodeMismatch(s);
        }
      }
      KantReadStringBuf(*this, s, strLength);
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  void read(char* buf, const UInt32 bufLen, UInt32& readLen, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeSimpleList: {
          uint8_t hheadType, hheadTag;
          readFromHead(*this, hheadType, hheadTag);
          if (kant_unlikely(hheadType != KantHeadeChar)) {
            char s[128];
            snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d, %d, %d", tag, headType, hheadType, hheadTag);
            throw KantDecodeMismatch(s);
          }
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > bufLen)) {
            char s[128];
            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, %d, size: %d", tag, headType, hheadType, size);
            throw KantDecodeInvalidValue(s);
          }
          //KantReadTypeBuf(*this, size, UInt32);
          this->readBuf(buf, size);
          readLen = size;
        } break;

        default: {
          char s[128];
          snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[128];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename K, typename V, typename Cmp, typename Alloc>
  void read(std::map<K, V, Cmp, Alloc>& m, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeMap: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid map, tag: %d, size: %d, headTag: %d", tag, size, headTag);
            throw KantDecodeInvalidValue(s);
          }
          m.clear();

          for (UInt32 i = 0; i < size; ++i) {
            std::pair<K, V> pr;
            read(pr.first, 0);
            read(pr.second, 1);
            m.insert(pr);
          }
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'map' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename K, typename V, typename H, typename Cmp, typename Alloc>
  void read(std::unordered_map<K, V, H, Cmp, Alloc>& m, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeMap: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid unordered_map, tag: %d, size: %d, headTag: %d", tag, size, headTag);
            throw KantDecodeInvalidValue(s);
          }
          m.clear();

          for (UInt32 i = 0; i < size; ++i) {
            std::pair<K, V> pr;
            read(pr.first, 0);
            read(pr.second, 1);
            m.insert(pr);
          }
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'map' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename CV, typename K, typename V, typename Cmp, typename Alloc>
  void readEx(std::map<K, V, Cmp, Alloc>& m, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeMap: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid map, tag: %d, size: %d, headTag: %d", tag, size, headTag);
            throw KantDecodeInvalidValue(s);
          }
          m.clear();

          for (UInt32 i = 0; i < size; ++i) {
            std::pair<K, V> pr;
            read(pr.first, 0);
            CV tmp(pr.second);
            read(tmp, 1);

            m.insert(pr);
          }
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'map' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename CV, typename K, typename V, typename H, typename Cmp, typename Alloc>
  void readEx(std::unordered_map<K, V, H, Cmp, Alloc>& m, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeMap: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid map, tag: %d, size: %d, headTag: %d", tag, size, headTag);
            throw KantDecodeInvalidValue(s);
          }
          m.clear();

          for (UInt32 i = 0; i < size; ++i) {
            std::pair<K, V> pr;
            read(pr.first, 0);
            CV tmp(pr.second);
            read(tmp, 1);

            m.insert(pr);
          }
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'map' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename Alloc>
  void read(std::vector<Char, Alloc>& v, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeSimpleList: {
          uint8_t hheadType, hheadTag;
          readFromHead(*this, hheadType, hheadTag);
          if (kant_unlikely(hheadType != KantHeadeChar)) {
            char s[128];
            snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d, %d, %d", tag, headType, hheadType, hheadTag);
            throw KantDecodeMismatch(s);
          }
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, %d, size: %d", tag, headType, hheadType, size);
            throw KantDecodeInvalidValue(s);
          }

          v.reserve(size);
          v.resize(size);

          this->readBuf(v.data(), size);
          //KantReadTypeBuf(*this, v[0], Int32);
        } break;
        case KantHeadeList: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d", tag, headType, size);
            throw KantDecodeInvalidValue(s);
          }
          v.reserve(size);
          v.resize(size);
          for (UInt32 i = 0; i < size; ++i) read(v[i], 0);
        } break;
        default: {
          char s[128];
          snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[128];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename T, typename Alloc>
  void read(std::vector<T, Alloc>& v, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeList: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d, headTag: %d", tag, headType, size,
                     headTag);
            throw KantDecodeInvalidValue(s);
          }
          v.reserve(size);
          v.resize(size);
          for (UInt32 i = 0; i < size; ++i) read(v[i], 0);
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'vector' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename T, typename Cmp, typename Alloc>
  void read(std::set<T, Cmp, Alloc>& v, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeList: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d, headTag: %d", tag, headType, size,
                     headTag);
            throw KantDecodeInvalidValue(s);
          }
          //					    v.reserve(size);
          //					    v.resize(size);
          for (UInt32 i = 0; i < size; ++i) {
            T t;
            read(t, 0);
            v.insert(t);
          }
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'set' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename T, typename H, typename Cmp, typename Alloc>
  void read(std::unordered_set<T, H, Cmp, Alloc>& v, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeList: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d, headTag: %d", tag, headType, size,
                     headTag);
            throw KantDecodeInvalidValue(s);
          }
          //					    v.reserve(size);
          //					    v.resize(size);
          for (UInt32 i = 0; i < size; ++i) {
            T t;
            read(t, 0);
            v.insert(t);
          }
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'set' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename CV, typename T, typename Cmp, typename Alloc>
  void readEx(std::set<T, Cmp, Alloc>& v, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeList: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d, headTag: %d", tag, headType, size,
                     headTag);
            throw KantDecodeInvalidValue(s);
          }
          //					    v.reserve(size);
          //					    v.resize(size);
          for (UInt32 i = 0; i < size; ++i) {
            T t;
            CV tmp(t);
            read(tmp, 0);
            v.insert(t);
          }
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'set' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename CV, typename T, typename H, typename Cmp, typename Alloc>
  void readEx(std::unordered_set<T, H, Cmp, Alloc>& v, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeList: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d, headTag: %d", tag, headType, size,
                     headTag);
            throw KantDecodeInvalidValue(s);
          }
          //					    v.reserve(size);
          //					    v.resize(size);
          for (UInt32 i = 0; i < size; ++i) {
            T t;
            CV tmp(t);
            read(tmp, 0);
            v.insert(t);
          }
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'set' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename CV, typename T, typename Alloc>
  void readEx(std::vector<T, Alloc>& v, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeList: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d, headTag: %d", tag, headType, size,
                     headTag);
            throw KantDecodeInvalidValue(s);
          }
          v.reserve(size);
          v.resize(size);
          for (UInt32 i = 0; i < size; ++i) {
            CV tmp(v[i]);
            read(tmp, 0);
          }
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'vector' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  /// 读取结构数组
  template <typename T>
  void read(T* v, const UInt32 len, UInt32& readLen, uint8_t tag, bool isRequire = true) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      switch (headType) {
        case KantHeadeList: {
          UInt32 size = 0;
          read(size, 0);
          if (kant_unlikely(size > this->size())) {
            char s[128];
            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d", tag, headType, size);
            throw KantDecodeInvalidValue(s);
          }
          for (UInt32 i = 0; i < size; ++i) read(v[i], 0);
          readLen = size;
        } break;
        default: {
          char s[64];
          snprintf(s, sizeof(s), "read 'vector struct' type mismatch, tag: %d, get type: %d.", tag, headType);
          throw KantDecodeMismatch(s);
        }
      }
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  template <typename T>
  void read(T& v, uint8_t tag, bool isRequire = true,
            typename detail::disable_if<detail::is_convertible<T*, KantStructBase*>, void***>::type dummy = 0) {
    Int32 n = 0;
    read(n, tag, isRequire);
    v = (T)n;
  }

  template <typename T>
  void read(T&& v, uint8_t tag, bool isRequire = true,
            typename detail::enable_if<detail::is_convertible<T*, KantStructBase*>, void***>::type dummy = 0) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      if (kant_unlikely(headType != KantHeadeStructBegin)) {
        char s[64];
        snprintf(s, sizeof(s), "read 'struct' type mismatch, tag: %d, get type: %d, headTag: %d.", tag, headType,
                 headTag);
        throw KantDecodeMismatch(s);
      }

      // 精度保存恢复都在 readFrom 里面做
      v.readFrom(*this);

      skipToStructEnd();
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
      throw KantDecodeRequireNotExist(s);
    }
  }

  /// 读取结构
  template <typename T>
  void read(T& v, uint8_t tag, bool isRequire = true,
            typename detail::enable_if<detail::is_convertible<T*, KantStructBase*>, void***>::type dummy = 0) {
    uint8_t headType = 0, headTag = 0;
    bool skipFlag = false;
    KantSkipToTag(skipFlag, tag, headType, headTag);
    if (kant_likely(skipFlag)) {
      if (kant_unlikely(headType != KantHeadeStructBegin)) {
        char s[64];
        snprintf(s, sizeof(s), "read 'struct' type mismatch, tag: %d, get type: %c, headTag: %c.", tag, headType,
                 headTag);
        throw KantDecodeMismatch(s);
      }
      v.readFrom(*this);
      skipToStructEnd();
    } else if (kant_unlikely(isRequire)) {
      char s[64];
      snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
      throw KantDecodeRequireNotExist(s);
    }
  }
};

//////////////////////////////////////////////////////////////////
template <typename WriterT = BufferWriter>
class KantOutputStream : public WriterT {
 public:
  void writeUnknown(const std::string& s) { this->writeBuf(s.data(), s.size()); }
  void writeUnknownV2(const std::string& s) {
    DataHead::writeTo(*this, DataHead::eStructBegin, 0);
    this->writeBuf(s.data(), s.size());
    DataHead::writeTo(*this, DataHead::eStructEnd, 0);
  }
  void write(Bool b, uint8_t tag) { write((Char)b, tag); }

  void write(Char n, uint8_t tag) {
    if (kant_unlikely(n == 0)) {
      KantWriteToHead(*this, KantHeadeZeroTag, tag);
    } else {
      KantWriteToHead(*this, KantHeadeChar, tag);
      KantWriteCharTypeBuf(*this, n, (*this)._len);
    }
  }

  void write(UInt8 n, uint8_t tag) { write((Short)n, tag); }

  void write(Short n, uint8_t tag) {
    if (n >= (-128) && n <= 127) {
      write((Char)n, tag);
    } else {
      KantWriteToHead(*this, KantHeadeShort, tag);
      n = htons(n);
      KantWriteShortTypeBuf(*this, n, (*this)._len);
    }
  }

  void write(UInt16 n, uint8_t tag) { write((Int32)n, tag); }

  void write(Int32 n, uint8_t tag) {
    if (n >= (-32768) && n <= 32767) {
      write((Short)n, tag);
    } else {
      KantWriteToHead(*this, KantHeadeInt32, tag);
      n = htonl(n);
      KantWriteInt32TypeBuf(*this, n, (*this)._len);
    }
  }

  void write(UInt32 n, uint8_t tag) { write((Int64)n, tag); }

  void write(Int64 n, uint8_t tag) {
    if (n >= (-2147483647 - 1) && n <= 2147483647) {
      write((Int32)n, tag);
    } else {
      KantWriteToHead(*this, KantHeadeInt64, tag);
      n = kant_htonll(n);
      KantWriteInt64TypeBuf(*this, n, (*this)._len);
    }
  }

  void write(Float n, uint8_t tag) {
    //DataHead h(DataHead::eFloat, tag);
    //h.writeTo(*this);
    KantWriteToHead(*this, KantHeadeFloat, tag);
    n = kant_htonf(n);
    KantWriteFloatTypeBuf(*this, n, (*this)._len);
  }

  void write(Double n, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeDouble, tag);
    n = kant_htond(n);
    KantWriteDoubleTypeBuf(*this, n, (*this)._len);
  }

  void write(const std::string& s, uint8_t tag) {
    if (kant_unlikely(s.size() > 255)) {
      if (kant_unlikely(s.size() > TARS_MAX_STRING_LENGTH)) {
        char ss[128];
        snprintf(ss, sizeof(ss), "invalid string size, tag: %d, size: %u", tag, (uint32_t)s.size());
        throw KantDecodeInvalidValue(ss);
      }
      KantWriteToHead(*this, KantHeadeString4, tag);
      uint32_t n = htonl((uint32_t)s.size());
      KantWriteUInt32TTypeBuf(*this, n, (*this)._len);

      KantWriteTypeBuf(*this, s.data(), s.size());
    } else {
      KantWriteToHead(*this, KantHeadeString1, tag);
      uint8_t n = (uint8_t)s.size();
      KantWriteUInt8TTypeBuf(*this, n, (*this)._len);

      KantWriteTypeBuf(*this, s.data(), s.size());
    }
  }

  void write(const char* buf, const UInt32 len, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeSimpleList, tag);
    KantWriteToHead(*this, KantHeadeChar, 0);
    write(len, 0);

    KantWriteTypeBuf(*this, buf, len);
  }

  template <typename K, typename V, typename Cmp, typename Alloc>
  void write(const std::map<K, V, Cmp, Alloc>& m, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeMap, tag);
    Int32 n = (Int32)m.size();
    write(n, 0);
    typedef typename std::map<K, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      write(i->first, 0);
      write(i->second, 1);
    }
  }

  template <typename K, typename V, typename H, typename Cmp, typename Alloc>
  void write(const std::unordered_map<K, V, H, Cmp, Alloc>& m, uint8_t tag) {
    {
      KantWriteToHead(*this, KantHeadeMap, tag);
      Int32 n = (Int32)m.size();
      write(n, 0);
      typedef typename std::unordered_map<K, V, H, Cmp, Alloc>::const_iterator IT;
      for (IT i = m.begin(); i != m.end(); ++i) {
        write(i->first, 0);
        write(i->second, 1);

        std::cout << "write:" << i->first << ", " << i->second << std::endl;
      }
    }
  }

  template <typename CV, typename K, typename V, typename Cmp, typename Alloc>
  void writeEx(const std::map<K, V, Cmp, Alloc>& m, uint8_t tag) {
    {
      KantWriteToHead(*this, KantHeadeMap, tag);
      Int32 n = (Int32)m.size();
      write(n, 0);
      typedef typename std::map<K, V, Cmp, Alloc>::const_iterator IT;
      for (IT i = m.begin(); i != m.end(); ++i) {
        write(i->first, 0);

        CV cv(i->second);
        write(cv, 1);
      }
    }
  }

  template <typename T, typename Alloc>
  void write(const std::vector<T, Alloc>& v, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeList, tag);
    Int32 n = (Int32)v.size();
    write(n, 0);
    typedef typename std::vector<T, Alloc>::const_iterator IT;
    for (IT i = v.begin(); i != v.end(); ++i) write(*i, 0);
  }

  template <typename T, typename Cmp, typename Alloc>
  void write(const std::set<T, Cmp, Alloc>& v, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeList, tag);
    Int32 n = (Int32)v.size();
    write(n, 0);
    typedef typename std::set<T, Cmp, Alloc>::const_iterator IT;
    for (IT i = v.begin(); i != v.end(); ++i) write(*i, 0);
  }

  template <typename T, typename H, typename Cmp, typename Alloc>
  void write(const std::unordered_set<T, H, Cmp, Alloc>& v, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeList, tag);
    Int32 n = (Int32)v.size();
    write(n, 0);
    typedef typename std::unordered_set<T, H, Cmp, Alloc>::const_iterator IT;
    for (IT i = v.begin(); i != v.end(); ++i) write(*i, 0);
  }

  template <typename CV, typename T, typename Cmp, typename Alloc>
  void writeEx(const std::set<T, Cmp, Alloc>& v, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeList, tag);
    Int32 n = (Int32)v.size();
    write(n, 0);
    typedef typename std::set<T, Cmp, Alloc>::const_iterator IT;
    for (IT i = v.begin(); i != v.end(); ++i) {
      CV cv(*i);
      write(cv, 0);
    }
  }

  template <typename CV, typename T, typename H, typename Cmp, typename Alloc>
  void writeEx(const std::unordered_set<T, H, Cmp, Alloc>& v, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeList, tag);
    Int32 n = (Int32)v.size();
    write(n, 0);
    typedef typename std::unordered_set<T, H, Cmp, Alloc>::const_iterator IT;
    for (IT i = v.begin(); i != v.end(); ++i) {
      CV cv(*i);
      write(cv, 0);
    }
  }

  template <typename CV, typename T, typename Alloc>
  void writeEx(const std::vector<T, Alloc>& v, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeList, tag);
    Int32 n = (Int32)v.size();
    write(n, 0);
    typedef typename std::vector<T, Alloc>::const_iterator IT;
    for (IT i = v.begin(); i != v.end(); ++i) {
      CV cv(*i);
      write(cv, 0);
    }
  }

  template <typename T>
  void write(const T* v, const UInt32 len, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeList, tag);
    write(len, 0);
    for (Int32 i = 0; i < (Int32)len; ++i) {
      write(v[i], 0);
    }
  }

  template <typename Alloc>
  void write(const std::vector<Char, Alloc>& v, uint8_t tag) {
    KantWriteToHead(*this, KantHeadeSimpleList, tag);
    KantWriteToHead(*this, KantHeadeChar, 0);
    Int32 n = (Int32)v.size();
    write(n, 0);

    KantWriteTypeBuf(*this, v.data(), v.size());
  }

  template <typename T>
  void write(const T& v, uint8_t tag,
             typename detail::disable_if<detail::is_convertible<T*, KantStructBase*>, void***>::type dummy = 0) {
    write((Int32)v, tag);
  }

  template <typename T>
  void write(const T& v, uint8_t tag,
             typename detail::enable_if<detail::is_convertible<T*, KantStructBase*>, void***>::type dummy = 0) {
    KantWriteToHead(*this, KantHeadeStructBegin, tag);
    v.writeTo(*this);
    KantWriteToHead(*this, KantHeadeStructEnd, 0);
  }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace kant

//支持iphone
#ifdef __APPLE__
#include "KantDisplayer.h"
#else
#include "tup/KantDisplayer.h"
#endif

#endif