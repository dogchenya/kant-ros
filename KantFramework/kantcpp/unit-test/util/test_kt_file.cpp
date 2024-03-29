﻿//
// Created by jarod on 2020/2/20.
//

#include "util/kt_file.h"
#include "util/kt_config.h"
#include "gtest/gtest.h"

using namespace kant;

class UtilFileTest : public testing::Test {
 public:
  //添加日志
  static void SetUpTestCase() {
    //        cout<<"SetUpTestCase"<<endl;
  }
  static void TearDownTestCase() {
    //        cout<<"TearDownTestCase"<<endl;
  }
  virtual void SetUp()  //TEST跑之前会执行SetUp
  {
    //        cout<<"SetUp"<<endl;
  }
  virtual void TearDown()  //TEST跑完之后会执行TearDown
  {
    //        cout<<"TearDown"<<endl;
  }
};

TEST_F(UtilFileTest, file)  //此时使用的是TEST_F宏
{
  string file = "./test.dat";
  string data = "helloword";
  data[3] = '\0';

  KT_File::save2file(file, data);
  size_t fileSize = KT_File::getFileSize(file);
  //    cout << "file size:" << fileSize  << endl;
  ASSERT_TRUE(fileSize == data.size());

  string load = KT_File::load2str(file);
  //    cout << "load file size:" << load.size() << endl;
  ASSERT_TRUE(load == data);

  bool fileExists = KT_File::isFileExist(file, S_IFREG);
  //    cout << "file exists:" << fileExists << endl;
  ASSERT_TRUE(fileExists);

  string dir = "test";
  KT_File::makeDir(dir);

  fileExists = KT_File::isFileExist(dir, S_IFDIR);
  //	cout << "dir exists:" << fileExists << endl;
  ASSERT_TRUE(fileExists);

  string newFile = dir + FILE_SEP + file;
  KT_File::save2file(newFile, data);
  fileExists = KT_File::isFileExist(newFile, S_IFREG);
  //	cout << "newFile exists:" << fileExists << endl;
  ASSERT_TRUE(fileExists);

  KT_File::makeDir(dir + FILE_SEP + "test1");
  KT_File::makeDir(dir + FILE_SEP + "test2");

  vector<string> v;
  KT_File::listDirectory(dir, v, true);
  //	cout << "listDirectory:" << endl;
  //	for(auto s : v)
  //	{
  //		cout << "    " << s << endl;
  //	}

  KT_File::removeFile(dir, true);
  fileExists = KT_File::isFileExist(file, S_IFDIR);
  //	cout << "dir exists:" << fileExists << endl;
  ASSERT_TRUE(!fileExists);
}

TEST_F(UtilFileTest, simplifyDirectory) {
#if TARGET_PLATFORM_WINDOWS
  //    cout << "simplifyDirectory:" << KT_File::simplifyDirectory("/./ab/tt//t///t//../tt/") << endl;

  ASSERT_TRUE(KT_File::simplifyDirectory("/./ab/tt//t///t//../tt/") == "ab\\tt\\t\\tt");
  //	cout << "simplifyDirectory:" << KT_File::simplifyDirectory("c:/ab/tt//t///t//../tt/") << endl;
  ASSERT_TRUE(KT_File::simplifyDirectory("c:/ab/tt//t///t//../tt/") == "c:\\ab\\tt\\t\\tt");
#else

  //	cout << "simplifyDirectory:" << KT_File::simplifyDirectory("/./ab/tt//t///t//../tt/") << endl;
  ASSERT_TRUE(KT_File::simplifyDirectory("/./ab/tt//t///t//../tt/") == "/ab/tt/t/tt");
  //	cout << "simplifyDirectory:" << KT_File::simplifyDirectory("/ab/tt//t///t//../tt/") << endl;
  ASSERT_TRUE(KT_File::simplifyDirectory("/ab/tt//t///t//../tt/") == "/ab/tt/t/tt");
#endif
}

TEST_F(UtilFileTest, nameAndPath) {
  ASSERT_TRUE(KT_File::extractFileExt("/usr/local/app/bin.exe") == "exe");
  ASSERT_TRUE(KT_File::extractFileExt("/usr/local/app/bin") == "");
  ASSERT_TRUE(KT_File::extractFileExt("/usr/local/app.dir/bin.exe") == "exe");
  ASSERT_TRUE(KT_File::extractFileExt("c:\\usr\\local\\app.dir\\bin.exe") == "exe");
  ASSERT_TRUE(KT_File::extractFileExt("c:\\usr\\local\\app.dir\\bin") == "");

  ASSERT_TRUE(KT_File::extractFileName("/usr/local/app/bin.exe") == "bin.exe");
  ASSERT_TRUE(KT_File::extractFileName("/usr/local/app/bin") == "bin");
  ASSERT_TRUE(KT_File::extractFileName("/usr/local/app.dir/bin.exe") == "bin.exe");
  ASSERT_TRUE(KT_File::extractFileName("c:\\usr\\local\\app.dir\\bin.exe") == "bin.exe");
  ASSERT_TRUE(KT_File::extractFileName("c:\\usr\\local\\app.dir\\bin") == "bin");
  ASSERT_TRUE(KT_File::extractFileName("bin.exe") == "bin.exe");

  ASSERT_TRUE(KT_File::extractFilePath("/usr/local/app/bin.exe") == "/usr/local/app/");
  ASSERT_TRUE(KT_File::extractFilePath("/usr/local/app/bin") == "/usr/local/app/");
  ASSERT_TRUE(KT_File::extractFilePath("/usr/local/app.dir/bin.exe") == "/usr/local/app.dir/");
  ASSERT_TRUE(KT_File::extractFilePath("c:\\usr\\local\\app.dir\\bin.exe") == "c:\\usr\\local\\app.dir\\");
  ASSERT_TRUE(KT_File::extractFilePath("c:\\usr\\local\\app.dir\\bin") == "c:\\usr\\local\\app.dir\\");
  ASSERT_TRUE(KT_File::extractFilePath("temp.gif") == string(".") + FILE_SEP);

  ASSERT_TRUE(KT_File::excludeFileExt("/usr/local/app/bin.exe") == "/usr/local/app/bin");

  ASSERT_TRUE(KT_File::excludeFileExt("/usr/local/app/bin") == "/usr/local/app/bin");
  ASSERT_TRUE(KT_File::excludeFileExt("/usr/local/app.dir/bin.exe") == "/usr/local/app.dir/bin");
  ASSERT_TRUE(KT_File::excludeFileExt("c:\\usr\\local\\app.dir\\bin.exe") == "c:\\usr\\local\\app.dir\\bin");
  ASSERT_TRUE(KT_File::excludeFileExt("c:\\usr\\local\\app.dir\\bin") == "c:\\usr\\local\\app.dir\\bin");
  ASSERT_TRUE(KT_File::excludeFileExt("temp.gif") == "temp");
}

#define CONFIG \
  "<tars> \r\n \
<application>\r\n \
<volumes>\r\n \
/Volumes/MyData/centos/=/data\r\n \
/Volumes/MyData/=/mnt/data\r\n \
</volumes>\r\n\
<ports>\r\n\
8080/tcp=0.0.0.0:8080\r\n \
8081/tcp=0.0.0.0:8081\r\n \
</ports>\r\n \
</application>\r\n \
</tars>"

TEST_F(UtilFileTest, config) {
  KT_Config conf;
  conf.parseString(CONFIG);

  auto volumes = conf.getDomainKey("/tars/application/volumes");

  ASSERT_TRUE(volumes.size() == 2);
}
