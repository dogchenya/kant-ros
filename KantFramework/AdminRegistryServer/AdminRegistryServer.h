#include "servant/Application.h"
#include "AdminReapThread.h"

using namespace kant;

/**
 *  AdminRegistryServer Server
 */
class AdminRegistryServer : public Application {
 protected:
  /**
     * 初始化, 只会进程调用一次
     */
  virtual void initialize();

  /**
     * 析构, 每个进程都会调用一次
     */
  virtual void destroyApp();

 public:
  /**
     * 获取registry对象的端口信息
     */
  map<string, string> getServantEndpoint() { return _mapServantEndpoint; }

 private:
  int loadServantEndpoint();

 protected:
  //用于执行定时操作的线程对象
  AdminReapThread _reapThread;

  //对象-适配器 列表
  map<string, string> _mapServantEndpoint;
};

extern AdminRegistryServer g_app;
extern const string SERVER_VERSION;
