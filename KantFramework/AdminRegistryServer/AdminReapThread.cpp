#include "AdminReapThread.h"

extern KT_Config* g_pconf;

AdminReapThread::AdminReapThread() : _terminate(false), _timeout(150) {}

AdminReapThread::~AdminReapThread() {
  if (isAlive()) {
    terminate();
    notifyAll();
    getThreadControl().join();
  }
}

int AdminReapThread::init() {
  TLOG_DEBUG("begin AdminReapThread init" << endl);

  //初始化配置db连接
  extern KT_Config* g_pconf;
  //_db.init(g_pconf);

  //服务心跳更新时间间隔
  _updateInterval = KT_Common::strto<int>((*g_pconf).get("/kant/reap<updateHeartInterval>", "10"));
  //最小值保护
  _updateInterval = _updateInterval < 5 ? 5 : _updateInterval;

  //管理主控心跳超时时间
  _timeout = KT_Common::strto<int>((*g_pconf)["/kant/reap<registryTimeout>"]);
  _timeout = _timeout < 5 ? 5 : _timeout;

  //是否关闭更新管理主控心跳时间,一般需要迁移时，设置此项为Y
  _heartBeatOff = (*g_pconf).get("/kant/reap<heartbeatoff>", "N") == "Y" ? true : false;

  DBPROXY->updateRegistryInfo2Db(_heartBeatOff);
  DBPROXY->loadIPPhysicalGroupInfo();

  TLOG_DEBUG("AdminReapThread init ok." << endl);

  return 0;
}

void AdminReapThread::terminate() {
  TLOG_DEBUG("[ReapThread terminate.]" << endl);
  _terminate = true;
}

void AdminReapThread::run() {
  //更新服务心跳时间
  time_t tLastUpdateTime = KT_TimeProvider::getInstance()->getNow();
  time_t tLastQueryServer = 0;
  time_t tNow;
  while (!_terminate) {
    try {
      tNow = TNOW;
      //更新心跳
      if (tNow - tLastUpdateTime >= _updateInterval) {
        tLastUpdateTime = tNow;
        DBPROXY->updateRegistryInfo2Db(_heartBeatOff);
        DBPROXY->loadIPPhysicalGroupInfo();
      }

      //轮询心跳超时的主控
      if (tNow - tLastQueryServer >= _timeout) {
        tLastQueryServer = tNow;
        DBPROXY->checkRegistryTimeout(_timeout);
      }

      KT_ThreadLock::Lock lock(*this);
      timedWait(100);  //ms
    } catch (exception& ex) {
      TLOG_ERROR("AdminReapThread exception:" << ex.what() << endl);
    } catch (...) {
      TLOG_ERROR("AdminReapThread unknown exception:" << endl);
    }
  }
}
