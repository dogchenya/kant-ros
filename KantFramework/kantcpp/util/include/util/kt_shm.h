﻿/**
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

#ifndef __KT_SHM_H__
#define __KT_SHM_H__

#include "util/kt_platform.h"
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#endif
#include "util/kt_ex.h"

namespace kant {
/////////////////////////////////////////////////
/** 
 * @file  kt_shm.h 
 * @brief  共享内存封装类. 
 *  
 * @author  jarodruan@tencent.com 
 */
/////////////////////////////////////////////////

/**
* @brief 共享内存异常类.
*/
struct KT_Shm_Exception : public KT_Exception {
  KT_Shm_Exception(const string &buffer, int err) : KT_Exception(buffer, err){};
  ~KT_Shm_Exception() throw(){};
};

#if TARGET_PLATFORM_WINDOWS
typedef int key_t;
typedef HANDLE SHMID;
#else
typedef int SHMID;
#endif

/** 
* @brief  共享内存连接类，说明: 
* 1 用于连接共享内存, 共享内存的权限是 0666 
* 2 _bOwner=false: 析够时不detach共享内存 
* 3 _bOwner=true: 析够时detach共享内存
*/
class KT_Shm {
 public:
  /**
	* @brief 构造函数.
	*  
	* @param bOwner  是否拥有共享内存,默认为false 
    */
  KT_Shm(bool bOwner = false) : _bOwner(bOwner), _shmSize(0), _shmKey(0), _bCreate(true), _pshm(NULL) {}

  /**
	* @brief 构造函数. 
	*  
    * @param iShmSize 共享内存大小
    * @param iKey     共享内存Key
    * @throws         KT_Shm_Exception
    */
  KT_Shm(size_t iShmSize, key_t iKey, bool bOwner = false);

  /**
    * @brief 析构函数.
    */
  ~KT_Shm();

  /**
	* @brief 初始化. 
	*  
    * @param iShmSize   共享内存大小
    * @param iKey       共享内存Key
    * @param bOwner     是否拥有共享内存
    * @throws           KT_Shm_Exception
    * @return Ξ
    */
  void init(size_t iShmSize, key_t iKey, bool bOwner = false);

  /** 
	* @brief 判断共享内存的类型，生成的共享内存,还是连接上的共享内存
	* 如果是生成的共享内存,此时可以根据需要做初始化 
	*  
    * @return  true,生成共享内存; false, 连接上的共享内存
    */
  bool iscreate() { return _bCreate; }

  /**
	* @brief  获取共享内存的指针.
	*  
    * @return   void* 共享内存指针
    */
  void *getPointer() { return _pshm; }

  /**
	* @brief  获取共享内存Key.
	*  
    * @return key_t* ,共享内存key
    */
  key_t getkey() { return _shmKey; }

  /**
	* @brief  获取共享内存ID.
	* 
    * @return int ,共享内存Id
    */
  SHMID getid() { return _shemID; }

  /**
	*  @brief  获取共享内存大小.
	*  
    * return size_t,共享内存大小
    */
  size_t size() { return _shmSize; }

  /** 
	*  @brief 解除共享内存，在当前进程中解除共享内存
    * 共享内存在当前进程中无效
    * @return int
    */
  int detach();

  /** 
	 *  @brief 删除共享内存.
	 * 
     * 完全删除共享内存
     */
  int del();

 protected:
  /**
     * 是否拥有共享内存
     */
  bool _bOwner;

  /**
    * 共享内存大小
    */
  size_t _shmSize;

  /**
    * 共享内存key
    */
  key_t _shmKey;

  /**
    * 是否是生成的共享内存
    */
  bool _bCreate;

  /**
    * 共享内存
    */
  void *_pshm;

  /**
    * 共享内存id
    */
  SHMID _shemID;
};

}  // namespace kant

#endif