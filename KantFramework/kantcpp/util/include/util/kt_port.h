#ifndef _KT_PORT_H_
#define _KT_PORT_H_

#include "util/kt_ex.h"
#include "util/kt_platform.h"

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS

#include <strings.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#else

#include <direct.h>
#include <io.h>

typedef unsigned short mode_t;

#define S_IFREG _S_IFREG  //��ʾΪ��ͨ�ļ���Ϊ�˿�ƽ̨��һ��ʹ��S_IFREG
#define S_IFDIR _S_IFDIR  //��ʾΪĿ¼��Ϊ�˿�ƽ̨��һ��ʹ��S_IFDIR

#endif

#include <stdio.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace kant {

/**
 * @brief ��ƽ̨port�쳣��
 */
struct KT_Port_Exception : public KT_Exception {
  KT_Port_Exception(const string &buffer) : KT_Exception(buffer){};
  ~KT_Port_Exception() throw(){};
};

class KT_Port {
 public:
  /**
   * @brief ��s1�ĳ���n������s2
   * @return ��������ָ��, �Ҳ�������NULL
   */
  static const char *strnstr(const char *s1, const char *s2, int pos1);

  static int strcmp(const char *s1, const char *s2);

  static int strncmp(const char *s1, const char *s2, size_t n);

  static int strcasecmp(const char *s1, const char *s2);

  static int strncasecmp(const char *s1, const char *s2, size_t n);

  static void localtime_r(const time_t *clock, struct tm *result);

  static void gmtime_r(const time_t *clock, struct tm *result);

  static time_t timegm(struct tm *timeptr);

  static int gettimeofday(struct timeval &tv);

  static int chmod(const char *path, mode_t mode);

  static FILE *fopen(const char *path, const char *mode);

#if TARGET_PLATFORM_WINDOWS
  typedef struct _stat stat_t;
#else
  typedef struct stat stat_t;
#endif
  static int lstat(const char *path, stat_t *buf);

  static int mkdir(const char *path);

  static int rmdir(const char *path);

  static int closeSocket(int fd);

  static int64_t getpid();

  /**
   * ��ȡ��������
   * @param name
   * @return
   */
  static std::string getEnv(const std::string &name);

  /**
   * ���û�������
   * @param name
   * @param value
   */
  static void setEnv(const std::string &name, const std::string &value);

  /**
   * ����һ���ű�
   * @param cmd
   * @param err
   * @return ����ı�׼���(���2k���������)
   */
  static std::string exec(const char *cmd);

  /**
   * ����һ���ű�(����+������)
   * @param cmd
   * @param err
   * @return: ����ı�׼���(���2k���������)
   */
  static std::string exec(const char *cmd, std::string &err);

  /**
   * fork�ӽ��̲����г���
   * @param sExe: ��ִ�г���·��
   * @param sPwdPath: �������еĵ�ǰ·��
   * @param sRollLogPath: ������־·��(stdout���ض��򵽹�����־), Ϊ�����ض���
   * @param vOptions: ����
   * @return �ӽ���id: ==0: �ӽ�����, >0: ��������(�ӽ���pid), �����׳��쳣
   * KT_Port_Exception
   */
  static int64_t forkExec(const string &sExe, const string &sPwdPath,
                          const string &sRollLogPath,
                          const vector<string> &vOptions);

  /**
   * ע��ctrl+c�ص��¼�(SIGINT/CTRL_C_EVENT)
   * @param callback
   * @return size_t, ע���¼���id, ȡ��ע��ʱ��Ҫ
   */
  static size_t registerCtrlC(std::function<void()> callback);

  /**
   * ȡ��ע��ctrl+c�ص��¼�
   * @param callback
   * @return
   */
  static void unregisterCtrlC(size_t id);

  /**
   * ע��term�¼��Ļص�(SIGTERM/CTRL_SHUTDOWN_EVENT)
   * @param callback
   * @return size_t, ע���¼���id, ȡ��ע��ʱ��Ҫ
   */
  static size_t registerTerm(std::function<void()> callback);

  /**
   * ȡ��ע��
   * @param id
   */
  static void unregisterTerm(size_t id);

 protected:
  static size_t registerSig(int sig, std::function<void()> callback);
  static void unregisterSig(int sig, size_t id);
  static void registerSig(int sig);

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  static void sighandler(int sig_no);
#else
  static BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);
#endif

  struct SigInfo {
    std::mutex _mutex;

    unordered_map<int, unordered_map<size_t, std::function<void()>>> _callbacks;

    std::atomic<size_t> _callbackId{0};
  };

  static shared_ptr<SigInfo> _sigInfo;
};

}  // namespace kant

#endif
