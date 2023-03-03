/**
 * Tencent is pleased to support the open source community by making Tars
 * available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the
 * License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include <string.h>

#include <thread>

#include "util/kt_common.h"
#include "util/kt_file.h"
#include "util/kt_logger.h"
#include "util/kt_platform.h"
#include "util/kt_port.h"

#if TARGET_PLATFORM_LINUX
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#endif

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
#include <limits.h>
#include <signal.h>
#include <sys/time.h>
#else

#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
// #include <winsock.h>
#include <sys/timeb.h>
#include <time.h>

#include "util/kt_strptime.h"
#endif

namespace kant {

int KT_Port::strcmp(const char *s1, const char *s2) {
#if TARGET_PLATFORM_WINDOWS
  return ::strcmp(s1, s2);
#else
  return ::strcmp(s1, s2);
#endif
}

int KT_Port::strncmp(const char *s1, const char *s2, size_t n) {
#if TARGET_PLATFORM_WINDOWS
  return ::strncmp(s1, s2, n);
#else
  return ::strncmp(s1, s2, n);
#endif
}

const char *KT_Port::strnstr(const char *s1, const char *s2, int pos1) {
  int l1 = 0;
  int l2;

  l2 = strlen(s2);
  if (!l2) return (char *)s1;

  const char *p = s1;
  while (l1 < pos1 && *p++ != '\0') {
    ++l1;
  }
  // l1 = strlen(s1);

  // pos1 = (pos1 > l1)?l1:pos1;

  while (pos1 >= l2) {
    pos1--;
    if (!memcmp(s1, s2, l2)) return s1;
    s1++;
  }
  return NULL;
}

int KT_Port::strcasecmp(const char *s1, const char *s2) {
#if TARGET_PLATFORM_WINDOWS
  return ::_stricmp(s1, s2);
#else
  return ::strcasecmp(s1, s2);
#endif
}

int KT_Port::strncasecmp(const char *s1, const char *s2, size_t n) {
#if TARGET_PLATFORM_WINDOWS
  return ::_strnicmp(s1, s2, n);
#else
  return ::strncasecmp(s1, s2, n);
#endif
}

void KT_Port::localtime_r(const time_t *clock, struct tm *result) {
  //��ʱ��ʱ��
#if TARGET_PLATFORM_WINDOWS
  ::localtime_s(result, clock);
#else
  ::localtime_r(clock, result);
#endif
}

void KT_Port::gmtime_r(const time_t *clock, struct tm *result) {
#if TARGET_PLATFORM_WINDOWS
  ::gmtime_s(result, clock);
#else
  ::gmtime_r(clock, result);
#endif
}

time_t KT_Port::timegm(struct tm *timeptr) {
#if TARGET_PLATFORM_WINDOWS
  return ::_mkgmtime(timeptr);
#else
  return ::timegm(timeptr);
#endif
}

int KT_Port::gettimeofday(struct timeval &tv) {
#if TARGET_PLATFORM_WINDOWS
  static const DWORDLONG FILETIME_to_timeval_skew = 116444736000000000;
  FILETIME tfile;
  ::GetSystemTimeAsFileTime(&tfile);

  ULARGE_INTEGER tmp;
  tmp.LowPart = tfile.dwLowDateTime;
  tmp.HighPart = tfile.dwHighDateTime;
  tmp.QuadPart -= FILETIME_to_timeval_skew;

  ULARGE_INTEGER largeInt;
  largeInt.QuadPart = tmp.QuadPart / (10000 * 1000);
  tv.tv_sec = (long)(tmp.QuadPart / (10000 * 1000));
  tv.tv_usec = (long)((tmp.QuadPart % (10000 * 1000)) / 10);
  return 0;
#else
  return ::gettimeofday(&tv, 0);
#endif
}

int KT_Port::chmod(const char *path, mode_t mode) {
  //��ʱ��ʱ��
#if TARGET_PLATFORM_WINDOWS
  return ::_chmod(path, mode);
#else
  return ::chmod(path, mode);
#endif
}

FILE *KT_Port::fopen(const char *path, const char *mode) {
#if TARGET_PLATFORM_WINDOWS
  FILE *fp;
  if (fopen_s(&fp, path, mode) != 0) {
    return NULL;
  }

  return fp;
#else
  return ::fopen(path, mode);
#endif
}

int KT_Port::lstat(const char *path, KT_Port::stat_t *buf) {
#if TARGET_PLATFORM_WINDOWS
  return ::_stat(path, buf);
#else
  return ::lstat(path, buf);
#endif
}

int KT_Port::mkdir(const char *path) {
#if TARGET_PLATFORM_WINDOWS
  int iRetCode = ::_mkdir(path);
#else
  int iRetCode = ::mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
  return iRetCode;
}

int KT_Port::rmdir(const char *path) {
#if TARGET_PLATFORM_WINDOWS
  return ::_rmdir(path);
#else
  return ::rmdir(path);
#endif
}

int KT_Port::closeSocket(int fd) {
#if TARGET_PLATFORM_WINDOWS
  return ::closesocket(fd);
#else
  return ::close(fd);
#endif
}

int64_t KT_Port::getpid() {
#if TARGET_PLATFORM_WINDOWS
  int64_t pid = ::GetCurrentProcessId();
#else
  int64_t pid = ::getpid();
#endif
  return pid;
}

string KT_Port::getEnv(const string &name) {
  char *p = getenv(name.c_str());
  string str = p ? string(p) : "";

  return str;
}

void KT_Port::setEnv(const string &name, const string &value) {
#if TARGET_PLATFORM_WINDOWS
  SetEnvironmentVariable((LPCTSTR)name.c_str(), (LPCTSTR)value.c_str());
#else
  setenv(name.c_str(), value.c_str(), true);
#endif
}

string KT_Port::exec(const char *cmd) {
  string err;
  return exec(cmd, err);
}

std::string KT_Port::exec(const char *cmd, std::string &err) {
  string fileData;
#if TARGET_PLATFORM_WINDOWS
  FILE *fp = _popen(cmd, "r");
#else
  FILE *fp = popen(cmd, "r");
#endif
  if (fp == NULL) {
    err = "open '" + string(cmd) + "' error";
    return "";
  }
  static size_t buf_len = 2 * 1024 * 1024;
  char *buf = new char[buf_len];
  memset(buf, 0, buf_len);
  fread(buf, sizeof(char), buf_len - 1, fp);
#if TARGET_PLATFORM_WINDOWS
  _pclose(fp);
#else
  pclose(fp);
#endif
  fileData = string(buf);
  delete[] buf;

  return fileData;
}

int64_t KT_Port::forkExec(const string &sExePath, const string &sPwdPath, const string &sRollLogPath,
                          const vector<string> &vOptions) {
  vector<string> vEnvs;

  if (sExePath.empty()) {
    throw KT_Port_Exception("[KT_Port::forkExec] server exe: " + sExePath + " is empty.");
  }

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  if (TC_File::isFileExistEx(sExePath) && !TC_File::canExecutable(sExePath)) {
    TC_File::setExecutable(sExePath, true);
  }
#endif

  //
  // Current directory
  //
  const char *pwdCStr = sPwdPath.c_str();
  //LPWSTR Wstr = KT_File::ConvertCharToLPWSTR(pwdCStr);

#if TARGET_PLATFORM_WINDOWS
  vector<string> vArgs;

  vArgs.insert(vArgs.end(), vOptions.begin(), vOptions.end());

  string path;
  for (vector<string>::const_iterator p = vArgs.begin(); p != vArgs.end(); ++p) {
    path += " " + *p;
  }

  string command = sExePath + " " + path;

  TCHAR p[1024];
  memcpy(p, command.c_str(), (command.length() + 1) * sizeof(TCHAR));
  //strncpy_s(p, sizeof(p) / sizeof(TCHAR), command.c_str(), );

  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE;  // TRUE��ʾ��ʾ�����Ľ��̵Ĵ���

  if (!CreateProcess(NULL,  //  ָ��һ��NULL��β�ġ�����ָ����ִ��ģ��Ŀ��ֽ��ַ���
                     p,     // �������ַ���
                     NULL,  //    ָ��һ��SECURITY_ATTRIBUTES�ṹ�壬����ṹ������Ƿ񷵻صľ�����Ա��ӽ��̼̳С�
                     NULL,  //    ���lpProcessAttributes����Ϊ�գ�NULL������ô������ܱ��̳С�<ͬ��>
                     false,  //    ָʾ�½����Ƿ�ӵ��ý��̴��̳��˾����
                     CREATE_NEW_CONSOLE | CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS |
                       CREATE_NO_WINDOW,  //  ָ�����ӵġ���������������ͽ��̵Ĵ����ı�
                     NULL,  //    ָ��һ���½��̵Ļ����顣����˲���Ϊ�գ��½���ʹ�õ��ý��̵Ļ���
                     (LPCWSTR)pwdCStr,  //    ָ���ӽ��̵Ĺ���·��
                     &si,               // �����½��̵������������ʾ��STARTUPINFO�ṹ��
                     &pi                // �����½��̵�ʶ����Ϣ��PROCESS_INFORMATION�ṹ��
                     )) {
    string err = KT_Exception::parseError(KT_Exception::getSystemCode());

    throw KT_Port_Exception("[KT_Port::forkExec] CreateProcessA exception:" + err);
  }

  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  return pi.dwProcessId;

#else
  vector<string> vArgs;
  vArgs.push_back(sExePath);
  vArgs.insert(vArgs.end(), vOptions.begin(), vOptions.end());

  int argc = static_cast<int>(vArgs.size());
  char **argv = static_cast<char **>(malloc((argc + 1) * sizeof(char *)));
  int i = 0;
  for (vector<string>::const_iterator p = vArgs.begin(); p != vArgs.end(); ++p, ++i) {
    assert(i < argc);
    argv[i] = strdup(p->c_str());
  }
  assert(i == argc);
  argv[argc] = 0;

  pid_t pid = fork();
  if (pid == -1) {
    throw KT_Port_Exception("[KT_Port::forkExec] fork exception");
  }

  if (pid == 0) {
    int maxFd = static_cast<int>(sysconf(_SC_OPEN_MAX));
    for (int fd = 3; fd < maxFd; ++fd) {
      close(fd);
    }

    // server stdcout ��־�ڹ�����־��ʾ
    if (!sRollLogPath.empty()) {
      TC_File::makeDirRecursive(TC_File::extractFilePath(sRollLogPath));
#if TARGET_PLATFORM_IOS
      if ((freopen(sRollLogPath.c_str(), "ab", stdout)) != NULL &&
          (freopen(sRollLogPath.c_str(), "ab", stderr)) != NULL)
#else
      if ((freopen64(sRollLogPath.c_str(), "ab", stdout)) != NULL &&
          (freopen64(sRollLogPath.c_str(), "ab", stderr)) != NULL)
#endif
      {
        cout << argv[0] << " redirect stdout and stderr  to " << sRollLogPath << endl;
      } else {
        //�ض���ʧ�� ֱ���˳�
        exit(0);
      }
    } else {
      cout << argv[0]
           << " cannot redirect stdout and stderr  to log file sRollLogPath is "
              "empty"
           << endl;
    }

    //		for_each(vEnvs.begin(), vEnvs.end(), EnvVal());

    if (strlen(pwdCStr) != 0) {
      if (chdir(pwdCStr) == -1) {
        cerr << argv[0] << " cannot change working directory to " << pwdCStr << "|errno=" << errno << endl;
      }
    }

    if (execvp(argv[0], argv) == -1) {
      cerr << "cannot execute " << argv[0] << "|errno=" << strerror(errno) << endl;
    }
    exit(0);
  } else {
    for (i = 0; argv[i]; i++) {
      free(argv[i]);
    }
    free(argv);
  }
  return pid;
#endif
}

shared_ptr<KT_Port::SigInfo> KT_Port::_sigInfo = std::make_shared<KT_Port::SigInfo>();

size_t KT_Port::registerSig(int sig, std::function<void()> callback) {
  std::lock_guard<std::mutex> lock(_sigInfo->_mutex);

  auto it = _sigInfo->_callbacks.find(sig);

  if (it == _sigInfo->_callbacks.end()) {
    //û��ע���, ��ע��
    registerSig(sig);
  }

  size_t id = ++_sigInfo->_callbackId;

  _sigInfo->_callbacks[sig][id] = callback;

  return id;
}

void KT_Port::unregisterSig(int sig, size_t id) {
  //ע��_sigInfo��ȫ�־�̬��, �п����Ѿ�������, ��Ҫ�����ж�һ��!
  if (_sigInfo && _sigInfo.use_count() > 0) {
    std::lock_guard<std::mutex> lock(_sigInfo->_mutex);
    auto it = _sigInfo->_callbacks.find(sig);

    if (it != _sigInfo->_callbacks.end()) {
      it->second.erase(id);
    }
  }
}

size_t KT_Port::registerCtrlC(std::function<void()> callback) {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  return registerSig(SIGINT, callback);
#else
  return registerSig(CTRL_C_EVENT, callback);
#endif
}

void KT_Port::unregisterCtrlC(size_t id) {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  unregisterSig(SIGINT, id);
#else
  unregisterSig(CTRL_C_EVENT, id);
#endif
}

size_t KT_Port::registerTerm(std::function<void()> callback) {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  return registerSig(SIGTERM, callback);
#else
  return registerSig(CTRL_SHUTDOWN_EVENT, callback);
#endif
}

void KT_Port::unregisterTerm(size_t id) {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  unregisterSig(SIGTERM, id);
#else
  unregisterSig(CTRL_SHUTDOWN_EVENT, id);
#endif
}

void KT_Port::registerSig(int sig) {
#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  signal(sig, KT_Port::sighandler);
#else
  SetConsoleCtrlHandler(KT_Port::HandlerRoutine, TRUE);
#endif
}

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
void KT_Port::sighandler(int sig_no) {
  std::thread th([=]() {
    unordered_map<size_t, std::function<void()>> data;

    {
      std::lock_guard<std::mutex> lock(_sigInfo->_mutex);

      auto it = KT_Port::_sigInfo->_callbacks.find(sig_no);
      if (it != KT_Port::_sigInfo->_callbacks.end()) {
        data = it->second;
      }
    }

    for (auto f : data) {
      try {
        f.second();
      } catch (...) {
      }
    }
  });
  th.detach();
}
#else
BOOL WINAPI KT_Port::HandlerRoutine(DWORD dwCtrlType) {
  std::thread th([=]() {
    unordered_map<size_t, std::function<void()>> data;

    {
      std::lock_guard<std::mutex> lock(_sigInfo->_mutex);

      auto it = _sigInfo->_callbacks.find(dwCtrlType);
      if (it != _sigInfo->_callbacks.end()) {
        data = it->second;
      }
    }

    for (auto f : data) {
      try {
        f.second();
      } catch (...) {
      }
    }
  });
  th.detach();
  return TRUE;
}
#endif

}  // namespace kant
