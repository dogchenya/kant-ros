#include "servant/AppCache.h"
#include "servant/Communicator.h"

namespace kant {

//////////////////////////////////////////////////////////////////////
// 缓存
void AppCache::setCacheInfo(const string &sFile, int32_t iSynInterval) {
  try {
    KT_LockT<KT_ThreadMutex> lock(*this);

    string sPath = KT_File::extractFilePath(sFile);

    KT_File::makeDirRecursive(sPath);

    _file = sFile;

    _synInterval = iSynInterval;

    if (KT_File::isFileExistEx(_file)) {
      _fileCache.parseFile(_file);
    }

    //如果是旧版本数据（无版本号)直接清理
    if (_fileCache.get(string(APPCACHE_ROOT_PATH) + "<kantversion>", "") == "") {
      KT_Config tFileCache;

      _fileCache = tFileCache;
    }
  } catch (exception &e) {
    TLOGERROR("[KANT][AppCache setCacheInfo ex:" << e.what() << "]" << endl);
  }
}

string AppCache::get(const string &sName, const string sDomain) {
  if (_file.empty()) {
    return "";
  }

  try {
    KT_LockT<KT_ThreadMutex> lock(*this);
    string sValue = _fileCache.get(string(APPCACHE_ROOT_PATH) + "/" + sDomain + "<" + sName + ">");
    return sValue;
  } catch (exception &e) {
    TLOGERROR("[KANT][AppCache get sName:" << sName << ",ex:" << e.what() << "]" << endl);
  }
  return "";
}

map<string, string> AppCache::getDomainMap(const string &path) {
  map<string, string> m;

  if (_file.empty()) {
    return m;
  }

  try {
    KT_LockT<KT_ThreadMutex> lock(*this);
    m = _fileCache.getDomainMap(string(APPCACHE_ROOT_PATH) + "/" + path);
  } catch (exception &e) {
    TLOGERROR("[KANT][AppCache getDomainMap path:" << path << ",ex:" << e.what() << "]" << endl);
  }
  return m;
}

int AppCache::set(const string &sName, const string &sValue, const string sDomain) {
  if (_file.empty()) {
    return -1;
  }

  try {
    KT_LockT<KT_ThreadMutex> lock(*this);

    map<string, string> m;
    m[sName] = sValue;

    KT_Config tConf;
    tConf.insertDomainParam(string(APPCACHE_ROOT_PATH) + "/" + sDomain, m, true);
    if (_lastSynTime == 0)  //第一次写数据 打印kantversion
    {
      m.clear();
      m["kantversion"] = ClientConfig::KantVersion;
      tConf.insertDomainParam(string(APPCACHE_ROOT_PATH), m, true);
    }

    {
      m.clear();
      m["modify"] = KT_Common::now2str("%Y-%m-%d %H:%M:%S");
      tConf.insertDomainParam(string(APPCACHE_ROOT_PATH), m, true);
    }

    _fileCache.joinConfig(tConf, true);

    time_t now = TNOW;
    if (_lastSynTime + _synInterval / 1000 > now) {
      return 0;
    }
    _lastSynTime = now;

    KT_File::save2file(_file, _fileCache.tostr());

    return 0;
  } catch (exception &e) {
    TLOGERROR("[KANT][AppCache set name:" << sName << ",value:" << sValue << ",ex:" << e.what() << "]" << endl);
  }

  return -1;
}

//////////////////////////////////////////////////////////////////////
}  // namespace kant
