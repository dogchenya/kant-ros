#include "util/kt_option.h"
#include "util/kt_file.h"
#include "util/kt_common.h"
#include "kant2cpp.h"

void usage() {
  cout << "Usage : kant2cpp [OPTION] kantfile" << endl;
  // cout << "  --coder=Demo::interface1;Demo::interface2   create interface encode and decode api" << endl;
  cout << "  --dir=DIRECTORY                             generate source file to DIRECTORY(create kant protocol file "
          "to DIRECTORY, default is current directory)"
       << endl;
  cout
    << "  --check-default=<true,false>                optional field with default value not do package(default: true)"
    << endl;
  cout << "  --unjson                                    not json interface" << endl;
  cout << "  --os                                        only create struct(not create interface) " << endl;
  cout << "  --include=\"dir1;dir2;dir3\"                set search path of kant protocol" << endl;
  // cout << "  --unknown                                   create unkown field" << endl;
  cout << "  --kantMaster                                create get registry info interface" << endl;
  cout << "  --currentPriority						   use current path first." << endl;
  cout << "  --without-trace                             不需要调用链追踪逻辑" << endl;
  cout << "  kant2cpp support type: bool byte short int long float double vector map" << endl;
  exit(0);
}

void check(vector<string>& vKant) {
  for (size_t i = 0; i < vKant.size(); i++) {
    string ext = kant::KT_File::extractFileExt(vKant[i]);
    if (ext == "kant") {
      if (!kant::KT_File::isFileExist(vKant[i])) {
        cerr << "file '" << vKant[i] << "' not exists" << endl;
        usage();
        exit(0);
      }
    } else {
      cerr << "only support kant file." << endl;
      exit(0);
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    usage();
  }

  kant::KT_Option option;
  option.decode(argc, argv);
  vector<string> vKant = option.getSingle();

  check(vKant);

  if (option.hasParam("help")) {
    usage();
  }

  // bool bCoder = option.hasParam("coder");
  // vector<string> vCoder;
  // if(bCoder)
  // {
  //     vCoder = kant::KT_Common::sepstr<string>(option.getValue("coder"), ";", false);
  //     if(vCoder.size() == 0)
  //     {
  //         usage();
  //         return 0;
  //     }
  // }

  Kant2Cpp t2c;

  if (option.hasParam("dir")) {
    t2c.setBaseDir(option.getValue("dir"));
  } else {
    t2c.setBaseDir(".");
  }

  t2c.setCheckDefault(kant::KT_Common::lower(option.getValue("check-default")) == "false" ? false : true);

  t2c.setOnlyStruct(option.hasParam("os"));

  //默认支持json
  t2c.setJsonSupport(true);

  if (option.hasParam("unjson")) {
    t2c.setJsonSupport(false);
  }

  if (option.hasParam("sql")) {
    t2c.setSqlSupport(true);
    t2c.setJsonSupport(true);
  }

  // 调用链追踪
  if (option.hasParam("without-trace")) {
    t2c.setTrace(false);
  } else {
    t2c.setTrace(true);
  }

  if (option.hasParam("xml")) {
    vector<string> vXmlIntf;
    string sXml = kant::KT_Common::trim(option.getValue("xml"));
    sXml = kant::KT_Common::trimleft(kant::KT_Common::trimright(sXml, "]"), "[");
    if (!sXml.empty()) {
      vXmlIntf = kant::KT_Common::sepstr<string>(sXml, ",", false);
    }
    t2c.setXmlSupport(true, vXmlIntf);
  }

  // if (option.hasParam("json"))
  // {
  //     t2c.setJsonSupport(true);
  //     string sJson = kant::KT_Common::trim(option.getValue("json"));
  //     sJson = kant::KT_Common::trimleft(kant::KT_Common::trimright(sJson, "]"), "[");
  //     if (!sJson.empty())
  //     {
  //         t2c.setJsonSupport(kant::KT_Common::sepstr<string>(sJson, ",", false));
  //     }
  // }

  t2c.setKantMaster(option.hasParam("kantMaster"));

  try {
    //增加include搜索路径
    g_parse->addIncludePath(option.getValue("include"));

    //是否可以以kant开头
    g_parse->setKant(option.hasParam("with-kant"));
    g_parse->setHeader(option.getValue("header"));
    g_parse->setCurrentPriority(option.hasParam("currentPriority"));

    // t2c.setUnknownField(option.hasParam("unknown"));
    for (size_t i = 0; i < vKant.size(); i++) {
      g_parse->parse(vKant[i]);
      t2c.createFile(vKant[i]);  //, vCoder);
    }
  } catch (exception& e) {
    cerr << e.what() << endl;
  }

  return 0;
}
