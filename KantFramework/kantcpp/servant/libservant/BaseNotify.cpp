#include "servant/BaseNotify.h"
#include "servant/NotifyObserver.h"

namespace kant {

BaseNotify::BaseNotify() {}

BaseNotify::~BaseNotify() {}

void BaseNotify::addAdminCommandPrefix(const string& command, TAdminFunc func) {
  KT_LockT<KT_ThreadRecMutex> lock(*this);

  _procFunctors.insert(std::make_pair(command, func));

  _observer->registerPrefix(command, this);
}

void BaseNotify::addAdminCommandNormal(const string& command, TAdminFunc func) {
  KT_LockT<KT_ThreadRecMutex> lock(*this);

  _procFunctors.insert(std::make_pair(command, func));

  _observer->registerNotify(command, this);
}

bool BaseNotify::notify(const string& cmd, const string& params, CurrentPtr current, string& result) {
  KT_LockT<KT_ThreadRecMutex> lock(*this);

  map<string, TAdminFunc>::iterator it;

  it = _procFunctors.find(cmd);

  if (it != _procFunctors.end()) {
    return (it->second)(cmd, params, result);
  }
  return false;
}
//////////////////////////////////////////////////////////////////
}  // namespace kant
