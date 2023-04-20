// **********************************************************************
// This file was generated by a KANT parser!
// KANT version KANT_VERSION.
// **********************************************************************

#ifndef __REQUESTF_H_
#define __REQUESTF_H_

#include <map>
#include <string>
#include <vector>
#include "tup/Kant.h"
#include "tup/KantJson.h"
using namespace std;


namespace kant
{
    struct RequestPacket : public kant::KantStructBase
    {
    public:
        static string className()
        {
            return "kant.RequestPacket";
        }
        static string MD5()
        {
            return "2a077f9b4ba3951ae36bed7ae78b580e";
        }
        RequestPacket()
        {
            resetDefautlt();
        }
        void resetDefautlt()
        {
            iVersion = 0;
            cPacketType = 0;
            iMessageType = 0;
            iRequestId = 0;
            sServantName = "";
            sFuncName = "";
            sBuffer.clear();
            iTimeout = 0;
            context.clear();
            status.clear();
        }
        template<typename WriterT>
        void writeTo(kant::KantOutputStream<WriterT>& _os) const
        {
            _os.write(iVersion, 1);
            _os.write(cPacketType, 2);
            _os.write(iMessageType, 3);
            _os.write(iRequestId, 4);
            _os.write(sServantName, 5);
            _os.write(sFuncName, 6);
            _os.write(sBuffer, 7);
            _os.write(iTimeout, 8);
            _os.write(context, 9);
            _os.write(status, 10);
        }
        template<typename ReaderT>
        void readFrom(kant::KantInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(iVersion, 1, true);
            _is.read(cPacketType, 2, true);
            _is.read(iMessageType, 3, true);
            _is.read(iRequestId, 4, true);
            _is.read(sServantName, 5, true);
            _is.read(sFuncName, 6, true);
            _is.read(sBuffer, 7, true);
            _is.read(iTimeout, 8, true);
            _is.read(context, 9, true);
            _is.read(status, 10, true);
        }
        kant::JsonValueObjPtr writeToJson() const
        {
            kant::JsonValueObjPtr p = std::make_shared<kant::JsonValueObj>();
            p->value["iVersion"] = kant::JsonOutput::writeJson(iVersion);
            p->value["cPacketType"] = kant::JsonOutput::writeJson(cPacketType);
            p->value["iMessageType"] = kant::JsonOutput::writeJson(iMessageType);
            p->value["iRequestId"] = kant::JsonOutput::writeJson(iRequestId);
            p->value["sServantName"] = kant::JsonOutput::writeJson(sServantName);
            p->value["sFuncName"] = kant::JsonOutput::writeJson(sFuncName);
            p->value["sBuffer"] = kant::JsonOutput::writeJson(sBuffer);
            p->value["iTimeout"] = kant::JsonOutput::writeJson(iTimeout);
            p->value["context"] = kant::JsonOutput::writeJson(context);
            p->value["status"] = kant::JsonOutput::writeJson(status);
            return p;
        }
        string writeToJsonString() const
        {
            return kant::KT_Json::writeValue(writeToJson());
        }
        void readFromJson(const kant::JsonValuePtr & p, bool isRequire = true)
        {
            resetDefautlt();
            if(NULL == p.get() || p->getType() != kant::eJsonTypeObj)
            {
                char s[128];
                snprintf(s, sizeof(s), "read 'struct' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
                throw kant::KT_Json_Exception(s);
            }
            kant::JsonValueObjPtr pObj=std::dynamic_pointer_cast<kant::JsonValueObj>(p);
            kant::JsonInput::readJson(iVersion,pObj->value["iVersion"], true);
            kant::JsonInput::readJson(cPacketType,pObj->value["cPacketType"], true);
            kant::JsonInput::readJson(iMessageType,pObj->value["iMessageType"], true);
            kant::JsonInput::readJson(iRequestId,pObj->value["iRequestId"], true);
            kant::JsonInput::readJson(sServantName,pObj->value["sServantName"], true);
            kant::JsonInput::readJson(sFuncName,pObj->value["sFuncName"], true);
            kant::JsonInput::readJson(sBuffer,pObj->value["sBuffer"], true);
            kant::JsonInput::readJson(iTimeout,pObj->value["iTimeout"], true);
            kant::JsonInput::readJson(context,pObj->value["context"], true);
            kant::JsonInput::readJson(status,pObj->value["status"], true);
        }
        void readFromJsonString(const string & str)
        {
            readFromJson(kant::KT_Json::getValue(str));
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.display(iVersion,"iVersion");
            _ds.display(cPacketType,"cPacketType");
            _ds.display(iMessageType,"iMessageType");
            _ds.display(iRequestId,"iRequestId");
            _ds.display(sServantName,"sServantName");
            _ds.display(sFuncName,"sFuncName");
            _ds.display(sBuffer,"sBuffer");
            _ds.display(iTimeout,"iTimeout");
            _ds.display(context,"context");
            _ds.display(status,"status");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.displaySimple(iVersion, true);
            _ds.displaySimple(cPacketType, true);
            _ds.displaySimple(iMessageType, true);
            _ds.displaySimple(iRequestId, true);
            _ds.displaySimple(sServantName, true);
            _ds.displaySimple(sFuncName, true);
            _ds.displaySimple(sBuffer, true);
            _ds.displaySimple(iTimeout, true);
            _ds.displaySimple(context, true);
            _ds.displaySimple(status, false);
            return _os;
        }
    public:
        kant::Short iVersion;
        kant::Char cPacketType;
        kant::Int32 iMessageType;
        kant::Int32 iRequestId;
        std::string sServantName;
        std::string sFuncName;
        vector<kant::Char> sBuffer;
        kant::Int32 iTimeout;
        map<std::string, std::string> context;
        map<std::string, std::string> status;
    };
    inline bool operator==(const RequestPacket&l, const RequestPacket&r)
    {
        return l.iVersion == r.iVersion && l.cPacketType == r.cPacketType && l.iMessageType == r.iMessageType && l.iRequestId == r.iRequestId && l.sServantName == r.sServantName && l.sFuncName == r.sFuncName && l.sBuffer == r.sBuffer && l.iTimeout == r.iTimeout && l.context == r.context && l.status == r.status;
    }
    inline bool operator!=(const RequestPacket&l, const RequestPacket&r)
    {
        return !(l == r);
    }
    inline ostream& operator<<(ostream & os,const RequestPacket&r)
    {
        os << r.writeToJsonString();
        return os;
    }
    inline istream& operator>>(istream& is,RequestPacket&l)
    {
        std::istreambuf_iterator<char> eos;
        std::string s(std::istreambuf_iterator<char>(is), eos);
        l.readFromJsonString(s);
        return is;
    }

    struct ResponsePacket : public kant::KantStructBase
    {
    public:
        static string className()
        {
            return "kant.ResponsePacket";
        }
        static string MD5()
        {
            return "d0337220d89fccd5aa21b25920dbcb2b";
        }
        ResponsePacket()
        {
            resetDefautlt();
        }
        void resetDefautlt()
        {
            iVersion = 0;
            cPacketType = 0;
            iRequestId = 0;
            iMessageType = 0;
            iRet = 0;
            sBuffer.clear();
            status.clear();
            sResultDesc = "";
            context.clear();
        }
        template<typename WriterT>
        void writeTo(kant::KantOutputStream<WriterT>& _os) const
        {
            _os.write(iVersion, 1);
            _os.write(cPacketType, 2);
            _os.write(iRequestId, 3);
            _os.write(iMessageType, 4);
            _os.write(iRet, 5);
            _os.write(sBuffer, 6);
            _os.write(status, 7);
            if (sResultDesc != "")
            {
                _os.write(sResultDesc, 8);
            }
            if (context.size() > 0)
            {
                _os.write(context, 9);
            }
        }
        template<typename ReaderT>
        void readFrom(kant::KantInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(iVersion, 1, true);
            _is.read(cPacketType, 2, true);
            _is.read(iRequestId, 3, true);
            _is.read(iMessageType, 4, true);
            _is.read(iRet, 5, true);
            _is.read(sBuffer, 6, true);
            _is.read(status, 7, true);
            _is.read(sResultDesc, 8, false);
            _is.read(context, 9, false);
        }
        kant::JsonValueObjPtr writeToJson() const
        {
            kant::JsonValueObjPtr p = std::make_shared<kant::JsonValueObj>();
            p->value["iVersion"] = kant::JsonOutput::writeJson(iVersion);
            p->value["cPacketType"] = kant::JsonOutput::writeJson(cPacketType);
            p->value["iRequestId"] = kant::JsonOutput::writeJson(iRequestId);
            p->value["iMessageType"] = kant::JsonOutput::writeJson(iMessageType);
            p->value["iRet"] = kant::JsonOutput::writeJson(iRet);
            p->value["sBuffer"] = kant::JsonOutput::writeJson(sBuffer);
            p->value["status"] = kant::JsonOutput::writeJson(status);
            p->value["sResultDesc"] = kant::JsonOutput::writeJson(sResultDesc);
            p->value["context"] = kant::JsonOutput::writeJson(context);
            return p;
        }
        string writeToJsonString() const
        {
            return kant::KT_Json::writeValue(writeToJson());
        }
        void readFromJson(const kant::JsonValuePtr & p, bool isRequire = true)
        {
            resetDefautlt();
            if(NULL == p.get() || p->getType() != kant::eJsonTypeObj)
            {
                char s[128];
                snprintf(s, sizeof(s), "read 'struct' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
                throw kant::KT_Json_Exception(s);
            }
            kant::JsonValueObjPtr pObj=std::dynamic_pointer_cast<kant::JsonValueObj>(p);
            kant::JsonInput::readJson(iVersion,pObj->value["iVersion"], true);
            kant::JsonInput::readJson(cPacketType,pObj->value["cPacketType"], true);
            kant::JsonInput::readJson(iRequestId,pObj->value["iRequestId"], true);
            kant::JsonInput::readJson(iMessageType,pObj->value["iMessageType"], true);
            kant::JsonInput::readJson(iRet,pObj->value["iRet"], true);
            kant::JsonInput::readJson(sBuffer,pObj->value["sBuffer"], true);
            kant::JsonInput::readJson(status,pObj->value["status"], true);
            kant::JsonInput::readJson(sResultDesc,pObj->value["sResultDesc"], false);
            kant::JsonInput::readJson(context,pObj->value["context"], false);
        }
        void readFromJsonString(const string & str)
        {
            readFromJson(kant::KT_Json::getValue(str));
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.display(iVersion,"iVersion");
            _ds.display(cPacketType,"cPacketType");
            _ds.display(iRequestId,"iRequestId");
            _ds.display(iMessageType,"iMessageType");
            _ds.display(iRet,"iRet");
            _ds.display(sBuffer,"sBuffer");
            _ds.display(status,"status");
            _ds.display(sResultDesc,"sResultDesc");
            _ds.display(context,"context");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.displaySimple(iVersion, true);
            _ds.displaySimple(cPacketType, true);
            _ds.displaySimple(iRequestId, true);
            _ds.displaySimple(iMessageType, true);
            _ds.displaySimple(iRet, true);
            _ds.displaySimple(sBuffer, true);
            _ds.displaySimple(status, true);
            _ds.displaySimple(sResultDesc, true);
            _ds.displaySimple(context, false);
            return _os;
        }
    public:
        kant::Short iVersion;
        kant::Char cPacketType;
        kant::Int32 iRequestId;
        kant::Int32 iMessageType;
        kant::Int32 iRet;
        vector<kant::Char> sBuffer;
        map<std::string, std::string> status;
        std::string sResultDesc;
        map<std::string, std::string> context;
    };
    inline bool operator==(const ResponsePacket&l, const ResponsePacket&r)
    {
        return l.iVersion == r.iVersion && l.cPacketType == r.cPacketType && l.iRequestId == r.iRequestId && l.iMessageType == r.iMessageType && l.iRet == r.iRet && l.sBuffer == r.sBuffer && l.status == r.status && l.sResultDesc == r.sResultDesc && l.context == r.context;
    }
    inline bool operator!=(const ResponsePacket&l, const ResponsePacket&r)
    {
        return !(l == r);
    }
    inline ostream& operator<<(ostream & os,const ResponsePacket&r)
    {
        os << r.writeToJsonString();
        return os;
    }
    inline istream& operator>>(istream& is,ResponsePacket&l)
    {
        std::istreambuf_iterator<char> eos;
        std::string s(std::istreambuf_iterator<char>(is), eos);
        l.readFromJsonString(s);
        return is;
    }


}



#endif
