#include "util/kt_platform.h"
#include "servant/AdminServant.h"
#include "servant/Application.h"
#include "servant/NotifyObserver.h"
#include "servant/ServantHelper.h"

namespace kant {

AdminServant::AdminServant() {}

AdminServant::~AdminServant() {}

void AdminServant::initialize() {}

void AdminServant::destroy() {}

void AdminServant::shutdown(CurrentPtr current) {
  TLOGERROR("[KANT][AdminServant::shutdown] from node" << endl);

#if TARGET_PLATFORM_WINDOWS
  HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
  if (hProcess == NULL) {
    return;
  }

  ::TerminateProcess(hProcess, 0);
#else
  kill(getpid(), SIGINT);  //通过给自己发信号的方式结束, 避免处理线程结束时自己join自己
                           // Application::terminate();
#endif
}

string AdminServant::notify(const string &command, CurrentPtr current) {
  RemoteNotify::getInstance()->report("AdminServant::notify:" + command);

  return this->getApplication()->getNotifyObserver()->notify(command, current);
}

///////////////////////////////////////////////////////////////////////
}  // namespace kant
