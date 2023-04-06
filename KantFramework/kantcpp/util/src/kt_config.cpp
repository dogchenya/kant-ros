#include <cerrno>
#include <fstream>
#include "util/kt_config.h"
#include "util/kt_common.h"

namespace kant {

KT_ConfigDomain::KT_ConfigDomain(const string &sLine) { _name = KT_Common::trim(sLine); }

KT_ConfigDomain::~KT_ConfigDomain() { destroy(); }

KT_ConfigDomain::KT_ConfigDomain(const KT_ConfigDomain &tcd) { (*this) = tcd; }

KT_ConfigDomain &KT_ConfigDomain::operator=(const KT_ConfigDomain &tcd) {
  if (this != &tcd) {
    destroy();

    _name = tcd._name;
    _param = tcd._param;
    _key = tcd._key;
    _domain = tcd._domain;
    _line = tcd._line;

    const map<string, KT_ConfigDomain *> &m = tcd.getDomainMap();
    map<string, KT_ConfigDomain *>::const_iterator it = m.begin();
    while (it != m.end()) {
      _subdomain[it->first] = it->second->clone();
      ++it;
    }
  }
  return *this;
}

KT_ConfigDomain::DomainPath KT_ConfigDomain::parseDomainName(const string &path, bool bWithParam) {
  KT_ConfigDomain::DomainPath dp;

  if (bWithParam) {
    string::size_type pos1 = path.find_first_of(KT_CONFIG_PARAM_BEGIN);
    if (pos1 == string::npos) {
      throw KT_Config_Exception("[KT_Config::parseDomainName] : param path '" + path + "' is invalid!");
    }

    if (path[0] != KT_CONFIG_DOMAIN_SEP) {
      throw KT_Config_Exception("[KT_Config::parseDomainName] : param path '" + path + "' must start with '/'!");
    }

    string::size_type pos2 = path.find_first_of(KT_CONFIG_PARAM_END);
    if (pos2 == string::npos) {
      throw KT_Config_Exception("[KT_Config::parseDomainName] : param path '" + path + "' is invalid!");
    }

    dp._domains = KT_Common::sepstr<string>(path.substr(1, pos1 - 1), KT_Common::tostr(KT_CONFIG_DOMAIN_SEP));
    dp._param = path.substr(pos1 + 1, pos2 - pos1 - 1);
  } else {
    //    	if(path.length() <= 1 || path[0] != KT_CONFIG_DOMAIN_SEP)
    if (path[0] != KT_CONFIG_DOMAIN_SEP) {
      throw KT_Config_Exception("[KT_Config::parseDomainName] : param path '" + path + "' must start with '/'!");
    }

    dp._domains = KT_Common::sepstr<string>(path.substr(1), KT_Common::tostr(KT_CONFIG_DOMAIN_SEP));
  }

  return dp;
}

KT_ConfigDomain *KT_ConfigDomain::addSubDomain(const string &name) {
  if (_subdomain.find(name) == _subdomain.end()) {
    _domain.push_back(name);

    _subdomain[name] = new KT_ConfigDomain(name);
  }
  return _subdomain[name];
}

string KT_ConfigDomain::getParamValue(const string &name) const {
  map<string, string>::const_iterator it = _param.find(name);
  if (it == _param.end()) {
    throw KT_ConfigNoParam_Exception("[KT_ConfigDomain::getParamValue] param '" + name + "' not exits!");
  }

  return it->second;
}

KT_ConfigDomain *KT_ConfigDomain::getSubKtConfigDomain(vector<string>::const_iterator itBegin,
                                                       vector<string>::const_iterator itEnd) {
  if (itBegin == itEnd) {
    return this;
  }

  map<string, KT_ConfigDomain *>::const_iterator it = _subdomain.find(*itBegin);

  //根据匹配规则找不到匹配的子域
  if (it == _subdomain.end()) {
    return NULL;
  }

  //继续在子域下搜索
  return it->second->getSubKtConfigDomain(itBegin + 1, itEnd);
}

const KT_ConfigDomain *KT_ConfigDomain::getSubKtConfigDomain(vector<string>::const_iterator itBegin,
                                                             vector<string>::const_iterator itEnd) const {
  if (itBegin == itEnd) {
    return this;
  }

  map<string, KT_ConfigDomain *>::const_iterator it = _subdomain.find(*itBegin);

  //根据匹配规则找不到匹配的子域
  if (it == _subdomain.end()) {
    return NULL;
  }

  //继续在子域下搜索
  return it->second->getSubKtConfigDomain(itBegin + 1, itEnd);
}

void KT_ConfigDomain::insertParamValue(const map<string, string> &m) {
  for (auto e : m) {
    _param[e.first] = e.second;
  }

  map<string, string>::const_iterator it = m.begin();
  while (it != m.end()) {
    size_t i = 0;
    for (; i < _key.size(); i++) {
      if (_key[i] == it->first) {
        break;
      }
    }

    //没有该key, 则添加到最后
    if (i == _key.size()) {
      _key.push_back(it->first);
    }

    ++it;
  }
}

void KT_ConfigDomain::setParamValue(const string &name, const string &value) {
  _param[name] = value;

  //如果key已经存在,则删除
  for (vector<string>::iterator it = _key.begin(); it != _key.end(); ++it) {
    if (*it == name) {
      _key.erase(it);
      break;
    }
  }

  _key.push_back(name);
}

void KT_ConfigDomain::setParamValue(const string &line) {
  if (line.empty()) {
    return;
  }

  _line.push_back(line);

  string::size_type pos = 0;
  for (; pos <= line.length() - 1; pos++) {
    if (line[pos] == '=') {
      if (pos > 0 && line[pos - 1] == '\\') {
        continue;
      }

      string name = parse(KT_Common::trim(line.substr(0, pos), " \r\n\t"));

      string value;
      if (pos < line.length() - 1) {
        value = parse(KT_Common::trim(line.substr(pos + 1), " \r\n\t"));
      }

      setParamValue(name, value);
      return;
    }
  }

  // also need parse
  string name = parse(KT_Common::trim(line, " \r\n\t"));
  setParamValue(name, "");
}

string KT_ConfigDomain::parse(const string &s) {
  if (s.empty()) {
    return "";
  }

  string param;
  string::size_type pos = 0;
  for (; pos <= s.length() - 1; pos++) {
    char c;
    if (s[pos] == '\\' && pos < s.length() - 1) {
      switch (s[pos + 1]) {
        case '\\':
          c = '\\';
          pos++;
          break;
        case 'r':
          c = '\r';
          pos++;
          break;
        case 'n':
          c = '\n';
          pos++;
          break;
        case 't':
          c = '\t';
          pos++;
          break;
        case '=':
          c = '=';
          pos++;
          break;
        default:
          throw KT_Config_Exception("[KT_ConfigDomain::parse] '" + s + "' is invalid, '" + KT_Common::tostr(s[pos]) +
                                    KT_Common::tostr(s[pos + 1]) + "' couldn't be parse!");
      }

      param += c;
    } else if (s[pos] == '\\') {
      throw KT_Config_Exception("[KT_ConfigDomain::parse] '" + s + "' is invalid, '" + KT_Common::tostr(s[pos]) +
                                "' couldn't be parse!");
    } else {
      param += s[pos];
    }
  }

  return param;
}

string KT_ConfigDomain::reverse_parse(const string &s) {
  if (s.empty()) {
    return "";
  }

  string param;
  string::size_type pos = 0;
  for (; pos <= s.length() - 1; pos++) {
    string c;
    switch (s[pos]) {
      case '\\':
        param += "\\\\";
        break;
      case '\r':
        param += "\\r";
        break;
      case '\n':
        param += "\\n";
        break;
      case '\t':
        param += "\\t";
        break;
      case '=':
        param += "\\=";
        break;
      case '<':
      case '>':
        throw KT_Config_Exception("[KT_ConfigDomain::reverse_parse] '" + s + "' is invalid, couldn't be parse!");
      default:
        param += s[pos];
    }
  }

  return param;
}

string KT_ConfigDomain::getName() const { return _name; }

void KT_ConfigDomain::setName(const string &name) { _name = name; }

vector<string> KT_ConfigDomain::getKey() const { return _key; }

vector<string> KT_ConfigDomain::getLine() const { return _line; }

vector<string> KT_ConfigDomain::getSubDomain() const { return _domain; }

void KT_ConfigDomain::destroy() {
  _param.clear();
  _key.clear();
  _line.clear();
  _domain.clear();

  map<string, KT_ConfigDomain *>::iterator it = _subdomain.begin();
  while (it != _subdomain.end()) {
    delete it->second;
    ++it;
  }

  _subdomain.clear();
}

string KT_ConfigDomain::tostr(int i) const {
  string sTab;
  for (int k = 0; k < i; ++k) {
    sTab += "  ";
  }

  ostringstream buf;

  buf << sTab << "<" << reverse_parse(_name) << ">" << endl;
  ;

  for (size_t n = 0; n < _key.size(); n++) {
    map<string, string>::const_iterator it = _param.find(_key[n]);

    assert(it != _param.end());

    //值为空, 则不打印出=
    if (it->second.empty()) {
      buf << "  " << sTab << reverse_parse(_key[n]) << endl;
    } else {
      buf << "  " << sTab << reverse_parse(_key[n]) << "=" << reverse_parse(it->second) << endl;
    }
  }

  ++i;

  for (size_t n = 0; n < _domain.size(); n++) {
    map<string, KT_ConfigDomain *>::const_iterator itm = _subdomain.find(_domain[n]);

    assert(itm != _subdomain.end());

    buf << itm->second->tostr(i);
  }

  buf << sTab << "</" << reverse_parse(_name) << ">" << endl;

  return buf.str();
}

/********************************************************************/
/*		KT_Config implement										    */
/********************************************************************/

KT_Config::KT_Config() : _root("") {}

KT_Config::KT_Config(const KT_Config &tc) : _root(tc._root) {}

KT_Config &KT_Config::operator=(const KT_Config &tc) {
  if (this != &tc) {
    _root = tc._root;
  }

  return *this;
}

void KT_Config::parse(istream &is) {
  _root.destroy();

  stack<KT_ConfigDomain *> stkKtCnfDomain;
  stkKtCnfDomain.push(&_root);

  string line;
  while (getline(is, line)) {
    line = KT_Common::trim(line, " \r\n\t");

    if (line.length() == 0) {
      continue;
    }

    if (line[0] == '#') {
      continue;
    } else if (line[0] == '<') {
      string::size_type posl = line.find_first_of('>');

      if (posl == string::npos) {
        throw KT_Config_Exception("[KT_Config::parse]:parse error! line : " + line);
      }

      if (line[1] == '/') {
        string sName(line.substr(2, (posl - 2)));

        if (stkKtCnfDomain.size() <= 0) {
          throw KT_Config_Exception("[KT_Config::parse]:parse error! <" + sName + "> hasn't matched domain.");
        }

        if (stkKtCnfDomain.top()->getName() != sName) {
          throw KT_Config_Exception("[KT_Config::parse]:parse error! <" + stkKtCnfDomain.top()->getName() +
                                    "> hasn't match <" + sName + ">.");
        }

        //弹出
        stkKtCnfDomain.pop();
      } else {
        string name(line.substr(1, posl - 1));

        stkKtCnfDomain.push(stkKtCnfDomain.top()->addSubDomain(name));
      }
    } else {
      stkKtCnfDomain.top()->setParamValue(line);
    }
  }

  if (stkKtCnfDomain.size() != 1) {
    throw KT_Config_Exception("[KT_Config::parse]:parse error : hasn't match");
  }
}

void KT_Config::parseFile(const string &sFileName) {
  if (sFileName.length() == 0) {
    throw KT_Config_Exception("[KT_Config::parseFile]:file name is empty");
  }

  ifstream ff;
  ff.open(sFileName.c_str());
  if (!ff) {
    THROW_EXCEPTION_SYSCODE(KT_Config_Exception, "[KT_Config::parseFile]:fopen fail: " + sFileName);
    // throw KT_Config_Exception("[KT_Config::parseFile]:fopen fail: " + sFileName, TC_Exception::getSystemCode());
  }

  parse(ff);
}

void KT_Config::parseString(const string &buffer) {
  istringstream iss;
  iss.str(buffer);

  parse(iss);
}

string KT_Config::operator[](const string &path) const {
  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(path, true);

  const KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain == NULL) {
    throw KT_ConfigNoParam_Exception("[KT_Config::operator[]] path '" + path + "' not exits!");
  }

  return pKtConfigDomain->getParamValue(dp._param);
}

string KT_Config::get(const string &sName, const string &sDefault) const {
  try {
    KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(sName, true);

    const KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

    if (pKtConfigDomain == NULL) {
      return sDefault;
      // throw KT_ConfigNoParam_Exception("[KT_Config::get] path '" + sName + "' not exits!");
    }

    return pKtConfigDomain->getParamValue(dp._param);
  } catch (KT_ConfigNoParam_Exception &ex) {
    return sDefault;
  }
}

void KT_Config::set(const string &sName, const string &value) {
  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(sName, true);

  map<string, string> v;
  v[dp._param] = value;

  KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain == NULL) {
    pKtConfigDomain = &_root;

    for (size_t i = 0; i < dp._domains.size(); i++) {
      pKtConfigDomain = pKtConfigDomain->addSubDomain(dp._domains[i]);
    }
  }

  pKtConfigDomain->insertParamValue(v);
}

bool KT_Config::getDomainMap(const string &path, map<string, string> &m) const {
  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(path, false);

  const KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain == NULL) {
    return false;
  }

  m = pKtConfigDomain->getParamMap();

  return true;
}

map<string, string> KT_Config::getDomainMap(const string &path) const {
  map<string, string> m;

  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(path, false);

  const KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain != NULL) {
    m = pKtConfigDomain->getParamMap();
  }

  return m;
}

vector<string> KT_Config::getDomainKey(const string &path) const {
  vector<string> v;

  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(path, false);

  const KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain != NULL) {
    v = pKtConfigDomain->getKey();
  }

  return v;
}

vector<string> KT_Config::getDomainLine(const string &path) const {
  vector<string> v;

  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(path, false);

  const KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain != NULL) {
    v = pKtConfigDomain->getLine();
  }

  return v;
}

bool KT_Config::hasDomainVector(const string &path) const {
  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(path, false);

  //根域, 特殊处理
  if (dp._domains.empty()) {
    return !_root.getSubDomain().empty();
  }

  const KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain == NULL) {
    return false;
  }

  return true;
}

bool KT_Config::getDomainVector(const string &path, vector<string> &vtDomains) const {
  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(path, false);

  //根域, 特殊处理
  if (dp._domains.empty()) {
    vtDomains = _root.getSubDomain();
    return !vtDomains.empty();
  }

  const KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain == NULL) {
    return false;
  }

  vtDomains = pKtConfigDomain->getSubDomain();

  return true;
}

vector<string> KT_Config::getDomainVector(const string &path) const {
  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(path, false);

  //根域, 特殊处理
  if (dp._domains.empty()) {
    return _root.getSubDomain();
  }

  const KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain == NULL) {
    return vector<string>();
  }

  return pKtConfigDomain->getSubDomain();
}

KT_ConfigDomain *KT_Config::newKtConfigDomain(const string &sName) { return new KT_ConfigDomain(sName); }

KT_ConfigDomain *KT_Config::searchKtConfigDomain(const vector<string> &domains) {
  return _root.getSubKtConfigDomain(domains.begin(), domains.end());
}

const KT_ConfigDomain *KT_Config::searchKtConfigDomain(const vector<string> &domains) const {
  return _root.getSubKtConfigDomain(domains.begin(), domains.end());
}

int KT_Config::insertDomain(const string &sCurDomain, const string &sAddDomain, bool bCreate) {
  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(sCurDomain, false);

  KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain == NULL) {
    if (bCreate) {
      pKtConfigDomain = &_root;

      for (size_t i = 0; i < dp._domains.size(); i++) {
        pKtConfigDomain = pKtConfigDomain->addSubDomain(dp._domains[i]);
      }
    } else {
      return -1;
    }
  }

  pKtConfigDomain->addSubDomain(sAddDomain);

  return 0;
}

int KT_Config::insertDomainParam(const string &sCurDomain, const map<string, string> &m, bool bCreate) {
  KT_ConfigDomain::DomainPath dp = KT_ConfigDomain::parseDomainName(sCurDomain, false);

  KT_ConfigDomain *pKtConfigDomain = searchKtConfigDomain(dp._domains);

  if (pKtConfigDomain == NULL) {
    if (bCreate) {
      pKtConfigDomain = &_root;

      for (size_t i = 0; i < dp._domains.size(); i++) {
        pKtConfigDomain = pKtConfigDomain->addSubDomain(dp._domains[i]);
      }
    } else {
      return -1;
    }
  }

  pKtConfigDomain->insertParamValue(m);

  return 0;
}

string KT_Config::tostr() const {
  string buffer;

  map<string, KT_ConfigDomain *> msd = _root.getDomainMap();
  map<string, KT_ConfigDomain *>::const_iterator it = msd.begin();
  while (it != msd.end()) {
    buffer += it->second->tostr(0);
    ++it;
  }

  return buffer;
}

void KT_Config::joinConfig(const KT_Config &cf, bool bUpdate) {
  string buffer;
  if (bUpdate) {
    buffer = tostr() + cf.tostr();
  } else {
    buffer = cf.tostr() + tostr();
  }
  parseString(buffer);
}

}  // namespace kant
