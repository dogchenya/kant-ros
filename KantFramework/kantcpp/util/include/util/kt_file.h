/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#ifndef _KT_FILE_H_
#define _KT_FILE_H_

#include <sys/stat.h>
#include <sys/types.h>

#include "util/kt_platform.h"
#include "util/kt_port.h"

#include <iostream>
#include <fstream>

#include "util/kt_ex.h"
#include "util/kt_common.h"

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
#define FILE_SEP "/"
#else
#define FILE_SEP "\\"
#endif

namespace kant {
/////////////////////////////////////////////////
/** 
 * @file kt_file.h 
 * @brief  �ļ�������. 
 *         
 */
/////////////////////////////////////////////////

/**
* @brief �ļ��쳣��. 
*  
*/
struct KT_File_Exception : public KT_Exception {
  KT_File_Exception(const string &buffer) : KT_Exception(buffer){};
  KT_File_Exception(const string &buffer, int err) : KT_Exception(buffer, err){};
  ~KT_File_Exception() throw(){};
};

/**
* @brief �����ļ���������.
*  
*/
class KT_File {
 public:
  /**
	* @brief ��ȡ�ļ���С, ����ļ�������, �򷵻�-1.
	*
	* @param  sFullFileName �ļ�ȫ·��(����Ŀ¼���ļ���)
	* @return               ofstream::pos_type�����ļ���С
	*/
  static ifstream::pos_type getFileSize(const string &sFullFileName);

  /**
	 * @brief �ж��Ƿ�Ϊ����·��, ���Կո���'/'��ͷ.
	 *
	 * @param sFullFileName �ļ�ȫ·��(����Ŀ¼���ļ���)
	 * @return              ture�Ǿ���·����false����Ǿ���·��
	 */
  static bool isAbsolute(const string &sFullFileName);

  /**
	* @brief �жϸ���·�����ļ��Ƿ����.
	* ����ļ��Ƿ�������,���Է��������ж϶������Է�������ָ����ļ��ж�
	* @param sFullFileName �ļ�ȫ·��
	* @param iFileType     �ļ�����, ȱʡS_IFREG
	* @return           true������ڣ�fals��������
	*/
  static bool isFileExist(const string &sFullFileName, mode_t iFileType = S_IFREG);

  /**
	* @brief �жϸ���·�����ļ��Ƿ����.
	* ע��: ����ļ��Ƿ�������,���Է�������ָ����ļ��ж�
	* @param sFullFileName  �ļ�ȫ·��
	* @param iFileType      �ļ�����, ȱʡS_IFREG
	* @return               true-���ڣ�fals-������
	*/
  static bool isFileExistEx(const string &sFullFileName, mode_t iFileType = S_IFREG);

  /**
	 * @brief ����Ŀ¼����, ��һЩ���õ�ȥ��, ����./��.
	 *
	 * @param path Ŀ¼����
	 * @return        �淶���Ŀ¼����
	 */
  static string simplifyDirectory(const string &path);

  /**
	* @brief ����Ŀ¼, ���Ŀ¼�Ѿ�����, ��Ҳ���سɹ�.
	*
	* @param sFullPath Ҫ������Ŀ¼����
	* @return bool  true-�����ɹ� ��false-����ʧ��
	*/
  static bool makeDir(const string &sDirectoryPath);

  /**
	 *@brief ѭ������Ŀ¼, ���Ŀ¼�Ѿ�����, ��Ҳ���سɹ�.
	 *
	 * @param sFullPath Ҫ������Ŀ¼����
	 * @return           true-�����ɹ���false-����ʧ��
	 */

  static bool makeDirRecursive(const string &sDirectoryPath);

  /**
	 * @brief ɾ��һ���ļ���Ŀ¼.
	 *
	 * @param sFullFileName �ļ�����Ŀ¼��ȫ·��
	 * @param bRecursive    �����Ŀ¼�Ƿ�ݹ�ɾ��
	 * @return              0-�ɹ���ʧ�ܿ���ͨ��errno�鿴ʧ�ܵ�ԭ��
	 */
  static int removeFile(const string &sFullFileName, bool bRecursive);

  /**
	 * @brief ������һ���ļ���Ŀ¼.
	 *
	 * @param sSrcFullFileName Դ�ļ���
	 * @param sDstFullFileName Ŀ���ļ���
	 * @return              0-�ɹ���ʧ�ܿ���ͨ��errno�鿴ʧ�ܵ�ԭ��
	 */
  static int renameFile(const string &sSrcFullFileName, const string &sDstFullFileName);

  /**
	* @brief ��ȡ�ļ���string
	* �ļ������򷵻��ļ����ݣ������ڻ��߶�ȡ�ļ������ʱ��, ����Ϊ��
	* @param sFullFileName �ļ�����
	* @return              �ļ�����
	*/
  static string load2str(const string &sFullFileName);
  static bool load2str(const string &sFullFileName, vector<char> &data);

  /**
	* @brief д�ļ�.
	*
	* @param sFullFileName �ļ�����
	* @param sFileData     �ļ�����
	* @return
	*/
  static void save2file(const string &sFullFileName, const string &sFileData);

  /**
	 * @brief д�ļ�.
	 *
	 * @param sFullFileName  �ļ���
	 * @param sFileData      ����ָ��
	 * @param length      д�볤��
	 * @return               0-�ɹ�,-1-ʧ��
	 */
  static int save2file(const string &sFullFileName, const char *sFileData, size_t length);

  /**
     * @brief �����ļ��Ƿ��ִ��. 
     *  
     * @param sFullFileName �ļ�ȫ·��
     * @param canExecutable true��ʾ��ִ��, false������֮�� 
     * @return                 �ɹ�����0, ����ʧ��
     */
  static int setExecutable(const string &sFullFileName, bool canExecutable);

  /**
     * @brief �ж��ļ��Ƿ��ִ��. 
     *  
     * @param sFullFileName �ļ�ȫ·��
     * @return                 true-��ִ��, false-����ִ�� 
     */
  static bool canExecutable(const string &sFullFileName);

  /**
     * @brief ��ȡǰ����ִ���ļ�·��.
     *
     * @return string ��ִ���ļ���·��ȫ����
     */
  static string getExePath();
#if TARGET_PLATFORM_WINDOWS
  static LPWSTR ConvertCharToLPWSTR(const char *szString);
#endif

  /**
	* @brief ��ȡ�ļ�����
	*��һ����ȫ�ļ�����ȥ��·��������:/usr/local/temp.gif��ȡtemp.gif
	*@param sFullFileName  �ļ�����ȫ����
	*@return string        ��ȡ����ļ�����
	*/
  static string extractFileName(const string &sFullFileName);

  /**
	* @brief ��һ����ȫ�ļ�������ȡ�ļ���·��.
	*
	* ����1: "/usr/local/temp.gif" ��ȡ"/usr/local/"
	* ����2: "temp.gif" ��ȡ "./"
	* @param sFullFileName �ļ�����ȫ����
	* @return              ��ȡ����ļ�·��
	*/
  static string extractFilePath(const string &sFullFileName);

  /**
	* @brief ��ȡ�ļ���չ��.
	*
	* ����1: "/usr/local/temp.gif" ��ȡ"gif"
	* ����2: "temp.gif" ��ȡ"gif"
	*@param sFullFileName �ļ�����
	*@return              �ļ���չ��
	*/
  static string extractFileExt(const string &sFullFileName);

  /**
	* @brief ��ȡ�ļ�����,ȥ����չ��.
	* ����1: "/usr/local/temp.gif" ��ȡ"/usr/local/temp"
	* ����2: "temp.gif" ��ȡ"temp"
	* @param sFullFileName �ļ�����
	* @return              ȥ����չ�����ļ�����
	*/
  static string excludeFileExt(const string &sFullFileName);

  /**
	* @brief �滻�ļ���չ��
	*
	* �ı��ļ����ͣ��������չ��,�������չ�� =?1:
	* ����1��"/usr/temp.gif" �� �� "jpg" �õ�"/usr/temp.jpg"
	* ����2: "/usr/local/temp" �� �� "jpg" �õ�"/usr/local/temp.jpg"
	* @param sFullFileName �ļ�����
	* @param sExt          ��չ��
	* @return              �滻��չ������ļ���
	*/
  static string replaceFileExt(const string &sFullFileName, const string &sExt);

  /**
	* @brief ��һ��url�л�ȡ��ȫ�ļ���.
	*
	* ��ȡ��http://��,��һ��'/'����������ַ�
	* ����1:http://www.qq.com/tmp/temp.gif ��ȡtmp/temp.gif
	* ����2:www.qq.com/tmp/temp.gif ��ȡtmp/temp.gif
	* ����3:/tmp/temp.gif ��ȡtmp/temp.gif
	* @param sUrl url�ַ���
	* @return     �ļ�����
	*/
  static string extractUrlFilePath(const string &sUrl);

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
  /**
	* @brief �����ļ�ʱȷ���Ƿ�ѡ��.
	*
	* @return 1-ѡ��, 0-��ѡ��
	*/
  typedef int (*FILE_SELECT)(const dirent *);

  /**
	* @brief ɨ��һ��Ŀ¼.
	*
	* @param sFilePath     ��Ҫɨ���·��
	* @param vtMatchFiles  ���ص��ļ���ʸ����
	* @param f             ƥ�亯��,ΪNULL��ʾ�����ļ�����ȡ
	* @param iMaxSize      ����ļ�����,iMaxSize <=0ʱ,��������ƥ���ļ�
	* @return              �ļ�����
	*/
  static size_t scanDir(const string &sFilePath, vector<string> &vtMatchFiles, FILE_SELECT f = NULL, int iMaxSize = 0);
#endif

  /**
	 * @brief ����Ŀ¼, ��ȡĿ¼����������ļ�����Ŀ¼.
	 *
	 * @param path       ��Ҫ������·��
	 * @param files      Ŀ��·�����������ļ�
	 * @param bRecursive �Ƿ�ݹ���Ŀ¼
	 *
	 **/
  static void listDirectory(const string &path, vector<string> &files, bool bRecursive);

  /**
	* @brief �����ļ���Ŀ¼.
	* ���ļ�����Ŀ¼��sExistFile���Ƶ�sNewFile
	* @param sExistFile ���Ƶ��ļ�����Ŀ¼Դ·��
	* @param sNewFile   ���Ƶ��ļ�����Ŀ¼Ŀ��·��
	* @param bRemove    �Ƿ���ɾ��sNewFile��copy ����ֹTextfile busy���¸���ʧ��
	* @return
	*/
  static void copyFile(const string &sExistFile, const string &sNewFile, bool bRemove = false);

  /**
	* @brief �Ƿ���windows�̷���ͷ.
	* @return
	*/
  static bool startWindowsPanfu(const string &sPath);

 private:
  static bool isPanfu(const string &sPath);
};
}  // namespace kant
#endif  // _KT_FILE_H_
