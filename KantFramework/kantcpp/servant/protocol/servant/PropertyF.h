// **********************************************************************
// This file was generated by a KANT parser!
// KANT version KANT_VERSION.
// **********************************************************************

#ifndef __PROPERTYF_H_
#define __PROPERTYF_H_

#include <map>
#include <string>
#include <vector>
#include "tup/Kant.h"
#include "tup/KantJson.h"
using namespace std;
#include "servant/ServantProxy.h"
#include "servant/Servant.h"
#include "promise/promise.h"


namespace kant
{
    struct StatPropMsgHead : public kant::KantStructBase
    {
    public:
        static string className()
        {
            return "kant.StatPropMsgHead";
        }
        static string MD5()
        {
            return "4b63e9dda34ad67da1592c5b805610da";
        }
        StatPropMsgHead()
        {
            resetDefautlt();
        }
        void resetDefautlt()
        {
            moduleName = "";
            ip = "";
            propertyName = "";
            setName = "";
            setArea = "";
            setID = "";
            sContainer = "";
            iPropertyVer = 1;
        }
        template<typename WriterT>
        void writeTo(kant::KantOutputStream<WriterT>& _os) const
        {
            _os.write(moduleName, 0);
            _os.write(ip, 1);
            _os.write(propertyName, 2);
            if (setName != "")
            {
                _os.write(setName, 3);
            }
            if (setArea != "")
            {
                _os.write(setArea, 4);
            }
            if (setID != "")
            {
                _os.write(setID, 5);
            }
            if (sContainer != "")
            {
                _os.write(sContainer, 6);
            }
            if (iPropertyVer != 1)
            {
                _os.write(iPropertyVer, 7);
            }
        }
        template<typename ReaderT>
        void readFrom(kant::KantInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(moduleName, 0, true);
            _is.read(ip, 1, true);
            _is.read(propertyName, 2, true);
            _is.read(setName, 3, false);
            _is.read(setArea, 4, false);
            _is.read(setID, 5, false);
            _is.read(sContainer, 6, false);
            _is.read(iPropertyVer, 7, false);
        }
        kant::JsonValueObjPtr writeToJson() const
        {
            kant::JsonValueObjPtr p = std::make_shared<kant::JsonValueObj>();
            p->value["moduleName"] = kant::JsonOutput::writeJson(moduleName);
            p->value["ip"] = kant::JsonOutput::writeJson(ip);
            p->value["propertyName"] = kant::JsonOutput::writeJson(propertyName);
            p->value["setName"] = kant::JsonOutput::writeJson(setName);
            p->value["setArea"] = kant::JsonOutput::writeJson(setArea);
            p->value["setID"] = kant::JsonOutput::writeJson(setID);
            p->value["sContainer"] = kant::JsonOutput::writeJson(sContainer);
            p->value["iPropertyVer"] = kant::JsonOutput::writeJson(iPropertyVer);
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
            kant::JsonInput::readJson(moduleName,pObj->value["moduleName"], true);
            kant::JsonInput::readJson(ip,pObj->value["ip"], true);
            kant::JsonInput::readJson(propertyName,pObj->value["propertyName"], true);
            kant::JsonInput::readJson(setName,pObj->value["setName"], false);
            kant::JsonInput::readJson(setArea,pObj->value["setArea"], false);
            kant::JsonInput::readJson(setID,pObj->value["setID"], false);
            kant::JsonInput::readJson(sContainer,pObj->value["sContainer"], false);
            kant::JsonInput::readJson(iPropertyVer,pObj->value["iPropertyVer"], false);
        }
        void readFromJsonString(const string & str)
        {
            readFromJson(kant::KT_Json::getValue(str));
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.display(moduleName,"moduleName");
            _ds.display(ip,"ip");
            _ds.display(propertyName,"propertyName");
            _ds.display(setName,"setName");
            _ds.display(setArea,"setArea");
            _ds.display(setID,"setID");
            _ds.display(sContainer,"sContainer");
            _ds.display(iPropertyVer,"iPropertyVer");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.displaySimple(moduleName, true);
            _ds.displaySimple(ip, true);
            _ds.displaySimple(propertyName, true);
            _ds.displaySimple(setName, true);
            _ds.displaySimple(setArea, true);
            _ds.displaySimple(setID, true);
            _ds.displaySimple(sContainer, true);
            _ds.displaySimple(iPropertyVer, false);
            return _os;
        }
    public:
        std::string moduleName;
        std::string ip;
        std::string propertyName;
        std::string setName;
        std::string setArea;
        std::string setID;
        std::string sContainer;
        kant::Int32 iPropertyVer;
    };
    inline bool operator==(const StatPropMsgHead&l, const StatPropMsgHead&r)
    {
        return l.moduleName == r.moduleName && l.ip == r.ip && l.propertyName == r.propertyName && l.setName == r.setName && l.setArea == r.setArea && l.setID == r.setID && l.sContainer == r.sContainer && l.iPropertyVer == r.iPropertyVer;
    }
    inline bool operator!=(const StatPropMsgHead&l, const StatPropMsgHead&r)
    {
        return !(l == r);
    }
    inline ostream& operator<<(ostream & os,const StatPropMsgHead&r)
    {
        os << r.writeToJsonString();
        return os;
    }
    inline istream& operator>>(istream& is,StatPropMsgHead&l)
    {
        std::istreambuf_iterator<char> eos;
        std::string s(std::istreambuf_iterator<char>(is), eos);
        l.readFromJsonString(s);
        return is;
    }
    inline bool operator<(const StatPropMsgHead&l, const StatPropMsgHead&r)
    {
        if(l.moduleName != r.moduleName)  return (l.moduleName < r.moduleName);
        if(l.ip != r.ip)  return (l.ip < r.ip);
        if(l.propertyName != r.propertyName)  return (l.propertyName < r.propertyName);
        if(l.setName != r.setName)  return (l.setName < r.setName);
        if(l.setArea != r.setArea)  return (l.setArea < r.setArea);
        if(l.setID != r.setID)  return (l.setID < r.setID);
        return false;
    }
    inline bool operator<=(const StatPropMsgHead&l, const StatPropMsgHead&r)
    {
        return !(r < l);
    }
    inline bool operator>(const StatPropMsgHead&l, const StatPropMsgHead&r)
    {
        return r < l;
    }
    inline bool operator>=(const StatPropMsgHead&l, const StatPropMsgHead&r)
    {
        return !(l < r);
    }

    struct StatPropInfo : public kant::KantStructBase
    {
    public:
        static string className()
        {
            return "kant.StatPropInfo";
        }
        static string MD5()
        {
            return "325d87d477a8cf7a6468ed6bb39da964";
        }
        StatPropInfo()
        {
            resetDefautlt();
        }
        void resetDefautlt()
        {
            policy = "";
            value = "";
        }
        template<typename WriterT>
        void writeTo(kant::KantOutputStream<WriterT>& _os) const
        {
            _os.write(policy, 0);
            _os.write(value, 1);
        }
        template<typename ReaderT>
        void readFrom(kant::KantInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(policy, 0, true);
            _is.read(value, 1, true);
        }
        kant::JsonValueObjPtr writeToJson() const
        {
            kant::JsonValueObjPtr p = std::make_shared<kant::JsonValueObj>();
            p->value["policy"] = kant::JsonOutput::writeJson(policy);
            p->value["value"] = kant::JsonOutput::writeJson(value);
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
            kant::JsonInput::readJson(policy,pObj->value["policy"], true);
            kant::JsonInput::readJson(value,pObj->value["value"], true);
        }
        void readFromJsonString(const string & str)
        {
            readFromJson(kant::KT_Json::getValue(str));
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.display(policy,"policy");
            _ds.display(value,"value");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.displaySimple(policy, true);
            _ds.displaySimple(value, false);
            return _os;
        }
    public:
        std::string policy;
        std::string value;
    };
    inline bool operator==(const StatPropInfo&l, const StatPropInfo&r)
    {
        return l.policy == r.policy && l.value == r.value;
    }
    inline bool operator!=(const StatPropInfo&l, const StatPropInfo&r)
    {
        return !(l == r);
    }
    inline ostream& operator<<(ostream & os,const StatPropInfo&r)
    {
        os << r.writeToJsonString();
        return os;
    }
    inline istream& operator>>(istream& is,StatPropInfo&l)
    {
        std::istreambuf_iterator<char> eos;
        std::string s(std::istreambuf_iterator<char>(is), eos);
        l.readFromJsonString(s);
        return is;
    }

    struct StatPropMsgBody : public kant::KantStructBase
    {
    public:
        static string className()
        {
            return "kant.StatPropMsgBody";
        }
        static string MD5()
        {
            return "8c122c103f91e1ccc311770ded2e7334";
        }
        StatPropMsgBody()
        {
            resetDefautlt();
        }
        void resetDefautlt()
        {
            vInfo.clear();
        }
        template<typename WriterT>
        void writeTo(kant::KantOutputStream<WriterT>& _os) const
        {
            _os.write(vInfo, 0);
        }
        template<typename ReaderT>
        void readFrom(kant::KantInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(vInfo, 0, true);
        }
        kant::JsonValueObjPtr writeToJson() const
        {
            kant::JsonValueObjPtr p = std::make_shared<kant::JsonValueObj>();
            p->value["vInfo"] = kant::JsonOutput::writeJson(vInfo);
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
            kant::JsonInput::readJson(vInfo,pObj->value["vInfo"], true);
        }
        void readFromJsonString(const string & str)
        {
            readFromJson(kant::KT_Json::getValue(str));
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.display(vInfo,"vInfo");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.displaySimple(vInfo, false);
            return _os;
        }
    public:
        vector<kant::StatPropInfo> vInfo;
    };
    inline bool operator==(const StatPropMsgBody&l, const StatPropMsgBody&r)
    {
        return l.vInfo == r.vInfo;
    }
    inline bool operator!=(const StatPropMsgBody&l, const StatPropMsgBody&r)
    {
        return !(l == r);
    }
    inline ostream& operator<<(ostream & os,const StatPropMsgBody&r)
    {
        os << r.writeToJsonString();
        return os;
    }
    inline istream& operator>>(istream& is,StatPropMsgBody&l)
    {
        std::istreambuf_iterator<char> eos;
        std::string s(std::istreambuf_iterator<char>(is), eos);
        l.readFromJsonString(s);
        return is;
    }


    /* callback of async proxy for client */
    class PropertyFPrxCallback: public kant::ServantProxyCallback
    {
    public:
        virtual ~PropertyFPrxCallback(){}
        virtual void callback_reportPropMsg(kant::Int32 ret)
        { throw std::runtime_error("callback_reportPropMsg() override incorrect."); }
        virtual void callback_reportPropMsg_exception(kant::Int32 ret)
        { throw std::runtime_error("callback_reportPropMsg_exception() override incorrect."); }

    public:
        virtual const map<std::string, std::string> & getResponseContext() const
        {
            CallbackThreadData * pCbtd = CallbackThreadData::getData();
            assert(pCbtd != NULL);

            if(!pCbtd->getContextValid())
            {
                throw KT_Exception("cann't get response context");
            }
            return pCbtd->getResponseContext();
        }

    public:
        virtual int onDispatch(kant::ReqMessagePtr msg)
        {
            static ::std::string __PropertyF_all[]=
            {
                "reportPropMsg"
            };
            pair<string*, string*> r = equal_range(__PropertyF_all, __PropertyF_all+1, string(msg->request.sFuncName));
            if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;
            switch(r.first - __PropertyF_all)
            {
                case 0:
                {
                    if (msg->response->iRet != kant::KANTSERVERSUCCESS)
                    {
                        callback_reportPropMsg_exception(msg->response->iRet);

                        return msg->response->iRet;
                    }
                    kant::KantInputStream<kant::BufferReader> _is;

                    _is.setBuffer(msg->response->sBuffer);
                    kant::Int32 _ret;
                    _is.read(_ret, 0, true);

                    CallbackThreadData * pCbtd = CallbackThreadData::getData();
                    assert(pCbtd != NULL);

                    pCbtd->setResponseContext(msg->response->context);

                    callback_reportPropMsg(_ret);

                    pCbtd->delResponseContext();

                    return kant::KANTSERVERSUCCESS;

                }
            }
            return kant::KANTSERVERNOFUNCERR;
        }

    };
    typedef std::shared_ptr<PropertyFPrxCallback> PropertyFPrxCallbackPtr;

    //callback of promise async proxy for client
    class PropertyFPrxCallbackPromise: public kant::ServantProxyCallback
    {
    public:
        virtual ~PropertyFPrxCallbackPromise(){}
    public:
        struct PromisereportPropMsg
        {
        public:
            kant::Int32 _ret;
            map<std::string, std::string> _mRspContext;
        };
        
        typedef std::shared_ptr< PropertyFPrxCallbackPromise::PromisereportPropMsg > PromisereportPropMsgPtr;

        PropertyFPrxCallbackPromise(const kant::Promise< PropertyFPrxCallbackPromise::PromisereportPropMsgPtr > &promise)
        : _promise_reportPropMsg(promise)
        {}
        
        virtual void callback_reportPropMsg(const PropertyFPrxCallbackPromise::PromisereportPropMsgPtr &ptr)
        {
            _promise_reportPropMsg.setValue(ptr);
        }
        virtual void callback_reportPropMsg_exception(kant::Int32 ret)
        {
            std::string str("");
            str += "Function:reportPropMsg_exception|Ret:";
            str += KT_Common::tostr(ret);
            _promise_reportPropMsg.setException(kant::copyException(str, ret));
        }

    protected:
        kant::Promise< PropertyFPrxCallbackPromise::PromisereportPropMsgPtr > _promise_reportPropMsg;

    public:
        virtual int onDispatch(kant::ReqMessagePtr msg)
        {
            static ::std::string __PropertyF_all[]=
            {
                "reportPropMsg"
            };

            pair<string*, string*> r = equal_range(__PropertyF_all, __PropertyF_all+1, string(msg->request.sFuncName));
            if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;
            switch(r.first - __PropertyF_all)
            {
                case 0:
                {
                    if (msg->response->iRet != kant::KANTSERVERSUCCESS)
                    {
                        callback_reportPropMsg_exception(msg->response->iRet);

                        return msg->response->iRet;
                    }
                    kant::KantInputStream<kant::BufferReader> _is;

                    _is.setBuffer(msg->response->sBuffer);

                    PropertyFPrxCallbackPromise::PromisereportPropMsgPtr ptr = std::make_shared<PropertyFPrxCallbackPromise::PromisereportPropMsg>();

                    try
                    {
                        _is.read(ptr->_ret, 0, true);

                    }
                    catch(std::exception &ex)
                    {
                        callback_reportPropMsg_exception(kant::KANTCLIENTDECODEERR);

                        return kant::KANTCLIENTDECODEERR;
                    }
                    catch(...)
                    {
                        callback_reportPropMsg_exception(kant::KANTCLIENTDECODEERR);

                        return kant::KANTCLIENTDECODEERR;
                    }

                    ptr->_mRspContext = msg->response->context;

                    callback_reportPropMsg(ptr);

                    return kant::KANTSERVERSUCCESS;

                }
            }
            return kant::KANTSERVERNOFUNCERR;
        }

    };
    typedef std::shared_ptr<PropertyFPrxCallbackPromise> PropertyFPrxCallbackPromisePtr;

    /* callback of coroutine async proxy for client */
    class PropertyFCoroPrxCallback: public PropertyFPrxCallback
    {
    public:
        virtual ~PropertyFCoroPrxCallback(){}
    public:
        virtual const map<std::string, std::string> & getResponseContext() const { return _mRspContext; }

        virtual void setResponseContext(const map<std::string, std::string> &mContext) { _mRspContext = mContext; }

    public:
        int onDispatch(kant::ReqMessagePtr msg)
        {
            static ::std::string __PropertyF_all[]=
            {
                "reportPropMsg"
            };

            pair<string*, string*> r = equal_range(__PropertyF_all, __PropertyF_all+1, string(msg->request.sFuncName));
            if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;
            switch(r.first - __PropertyF_all)
            {
                case 0:
                {
                    if (msg->response->iRet != kant::KANTSERVERSUCCESS)
                    {
                        callback_reportPropMsg_exception(msg->response->iRet);

                        return msg->response->iRet;
                    }
                    kant::KantInputStream<kant::BufferReader> _is;

                    _is.setBuffer(msg->response->sBuffer);
                    try
                    {
                        kant::Int32 _ret;
                        _is.read(_ret, 0, true);

                        setResponseContext(msg->response->context);

                        callback_reportPropMsg(_ret);

                    }
                    catch(std::exception &ex)
                    {
                        callback_reportPropMsg_exception(kant::KANTCLIENTDECODEERR);

                        return kant::KANTCLIENTDECODEERR;
                    }
                    catch(...)
                    {
                        callback_reportPropMsg_exception(kant::KANTCLIENTDECODEERR);

                        return kant::KANTCLIENTDECODEERR;
                    }

                    return kant::KANTSERVERSUCCESS;

                }
            }
            return kant::KANTSERVERNOFUNCERR;
        }

    protected:
        map<std::string, std::string> _mRspContext;
    };
    typedef std::shared_ptr<PropertyFCoroPrxCallback> PropertyFCoroPrxCallbackPtr;

    /* proxy for client */
    class PropertyFProxy : public kant::ServantProxy
    {
    public:
        typedef map<string, string> KANT_CONTEXT;
        PropertyFProxy(Communicator* pCommunicator, const string& name, const string& setName)
                : ServantProxy(pCommunicator, name, setName) {}

        kant::Int32 reportPropMsg(const map<kant::StatPropMsgHead, kant::StatPropMsgBody> & statmsg,const map<string, string> &context = KANT_CONTEXT(),map<string, string> * pResponseContext = NULL)
        {
            kant::KantOutputStream<kant::BufferWriterVector> _os;
            _os.write(statmsg, 1);
            std::map<string, string> _mStatus;
            shared_ptr<kant::ResponsePacket> rep = kant_invoke(kant::KANTNORMAL,"reportPropMsg", _os, context, _mStatus);
            if(pResponseContext)
            {
                pResponseContext->swap(rep->context);
            }

            kant::KantInputStream<kant::BufferReader> _is;
            _is.setBuffer(rep->sBuffer);
            kant::Int32 _ret;
            _is.read(_ret, 0, true);
            return _ret;
        }

        void async_reportPropMsg(PropertyFPrxCallbackPtr callback,const map<kant::StatPropMsgHead, kant::StatPropMsgBody> &statmsg,const map<string, string>& context = KANT_CONTEXT())
        {
            kant::KantOutputStream<kant::BufferWriterVector> _os;
            _os.write(statmsg, 1);
            std::map<string, string> _mStatus;
            kant_invoke_async(kant::KANTNORMAL,"reportPropMsg", _os, context, _mStatus, callback);
        }
        
        kant::Future< PropertyFPrxCallbackPromise::PromisereportPropMsgPtr > promise_async_reportPropMsg(const map<kant::StatPropMsgHead, kant::StatPropMsgBody> &statmsg,const map<string, string>& context)
        {
            kant::Promise< PropertyFPrxCallbackPromise::PromisereportPropMsgPtr > promise;
            PropertyFPrxCallbackPromisePtr callback = std::make_shared<PropertyFPrxCallbackPromise>(promise);

            kant::KantOutputStream<kant::BufferWriterVector> _os;
            _os.write(statmsg, 1);
            std::map<string, string> _mStatus;
            kant_invoke_async(kant::KANTNORMAL,"reportPropMsg", _os, context, _mStatus, callback);

            return promise.getFuture();
        }

        void coro_reportPropMsg(PropertyFCoroPrxCallbackPtr callback,const map<kant::StatPropMsgHead, kant::StatPropMsgBody> &statmsg,const map<string, string>& context = KANT_CONTEXT())
        {
            kant::KantOutputStream<kant::BufferWriterVector> _os;
            _os.write(statmsg, 1);
            std::map<string, string> _mStatus;
            kant_invoke_async(kant::KANTNORMAL,"reportPropMsg", _os, context, _mStatus, callback, true);
        }

        PropertyFProxy* kant_hash(int64_t key)
        {
            return (PropertyFProxy*)ServantProxy::kant_hash(key);
        }

        PropertyFProxy* kant_consistent_hash(int64_t key)
        {
            return (PropertyFProxy*)ServantProxy::kant_consistent_hash(key);
        }

        PropertyFProxy* kant_open_trace(bool traceParam = false)
        {
            return (PropertyFProxy*)ServantProxy::kant_open_trace(traceParam);
        }

        PropertyFProxy* kant_set_timeout(int msecond)
        {
            return (PropertyFProxy*)ServantProxy::kant_set_timeout(msecond);
        }

        static const char* kant_prxname() { return "PropertyFProxy"; }
    };
    typedef std::shared_ptr<PropertyFProxy> PropertyFPrx;

    /* servant for server */
    class PropertyF : public kant::Servant
    {
    public:
        virtual ~PropertyF(){}
        virtual kant::Int32 reportPropMsg(const map<kant::StatPropMsgHead, kant::StatPropMsgBody> & statmsg,kant::KantCurrentPtr current) = 0;
        static void async_response_reportPropMsg(kant::KantCurrentPtr current, kant::Int32 _ret)
        {
            if (current->getRequestVersion() == TUPVERSION )
            {
                UniAttribute<kant::BufferWriterVector, kant::BufferReader>  kantAttr;
                kantAttr.setVersion(current->getRequestVersion());
                kantAttr.put("", _ret);
                kantAttr.put("kant_ret", _ret);

                vector<char> sTupResponseBuffer;
                kantAttr.encode(sTupResponseBuffer);
                current->sendResponse(kant::KANTSERVERSUCCESS, sTupResponseBuffer);
            }
            else if (current->getRequestVersion() == JSONVERSION)
            {
                kant::JsonValueObjPtr _p = std::make_shared< kant::JsonValueObj>();
                _p->value["kant_ret"] = kant::JsonOutput::writeJson(_ret);
                vector<char> sJsonResponseBuffer;
                kant::KT_Json::writeValue(_p, sJsonResponseBuffer);
                current->sendResponse(kant::KANTSERVERSUCCESS, sJsonResponseBuffer);
            }
            else
            {
                kant::KantOutputStream<kant::BufferWriterVector> _os;
                _os.write(_ret, 0);

                current->sendResponse(kant::KANTSERVERSUCCESS, _os.getByteBuffer());
            }
        }

    public:
        int onDispatch(kant::KantCurrentPtr _current, vector<char> &_sResponseBuffer)
        {
            static ::std::string __kant__PropertyF_all[]=
            {
                "reportPropMsg"
            };

            pair<string*, string*> r = equal_range(__kant__PropertyF_all, __kant__PropertyF_all+1, _current->getFuncName());
            if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;
            switch(r.first - __kant__PropertyF_all)
            {
                case 0:
                {
                    kant::KantInputStream<kant::BufferReader> _is;
                    _is.setBuffer(_current->getRequestBuffer());
                    map<kant::StatPropMsgHead, kant::StatPropMsgBody> statmsg;
                    if (_current->getRequestVersion() == TUPVERSION)
                    {
                        UniAttribute<kant::BufferWriterVector, kant::BufferReader>  kantAttr;
                        kantAttr.setVersion(_current->getRequestVersion());
                        kantAttr.decode(_current->getRequestBuffer());
                        kantAttr.get("statmsg", statmsg);
                    }
                    else if (_current->getRequestVersion() == JSONVERSION)
                    {
                        kant::JsonValueObjPtr _jsonPtr = std::dynamic_pointer_cast<kant::JsonValueObj>(kant::KT_Json::getValue(_current->getRequestBuffer()));
                        kant::JsonInput::readJson(statmsg, _jsonPtr->value["statmsg"], true);
                    }
                    else
                    {
                        _is.read(statmsg, 1, true);
                    }
                    kant::Int32 _ret = reportPropMsg(statmsg, _current);
                    if(_current->isResponse())
                    {
                        if (_current->getRequestVersion() == TUPVERSION)
                        {
                            UniAttribute<kant::BufferWriterVector, kant::BufferReader>  kantAttr;
                            kantAttr.setVersion(_current->getRequestVersion());
                            kantAttr.put("", _ret);
                            kantAttr.put("kant_ret", _ret);
                            kantAttr.encode(_sResponseBuffer);
                        }
                        else if (_current->getRequestVersion() == JSONVERSION)
                        {
                            kant::JsonValueObjPtr _p = std::make_shared< kant::JsonValueObj>();
                            _p->value["kant_ret"] = kant::JsonOutput::writeJson(_ret);
                            kant::KT_Json::writeValue(_p, _sResponseBuffer);
                        }
                        else
                        {
                            kant::KantOutputStream<kant::BufferWriterVector> _os;
                            _os.write(_ret, 0);
                            _os.swap(_sResponseBuffer);
                        }
                    }
                    return kant::KANTSERVERSUCCESS;

                }
            }
            return kant::KANTSERVERNOFUNCERR;
        }
    };


}



#endif
