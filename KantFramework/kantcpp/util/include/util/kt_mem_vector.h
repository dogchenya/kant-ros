/**
 * Tencent is pleased to support the open source community by making Tars available.
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

#ifndef __KT_MEM_VECTOR_H__
#define __KT_MEM_VECTOR_H__

#include "util/kt_ex.h"
#include <sstream>
#include <string.h>

namespace kant {
/////////////////////////////////////////////////
/** 
* @file kt_mem_vector.h 
* @brief  共享内存数组类. 
* @brief  Shared Memory Array Class
* 
*/
/////////////////////////////////////////////////

/**
* @brief 异常类
* @brief Exception Class
*/
struct KT_MemVectorException : public KT_Exception {
  KT_MemVectorException(const string &buffer) : KT_Exception(buffer){};
  ~KT_MemVectorException() throw(){};
};

/**
* @brief 共享内存随机访问数据块.
* @brief Shared Memory Random Access Data Block.
*  
* 每块数据快大小相等，模板对象T只能是简单的数据类型 ，
* Template object T can only be a simple data type.
*  
* 需要具备bit-copy的语义才行 
* Require bit-copy semantics
*/
template <typename T>
class KT_MemVector {
 public:
  /**
    * @brief 构造函数
    * @brief Constructor
    */
  KT_MemVector() : _pHead(NULL), _pData(NULL) {}

  /**
     * @brief 
     * @param mv
     */
  KT_MemVector(const KT_MemVector<T> &mv) : _pHead(mv._pHead), _pData(mv._pData) {}

  /**
     * @brief 
     * @param mv
     *
     * @return bool
     */
  bool operator==(const KT_MemVector<T> &mv) { return _pHead == mv._pHead && _pData == mv._pData; }

  /**
     * @brief 
     * @param mv
     *
     * @return bool
     */
  bool operator!=(const KT_MemVector<T> &mv) { return _pHead != mv._pHead || _pData != mv._pData; }

  /**
     * @brief 计算需要的内存空间
     * @brief Calculate required memory space
	 * @param iCount  数据个数 
     * @param iCount  number of data
     * @return size_t  内存空间大小
     * @return size_t  Memory space size
     */
  static size_t calcMemSize(size_t iCount) { return sizeof(T) * iCount + sizeof(tagMemQueueHead); }

  /**
    * @brief 初始化
    * @brief Initialization
    * @param pAddr       指令队列空间的指针
    * @param pAddr       Pointer to instruction queue space
    * @param iSize       空间的指针
    * @param iSize       Pointer to space
    * @param iBlockSize 每个block的大小
    * @param iBlockSize Size of each block
    */
  void create(void *pAddr, size_t iSize);

  /**
    * @brief 连接上队列
    * @brief Connect Queue
    * @param pAddr 指令队列空间的指针
    * @param pAddr Pointer to instruction queue space
    */
  void connect(void *pAddr) {
    init(pAddr);
    assert(_pHead->_iBlockSize == sizeof(T));
  }

  /**
    * @brief 元素个数
    * @brief Number of elements
    * @return size_t，队列长度
    * @return size_t，queue length
    */
  size_t size() { return _pHead->_iBlockCount; }

  /**
    * @brief 共享内存长度
    * @brief Shared memory length
    * @return size_t : 共享内存长度
    * @return size_t : Shared memory length
    */
  size_t getMemSize() { return _pHead->_iSize; }

  /**
     * @brief 重建
     * @brief Rebuild
     */
  void clear();

  /**
     * @brief 
     * @param simple
     *
     * @return string
     */
  string desc() const;

  /**
     * @brief 迭代器
     * @brief iterator
     */
  class KT_MemVectorIterator : public std::iterator<std::random_access_iterator_tag, T> {
   public:
    /**
         * @brief 构造函数
         * @brief Constructor
         * @param pmv
         * @param iIndex
         */
    KT_MemVectorIterator(KT_MemVector *pmv, size_t iIndex) : _pmv(pmv), _iIndex(iIndex) {}

    /**
         * @brief 前置++
         * @brief Pre++
         *
         * @return KT_MemVectorIterator&
         */
    KT_MemVectorIterator &operator++() {
      ++_iIndex;
      return *this;
    }

    /**
         * @brief 后置++
         * @brief Post++
         */
    KT_MemVectorIterator operator++(int) {
      KT_MemVectorIterator tmp = *this;

      ++_iIndex;
      return tmp;
    }

    /**
         * @brief 
         * @param mv
         *
         * @return KT_MemVectorIterator
         */
    bool operator==(const KT_MemVectorIterator &mv) { return _iIndex == mv._iIndex && _pmv == mv._pmv; }

    /**
         * @brief 
         * @param mv
         *
         * @return KT_MemVectorIterator
         */
    bool operator!=(const KT_MemVectorIterator &mv) { return _iIndex != mv._iIndex || _pmv != mv._pmv; }

    /**
         * @brief
         *
         * @return T&
         */
    T &operator*() const { return (*_pmv)[_iIndex]; }

    /**
         * @brief
         *
         * @return T*
         */
    T *operator->() const { return &(*_pmv)[_iIndex]; }

   private:
    /**
         *
         */
    KT_MemVector *_pmv;

    /**
         *
         */
    size_t _iIndex;
  };

  typedef KT_MemVectorIterator iterator;

  /**
     *
     *
     * @return KT_MemVectorIterator
     */
  KT_MemVectorIterator begin() { return KT_MemVectorIterator(this, 0); }

  /**
     *
     *
     * @return KT_MemVectorIterator
     */
  KT_MemVectorIterator end() { return KT_MemVectorIterator(this, _pHead->_iBlockCount); }

  /**
     * @brief 获取数据
     * @brief Get data
     * @param pData
     * @param iDataLen
     */
  T &operator[](size_t iIndex) {
    if (iIndex >= _pHead->_iBlockCount) {
      ostringstream s;
      s << string("[KT_MemVector::get] index beyond : index = ") << iIndex << " > " << _pHead->_iBlockCount;

      throw KT_MemVectorException(s.str());
    }

    return *(T *)((char *)_pData + iIndex * _pHead->_iBlockSize);
  }

  /**
     * @brief 获取头地址
     * @brief Get header address
     *
     * @return void*
     */
  void *getAddr() { return (void *)_pHead; }

  /**
    *  @brief 队列控制结构
    *  @brief Queue Control Structure
    */
#pragma pack(1)
  struct tagMemQueueHead {
    /*Memory Size*/
    size_t _iSize;  //内存大小
    /*Number of elements*/
    size_t _iBlockCount;  //元素个数
    /*Block Size*/
    size_t _iBlockSize;  //区块大小
  };
#pragma pack()

 protected:
  /**
     * @brief
     * @param pAddr
     */
  void init(void *pAddr) {
    _pHead = static_cast<tagMemQueueHead *>(pAddr);
    _pData = (char *)_pHead + sizeof(tagMemQueueHead);
  }

  /**
    * 队列控制快
    * Quick Queue Control
    */
  tagMemQueueHead *_pHead;

  /**
    * 共享内存地址
    * Shared memory address
    */
  void *_pData;
};

template <typename T>
void KT_MemVector<T>::create(void *pAddr, size_t iSize) {
  size_t iBlockSize = sizeof(T);

  if (iSize <= sizeof(tagMemQueueHead) || ((iSize - sizeof(tagMemQueueHead)) / iBlockSize == 0)) {
    throw KT_MemVectorException("[KT_MemVector::create] memory size not enough.");
  }

  init(pAddr);

  memset(pAddr, 0x00, iSize);

  _pHead->_iSize = iSize;
  _pHead->_iBlockCount = (iSize - sizeof(tagMemQueueHead)) / iBlockSize;
  _pHead->_iBlockSize = iBlockSize;
}

template <typename T>
void KT_MemVector<T>::clear() {
  assert(_pHead);

  memset(_pData, 0x00, _pHead->_iBlockSize * _pHead->_iBlockCount);
}

template <typename T>
string KT_MemVector<T>::desc() const {
  ostringstream s;
  s << "[KT_MemVector] [_iSize=" << _pHead->_iSize << "] "
    << "[_iBlockCount=" << _pHead->_iBlockCount << "] "
    << "[_iBlockSize=" << _pHead->_iBlockSize << "] " << endl;
  s << "[~KT_MemVector]";

  return s.str();
}

}  // namespace kant

#endif
