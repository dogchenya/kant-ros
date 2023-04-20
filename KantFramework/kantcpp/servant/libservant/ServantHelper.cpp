#include "servant/ServantHelper.h"

namespace kant {

ServantPtr ServantHelperManager::create(const string &sAdapter) {
  if (_adapter_servant.find(sAdapter) == _adapter_servant.end()) {
    return NULL;
  }

  ServantPtr servant = NULL;

  //根据adapter查找servant名称
  string s = _adapter_servant[sAdapter];

  if (_servant_creator.find(s) != _servant_creator.end()) {
    servant = _servant_creator[s]->create(s);
  }
  return servant;
}

void ServantHelperManager::setAdapterServant(const string &sAdapter, const string &sServant) {
  _adapter_servant[sAdapter] = sServant;

  _servant_adapter[sServant] = sAdapter;
}

bool ServantHelperManager::setDyeing(const string &sDyeingKey, const string &sDyeingServant,
                                     const string &sDyeingInterface) {
  KT_LockT<KT_SpinLock> lock(_mutex);

  _dyeingKey = sDyeingKey;
  _dyeingServant = sDyeingServant;
  _dyeingInterface = sDyeingInterface;

  _isDyeing = !sDyeingKey.empty();

  return true;
}

bool ServantHelperManager::isDyeingReq(const string &sKey, const string &sServant, const string &sInterface) const {
  KT_LockT<KT_SpinLock> lock(_mutex);

  return ((_dyeingKey == sKey) && (_dyeingServant == sServant) &&
          (_dyeingInterface == "" || _dyeingInterface == sInterface));
}

}  // namespace kant
