#include "AdminRegistryServer.h"
#include "AdminRegistryImp.h"
#include "DbProxy.h"
#include "ExecuteTask.h"

KT_Config *g_pconf;
AdminRegistryServer g_app;

extern KT_Config *g_pconf;

//内部版本
const string SERVER_VERSION = "B003";

void AdminRegistryServer::initialize() {
  TLOG_DEBUG("AdminRegistryServer::initialize..." << endl);

  try {
    extern KT_Config *g_pconf;
    string size = Application::getCommunicator()->getProperty("timeout-queue-size", "");
    if (size.empty()) {
      Application::getCommunicator()->setProperty("timeout-queue-size", "100");
    }

    loadServantEndpoint();

    DBPROXY->init(g_pconf);
    //轮询线程
    _reapThread.init();
    _reapThread.start();

    //供admin client访问的对象
    string adminObj = g_pconf->get("/kant/objname<AdminRegObjName>", "");
    if (adminObj != "") {
      addServant<AdminRegistryImp>(adminObj);
    }
    ExecuteTask::getInstance()->init(g_pconf);
  } catch (KT_Exception &ex) {
    TLOG_ERROR("RegistryServer initialize exception:" << ex.what() << endl);
    cerr << "RegistryServer initialize exception:" << ex.what() << endl;
    exit(-1);
  }

  TLOG_DEBUG("RegistryServer::initialize OK!" << endl);
}

int AdminRegistryServer::loadServantEndpoint() {
  map<string, string> mapAdapterServant;
  mapAdapterServant = _servantHelper->getAdapterServant();

  map<string, string>::iterator iter;
  for (iter = mapAdapterServant.begin(); iter != mapAdapterServant.end(); iter++) {
    KT_Endpoint ep = getEpollServer()->getBindAdapter(iter->first)->getEndpoint();

    _mapServantEndpoint[iter->second] = ep.toString();

    TLOG_DEBUG("registry obj: " << iter->second << " = " << ep.toString() << endl);
  }

  return 0;
}

void AdminRegistryServer::destroyApp() {
  _reapThread.terminate();

  TLOG_DEBUG("AdminRegistryServer::destroyApp ok" << endl);
}

void doMonitor(const string &configFile) {
  KT_Config conf;
  conf.parseFile(configFile);

  string obj = "AdminObj@" + conf["/kant/application/server<local>"];
  ServantPrx prx = Application::getCommunicator()->stringToProxy<ServantPrx>(obj);
  prx->kant_ping();
}
void doCommand(int argc, char *argv[]) {
  KT_Option tOp;
  tOp.decode(argc, argv);
  if (tOp.hasParam("version")) {
    cout << "KANT:" << Application::getKantVersion() << endl;
    exit(0);
  }
  if (tOp.hasParam("monitor")) {
    try {
      string configFile = tOp.getValue("config");
      doMonitor(configFile);
    } catch (exception &ex) {
      cout << "doMonitor failed:" << ex.what() << endl;
      exit(-1);
    }
    exit(0);
    return;
  }
}
int main(int argc, char *argv[]) {
  try {
    doCommand(argc, argv);
    g_pconf = &g_app.getConfig();
    g_app.main(argc, argv);

    g_app.waitForShutdown();
  } catch (exception &ex) {
    cerr << ex.what() << endl;
  }

  return 0;
}
