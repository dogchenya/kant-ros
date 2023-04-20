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

#include "util/kt_shm.h"
#include "util/kt_common.h"
#include <cassert>
#include <errno.h>

namespace kant {

KT_Shm::KT_Shm(size_t iShmSize, key_t iKey, bool bOwner) : _pshm(NULL) { init(iShmSize, iKey, bOwner); }

KT_Shm::~KT_Shm() {
  if (_bOwner) {
    detach();
  }
}

void KT_Shm::init(size_t iShmSize, key_t iKey, bool bOwner) {
  assert(_pshm == NULL);

#if TARGET_PLATFORM_WINDOWS
  _bOwner = bOwner;

  // 首先试图打开一个命名的内存映射文件对象
  HANDLE hMap = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS, 0, KT_Common::tostr(iKey).c_str());
  if (NULL == hMap) {
    // 打开失败，创建之
    hMap = ::CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)iShmSize,
                                KT_Common::tostr(iKey).c_str());
    // 映射对象的一个视图，得到指向共享内存的指针，设置里面的数据
    _pshm = ::MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  } else {  // 打开成功，映射对象的一个视图，得到指向共享内存的指针，显示出里面的数据
    _pshm = ::MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  }

  _shemID = hMap;
  _shmKey = iKey;
#else
  _bOwner = bOwner;

  //注意_bCreate的赋值位置:保证多线程用一个对象的时候也不会有问题
  //试图创建
  if ((_shemID = shmget(iKey, iShmSize, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
    _bCreate = false;
    //有可能是已经存在同样的key_shm,则试图连接
    if ((_shemID = shmget(iKey, iShmSize, 0666)) < 0) {
      THROW_EXCEPTION_SYSCODE(KT_Shm_Exception, "[KT_Shm::init()] shmget error");
      // throw KT_Shm_Exception("[KT_Shm::init()] shmget error", KT_Exception::getSystemCode());
    }
  } else {
    _bCreate = true;
  }

  //try to access shm
  if ((_pshm = shmat(_shemID, NULL, 0)) == (char *)-1) {
    THROW_EXCEPTION_SYSCODE(KT_Shm_Exception, "[KT_Shm::init()] shmat error");
    // throw KT_Shm_Exception("[KT_Shm::init()] shmat error", KT_Exception::getSystemCode());
  }

  _shmSize = iShmSize;
  _shmKey = iKey;
#endif
}

int KT_Shm::detach() {
#if TARGET_PLATFORM_WINDOWS
  if (_pshm != NULL) {
    UnmapViewOfFile(_pshm);
    CloseHandle(_shemID);
    _pshm = NULL;
  }
  return 0;
#else
  int iRetCode = 0;
  if (_pshm != NULL) {
    iRetCode = shmdt(_pshm);

    _pshm = NULL;
  }

  return iRetCode;

#endif
}

int KT_Shm::del() {
#if TARGET_PLATFORM_WINDOWS
  return 0;
#else
  int iRetCode = 0;
  if (_pshm != NULL) {
    iRetCode = shmctl(_shemID, IPC_RMID, 0);

    _pshm = NULL;
  }

  return iRetCode;
#endif
}

}  // namespace kant
