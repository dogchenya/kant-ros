#include "HelloServer.h"
#include "HelloImp.h"

using namespace std;

HelloServer g_app;

/////////////////////////////////////////////////////////////////
void HelloServer::initialize() {
  //initialize application here:
  //...

  addServant<HelloImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".AuthObj");
}
/////////////////////////////////////////////////////////////////
void HelloServer::destroyApp() {
  //destroy application here:
  //...
}
/////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  try {
    g_app.main(argc, argv);
    g_app.waitForShutdown();
  } catch (std::exception& e) {
    LOG_CONSOLE_DEBUG << "std::exception error:" << e.what() << std::endl;
  } catch (...) {
    LOG_CONSOLE_DEBUG << "unknown exception." << std::endl;
  }
  return -1;
}
/////////////////////////////////////////////////////////////////
