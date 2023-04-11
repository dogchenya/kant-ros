#include "kant2cpp.h"
#include "util/kt_md5.h"
#include "util/kt_file.h"
#include "util/kt_common.h"
#include <string>

#define TAB g_parse->getTab()
#define INC_TAB g_parse->incTab()
#define DEL_TAB g_parse->delTab()
#define G_TRACE_PARAM_OVER_MAX_LEN "\"{\\\"trace_param_over_max_len\\\":true}\""

//////////////////////////////////////////////////////////////////////////////////
//
Kant2Cpp::Kant2Cpp()
  : _checkDefault(false),
    _onlyStruct(false),
    _bSqlSupport(false),
    _bXmlSupport(false),
    _bJsonSupport(true),
    _namespace("kant")
    // , _unknownField(false)
    ,
    _kantMaster(false),
    _bTrace(true) {}

string Kant2Cpp::writeToXml(const TypeIdPtr& pPtr) const {
  ostringstream s;
  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    s << TAB << "p->value[\"" << pPtr->getId()
      << "\"] = " + _namespace + "::XmlOutput::writeXml((" + _namespace + "::Int32)" << pPtr->getId()
      << ", _cdata_format);" << endl;
  } else if (pPtr->getTypePtr()->isArray()) {
    s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::XmlOutput::writeXml((const "
      << tostr(pPtr->getTypePtr()) << " *)" << pPtr->getId() << "Len"
      << ");" << endl;
  } else if (pPtr->getTypePtr()->isPointer()) {
    s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::XmlOutput::writeXml((const "
      << tostr(pPtr->getTypePtr()) << " )" << pPtr->getId() << "Len"
      << ");" << endl;
  } else {
    MapPtr mPtr = std::dynamic_pointer_cast<Map>(pPtr->getTypePtr());
    VectorPtr vPtr = std::dynamic_pointer_cast<Vector>(pPtr->getTypePtr());
    if (!_checkDefault || pPtr->isRequire() || (!pPtr->hasDefault() && !mPtr && !vPtr)) {
      BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getTypePtr());
      if (pPtr->getTypePtr()->isSimple() || (bPtr && bPtr->kind() == Builtin::KindString)) {
        s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::XmlOutput::writeXml(" << pPtr->getId()
          << ", _cdata_format);" << endl;
      } else {
        s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::XmlOutput::writeXml(" << pPtr->getId()
          << ");" << endl;
      }
    } else {
      string sDefault = pPtr->def();
      BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getTypePtr());
      if (bPtr && bPtr->kind() == Builtin::KindString) {
        sDefault = "\"" + KT_Common::replace(pPtr->def(), "\"", "\\\"") + "\"";
      }

      if (mPtr || vPtr) {
        s << TAB << "if (" << pPtr->getId() << ".size() > 0)" << endl;
      } else {
        s << TAB << "if (" << pPtr->getId() << " != " << sDefault << ")" << endl;
      }

      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::XmlOutput::writeXml(" << pPtr->getId()
        << ");" << endl;
      DEL_TAB;
      s << TAB << "}" << endl;
    }
  }

  return s.str();
}

string Kant2Cpp::readFromXml(const TypeIdPtr& pPtr, bool bIsRequire) const {
  ostringstream s;
  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    s << TAB << "kant::XmlInput::readXml((kant::Int32&)" << pPtr->getId() << ", pObj->value[\"" << pPtr->getId()
      << "\"]";
  } else if (pPtr->getTypePtr()->isArray()) {
    s << TAB << "kant::XmlInput::readXml(" << pPtr->getId() << "Len"
      << ", pObj->value[\"" << pPtr->getId() << "\"]" << getSuffix(pPtr);
  } else if (pPtr->getTypePtr()->isPointer()) {
    // "not support";
  } else {
    s << TAB << "kant::XmlInput::readXml(" << pPtr->getId() << ",pObj->value[\"" << pPtr->getId() << "\"]";
  }
  s << ", " << ((pPtr->isRequire() && bIsRequire) ? "true" : "false") << ");" << endl;

  return s.str();
}

string Kant2Cpp::writeToSql(const TypeIdPtr& pPtr) const {
  ostringstream s;
  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    s << TAB << "_mycols[\"" << pPtr->getId() << "\"] = make_pair(kant::KT_Mysql::DB_INT, kant::KT_Common::tostr("
      << pPtr->getId() << "));" << endl;
  }

  BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getTypePtr());
  if (bPtr) {
    switch (bPtr->kind()) {
      case Builtin::KindBool:
      case Builtin::KindByte:
      case Builtin::KindShort:
      case Builtin::KindInt:
      case Builtin::KindLong:
        s << TAB << "_mycols[\"" << pPtr->getId() << "\"] = make_pair(kant::KT_Mysql::DB_INT, kant::KT_Common::tostr("
          << pPtr->getId() << "));" << endl;
        break;
      case Builtin::KindFloat:
      case Builtin::KindDouble:
        s << TAB << "_mycols[\"" << pPtr->getId() << "\"] = make_pair(kant::KT_Mysql::DB_STR, kant::KT_Common::tostr("
          << pPtr->getId() << "));" << endl;
        break;
      case Builtin::KindString:
        s << TAB << "_mycols[\"" << pPtr->getId() << "\"] = make_pair(kant::KT_Mysql::DB_STR, kant::KT_Common::trim("
          << pPtr->getId() << "));" << endl;
        break;
      default:
        break;
    }
  } else if (!pPtr->getTypePtr()->isSimple()) {
    s << TAB << "_mycols[\"" << pPtr->getId()
      << "\"] = make_pair(kant::KT_Mysql::DB_STR, kant::KT_Json::writeValue(kant::JsonOutput::writeJson("
      << pPtr->getId() << ")));" << endl;
  }

  return s.str();
  return s.str();
}

string Kant2Cpp::readFromSql(const TypeIdPtr& pPtr, bool bIsRequire) const {
  ostringstream s;
  EnumPtr ePtr = std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr());
  if (ePtr) {
    s << TAB << pPtr->getId() << " = (" << ePtr->getSid() << ")KT_Common::strto<kant::Int32>(_mysrd[\"" << pPtr->getId()
      << "\"]);" << endl;
  }

  BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getTypePtr());
  if (bPtr) {
    switch (bPtr->kind()) {
      case Builtin::KindBool:
        s << TAB << pPtr->getId() << " = KT_Common::strto<kant::Bool>(_mysrd[\"" << pPtr->getId() << "\"]);" << endl;
        break;
      case Builtin::KindByte:
        s << TAB << pPtr->getId() << " = KT_Common::strto<kant::" << (bPtr->isUnsigned() ? "UInt8" : "Char")
          << ">(_mysrd[\"" << pPtr->getId() << "\"]);" << endl;
        break;
      case Builtin::KindShort:
        s << TAB << pPtr->getId() << " = KT_Common::strto<kant::" << (bPtr->isUnsigned() ? "UInt16" : "Short")
          << ">(_mysrd[\"" << pPtr->getId() << "\"]);" << endl;
        break;
      case Builtin::KindInt:
        s << TAB << pPtr->getId() << " = KT_Common::strto<kant::" << (bPtr->isUnsigned() ? "UInt32" : "Int32")
          << ">(_mysrd[\"" << pPtr->getId() << "\"]);" << endl;
        break;
      case Builtin::KindLong:
        s << TAB << pPtr->getId() << " = KT_Common::strto<kant::" << (bPtr->isUnsigned() ? "UInt64" : "Int64")
          << ">(_mysrd[\"" << pPtr->getId() << "\"]);" << endl;
        break;
      case Builtin::KindFloat:
        s << TAB << pPtr->getId() << " = KT_Common::strto<kant::Float>(_mysrd[\"" << pPtr->getId() << "\"]);" << endl;
        break;
      case Builtin::KindDouble:
        s << TAB << pPtr->getId() << " = KT_Common::strto<kant::Double>(_mysrd[\"" << pPtr->getId() << "\"]);" << endl;
        break;
      case Builtin::KindString:
        s << TAB << pPtr->getId() << " = KT_Common::trim(_mysrd[\"" << pPtr->getId() << "\"]);" << endl;
        break;
      default:
        break;
    }
  } else if (!pPtr->getTypePtr()->isSimple()) {
    s << TAB << "kant::JsonInput::readJson(" << pPtr->getId() << ", kant::KT_Json::getValue(_mysrd[\"" << pPtr->getId()
      << "\"]), false);" << endl;
  }

  return s.str();
}

string Kant2Cpp::writeToJson(const TypeIdPtr& pPtr) const {
  ostringstream s;
  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    s << TAB << "p->value[\"" << pPtr->getId()
      << "\"] = " + _namespace + "::JsonOutput::writeJson((" + _namespace + "::Int32)" << pPtr->getId() << ");" << endl;
  } else if (pPtr->getTypePtr()->isArray()) {
    /*
        s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::JsonOutput::writeJson((const "
            << tostr(pPtr->getTypePtr()) << " *)" << pPtr->getId() << "Len"  << ");" << endl;
        */
    s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::JsonOutput::writeJson((const "
      << tostr(pPtr->getTypePtr()) << " *)" << pPtr->getId() << ", " << pPtr->getId() << "Len"
      << ");" << endl;
  } else if (pPtr->getTypePtr()->isPointer()) {
    /*
        s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::JsonOutput::writeJson((const "
            << tostr(pPtr->getTypePtr()) << " )" << pPtr->getId() << "Len"  << ");" << endl;
        */
    s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::JsonOutput::writeJson((const "
      << tostr(pPtr->getTypePtr()) << " )" << pPtr->getId() << ", " << pPtr->getId() << "Len"
      << ");" << endl;

  } else {
    MapPtr mPtr = std::dynamic_pointer_cast<Map>(pPtr->getTypePtr());
    VectorPtr vPtr = std::dynamic_pointer_cast<Vector>(pPtr->getTypePtr());

    // 对于json， 不检测默认值
    if (true || !_checkDefault || pPtr->isRequire() || (!pPtr->hasDefault() && !mPtr && !vPtr)) {
      s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::JsonOutput::writeJson(" << pPtr->getId()
        << ");" << endl;
    } else {
      string sDefault = pPtr->def();

      BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getTypePtr());
      if (bPtr && bPtr->kind() == Builtin::KindString) {
        sDefault = "\"" + kant::KT_Common::replace(pPtr->def(), "\"", "\\\"") + "\"";
      }

      if (mPtr || vPtr) {
        s << TAB << "if (" << pPtr->getId() << ".size() > 0)" << endl;
      } else if (bPtr && (bPtr->kind() == Builtin::KindFloat || bPtr->kind() == Builtin::KindDouble)) {
        s << TAB << "if (!kant::KT_Common::equal(" << pPtr->getId() << "," << sDefault << "))" << endl;
      } else {
        s << TAB << "if (" << pPtr->getId() << " != " << sDefault << ")" << endl;
      }

      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB << "p->value[\"" << pPtr->getId() << "\"] = " + _namespace + "::JsonOutput::writeJson(" << pPtr->getId()
        << ");" << endl;
      DEL_TAB;
      s << TAB << "}" << endl;
    }
  }

  return s.str();
}

string Kant2Cpp::readFromJson(const TypeIdPtr& pPtr, bool bIsRequire) const {
  ostringstream s;
  // if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr()))
  // {
  //     s << TAB << _namespace + "::JsonInput::readJson((" + _namespace + "::Int32&)" << pPtr->getId() << ",pObj->value[\"" << pPtr->getId() << "\"]";
  // }
  // else

  if (pPtr->getTypePtr()->isArray()) {
    //s << TAB << _namespace + "::JsonInput::readJson(" << pPtr->getId() << "Len" << ",pObj->value[\"" << pPtr->getId() << "\"]" << getSuffix(pPtr);
    s << TAB << _namespace + "::JsonInput::readJson(" << pPtr->getId() << ", "
      << "sizeof(" + pPtr->getId() + ")"
      << ", " << pPtr->getId() << "Len"
      << ", pObj->value[\"" << pPtr->getId() << "\"]";
  } else if (pPtr->getTypePtr()->isPointer()) {
#if 0
        s << TAB << pPtr->getId() <<" = ("<<tostr(pPtr->getTypePtr())<<")_is.cur();"<<endl;
        s << TAB << "_is.read("<< pPtr->getId()<<", _is.left(), "<< pPtr->getId() << "Len";
#endif
    s << TAB << "not support";
  } else {
    s << TAB << _namespace + "::JsonInput::readJson(" << pPtr->getId() << ",pObj->value[\"" << pPtr->getId() << "\"]";
  }
  s << ", " << ((pPtr->isRequire() && bIsRequire) ? "true" : "false") << ");" << endl;

#if 0
    if(pPtr->getTypePtr()->isPointer())
    s << TAB <<"_is.mapBufferSkip("<<pPtr->getId() << "Len);"<<endl;
#endif

  return s.str();
}

string Kant2Cpp::writeTo(const TypeIdPtr& pPtr) const {
  ostringstream s;
  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    s << TAB << "_os.write((" + _namespace + "::Int32)" << pPtr->getId() << ", " << pPtr->getTag() << ");" << endl;
  } else if (pPtr->getTypePtr()->isArray()) {
    s << TAB << "_os.write((const " << tostr(pPtr->getTypePtr()) << " *)" << pPtr->getId() << ", " << pPtr->getId()
      << "Len"
      << ", " << pPtr->getTag() << ");" << endl;
  } else if (pPtr->getTypePtr()->isPointer()) {
    s << TAB << "_os.write((const " << tostr(pPtr->getTypePtr()) << ")" << pPtr->getId() << ", " << pPtr->getId()
      << "Len"
      << ", " << pPtr->getTag() << ");" << endl;
  } else {
    MapPtr mPtr = std::dynamic_pointer_cast<Map>(pPtr->getTypePtr());
    VectorPtr vPtr = std::dynamic_pointer_cast<Vector>(pPtr->getTypePtr());

    if (!_checkDefault || pPtr->isRequire() || (!pPtr->hasDefault() && !mPtr && !vPtr)) {
      s << TAB << "_os.write(" << pPtr->getId() << ", " << pPtr->getTag() << ");" << endl;
    } else {
      string sDefault = pPtr->def();

      BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getTypePtr());
      if (bPtr && bPtr->kind() == Builtin::KindString) {
        sDefault = "\"" + kant::KT_Common::replace(pPtr->def(), "\"", "\\\"") + "\"";
      }

      if (mPtr || vPtr) {
        s << TAB << "if (" << pPtr->getId() << ".size() > 0)" << endl;
      } else {
        s << TAB << "if (" << pPtr->getId() << " != " << sDefault << ")" << endl;
      }

      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB << "_os.write(" << pPtr->getId() << ", " << pPtr->getTag() << ");" << endl;
      DEL_TAB;
      s << TAB << "}" << endl;
    }
  }

  return s.str();
}

string Kant2Cpp::readFrom(const TypeIdPtr& pPtr, bool bIsRequire) const {
  ostringstream s;
  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    //枚举强制类型转换在O2编译选项情况下会告警
    string tmp = _namespace + "::Int32 eTemp" + KT_Common::tostr(pPtr->getTag()) + generateInitValue(pPtr);

    s << TAB << tmp << endl;
    s << TAB << "_is.read(eTemp" << KT_Common::tostr(pPtr->getTag());
  } else if (pPtr->getTypePtr()->isArray()) {
    //s << TAB << "_is.read(" << pPtr->getId() << ", " << getSuffix(pPtr) << ", " << pPtr->getId() << "Len";
    s << TAB << "_is.read(" << pPtr->getId() << ", "
      << "sizeof(" + pPtr->getId() + ")"
      << ", " << pPtr->getId() << "Len";
  } else if (pPtr->getTypePtr()->isPointer()) {
    s << TAB << pPtr->getId() << " = (" << tostr(pPtr->getTypePtr()) << ")_is.cur();" << endl;
    s << TAB << "_is.read(" << pPtr->getId() << ", _is.left(), " << pPtr->getId() << "Len";
  } else {
    s << TAB << "_is.read(" << pPtr->getId();
  }

  s << ", " << pPtr->getTag() << ", " << ((pPtr->isRequire() && bIsRequire) ? "true" : "false") << ");" << endl;

  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    s << TAB << pPtr->getId() << " = (" << tostr(pPtr->getTypePtr()) << ")eTemp" << KT_Common::tostr(pPtr->getTag())
      << ";" << endl;
  }

  if (pPtr->getTypePtr()->isPointer()) s << TAB << "_is.mapBufferSkip(" << pPtr->getId() << "Len);" << endl;

  return s.str();
}

// string Kant2Cpp::readUnknown(const TypeIdPtr& pPtr) const
// {
//     ostringstream s;
//     s << TAB << "_is.readUnknown(sUnknownField, " <<  pPtr->getTag() << ");" << endl;
//     return s.str();
// }
// string Kant2Cpp::writeUnknown() const
// {
//     ostringstream s;
//     s << TAB << "_os.writeUnknown(sUnknownField);" << endl;
//     return s.str();
// }

string Kant2Cpp::display(const TypeIdPtr& pPtr) const {
  ostringstream s;
  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    s << TAB << "_ds.display((" + _namespace + "::Int32)" << pPtr->getId() << ",\"" << pPtr->getId() << "\");" << endl;
    ;
  } else if (pPtr->getTypePtr()->isArray() || pPtr->getTypePtr()->isPointer()) {
    s << TAB << "_ds.display(" << pPtr->getId() << ", " << pPtr->getId() << "Len"
      << ",\"" << pPtr->getId() << "\");" << endl;
  } else {
    s << TAB << "_ds.display(" << pPtr->getId() << ",\"" << pPtr->getId() << "\");" << endl;
    ;
  }

  return s.str();
}

string Kant2Cpp::displaySimple(const TypeIdPtr& pPtr, bool bSep) const {
  ostringstream s;
  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    s << TAB << "_ds.displaySimple((" + _namespace + "::Int32)" << pPtr->getId() << ", " << (bSep ? "true" : "false")
      << ");" << endl;
  } else if (pPtr->getTypePtr()->isArray()) {
    s << TAB << "_ds.displaySimple(" << pPtr->getId() << ", " << pPtr->getId() << "Len"
      << "," << (bSep ? "true" : "false") << ");" << endl;
  } else if (pPtr->getTypePtr()->isPointer()) {
    s << TAB << "_ds.displaySimple(";
    s << (bSep ? "" : ("(const " + tostr(pPtr->getTypePtr()) + ")"));
    s << pPtr->getId() << ", " << pPtr->getId() << "Len"
      << "," << (bSep ? "true" : "false") << ");" << endl;
  } else {
    s << TAB << "_ds.displaySimple(" << pPtr->getId() << ", " << (bSep ? "true" : "false") << ");" << endl;
  }

  return s.str();
}

string Kant2Cpp::generateCollection(const TypeIdPtr& pPtr, bool bSep) const {
  ostringstream s;
  if (pPtr->getTypePtr()->isArray()) {
    s << TAB << "_jj.generateCollection("
      << "\"" << pPtr->getId() << "\""
      << ", " << pPtr->getId() << ", " << pPtr->getId() << "Len"
      << ");"
      << "\n"
      << (bSep ? TAB + "_jj.append(\",\", false);" : "") << endl;
  } else {
    s << TAB << "_jj.generateCollection("
      << "\"" << pPtr->getId() << "\""
      << ", " << pPtr->getId() << ");"
      << "\n"
      << (bSep ? TAB + "_jj.append(\",\", false);" : "") << endl;
  }

  return s.str();
}

/*******************************获取定长数组坐标********************************/
int Kant2Cpp::getSuffix(const TypeIdPtr& pPtr) const {
  BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getTypePtr());
  if (bPtr && bPtr->kind() == Builtin::KindString && bPtr->isArray()) {
    return bPtr->getSize();
  }

  VectorPtr vPtr = std::dynamic_pointer_cast<Vector>(pPtr->getTypePtr());
  if (vPtr && vPtr->isArray()) {
    return vPtr->getSize();
  }

  return -1;
}

/*******************************定长数组坐标********************************/

string Kant2Cpp::toStrSuffix(const TypeIdPtr& pPtr) const {
  ostringstream s;

  int i = getSuffix(pPtr);

  if (i >= 0) {
    s << "[" << i << "]";
  }
  return s.str();
}
/*******************************BuiltinPtr********************************/

string Kant2Cpp::tostr(const TypePtr& pPtr) const {
  BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(pPtr);
  if (bPtr) return tostrBuiltin(bPtr);

  VectorPtr vPtr = std::dynamic_pointer_cast<Vector>(pPtr);
  if (vPtr) return tostrVector(vPtr);

  MapPtr mPtr = std::dynamic_pointer_cast<Map>(pPtr);
  if (mPtr) return tostrMap(mPtr);

  StructPtr sPtr = std::dynamic_pointer_cast<Struct>(pPtr);
  if (sPtr) return tostrStruct(sPtr);

  EnumPtr ePtr = std::dynamic_pointer_cast<Enum>(pPtr);
  if (ePtr) return tostrEnum(ePtr);

  if (!pPtr) return "void";

  assert(false);
  return "";
}

string Kant2Cpp::tostrBuiltin(const BuiltinPtr& pPtr) const {
  string s;

  switch (pPtr->kind()) {
    case Builtin::KindBool:
      s = _namespace + "::Bool";
      break;
    case Builtin::KindByte:
      s = _namespace + "::Char";
      break;
    case Builtin::KindShort:
      //为了兼容java无unsigned, 编解码时把kant文件中 unsigned char 对应到short
      //c++中需要还原回来
      s = (pPtr->isUnsigned() ? _namespace + "::UInt8" : _namespace + "::Short");
      break;
    case Builtin::KindInt:
      s = (pPtr->isUnsigned() ? _namespace + "::UInt16" : _namespace + "::Int32");
      break;
    case Builtin::KindLong:
      s = (pPtr->isUnsigned() ? _namespace + "::" + "UInt32" : _namespace + "::Int64");
      break;
    case Builtin::KindFloat:
      s = _namespace + "::Float";
      break;
    case Builtin::KindDouble:
      s = _namespace + "::Double";
      break;
    case Builtin::KindString:
      if (pPtr->isArray())
        s = _namespace + "::Char";  //char a [8];
      else
        s = "std::string";  //string a;
      break;
    case Builtin::KindVector:
      s = "std::vector";
      break;
    case Builtin::KindMap:
      s = "std::map";
      break;
    default:
      assert(false);
      break;
  }

  return s;
}
/*******************************VectorPtr********************************/
string Kant2Cpp::tostrVector(const VectorPtr& pPtr) const {
  //数组类型
  if (pPtr->isArray()) {
    return tostr(pPtr->getTypePtr());
  }

  //指针类型
  if (pPtr->isPointer()) {
    return tostr(pPtr->getTypePtr()) + " *";
  }

  string s = Builtin::builtinTable[Builtin::KindVector] + string("<") + tostr(pPtr->getTypePtr());

  if (std::dynamic_pointer_cast<Map>(pPtr->getTypePtr()) || std::dynamic_pointer_cast<Vector>(pPtr->getTypePtr())) {
    s += " >";
  } else {
    s += ">";
  }
  return s;
}
/*******************************MapPtr********************************/
string Kant2Cpp::tostrMap(const MapPtr& pPtr) const {
  string s = Builtin::builtinTable[Builtin::KindMap] + string("<") + tostr(pPtr->getLeftTypePtr()) + ", " +
             tostr(pPtr->getRightTypePtr());
  if (std::dynamic_pointer_cast<Map>(pPtr->getRightTypePtr()) ||
      std::dynamic_pointer_cast<Vector>(pPtr->getRightTypePtr())) {
    s += " >";
  } else {
    s += ">";
  }
  return s;
}

/*******************************StructPtr********************************/
string Kant2Cpp::tostrStruct(const StructPtr& pPtr) const { return pPtr->getSid(); }

string Kant2Cpp::MD5(const StructPtr& pPtr) const {
  string s;
  vector<TypeIdPtr>& member = pPtr->getAllMemberPtr();
  for (size_t j = 0; j < member.size(); j++) {
    s += "_" + tostr(member[j]->getTypePtr());
  }

  return "\"" + kant::KT_MD5::md5str(s) + "\"";
}

/////////////////////////////////////////////////////////////////////
string Kant2Cpp::tostrEnum(const EnumPtr& pPtr) const { return pPtr->getSid(); }
///////////////////////////////////////////////////////////////////////
string Kant2Cpp::generateH(const StructPtr& pPtr, const string& namespaceId) const {
  ostringstream s;

  s << TAB << "struct " << pPtr->getId() << " : public " + _namespace + "::KantStructBase" << endl;
  s << TAB << "{" << endl;

  s << TAB << "public:" << endl;

  INC_TAB;

  s << TAB << "static string className()" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "return "
    << "\"" << namespaceId << "." << pPtr->getId() << "\""
    << ";" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << TAB << "static string MD5()" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "return " << MD5(pPtr) << ";" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  ////////////////////////////////////////////////////////////
  //定义缺省构造函数
  s << TAB << pPtr->getId() << "()" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "resetDefautlt();" << endl;
  vector<TypeIdPtr>& member = pPtr->getAllMemberPtr();

  /*
    bool b = false;
    //定义初始化列表
    for (size_t j = 0; j < member.size(); j++)
    {
        if (member[j]->getTypePtr()->isArray())
        {
            if (!b) s << TAB << ":";
            else s << ",";
            s << member[j]->getId() << "Len(0)";
            b = true;
            continue;
        }

        if (member[j]->getTypePtr()->isPointer())
        {
            if (!b) s << TAB << ":";
            else s << ",";
            s << member[j]->getId() << "Len(0)," << member[j]->getId() << "(NULL)";
            b = true;
            continue;
        }

        if (member[j]->hasDefault())
        {
            if (!b) s << TAB << ":";
            else s << ",";

            BuiltinPtr bPtr  = std::dynamic_pointer_cast<Builtin>(member[j]->getTypePtr());
            //string值要转义
            if (bPtr && bPtr->kind() == Builtin::KindString)
            {
                string tmp = kant::KT_Common::replace(member[j]->def(), "\"", "\\\"");
                s << member[j]->getId() << "(\"" << tmp << "\")";
            }
            else
            {
                s << member[j]->getId() << "(" << member[j]->def() << ")";
            }
            b = true;
        }
        else
        {   //没有提供初始值才会走到这里,提供枚举类型初始化值
            EnumPtr ePtr = std::dynamic_pointer_cast<Enum>(member[j]->getTypePtr());
            if (ePtr)
            {
                vector<TypeIdPtr>& eMember = ePtr->getAllMemberPtr();
                if (eMember.size() > 0)
                {
                    if (!b) s << TAB << ":";
                    else s << ",";
                    string sid = ePtr->getSid();
                    s << member[j]->getId() << "(" << sid.substr(0, sid.find_first_of("::")) << "::" << eMember[0]->getId() << ")";
                    b = true;
                }
            }
        }
    }

    // if (b)
    // {
    // 	if (_unknownField)
    // 	{
    //     	s << ",sUnknownField(\"\")";
    // 	}
    //     s << endl;
    // }

    s << TAB << "{" << endl;
    INC_TAB;
    for (size_t j = 0; j < member.size(); j++)
    {
        VectorPtr vPtr   = std::dynamic_pointer_cast<Vector>(member[j]->getTypePtr());
        if (vPtr)
        {
            BuiltinPtr bPtr  = std::dynamic_pointer_cast<Builtin>(vPtr->getTypePtr());
            if (!bPtr ||  bPtr->kind() == Builtin::KindString) //非内建类型或者string 类型不能memset
            {
                continue;
            }
        }
        if (!member[j]->getTypePtr()->isArray())
        {
            continue;
        }
        s << TAB << "memset(" << member[j]->getId() << ", 0, " << "sizeof(" << member[j]->getId() << "));" << endl;
    }

    */

  if (_bXmlSupport) {
    s << TAB << "_cdata_format = false;" << endl;
  }

  DEL_TAB;
  s << TAB << "}" << endl;

  //resetDefault()函数
  s << TAB << "void resetDefautlt()" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  member = pPtr->getAllMemberPtr();
  for (size_t j = 0; j < member.size(); j++) {
    if (member[j]->getTypePtr()->isArray()) {
      s << TAB << member[j]->getId() << "Len = 0;" << endl;
      VectorPtr vPtr = std::dynamic_pointer_cast<Vector>(member[j]->getTypePtr());
      if (vPtr) {
        BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(vPtr->getTypePtr());
        if (bPtr && bPtr->kind() != Builtin::KindString)  //非内建类型或者string 类型不能memset
        {
          s << TAB << "memset(" << member[j]->getId() << ", 0, "
            << "sizeof(" << member[j]->getId() << "));" << endl;
        }
      }
      continue;
    }

    if (member[j]->getTypePtr()->isPointer()) {
      s << TAB << member[j]->getId() << "Len = 0;" << endl;
      s << TAB << member[j]->getId() << " = NULL;" << endl;
      continue;
    }

    VectorPtr vPtr = std::dynamic_pointer_cast<Vector>(member[j]->getTypePtr());
    MapPtr mPtr = std::dynamic_pointer_cast<Map>(member[j]->getTypePtr());
    // 如果是vector或者map，reset时需要调用clear方法
    if (vPtr || mPtr) {
      s << TAB << member[j]->getId() << ".clear();" << endl;
      continue;
    }

    StructPtr sPtr = std::dynamic_pointer_cast<Struct>(member[j]->getTypePtr());
    // 如果是结构体，那么需要调用resetDefautlt方法
    if (sPtr) {
      s << TAB << member[j]->getId() << ".resetDefautlt();" << endl;
      continue;
    }

    if (member[j]->hasDefault()) {
      BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(member[j]->getTypePtr());
      //string值要转义
      if (bPtr && bPtr->kind() == Builtin::KindString) {
        string tmp = kant::KT_Common::replace(member[j]->def(), "\"", "\\\"");
        s << TAB << member[j]->getId() << " = \"" << tmp << "\";" << endl;
      } else {
        s << TAB << member[j]->getId() << " = " << member[j]->def() << ";" << endl;
      }
    } else {  //没有提供初始值才会走到这里,提供枚举类型初始化值
      EnumPtr ePtr = std::dynamic_pointer_cast<Enum>(member[j]->getTypePtr());
      if (ePtr) {
        vector<TypeIdPtr>& eMember = ePtr->getAllMemberPtr();
        if (eMember.size() > 0) {
          string sid = ePtr->getSid();
          s << TAB << member[j]->getId() << " = " << sid.substr(0, sid.find_first_of("::"))
            << "::" << eMember[0]->getId() << ";" << endl;
        }
      }
    }
  }

  DEL_TAB;
  s << TAB << "}" << endl;

  s << TAB << "template<typename WriterT>" << endl;
  s << TAB << "void writeTo(" + _namespace + "::KantOutputStream<WriterT>& _os) const" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  for (size_t j = 0; j < member.size(); j++) {
    s << writeTo(member[j]);
  }
  // if (_unknownField)
  // {
  // 	s << writeUnknown();
  // }
  DEL_TAB;
  s << TAB << "}" << endl;

  ///////////////////////////////////////////////////////////
  s << TAB << "template<typename ReaderT>" << endl;
  s << TAB << "void readFrom(" + _namespace + "::KantInputStream<ReaderT>& _is)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "resetDefautlt();" << endl;
  for (size_t j = 0; j < member.size(); j++) {
    s << readFrom(member[j]);
  }
  // if (_unknownField)
  // {
  //     s << readUnknown(member[member.size() - 1]);
  // }

  DEL_TAB;
  s << TAB << "}" << endl;

  if (_bJsonSupport) {
    s << TAB << "kant::JsonValueObjPtr writeToJson() const" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "kant::JsonValueObjPtr p = std::make_shared<kant::JsonValueObj>();" << endl;
    for (size_t j = 0; j < member.size(); j++) {
      s << writeToJson(member[j]);
    }
    s << TAB << "return p;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    s << TAB << "string writeToJsonString() const" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "return kant::KT_Json::writeValue(writeToJson());" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    s << TAB << "void readFromJson(const kant::JsonValuePtr & p, bool isRequire = true)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "resetDefautlt();" << endl;
    s << TAB << "if(NULL == p.get() || p->getType() != kant::eJsonTypeObj)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "char s[128];" << endl;
    s << TAB << "snprintf(s, sizeof(s), \"read 'struct' type mismatch, get type: %d.\", (p.get() ? p->getType() : 0));"
      << endl;
    s << TAB << "throw kant::KT_Json_Exception(s);" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB << "kant::JsonValueObjPtr pObj=std::dynamic_pointer_cast<kant::JsonValueObj>(p);" << endl;
    for (size_t j = 0; j < member.size(); j++) {
      s << readFromJson(member[j]);
    }
    DEL_TAB;
    s << TAB << "}" << endl;

    s << TAB << "void readFromJsonString(const string & str)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "readFromJson(kant::KT_Json::getValue(str));" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
  }

  if (_bXmlSupport) {
    s << TAB << "void setXmlFormat(bool cdata = false)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "_cdata_format = cdata;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    s << TAB << "kant::XmlValueObjPtr writeToXml() const" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "kant::XmlValueObjPtr p = std::make_shared<kant::XmlValueObj>();" << endl;
    for (size_t j = 0; j < member.size(); j++) {
      s << writeToXml(member[j]);
    }
    s << TAB << "return p;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    s << TAB << "string writeToXmlString() const" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "return kant::KT_Xml::writeValue(writeToXml());" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    s << TAB << "void readFromXml(const kant::XmlValuePtr & p, bool isRequire = true)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "resetDefautlt();" << endl;
    s << TAB << "if(NULL == p.get() || p->getType() != eXmlTypeObj)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "char s[128];" << endl;
    s << TAB << "snprintf(s, sizeof(s), \"read 'struct' type mismatch, get type: %d.\", p->getType());" << endl;
    s << TAB << "throw KT_Xml_Exception(s);" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB << "kant::XmlValueObjPtr pObj= std::dynamic_pointer_cast<kant::XmlValueObj>(p);" << endl;
    for (size_t j = 0; j < member.size(); j++) {
      s << readFromXml(member[j]);
    }
    DEL_TAB;
    s << TAB << "}" << endl;

    s << TAB << "void readFromXmlString(const string & str)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "readFromXml(kant::KT_Xml::getValue(str));" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
  }

  if (_bSqlSupport) {
    s << TAB << "kant::KT_Mysql::RECORD_DATA& toSql(kant::KT_Mysql::RECORD_DATA& _mycols) const" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    for (size_t j = 0; j < member.size(); j++) {
      s << writeToSql(member[j]);
    }
    s << TAB << "return _mycols;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB << "void fromSql(kant::KT_Mysql::MysqlRecord& _mysrd)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "resetDefautlt();" << endl;
    for (size_t j = 0; j < member.size(); j++) {
      s << readFromSql(member[j]);
    }
    DEL_TAB;
    s << TAB << "}" << endl;
  }

  s << TAB << "ostream& display(ostream& _os, int _level=0) const" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << _namespace + "::KantDisplayer _ds(_os, _level);" << endl;

  for (size_t j = 0; j < member.size(); j++) {
    s << display(member[j]);
  }
  s << TAB << "return _os;" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  s << TAB << "ostream& displaySimple(ostream& _os, int _level=0) const" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << _namespace + "::KantDisplayer _ds(_os, _level);" << endl;

  for (size_t j = 0; j < member.size(); j++) {
    s << displaySimple(member[j], (j != member.size() - 1 ? true : false));
  }
  s << TAB << "return _os;" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  DEL_TAB;
  s << TAB << "public:" << endl;
  INC_TAB;

  //定义成员变量
  for (size_t j = 0; j < member.size(); j++) {
    if (member[j]->getTypePtr()->isArray() || member[j]->getTypePtr()->isPointer())  //数组类型、指针类型需要定义长度
    {
      s << TAB << _namespace + "::"
        << "UInt32 " << member[j]->getId() << "Len"
        << ";" << endl;
    }
    s << TAB << tostr(member[j]->getTypePtr()) << " " << member[j]->getId() << toStrSuffix(member[j]) << ";" << endl;
  }

  if (_bXmlSupport) {
    DEL_TAB;
    s << TAB << "private:" << endl;
    INC_TAB;
    s << TAB << "bool _cdata_format;" << endl;
  }

  // if  (_unknownField)
  // {
  //     s << TAB << "std::string sUnknownField;" << endl;
  // }
  DEL_TAB;
  s << TAB << "};" << endl;

  //定义==操作
  s << TAB << "inline bool operator==(const " << pPtr->getId() << "&l, const " << pPtr->getId() << "&r)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "return ";
  for (size_t j = 0; j < member.size(); j++) {
    if (member[j]->getTypePtr()->isArray() || member[j]->getTypePtr()->isPointer())  //数组类型、指针类型
    {
      s << "!memcmp(l." << member[j]->getId() << ",r." << member[j]->getId() << ",l." << member[j]->getId() << "Len)";
    } else {
      BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(member[j]->getTypePtr());
      MapPtr mPtr = std::dynamic_pointer_cast<Map>(member[j]->getTypePtr());
      VectorPtr vPtr = std::dynamic_pointer_cast<Vector>(member[j]->getTypePtr());

      bool mapDouble = false;
      if (mPtr) {
        {
          BuiltinPtr innerPtr = std::dynamic_pointer_cast<Builtin>(mPtr->getLeftTypePtr());
          if (innerPtr && (innerPtr->kind() == Builtin::KindFloat || innerPtr->kind() == Builtin::KindDouble)) {
            mapDouble = true;
          }
        }
        {
          BuiltinPtr innerPtr = std::dynamic_pointer_cast<Builtin>(mPtr->getRightTypePtr());
          if (innerPtr && (innerPtr->kind() == Builtin::KindFloat || innerPtr->kind() == Builtin::KindDouble)) {
            mapDouble = true;
          }
        }
      }
      bool vecDouble = false;
      //vector比较
      if (vPtr) {
        BuiltinPtr innerPtr = std::dynamic_pointer_cast<Builtin>(vPtr->getTypePtr());
        if (innerPtr && (innerPtr->kind() == Builtin::KindFloat || innerPtr->kind() == Builtin::KindDouble)) {
          vecDouble = true;
        }
      }
      if (mapDouble || vecDouble) {
        s << "kant::KT_Common::equal("
          << "l." << member[j]->getId() << ","
          << "r." << member[j]->getId() << ")";
      } else if (bPtr && (bPtr->kind() == Builtin::KindFloat || bPtr->kind() == Builtin::KindDouble)) {
        s << "kant::KT_Common::equal("
          << "l." << member[j]->getId() << ","
          << "r." << member[j]->getId() << ")";
      } else {
        s << "l." << member[j]->getId() << " == r." << member[j]->getId();
      }
      // s << "l." << member[j]->getId() << " == r." << member[j]->getId();
    }
    if (j != member.size() - 1) {
      s << " && ";
    }
  }
  s << ";" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  //定义!=
  s << TAB << "inline bool operator!=(const " << pPtr->getId() << "&l, const " << pPtr->getId() << "&r)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "return !(l == r);" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  //定义 << >>
  if (_bJsonSupport) {
    //重载 <<
    s << TAB << "inline ostream& operator<<(ostream & os,const " << pPtr->getId() << "&r)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "os << r.writeToJsonString();" << endl;
    s << TAB << "return os;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    //重载 >>
    s << TAB << "inline istream& operator>>(istream& is," << pPtr->getId() << "&l)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "std::istreambuf_iterator<char> eos;" << endl;
    s << TAB << "std::string s(std::istreambuf_iterator<char>(is), eos);" << endl;
    s << TAB << "l.readFromJsonString(s);" << endl;
    s << TAB << "return is;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
  }

  vector<string> key = pPtr->getKey();
  //定义<
  if (key.size() > 0) {
    s << TAB << "inline bool operator<(const " << pPtr->getId() << "&l, const " << pPtr->getId() << "&r)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    for (size_t i = 0; i < key.size(); i++) {
      s << TAB << "if(l." << key[i] << " != r." << key[i] << ") ";
      for (size_t z = 0; z < member.size(); z++) {
        if (key[i] == member[z]->getId() &&
            (member[z]->getTypePtr()->isArray() || member[z]->getTypePtr()->isPointer()))  //数组类型、指针类型
        {
          s << "memcmp(l." << key[i] << ",r." << key[i] << ",l." << key[i] << "Len)< 0";
        }
      }
      s << " return (l." << key[i] << " < r." << key[i] << ");" << endl;
    }

    s << TAB << "return false;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    //定义<=
    s << TAB << "inline bool operator<=(const " << pPtr->getId() << "&l, const " << pPtr->getId() << "&r)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "return !(r < l);" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    //定义>
    s << TAB << "inline bool operator>(const " << pPtr->getId() << "&l, const " << pPtr->getId() << "&r)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "return r < l;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    //定义>=
    s << TAB << "inline bool operator>=(const " << pPtr->getId() << "&l, const " << pPtr->getId() << "&r)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "return !(l < r);" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
  }

  return s.str();
}

/*******************************ContainerPtr********************************/
string Kant2Cpp::generateH(const ContainerPtr& pPtr) const {
  ostringstream s;
  for (size_t i = 0; i < pPtr->getAllNamespacePtr().size(); i++) {
    s << generateH(pPtr->getAllNamespacePtr()[i]) << endl;
    s << endl;
  }
  return s.str();
}

/******************************ParamDeclPtr***************************************/
string Kant2Cpp::generateH(const ParamDeclPtr& pPtr) const {
  ostringstream s;

  //输出参数, 或简单类型
  if (pPtr->isOut() || pPtr->getTypeIdPtr()->getTypePtr()->isSimple()) {
    s << tostr(pPtr->getTypeIdPtr()->getTypePtr());
  } else {
    //结构, map, vector, string
    s << "const " << tostr(pPtr->getTypeIdPtr()->getTypePtr()) << " &";
  }

  if (pPtr->isOut()) {
    s << " &";
  } else {
    s << " ";
  }
  s << pPtr->getTypeIdPtr()->getId();

  return s.str();
}

string Kant2Cpp::generateOutH(const ParamDeclPtr& pPtr) const {
  if (!pPtr->isOut()) return "";

  ostringstream s;

  //输出参数, 或简单类型
  if (pPtr->getTypeIdPtr()->getTypePtr()->isSimple()) {
    s << tostr(pPtr->getTypeIdPtr()->getTypePtr()) << " ";
  } else {
    //结构, map, vector, string
    s << "const " << tostr(pPtr->getTypeIdPtr()->getTypePtr()) << " &";
  }
  s << pPtr->getTypeIdPtr()->getId();

  return s.str();
}

string Kant2Cpp::generateParamDecl(const ParamDeclPtr& pPtr) const {
  ostringstream s;

  if (pPtr->isOut() || pPtr->getTypeIdPtr()->getTypePtr()->isSimple()) {
    s << tostr(pPtr->getTypeIdPtr()->getTypePtr());

    if (pPtr->isOut())
      s << " &";
    else
      s << " ";
  } else {
    //输入参数
    s << "const " << tostr(pPtr->getTypeIdPtr()->getTypePtr()) << " &";
  }

  s << pPtr->getTypeIdPtr()->getId();

  return s.str();
}

string Kant2Cpp::generateDispatchAsync(const OperationPtr& pPtr, const string& cn) const {
  ostringstream s;
  s << TAB << "if (msg->response->iRet != kant::KANTSERVERSUCCESS)" << endl << TAB << "{" << endl;

  INC_TAB;
  s << TAB << "callback_" << pPtr->getId() << "_exception(msg->response->iRet);" << endl;
  s << endl;

  s << TAB << "return msg->response->iRet;" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  s << TAB << _namespace + "::KantInputStream<" + _namespace + "::BufferReader> _is;" << endl;
  s << endl;
  vector<ParamDeclPtr>& vParamDecl = pPtr->getAllParamDeclPtr();

  s << TAB << "_is.setBuffer(msg->response->sBuffer);" << endl;

  //对输出参数编码
  if (pPtr->getReturnPtr()->getTypePtr()) {
    s << TAB << tostr(pPtr->getReturnPtr()->getTypePtr()) << " " << pPtr->getReturnPtr()->getId()
      << generateInitValue(pPtr->getReturnPtr()) << ";" << endl;
    s << readFrom(pPtr->getReturnPtr()) << endl;
  }

  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (vParamDecl[i]->isOut()) {
      s << TAB << tostr(vParamDecl[i]->getTypeIdPtr()->getTypePtr()) << " " << vParamDecl[i]->getTypeIdPtr()->getId()
        << generateInitValue(vParamDecl[i]->getTypeIdPtr()) << ";" << endl;
      s << readFrom(vParamDecl[i]->getTypeIdPtr());
    }
  }

  // 处理调用链
  if (_bTrace) {
    s << TAB << "ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();" << endl;
    s << TAB << "if (pSptd && pSptd->_traceCall)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "string _trace_param_;" << endl;
    s << TAB
      << "int _trace_param_flag_ = pSptd->needTraceParam(ServantProxyThreadData::TraceContext::EST_CR, _is.size());"
      << endl;
    s << TAB << "if (ServantProxyThreadData::TraceContext::ENP_NORMAL == _trace_param_flag_)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << _namespace << "::JsonValueObjPtr _p_ = std::make_shared<" << _namespace << "::JsonValueObj>();" << endl;
    if (pPtr->getReturnPtr()->getTypePtr()) {
      s << TAB << "_p_->value[\"\"] = " << _namespace << "::JsonOutput::writeJson(_ret);" << endl;
    }
    for (size_t i = 0; i < vParamDecl.size(); i++) {
      if (vParamDecl[i]->isOut()) {
        s << TAB << "_p_->value[\"" << vParamDecl[i]->getTypeIdPtr()->getId() << "\"] = " << _namespace
          << "::JsonOutput::writeJson(" << vParamDecl[i]->getTypeIdPtr()->getId() << ");" << endl;
      }
    }
    s << TAB << "_trace_param_ = " + _namespace + "::KT_Json::writeValue(_p_);" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB << "else if(ServantProxyThreadData::TraceContext::ENP_OVERMAXLEN == _trace_param_flag_)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "_trace_param_ = " << G_TRACE_PARAM_OVER_MAX_LEN << ";" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    s << TAB
      << "KANT_TRACE(pSptd->getTraceKey(ServantProxyThreadData::TraceContext::EST_CR), TRACE_ANNOTATION_CR, \"\", "
         "ServerConfig::Application + \".\" + ServerConfig::ServerName, \""
      << pPtr->getId() << "\", 0, _trace_param_, \"\");" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << endl;
  }

  //处理线程私有数据
  s << TAB << "CallbackThreadData * pCbtd = CallbackThreadData::getData();" << endl;
  s << TAB << "assert(pCbtd != NULL);" << endl;
  s << endl;
  s << TAB << "pCbtd->setResponseContext(msg->response->context);" << endl;
  s << endl;

  //异步回调都无返回值
  s << TAB << "callback_" << pPtr->getId() << "(";
  string sParams;
  if (pPtr->getReturnPtr()->getTypePtr()) {
    sParams = pPtr->getReturnPtr()->getId() + ", ";
  }
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (vParamDecl[i]->isOut()) {
      sParams += vParamDecl[i]->getTypeIdPtr()->getId() + ", ";
    }
  }
  s << kant::KT_Common::trimright(sParams, ", ", false) << ");" << endl;

  s << endl;
  s << TAB << "pCbtd->delResponseContext();" << endl;
  s << endl;

  s << TAB << "return kant::KANTSERVERSUCCESS;" << endl;

  return s.str();
}

///////////////////////////////////////////////////////////////////////////////////////////////
string Kant2Cpp::generateDispatchCoroAsync(const OperationPtr& pPtr, const string& cn) const {
  ostringstream s;
  s << TAB << "if (msg->response->iRet != kant::KANTSERVERSUCCESS)" << endl << TAB << "{" << endl;

  INC_TAB;
  s << TAB << "callback_" << pPtr->getId() << "_exception(msg->response->iRet);" << endl;
  s << endl;

  s << TAB << "return msg->response->iRet;" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  s << TAB << _namespace + "::KantInputStream<" + _namespace + "::BufferReader> _is;" << endl;
  s << endl;
  vector<ParamDeclPtr>& vParamDecl = pPtr->getAllParamDeclPtr();

  s << TAB << "_is.setBuffer(msg->response->sBuffer);" << endl;

  if (pPtr->getReturnPtr()->getTypePtr() || vParamDecl.size() > 0) {
    s << TAB << "try" << endl;
    s << TAB << "{" << endl;

    INC_TAB;
  }

  //对输出参数编码
  if (pPtr->getReturnPtr()->getTypePtr()) {
    s << TAB << tostr(pPtr->getReturnPtr()->getTypePtr()) << " " << pPtr->getReturnPtr()->getId()
      << generateInitValue(pPtr->getReturnPtr()) << ";" << endl;
    s << readFrom(pPtr->getReturnPtr()) << endl;
  }

  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (vParamDecl[i]->isOut()) {
      s << TAB << tostr(vParamDecl[i]->getTypeIdPtr()->getTypePtr()) << " " << vParamDecl[i]->getTypeIdPtr()->getId()
        << generateInitValue(vParamDecl[i]->getTypeIdPtr()) << ";" << endl;
      s << readFrom(vParamDecl[i]->getTypeIdPtr());
    }
  }

  s << TAB << "setResponseContext(msg->response->context);" << endl;
  s << endl;

  //异步回调都无返回值
  s << TAB << "callback_" << pPtr->getId() << "(";
  string sParams;
  if (pPtr->getReturnPtr()->getTypePtr()) {
    sParams = pPtr->getReturnPtr()->getId() + ", ";
  }
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (vParamDecl[i]->isOut()) {
      sParams += vParamDecl[i]->getTypeIdPtr()->getId() + ", ";
    }
  }
  s << kant::KT_Common::trimright(sParams, ", ", false) << ");" << endl;

  s << endl;

  if (pPtr->getReturnPtr()->getTypePtr() || vParamDecl.size() > 0) {
    DEL_TAB;

    s << TAB << "}" << endl;
    s << TAB << "catch(std::exception &ex)" << endl;
    s << TAB << "{" << endl;

    INC_TAB;
    s << TAB << "callback_" << pPtr->getId() << "_exception(kant::KANTCLIENTDECODEERR);" << endl;
    s << endl;
    s << TAB << "return kant::KANTCLIENTDECODEERR;" << endl;
    DEL_TAB;

    s << TAB << "}" << endl;
    s << TAB << "catch(...)" << endl;
    s << TAB << "{" << endl;

    INC_TAB;
    s << TAB << "callback_" << pPtr->getId() << "_exception(kant::KANTCLIENTDECODEERR);" << endl;
    s << endl;
    s << TAB << "return kant::KANTCLIENTDECODEERR;" << endl;
    DEL_TAB;

    s << TAB << "}" << endl;
    s << endl;
  }

  s << TAB << "return kant::KANTSERVERSUCCESS;" << endl;

  return s.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
string Kant2Cpp::generateHAsync(const OperationPtr& pPtr) const {
  ostringstream s;
  vector<ParamDeclPtr>& vParamDecl = pPtr->getAllParamDeclPtr();
  s << TAB << "virtual void "
    << "callback_" << pPtr->getId() << "(";

  string sParams;
  if (pPtr->getReturnPtr()->getTypePtr()) {
    if (pPtr->getReturnPtr()->getTypePtr()->isSimple()) {
      sParams = tostr(pPtr->getReturnPtr()->getTypePtr()) + " ret, ";
    } else {
      //结构, map, vector, string
      sParams = "const " + tostr(pPtr->getReturnPtr()->getTypePtr()) + "& ret, ";
    }
  }
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    ParamDeclPtr& pPtr = vParamDecl[i];
    if (pPtr->isOut()) {
      //输出参数, 或简单类型
      if (pPtr->getTypeIdPtr()->getTypePtr()->isSimple()) {
        sParams += tostr(pPtr->getTypeIdPtr()->getTypePtr());
      } else {
        //结构, map, vector, string
        sParams += " const " + tostr(pPtr->getTypeIdPtr()->getTypePtr()) + "&";
      }
      sParams += " " + pPtr->getTypeIdPtr()->getId() + ", ";
    }
  }
  s << kant::KT_Common::trimright(sParams, ", ", false) << ")" << endl;

  s << TAB << "{ throw std::runtime_error(\"callback_" << pPtr->getId() << "() override incorrect.\"); }" << endl;
  s << TAB << "virtual void "
    << "callback_" << pPtr->getId() << "_exception(" + _namespace + "::Int32 ret)" << endl;
  s << TAB << "{ throw std::runtime_error(\"callback_" << pPtr->getId() << "_exception() override incorrect.\"); }";
  s << endl;

  return s.str();
}

string Kant2Cpp::generateInitValue(const TypeIdPtr& pPtr) const {
  BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getTypePtr());
  string init = "";
  if (bPtr && Builtin::KindBool == bPtr->kind()) {
    init = " = false";
  }

  //枚举强制类型转换在O2编译选项情况下会告警
  EnumPtr ePtr = std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr());
  if (ePtr) {
    if (pPtr->hasDefault()) {
      string sid = ePtr->getSid();
      init = " = " + pPtr->def() + ";";
    } else {
      vector<TypeIdPtr>& eMember = ePtr->getAllMemberPtr();
      if (eMember.size() > 0) {
        string sid = ePtr->getSid();
        init = " = " + sid.substr(0, sid.find_first_of("::")) + "::" + eMember[0]->getId() + ";";
      }
    }
  }
  return init;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
string Kant2Cpp::generateServantDispatch(const OperationPtr& pPtr, const string& cn) const {
  ostringstream s;
  s << TAB << _namespace + "::KantInputStream<" + _namespace + "::BufferReader> _is;" << endl;
  s << TAB << "_is.setBuffer(_current->getRequestBuffer());" << endl;

  vector<ParamDeclPtr>& vParamDecl = pPtr->getAllParamDeclPtr();

  //    string routekey;

  for (size_t i = 0; i < vParamDecl.size(); i++) {
    s << TAB << tostr(vParamDecl[i]->getTypeIdPtr()->getTypePtr()) << " " << vParamDecl[i]->getTypeIdPtr()->getId()
      << generateInitValue(vParamDecl[i]->getTypeIdPtr()) << ";" << endl;
    //
    //        if (routekey.empty() && vParamDecl[i]->isRouteKey())
    //        {
    //            routekey = vParamDecl[i]->getTypeIdPtr()->getId();
    //        }
  }

  s << TAB << "if (_current->getRequestVersion() == TUPVERSION)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "UniAttribute<" + _namespace + "::BufferWriterVector, " + _namespace + "::BufferReader>  kantAttr;"
    << endl;
  s << TAB << "kantAttr.setVersion(_current->getRequestVersion());" << endl;
  s << TAB << "kantAttr.decode(_current->getRequestBuffer());" << endl;
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    string sParamName = vParamDecl[i]->getTypeIdPtr()->getId();
    string sEnum2Int = (std::dynamic_pointer_cast<Enum>(vParamDecl[i]->getTypeIdPtr()->getTypePtr()))
                         ? "(" + _namespace + "::Int32)"
                         : "";
    if (!vParamDecl[i]->isOut()) {
      //枚举类型转成int
      if (std::dynamic_pointer_cast<Enum>(vParamDecl[i]->getTypeIdPtr()->getTypePtr())) {
        s << TAB << sParamName << " = (" << tostr(vParamDecl[i]->getTypeIdPtr()->getTypePtr())
          << ") kantAttr.get<" + _namespace + "::Int32>(\"" << sParamName << "\");" << endl;
      } else {
        s << TAB << "kantAttr.get(\"" << sParamName << "\", " << sParamName << ");" << endl;
      }
    } else {
      //枚举类型转成int
      if (std::dynamic_pointer_cast<Enum>(vParamDecl[i]->getTypeIdPtr()->getTypePtr())) {
        s << TAB << sParamName << " = (" << tostr(vParamDecl[i]->getTypeIdPtr()->getTypePtr())
          << ") kantAttr.getByDefault<" + _namespace + "::Int32>(\"" << sParamName << "\", " << sEnum2Int << sParamName
          << ");" << endl;
      } else {
        s << TAB << "kantAttr.getByDefault(\"" << sParamName << "\", " << sEnum2Int << sParamName << ", " << sEnum2Int
          << sParamName << ");" << endl;
      }
    }
  }
  DEL_TAB;
  s << TAB << "}" << endl;

  // 支持JSON协议分发
  //if (_bJsonSupport && kant::KT_Common::matchPeriod(pPtr->getId(), _vJsonIntf))
  if (_bJsonSupport) {
    s << TAB << "else if (_current->getRequestVersion() == JSONVERSION)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << _namespace << "::JsonValueObjPtr _jsonPtr = std::make_shared<" << _namespace << "::JsonValueObj>("
      << _namespace << "::KT_Json::getValue(_current->getRequestBuffer()));" << endl;
    for (size_t i = 0; i < vParamDecl.size(); i++) {
      string sParamName = vParamDecl[i]->getTypeIdPtr()->getId();
      string sEnum2Int = (std::dynamic_pointer_cast<Enum>(vParamDecl[i]->getTypeIdPtr()->getTypePtr()))
                           ? "(" + _namespace + "::Int32)"
                           : "";
      if (!vParamDecl[i]->isOut()) {
        // kant::JsonInput::readJson(uin, _jsonPtr->value["uin"], true); 枚举类型转成int
        s << TAB << _namespace << "::JsonInput::readJson(" << sParamName << ", _jsonPtr->value[\"" << sParamName
          << "\"], true);" << endl;
      } else {
        s << TAB << _namespace << "::JsonInput::readJson(" << sParamName << ", _jsonPtr->value[\"" << sParamName
          << "\"], false);" << endl;
      }
    }
    DEL_TAB;
    s << TAB << "}" << endl;
  }

  // 支持XML协议分发
  if (_bXmlSupport && kant::KT_Common::matchPeriod(pPtr->getId(), _vXmlIntf)) {
    s << TAB << "else if (_current->getRequestVersion() == XMLVERSION)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;

    s << TAB
      << "kant::XmlValueObjPtr _xmlPtr = "
         "std::make_shared<kant::XmlValueObj>(kant::KT_Xml::getValue(_current->getRequestBuffer()));"
      << endl;
    for (size_t i = 0; i < vParamDecl.size(); i++) {
      string sParamName = vParamDecl[i]->getTypeIdPtr()->getId();
      if (!vParamDecl[i]->isOut()) {
        //枚举类型转成int
        s << TAB << "kant::XmlInput::readXml(" << sParamName << ", _xmlPtr->value[\"" << sParamName << "\"], true);"
          << endl;
      } else {
        s << TAB << "kant::XmlInput::readXml(" << sParamName << ", _xmlPtr->value[\"" << sParamName << "\"], false);"
          << endl;
      }
    }

    DEL_TAB;
    s << TAB << "}" << endl;
  }

  s << TAB << "else" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  //普通kant请求
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    s << readFrom(vParamDecl[i]->getTypeIdPtr(), (!vParamDecl[i]->isOut()));
  }

  DEL_TAB;
  s << TAB << "}" << endl;

  // 处理调用链
  if (_bTrace) {
    s << TAB << "ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();" << endl;
    s << TAB << "if (pSptd && pSptd->_traceCall)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "string _trace_param_;" << endl;
    s << TAB
      << "int _trace_param_flag_ = pSptd->needTraceParam(ServantProxyThreadData::TraceContext::EST_SR, _is.size());"
      << endl;
    s << TAB << "if (ServantProxyThreadData::TraceContext::ENP_NORMAL == _trace_param_flag_)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << _namespace << "::JsonValueObjPtr _p_ = std::make_shared<" << _namespace << "::JsonValueObj>();" << endl;
    for (size_t i = 0; i < vParamDecl.size(); i++) {
      if (vParamDecl[i]->isOut()) continue;
      s << TAB << "_p_->value[\"" << vParamDecl[i]->getTypeIdPtr()->getId() << "\"] = " << _namespace
        << "::JsonOutput::writeJson(" << vParamDecl[i]->getTypeIdPtr()->getId() << ");" << endl;
    }
    s << TAB << "_trace_param_ = " + _namespace + "::KT_Json::writeValue(_p_);" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB << "else if(ServantProxyThreadData::TraceContext::ENP_OVERMAXLEN == _trace_param_flag_)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "_trace_param_ = " << G_TRACE_PARAM_OVER_MAX_LEN << ";" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    s << TAB
      << "KANT_TRACE(pSptd->getTraceKey(ServantProxyThreadData::TraceContext::EST_SR), TRACE_ANNOTATION_SR, \"\", "
         "ServerConfig::Application + \".\" + ServerConfig::ServerName, \""
      << pPtr->getId() << "\", 0, _trace_param_, \"\");" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << endl;
  }

  if (pPtr->getReturnPtr()->getTypePtr()) {
    s << TAB << tostr(pPtr->getReturnPtr()->getTypePtr()) << " " << pPtr->getReturnPtr()->getId() << " = "
      << pPtr->getId() << "(";
  } else {
    s << TAB << pPtr->getId() << "(";
  }

  for (size_t i = 0; i < vParamDecl.size(); i++) {
    s << vParamDecl[i]->getTypeIdPtr()->getId();
    if (i != vParamDecl.size() - 1)
      s << ",";
    else
      s << ", _current);" << endl;
  }
  if (vParamDecl.size() == 0) {
    s << "_current);" << endl;
  }
  s << TAB << "if(_current->isResponse())" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  s << TAB << "if (_current->getRequestVersion() == TUPVERSION)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  s << TAB << "UniAttribute<" + _namespace + "::BufferWriterVector, " + _namespace + "::BufferReader>  kantAttr;"
    << endl;
  s << TAB << "kantAttr.setVersion(_current->getRequestVersion());" << endl;
  if (pPtr->getReturnPtr()->getTypePtr()) {
    string sEnum2Int =
      (std::dynamic_pointer_cast<Enum>(pPtr->getReturnPtr()->getTypePtr())) ? "(" + _namespace + "::Int32)" : "";
    s << TAB << "kantAttr.put(\"\", " << sEnum2Int << "_ret);" << endl;
    s << TAB << "kantAttr.put(\"kant_ret\", " << sEnum2Int << "_ret);" << endl;
  }
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    string sParamName = vParamDecl[i]->getTypeIdPtr()->getId();
    string sEnum2Int = (std::dynamic_pointer_cast<Enum>(vParamDecl[i]->getTypeIdPtr()->getTypePtr()))
                         ? "(" + _namespace + "::Int32)"
                         : "";
    if (vParamDecl[i]->isOut()) {
      s << TAB << "kantAttr.put(\"" << sParamName << "\", " << sEnum2Int << sParamName << ");" << endl;
    }
  }
  s << TAB << "kantAttr.encode(_sResponseBuffer);" << endl;

  DEL_TAB;
  s << TAB << "}" << endl;

  // 支持JSON协议分发
  if (_bJsonSupport)
  //if (_bJsonSupport && kant::KT_Common::matchPeriod(pPtr->getId(), _vJsonIntf))
  {
    s << TAB << "else if (_current->getRequestVersion() == JSONVERSION)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << _namespace << "::JsonValueObjPtr _p = std::make_shared< " << _namespace << "::JsonValueObj>();" << endl;
    for (size_t i = 0; i < vParamDecl.size(); i++) {
      string sParamName = vParamDecl[i]->getTypeIdPtr()->getId();
      if (vParamDecl[i]->isOut()) {
        s << TAB << "_p->value[\"" << sParamName << "\"] = " << _namespace << "::JsonOutput::writeJson(" << sParamName
          << ");" << endl;
      }
    }

    if (pPtr->getReturnPtr()->getTypePtr()) {
      s << TAB << "_p->value[\"kant_ret\"] = " << _namespace << "::JsonOutput::writeJson(_ret);" << endl;
      // BuiltinPtr retPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getReturnPtr()->getTypePtr());
      // if (retPtr->kind() >= Builtin::KindBool && retPtr->kind() <= Builtin::KindLong)
      // {
      //     s << TAB << "_p->value[\"kant_ret\"] = "<< _namespace << "::JsonOutput::writeJson(" << pPtr->getReturnPtr()->getId() << ");" << endl;
      // }
    }

    s << TAB << _namespace << "::KT_Json::writeValue(_p, _sResponseBuffer);" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
  }

  // 支持XML协议分发
  if (_bXmlSupport && kant::KT_Common::matchPeriod(pPtr->getId(), _vXmlIntf)) {
    s << TAB << "else if (_current->getRequestVersion() == XMLVERSION)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;

    s << TAB << "kant::XmlValueObjPtr _p = std::make_shared<kant::XmlValueObj>();" << endl;

    for (size_t i = 0; i < vParamDecl.size(); i++) {
      string sParamName = vParamDecl[i]->getTypeIdPtr()->getId();
      if (vParamDecl[i]->isOut()) {
        s << TAB << "_p->value[\"" << sParamName << "\"] = kant::XmlOutput::writeXml(" << sParamName << ");" << endl;
      }
    }

    if (pPtr->getReturnPtr()->getTypePtr()) {
      BuiltinPtr retPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getReturnPtr()->getTypePtr());
      if (retPtr->kind() >= Builtin::KindBool && retPtr->kind() <= Builtin::KindLong) {
        s << TAB << "_p->value[\"ret\"] = kant::XmlOutput::writeXml(" << pPtr->getReturnPtr()->getId() << ");" << endl;
      }
    }

    s << TAB << "kant::KT_Xml::writeValue(_p, _sResponseBuffer);" << endl;

    DEL_TAB;
    s << TAB << "}" << endl;
  }

  s << TAB << "else" << endl;

  //普通kant调用输出参数
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << _namespace + "::KantOutputStream<" + _namespace + "::BufferWriterVector> _os;" << endl;

  if (pPtr->getReturnPtr()->getTypePtr()) {
    s << writeTo(pPtr->getReturnPtr());
  }
  //解码输出参数
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (vParamDecl[i]->isOut()) {
      s << writeTo(vParamDecl[i]->getTypeIdPtr());
    }
  }
  s << TAB << "_os.swap(_sResponseBuffer);" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  if (_bTrace) {
    s << TAB << "if (pSptd && pSptd->_traceCall)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "string _trace_param_;" << endl;
    s << TAB
      << "int _trace_param_flag_ = pSptd->needTraceParam(ServantProxyThreadData::TraceContext::EST_SS, "
         "_sResponseBuffer.size());"
      << endl;
    s << TAB << "if (ServantProxyThreadData::TraceContext::ENP_NORMAL == _trace_param_flag_)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << _namespace << "::JsonValueObjPtr _p_ = std::make_shared<" << _namespace << "::JsonValueObj>();" << endl;
    if (pPtr->getReturnPtr()->getTypePtr()) {
      s << TAB << "_p_->value[\"\"] = " << _namespace << "::JsonOutput::writeJson(_ret);" << endl;
    }
    for (size_t i = 0; i < vParamDecl.size(); i++) {
      if (vParamDecl[i]->isOut()) {
        s << TAB << "_p_->value[\"" << vParamDecl[i]->getTypeIdPtr()->getId() << "\"] = " << _namespace
          << "::JsonOutput::writeJson(" << vParamDecl[i]->getTypeIdPtr()->getId() << ");" << endl;
      }
    }
    s << TAB << "_trace_param_ = " + _namespace + "::KT_Json::writeValue(_p_);" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB << "else if(ServantProxyThreadData::TraceContext::ENP_OVERMAXLEN == _trace_param_flag_)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "_trace_param_ = " << G_TRACE_PARAM_OVER_MAX_LEN << ";" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB
      << "KANT_TRACE(pSptd->getTraceKey(ServantProxyThreadData::TraceContext::EST_SS), TRACE_ANNOTATION_SS, \"\", "
         "ServerConfig::Application + \".\" + ServerConfig::ServerName, \""
      << pPtr->getId() << "\", 0, _trace_param_, \"\");" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << endl;
  }

  DEL_TAB;
  s << TAB << "}" << endl;

  // 处理调用链
  if (_bTrace) {
    s << TAB << "else if(pSptd && pSptd->_traceCall)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB
      << "_current->setTrace(pSptd->_traceCall, pSptd->getTraceKey(ServantProxyThreadData::TraceContext::EST_SS));"
      << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << endl;
  }

  s << TAB << "return kant::KANTSERVERSUCCESS;" << endl;

  return s.str();
}

string Kant2Cpp::promiseReadFrom(const TypeIdPtr& pPtr, bool bIsRequire) const {
  ostringstream s;
  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    //枚举强制类型转换在O2编译选项情况下会告警
    string tmp = _namespace + "::Int32 eTemp" + KT_Common::tostr(pPtr->getTag()) + generateInitValue(pPtr);
    s << TAB << tmp << endl;
    s << TAB << "_is.read(eTemp" << KT_Common::tostr(pPtr->getTag());
  } else if (pPtr->getTypePtr()->isArray()) {
    s << TAB << "_is.read(ptr->" << pPtr->getId() << ", " << getSuffix(pPtr) << ", " << pPtr->getId() << "Len";
  } else if (pPtr->getTypePtr()->isPointer()) {
    s << TAB << pPtr->getId() << " = (" << tostr(pPtr->getTypePtr()) << ")_is.cur();" << endl;
    s << TAB << "_is.read(ptr->" << pPtr->getId() << ", _is.left(), " << pPtr->getId() << "Len";
  } else {
    s << TAB << "_is.read(ptr->" << pPtr->getId();
  }
  s << ", " << pPtr->getTag() << ", " << ((pPtr->isRequire() && bIsRequire) ? "true" : "false") << ");" << endl;
  if (std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr())) {
    s << TAB << "ptr->" << pPtr->getId() << " = (" << tostr(pPtr->getTypePtr()) << ")eTemp"
      << KT_Common::tostr(pPtr->getTag()) << ";" << endl;
  }
  if (pPtr->getTypePtr()->isPointer()) s << TAB << "_is.mapBufferSkip(" << pPtr->getId() << "Len);" << endl;
  return s.str();
}

bool Kant2Cpp::isPromiseDispatchInitValue(const TypeIdPtr& pPtr) const {
  BuiltinPtr bPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getTypePtr());
  string init = "";
  if (bPtr && Builtin::KindBool == bPtr->kind()) {
    return true;
  }
  //枚举强制类型转换在O2编译选项情况下会告警
  EnumPtr ePtr = std::dynamic_pointer_cast<Enum>(pPtr->getTypePtr());
  if (ePtr) {
    if (pPtr->hasDefault()) {
      return true;
    } else {
      vector<TypeIdPtr>& eMember = ePtr->getAllMemberPtr();
      if (eMember.size() > 0) {
        return true;
      }
    }
  }
  return false;
}
string Kant2Cpp::generateHAsync(const OperationPtr& pPtr, const string& cn) const {
  ostringstream s;
  //生成函数声明
  s << TAB << "void async_" << pPtr->getId() << "(";
  s << cn << "PrxCallbackPtr callback,";

  vector<ParamDeclPtr>& vParamDecl = pPtr->getAllParamDeclPtr();

  string routekey = "";

  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (!vParamDecl[i]->isOut()) {
      s << generateParamDecl(vParamDecl[i]) << ",";
    }

    if (routekey.empty() && vParamDecl[i]->isRouteKey()) {
      routekey = vParamDecl[i]->getTypeIdPtr()->getId();
    }
  }
  s << "const map<string, string>& context = KANT_CONTEXT())";
  s << endl;

  s << TAB << "{" << endl;
  INC_TAB;

  if (_kantMaster) {
    s << TAB << "this->kant_setMasterFlag(true);" << endl;
  }

  s << TAB << _namespace + "::KantOutputStream<" + _namespace + "::BufferWriterVector> _os;" << endl;

  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (vParamDecl[i]->isOut()) {
      continue;
    }
    s << writeTo(vParamDecl[i]->getTypeIdPtr());
  }

  s << TAB << "std::map<string, string> _mStatus;" << endl;

  if (!routekey.empty()) {
    ostringstream os;

    os << routekey;

    s << TAB << "_mStatus.insert(std::make_pair(ServantProxy::STATUS_GRID_KEY, " << os.str() << "));" << endl;
  }

  // 处理调用链
  if (_bTrace) {
    s << TAB << "ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();" << endl;
    s << TAB << "if (pSptd && pSptd->_traceCall)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "pSptd->newSpan();" << endl;
    s << TAB << "string _trace_param_;" << endl;
    s << TAB
      << "int _trace_param_flag_ = pSptd->needTraceParam(ServantProxyThreadData::TraceContext::EST_CS, "
         "_os.getLength());"
      << endl;
    s << TAB << "if (ServantProxyThreadData::TraceContext::ENP_NORMAL == _trace_param_flag_)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << _namespace << "::JsonValueObjPtr _p_ = std::make_shared<" << _namespace << "::JsonValueObj>();" << endl;
    for (size_t i = 0; i < vParamDecl.size(); i++) {
      if (vParamDecl[i]->isOut()) continue;
      s << TAB << "_p_->value[\"" << vParamDecl[i]->getTypeIdPtr()->getId() << "\"] = " << _namespace
        << "::JsonOutput::writeJson(" << vParamDecl[i]->getTypeIdPtr()->getId() << ");" << endl;
    }
    s << TAB << "_trace_param_ = " + _namespace + "::KT_Json::writeValue(_p_);" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB << "else if(ServantProxyThreadData::TraceContext::ENP_OVERMAXLEN == _trace_param_flag_)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "_trace_param_ = " << G_TRACE_PARAM_OVER_MAX_LEN << ";" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB
      << "KANT_TRACE(pSptd->getTraceKey(ServantProxyThreadData::TraceContext::EST_CS), TRACE_ANNOTATION_CS, "
         "ServerConfig::Application + \".\" + ServerConfig::ServerName, kant_name(), \""
      << pPtr->getId() << "\", 0, _trace_param_, \"\");" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
  }

  s << TAB << "kant_invoke_async(kant::KANTNORMAL,\"" << pPtr->getId() << "\", _os, context, _mStatus, callback);"
    << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << TAB << endl;
  //promise异步的函数声明
  string sStruct = pPtr->getId();
  s << TAB << "kant::Future< " << cn << "PrxCallbackPromise::Promise" << sStruct << "Ptr > promise_async_"
    << pPtr->getId() << "(";
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (!vParamDecl[i]->isOut()) {
      s << generateParamDecl(vParamDecl[i]) << ",";
    }
  }
  s << "const map<string, string>& context)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  if (_kantMaster) {
    s << TAB << "this->kant_setMasterFlag(true);" << endl;
  }
  s << TAB << "kant::Promise< " << cn << "PrxCallbackPromise::Promise" << sStruct << "Ptr > promise;" << endl;
  s << TAB << cn << "PrxCallbackPromisePtr callback = std::make_shared<" << cn << "PrxCallbackPromise>(promise);"
    << endl;
  s << endl;
  s << TAB << _namespace + "::KantOutputStream<" + _namespace + "::BufferWriterVector> _os;" << endl;
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (vParamDecl[i]->isOut()) {
      continue;
    }
    s << writeTo(vParamDecl[i]->getTypeIdPtr());
  }
  s << TAB << "std::map<string, string> _mStatus;" << endl;
  if (!routekey.empty()) {
    ostringstream os;
    os << routekey;
    s << TAB << "_mStatus.insert(std::make_pair(ServantProxy::STATUS_GRID_KEY, " << os.str() << "));" << endl;
  }

  s << TAB << "kant_invoke_async(kant::KANTNORMAL,\"" << pPtr->getId() << "\", _os, context, _mStatus, callback);"
    << endl;
  s << endl;
  s << TAB << "return promise.getFuture();" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << endl;

  //协程并行异步的函数声明
  s << TAB << "void coro_" << pPtr->getId() << "(";
  s << cn << "CoroPrxCallbackPtr callback,";

  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (!vParamDecl[i]->isOut()) {
      s << generateParamDecl(vParamDecl[i]) << ",";
    }
  }
  s << "const map<string, string>& context = KANT_CONTEXT())";
  s << endl;

  s << TAB << "{" << endl;
  INC_TAB;

  if (_kantMaster) {
    s << TAB << "this->kant_setMasterFlag(true);" << endl;
  }

  s << TAB << _namespace + "::KantOutputStream<" + _namespace + "::BufferWriterVector> _os;" << endl;

  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (vParamDecl[i]->isOut()) {
      continue;
    }
    s << writeTo(vParamDecl[i]->getTypeIdPtr());
  }

  s << TAB << "std::map<string, string> _mStatus;" << endl;
  if (!routekey.empty()) {
    ostringstream os;

    os << routekey;

    s << TAB << "_mStatus.insert(std::make_pair(ServantProxy::STATUS_GRID_KEY, " << os.str() << "));" << endl;
  }

  s << TAB << "kant_invoke_async(kant::KANTNORMAL,\"" << pPtr->getId() << "\", _os, context, _mStatus, callback, true);"
    << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  return s.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
string Kant2Cpp::generateH(const OperationPtr& pPtr, bool bVirtual, const string& interfaceId) const {
  ostringstream s;
  vector<ParamDeclPtr>& vParamDecl = pPtr->getAllParamDeclPtr();

  s << TAB;

  if (bVirtual) s << "virtual ";

  s << tostr(pPtr->getReturnPtr()->getTypePtr()) << " " << pPtr->getId() << "(";

  string routekey = "";
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    s << generateH(vParamDecl[i]) << ",";

    if (routekey.empty() && vParamDecl[i]->isRouteKey()) {
      routekey = vParamDecl[i]->getTypeIdPtr()->getId();
    }
  }

  if (bVirtual) {
    s << "kant::KantCurrentPtr current) = 0;";
  } else {
    s << "const map<string, string> &context = KANT_CONTEXT(),map<string, string> * pResponseContext = NULL)";

    s << endl;

    s << TAB << "{" << endl;

    INC_TAB;

    if (_kantMaster) {
      s << TAB << "this->kant_setMasterFlag(true);" << endl;
    }

    s << TAB << _namespace + "::KantOutputStream<" + _namespace + "::BufferWriterVector> _os;" << endl;

    for (size_t i = 0; i < vParamDecl.size(); i++) {
      //if(vParamDecl[i]->isOut()) continue;
      s << writeTo(vParamDecl[i]->getTypeIdPtr());
    }

    // s << TAB << "" + _namespace + "::ResponsePacket rep;" << endl;

    // 处理调用链
    if (_bTrace) {
      s << TAB << "ServantProxyThreadData *pSptd = ServantProxyThreadData::getData();" << endl;
      s << TAB << "if (pSptd && pSptd->_traceCall)" << endl;
      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB << "pSptd->newSpan();" << endl;
      s << TAB << "string _trace_param_;" << endl;
      s << TAB
        << "int _trace_param_flag_ = pSptd->needTraceParam(ServantProxyThreadData::TraceContext::EST_CS, "
           "_os.getLength());"
        << endl;
      s << TAB << "if (ServantProxyThreadData::TraceContext::ENP_NORMAL == _trace_param_flag_)" << endl;
      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB << _namespace << "::JsonValueObjPtr _p_ = std::make_shared<" << _namespace << "::JsonValueObj>();"
        << endl;
      for (size_t i = 0; i < vParamDecl.size(); i++) {
        if (vParamDecl[i]->isOut()) continue;
        s << TAB << "_p_->value[\"" << vParamDecl[i]->getTypeIdPtr()->getId() << "\"] = " << _namespace
          << "::JsonOutput::writeJson(" << vParamDecl[i]->getTypeIdPtr()->getId() << ");" << endl;
      }
      s << TAB << "_trace_param_ = " + _namespace + "::KT_Json::writeValue(_p_);" << endl;
      DEL_TAB;
      s << TAB << "}" << endl;
      s << TAB << "else if(ServantProxyThreadData::TraceContext::ENP_OVERMAXLEN == _trace_param_flag_)" << endl;
      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB << "_trace_param_ = " << G_TRACE_PARAM_OVER_MAX_LEN << ";" << endl;
      DEL_TAB;
      s << TAB << "}" << endl;
      s << TAB
        << "KANT_TRACE(pSptd->getTraceKey(ServantProxyThreadData::TraceContext::EST_CS), TRACE_ANNOTATION_CS, "
           "ServerConfig::Application + \".\" + ServerConfig::ServerName, kant_name(), \""
        << pPtr->getId() << "\", 0, _trace_param_, \"\");" << endl;
      DEL_TAB;
      s << TAB << "}" << endl;
      s << endl;
    }

    s << TAB << "std::map<string, string> _mStatus;" << endl;

    if (!routekey.empty()) {
      ostringstream os;

      os << routekey;

      s << TAB << "_mStatus.insert(std::make_pair(ServantProxy::STATUS_GRID_KEY, " << os.str() << "));" << endl;
    }

    // s << TAB << "kant_invoke(kant::KANTNORMAL,\"" << pPtr->getId() << "\", _os.getByteBuffer(), context, _mStatus, rep);" << endl;
    s << TAB << "shared_ptr<" + _namespace + "::ResponsePacket> rep = kant_invoke(kant::KANTNORMAL,\"" << pPtr->getId()
      << "\", _os, context, _mStatus);" << endl;
    s << TAB << "if(pResponseContext)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "pResponseContext->swap(rep->context);" << endl;
    // s << TAB << "*pResponseContext = rep.context;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;

    s << endl;

    if (vParamDecl.size() > 0 || pPtr->getReturnPtr()->getTypePtr()) {
      s << TAB << _namespace + "::KantInputStream<" + _namespace + "::BufferReader> _is;" << endl;
      s << TAB << "_is.setBuffer(rep->sBuffer);" << endl;
      if (pPtr->getReturnPtr()->getTypePtr()) {
        s << TAB << tostr(pPtr->getReturnPtr()->getTypePtr()) << " " << pPtr->getReturnPtr()->getId()
          << generateInitValue(pPtr->getReturnPtr()) << ";" << endl;
        s << readFrom(pPtr->getReturnPtr());
      }
      for (size_t i = 0; i < vParamDecl.size(); i++) {
        if (vParamDecl[i]->isOut()) {
          s << readFrom(vParamDecl[i]->getTypeIdPtr());
        }
      }

      // 处理调用链
      if (_bTrace) {
        s << TAB << "if (pSptd && pSptd->_traceCall)" << endl;
        s << TAB << "{" << endl;
        INC_TAB;
        s << TAB << "string _trace_param_;" << endl;
        s << TAB
          << "int _trace_param_flag_ = pSptd->needTraceParam(ServantProxyThreadData::TraceContext::EST_CR, _is.size());"
          << endl;
        s << TAB << "if (ServantProxyThreadData::TraceContext::ENP_NORMAL == _trace_param_flag_)" << endl;
        s << TAB << "{" << endl;
        INC_TAB;
        s << TAB << _namespace << "::JsonValueObjPtr _p_ = std::make_shared<" << _namespace << "::JsonValueObj>();"
          << endl;
        if (pPtr->getReturnPtr()->getTypePtr()) {
          s << TAB << "_p_->value[\"\"] = " << _namespace << "::JsonOutput::writeJson(_ret);" << endl;
        }
        for (size_t i = 0; i < vParamDecl.size(); i++) {
          if (vParamDecl[i]->isOut()) {
            s << TAB << "_p_->value[\"" << vParamDecl[i]->getTypeIdPtr()->getId() << "\"] = " << _namespace
              << "::JsonOutput::writeJson(" << vParamDecl[i]->getTypeIdPtr()->getId() << ");" << endl;
          }
        }
        s << TAB << "_trace_param_ = " + _namespace + "::KT_Json::writeValue(_p_);" << endl;
        DEL_TAB;
        s << TAB << "}" << endl;
        s << TAB << "else if(ServantProxyThreadData::TraceContext::ENP_OVERMAXLEN == _trace_param_flag_)" << endl;
        s << TAB << "{" << endl;
        INC_TAB;
        s << TAB << "_trace_param_ = " << G_TRACE_PARAM_OVER_MAX_LEN << ";" << endl;
        DEL_TAB;
        s << TAB << "}" << endl;
        s << TAB
          << "KANT_TRACE(pSptd->getTraceKey(ServantProxyThreadData::TraceContext::EST_CR), TRACE_ANNOTATION_CR, "
             "ServerConfig::Application + \".\" + ServerConfig::ServerName, kant_name(), \""
          << pPtr->getId() << "\", 0, _trace_param_, \"\");" << endl;
        DEL_TAB;
        s << TAB << "}" << endl;
        s << endl;
      }

      if (pPtr->getReturnPtr()->getTypePtr()) {
        s << TAB << "return " << pPtr->getReturnPtr()->getId() << ";" << endl;
      }
    } else if (_bTrace) {
      // 处理调用链
      s << TAB << "if (pSptd && pSptd->_traceCall)" << endl;
      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB
        << "KANT_TRACE(pSptd->getTraceKey(ServantProxyThreadData::TraceContext::EST_CR), TRACE_ANNOTATION_CR, \"\", "
           "\"\", \""
        << pPtr->getId() << "\", 0, \"\", \"\");" << endl;
      DEL_TAB;
      s << "}" << endl;
    }
    DEL_TAB;
    s << TAB << "}" << endl;
  }

  s << endl;

  if (bVirtual) {
    //异步回调
    s << TAB << "static void async_response_" << pPtr->getId() << "(kant::KantCurrentPtr current";
    if (pPtr->getReturnPtr()->getTypePtr()) {
      s << ", ";
      if (pPtr->getReturnPtr()->getTypePtr()->isSimple()) {
        s << tostr(pPtr->getReturnPtr()->getTypePtr()) << " ";
      } else {
        //结构, map, vector, string
        s << "const " << tostr(pPtr->getReturnPtr()->getTypePtr()) << " &";
      }
      s << pPtr->getReturnPtr()->getId();
    }
    for (size_t i = 0; i < vParamDecl.size(); i++) {
      if (!vParamDecl[i]->isOut()) continue;

      s << ", ";
      s << generateOutH(vParamDecl[i]);
    }
    s << ")" << endl;

    s << TAB << "{" << endl;
    INC_TAB;

    if (_bTrace) {
      s << TAB << "size_t _rsp_len_ = 0;" << endl;
    }

    s << TAB << "if (current->getRequestVersion() == TUPVERSION )" << endl;
    s << TAB << "{" << endl;
    INC_TAB;

    s << TAB << "UniAttribute<" + _namespace + "::BufferWriterVector, " + _namespace + "::BufferReader>  kantAttr;"
      << endl;
    s << TAB << "kantAttr.setVersion(current->getRequestVersion());" << endl;
    if (pPtr->getReturnPtr()->getTypePtr()) {
      string sEnum2Int =
        (std::dynamic_pointer_cast<Enum>(pPtr->getReturnPtr()->getTypePtr())) ? "(" + _namespace + "::Int32)" : "";
      s << TAB << "kantAttr.put(\"\", " << sEnum2Int << "_ret);" << endl;
      s << TAB << "kantAttr.put(\"kant_ret\", " << sEnum2Int << "_ret);" << endl;
    }
    for (size_t i = 0; i < vParamDecl.size(); i++) {
      string sParamName = vParamDecl[i]->getTypeIdPtr()->getId();
      string sEnum2Int = (std::dynamic_pointer_cast<Enum>(vParamDecl[i]->getTypeIdPtr()->getTypePtr()))
                           ? "(" + _namespace + "::Int32)"
                           : "";
      if (vParamDecl[i]->isOut()) {
        s << TAB << "kantAttr.put(\"" << sParamName << "\", " << sEnum2Int << sParamName << ");" << endl;
      }
    }
    s << endl;
    s << TAB << "vector<char> sTupResponseBuffer;" << endl;
    s << TAB << "kantAttr.encode(sTupResponseBuffer);" << endl;
    s << TAB << "current->sendResponse(kant::KANTSERVERSUCCESS, sTupResponseBuffer);" << endl;
    if (_bTrace) {
      s << TAB << "_rsp_len_ = sTupResponseBuffer.size();" << endl;
    }

    DEL_TAB;
    s << TAB << "}" << endl;

    if (_bJsonSupport) {
      s << TAB << "else if (current->getRequestVersion() == JSONVERSION)" << endl;
      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB << _namespace << "::JsonValueObjPtr _p = std::make_shared< " << _namespace << "::JsonValueObj>();"
        << endl;
      for (size_t i = 0; i < vParamDecl.size(); i++) {
        string sParamName = vParamDecl[i]->getTypeIdPtr()->getId();
        if (vParamDecl[i]->isOut()) {
          s << TAB << "_p->value[\"" << sParamName << "\"] = " << _namespace << "::JsonOutput::writeJson(" << sParamName
            << ");" << endl;
        }
      }

      if (pPtr->getReturnPtr()->getTypePtr()) {
        s << TAB << "_p->value[\"kant_ret\"] = " << _namespace << "::JsonOutput::writeJson(_ret);" << endl;
        // BuiltinPtr retPtr = std::dynamic_pointer_cast<Builtin>(pPtr->getReturnPtr()->getTypePtr());
        // if (retPtr && retPtr->kind() >= Builtin::KindBool && retPtr->kind() <= Builtin::KindLong)
        // {
        //     s << TAB << "_p->value[\"kant_ret\"] = " << _namespace << "::JsonOutput::writeJson(" << pPtr->getReturnPtr()->getId() << ");" << endl;
        //     //s << TAB << "_p->value[\"\"] = " << _namespace << "::JsonOutput::writeJson(" << pPtr->getReturnPtr()->getId() << ");" << endl;
        // }
      }

      s << TAB << "vector<char> sJsonResponseBuffer;" << endl;

      s << TAB << _namespace << "::KT_Json::writeValue(_p, sJsonResponseBuffer);" << endl;
      s << TAB << "current->sendResponse(kant::KANTSERVERSUCCESS, sJsonResponseBuffer);" << endl;
      if (_bTrace) {
        s << TAB << "_rsp_len_ = sJsonResponseBuffer.size();" << endl;
      }
      DEL_TAB;
      s << TAB << "}" << endl;
    }

    s << TAB << "else" << endl;
    s << TAB << "{" << endl;

    INC_TAB;

    s << TAB << _namespace + "::KantOutputStream<" + _namespace + "::BufferWriterVector> _os;" << endl;
    if (pPtr->getReturnPtr()->getTypePtr()) {
      s << writeTo(pPtr->getReturnPtr()) << endl;
    }
    for (size_t i = 0; i < vParamDecl.size(); i++) {
      if (!vParamDecl[i]->isOut()) continue;

      s << writeTo(vParamDecl[i]->getTypeIdPtr()) << endl;
    }

    //s << TAB << "current->sendResponse(kant::KANTSERVERSUCCESS, string(_os.getBuffer(), _os.getLength()));" << endl;
    s << TAB << "current->sendResponse(kant::KANTSERVERSUCCESS, _os.getByteBuffer());" << endl;
    if (_bTrace) {
      s << TAB << "_rsp_len_ = _os.getLength();" << endl;
    }

    DEL_TAB;
    s << TAB << "}" << endl;

    if (_bTrace) {
      s << TAB << "if (current->isTraced())" << endl;
      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB << "string _trace_param_;" << endl;
      s << TAB
        << "int _trace_param_flag_ = "
           "ServantProxyThreadData::needTraceParam(ServantProxyThreadData::TraceContext::EST_SS, "
           "current->getTraceKey(), _rsp_len_);"
        << endl;
      s << TAB << "if (ServantProxyThreadData::TraceContext::ENP_NORMAL == _trace_param_flag_)" << endl;
      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB << _namespace << "::JsonValueObjPtr _p_ = std::make_shared<" << _namespace << "::JsonValueObj>();"
        << endl;
      if (pPtr->getReturnPtr()->getTypePtr()) {
        s << TAB << "_p_->value[\"\"] = " << _namespace << "::JsonOutput::writeJson(_ret);" << endl;
      }
      for (size_t i = 0; i < vParamDecl.size(); i++) {
        if (vParamDecl[i]->isOut()) {
          s << TAB << "_p_->value[\"" << vParamDecl[i]->getTypeIdPtr()->getId() << "\"] = " << _namespace
            << "::JsonOutput::writeJson(" << vParamDecl[i]->getTypeIdPtr()->getId() << ");" << endl;
        }
      }
      s << TAB << "_trace_param_ = " + _namespace + "::KT_Json::writeValue(_p_);" << endl;
      DEL_TAB;
      s << TAB << "}" << endl;
      s << TAB << "else if(ServantProxyThreadData::TraceContext::ENP_OVERMAXLEN == _trace_param_flag_)" << endl;
      s << TAB << "{" << endl;
      INC_TAB;
      s << TAB << "_trace_param_ = " << G_TRACE_PARAM_OVER_MAX_LEN << ";" << endl;
      DEL_TAB;
      s << TAB << "}" << endl;
      s << TAB
        << "KANT_TRACE(current->getTraceKey(), TRACE_ANNOTATION_SS, \"\", ServerConfig::Application + \".\" + "
           "ServerConfig::ServerName, \""
        << pPtr->getId() << "\", 0, _trace_param_, \"\");" << endl;
      DEL_TAB;
      s << TAB << "}" << endl;
      s << endl;
    }

    DEL_TAB;
    s << TAB << "}" << endl;
  }
  return s.str();
}

/**
 * 对接口名排序
 */
struct SortOperation {
  bool operator()(const OperationPtr& o1, const OperationPtr& o2) { return o1->getId() < o2->getId(); }
};

string Kant2Cpp::generateHPromiseAsync(const InterfacePtr& pInter, const OperationPtr& pPtr) const {
  ostringstream s;
  string sStruct = pPtr->getId();
  vector<ParamDeclPtr>& vParamDecl = pPtr->getAllParamDeclPtr();
  ////////
  DEL_TAB;
  s << TAB << "public:" << endl;
  INC_TAB;
  s << TAB << "struct Promise" << sStruct << endl;
  s << TAB << "{" << endl;
  s << TAB << "public:" << endl;
  INC_TAB;
  if (pPtr->getReturnPtr()->getTypePtr()) {
    s << TAB << tostr(pPtr->getReturnPtr()->getTypePtr()) << " _ret;" << endl;
  }
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    ParamDeclPtr& pPtr = vParamDecl[i];
    if (pPtr->isOut()) {
      s << TAB << tostr(pPtr->getTypeIdPtr()->getTypePtr()) << " " << pPtr->getTypeIdPtr()->getId() << ";" << endl;
    }
  }
  s << TAB << "map<std::string, std::string> _mRspContext;" << endl;
  DEL_TAB;
  s << TAB << "};" << endl;
  s << TAB << endl;
  s << TAB << "typedef std::shared_ptr< " << pInter->getId() << "PrxCallbackPromise::Promise" << sStruct << " > Promise"
    << sStruct << "Ptr;" << endl;
  s << endl;
  s << TAB << pInter->getId() << "PrxCallbackPromise(const kant::Promise< " << pInter->getId()
    << "PrxCallbackPromise::Promise" << sStruct << "Ptr > &promise)" << endl;
  s << TAB << ": _promise_" << sStruct << "(promise)" << endl;
  s << TAB << "{}" << endl;
  s << TAB << endl;
  s << TAB << "virtual void "
    << "callback_" << pPtr->getId() << "(const " << pInter->getId() << "PrxCallbackPromise::Promise" << sStruct
    << "Ptr &ptr)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "_promise_" << sStruct << ".setValue(ptr);" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << TAB << "virtual void "
    << "callback_" << pPtr->getId() << "_exception(" + _namespace + "::Int32 ret)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "std::string str(\"\");" << endl;
  s << TAB << "str += \"Function:" << pPtr->getId() << "_exception|Ret:\";" << endl;
  s << TAB << "str += KT_Common::tostr(ret);" << endl;
  s << TAB << "_promise_" << sStruct << ".setException(kant::copyException(str, ret));" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << endl;
  DEL_TAB;
  s << TAB << "protected:" << endl;
  INC_TAB;
  s << TAB << "kant::Promise< " << pInter->getId() << "PrxCallbackPromise::Promise" << sStruct << "Ptr > _promise_"
    << sStruct << ";" << endl;
  return s.str();
}
string Kant2Cpp::generateDispatchPromiseAsync(const OperationPtr& pPtr, const string& cn) const {
  ostringstream s;
  s << TAB << "if (msg->response->iRet != kant::KANTSERVERSUCCESS)" << endl << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "callback_" << pPtr->getId() << "_exception(msg->response->iRet);" << endl;
  s << endl;
  s << TAB << "return msg->response->iRet;" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << TAB << _namespace + "::KantInputStream<" + _namespace + "::BufferReader> _is;" << endl;
  s << endl;
  vector<ParamDeclPtr>& vParamDecl = pPtr->getAllParamDeclPtr();
  s << TAB << "_is.setBuffer(msg->response->sBuffer);" << endl;
  s << endl;
  string sStruct = pPtr->getId();
  s << TAB << cn << "PrxCallbackPromise::Promise" << sStruct << "Ptr ptr = std::make_shared<" << cn
    << "PrxCallbackPromise::Promise" << sStruct << ">();" << endl;
  s << endl;
  if (pPtr->getReturnPtr()->getTypePtr() || vParamDecl.size() > 0) {
    s << TAB << "try" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
  }
  //对输出参数编码
  if (pPtr->getReturnPtr()->getTypePtr()) {
    if (isPromiseDispatchInitValue(pPtr->getReturnPtr()))
      s << TAB << "ptr->_ret " << generateInitValue(pPtr->getReturnPtr()) << ";" << endl;
    s << promiseReadFrom(pPtr->getReturnPtr()) << endl;
  }
  for (size_t i = 0; i < vParamDecl.size(); i++) {
    if (vParamDecl[i]->isOut()) {
      if (isPromiseDispatchInitValue(vParamDecl[i]->getTypeIdPtr()))
        s << TAB << "ptr->" << vParamDecl[i]->getTypeIdPtr()->getId()
          << generateInitValue(vParamDecl[i]->getTypeIdPtr()) << ";" << endl;
      s << promiseReadFrom(vParamDecl[i]->getTypeIdPtr());
    }
  }
  if (pPtr->getReturnPtr()->getTypePtr() || vParamDecl.size() > 0) {
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB << "catch(std::exception &ex)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "callback_" << pPtr->getId() << "_exception(kant::KANTCLIENTDECODEERR);" << endl;
    s << endl;
    s << TAB << "return kant::KANTCLIENTDECODEERR;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << TAB << "catch(...)" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << TAB << "callback_" << pPtr->getId() << "_exception(kant::KANTCLIENTDECODEERR);" << endl;
    s << endl;
    s << TAB << "return kant::KANTCLIENTDECODEERR;" << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
    s << endl;
  }
  s << TAB << "ptr->_mRspContext = msg->response->context;" << endl;
  s << endl;
  s << TAB << "callback_" << pPtr->getId() << "(ptr);" << endl;
  s << endl;
  s << TAB << "return kant::KANTSERVERSUCCESS;" << endl;
  return s.str();
}
/******************************InterfacePtr***************************************/
string Kant2Cpp::generateH(const InterfacePtr& pPtr, const NamespacePtr& nPtr) const {
  ostringstream s;
  vector<OperationPtr>& vOperation = pPtr->getAllOperationPtr();

  std::sort(vOperation.begin(), vOperation.end(), SortOperation());

  //生成异步回调Proxy
  s << TAB << "/* callback of async proxy for client */" << endl;
  s << TAB << "class " << pPtr->getId() << "PrxCallback: public kant::ServantProxyCallback" << endl;
  s << TAB << "{" << endl;
  s << TAB << "public:" << endl;
  INC_TAB;
  s << TAB << "virtual ~" << pPtr->getId() << "PrxCallback(){}" << endl;

  for (size_t i = 0; i < vOperation.size(); i++) {
    s << generateHAsync(vOperation[i]) << endl;
  }

  DEL_TAB;
  s << TAB << "public:" << endl;
  INC_TAB;
  s << TAB << "virtual const map<std::string, std::string> & getResponseContext() const" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  s << TAB << "CallbackThreadData * pCbtd = CallbackThreadData::getData();" << endl;
  s << TAB << "assert(pCbtd != NULL);" << endl;
  s << endl;
  s << TAB << "if(!pCbtd->getContextValid())" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "throw KT_Exception(\"cann't get response context\");" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << TAB << "return pCbtd->getResponseContext();" << endl;

  DEL_TAB;
  s << TAB << "}" << endl;
  s << endl;

  DEL_TAB;
  s << TAB << "public:" << endl;
  INC_TAB;

  s << TAB << "virtual int onDispatch(kant::ReqMessagePtr msg)" << endl;

  //生成异步回调接口
  s << TAB << "{" << endl;
  INC_TAB;
  string dname = "__" + pPtr->getId() + "_all";
  string dispatch = "static ::std::string " + dname;
  s << TAB << dispatch << "[]=" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  for (size_t i = 0; i < vOperation.size(); i++) {
    s << TAB << "\"" << vOperation[i]->getId() << "\"";
    if (i != vOperation.size() - 1) {
      s << ",";
    }
    s << endl;
  }
  DEL_TAB;
  s << TAB << "};" << endl;

  s << TAB << "pair<string*, string*> r = equal_range(" << dname << ", " << dname << "+" << vOperation.size()
    << ", string(msg->request.sFuncName));" << endl;

  s << TAB << "if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;" << endl;

  s << TAB << "switch(r.first - " << dname << ")" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  for (size_t i = 0; i < vOperation.size(); i++) {
    s << TAB << "case " << i << ":" << endl;
    s << TAB << "{" << endl;
    INC_TAB;

    s << generateDispatchAsync(vOperation[i], pPtr->getId()) << endl;

    DEL_TAB;
    s << TAB << "}" << endl;
  }

  DEL_TAB;
  s << TAB << "}" << endl;

  s << TAB << "return kant::KANTSERVERNOFUNCERR;" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  s << endl;

  DEL_TAB;
  s << TAB << "};" << endl;

  s << TAB << "typedef std::shared_ptr<" << pPtr->getId() << "PrxCallback> " << pPtr->getId() << "PrxCallbackPtr;"
    << endl;
  s << endl;
  //生成promise异步回调Proxy
  s << TAB << "//callback of promise async proxy for client" << endl;
  s << TAB << "class " << pPtr->getId() << "PrxCallbackPromise: public kant::ServantProxyCallback" << endl;
  s << TAB << "{" << endl;
  s << TAB << "public:" << endl;
  INC_TAB;
  s << TAB << "virtual ~" << pPtr->getId() << "PrxCallbackPromise(){}" << endl;
  for (size_t i = 0; i < vOperation.size(); i++) {
    s << generateHPromiseAsync(pPtr, vOperation[i]) << endl;
  }
  DEL_TAB;
  s << TAB << "public:" << endl;
  INC_TAB;
  s << TAB << "virtual int onDispatch(kant::ReqMessagePtr msg)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  dname = "__" + pPtr->getId() + "_all";
  dispatch = "static ::std::string " + dname;
  s << TAB << dispatch << "[]=" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  for (size_t i = 0; i < vOperation.size(); i++) {
    s << TAB << "\"" << vOperation[i]->getId() << "\"";
    if (i != vOperation.size() - 1) {
      s << ",";
    }
    s << endl;
  }
  DEL_TAB;
  s << TAB << "};" << endl;
  s << endl;
  s << TAB << "pair<string*, string*> r = equal_range(" << dname << ", " << dname << "+" << vOperation.size()
    << ", string(msg->request.sFuncName));" << endl;
  s << TAB << "if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;" << endl;
  s << TAB << "switch(r.first - " << dname << ")" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  for (size_t i = 0; i < vOperation.size(); i++) {
    s << TAB << "case " << i << ":" << endl;
    s << TAB << "{" << endl;
    INC_TAB;
    s << generateDispatchPromiseAsync(vOperation[i], pPtr->getId()) << endl;
    DEL_TAB;
    s << TAB << "}" << endl;
  }
  DEL_TAB;
  s << TAB << "}" << endl;
  s << TAB << "return kant::KANTSERVERNOFUNCERR;" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << endl;
  DEL_TAB;
  s << TAB << "};" << endl;
  s << TAB << "typedef std::shared_ptr<" << pPtr->getId() << "PrxCallbackPromise> " << pPtr->getId()
    << "PrxCallbackPromisePtr;" << endl;
  s << endl;
  //生成协程异步回调类，用于并发请求
  s << TAB << "/* callback of coroutine async proxy for client */" << endl;
  s << TAB << "class " << pPtr->getId() << "CoroPrxCallback: public " << pPtr->getId() << "PrxCallback" << endl;
  s << TAB << "{" << endl;
  s << TAB << "public:" << endl;
  INC_TAB;
  s << TAB << "virtual ~" << pPtr->getId() << "CoroPrxCallback(){}" << endl;
  DEL_TAB;

  s << TAB << "public:" << endl;

  INC_TAB;
  s << TAB << "virtual const map<std::string, std::string> & getResponseContext() const { return _mRspContext; }"
    << endl;
  s << endl;
  s << TAB
    << "virtual void setResponseContext(const map<std::string, std::string> &mContext) { _mRspContext = mContext; }"
    << endl;
  s << endl;
  DEL_TAB;

  s << TAB << "public:" << endl;
  INC_TAB;
  //生成协程异步回调接口
  s << TAB << "int onDispatch(kant::ReqMessagePtr msg)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  dname = "__" + pPtr->getId() + "_all";
  dispatch = "static ::std::string " + dname;
  s << TAB << dispatch << "[]=" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  for (size_t i = 0; i < vOperation.size(); i++) {
    s << TAB << "\"" << vOperation[i]->getId() << "\"";
    if (i != vOperation.size() - 1) {
      s << ",";
    }
    s << endl;
  }

  DEL_TAB;
  s << TAB << "};" << endl;

  s << endl;

  s << TAB << "pair<string*, string*> r = equal_range(" << dname << ", " << dname << "+" << vOperation.size()
    << ", string(msg->request.sFuncName));" << endl;

  s << TAB << "if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;" << endl;

  s << TAB << "switch(r.first - " << dname << ")" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  for (size_t i = 0; i < vOperation.size(); i++) {
    s << TAB << "case " << i << ":" << endl;
    s << TAB << "{" << endl;
    INC_TAB;

    s << generateDispatchCoroAsync(vOperation[i], pPtr->getId()) << endl;

    DEL_TAB;
    s << TAB << "}" << endl;
  }

  DEL_TAB;
  s << TAB << "}" << endl;

  s << TAB << "return kant::KANTSERVERNOFUNCERR;" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  s << endl;

  ////////////////////////////////////////////////////////////////////
  DEL_TAB;
  s << TAB << "protected:" << endl;

  INC_TAB;
  s << TAB << "map<std::string, std::string> _mRspContext;" << endl;
  //s << TAB << "kant::ParallelSharedBasePtr _pPtr;" << endl;
  DEL_TAB;

  s << TAB << "};" << endl;

  s << TAB << "typedef std::shared_ptr<" << pPtr->getId() << "CoroPrxCallback> " << pPtr->getId()
    << "CoroPrxCallbackPtr;" << endl;
  s << endl;

  //生成客户端代理
  s << TAB << "/* proxy for client */" << endl;
  s << TAB << "class " << pPtr->getId() << "Proxy : public kant::ServantProxy" << endl;
  s << TAB << "{" << endl;
  s << TAB << "public:" << endl;
  INC_TAB;
  s << TAB << "typedef map<string, string> KANT_CONTEXT;" << endl;

  for (size_t i = 0; i < vOperation.size(); i++) {
    s << generateH(vOperation[i], false, pPtr->getId());  // << endl;
    s << generateHAsync(vOperation[i], pPtr->getId()) << endl;
  }

  s << TAB << pPtr->getId() << "Proxy* kant_hash(int64_t key)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "return (" << pPtr->getId() + "Proxy*)ServantProxy::kant_hash(key);" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << endl;

  s << TAB << pPtr->getId() << "Proxy* kant_consistent_hash(int64_t key)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "return (" << pPtr->getId() + "Proxy*)ServantProxy::kant_consistent_hash(key);" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << endl;

  s << TAB << pPtr->getId() << "Proxy* kant_open_trace(bool traceParam = false)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "return (" << pPtr->getId() + "Proxy*)ServantProxy::kant_open_trace(traceParam);" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << endl;

  s << TAB << pPtr->getId() << "Proxy* kant_set_timeout(int msecond)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  s << TAB << "return (" << pPtr->getId() + "Proxy*)ServantProxy::kant_set_timeout(msecond);" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;
  s << endl;

  s << TAB << "static const char* kant_prxname() { return \"" << pPtr->getId() << "Proxy\"; }" << endl;

  DEL_TAB;
  s << TAB << "};" << endl;

  s << TAB << "typedef std::shared_ptr<" << pPtr->getId() << "Proxy> " << pPtr->getId() << "Prx;" << endl;
  s << endl;

  //生成服务端Servant
  s << TAB << "/* servant for server */" << endl;
  s << TAB << "class " << pPtr->getId() << " : public kant::Servant" << endl;
  s << TAB << "{" << endl;
  s << TAB << "public:" << endl;
  INC_TAB;
  s << TAB << "virtual ~" << pPtr->getId() << "(){}" << endl;

  for (size_t i = 0; i < vOperation.size(); i++) {
    s << generateH(vOperation[i], true, pPtr->getId()) << endl;
  }

  DEL_TAB;
  s << TAB << "public:" << endl;
  INC_TAB;

  s << TAB << "int onDispatch(kant::KantCurrentPtr _current, vector<char> &_sResponseBuffer)" << endl;

  s << TAB << "{" << endl;
  INC_TAB;
  dname = "__" + nPtr->getId() + "__" + pPtr->getId() + "_all";
  dispatch = "static ::std::string " + dname;
  s << TAB << dispatch << "[]=" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  for (size_t i = 0; i < vOperation.size(); i++) {
    s << TAB << "\"" << vOperation[i]->getId() << "\"";
    if (i != vOperation.size() - 1) {
      s << ",";
    }
    s << endl;
  }

  DEL_TAB;
  s << TAB << "};" << endl;

  s << endl;

  s << TAB << "pair<string*, string*> r = equal_range(" << dname << ", " << dname << "+" << vOperation.size()
    << ", _current->getFuncName());" << endl;

  s << TAB << "if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;" << endl;

  s << TAB << "switch(r.first - " << dname << ")" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  for (size_t i = 0; i < vOperation.size(); i++) {
    s << TAB << "case " << i << ":" << endl;
    s << TAB << "{" << endl;
    INC_TAB;

    s << generateServantDispatch(vOperation[i], pPtr->getId()) << endl;

    DEL_TAB;
    s << TAB << "}" << endl;
  }

  DEL_TAB;
  s << TAB << "}" << endl;

  s << TAB << "return kant::KANTSERVERNOFUNCERR;" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  DEL_TAB;
  s << TAB << "};" << endl;

  return s.str();
}

//struct SortOperation
//{
//    bool operator()(const OperationPtr &o1, const OperationPtr &o2)
//    {
//        return o1->getId() < o2->getId();
//    }
//};

/******************************EnumPtr***************************************/

string Kant2Cpp::generateH(const EnumPtr& pPtr) const {
  ostringstream s;
  s << TAB << "enum " << pPtr->getId() << endl;
  s << TAB << "{" << endl;
  INC_TAB;
  vector<TypeIdPtr>& member = pPtr->getAllMemberPtr();
  for (size_t i = 0; i < member.size(); i++) {
    s << TAB << member[i]->getId();
    if (member[i]->hasDefault()) {
      s << " = " << member[i]->def();
    }
    s << "," << endl;
  }
  DEL_TAB;
  s << TAB << "};" << endl;

  //生成枚举转字符串函数
  s << TAB << "inline string etos"
    << "(const " << pPtr->getId() << " & e)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  s << TAB << "switch(e)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  for (size_t i = 0; i < member.size(); i++) {
    s << TAB << "case " << member[i]->getId() << ": return "
      << "\"" << member[i]->getId() << "\";" << endl;
  }
  s << TAB << "default: return \"\";" << endl;

  DEL_TAB;
  s << TAB << "}" << endl;
  //s << TAB << "return \"\";" << endl;
  DEL_TAB;
  s << TAB << "}" << endl;

  //生成字符串转枚举函数
  s << TAB << "inline int stoe"
    << "(const string & s, " << pPtr->getId() << " & e)" << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  for (size_t i = 0; i < member.size(); i++) {
    s << TAB << "if(s == \"" << member[i]->getId() << "\")  { e=" << member[i]->getId() << "; return 0;}" << endl;
  }
  s << endl;
  s << TAB << "return -1;" << endl;

  DEL_TAB;
  s << TAB << "}" << endl;

  return s.str();
}

/******************************ConstPtr***************************************/
string Kant2Cpp::generateH(const ConstPtr& pPtr) const {
  ostringstream s;

  if (pPtr->getConstGrammarPtr()->t == ConstGrammar::STRING) {
    string tmp = kant::KT_Common::replace(pPtr->getConstGrammarPtr()->v, "\"", "\\\"");
    s << TAB << "const " << tostr(pPtr->getTypeIdPtr()->getTypePtr()) << " " << pPtr->getTypeIdPtr()->getId() << " = \""
      << tmp << "\";" << endl;
  } else {
    s << TAB << "const " << tostr(pPtr->getTypeIdPtr()->getTypePtr()) << " " << pPtr->getTypeIdPtr()->getId() << " = "
      << pPtr->getConstGrammarPtr()->v
      << ((tostr(pPtr->getTypeIdPtr()->getTypePtr()) == _namespace + "::Int64") ? "LL;" : ";") << endl;
  }

  return s.str();
}
/******************************NamespacePtr***************************************/

string Kant2Cpp::generateH(const NamespacePtr& pPtr) const {
  ostringstream s;
  vector<InterfacePtr>& is = pPtr->getAllInterfacePtr();
  vector<StructPtr>& ss = pPtr->getAllStructPtr();
  vector<EnumPtr>& es = pPtr->getAllEnumPtr();
  vector<ConstPtr>& cs = pPtr->getAllConstPtr();

  s << endl;
  s << TAB << "namespace " << pPtr->getId() << endl;
  s << TAB << "{" << endl;
  INC_TAB;

  for (size_t i = 0; i < cs.size(); i++) {
    s << generateH(cs[i]) << endl;
  }

  for (size_t i = 0; i < es.size(); i++) {
    s << generateH(es[i]) << endl;
  }

  for (size_t i = 0; i < ss.size(); i++) {
    s << generateH(ss[i], pPtr->getId()) << endl;
  }

  s << endl;

  for (size_t i = 0; i < is.size() && _onlyStruct == false; i++) {
    s << generateH(is[i], pPtr) << endl;
    s << endl;
  }

  DEL_TAB;
  s << "}";

  s << endl << endl;

  return s.str();
}

void Kant2Cpp::generateH(const ContextPtr& pPtr) const {
  string n = g_parse->getFileName(pPtr->getFileName());
  string fileH = g_parse->getAbsoluteFileName(_baseDir, g_parse->replaceFileName(n, "h"));
  string fileCpp = g_parse->getAbsoluteFileName(_baseDir, g_parse->replaceFileName(n, "cpp"));
  //
  //    string n        = kant::KT_File::excludeFileExt(kant::KT_File::extractFileName(pPtr->getFileName()));
  //
  //    string fileH    = _baseDir + FILE_SEP + n + ".h";

  string define = kant::KT_Common::upper("__" + n + "_h_");

  ostringstream s;

  s << g_parse->printHeaderRemark();

  s << "#ifndef " << define << endl;
  s << "#define " << define << endl;
  s << endl;
  s << "#include <map>" << endl;
  s << "#include <string>" << endl;
  s << "#include <vector>" << endl;
  s << "#include \"tup/Kant.h\"" << endl;
  if (_bJsonSupport) s << "#include \"tup/KantJson.h\"" << endl;
  if (_bSqlSupport) s << "#include \"util/tc_mysql.h\"" << endl;
  if (_bXmlSupport) s << "#include \"tup/KantXml.h\"" << endl;

  s << "using namespace std;" << endl;

  vector<string> include = pPtr->getIncludes();
  for (size_t i = 0; i < include.size(); i++) {
    s << "#include \"" << g_parse->getHeader() << kant::KT_File::extractFileName(include[i]) << "\"" << endl;
  }

  vector<NamespacePtr> namespaces = pPtr->getNamespaces();

  //名字空间有接口
  for (size_t i = 0; i < namespaces.size() && _onlyStruct == false; i++) {
    if (namespaces[i]->hasInterface()) {
      s << "#include \"servant/ServantProxy.h\"" << endl;
      s << "#include \"servant/Servant.h\"" << endl;
      s << "#include \"promise/promise.h\"" << endl;
      if (_bTrace) {
        s << "#include \"servant/Application.h\"" << endl;
      }
      break;
    }
  }

  s << endl;

  for (size_t i = 0; i < namespaces.size(); i++) {
    s << generateH(namespaces[i]) << endl;
  }

  s << endl;
  s << "#endif" << endl;

  kant::KT_File::makeDirRecursive(_baseDir);
  kant::KT_File::save2file(fileH, s.str());
}

void Kant2Cpp::createFile(const string& file)  //, const vector<string>& vsCoder)
{
  std::vector<ContextPtr> contexts = g_parse->getContexts();
  for (size_t i = 0; i < contexts.size(); i++) {
    if (file == contexts[i]->getFileName()) {
      // if (vsCoder.size() == 0)
      // {
      generateH(contexts[i]);
      // }
      // else
      // {
      //     for (size_t j = 0; j < vsCoder.size(); j++)
      //     {
      //         generateCoder(contexts[i], vsCoder[j]);
      //     }
      // }
    }
  }
}

StructPtr Kant2Cpp::findStruct(const ContextPtr& pPtr, const string& id) {
  string sid = id;

  //在当前namespace中查找
  vector<NamespacePtr> namespaces = pPtr->getNamespaces();
  for (size_t i = 0; i < namespaces.size(); i++) {
    NamespacePtr np = namespaces[i];
    vector<StructPtr> structs = np->getAllStructPtr();

    for (size_t i = 0; i < structs.size(); i++) {
      if (structs[i]->getSid() == sid) {
        return structs[i];
      }
    }
  }

  return NULL;
}

// ////////////////////////////////
// //for coder generating
// ////////////////////////////////

// string Kant2Cpp::generateCoder(const NamespacePtr& pPtr, const string& sInterface) const
// {
//     ostringstream s;
//     vector<InterfacePtr>& is    = pPtr->getAllInterfacePtr();
//     vector<StructPtr>& ss    = pPtr->getAllStructPtr();
//     vector<EnumPtr>& es    = pPtr->getAllEnumPtr();
//     vector<ConstPtr>& cs    = pPtr->getAllConstPtr();

//     s << endl;
//     s << TAB << "namespace " << pPtr->getId() << endl;
//     s << TAB << "{" << endl;
//     INC_TAB;

//     for (size_t i = 0; i < cs.size(); i++)
//     {
//         s << generateH(cs[i]) << endl;
//     }

//     for (size_t i = 0; i < es.size(); i++)
//     {
//         s << generateH(es[i]) << endl;
//     }

//     for (size_t i = 0; i < ss.size(); i++)
//     {
//         s << generateH(ss[i], pPtr->getId()) << endl;
//     }

//     s << endl;

//     for (size_t i = 0; i < is.size(); i++)
//     {
//         if (pPtr->getId() + "::" + is[i]->getId() == sInterface)
//         {
//             s << generateCoder(is[i]) << endl;
//             s << endl;
//         }
//     }

//     DEL_TAB;
//     s << "}";

//     s << endl << endl;

//     return s.str();
// }

// string Kant2Cpp::generateCoder(const InterfacePtr& pPtr) const
// {
//     ostringstream s;

//     vector<OperationPtr>& vOperation = pPtr->getAllOperationPtr();

//     //生成编解码类
//     s << TAB << "// encode and decode for client" << endl;
//     s << TAB << "class " << pPtr->getId() << "Coder" << endl;
//     s << TAB << "{" << endl;
//     s << TAB << "public:" << endl << endl;
//     INC_TAB;
//     s << TAB << "typedef map<string, string> KANT_CONTEXT;" << endl << endl;

//     s << TAB << "enum enumResult" << endl;
//     s << TAB << "{" << endl;
//     INC_TAB;

//     s << TAB << "eKantServerSuccess      = 0," << endl;
//     s << TAB << "eKantPacketLess         = 1," << endl;
//     s << TAB << "eKantPacketErr          = 2," << endl;
//     s << TAB << "eKantServerDecodeErr    = -1," << endl;
//     s << TAB << "eKantServerEncodeErr    = -2," << endl;
//     s << TAB << "eKantServerNoFuncErr    = -3," << endl;
//     s << TAB << "eKantServerNoServantErr = -4," << endl;
//     s << TAB << "eKantServerQueueTimeout = -6," << endl;
//     s << TAB << "eKantAsyncCallTimeout   = -7," << endl;
//     s << TAB << "eKantProxyConnectErr    = -8," << endl;
//     s << TAB << "eKantServerUnknownErr   = -99," << endl;

//     DEL_TAB;
//     s << TAB << "};" << endl << endl;

//     for (size_t i = 0; i < vOperation.size(); i++)
//     {
//         s << generateCoder(vOperation[i]) << endl;
//     }

//     DEL_TAB;
//     s << TAB << "protected:" << endl << endl;
//     INC_TAB;
//     s << TAB << "static " + _namespace + "::Int32 fetchPacket(const string & in, string & out)" << endl;
//     s << TAB << "{" << endl;

//     INC_TAB;
//     s << TAB << "if(in.length() < sizeof(" + _namespace + "::Int32)) return eKantPacketLess;" << endl;

//     s << TAB << "" + _namespace + "::Int32 iHeaderLen;" << endl;
//     s << TAB << "memcpy(&iHeaderLen, in.c_str(), sizeof(" + _namespace + "::Int32));" << endl;

//     s << TAB << "iHeaderLen = ntohl(iHeaderLen);" << endl;
//     s << TAB << "if(iHeaderLen < (" + _namespace + "::Int32)sizeof(" + _namespace + "::Int32) || iHeaderLen > 100000000) return eKantPacketErr;" << endl;
//     s << TAB << "if((" + _namespace + "::Int32)in.length() < iHeaderLen) return eKantPacketLess;" << endl;

//     s << TAB << "out = in.substr(sizeof(" + _namespace + "::Int32), iHeaderLen - sizeof(" + _namespace + "::Int32)); " << endl;
//     s << TAB << "return 0;" << endl;

//     DEL_TAB;
//     s << TAB << "}" << endl;

//     s << endl;
//     s << TAB << "static string encodeBasePacket(const string & sServantName, const string & sFuncName, const vector<char> & buffer, "
//         << "const map<string, string>& context = KANT_CONTEXT())" << endl;
//     s << TAB << "{" << endl;
//     INC_TAB;

//     s << TAB << _namespace + "::KantOutputStream<" + _namespace + "::BufferWriterVector> os;" << endl;
//     s << TAB << "os.write(1, 1);" << endl;
//     s << TAB << "os.write(0, 2);" << endl;
//     s << TAB << "os.write(0, 3);" << endl;
//     s << TAB << "os.write(0, 4);" << endl;
//     s << TAB << "os.write(sServantName, 5);" << endl;
//     s << TAB << "os.write(sFuncName, 6);" << endl;
//     s << TAB << "os.write(buffer, 7);" << endl;
//     s << TAB << "os.write(60, 8);" << endl;
//     s << TAB << "os.write(context, 9);" << endl;
//     s << TAB << "os.write(map<string, string>(), 10);" << endl;

//     s << TAB << _namespace + "::Int32 iHeaderLen;" << endl;
//     s << TAB << "iHeaderLen = htonl(sizeof(" + _namespace + "::Int32) + os.getLength());" << endl;
//     s << TAB << "string s;" << endl;
//     s << TAB << "s.append((const char*)&iHeaderLen, sizeof(" + _namespace + "::Int32));" << endl;
//     s << TAB << "s.append(os.getBuffer(), os.getLength());" << endl;

//     s << TAB << "return s;" << endl;

//     DEL_TAB;
//     s << TAB << "}" << endl;

//     s << endl;
//     s << TAB << "static " + _namespace + "::Int32 decodeBasePacket(const string & in, " + _namespace + "::Int32 & iServerRet, vector<char> & buffer)" << endl;
//     s << TAB << "{" << endl;
//     INC_TAB;

//     s << TAB << _namespace + "::KantInputStream<" + _namespace + "::BufferReader> is;" << endl;
//     s << TAB << "is.setBuffer(in.c_str(), in.length());" << endl;
//     s << TAB << "is.read(iServerRet, 5, true);" << endl;
//     s << TAB << "is.read(buffer, 6, true);" << endl;

//     s << TAB << "return 0;" << endl;

//     DEL_TAB;
//     s << TAB << "}" << endl;

//     s << endl;

//     DEL_TAB;
//     s << TAB << "};" << endl;

//     return s.str();
// }

// string Kant2Cpp::generateCoder(const OperationPtr& pPtr) const
// {
//     ostringstream s;
//     vector<ParamDeclPtr>& vParamDecl = pPtr->getAllParamDeclPtr();

//     //编码函数
//     s << TAB << "//encode & decode function for '" << pPtr->getId() << "()'" << endl << endl;
//     s << TAB << "static string encode_" << pPtr->getId() << "(const string & sServantName, ";

//     for (size_t i = 0; i < vParamDecl.size(); i++)
//     {
//         if (!vParamDecl[i]->isOut())
//         {
//             s << generateH(vParamDecl[i]) << ",";
//         }
//     }
//     s << endl;
//     s << TAB << "    const map<string, string>& context = KANT_CONTEXT())" << endl;
//     s << TAB << "{" << endl;

//     INC_TAB;
//     s << TAB << "try" << endl;
//     s << TAB << "{" << endl;

//     INC_TAB;
//     s << TAB << _namespace + "::KantOutputStream<" + _namespace + "::BufferWriterVector> _os;" << endl;

//     for (size_t i = 0; i < vParamDecl.size(); i++)
//     {
//         if (vParamDecl[i]->isOut()) continue;
//         s << writeTo(vParamDecl[i]->getTypeIdPtr());
//     }

//     s << TAB << "return encodeBasePacket(sServantName, \"" << pPtr->getId() << "\", _os.getByteBuffer(), context);" << endl;

//     DEL_TAB;

//     s << TAB << "}" << endl;
//     s << TAB << "catch (" + _namespace + "::KantException & ex)" << endl;
//     s << TAB << "{" << endl;
//     INC_TAB;
//     s << TAB << "return \"\";" << endl;
//     DEL_TAB;
//     s << TAB << "}" << endl;
//     DEL_TAB;
//     s << TAB << "}" << endl;

//     s << endl;

//     //解码函数

//     s << TAB << "static " + _namespace + "::Int32 decode_" << pPtr->getId() << "(const string & in ";

//     if (pPtr->getReturnPtr()->getTypePtr())
//     {
//         s << ", " << tostr(pPtr->getReturnPtr()->getTypePtr()) << " & _ret ";
//     }
//     for (size_t i = 0; i < vParamDecl.size(); i++)
//     {
//         if(!vParamDecl[i]->isOut())
//             continue;

//         s << ", " << generateH(vParamDecl[i]);
//     }
//     s << ")" << endl;

//     s << TAB << "{" << endl;

//     INC_TAB;
//     s << TAB << "try" << endl;
//     s << TAB << "{" << endl;

//     INC_TAB;
//     s << TAB << "string out;" << endl;
//     s << TAB << _namespace + "::Int32 iRet = 0;" << endl;
//     s << TAB << "if((iRet = fetchPacket(in, out)) != 0) return iRet;" << endl;

//     s << TAB << _namespace + "::KantInputStream<" + _namespace + "::BufferReader> _is;" << endl;
//     s << TAB << _namespace + "::Int32 iServerRet=0;" << endl;
//     s << TAB << "vector<char> buffer;" << endl;
//     s << TAB << "decodeBasePacket(out, iServerRet, buffer);" << endl;
//     s << TAB << "if(iServerRet != 0)  return iServerRet;" << endl;

//     s << TAB << "_is.setBuffer(buffer);" << endl;

//     if (pPtr->getReturnPtr()->getTypePtr())
//     {
//         s << readFrom(pPtr->getReturnPtr());
//     }

//     for (size_t i = 0; i < vParamDecl.size(); i++)
//     {
//         if (vParamDecl[i]->isOut())
//         {
//             s << readFrom(vParamDecl[i]->getTypeIdPtr());
//         }
//     }

//     s << TAB << "return 0;" << endl;

//     DEL_TAB;
//     s << TAB << "}" << endl;
//     s << TAB << "catch (" + _namespace + "::KantException & ex)" << endl;
//     s << TAB << "{" << endl;
//     INC_TAB;
//     s << TAB << "return eKantPacketErr;" << endl;
//     DEL_TAB;
//     s << TAB << "}" << endl;

//     DEL_TAB;
//     s << TAB << "}" << endl;

//     s << endl;

//     return s.str();
// }

// void Kant2Cpp::generateCoder(const ContextPtr& pPtr, const string& sInterface) const
// {
//     cout << "Interface:" << sInterface << endl;
//     string n        = kant::KT_File::excludeFileExt(kant::KT_File::extractFileName(pPtr->getFileName())) + "Coder";

//     string fileH    = _baseDir + FILE_SEP + n + ".h";

//     string define   = kant::KT_Common::upper("__" + n + "_h_");

//     ostringstream s;

//     s << g_parse->printHeaderRemark();

//     s << "#ifndef " << define << endl;
//     s << "#define " << define << endl;
//     s << endl;
//     s << "#include <map>" << endl;
//     s << "#include <string>" << endl;
//     s << "#include <vector>" << endl;
//     s << "#include \"tup/Kant.h\"" << endl;

//     s << "using namespace std;" << endl;

//     vector<string> include = pPtr->getIncludes();
//     for (size_t i = 0; i < include.size(); i++)
//     {
//         s << "#include \"" << g_parse->getHeader()
//             << kant::KT_Common::replace(kant::KT_File::extractFileName(include[i]), ".h", "Coder.h") << "\"" << endl;
//     }

//     vector<NamespacePtr> namespaces = pPtr->getNamespaces();

//     s << endl;

//     for (size_t i = 0; i < namespaces.size(); i++)
//     {
//         s << generateCoder(namespaces[i], sInterface) << endl;
//     }

//     s << endl;
//     s << "#endif" << endl;

//
