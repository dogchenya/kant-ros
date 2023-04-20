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

#ifndef __KANT_COMMUNICATOR_FACTORY_H_
#define __KANT_COMMUNICATOR_FACTORY_H_

#include "servant/Global.h"
#include "servant/Communicator.h"

namespace kant {
//////////////////////////////////////////////////////////////////////////////
/**
 * 创建CommunicatorPtr对象
 */
class CommunicatorFactory : public KT_Singleton<CommunicatorFactory>, public KT_ThreadRecMutex {
 public:
  /**
     * 构造函数
     * @param comm
     */
  CommunicatorFactory(){};

  /**
     * 析构
     */
  ~CommunicatorFactory(){};

  /**
     * 获取CommunicatorPtr对象
     * @param name
     * @return ServantPrx
     */
  CommunicatorPtr getCommunicator(const string& name = "default") {
    KT_LockT<KT_ThreadRecMutex> lock(*this);

    map<string, CommunicatorPtr>::iterator it = _comms.find(name);

    if (it == _comms.end()) {
      _comms[name] = std::make_shared<Communicator>();

      it = _comms.find(name);
    }
    return it->second;
  }

  /**
     * 获取CommunicatorPtr对象 
     * @param conf 
     * @param name
     * @return ServantPrx
     */
  CommunicatorPtr getCommunicator(KT_Config& conf, const string& name = "default") {
    KT_LockT<KT_ThreadRecMutex> lock(*this);

    map<string, CommunicatorPtr>::iterator it = _comms.find(name);

    if (it == _comms.end()) {
      _comms[name] = std::make_shared<Communicator>(conf);

      it = _comms.find(name);

      return it->second;
    }

    string s = "";

    it->second->setProperty(conf);

    it->second->reloadProperty(s);

    return it->second;
  }

 private:
  /**
     * 已创建的对象
     */
  map<string, CommunicatorPtr> _comms;
};
//////////////////////////////////////////////////////
}  // namespace kant

#endif
