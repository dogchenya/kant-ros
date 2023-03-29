#include "util/kt_network_buffer.h"
#include "util/kt_http.h"
#include "util/kt_logger.h"
#include <cmath>

using namespace std;

namespace kant {

void KT_NetWorkBuffer::Buffer::alloc(size_t len) {
  _readIdx = 0;
  _writeIdx = 0;

  //没有分配过, 使用当前空间大小
  if (!_buffer) {
    _capacity = len;
  }

  //当前容量小了, 分配到新的容量
  if (_capacity < len) {
    _capacity = len;
    if (_buffer) {
      delete _buffer;
      _buffer = NULL;
    }
  }

  if (!_buffer) {
    _buffer = new char[_capacity];
  }
}

void KT_NetWorkBuffer::Buffer::compact() {
  //还没分配空间, 分配空间
  if (_buffer == NULL) {
    alloc(_capacity);
  }

  if (_readIdx != 0) {
    assert(_buffer);

    //当前数据+要添加的数据 < 总容量, 可以复用当前空间, 不再分配新空间
    memmove((void *)_buffer, buffer(), length());

    _writeIdx = length();
    _readIdx = 0;
  }
}

//保留当前数据, 并扩展空间, 并移除无效数据
void KT_NetWorkBuffer::Buffer::expansion(size_t len) {
  if (_capacity < len) {
    //重新分配空间, 并copy数据
    char *p = new char[len];

    if (_buffer) {
      if (length() > 0) {
        //之前有数据, copy之前数据到新的缓存
        memcpy((void *)p, buffer(), length());

        _writeIdx = length();
        _readIdx = 0;
      }
      //删除之前空间
      delete[] _buffer;
    }

    _buffer = p;
    _capacity = len;

  } else {
    //紧凑数据
    compact();
  }
}

void KT_NetWorkBuffer::Buffer::addBuffer(const char *buff, size_t len) {
  //还没分配空间, 分配空间
  if (_buffer == NULL) {
    alloc(len);
  }

  //剩余空间长度大于要添加的数据长度, 直接copy即可
  if (left() >= len) {
    memcpy((void *)free(), buff, len);
    _writeIdx += len;
  } else if (_capacity >= len + length()) {
    //当前数据+要添加的数据 < 总容量, 可以复用当前空间, 不再分配新空间
    memmove((void *)_buffer, buffer(), length());
    memcpy((void *)(_buffer + length()), buff, len);

    _writeIdx = length() + len;
    _readIdx = 0;
  } else {
    //空间容量不够, 得全新分配空间
    char *p = new char[length() + len];
    memcpy(p, buffer(), length());
    memcpy(p + length(), buff, len);
    if (_buffer) {
      delete[] _buffer;
      _buffer = NULL;
    }

    _buffer = p;
    _readIdx = 0;
    _writeIdx = length() + len;
    _capacity = _writeIdx;
  }
}

void KT_NetWorkBuffer::Buffer::replaceBuffer(const char *buff, size_t len) {
  if (_buffer) {
    delete[] _buffer;
  }

  _buffer = buff;
  _readIdx = 0;
  _writeIdx = len;
  _capacity = len;
}

void KT_NetWorkBuffer::Buffer::setBuffer(const char *buff, size_t len) {
  if (_buffer == NULL || _capacity <= len) {
    alloc(len);
  }

  _readIdx = 0;
  _writeIdx = 0;

  memcpy((void *)buffer(), buff, len);
  _writeIdx += len;
}

//////////////////////////////////////////////////////////////////////////////////////////

void KT_NetWorkBuffer::addBuffer(const shared_ptr<KT_NetWorkBuffer::Buffer> &buff) {
  if (buff->empty()) return;

  _bufferList.push_back(buff);
  _length += buff->length();
}

void KT_NetWorkBuffer::addBuffer(const vector<char> &buff) { addBuffer(buff.data(), buff.size()); }

void KT_NetWorkBuffer::addBuffer(const std::string &buff) { addBuffer(buff.c_str(), buff.size()); }

void KT_NetWorkBuffer::addBuffer(const char *buff, size_t length) {
  if (buff == NULL || length == 0) return;

  size_t capacity = length;
  if (capacity < 1024) {
    capacity = 1024;
  }
  //多预留一些空间
  auto data = getOrCreateBuffer(capacity, capacity * 1.5);

  assert(data->left() >= length);

  memcpy((void *)data->free(), buff, length);

  data->addWriteIdx(length);

  _length += length;
}

shared_ptr<KT_NetWorkBuffer::Buffer> KT_NetWorkBuffer::getOrCreateBuffer(size_t minCapacity, size_t maxCapacity) {
  assert(minCapacity <= maxCapacity);

  if (_bufferList.empty()) {
    if (!_defaultBuff) {
      _defaultBuff = std::make_shared<Buffer>();
      _defaultBuff->alloc(maxCapacity);
    } else {
      //			assert(_defaultBuff->length() == 0);
      _defaultBuff->clear();
      _defaultBuff->expansion(maxCapacity);
    }

    _bufferList.push_back(_defaultBuff);
  } else {
    auto buff = _bufferList.back();
    if (buff->left() < minCapacity) {
      //剩余空间太小了, 检查看看是否容量够, 如果够, compact一下
      if (buff->capacity() - buff->length() >= minCapacity && buff->length() * 3 < buff->capacity()) {
        buff->compact();
      } else {
        buff = std::make_shared<Buffer>();
        buff->alloc(maxCapacity);
        _bufferList.push_back(buff);
      }
    }
  }

  assert(!_bufferList.empty());

  return _bufferList.back();
}

void KT_NetWorkBuffer::addLength(size_t length) { this->_length += length; }

void KT_NetWorkBuffer::subLength(size_t length) {
  assert(this->_length >= length);

  this->_length -= length;
}

void KT_NetWorkBuffer::compute() {
  _length = 0;

  while (!_bufferList.empty()) {
    if ((*_bufferList.begin())->empty()) {
      _bufferList.erase(_bufferList.begin());
    } else {
      break;
    }
  }

  for (auto it : _bufferList) {
    _length += it->length();
  }
}

shared_ptr<KT_NetWorkBuffer::Buffer> KT_NetWorkBuffer::getBuffer() {
  if (_bufferList.empty()) {
    return NULL;
  }
  return *_bufferList.begin();
}

KT_NetWorkBuffer::buffer_iterator KT_NetWorkBuffer::begin() const { return buffer_iterator(this, 0); }

KT_NetWorkBuffer::buffer_iterator KT_NetWorkBuffer::end() const {
  return buffer_iterator(this, this->getBufferLength());
}

KT_NetWorkBuffer::buffer_iterator KT_NetWorkBuffer::find(const char *str, size_t length) {
  return std::search(begin(), end(), str, str + length);
}

void KT_NetWorkBuffer::clearBuffers() {
  _bufferList.clear();
  _length = 0;
}

bool KT_NetWorkBuffer::empty() const {
  //	if(_length == 0 && !_bufferList.empty())
  //	{
  //		assert(_bufferList.empty());
  //	}
  return _length == 0;
}

size_t KT_NetWorkBuffer::getBufferLength() const { return _length; }

pair<const char *, size_t> KT_NetWorkBuffer::getBufferPointer() const {
  if (empty()) {
    return make_pair((const char *)NULL, 0);
  }

  auto it = _bufferList.begin();

  return make_pair((*it)->buffer(), (*it)->length());
}

const char *KT_NetWorkBuffer::mergeBuffers() {
  //merge to one buffer
  if (_bufferList.size() > 1) {
    std::shared_ptr<Buffer> buff = std::make_shared<Buffer>();

    getBuffers(buff);

    std::list<std::shared_ptr<Buffer>> bufferList;

    bufferList.push_back(buff);

    _bufferList.swap(bufferList);
  }

  assert(_bufferList.size() <= 1);

  if (!_bufferList.empty()) {
    return (*_bufferList.begin())->buffer();
  }

  return NULL;
}

size_t KT_NetWorkBuffer::getBuffers(char *buffer, size_t length) const {
  assert(length <= getBufferLength());

  auto it = _bufferList.begin();

  size_t left = length;
  size_t pos = 0;

  while (it != _bufferList.end() && left != 0) {
    size_t len = min(left, (*it)->length());

    memcpy(buffer + pos, (*it)->buffer(), len);

    left -= len;
    pos += len;

    ++it;
  }

  return pos;
}

string KT_NetWorkBuffer::getBuffersString() const {
  string buffer;
  buffer.resize(_length);

  getBuffers(&buffer[0], _length);

  return buffer;
}

vector<char> KT_NetWorkBuffer::getBuffers() const {
  vector<char> buffer;

  buffer.resize(_length);

  getBuffers(&buffer[0], _length);

  return buffer;
}

void KT_NetWorkBuffer::getBuffers(shared_ptr<Buffer> &buff) const {
  buff->alloc(_length);

  getBuffers(buff->buffer(), _length);

  buff->addWriteIdx(_length);
}

bool KT_NetWorkBuffer::getHeader(size_t len, std::string &buffer) const {
  if (getBufferLength() < len) return false;

  buffer.clear();

  if (len == 0) {
    return true;
  }

  buffer.resize(len);

  getBuffers(&buffer[0], len);

  return true;
}

bool KT_NetWorkBuffer::getHeader(size_t len, std::vector<char> &buffer) const {
  if (getBufferLength() < len) return false;

  buffer.clear();

  if (len == 0) {
    return true;
  }

  buffer.resize(len);

  getBuffers(&buffer[0], len);

  return true;
}

bool KT_NetWorkBuffer::moveHeader(size_t len) {
  if (getBufferLength() < len) return false;

  if (len == 0) return true;

  auto it = _bufferList.begin();

  //    assert(it->size() >= _pos);

  size_t left = (*it)->length();

  if (left > len) {
    (*it)->addReadIdx(len);
    _length -= len;
  } else if (left == len) {
    (*it)->addReadIdx(len);
    _length -= len;
    _bufferList.erase(it);
  } else {
    (*it)->addReadIdx(left);
    _length -= left;

    _bufferList.erase(it);

    return moveHeader(len - left);
  }
  return true;
}

uint8_t KT_NetWorkBuffer::getValueOf1() const { return getValue<uint8_t>(); }

uint16_t KT_NetWorkBuffer::getValueOf2() const { return getValue<uint16_t>(); }

uint32_t KT_NetWorkBuffer::getValueOf4() const { return getValue<uint32_t>(); }

KT_NetWorkBuffer::PACKET_TYPE KT_NetWorkBuffer::parseBufferOf1(vector<char> &buffer, uint8_t minLength,
                                                               uint8_t maxLength) {
  return parseBuffer<uint8_t>(buffer, minLength, maxLength);
}

KT_NetWorkBuffer::PACKET_TYPE KT_NetWorkBuffer::parseBufferOf2(vector<char> &buffer, uint16_t minLength,
                                                               uint16_t maxLength) {
  return parseBuffer<uint16_t>(buffer, minLength, maxLength);
}

KT_NetWorkBuffer::PACKET_TYPE KT_NetWorkBuffer::parseBufferOf4(vector<char> &buffer, uint32_t minLength,
                                                               uint32_t maxLength) {
  return parseBuffer<uint32_t>(buffer, minLength, maxLength);
}

KT_NetWorkBuffer::PACKET_TYPE KT_NetWorkBuffer::checkHttp() {
  try {
    bool b = KT_HttpRequest::checkRequest(*this);

    return b ? PACKET_FULL : PACKET_LESS;
  } catch (exception &ex) {
    return PACKET_ERR;
  }

  return PACKET_LESS;
}

KT_NetWorkBuffer::PACKET_TYPE KT_NetWorkBuffer::parseHttp(KT_NetWorkBuffer &in, vector<char> &out) {
  //	LOG_CONSOLE_DEBUG << in.mergeBuffers() << endl;

  KT_NetWorkBuffer::PACKET_TYPE b = in.checkHttp();

  if (b == PACKET_FULL) {
    out = in.getBuffers();

    in.clearBuffers();
  }

  return b;
}

KT_NetWorkBuffer::PACKET_TYPE KT_NetWorkBuffer::parseEcho(KT_NetWorkBuffer &in, vector<char> &out) {
  try {
    if (in.empty()) {
      return KT_NetWorkBuffer::PACKET_LESS;
    }
    out = in.getBuffers();
    in.clearBuffers();
    return KT_NetWorkBuffer::PACKET_FULL;
  } catch (exception &ex) {
    return KT_NetWorkBuffer::PACKET_ERR;
  }

  return KT_NetWorkBuffer::PACKET_LESS;  //表示收到的包不完全
}

}  // namespace kant
