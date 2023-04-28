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

#include "servant/ServantProxyFactory.h"
#include "servant/Communicator.h"
#include "servant/RemoteLogger.h"

namespace kant {

ServantProxyFactory::ServantProxyFactory(Communicator* cm) : _comm(cm) {}

ServantProxyFactory::~ServantProxyFactory() {}

ServantPrx ServantProxyFactory::initServantProxy(ServantPrx sp, const std::string& tmpObjName, bool rootServant) {
  //需要主动初始化一次
  sp->kant_initialize(rootServant);

  int syncTimeout = KT_Common::strto<int>(_comm->getProperty("sync-invoke-timeout", "3000"));
  int asyncTimeout = KT_Common::strto<int>(_comm->getProperty("async-invoke-timeout", "5000"));
  int conTimeout = KT_Common::strto<int>(_comm->getProperty("connect-timeout", "1500"));

  sp->kant_timeout(syncTimeout);
  sp->kant_async_timeout(asyncTimeout);
  sp->kant_connect_timeout(conTimeout);

  _servantProxy[tmpObjName] = sp;

  return sp;
}

///////////////////////////////////////////////////////////////////////////////
}  // namespace kant
