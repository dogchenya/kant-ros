#ifndef __AMINREAP_THREAD_H__
#define __AMINREAP_THREAD_H__

#include <iostream>
#include "util/kt_thread.h"
#include "DbProxy.h"

using namespace kant;

/**
 * 用于执行定时操作的线程类
 */
class AdminReapThread : public KT_Thread, public KT_ThreadLock {
 public:
  /*
    * 初始化函数
    */
  AdminReapThread();

  /*
    * 析构函数
    */
  ~AdminReapThread();

  /*
     * 结束线程
     */
  void terminate();

  /**
     * 初始化
     */
  int init();

  /**
     * 轮询函数
     */
  virtual void run();

 protected:
  //线程结束标志
  bool _terminate;

  //数据库操作
  //DbProxy _db;

  //心跳更新时间间隔
  int _updateInterval;

  //服务心跳超时时间
  int _timeout;

  //服务心跳包开关
  bool _heartBeatOff;
};

#endif
