#include <sstream>
#include "util/kt_option.h"
#include "util/kt_common.h"
#include "servant/KeepAliveNodeF.h"
#include "servant/Application.h"
#include "servant/AppProtocol.h"
#include "servant/AdminServant.h"
#include "servant/ServantHandle.h"
#include "servant/BaseF.h"
#include "servant/AppCache.h"
#include "servant/NotifyObserver.h"
#include "servant/AuthLogic.h"
#include "servant/CommunicatorFactory.h"

#include <signal.h>
#if TARGET_PLATFORM_LINUX
#include <sys/resource.h>
#endif

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
#include <fcntl.h>
#endif

#if KANT_SSL
#include "util/tc_openssl.h"
#endif

static KT_RollLogger __out__;

#define NOTIFY_AND_WAIT(msg)                                    \
  {                                                             \
    RemoteNotify::getInstance()->report(msg);                   \
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); \
  }

namespace kant {

std::string ServerConfig::KantPath;     //服务路径
std::string ServerConfig::Application;  //应用名称
std::string ServerConfig::ServerName;   //服务名称,一个服务名称含一个或多个服务标识
std::string ServerConfig::LocalIp;      //本机IP
std::string ServerConfig::BasePath;     //应用程序路径，用于保存远程系统配置的本地目录
std::string ServerConfig::DataPath;     //应用程序路径，用于本地数据
std::string ServerConfig::Local;        //本地套接字
std::string ServerConfig::Node;         //本机node地址
std::string ServerConfig::Log;          //日志中心地址
std::string ServerConfig::Config;       //配置中心地址
std::string ServerConfig::Notify;       //信息通知中心
std::string ServerConfig::LogPath;      //logpath
int ServerConfig::LogSize;              //log大小(字节)
int ServerConfig::LogNum;               //log个数()
std::string ServerConfig::LogLevel;     //log日志级别
std::string ServerConfig::ConfigFile;   //框架配置文件路径
int ServerConfig::ReportFlow = 1;  //是否服务端上报所有接口stat流量 0不上报 1上报 (用于非kant协议服务流量统计)
int ServerConfig::IsCheckSet = 1;           //是否对按照set规则调用进行合法性检查 0,不检查，1检查
int ServerConfig::OpenCoroutine = 0;        //是否启用协程处理方式
size_t ServerConfig::CoroutineMemSize;      //协程占用内存空间的最大大小
uint32_t ServerConfig::CoroutineStackSize;  //每个协程的栈大小(默认128k)
bool ServerConfig::ManualListen = false;    //手工启动监听端口
//bool        ServerConfig::MergeNetImp = false;     //合并网络和处理线程
int ServerConfig::NetThread = 1;  //servernet thread
bool ServerConfig::CloseCout = true;
int ServerConfig::BackPacketLimit = 0;
int ServerConfig::BackPacketMin = 1024;
//int         ServerConfig::Pattern = 0;

#if KANT_SSL
std::string ServerConfig::CA;
std::string ServerConfig::Cert;
std::string ServerConfig::Key;
bool ServerConfig::VerifyClient = false;
std::string ServerConfig::Ciphers;
#endif

map<string, string> ServerConfig::Context;

///////////////////////////////////////////////////////////////////////////////////////////
//KT_Config                       Application::_conf;
//KT_EpollServerPtr               Application::_epollServer  = NULL;
CommunicatorPtr Application::_communicator = NULL;

PropertyReportPtr g_pReportRspQueue;

/**上报服务端发送队列大小的间隔时间**/
#define REPORT_SEND_QUEUE_INTERVAL 10

///////////////////////////////////////////////////////////////////////////////////////////
Application::Application() {
  _servantHelper = std::make_shared<ServantHelperManager>();

  _notifyObserver = std::make_shared<NotifyObserver>();

  setNotifyObserver(_notifyObserver);

#if TARGET_PLATFORM_WINDOWS
  WSADATA wsadata;
  WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif
}

Application::~Application() {
  if (_epollServer) {
    _epollServer->terminate();
    _epollServer = NULL;
  }
#if TARGET_PLATFORM_WINDOWS
  WSACleanup();
#endif
}

string Application::getKantVersion() { return "KANT_VERSION"; }

CommunicatorPtr& Application::getCommunicator() { return _communicator; }

void reportRspQueue(KT_EpollServer* epollServer) {
  if (!g_pReportRspQueue) return;

  static time_t iLastCheckTime = TNOW;

  time_t iNow = TNOW;

  if (iNow - iLastCheckTime > REPORT_SEND_QUEUE_INTERVAL) {
    iLastCheckTime = iNow;

    const vector<KT_EpollServer::BindAdapterPtr>& adapters = epollServer->getBindAdapters();

    size_t n = 0;
    for (size_t i = 0; i < adapters.size(); ++i) {
      n = n + adapters[i]->getSendBufferSize();
    }

    g_pReportRspQueue->report((int)n);
  }
}

void heartBeatFunc(const string& adapterName) { KANT_KEEPALIVE(adapterName); }

void Application::manualListen() {
  vector<KT_EpollServer::BindAdapterPtr> v = getEpollServer()->getBindAdapters();

  for (auto& b : v) {
    b->manualListen();
  }
}

void Application::waitForShutdown() {
  assert(_epollServer);

  _epollServer->setCallbackFunctor(reportRspQueue);

  _epollServer->setHeartBeatFunctor(heartBeatFunc);

  _epollServer->setDestroyAppFunctor([&](KT_EpollServer* epollServer) {
    this->destroyApp();

    NOTIFY_AND_WAIT("stop");
  });

  _epollServer->waitForShutdown();

  KT_Port::unregisterCtrlC(_ctrlCId);
  KT_Port::unregisterTerm(_termId);

  _epollServer = NULL;
}

void Application::waitForReady() {
  if (_epollServer) {
    _epollServer->waitForReady();
  }
}

void Application::terminate() {
  if (_epollServer && !_epollServer->isTerminate()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  //稍微休息一下, 让当前处理包能够回复

    _epollServer->terminate();
  }
}

bool Application::cmdViewStatus(const string& command, const string& params, string& result) {
  TLOGKANT("Application::cmdViewStatus:" << command << " " << params << endl);

  ostringstream os;

  os << OUT_LINE_LONG << endl;

  os << KT_Common::outfill("[proxy config]:") << endl;

  outClient(os);

  os << OUT_LINE << "\n" << KT_Common::outfill("[server config]:") << endl;

  outServer(os);

  os << OUT_LINE << endl;

  outAllAdapter(os);

  result = os.str();

  return true;
}

bool Application::cmdCloseCoreDump(const string& command, const string& params, string& result) {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  struct rlimit tlimit;
  int ret = 0;
  ostringstream os;

  ret = getrlimit(RLIMIT_CORE, &tlimit);
  if (ret != 0) {
    TLOGERROR("error: " << strerror(errno) << endl);
    return false;
  }

  TLOGDEBUG("before :cur:" << tlimit.rlim_cur << ";max: " << tlimit.rlim_max << endl);

  os << (ServerConfig::Application + "." + ServerConfig::ServerName);

  os << "|before set:cur:" << tlimit.rlim_cur << ";max: " << tlimit.rlim_max;

  string param = KT_Common::lower(KT_Common::trim(params));

  bool bClose = (param == "yes") ? true : false;

  if (bClose) {
    tlimit.rlim_cur = 0;
  } else {
    tlimit.rlim_cur = tlimit.rlim_max;
  }

  ret = setrlimit(RLIMIT_CORE, &tlimit);
  if (ret != 0) {
    TLOGERROR("error: " << strerror(errno) << endl);
    return false;
  }

  ret = getrlimit(RLIMIT_CORE, &tlimit);
  if (ret != 0) {
    TLOGERROR("error: " << strerror(errno) << endl);
    return false;
  }

  TLOGDEBUG("after cur:" << tlimit.rlim_cur << ";max: " << tlimit.rlim_max << endl);
  os << "|after set cur:" << tlimit.rlim_cur << ";max: " << tlimit.rlim_max << endl;

  result = os.str();
#else
  TLOGDEBUG("windows not support!");
#endif
  return true;
}

bool Application::cmdSetLogLevel(const string& command, const string& params, string& result) {
  TLOGKANT("Application::cmdSetLogLevel:" << command << " " << params << endl);

  string level = KT_Common::trim(params);

  int ret = LocalRollLogger::getInstance()->logger()->setLogLevel(level);

  if (ret == 0) {
    ServerConfig::LogLevel = KT_Common::upper(level);

    result = "set log level [" + level + "] ok";

    AppCache::getInstance()->set("logLevel", level);
  } else {
    result = "set log level [" + level + "] error";
  }

  return true;
}

bool Application::cmdEnableDayLog(const string& command, const string& params, string& result) {
  TLOGKANT("Application::cmdEnableDayLog:" << command << " " << params << endl);

  vector<string> vParams = KT_Common::sepstr<string>(KT_Common::trim(params), "|");

  size_t nNum = vParams.size();

  if (!(nNum == 2 || nNum == 3)) {
    result = "usage: kant.enabledaylog {remote|local}|[logname]|{true|false}";
    return false;
  }

  if ((vParams[0] != "local" && vParams[0] != "remote")) {
    result = "usage: kant.enabledaylog {remote|local}|[logname]|{true|false}";
    return false;
  }

  if (nNum == 2 && (vParams[1] != "true" && vParams[1] != "false")) {
    result = "usage: kant.enabledaylog {remote|local}|[logname]|{true|false}";
    return false;
  }

  if (nNum == 3 && (vParams[2] != "true" && vParams[2] != "false")) {
    result = "usage: kant.enabledaylog {remote|local}|[logname]|{true|false}";
    return false;
  }

  bool bEnable = true;
  string sFile;

  if (nNum == 2) {
    bEnable = (vParams[1] == "true") ? true : false;
    sFile = "";
    result = "set " + vParams[0] + " " + vParams[1] + " ok";
  } else if (nNum == 3) {
    bEnable = (vParams[2] == "true") ? true : false;
    sFile = vParams[1];
    result = "set " + vParams[0] + " " + vParams[1] + " " + vParams[2] + " ok";
  }

  if (vParams[0] == "local") {
    RemoteTimeLogger::getInstance()->enableLocal(sFile, bEnable);
    return true;
  }

  if (vParams[0] == "remote") {
    RemoteTimeLogger::getInstance()->enableRemote(sFile, bEnable);
    return true;
  }

  result = "usage: kant.enabledaylog {remote|local}|[logname]|{true|false}";
  return false;
}

bool Application::cmdLoadConfig(const string& command, const string& params, string& result) {
  TLOGKANT("Application::cmdLoadConfig:" << command << " " << params << endl);

  string filename = KT_Common::trim(params);

  if (RemoteConfig::getInstance()->addConfig(filename, result, false)) {
    RemoteNotify::getInstance()->report(result);

    return true;
  }

  RemoteNotify::getInstance()->report(result);

  return true;
}

bool Application::cmdConnections(const string& command, const string& params, string& result) {
  TLOGKANT("Application::cmdConnections:" << command << " " << params << endl);

  ostringstream os;

  os << OUT_LINE_LONG << endl;

  auto m = _epollServer->getListenSocketInfo();

  for (auto it = m.begin(); it != m.end(); ++it) {
    vector<KT_EpollServer::ConnStatus> v = it->second->getConnStatus();

    os << OUT_LINE << "\n"
       << KT_Common::outfill("[adapter:" + it->second->getName() + "] [connections:" + KT_Common::tostr(v.size()) + "]")
       << endl;

    os << KT_Common::outfill("conn-uid", ' ', 15) << KT_Common::outfill("ip:port", ' ', 25)
       << KT_Common::outfill("last-time", ' ', 25) << KT_Common::outfill("timeout", ' ', 10)
       << KT_Common::outfill("recvBufferSize", ' ', 30) << KT_Common::outfill("sendBufferSize", ' ', 30) << endl;

    for (size_t i = 0; i < v.size(); i++) {
      os << KT_Common::outfill(KT_Common::tostr<uint32_t>(v[i].uid), ' ', 15)
         << KT_Common::outfill(v[i].ip + ":" + KT_Common::tostr(v[i].port), ' ', 25)
         << KT_Common::outfill(KT_Common::tm2str(v[i].iLastRefreshTime, "%Y-%m-%d %H:%M:%S"), ' ', 25)
         << KT_Common::outfill(KT_Common::tostr(v[i].timeout), ' ', 10)
         << KT_Common::outfill(KT_Common::tostr(v[i].recvBufferSize), ' ', 30)
         << KT_Common::outfill(KT_Common::tostr(v[i].sendBufferSize), ' ', 30) << endl;
    }
  }
  os << OUT_LINE_LONG << endl;

  result = os.str();

  return true;
}

bool Application::cmdViewVersion(const string& command, const string& params, string& result) {
  result = "$" + string("KANT_VERSION") + "$";
  return true;
}

bool Application::cmdViewBuildID(const string& command, const string& params, string& result) {
#define YEARSUF ((__DATE__[9] - '0') * 10 + (__DATE__[10] - '0'))

#define MONTH                               \
  (__DATE__[2] == 'n'                       \
     ? (__DATE__[1] == 'a' ? 0 : 5)         \
     : __DATE__[2] == 'b'                   \
         ? 1                                \
         : __DATE__[2] == 'r'               \
             ? (__DATE__[0] == 'M' ? 2 : 3) \
             : __DATE__[2] == 'y'           \
                 ? 4                        \
                 : __DATE__[2] == 'l'       \
                     ? 6                    \
                     : __DATE__[2] == 'g'   \
                         ? 7                \
                         : __DATE__[2] == 'p' ? 8 : __DATE__[2] == 't' ? 9 : __DATE__[2] == 'v' ? 10 : 11)

#define DAY ((__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 + (__DATE__[5] - '0'))

#define TIMEINT                                                                                                       \
  ((((((__TIME__[0] - '0') * 10 + (__TIME__[1] - '0')) * 10 + (__TIME__[3] - '0')) * 10 + (__TIME__[4] - '0')) * 10 + \
    (__TIME__[6] - '0')) *                                                                                            \
     10 +                                                                                                             \
   (__TIME__[7] - '0'))

  char buildTime[50] = {0};
  sprintf(buildTime, "%d.%02d%02d.%06d", YEARSUF, MONTH + 1, DAY, TIMEINT);
  result = "$" + ServerConfig::Application + "." + ServerConfig::ServerName + "-" + string(buildTime) + "$";
  return true;
}

bool Application::cmdLoadProperty(const string& command, const string& params, string& result) {
  try {
    TLOGKANT("Application::cmdLoadProperty:" << command << " " << params << endl);

    //重新解析配置文件
    _conf.parseFile(ServerConfig::ConfigFile);

    string sResult = "";

    //加载通讯器属性
    _communicator->setProperty(_conf);

    _communicator->reloadProperty(sResult);

    //加载远程对象
    ServerConfig::Log = _conf.get("/kant/application/server<log>");

    RemoteTimeLogger::getInstance()->setLogInfo(_communicator, ServerConfig::Log, ServerConfig::Application,
                                                ServerConfig::ServerName, ServerConfig::LogPath, setDivision());

    ServerConfig::Config = _conf.get("/kant/application/server<config>");

    RemoteConfig::getInstance()->setConfigInfo(_communicator, ServerConfig::Config, ServerConfig::Application,
                                               ServerConfig::ServerName, ServerConfig::BasePath, setDivision(), 5);

    ServerConfig::Notify = _conf.get("/kant/application/server<notify>");

    RemoteNotify::getInstance()->setNotifyInfo(_communicator, ServerConfig::Notify, ServerConfig::Application,
                                               ServerConfig::ServerName, setDivision(), ServerConfig::LocalIp);

    result = "loaded config items:\r\n" + sResult + "log=" + ServerConfig::Log + "\r\n" +
             "config=" + ServerConfig::Config + "\r\n" + "notify=" + ServerConfig::Notify + "\r\n";
  } catch (KT_Config_Exception& ex) {
    result = "load config " + ServerConfig::ConfigFile + " error:" + ex.what();
  } catch (exception& ex) {
    result = ex.what();
  }
  return true;
}

bool Application::cmdViewAdminCommands(const string& command, const string& params, string& result) {
  TLOGKANT("Application::cmdViewAdminCommands:" << command << " " << params << endl);

  result = result + _notifyObserver->viewRegisterCommand();

  return true;
}

bool Application::cmdSetDyeing(const string& command, const string& params, string& result) {
  vector<string> vDyeingParams = KT_Common::sepstr<string>(params, " ");

  if (vDyeingParams.size() == 2 || vDyeingParams.size() == 3) {
    _servantHelper->setDyeing(vDyeingParams[0], vDyeingParams[1], vDyeingParams.size() == 3 ? vDyeingParams[2] : "");

    result = "DyeingKey=" + vDyeingParams[0] + "\r\n" + "DyeingServant=" + vDyeingParams[1] + "\r\n" +
             "DyeingInterface=" + (vDyeingParams.size() == 3 ? vDyeingParams[2] : "") + "\r\n";
  } else {
    result = "Invalid parameters.Should be: dyeingKey dyeingServant [dyeingInterface]";
  }
  return true;
}

bool Application::cmdCloseCout(const string& command, const string& params, string& result) {
  TLOGKANT("Application::cmdCloseCout:" << command << " " << params << endl);

  string s = KT_Common::lower(KT_Common::trim(params));

  if (s == "yes") {
    AppCache::getInstance()->set("closeCout", "1");
  } else {
    AppCache::getInstance()->set("closeCout", "0");
  }

  result = "set closeCout  [" + s + "] ok";

  return true;
}

bool Application::cmdReloadLocator(const string& command, const string& params, string& result) {
  TLOGDEBUG("Application::cmdReloadLocator:" << command << " " << params << endl);

  string sPara = KT_Common::lower(KT_Common::trim(params));

  bool bSucc(true);
  if (sPara == "reload") {
    TLOGDEBUG(__FUNCTION__ << "|" << __LINE__ << "|conf file:" << ServerConfig::ConfigFile << endl);

    KT_Config reloadConf;

    reloadConf.parseFile(ServerConfig::ConfigFile);
    string sLocator = reloadConf.get("/kant/application/client/<locator>", "");

    TLOGDEBUG(__FUNCTION__ << "|" << __LINE__ << "|conf file:" << ServerConfig::ConfigFile << "\n"
                           << "|sLocator:" << sLocator << endl);

    if (sLocator.empty()) {
      bSucc = false;
      result = "locator info is null.";
    } else {
      _communicator->setProperty("locator", sLocator);
      _communicator->reloadLocator();
      result = sLocator + " set succ.";
    }

  } else {
    result = "please input right paras.";
    bSucc = false;
  }

  return bSucc;
}

bool Application::cmdViewResource(const string& command, const string& params, string& result) {
  TLOGDEBUG("Application::cmdViewResource:" << command << " " << params << endl);

  ostringstream os;

  os << _communicator->getResourcesInfo() << endl;

  os << OUT_LINE << endl;

  vector<KT_EpollServer::BindAdapterPtr> adapters = _epollServer->getBindAdapters();
  for (auto adapter : adapters) {
    outAdapter(os, _servantHelper->getAdapterServant(adapter->getName()), adapter);
    os << KT_Common::outfill("recv-buffer-count") << adapter->getRecvBufferSize() << endl;
    os << KT_Common::outfill("send-buffer-count") << adapter->getSendBufferSize() << endl;
  }

  result += os.str();

  TLOGDEBUG("Application::cmdViewResource result:" << result << endl);

  return true;
}

void Application::outAllAdapter(ostream& os) {
  auto m = _epollServer->getListenSocketInfo();

  for (auto it = m.begin(); it != m.end(); ++it) {
    outAdapter(os, _servantHelper->getAdapterServant(it->second->getName()), it->second);

    os << OUT_LINE << endl;
  }
}

bool Application::addConfig(const string& filename) {
  string result;

  if (RemoteConfig::getInstance()->addConfig(filename, result, false)) {
    RemoteNotify::getInstance()->report(result);

    return true;
  }
  RemoteNotify::getInstance()->report(result);

  return true;
}

bool Application::addAppConfig(const string& filename) {
  string result = "";

  // true-只获取应用级别配置
  if (RemoteConfig::getInstance()->addConfig(filename, result, true))

  {
    RemoteNotify::getInstance()->report(result);

    return true;
  }

  RemoteNotify::getInstance()->report(result);

  return true;
}

void Application::main(int argc, char* argv[]) {
  KT_Option op;
  op.decode(argc, argv);

  main(op);
}

void Application::main(const KT_Option& option) {
  __out__.modFlag(0xfffff, false);
  //直接输出编译的TAF版本
  if (option.hasParam("version")) {
    __out__.debug() << "KANT:"
                    << "KANT_VERSION" << endl;
    exit(0);
  }

  //加载配置文件
  ServerConfig::ConfigFile = option.getValue("config");

  if (ServerConfig::ConfigFile == "") {
    cerr << "start server with config, for example: exe --config=config.conf" << endl;

    exit(-1);
  }

  string config = KT_File::load2str(ServerConfig::ConfigFile);

  __out__.debug() << "config:" << ServerConfig::ConfigFile << endl;
  __out__.debug() << "config:" << config << endl;

  main(config);
}

void Application::main(const string& config) {
  try {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
    KT_Common::ignorePipe();
#endif
    __out__.modFlag(0xFFFF, false);

    //解析配置文件
    parseConfig(config);

    //初始化Proxy部分
    initializeClient();

    //初始化Server部分
    initializeServer();

    vector<KT_EpollServer::BindAdapterPtr> adapters;

    //绑定对象和端口
    bindAdapter(adapters);

    stringstream os;

    //输出所有adapter
    outAllAdapter(os);

    __out__.info() << os.str();

    __out__.info() << "\n" << KT_Common::outfill("[initialize server] ", '.') << " [Done]" << endl;

    __out__.info() << OUT_LINE_LONG << endl;

    {
      bool initing = true;
      std::mutex mtx;
      std::condition_variable cond;

      std::thread keepActiving([&] {
        do {
          //发送心跳给node, 表示正在启动
          KANT_KEEPACTIVING;

          //等待initialize初始化完毕
          std::unique_lock<std::mutex> lock(mtx);
          cond.wait_for(lock, std::chrono::seconds(5), [&]() { return !initing; });

        } while (initing);
      });

      try {
        //业务应用的初始化
        initialize();

        {
          std::unique_lock<std::mutex> lock(mtx);
          initing = false;
          cond.notify_all();
        }

        keepActiving.join();
      } catch (exception& ex) {
        keepActiving.detach();

        NOTIFY_AND_WAIT("exit: " + string(ex.what()));

        __out__.error() << "[init exception]:" << ex.what() << endl;
        exit(-1);
      }
    }

    //动态加载配置文件
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_LOAD_CONFIG, Application::cmdLoadConfig);

    //动态设置滚动日志等级
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_SET_LOG_LEVEL, Application::cmdSetLogLevel);

    //动态设置按天日志等级
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_SET_DAYLOG_LEVEL, Application::cmdEnableDayLog);

    //查看服务状态
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_VIEW_STATUS, Application::cmdViewStatus);

    //查看当前链接状态
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_CONNECTIONS, Application::cmdConnections);

    //查看编译的KANT版本
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_VIEW_VERSION, Application::cmdViewVersion);

    //查看服务buildid(编译时间）
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_VIEW_BID, Application::cmdViewBuildID);

    //加载配置文件中的属性信息
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_LOAD_PROPERTY, Application::cmdLoadProperty);

    //查看服务支持的管理命令
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_VIEW_ADMIN_COMMANDS, Application::cmdViewAdminCommands);

    //设置染色信息
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_SET_DYEING, Application::cmdSetDyeing);

    //设置服务的core limit
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_CLOSE_CORE, Application::cmdCloseCoreDump);

    //设置是否标准输出
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_CLOSE_COUT, Application::cmdCloseCout);

    //设置是否标准输出
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_RELOAD_LOCATOR, Application::cmdReloadLocator);

    //设置是否标准输出
    KANT_ADD_ADMIN_CMD_PREFIX(KANT_CMD_RESOURCE, Application::cmdViewResource);

    //上报版本
    KANT_REPORTVERSION("KANT_VERSION");

    //发送心跳给node, 表示启动了
    KANT_KEEPALIVE("");

    //发送给notify表示服务启动了
    RemoteNotify::getInstance()->report("restart");

    //ctrl + c能够完美结束服务
    _ctrlCId = KT_Port::registerCtrlC([=] {
      this->terminate();
#if TARGET_PLATFORM_WINDOWS
      ExitProcess(0);
#endif
    });
    _termId = KT_Port::registerTerm([=] {
      this->terminate();
#if TARGET_PLATFORM_WINDOWS
      ExitProcess(0);
#endif
    });

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
    if (_conf.get("/kant/application/server<closecout>", AppCache::getInstance()->get("closeCout")) != "0") {
      // 重定向stdin、stdout、stderr
      int fd = open("/dev/null", O_RDWR);
      if (fd != -1) {
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
      } else {
        close(0);
        close(1);
        close(2);
      }
    }
#endif
  } catch (exception& ex) {
    __out__.error() << "[Application]:" << ex.what() << endl;

    terminate();

    NOTIFY_AND_WAIT("exit: " + string(ex.what()));

    exit(-1);
  }

  //初始化完毕后, 日志再修改为异步
  LocalRollLogger::getInstance()->sync(false);
}

void Application::parseConfig(const string& config) {
  _conf.parseString(config);

  __out__.setLogLevel(_conf.get("/kant/application/server<start_output>", "DEBUG"));

  onParseConfig(_conf);
}

KT_EpollServer::BindAdapter::EOrder Application::parseOrder(const string& s) {
  vector<string> vtOrder = KT_Common::sepstr<string>(s, ";, \t", false);

  if (vtOrder.size() != 2) {
    cerr << "invalid order '" << KT_Common::tostr(vtOrder) << "'." << endl;

    exit(0);
  }
  if ((KT_Common::lower(vtOrder[0]) == "allow") && (KT_Common::lower(vtOrder[1]) == "deny")) {
    return KT_EpollServer::BindAdapter::ALLOW_DENY;
  }
  if ((KT_Common::lower(vtOrder[0]) == "deny") && (KT_Common::lower(vtOrder[1]) == "allow")) {
    return KT_EpollServer::BindAdapter::DENY_ALLOW;
  }

  cerr << "invalid order '" << KT_Common::tostr(vtOrder) << "'." << endl;

  exit(0);
}

void Application::initializeClient() {
  __out__.info() << "\n" << OUT_LINE_LONG << endl;

  //初始化通信器
  _communicator = CommunicatorFactory::getInstance()->getCommunicator(_conf);

  __out__.info() << KT_Common::outfill("[proxy config]:") << endl;

  //输出
  stringstream os;
  outClient(os);

  __out__.info() << os.str();
}

void Application::outClient(ostream& os) {
  os << OUT_LINE << "\n" << KT_Common::outfill("[load client]:") << endl;

  os << KT_Common::outfill("locator") << _communicator->getProperty("locator") << endl;
  os << KT_Common::outfill("sync-invoke-timeout") << _communicator->getProperty("sync-invoke-timeout") << endl;
  os << KT_Common::outfill("async-invoke-timeout") << _communicator->getProperty("async-invoke-timeout") << endl;
  os << KT_Common::outfill("refresh-endpoint-interval") << _communicator->getProperty("refresh-endpoint-interval")
     << endl;
  os << KT_Common::outfill("stat") << _communicator->getProperty("stat") << endl;
  os << KT_Common::outfill("property") << _communicator->getProperty("property") << endl;
  os << KT_Common::outfill("report-interval") << _communicator->getProperty("report-interval") << endl;
  os << KT_Common::outfill("keep-alive-interval") << _communicator->getProperty("keep-alive-interval") << endl;
  //    os << KT_Common::outfill("sample-rate")                 << _communicator->getProperty("sample-rate") << endl;
  //    os << KT_Common::outfill("max-sample-count")            << _communicator->getProperty("max-sample-count") << endl;
  os << KT_Common::outfill("netthread") << _communicator->getProperty("netthread") << endl;
  os << KT_Common::outfill("asyncthread") << _communicator->getProperty("asyncthread") << endl;
  os << KT_Common::outfill("modulename") << _communicator->getProperty("modulename") << endl;
  os << KT_Common::outfill("enableset") << _communicator->getProperty("enableset") << endl;
  os << KT_Common::outfill("setdivision") << _communicator->getProperty("setdivision") << endl;
}

string Application::toDefault(const string& s, const string& sDefault) {
  if (s.empty()) {
    return sDefault;
  }
  return s;
}

string Application::setDivision() {
  bool bEnableSet = KT_Common::lower(_conf.get("/kant/application<enableset>", "n")) == "y" ? true : false;
  ;

  string sSetDevision = bEnableSet ? _conf.get("/kant/application<setdivision>", "") : "";
  return sSetDevision;
}

void Application::addServantProtocol(const string& servant, const KT_NetWorkBuffer::protocol_functor& protocol) {
  string adapterName = _servantHelper->getServantAdapter(servant);

  if (adapterName == "") {
    throw runtime_error("addServantProtocol fail, no found adapter for servant:" + servant);
  }
  getEpollServer()->getBindAdapter(adapterName)->setProtocol(protocol);
}

void Application::addAcceptCallback(const KT_EpollServer::accept_callback_functor& cb) { _acceptFuncs.push_back(cb); }

void Application::onAccept(KT_EpollServer::Connection* cPtr) {
  for (size_t i = 0; i < _acceptFuncs.size(); ++i) {
    _acceptFuncs[i](cPtr);
  }
}

//void Application::addServantOnClose(const string& servant, const KT_EpollServer::close_functor& cf)
//{
//    string adapterName = _servantHelper->getServantAdapter(servant);
//
//    if (adapterName.empty())
//    {
//        throw runtime_error("setServantOnClose fail, no found adapter for servant:" + servant);
//    }
//
//    getEpollServer()->getBindAdapter(adapterName)->setOnClose(cf);
//}

void Application::outServer(ostream& os) {
  os << KT_Common::outfill("Application(app)") << ServerConfig::Application << endl;
  os << KT_Common::outfill("ServerName(server)") << ServerConfig::ServerName << endl;
  os << KT_Common::outfill("BasePath(basepath)") << ServerConfig::BasePath << endl;
  os << KT_Common::outfill("DataPath(datapath)") << ServerConfig::DataPath << endl;
  os << KT_Common::outfill("LocalIp(localip)") << ServerConfig::LocalIp << endl;
  os << KT_Common::outfill("Local(local)") << ServerConfig::Local << endl;
  os << KT_Common::outfill("LogPath(logpath)") << ServerConfig::LogPath << endl;
  os << KT_Common::outfill("LogSize(logsize)") << ServerConfig::LogSize << endl;
  os << KT_Common::outfill("LogNum(lognum)") << ServerConfig::LogNum << endl;
  os << KT_Common::outfill("LogLevel(loglevel)") << ServerConfig::LogLevel << endl;
  os << KT_Common::outfill("Log(log)") << ServerConfig::Log << endl;
  os << KT_Common::outfill("Node(node)") << ServerConfig::Node << endl;
  os << KT_Common::outfill("Config(config)") << ServerConfig::Config << endl;
  os << KT_Common::outfill("Notify(notify)") << ServerConfig::Notify << endl;
  os << KT_Common::outfill("OpenCoroutine(opencoroutine)") << ServerConfig::OpenCoroutine << endl;
  os << KT_Common::outfill("CoroutineMemSize(coroutinememsize)") << ServerConfig::CoroutineMemSize << endl;
  os << KT_Common::outfill("CoroutineStackSize(coroutinestack)") << ServerConfig::CoroutineStackSize << endl;
  os << KT_Common::outfill("CloseCout(closecout)") << ServerConfig::CloseCout << endl;
  os << KT_Common::outfill("NetThread(netthread)") << ServerConfig::NetThread << endl;
  os << KT_Common::outfill("ManualListen(manuallisten)") << ServerConfig::ManualListen << endl;
  //	os << KT_Common::outfill("MergeNetImp(mergenetimp)")       << ServerConfig::MergeNetImp << endl;
  os << KT_Common::outfill("ReportFlow(reportflow)") << ServerConfig::ReportFlow << endl;
  os << KT_Common::outfill("BackPacketLimit(backpacketlimit)") << ServerConfig::BackPacketLimit << endl;
  os << KT_Common::outfill("BackPacketMin(backpacketmin)") << ServerConfig::BackPacketMin << endl;

#if KANT_SSL
  os << KT_Common::outfill("Ca(ca)") << ServerConfig::CA << endl;
  os << KT_Common::outfill("Cert(cert)") << ServerConfig::Cert << endl;
  os << KT_Common::outfill("Key(key)") << ServerConfig::Key << endl;
  os << KT_Common::outfill("VerifyClient(verifyclient)") << ServerConfig::VerifyClient << endl;
  os << KT_Common::outfill("Ciphers(ciphers)") << ServerConfig::Ciphers << endl;
#endif
}

void Application::initializeServer() {
  __out__.info() << OUT_LINE << "\n" << KT_Common::outfill("[server config]:") << endl;

  ServerConfig::Application = toDefault(_conf.get("/kant/application/server<app>"), "UNKNOWN");

  //缺省采用进程名称
  string exe = "";

  try {
    exe = KT_File::extractFileName(KT_File::getExePath());
  } catch (KT_File_Exception& ex) {
    //取失败则使用ip代替进程名
    exe = _conf.get("/kant/application/server<localip>");
  }

  ServerConfig::ServerName = toDefault(_conf.get("/kant/application/server<server>"), exe);

#if TARGET_PLATFORM_WINDOWS
  ServerConfig::BasePath = KT_File::simplifyDirectory(_conf.get("/kant/application/server<basepath.win>")) + FILE_SEP;
  if (ServerConfig::BasePath == FILE_SEP) {
    ServerConfig::BasePath =
      KT_File::simplifyDirectory(toDefault(_conf.get("/kant/application/server<basepath>"), ".")) + FILE_SEP;
  }

  ServerConfig::DataPath = KT_File::simplifyDirectory(_conf.get("/kant/application/server<datapath.win>")) + FILE_SEP;
  if (ServerConfig::DataPath == FILE_SEP) {
    ServerConfig::DataPath =
      KT_File::simplifyDirectory(toDefault(_conf.get("/kant/application/server<datapath>"), ".")) + FILE_SEP;
  }

  ServerConfig::LogPath = KT_File::simplifyDirectory(_conf.get("/kant/application/server<logpath.win>")) + FILE_SEP;
  if (ServerConfig::LogPath == FILE_SEP) {
    ServerConfig::LogPath =
      KT_File::simplifyDirectory(toDefault(_conf.get("/kant/application/server<logpath>"), ".")) + FILE_SEP;
  }
#else
  ServerConfig::BasePath =
    KT_File::simplifyDirectory(toDefault(_conf.get("/kant/application/server<basepath>"), ".")) + FILE_SEP;
  ServerConfig::DataPath =
    KT_File::simplifyDirectory(toDefault(_conf.get("/kant/application/server<datapath>"), ".")) + FILE_SEP;
  ServerConfig::LogPath =
    KT_File::simplifyDirectory(toDefault(_conf.get("/kant/application/server<logpath>"), ".")) + FILE_SEP;

#endif
  ServerConfig::KantPath = KT_File::simplifyDirectory(ServerConfig::LogPath + FILE_SEP + ".." + FILE_SEP) + FILE_SEP;

  ServerConfig::LogSize =
    KT_Common::toSize(toDefault(_conf.get("/kant/application/server<logsize>"), "52428800"), 52428800);
  ServerConfig::LogNum = KT_Common::strto<int>(toDefault(_conf.get("/kant/application/server<lognum>"), "10"));
  ServerConfig::LocalIp = _conf.get("/kant/application/server<localip>");
  ServerConfig::Local = _conf.get("/kant/application/server<local>");
  ServerConfig::Node = _conf.get("/kant/application/server<node>");
  ServerConfig::Log = _conf.get("/kant/application/server<log>");
  ServerConfig::Config = _conf.get("/kant/application/server<config>");
  ServerConfig::Notify = _conf.get("/kant/application/server<notify>");
  ServerConfig::ReportFlow = _conf.get("/kant/application/server<reportflow>") == "0" ? 0 : 1;
  ServerConfig::IsCheckSet = _conf.get("/kant/application/server<checkset>", "1") == "0" ? 0 : 1;
  ServerConfig::OpenCoroutine =
    KT_Common::strto<int>(toDefault(_conf.get("/kant/application/server<opencoroutine>"), "0"));
  ServerConfig::CoroutineMemSize =
    KT_Common::toSize(toDefault(_conf.get("/kant/application/server<coroutinememsize>"), "1G"), 1024 * 1024 * 1024);
  ServerConfig::CoroutineStackSize =
    (uint32_t)KT_Common::toSize(toDefault(_conf.get("/kant/application/server<coroutinestack>"), "128K"), 1024 * 128);
  ServerConfig::ManualListen = _conf.get("/kant/application/server<manuallisten>", "0") == "0" ? false : true;
  //	ServerConfig::MergeNetImp       = _conf.get("/kant/application/server<mergenetimp>", "0") == "0" ? false : true;
  ServerConfig::NetThread = KT_Common::strto<int>(toDefault(_conf.get("/kant/application/server<netthread>"), "1"));
  ServerConfig::CloseCout = _conf.get("/kant/application/server<closecout>", "1") == "0" ? 0 : 1;
  ServerConfig::BackPacketLimit =
    KT_Common::strto<int>(_conf.get("/kant/application/server<backpacketlimit>", KT_Common::tostr(100 * 1024 * 1024)));
  ServerConfig::BackPacketMin = KT_Common::strto<int>(_conf.get("/kant/application/server<backpacketmin>", "1024"));

  ServerConfig::Context["node_name"] = ServerConfig::LocalIp;
#if KANT_SSL
  ServerConfig::CA = _conf.get("/kant/application/server<ca>");
  ServerConfig::Cert = _conf.get("/kant/application/server<cert>");
  ServerConfig::Key = _conf.get("/kant/application/server<key>");
  ServerConfig::VerifyClient = _conf.get("/kant/application/server<verifyclient>", "0") == "0" ? false : true;
  ServerConfig::Ciphers = _conf.get("/kant/application/server<ciphers>");

  if (!ServerConfig::Cert.empty()) {
    _ctx = KT_OpenSSL::newCtx(ServerConfig::CA, ServerConfig::Cert, ServerConfig::Key, ServerConfig::VerifyClient,
                              ServerConfig::Ciphers);

    if (!_ctx) {
      TLOGERROR("[load server ssl error, ca:" << ServerConfig::CA << endl);
      exit(-1);
    }
  }
#endif

  if (ServerConfig::LocalIp.empty()) {
    // ServerConfig::LocalIp = "127.0.0.1";
    vector<string> v = KT_Socket::getLocalHosts();

    ServerConfig::LocalIp = "127.0.0.1";
    //获取第一个非127.0.0.1的IP
    for (size_t i = 0; i < v.size(); i++) {
      if (v[i] != "127.0.0.1") {
        ServerConfig::LocalIp = v[i];
        break;
      }
    }
  }

  onServerConfig();

  ostringstream os;
  //输出信息
  outServer(os);

  __out__.info() << os.str();

  if (ServerConfig::NetThread < 1) {
    ServerConfig::NetThread = 1;
    __out__.info() << OUT_LINE << "\nwarning:netThreadNum < 1." << endl;
  }

  //网络线程的配置数目不能15个
  if (ServerConfig::NetThread > 15) {
    ServerConfig::NetThread = 15;
    __out__.info() << OUT_LINE << "\nwarning:netThreadNum > 15." << endl;
  }

  if (ServerConfig::CoroutineMemSize / ServerConfig::CoroutineStackSize <= 0) {
    __out__.error() << OUT_LINE << "\nerror:coroutine paramter error: coroutinememsize/coroutinestack <= 0!." << endl;
    exit(-1);
  }
  _epollServer = std::make_shared<KT_EpollServer>();

  _epollServer->setThreadNum(ServerConfig::NetThread);
  _epollServer->setOpenCoroutine((KT_EpollServer::SERVER_OPEN_COROUTINE)ServerConfig::OpenCoroutine);
  _epollServer->setCoroutineStack(ServerConfig::CoroutineMemSize / ServerConfig::CoroutineStackSize,
                                  ServerConfig::CoroutineStackSize);

  _epollServer->setOnAccept(std::bind(&Application::onAccept, this, std::placeholders::_1));

  //初始化服务是否对空链接进行超时检查
  //    bool bEnable = (_conf.get("/kant/application/server<emptyconcheck>","0")=="1")?true:false;

  //    _epollServer->enAntiEmptyConnAttack(bEnable);
  _epollServer->setEmptyConnTimeout(
    KT_Common::strto<int>(toDefault(_conf.get("/kant/application/server<emptyconntimeout>"), "0")));

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //初始化本地文件cache
  __out__.info() << OUT_LINE << "\n" << KT_Common::outfill("[set file cache ]") << "OK" << endl;
  AppCache::getInstance()->setCacheInfo(ServerConfig::DataPath + ServerConfig::ServerName + ".kantdat", 0);

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //初始化本地Log
  __out__.info() << OUT_LINE << "\n" << KT_Common::outfill("[set roll logger] ") << "OK" << endl;
  LocalRollLogger::getInstance()->setLogInfo(ServerConfig::Application, ServerConfig::ServerName, ServerConfig::LogPath,
                                             ServerConfig::LogSize, ServerConfig::LogNum, _communicator,
                                             ServerConfig::Log);
  _epollServer->setLocalLogger(LocalRollLogger::getInstance()->logger());

  //初始化是日志为同步
  LocalRollLogger::getInstance()->sync(true);

  //设置日志级别
  string level = AppCache::getInstance()->get("logLevel");
  if (level.empty()) {
    level = _conf.get("/kant/application/server<logLevel>", "DEBUG");
  }

  ServerConfig::LogLevel = KT_Common::upper(level);

  LocalRollLogger::getInstance()->logger()->setLogLevel(ServerConfig::LogLevel);

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //初始化到LogServer代理
  __out__.info() << OUT_LINE << "\n" << KT_Common::outfill("[set time logger] ") << "OK" << endl;
  bool bLogStatReport = (_conf.get("/kant/application/server<logstatreport>", "0") == "1") ? true : false;
  RemoteTimeLogger::getInstance()->setLogInfo(_communicator, ServerConfig::Log, ServerConfig::Application,
                                              ServerConfig::ServerName, ServerConfig::LogPath, setDivision(),
                                              bLogStatReport);

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //初始化到配置中心代理
  __out__.info() << OUT_LINE << "\n" << KT_Common::outfill("[set remote config] ") << "OK" << endl;
  RemoteConfig::getInstance()->setConfigInfo(_communicator, ServerConfig::Config, ServerConfig::Application,
                                             ServerConfig::ServerName, ServerConfig::BasePath, setDivision());

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //初始化到信息中心代理
  __out__.info() << OUT_LINE << "\n" << KT_Common::outfill("[set remote notify] ") << "OK" << endl;
  RemoteNotify::getInstance()->setNotifyInfo(_communicator, ServerConfig::Notify, ServerConfig::Application,
                                             ServerConfig::ServerName, setDivision(), ServerConfig::LocalIp);

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //初始化到Node的代理
  __out__.info() << OUT_LINE << "\n" << KT_Common::outfill("[set node proxy]") << "OK" << endl;
  KeepAliveNodeFHelper::getInstance()->setNodeInfo(_communicator, ServerConfig::Node, ServerConfig::Application,
                                                   ServerConfig::ServerName);

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //初始化管理对象
  __out__.info() << OUT_LINE << "\n" << KT_Common::outfill("[set admin adapter]") << "OK" << endl;

  if (!ServerConfig::Local.empty()) {
    _servantHelper->addServant<AdminServant>("AdminObj", this);

    string adminAdapter = "AdminAdapter";

    _servantHelper->setAdapterServant(adminAdapter, "AdminObj");

    KT_EpollServer::BindAdapterPtr lsPtr =
      _epollServer->createBindAdapter<ServantHandle>(adminAdapter, ServerConfig::Local, 1, this);

    setAdapter(lsPtr, adminAdapter);

    lsPtr->setMaxConns(KT_EpollServer::BindAdapter::DEFAULT_MAX_CONN);

    lsPtr->setQueueCapacity(KT_EpollServer::BindAdapter::DEFAULT_QUEUE_CAP);

    lsPtr->setQueueTimeout(KT_EpollServer::BindAdapter::DEFAULT_QUEUE_TIMEOUT);

    lsPtr->setProtocolName("kant");

    lsPtr->setProtocol(AppProtocol::parse);

    _epollServer->bind(lsPtr);
  }

  //队列取平均值
  if (!_communicator->getProperty("property").empty()) {
    string sRspQueue("");
    sRspQueue += ServerConfig::Application;
    sRspQueue += ".";
    sRspQueue += ServerConfig::ServerName;
    sRspQueue += ".sendrspqueue";

    g_pReportRspQueue = _communicator->getStatReport()->createPropertyReport(sRspQueue, PropertyReport::avg());
  }

  KantTimeLogger::getInstance()->enableLocal(TRACE_LOG_FILENAME, false);
}

void Application::setAdapter(KT_EpollServer::BindAdapterPtr& adapter, const string& name) {
  // 设置该obj的鉴权账号密码，只要一组就够了
  {
    std::string accKey = _conf.get("/kant/application/server/" + name + "<accesskey>");
    std::string secretKey = _conf.get("/kant/application/server/" + name + "<secretkey>");

    //注意这里必须用weak, 否则adapter最终释放不了!
    weak_ptr<KT_EpollServer::BindAdapter> a = adapter;
    adapter->setAkSkCallback(accKey, secretKey,
                             std::bind(&kant::serverVerifyAuthCallback, std::placeholders::_1, std::placeholders::_2, a,
                                       _servantHelper->getAdapterServant(name)));
  }

#if KANT_SSL
  string cert = _conf.get("/kant/application/server/" + name + "<cert>");

  if (!cert.empty()) {
    string ca = _conf.get("/kant/application/server/" + name + "<ca>");
    string key = _conf.get("/kant/application/server/" + name + "<key>");
    bool verifyClient = _conf.get("/kant/application/server/" + name + "<verifyclient>", "0") == "0" ? false : true;
    string ciphers = _conf.get("/kant/application/server/" + name + "<ciphers>");

    shared_ptr<KT_OpenSSL::CTX> ctx = KT_OpenSSL::newCtx(ca, cert, key, verifyClient, ciphers);

    if (!ctx) {
      TLOGERROR("load server ssl error, cert:" << cert << endl);
      exit(-1);
    }

    adapter->setSSLCtx(ctx);
  } else {
    adapter->setSSLCtx(_ctx);
  }
#endif
}

void Application::bindAdapter(vector<KT_EpollServer::BindAdapterPtr>& adapters) {
  string sPrefix = ServerConfig::Application + "." + ServerConfig::ServerName + ".";

  vector<string> adapterName;

  map<string, ServantHandle*> servantHandles;

  if (_conf.getDomainVector("/kant/application/server", adapterName)) {
    for (size_t i = 0; i < adapterName.size(); i++) {
      string servant = _conf.get("/kant/application/server/" + adapterName[i] + "<servant>");

      checkServantNameValid(servant, sPrefix);

      _servantHelper->setAdapterServant(adapterName[i], servant);

      string sLastPath = "/kant/application/server/" + adapterName[i];
      KT_Endpoint ep;
      ep.parse(_conf[sLastPath + "<endpoint>"]);
      if (ep.getHost() == "localip") {
        ep.setHost(ServerConfig::LocalIp);
      }

      KT_EpollServer::BindAdapterPtr bindAdapter = _epollServer->createBindAdapter<ServantHandle>(
        adapterName[i], _conf[sLastPath + "<endpoint>"], KT_Common::strto<int>(_conf.get(sLastPath + "<threads>", "1")),
        this);

      //init auth & ssl
      setAdapter(bindAdapter, adapterName[i]);

      bindAdapter->setMaxConns(KT_Common::strto<int>(_conf.get(sLastPath + "<maxconns>", "128")));

      bindAdapter->setOrder(parseOrder(_conf.get(sLastPath + "<order>", "allow,deny")));

      bindAdapter->setAllow(KT_Common::sepstr<string>(_conf[sLastPath + "<allow>"], ";,", false));

      bindAdapter->setDeny(KT_Common::sepstr<string>(_conf.get(sLastPath + "<deny>", ""), ";,", false));

      bindAdapter->setQueueCapacity(KT_Common::strto<int>(_conf.get(sLastPath + "<queuecap>", "1024")));

      bindAdapter->setQueueTimeout(KT_Common::strto<int>(_conf.get(sLastPath + "<queuetimeout>", "10000")));

      bindAdapter->setProtocolName(_conf.get(sLastPath + "<protocol>", "kant"));

      bindAdapter->setBackPacketBuffLimit(ServerConfig::BackPacketLimit);

      bindAdapter->setBackPacketBuffMin(ServerConfig::BackPacketMin);

      if (bindAdapter->isKantProtocol()) {
        bindAdapter->setProtocol(AppProtocol::parse);
      }

      //校验ssl正常初始化
#if KANT_SSL
      if (bindAdapter->getEndpoint().isSSL() && (!(bindAdapter->getSSLCtx()))) {
        __out__.error() << "load server ssl error, no cert config!" << bindAdapter->getEndpoint().toString() << endl;
        exit(-1);
      }
#endif

      if (ServerConfig::ManualListen) {
        //手工监听
        bindAdapter->enableManualListen();
      }

      _epollServer->bind(bindAdapter);

      adapters.push_back(bindAdapter);

      //队列取平均值
      if (!_communicator->getProperty("property").empty()) {
        PropertyReportPtr p;
        p = _communicator->getStatReport()->createPropertyReport(bindAdapter->getName() + ".queue",
                                                                 PropertyReport::avg());
        bindAdapter->_pReportQueue = p.get();

        p = _communicator->getStatReport()->createPropertyReport(bindAdapter->getName() + ".connectRate",
                                                                 PropertyReport::avg());
        bindAdapter->_pReportConRate = p.get();

        p = _communicator->getStatReport()->createPropertyReport(bindAdapter->getName() + ".timeoutNum",
                                                                 PropertyReport::sum());
        bindAdapter->_pReportTimeoutNum = p.get();
      }
    }
  }
}

void Application::checkServantNameValid(const string& servant, const string& sPrefix) {
  if ((servant.length() <= sPrefix.length()) || (servant.substr(0, sPrefix.length()) != sPrefix)) {
    ostringstream os;

    os << "Servant '" << servant << "' error: must be start with '" << sPrefix << "'";

    NOTIFY_AND_WAIT("exit: " + string(os.str()));

    __out__.error() << os.str() << endl;

    exit(-1);
  }
}

void Application::outAdapter(ostream& os, const string& v, KT_EpollServer::BindAdapterPtr lsPtr) {
  os << KT_Common::outfill("name") << lsPtr->getName() << endl;
  os << KT_Common::outfill("servant") << v << endl;
  os << KT_Common::outfill("endpoint") << lsPtr->getEndpoint().toString() << endl;
  os << KT_Common::outfill("maxconns") << lsPtr->getMaxConns() << endl;
  os << KT_Common::outfill("queuecap") << lsPtr->getQueueCapacity() << endl;
  os << KT_Common::outfill("queuetimeout") << lsPtr->getQueueTimeout() << "ms" << endl;
  os << KT_Common::outfill("order")
     << (lsPtr->getOrder() == KT_EpollServer::BindAdapter::ALLOW_DENY ? "allow,deny" : "deny,allow") << endl;
  os << KT_Common::outfill("allow") << KT_Common::tostr(lsPtr->getAllow()) << endl;
  os << KT_Common::outfill("deny") << KT_Common::tostr(lsPtr->getDeny()) << endl;
  // os << outfill("queuesize")        << lsPtr->getRecvBufferSize() << endl;
  os << KT_Common::outfill("connections") << lsPtr->getNowConnection() << endl;
  os << KT_Common::outfill("protocol") << lsPtr->getProtocolName() << endl;
  os << KT_Common::outfill("handlethread") << lsPtr->getHandleNum() << endl;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace kant
