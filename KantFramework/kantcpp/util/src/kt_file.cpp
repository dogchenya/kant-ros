#include "util/kt_file.h"
#include "util/kt_port.h"
#include <set>
#include <string.h>

#if TARGET_PLATFORM_WINDOWS
#include <atlstr.h>
#endif

#if TARGET_PLATFORM_IOS
#include <sys/proc_info.h>
#include <libproc.h>
#endif

namespace kant {

ifstream::pos_type KT_File::getFileSize(const string &sFullFileName) {
  ifstream ifs(sFullFileName.c_str());
  ifs.seekg(0, ios_base::end);
  return ifs.tellg();
}

bool KT_File::isAbsolute(const string &sFullFileName) {
  if (sFullFileName.empty()) {
    return false;
  }

  unsigned i = 0;
  while (isspace(sFullFileName[i])) {
    ++i;
  }
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  return sFullFileName[i] == FILE_SEP[0];
#else
  if (sFullFileName.length() >= i + 2) {
    if (isPanfu(sFullFileName.substr(i, 2))) {
      return true;
    }
  }
  return false;
#endif
}

bool KT_File::isFileExist(const string &sFullFileName, mode_t iFileType) {
  KT_Port::stat_t f_stat;
  if (KT_Port::lstat(sFullFileName.c_str(), &f_stat) == -1) {
    return false;
  }

  if (!(f_stat.st_mode & iFileType)) {
    return false;
  }
  return true;
}

bool KT_File::isFileExistEx(const string &sFullFileName, mode_t iFileType) {
  struct stat f_stat;
  if (stat(sFullFileName.c_str(), &f_stat) == -1) {
    return false;
  }

  if (!(f_stat.st_mode & iFileType)) {
    return false;
  }

  return true;
}

int KT_File::setExecutable(const string &sFullFileName, bool canExecutable) {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS

  struct stat f_stat;

  if (stat(sFullFileName.c_str(), &f_stat) == -1) {
    return -1;
  }

  return KT_Port::chmod(sFullFileName.c_str(), canExecutable ? f_stat.st_mode | S_IXUSR : f_stat.st_mode & ~S_IXUSR);
#else
  return 0;
#endif
}

struct classcomp {
  bool operator()(const string &lhs, const string &rhs) const {
    return KT_Port::strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
  }
};

bool KT_File::canExecutable(const string &sFullFileName) {
#if TARGET_PLATFORM_WINDOWS
  string ex = extractFileExt(sFullFileName);

  static set<string, classcomp> ext = {"exe", "bat", "com"};

  return ext.find(ex) != ext.end();
#else
  struct stat f_stat;

  if (stat(sFullFileName.c_str(), &f_stat) == -1) {
    return false;
  }

  return f_stat.st_mode & S_IXUSR;
#endif
}

#if TARGET_PLATFORM_WINDOWS
string KT_File::getExePath() {
  char exeFullPath[MAX_PATH];  // Full path
  LPWSTR Wstr = ConvertCharToLPWSTR(exeFullPath);
  //函数返回当前进程已加载可执行或DLL文件的完整路径名(以'\0'终止)
  GetModuleFileName(NULL, Wstr, MAX_PATH);
  return exeFullPath;  // Get full path of the file
}
LPWSTR KT_File::ConvertCharToLPWSTR(const char *szString) {
  int dwLen = strlen(szString) + 1;
  int nwLen = MultiByteToWideChar(CP_ACP, 0, szString, dwLen, NULL, 0);  //算出合适的长度
  LPWSTR lpszPath = new WCHAR[dwLen];
  MultiByteToWideChar(CP_ACP, 0, szString, dwLen, lpszPath, nwLen);
  return lpszPath;
}
#elif TARGET_PLATFORM_IOS
string KT_File::getExePath() {
  int numberOfProcesses = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
  pid_t pids[numberOfProcesses];
  bzero(pids, sizeof(pids));
  proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
  char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
  bzero(pathBuffer, PROC_PIDPATHINFO_MAXSIZE);
  for (int i = 0; i < numberOfProcesses; ++i) {
    if (pids[i] == 0) {
      continue;
    }

    if (pids[i] == getpid()) {
      proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer));
      break;
    }
  }
  return pathBuffer;
}

#else
string KT_File::getExePath() {
  string proc = "/proc/self/exe";
  char buf[2048] = "\0";

  int bufsize = sizeof(buf) / sizeof(char);

  int count = readlink(proc.c_str(), buf, bufsize);

  if (count < 0) {
    THROW_EXCEPTION_SYSCODE(KT_File_Exception, "[KT_File::getExePath] could not get exe path error");
    // throw KT_File_Exception("[KT_File::getExePath] could not get exe path error", TC_Exception::getSystemCode());
  }

  count = (count >= bufsize) ? (bufsize - 1) : count;

  buf[count] = '\0';
  return buf;
}
#endif

bool KT_File::makeDir(const string &sDirectoryPath) {
  int iRetCode = KT_Port::mkdir(sDirectoryPath.c_str());

  if (iRetCode < 0 && errno == EEXIST) {
    return isFileExistEx(sDirectoryPath, S_IFDIR);
  }
  return iRetCode == 0;
}

bool KT_File::makeDirRecursive(const string &sDirectoryPath) {
  string simple = simplifyDirectory(sDirectoryPath);

  string::size_type pos = 0;
  for (; pos != string::npos;) {
    pos = simple.find(FILE_SEP, pos + 1);
    string s;
    if (pos == string::npos) {
      s = simple.substr(0, simple.size());
#if TARGET_PLATFORM_WINDOWS
      if (isPanfu(s)) {
        return false;
      }
#endif
      return makeDir(s.c_str());
    } else {
      s = simple.substr(0, pos);
#if TARGET_PLATFORM_WINDOWS
      if (isPanfu(s)) {
        continue;
      }
#endif
      if (!makeDir(s.c_str())) return false;
    }
  }
  return true;
}

int KT_File::removeFile(const string &sFullFileName, bool bRecursive) {
  string path = simplifyDirectory(sFullFileName);

  if (isFileExist(path, S_IFDIR)) {
    if (bRecursive) {
      vector<string> files;
      listDirectory(path, files, false);
      for (size_t i = 0; i < files.size(); i++) {
        removeFile(files[i], bRecursive);
      }

      if (path != FILE_SEP) {
        if (KT_Port::rmdir(path.c_str()) == -1) {
          return -1;
        }
        return 0;
      }
    } else {
      if (KT_Port::rmdir(path.c_str()) == -1) {
        return -1;
      }
    }
  } else {
    if (::remove(path.c_str()) == -1) {
      return -1;
    }
  }

  return 0;
}

int KT_File::renameFile(const string &sSrcFullFileName, const string &sDstFullFileName) {
  return rename(sSrcFullFileName.c_str(), sDstFullFileName.c_str());
}

string KT_File::simplifyDirectory(const string &path) {
  string result = path;

#if TARGET_PLATFORM_WINDOWS
  result = KT_Common::replace(result, "/", "\\");
#else
  result = KT_Common::replace(result, "\\", "/");
#endif

  string sep(FILE_SEP);
  string::size_type pos;

  pos = 0;
  while ((pos = result.find(sep + FILE_SEP, pos)) != string::npos) {
    result.erase(pos, 1);
  }

  pos = 0;
  while ((pos = result.find(sep + "." + FILE_SEP, pos)) != string::npos) {
    result.erase(pos, 2);
  }

  while (result.substr(0, 4) == sep + ".." + FILE_SEP) {
    result.erase(0, 3);
  }

  if (result.find(sep + ".." + FILE_SEP) != string::npos) {
    bool ab = KT_File::isAbsolute(result);

    vector<string> dirs = KT_Common::sepstr<string>(result, FILE_SEP);
    stack<string> q;
    for (size_t i = 0; i < dirs.size(); i++) {
      if (dirs[i] == ".." && !q.empty()) {
        if (!KT_File::startWindowsPanfu(q.top()) && q.top() != ".." && q.top() != ".")
          q.pop();
        else {
          q.push(dirs[i]);
        }
      } else {
        q.push(dirs[i]);
      }
    }

    result = "";

    while (!q.empty()) {
      result = q.top() + FILE_SEP + result;
      q.pop();
    }

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
    if (ab) {
      result = FILE_SEP + result;
    }
#endif
  }

  if (result == sep + ".") {
    return result.substr(0, result.size() - 1);
  }

  if (result.size() >= 2 && result.substr(result.size() - 2, 2) == sep + ".") {
    result.erase(result.size() - 2, 2);
  }

  if (result == FILE_SEP) {
    return result;
  }

  if (result.size() >= 1 && result[result.size() - 1] == FILE_SEP[0]) {
    result.erase(result.size() - 1);
  }

  if (result == sep + "..") {
    result = FILE_SEP;
  }

  return result;
}

string KT_File::load2str(const string &sFullFileName) {
  FILE *fd = KT_Port::fopen(sFullFileName.data(), "rb");
  if (fd == NULL) return "";
  string s;
  int nread = -1;
  do {
    char buf[8096] = {'\0'};
    nread = fread(buf, 1, sizeof(buf), fd);
    if (nread > 0) {
      s.append(buf, nread);
    }
  } while (nread > 0);
  fclose(fd);
  return s;
}

bool KT_File::load2str(const string &sFullFileName, vector<char> &data) {
  FILE *fd = KT_Port::fopen(sFullFileName.data(), "rb");
  if (fd == NULL) return false;

  int nread = -1;
  do {
    char buf[8096] = {'\0'};
    nread = fread(buf, 1, sizeof(buf), fd);
    if (nread > 0) {
      data.insert(data.end(), buf, buf + nread);
    }
  } while (nread > 0);
  fclose(fd);
  return true;
}

void KT_File::save2file(const string &sFullFileName, const string &sFileData) {
  save2file(sFullFileName, sFileData.c_str(), sFileData.length());
}

int KT_File::save2file(const string &sFullFileName, const char *sFileData, size_t length) {
  FILE *fp = KT_Port::fopen(sFullFileName.c_str(), "wb");
  if (fp == NULL) {
    return -1;
  }

  size_t ret = fwrite((void *)sFileData, 1, length, fp);
  fclose(fp);

  if (ret == length) {
    return 0;
  }
  return -1;
}

string KT_File::extractFileName(const string &sFullFileName) {
  if (sFullFileName.length() <= 0) {
    return "";
  }

  string::size_type found = sFullFileName.find_last_of("/\\");
  // string::size_type pos = sFullFileName.rfind(FILE_SEP);
  if (found == string::npos) {
    return sFullFileName;
  }

  return sFullFileName.substr(found + 1);
}

string KT_File::extractFilePath(const string &sFullFileName) {
  // #if TARGET_PLATFORM_WINDOWS
  //     string sFullFileNameTmp = KT_Common::replace(sFullFileName, "/", "\\");
  // #else
  //     string sFullFileNameTmp = KT_Common::replace(sFullFileName, "\\", "/");
  // #endif

  if (sFullFileName.length() <= 0) {
    return string(".") + FILE_SEP;
  }

  string::size_type found = sFullFileName.find_last_of("/\\");
  if (found == string::npos) {
    return string(".") + FILE_SEP;
  }

  return sFullFileName.substr(0, found + 1);

  // return string(".") + FILE_SEP;
}

string KT_File::extractFileExt(const string &sFullFileName) {
  string::size_type found = sFullFileName.find_last_of("/\\");
  if (found == string::npos) {
    if ((found = sFullFileName.rfind('.')) == string::npos) {
      return string("");
    }

    return sFullFileName.substr(found + 1);
  } else {
    for (string::size_type i = sFullFileName.size() - 1; i > found; i--) {
      if (sFullFileName[i] == '.') {
        return sFullFileName.substr(i + 1);
      }
    }

    return "";
  }
}

string KT_File::excludeFileExt(const string &sFullFileName) {
  string::size_type found = sFullFileName.find_last_of("./\\");
  if (found != string::npos) {
    if (sFullFileName[found] == '.') {
      return sFullFileName.substr(0, found);
    }
  }

  return sFullFileName;
  // return sFullFileName.substr(0, pos);
}

string KT_File::replaceFileExt(const string &sFullFileName, const string &sExt) {
  return excludeFileExt(sFullFileName) + "." + sExt;
}

string KT_File::extractUrlFilePath(const string &sUrl) {
  string sLowerUrl = KT_Common::lower(sUrl);
  string::size_type pos = sLowerUrl.find("http://");

  if (pos == 0) {
    pos += strlen("http://");
  } else if (pos == string::npos) {
    pos = 0;
  }

  for (; pos < sUrl.length(); ++pos) {
    if (sUrl[pos] == '/') {
      if (pos < sUrl.length() - 1) {
        pos++;
        break;
      } else {
        return "";
      }
    }
  }

  if (pos == string::npos || pos == sUrl.length()) {
    pos = 0;
  }

  return sUrl.substr(pos);
}

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
size_t KT_File::scanDir(const string &sFilePath, vector<string> &vtMatchFiles, FILE_SELECT f, int iMaxSize) {
  vtMatchFiles.clear();

  struct dirent **namelist;
  int n = scandir(sFilePath.c_str(), &namelist, f, alphasort);

  if (n < 0) {
    return 0;
  } else {
    while (n--) {
      if (iMaxSize > 0 && vtMatchFiles.size() >= (size_t)iMaxSize) {
        free(namelist[n]);
        break;
      } else {
        vtMatchFiles.push_back(namelist[n]->d_name);
        free(namelist[n]);
      }
    }
    free(namelist);
  }

  return vtMatchFiles.size();
}
#endif

void KT_File::listDirectory(const string &path, vector<string> &files, bool bRecursive) {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  vector<string> tf;
  scanDir(path, tf, 0, 0);

  for (size_t i = 0; i < tf.size(); i++) {
    if (tf[i] == "." || tf[i] == "..") continue;

    string s = path + FILE_SEP + tf[i];

    if (isFileExist(s, S_IFDIR)) {
      files.push_back(simplifyDirectory(s));
      if (bRecursive) {
        listDirectory(s, files, bRecursive);
      }
    } else {
      files.push_back(simplifyDirectory(s));
    }
  }
#elif TARGET_PLATFORM_WINDOWS
  intptr_t hFile;
  _finddata_t fileinfo;
  if ((hFile = _findfirst(string(path + "\\*.*").c_str(), &fileinfo)) != -1) {
    do {
      string sName = fileinfo.name;
      if (sName == "." || sName == "..") continue;

      string s = path + FILE_SEP + sName;

      if (fileinfo.attrib & _A_SUBDIR) {
        files.push_back(simplifyDirectory(s));
        if (bRecursive) {
          listDirectory(s, files, bRecursive);
        }
      } else {
        files.push_back(simplifyDirectory(s));
      }
    } while (_findnext(hFile, &fileinfo) == 0);
    _findclose(hFile);
  }
#endif
}

void KT_File::copyFile(const string &sExistFile, const string &sNewFile, bool bRemove) {
  if (KT_File::isFileExist(sExistFile, S_IFDIR)) {
    KT_File::makeDir(sNewFile);
    vector<string> tf;
    KT_File::listDirectory(sExistFile, tf, false);
    for (size_t i = 0; i < tf.size(); i++) {
      string fileName = KT_File::extractFileName(tf[i]);
      if (fileName == "." || fileName == "..") continue;
      string s = sExistFile + FILE_SEP + fileName;
      string d = sNewFile + FILE_SEP + fileName;
      copyFile(s, d, bRemove);
    }
  } else {
    if (bRemove) std::remove(sNewFile.c_str());

    std::ifstream fin(sExistFile.c_str(), ios::binary);
    if (!fin) {
      THROW_EXCEPTION_SYSCODE(KT_File_Exception, "[KT_File::copyFile] error: " + sExistFile);
    }
    std::ofstream fout(sNewFile.c_str(), ios::binary);
    if (!fout) {
      THROW_EXCEPTION_SYSCODE(KT_File_Exception, "[KT_File::copyFile] error: " + sNewFile);
    }

    fout << fin.rdbuf();
    fin.close();
    fout.close();

    KT_Port::stat_t f_stat;
    if (KT_Port::lstat(sExistFile.c_str(), &f_stat) == -1) {
      THROW_EXCEPTION_SYSCODE(KT_File_Exception, "[KT_File::copyFile] error: " + sExistFile);
    }

    KT_Port::chmod(sNewFile.c_str(), f_stat.st_mode);
  }
}

bool KT_File::startWindowsPanfu(const string &sPath) {
  if (sPath.length() < 2) {
    return false;
  }

  char c = sPath[0];

  return isalpha(c) && (sPath[1] == ':');
}

bool KT_File::isPanfu(const string &sPath) {
  if (sPath.length() != 2) {
    return false;
  }

  char c = sPath[0];

  return isalpha(c) && (sPath[1] == ':');
}
}  // namespace kant
