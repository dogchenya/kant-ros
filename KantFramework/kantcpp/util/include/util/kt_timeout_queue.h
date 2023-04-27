﻿#ifndef _KT_TIMEOUT_QUEUE_H_
#define _KT_TIMEOUT_QUEUE_H_

#include <deque>
#include <iostream>
#include <list>
#include <cassert>
#include <unordered_map>
#include <chrono>
//#include "util/kt_autoptr.h"
#include <mutex>
#include <functional>

using namespace std;

namespace kant {
/////////////////////////////////////////////////
/**
 * @file kt_timeout_queue.h
 * @brief 超时队列(模板元素只能是智能指针, 且继承KT_HandleBase).
 *
 * @author
 */
/////////////////////////////////////////////////

template <class T>
class KT_TimeoutQueue {
 public:
  struct PtrInfo;

  struct NodeInfo;

  // typedef map<uint32_t, PtrInfo> data_type;
  typedef unordered_map<uint32_t, PtrInfo> data_type;

  typedef list<NodeInfo> time_type;

  //    typedef KT_Functor<void, TL::TYPELIST_1(T&)> data_functor;
  typedef std::function<void(T &)> data_functor;

  struct PtrInfo {
    T ptr;

    typename time_type::iterator timeIter;
  };

  struct NodeInfo {
    bool hasPoped;

    int64_t createTime;

    typename data_type::iterator dataIter;
  };

  /**
     * @brief 超时队列，缺省5s超时.
     *
     * @param timeout 超时设定时间
     * @param size
     */
  KT_TimeoutQueue(int timeout = 5 * 1000) : _uniqId(0), _timeout(timeout) {
    _firstNoPopIter = _time.end();
    //        _data.resize(size);
  }

  /**
     * @brief  产生该队列的下一个ID
     */
  uint32_t generateId();

  /**
     * @brief 获取指定id的数据.
     *
     * @param id 指定的数据的id
     * @return T 指定id的数据
     */
  T get(uint32_t uniqId, bool bErase = true);

  /**
     * @brief, 获取数据并更新时间链, 从而能够不超时
     * @param  uniqId
     * @return T 指定id的数据
     */
  T getAndRefresh(uint32_t uniqId);

  /**
     * @brief 删除.
     *
     * @param uniqId 要删除的数据的id
     * @return T     被删除的数据
     */
  T erase(uint32_t uniqId);

  /**
     * @brief 设置消息到队列尾端.
     *
     * @param ptr        要插入到队列尾端的消息
     * @return uint32_t id号
     */
  bool push(const T &ptr, uint32_t uniqId);

  /**
     * @brief 超时删除数据
     */
  void timeout();

  /**
     * @brief 删除超时的数据，并用df对数据做处理
     */
  void timeout(data_functor &df);

  /**
     * @brief 取出队列头部的消息.
     *
     * @return T 队列头部的消息
     */
  T pop();

  /**
     * @brief 取出队列头部的消息(减少一次copy).
     *
     * @param t
     */
  bool pop(T &t);

  /**
     * @brief 交换数据.
     *
     * @param q
     * @return bool
     */
  bool swap(deque<T> &q);

  /**
     * @brief 设置超时时间(毫秒).
     *
     * @param timeout
     */
  void setTimeout(time_t timeout) { _timeout = timeout; }

  /**
     * @brief 获取超时时间
     * @return [description]
     */
  time_t getTimeout() const { return _timeout; }

  /**
     * @brief 队列中的数据.
     *
     * @return size_t
     */
  size_t size() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _data.size();
  }

  /**
     * @brief is empty
     * @return
     */
  bool empty() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _data.empty();
  }

 protected:
  uint32_t _uniqId;
  time_t _timeout;
  data_type _data;
  time_type _time;
  typename time_type::iterator _firstNoPopIter;
  mutable std::mutex _mutex;
};

template <typename T>
T KT_TimeoutQueue<T>::get(uint32_t uniqId, bool bErase) {
  std::lock_guard<std::mutex> lock(_mutex);

  typename data_type::iterator it = _data.find(uniqId);

  if (it == _data.end()) {
    return NULL;
  }

  T ptr = it->second.ptr;

  if (bErase) {
    if (_firstNoPopIter == it->second.timeIter) {
      ++_firstNoPopIter;
    }
    _time.erase(it->second.timeIter);
    _data.erase(it);
  }

  return ptr;
}

template <typename T>
T KT_TimeoutQueue<T>::getAndRefresh(uint32_t uniqId) {
  std::lock_guard<std::mutex> lock(_mutex);

  typename data_type::iterator it = _data.find(uniqId);

  if (it == _data.end()) {
    return NULL;
  }

  T ptr = it->second.ptr;

  //从时间队列中删除
  if (_firstNoPopIter == it->second.timeIter) {
    ++_firstNoPopIter;
  }
  _time.erase(it->second.timeIter);

  //再插入到时间队列末尾
  NodeInfo ni;

  ni.createTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

  ni.dataIter = it;

  ni.hasPoped = false;

  _time.push_back(ni);

  typename time_type::iterator tmp = _time.end();

  --tmp;

  it->second.timeIter = tmp;

  if (_firstNoPopIter == _time.end()) {
    _firstNoPopIter = tmp;
  }

  return ptr;
}
template <typename T>
uint32_t KT_TimeoutQueue<T>::generateId() {
  std::lock_guard<std::mutex> lock(_mutex);

  while (++_uniqId == 0)
    ;

  return _uniqId;
}

template <typename T>
bool KT_TimeoutQueue<T>::push(const T &ptr, uint32_t uniqId) {
  std::lock_guard<std::mutex> lock(_mutex);

  PtrInfo pi;

  pi.ptr = ptr;

  pair<typename data_type::iterator, bool> result;

  result = _data.insert(make_pair(uniqId, pi));

  if (result.second == false) return false;

  typename data_type::iterator it = result.first;

  NodeInfo ni;

  ni.createTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

  ni.dataIter = it;

  ni.hasPoped = false;

  _time.push_back(ni);

  typename time_type::iterator tmp = _time.end();

  --tmp;

  it->second.timeIter = tmp;

  if (_firstNoPopIter == _time.end()) {
    _firstNoPopIter = tmp;
  }

  return true;
}

template <typename T>
void KT_TimeoutQueue<T>::timeout() {
  //    struct timeval tv;
  //    KT_TimeProvider::getInstance()->getNow(&tv);

  auto ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

  while (true) {
    std::lock_guard<std::mutex> lock(_mutex);

    typename time_type::iterator it = _time.begin();

    if (it != _time.end() && ms - it->createTime > _timeout) {
      _data.erase(it->dataIter);

      if (_firstNoPopIter == it) {
        ++_firstNoPopIter;
      }
      _time.erase(it);
    } else {
      break;
    }
  }
}

template <typename T>
void KT_TimeoutQueue<T>::timeout(data_functor &df) {
  //    struct timeval tv;
  //    KT_TimeProvider::getInstance()->getNow(&tv);
  //
  auto ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

  while (true) {
    T ptr;

    {
      std::lock_guard<std::mutex> lock(_mutex);

      typename time_type::iterator it = _time.begin();

      if (it != _time.end() && ms - it->createTime > _timeout) {
        ptr = (*it->dataIter).second.ptr;

        _data.erase(it->dataIter);

        if (_firstNoPopIter == it) {
          _firstNoPopIter++;
        }
        _time.erase(it);
      } else {
        break;
      }
    }

    try {
      df(ptr);
    } catch (...) {
    }
  }
}

template <typename T>
T KT_TimeoutQueue<T>::erase(uint32_t uniqId) {
  std::lock_guard<std::mutex> lock(_mutex);

  typename data_type::iterator it = _data.find(uniqId);

  if (it == _data.end()) {
    return NULL;
  }

  T ptr = it->second.ptr;

  if (_firstNoPopIter == it->second.timeIter) {
    _firstNoPopIter++;
  }
  _time.erase(it->second.timeIter);

  _data.erase(it);

  return ptr;
}

template <typename T>
T KT_TimeoutQueue<T>::pop() {
  T ptr;

  return pop(ptr) ? ptr : NULL;
}

template <typename T>
bool KT_TimeoutQueue<T>::pop(T &ptr) {
  std::lock_guard<std::mutex> lock(_mutex);

  if (_time.empty()) {
    return false;
  }

  typename time_type::iterator it = _time.begin();

  if (it->hasPoped == true) {
    it = _firstNoPopIter;
  }

  if (it == _time.end()) {
    return false;
  }

  assert(it->hasPoped == false);

  ptr = it->dataIter->second.ptr;

  it->hasPoped = true;

  _firstNoPopIter = it;

  ++_firstNoPopIter;

  return true;
}

template <typename T>
bool KT_TimeoutQueue<T>::swap(deque<T> &q) {
  std::lock_guard<std::mutex> lock(_mutex);

  if (_time.empty()) {
    return false;
  }

  typename time_type::iterator it = _time.begin();

  while (it != _time.end()) {
    if (it->hasPoped == true) {
      it = _firstNoPopIter;
    }

    if (it == _time.end()) {
      break;
    }

    assert(it->hasPoped == false);

    T ptr = it->dataIter->second.ptr;

    it->hasPoped = true;

    _firstNoPopIter = it;

    ++_firstNoPopIter;

    q.push_back(ptr);

    ++it;
  }

  if (q.empty()) {
    return false;
  }

  return true;
}
/////////////////////////////////////////////////////////////////
}  // namespace kant
#endif