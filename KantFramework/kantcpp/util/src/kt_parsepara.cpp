#include "util/kt_parsepara.h"

namespace kant {

#define ENCODE_TABLE "=&%\r\n"

KT_Parsepara::KT_Parsepara(const string &sParam) { load(sParam); }

KT_Parsepara::KT_Parsepara(const map<string, string> &mpParam) { load(mpParam); }

KT_Parsepara::KT_Parsepara(const KT_Parsepara &para) { _param = para._param; }

KT_Parsepara &KT_Parsepara::operator=(const KT_Parsepara &para) {
  if (this != &para) {
    clear();

    _param = para._param;
  }

  return *this;
}

bool KT_Parsepara::operator==(const KT_Parsepara &para) { return _param == para._param; }

const KT_Parsepara KT_Parsepara::operator+(const KT_Parsepara &para) {
  map<string, string> mpParam;
  mpParam = _param;
  mpParam.insert(para._param.begin(), para._param.end());

  return KT_Parsepara(mpParam);
}

KT_Parsepara &KT_Parsepara::operator+=(const KT_Parsepara &para) {
  _param.insert(para._param.begin(), para._param.end());

  return *this;
}

KT_Parsepara::~KT_Parsepara() { clear(); }

void KT_Parsepara::clear() { _param.clear(); }

string KT_Parsepara::encodeMap(const map<string, string> &mpParam) const {
  string sParsepara("");

  map<string, string>::const_iterator it = mpParam.begin();

  while (it != mpParam.end()) {
    sParsepara += encodestr((*it).first) + "=" + encodestr((*it).second);

    it++;

    if (it != mpParam.end()) {
      sParsepara += "&";
    }
  }

  return sParsepara;
}

void KT_Parsepara::decodeMap(const string &sParam, map<string, string> &mpParam) const {
  int iFlag = 0;
  char ch1 = '=';
  char ch2 = '&';
  string sName;
  string sValue;
  string sBuffer;

  if (sParam.length() == 0) {
    mpParam.clear();
    return;
  }

  string::size_type pos = 0;
  while (pos <= sParam.length()) {
    if (sParam[pos] == ch1)  //中间分隔符,前面读入是name
    {
      sName = decodestr(sBuffer);
      sBuffer = "";

      iFlag = 1;
    } else if (sParam[pos] == ch2 || pos == sParam.length())  //结束符,读入的是值
    {
      sValue = decodestr(sBuffer);
      sBuffer = "";

      if (sName.length() > 0 && iFlag) {
        mpParam[sName] = decodestr(sValue);
        iFlag = 0;
      }
    } else {
      sBuffer += sParam[pos];
    }

    pos++;
  }
}

void KT_Parsepara::load(const string &sParam) {
  clear();
  decodeMap(sParam, _param);
}

void KT_Parsepara::load(const map<string, string> &mpParam) { _param = mpParam; }

string KT_Parsepara::tostr() const { return encodeMap(_param); }

string &KT_Parsepara::operator[](const string &sName) { return _param[sName]; }

string KT_Parsepara::getValue(const string &sName) const {
  string sValue;
  map<string, string>::const_iterator it;

  if ((it = _param.find(sName)) != _param.end()) {
    sValue = it->second;
  }

  return sValue;
}

void KT_Parsepara::setValue(const string &sName, const string &sValue) { _param[sName] = sValue; }

map<string, string> &KT_Parsepara::toMap() { return _param; }

const map<string, string> &KT_Parsepara::toMap() const { return _param; }

void KT_Parsepara::traverse(KT_ParseparaTraverseFunc func, void *pParam) {
  map<string, string>::iterator it = _param.begin();
  map<string, string>::iterator itEnd = _param.end();

  while (it != itEnd) {
    func(it->first, it->second, pParam);

    ++it;
  }
}

char KT_Parsepara::x2c(const string &sWhat) {
  register char digit;

  if (sWhat.length() < 2) {
    return '\0';
  }

  digit = (sWhat[0] >= 'A' ? ((sWhat[0] & 0xdf) - 'A') + 10 : (sWhat[0] - '0'));
  digit *= 16;
  digit += (sWhat[1] >= 'A' ? ((sWhat[1] & 0xdf) - 'A') + 10 : (sWhat[1] - '0'));

  return (digit);
}

string KT_Parsepara::decodestr(const string &sParam) {
  string sBuffer("");

  string::size_type pos = 0;

  while (pos < sParam.length()) {
    if (sParam[pos] == '%') {
      if (pos >= sParam.length() - 2) {
        break;
      }

      sBuffer += x2c(sParam.substr(pos + 1));

      pos += 3;
    } else {
      sBuffer += sParam[pos];

      pos++;
    }
  }

  return sBuffer;
}

string KT_Parsepara::encodestr(const string &sParam) {
  string sBuffer("");
  static char sHexTable[17] = "0123456789ABCDEF";

  string::size_type pos = 0;

  while (pos < sParam.length()) {
    if (string(ENCODE_TABLE).find_first_of(sParam[pos]) != string::npos) {
      sBuffer += '%';
      sBuffer += sHexTable[(sParam[pos] >> 4) & 0x0f];
      sBuffer += sHexTable[sParam[pos] & 0x0f];
    } else {
      sBuffer += sParam[pos];
    }

    pos++;
  }

  return sBuffer;
}

}  // namespace kant
