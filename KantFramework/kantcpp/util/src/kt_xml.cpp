#include "util/kt_xml.h"
#include "util/kt_common.h"

#include <math.h>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace kant {

#define FILTER_SPACE                   \
  while (isspace((int)reader.get())) { \
    reader.skip();                     \
  }
#define FILTER_NODENAME                         \
  while (lookup_node_name[(int)reader.get()]) { \
    reader.skip();                              \
  }
#define XML_PARSE_ERROR(what) \
  { throw KT_Xml_Exception(what); }

// Node name (anything but space \n \r \t / > ? \0)
const unsigned char lookup_node_name[256] = {
  // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
  0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1,  // 0
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 1
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,  // 2
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,  // 3
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 5
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 7
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 8
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 9
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // A
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // B
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // C
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // D
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // E
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1   // F
};

//// Text (i.e. PCDATA) (anything but < \0)
//const unsigned char lookup_text[256] =
//{
//  // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
//     0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  // 3
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
//     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
//};

// Text (i.e. PCDATA) that does not require processing when ws normalization is disabled
// (anything but < \0 &)
const unsigned char lookup_text_pure_no_ws[256] = {
  // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 1
  1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 2
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,  // 3
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 5
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 7
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 8
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 9
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // A
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // B
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // C
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // D
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // E
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1   // F
};

// Digits (dec and hex, 255 denotes end of numeric character reference)
const unsigned char lookup_digits[256] = {
  // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // 0
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // 1
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // 2
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   255, 255, 255, 255, 255, 255,  // 3
  255, 10,  11,  12,  13,  14,  15,  255, 255, 255, 255, 255, 255, 255, 255, 255,  // 4
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // 5
  255, 10,  11,  12,  13,  14,  15,  255, 255, 255, 255, 255, 255, 255, 255, 255,  // 6
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // 7
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // 8
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // 9
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // A
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // B
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // C
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // D
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  // E
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255   // F
};

XmlValuePtr KT_Xml::getValue(const string& str) {
  BufferXmlReader reader;
  XmlValueObjPtr p = std::make_shared<XmlValueObj>();

  // Parse BOM, if any
  reader.setBuffer(str.c_str(), str.length());
  if ((unsigned char)reader.get(0) == 0xEF && (unsigned char)reader.get(1) == 0xBB &&
      (unsigned char)reader.get(2) == 0xBF) {
    reader._pos += 3;  // Skup utf-8 bom
  }

  FILTER_SPACE;
  // Parse and append new child
  if (reader.get() == '<' && reader.get(1) == '?') {
    reader.skip(2);
    ignoreDeclaration(reader);
  }

  FILTER_SPACE;
  return getNode(reader);
}

XmlValuePtr KT_Xml::getValue(const vector<char>& buf) {
  BufferXmlReader reader;
  XmlValueObjPtr p = std::make_shared<XmlValueObj>();

  // Parse BOM, if any
  reader.setBuffer(buf);
  if ((unsigned char)reader.get(0) == 0xEF && (unsigned char)reader.get(1) == 0xBB &&
      (unsigned char)reader.get(2) == 0xBF) {
    reader._pos += 3;  // Skup utf-8 bom
  }

  FILTER_SPACE;
  // Parse and append new child
  if (reader.get() == '<' && reader.get(1) == '?') {
    reader.skip(2);
    ignoreDeclaration(reader);
  }

  FILTER_SPACE;
  return getNode(reader);
}

XmlValuePtr KT_Xml::getNode(BufferXmlReader& reader, const string& nodename) {
  XmlValueObjPtr p = std::make_shared<XmlValueObj>();
  while (1) {
    // 开始符号
    if (!reader.expect('<')) {
      return getValue(reader);
    }

    // 判断是否是注释
    if (ignoreComment(reader)) {
      continue;
    }

    // CDATA
    if (reader.get(0) == '!' && reader.get(1) == '[' && reader.get(2) == 'C' && reader.get(3) == 'D' &&
        reader.get(4) == 'A' && reader.get(5) == 'T' && reader.get(6) == 'A' && reader.get(7) == '[') {
      reader.skip(8);
      return getCdata(reader);
    }

    // 判断是否是node结束符
    if (isEndNode(reader, nodename)) {
      break;
    }

    // 取node名称
    FILTER_SPACE;
    size_t pos = reader.pos();
    FILTER_NODENAME;
    string name = string(reader._buf + pos, reader.pos() - pos);
    while (reader.read() != '>')
      ;
    if (reader.get(-2) == '/') {
      XmlValuePtr s = std::make_shared<XmlValueString>();
      insertArray(name, s, p);
      FILTER_SPACE;
      continue;
    }

    // 取Node内容
    FILTER_SPACE;
    XmlValuePtr q = getNode(reader, name);
    insertArray(name, q, p);

    // 遇到尾部就退出
    FILTER_SPACE;
    if (reader.hasEnd()) {
      break;
    }
  }

  if (p->value.size() == 0)  // node内容为空直接返回空字符对象
  {
    XmlValuePtr ss = std::make_shared<XmlValueString>();
    return ss;
  }
  return p;
}

bool KT_Xml::isEndNode(BufferXmlReader& reader, const string& nodename) {
  if (reader.get() == '/' && reader.get(1) != '>' && !nodename.empty()) {
    size_t lastpos = reader.pos();
    size_t start = 0;
    while (isspace(reader.get(start++)))
      ;
    size_t end = start;
    while (lookup_node_name[(int)reader.get(++end)])
      ;
    string backname = string(reader._buf + lastpos + start, end - start);
    if (backname == nodename) {
      while (reader.read() != '>')
        ;
      return true;
    }
  }
  return false;
}

XmlValueStringPtr KT_Xml::getCdata(BufferXmlReader& reader) {
  size_t pos = reader.pos();
  XmlValueStringPtr p = std::make_shared<XmlValueString>(true);
  while (reader.get() != ']' || reader.get(1) != ']' || reader.get(2) != '>') {
    reader.skip(1);
  }
  p->value = string(reader._buf + pos, reader.pos() - pos);
  reader.skip(3);
  while (reader.read() != '>')
    ;
  return p;
}

XmlValueStringPtr KT_Xml::getValue(BufferXmlReader& reader) {
  XmlValueStringPtr p = std::make_shared<XmlValueString>();
  FILTER_SPACE
  while (lookup_text_pure_no_ws[(int)reader.get()]) {
    if (reader.get() != '&') {
      p->value.append(1, reader.read());
    }

    if (reader.get(1) == 'g' && reader.get(2) == 't' && reader.get(3) == ';') {
      p->value.append(1, '>');
      reader.skip(4);
      continue;
    }

    if (reader.get(1) == 'l' && reader.get(2) == 't' && reader.get(3) == ';') {
      p->value.append(1, '<');
      reader.skip(4);
      continue;
    }

    if (reader.get(1) == 'a' && reader.get(2) == 'm' && reader.get(3) == 'p' && reader.get(4) == ';') {
      p->value.append(1, '&');
      reader.skip(5);
      continue;
    }

    if (reader.get(1) == 'a' && reader.get(2) == 'p' && reader.get(3) == 'o' && reader.get(4) == 's' &&
        reader.get(5) == ';') {
      p->value.append(1, '\'');
      reader.skip(6);
      continue;
    }

    if (reader.get(1) == 'q' && reader.get(2) == 'u' && reader.get(3) == 'o' && reader.get(4) == 's' &&
        reader.get(5) == ';') {
      p->value.append(1, '"');
      reader.skip(6);
      continue;
    }

    // 中文转码
    if (reader.get(1) == '#') {
      unsigned long code = 0;
      if (reader.get(2) == 'x') {
        reader.skip(3);
        unsigned char digit = reader.get();
        while (lookup_digits[digit] != 0xFF) {
          code = code * 16 + digit;
          digit = (unsigned char)reader.read();
        }
      } else {
        reader.skip(2);
        unsigned char digit = reader.get();
        while (lookup_digits[digit] != 0xFF) {
          code = code * 10 + digit;
          digit = (unsigned char)reader.read();
        }
      }

      if (reader.read() != ';') {
        XML_PARSE_ERROR("expected ;");
      }

      // Insert UTF8 sequence
      char text[8] = {0};
      if (code < 0x80)  // 1 byte sequence
      {
        text[0] = static_cast<char>(code);
      } else if (code < 0x800)  // 2 byte sequence
      {
        text[1] = static_cast<char>((code | 0x80) & 0xBF);
        code >>= 6;
        text[0] = static_cast<char>(code | 0xc0);
      } else if (code < 0x10000)  // 3 byte sequence
      {
        text[2] = static_cast<char>((code | 0x80) & 0xBF);
        code >>= 6;
        text[1] = static_cast<char>((code | 0x80) & 0xBF);
        code >>= 6;
        text[0] = static_cast<char>(code | 0xE0);
      } else if (code < 0x110000)  // 4 byte sequence
      {
        text[3] = static_cast<unsigned char>((code | 0x80) & 0xBF);
        code >>= 6;
        text[2] = static_cast<unsigned char>((code | 0x80) & 0xBF);
        code >>= 6;
        text[1] = static_cast<unsigned char>((code | 0x80) & 0xBF);
        code >>= 6;
        text[0] = static_cast<unsigned char>(code | 0xF0);
      } else  // Invalid, only codes up to 0x10FFFF are allowed in Unicode
      {
        XML_PARSE_ERROR("invalid numeric character entity");
      }
      p->value.append(text);
    }
  }
  while (reader.read() != '>')
    ;
  return p;
}

void KT_Xml::insertArray(const string& name, XmlValuePtr& v, XmlValueObjPtr& p) {
  if (p->value.find(name) == p->value.end()) {
    p->value[name] = v;
  } else if (p->value[name]->getType() == v->getType()) {
    XmlValueArrayPtr array = std::make_shared<XmlValueArray>();
    array->push_back(p->value[name]);
    array->push_back(v);
    p->value[name] = array;
  } else if (p->value[name]->getType() == eXmlTypeArray) {
    XmlValueArrayPtr array = std::dynamic_pointer_cast<XmlValueArray>(p->value[name]);
    if (array->value.size() > 0 && array->value[0]->getType() == v->getType()) {
      array->push_back(v);
      p->value[name] = array;
    }
  }
}

bool KT_Xml::ignoreComment(BufferXmlReader& reader) {
  if (reader.get() == '!' && reader.get(1) == '-' && reader.get(2) == '-') {
    reader.skip(3);
    while (1) {
      if (reader.read() == '-' && reader.read() == '-' && reader.read() == '>') {
        return true;
      }
    }
  }
  return false;
}

void KT_Xml::ignoreDeclaration(BufferXmlReader& reader) {
  if ((reader.get(0) == 'x' || reader.get(0) == 'X') && (reader.get(1) == 'm' || reader.get(1) == 'M') &&
      (reader.get(2) == 'l' || reader.get(2) == 'L') && isspace(reader.get(3))) {
    // '<?xml ' - xml declaration,  ignore it
    reader.skip(4);
    while (1) {
      if (reader.read() == '?' && reader.read() == '>') {
        return;
      }
    }
  }
  XML_PARSE_ERROR("unexpected xml head")
}

string KT_Xml::writeValue(const XmlValuePtr& p, bool bHead) {
  ostringstream os;
  os << (bHead ? "<?xml version='1.0' encoding='utf-8'?>" : "");
  if (!p || p->getType() != eXmlTypeObj) {
    return os.str();
  }

  writeObj(os, std::dynamic_pointer_cast<XmlValueObj>(p));
  return os.str();
}

void KT_Xml::writeValue(const XmlValuePtr& p, vector<char>& buf, bool bHead) {
  if (!p || p->getType() != eXmlTypeObj) {
    return;
  }

  ostringstream os;
  os << (bHead ? "<?xml version='1.0' encoding='utf-8'?>" : "");
  writeObj(os, std::dynamic_pointer_cast<XmlValueObj>(p));
  string s = os.str();
  buf.assign(s.begin(), s.end());
}

void KT_Xml::writeObj(std::ostream& os, const XmlValuePtr& p) {
  if (p->getType() != eXmlTypeObj) {
    XML_PARSE_ERROR("not support but xmlobj")
  }

  os << "\n";
  XmlValueObjPtr q = std::dynamic_pointer_cast<XmlValueObj>(p);
  for (map<string, XmlValuePtr>::const_iterator it = q->value.begin(); it != q->value.end(); it++) {
    switch (it->second->getType()) {
      case eXmlTypeString:
        os << "<" << it->first << ">";
        writeString(os, it->second);
        os << "</" << it->first << ">\n";
        break;
      case eXmlTypeArray:
        writeArray(os, it->first, it->second);
        break;
      case eXmlTypeObj:
      default:
        os << "<" << it->first << ">";
        writeObj(os, it->second);
        os << "</" << it->first << ">\n";
    }
  }
}

void KT_Xml::writeString(std::ostream& os, const XmlValuePtr& p) {
  XmlValueStringPtr q = std::dynamic_pointer_cast<XmlValueString>(p);
  if (q->cdata) {
    os << "<![CDATA[" << q->value << "]]>";
    return;
  }
  writeEChar(os, q->value);
}

void KT_Xml::writeArray(std::ostream& os, const string& name, const XmlValuePtr& p) {
  XmlValueArrayPtr q = std::dynamic_pointer_cast<XmlValueArray>(p);
  for (size_t i = 0; i < q->value.size(); i++) {
    os << "<" << name << ">";
    if (q->value[i]->getType() == eXmlTypeString) {
      writeString(os, q->value[i]);
    } else {
      writeObj(os, q->value[i]);
    }
    os << "</" << name << ">\r\n";
  }
}

void KT_Xml::writeEChar(std::ostream& os, const string& data) {
  string s(data);
  s = KT_Common::replace(s, "<", "&lt;");
  s = KT_Common::replace(s, ">", "&lt;");
  s = KT_Common::replace(s, "\'", "&apos;");
  s = KT_Common::replace(s, "\"", "&quot;");
  os << s;
}

//Xml里面定义的空白字符
bool KT_Xml::isspace(char c) {
  if (c == ' ' || c == '\t' || c == '\r' || c == '\n') return true;
  return false;
}

}  // namespace kant
