#ifndef _KANT_XML_H_
#define _KANT_XML_H_

#include <netinet/in.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "util/kt_xml.h"
#include "util/kt_common.h"

namespace kant {
class XmlInput {
 public:
  static void readXml(Bool& b, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      b = KT_Common::lower(std::dynamic_pointer_cast<XmlValueString>(p)->value) == "true";
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'Bool' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(Char& c, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      c = KT_Common::strto<Char>(std::dynamic_pointer_cast<XmlValueString>(p)->value);
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'Char' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(UInt8& n, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      n = KT_Common::strto<UInt8>(std::dynamic_pointer_cast<XmlValueString>(p)->value);
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'Uint8' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(Short& n, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      n = KT_Common::strto<Short>(std::dynamic_pointer_cast<XmlValueString>(p)->value);
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'Short' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(UInt16& n, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      n = KT_Common::strto<UInt16>(std::dynamic_pointer_cast<XmlValueString>(p)->value);
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'Uint16' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(Int32& n, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      n = KT_Common::strto<Int32>(std::dynamic_pointer_cast<XmlValueString>(p)->value);
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'Int32' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(UInt32& n, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      n = KT_Common::strto<UInt32>(std::dynamic_pointer_cast<XmlValueString>(p)->value);
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'UInt32' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(Int64& n, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      n = KT_Common::strto<Int64>(std::dynamic_pointer_cast<XmlValueString>(p)->value);
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'Int64' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(Float& n, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      n = KT_Common::strto<Float>(std::dynamic_pointer_cast<XmlValueString>(p)->value);
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'Float' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(Double& n, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      n = KT_Common::strto<Double>(std::dynamic_pointer_cast<XmlValueString>(p)->value);
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'Double' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(std::string& s, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      s = std::dynamic_pointer_cast<XmlValueString>(p)->value;
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'string' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  static void readXml(char* buf, const UInt32 bufLen, UInt32& readLen, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeString) {
      XmlValueStringPtr pString = std::dynamic_pointer_cast<XmlValueString>(p);
      if ((UInt32)pString->value.size() > bufLen) {
        char s[128];
        snprintf(s, sizeof(s), "invalid char * size, size: %u", (UInt32)pString->value.size());
        throw KT_Xml_Exception(s);
      }
      memcpy(buf, pString->value.c_str(), pString->value.size());
      readLen = pString->value.size();
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'char *' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<string, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<string, V> pr;
        pr.first = iter->first;
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<bool, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<bool, V> pr;
        pr.first = KT_Common::strto<bool>(iter->first);
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<Char, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<Char, V> pr;
        pr.first = KT_Common::strto<Int32>(iter->first);
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }
  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<UInt8, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<UInt8, V> pr;
        pr.first = KT_Common::strto<UInt8>(iter->first);
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }
  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<Short, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<Short, V> pr;
        pr.first = KT_Common::strto<Short>(iter->first);
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<UInt16, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<UInt16, V> pr;
        pr.first = KT_Common::strto<UInt16>(iter->first);
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<Int32, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<Int32, V> pr;
        pr.first = KT_Common::strto<Int32>(iter->first);
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<UInt32, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<UInt32, V> pr;
        pr.first = KT_Common::strto<UInt32>(iter->first);
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<Int64, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<Int64, V> pr;
        pr.first = KT_Common::strto<Int64>(iter->first);
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<Float, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<Float, V> pr;
        pr.first = KT_Common::strto<Float>(iter->first);
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<Double, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      map<string, XmlValuePtr>::iterator iter;
      iter = pObj->value.begin();
      for (; iter != pObj->value.end(); ++iter) {
        std::pair<Double, V> pr;
        pr.first = KT_Common::strto<Double>(iter->first);
        readXml(pr.second, iter->second);
        m.insert(pr);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'map' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename K, typename V, typename Cmp, typename Alloc>
  static void readXml(std::map<K, V, Cmp, Alloc>& m, const XmlValuePtr& p, bool isRequire = true) {
    char s[128];
    snprintf(s, sizeof(s), "map key is not Basic type. map key is only string|bool|num");
    throw KT_Xml_Exception(s);
  }

  template <typename T, typename Alloc>
  static void readXml(std::vector<T, Alloc>& v, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeArray) {
      XmlValueArrayPtr pArray = XmlValueArrayPtr::dynamicCast(p);
      v.resize(pArray->value.size());
      for (size_t i = 0; i < pArray->value.size(); ++i) {
        readXml(v[i], pArray->value[i]);
      }
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'vector' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  /// 读取结构数组
  template <typename T>
  static void readXml(T* v, const UInt32 len, UInt32& readLen, const XmlValuePtr& p, bool isRequire = true) {
    if (NULL != p.get() && p->getType() == eXmlTypeArray) {
      XmlValueArrayPtr pArray = XmlValueArrayPtr::dynamicCast(p);
      if (pArray->value.size() > len) {
        char s[128];
        snprintf(s, sizeof(s), "read 'T *' invalid size, size: %u", (uint32_t)pArray->value.size());
        throw KT_Xml_Exception(s);
      }
      for (size_t i = 0; i < pArray->value.size(); ++i) {
        readXml(v[i], pArray->value[i]);
      }
      readLen = pArray->value.size();
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'T *' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }

  template <typename T>
  static void readXml(
    T& v, const XmlValuePtr& p, bool isRequire = true,
    typename detail::disable_if<detail::is_convertible<T*, TarsStructBase*>, void***>::type dummy = 0) {
    Int32 n = 0;
    readXml(n, p, isRequire);
    v = (T)n;
  }

  /// 读取结构
  template <typename T>
  static void readXml(
    T& v, const XmlValuePtr& p, bool isRequire = true,
    typename detail::enable_if<detail::is_convertible<T*, TarsStructBase*>, void***>::type dummy = 0) {
    if (NULL != p.get() && p->getType() == eXmlTypeObj) {
      XmlValueObjPtr pObj = std::dynamic_pointer_cast<XmlValueObj>(p);
      v.readFromXml(pObj);
    } else if (isRequire) {
      char s[128];
      snprintf(s, sizeof(s), "read 'Char' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
      throw KT_Xml_Exception(s);
    }
  }
};

class XmlOutput {
 public:
  static XmlValueStringPtr writeXml(Bool b, bool cdata = false) {
    return (new XmlValueString(b ? "true" : "false", cdata));
  }

  static XmlValueStringPtr writeXml(Char n, bool cdata = false) {
    return (new XmlValueString(KT_Common::tostr(n), cdata));
  }

  static XmlValueStringPtr writeXml(UInt8 n, bool cdata = false) {
    return (new XmlValueString(KT_Common::tostr(n), cdata));
  }

  static XmlValueStringPtr writeXml(Short n, bool cdata = false) {
    return (new XmlValueString(KT_Common::tostr(n), cdata));
  }

  static XmlValueStringPtr writeXml(UInt16 n, bool cdata = false) {
    return (new XmlValueString(KT_Common::tostr(n), cdata));
  }

  static XmlValueStringPtr writeXml(Int32 n, bool cdata = false) {
    return (new XmlValueString(KT_Common::tostr(n), cdata));
  }

  static XmlValueStringPtr writeXml(UInt32 n, bool cdata = false) {
    return (new XmlValueString(KT_Common::tostr(n), cdata));
  }

  static XmlValueStringPtr writeXml(Int64 n, bool cdata = false) {
    return (new XmlValueString(KT_Common::tostr(n), cdata));
  }

  static XmlValueStringPtr writeXml(Float n, bool cdata = false) {
    return (new XmlValueString(KT_Common::tostr(n), cdata));
  }

  static XmlValueStringPtr writeXml(Double n, bool cdata = false) {
    return (new XmlValueString(KT_Common::tostr(n), cdata));
  }

  static XmlValueStringPtr writeXml(const std::string& s, bool cdata = false) { return (new XmlValueString(s, cdata)); }

  static XmlValueStringPtr writeXml(const char* buf, const UInt32 len, bool cdata = false) {
    return (new XmlValueString(string(buf, len), cdata));
  }

  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<string, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<string, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[i->first] = writeXml(i->second);
    }
    return pObj;
  }

  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<Bool, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<Bool, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[KT_Common::tostr(i->first)] = writeXml(i->second);
    }
    return pObj;
  }

  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<Char, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<Char, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[KT_Common::tostr((Int32)i->first)] = writeXml(i->second);
    }
    return pObj;
  }

  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<UInt8, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<UInt8, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[KT_Common::tostr(i->first)] = writeXml(i->second);
    }
    return pObj;
  }

  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<Short, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<Short, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[KT_Common::tostr(i->first)] = writeXml(i->second);
    }
    return pObj;
  }
  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<UInt16, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<UInt16, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[KT_Common::tostr(i->first)] = writeXml(i->second);
    }
    return pObj;
  }
  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<Int32, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<Int32, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[KT_Common::tostr(i->first)] = writeXml(i->second);
    }
    return pObj;
  }
  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<UInt32, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<UInt32, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[KT_Common::tostr(i->first)] = writeXml(i->second);
    }
    return pObj;
  }

  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<Int64, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<Int64, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[KT_Common::tostr(i->first)] = writeXml(i->second);
    }
    return pObj;
  }

  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<Float, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<Float, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[KT_Common::tostr(i->first)] = writeXml(i->second);
    }
    return pObj;
  }

  template <typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<Double, V, Cmp, Alloc>& m) {
    XmlValueObjPtr pObj = new XmlValueObj();
    typedef typename std::map<Double, V, Cmp, Alloc>::const_iterator IT;
    for (IT i = m.begin(); i != m.end(); ++i) {
      pObj->value[KT_Common::tostr(i->first)] = writeXml(i->second);
    }
    return pObj;
  }

  template <typename K, typename V, typename Cmp, typename Alloc>
  static XmlValueObjPtr writeXml(const std::map<K, V, Cmp, Alloc>& m) {
    char s[128];
    snprintf(s, sizeof(s), "map key is not Basic type. map key is only string|bool|num");
    throw KT_Xml_Exception(s);
  }

  template <typename T, typename Alloc>
  static XmlValueArrayPtr writeXml(const std::vector<T, Alloc>& v) {
    XmlValueArrayPtr pArray = new XmlValueArray();
    typedef typename std::vector<T, Alloc>::const_iterator IT;
    for (IT i = v.begin(); i != v.end(); ++i) pArray->value.push_back(writeXml(*i));
    return pArray;
  }

  template <typename T>
  static XmlValueArrayPtr writeXml(const T* v, const UInt32 len) {
    XmlValueArrayPtr pArray = new XmlValueArray();
    for (size_t i = 0; i < len; ++i) {
      pArray->value.push_back(writeXml(v[i]));
    }
    return pArray;
  }

  template <typename T>
  static XmlValueStringPtr writeXml(
    const T& v, typename detail::disable_if<detail::is_convertible<T*, TarsStructBase*>, void***>::type dummy = 0) {
    return writeXml((Int32)v);
  }

  template <typename T>
  static XmlValueObjPtr writeXml(
    const T& v, typename detail::enable_if<detail::is_convertible<T*, TarsStructBase*>, void***>::type dummy = 0) {
    return v.writeToXml();
  }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace kant

#endif
