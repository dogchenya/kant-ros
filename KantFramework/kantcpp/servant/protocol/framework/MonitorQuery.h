// **********************************************************************
// This file was generated by a KANT parser!
// KANT version KANT_VERSION.
// **********************************************************************

#ifndef __MONITORQUERY_H_
#define __MONITORQUERY_H_

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
    enum OP
    {
        EQ,
        GT,
        GTE,
        LT,
        LTE,
        LIKE,
    };
    inline string etos(const OP & e)
    {
        switch(e)
        {
            case EQ: return "EQ";
            case GT: return "GT";
            case GTE: return "GTE";
            case LT: return "LT";
            case LTE: return "LTE";
            case LIKE: return "LIKE";
            default: return "";
        }
    }
    inline int stoe(const string & s, OP & e)
    {
        if(s == "EQ")  { e=EQ; return 0;}
        if(s == "GT")  { e=GT; return 0;}
        if(s == "GTE")  { e=GTE; return 0;}
        if(s == "LT")  { e=LT; return 0;}
        if(s == "LTE")  { e=LTE; return 0;}
        if(s == "LIKE")  { e=LIKE; return 0;}

        return -1;
    }

    struct Condition : public kant::KantStructBase
    {
    public:
        static string className()
        {
            return "kant.Condition";
        }
        static string MD5()
        {
            return "0628aeee3404bacfd25c68461638e8fb";
        }
        Condition()
        {
            resetDefautlt();
        }
        void resetDefautlt()
        {
            field = "";
            op = kant::EQ;
            val = "";
        }
        template<typename WriterT>
        void writeTo(kant::KantOutputStream<WriterT>& _os) const
        {
            _os.write(field, 0);
            _os.write((kant::Int32)op, 1);
            _os.write(val, 2);
        }
        template<typename ReaderT>
        void readFrom(kant::KantInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(field, 0, true);
            kant::Int32 eTemp1 = kant::EQ;
            _is.read(eTemp1, 1, true);
            op = (kant::OP)eTemp1;
            _is.read(val, 2, true);
        }
        kant::JsonValueObjPtr writeToJson() const
        {
            kant::JsonValueObjPtr p = std::make_shared<kant::JsonValueObj>();
            p->value["field"] = kant::JsonOutput::writeJson(field);
            p->value["op"] = kant::JsonOutput::writeJson((kant::Int32)op);
            p->value["val"] = kant::JsonOutput::writeJson(val);
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
            kant::JsonInput::readJson(field,pObj->value["field"], true);
            kant::JsonInput::readJson(op,pObj->value["op"], true);
            kant::JsonInput::readJson(val,pObj->value["val"], true);
        }
        void readFromJsonString(const string & str)
        {
            readFromJson(kant::KT_Json::getValue(str));
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.display(field,"field");
            _ds.display((kant::Int32)op,"op");
            _ds.display(val,"val");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.displaySimple(field, true);
            _ds.displaySimple((kant::Int32)op, true);
            _ds.displaySimple(val, false);
            return _os;
        }
    public:
        std::string field;
        kant::OP op;
        std::string val;
    };
    inline bool operator==(const Condition&l, const Condition&r)
    {
        return l.field == r.field && l.op == r.op && l.val == r.val;
    }
    inline bool operator!=(const Condition&l, const Condition&r)
    {
        return !(l == r);
    }
    inline ostream& operator<<(ostream & os,const Condition&r)
    {
        os << r.writeToJsonString();
        return os;
    }
    inline istream& operator>>(istream& is,Condition&l)
    {
        std::istreambuf_iterator<char> eos;
        std::string s(std::istreambuf_iterator<char>(is), eos);
        l.readFromJsonString(s);
        return is;
    }

    struct MonitorQueryReq : public kant::KantStructBase
    {
    public:
        static string className()
        {
            return "kant.MonitorQueryReq";
        }
        static string MD5()
        {
            return "343666346fe51e654ddb0a1e3256afee";
        }
        MonitorQueryReq()
        {
            resetDefautlt();
        }
        void resetDefautlt()
        {
            uid = "";
            method = "query";
            dataid = "";
            date = "";
            tflag1 = "";
            tflag2 = "";
            conditions.clear();
            indexs.clear();
            groupby.clear();
        }
        template<typename WriterT>
        void writeTo(kant::KantOutputStream<WriterT>& _os) const
        {
            _os.write(uid, 0);
            _os.write(method, 1);
            _os.write(dataid, 2);
            _os.write(date, 3);
            _os.write(tflag1, 4);
            _os.write(tflag2, 5);
            _os.write(conditions, 6);
            _os.write(indexs, 7);
            _os.write(groupby, 8);
        }
        template<typename ReaderT>
        void readFrom(kant::KantInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(uid, 0, true);
            _is.read(method, 1, true);
            _is.read(dataid, 2, true);
            _is.read(date, 3, true);
            _is.read(tflag1, 4, true);
            _is.read(tflag2, 5, true);
            _is.read(conditions, 6, true);
            _is.read(indexs, 7, true);
            _is.read(groupby, 8, true);
        }
        kant::JsonValueObjPtr writeToJson() const
        {
            kant::JsonValueObjPtr p = std::make_shared<kant::JsonValueObj>();
            p->value["uid"] = kant::JsonOutput::writeJson(uid);
            p->value["method"] = kant::JsonOutput::writeJson(method);
            p->value["dataid"] = kant::JsonOutput::writeJson(dataid);
            p->value["date"] = kant::JsonOutput::writeJson(date);
            p->value["tflag1"] = kant::JsonOutput::writeJson(tflag1);
            p->value["tflag2"] = kant::JsonOutput::writeJson(tflag2);
            p->value["conditions"] = kant::JsonOutput::writeJson(conditions);
            p->value["indexs"] = kant::JsonOutput::writeJson(indexs);
            p->value["groupby"] = kant::JsonOutput::writeJson(groupby);
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
            kant::JsonInput::readJson(uid,pObj->value["uid"], true);
            kant::JsonInput::readJson(method,pObj->value["method"], true);
            kant::JsonInput::readJson(dataid,pObj->value["dataid"], true);
            kant::JsonInput::readJson(date,pObj->value["date"], true);
            kant::JsonInput::readJson(tflag1,pObj->value["tflag1"], true);
            kant::JsonInput::readJson(tflag2,pObj->value["tflag2"], true);
            kant::JsonInput::readJson(conditions,pObj->value["conditions"], true);
            kant::JsonInput::readJson(indexs,pObj->value["indexs"], true);
            kant::JsonInput::readJson(groupby,pObj->value["groupby"], true);
        }
        void readFromJsonString(const string & str)
        {
            readFromJson(kant::KT_Json::getValue(str));
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.display(uid,"uid");
            _ds.display(method,"method");
            _ds.display(dataid,"dataid");
            _ds.display(date,"date");
            _ds.display(tflag1,"tflag1");
            _ds.display(tflag2,"tflag2");
            _ds.display(conditions,"conditions");
            _ds.display(indexs,"indexs");
            _ds.display(groupby,"groupby");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.displaySimple(uid, true);
            _ds.displaySimple(method, true);
            _ds.displaySimple(dataid, true);
            _ds.displaySimple(date, true);
            _ds.displaySimple(tflag1, true);
            _ds.displaySimple(tflag2, true);
            _ds.displaySimple(conditions, true);
            _ds.displaySimple(indexs, true);
            _ds.displaySimple(groupby, false);
            return _os;
        }
    public:
        std::string uid;
        std::string method;
        std::string dataid;
        std::string date;
        std::string tflag1;
        std::string tflag2;
        vector<kant::Condition> conditions;
        vector<std::string> indexs;
        vector<std::string> groupby;
    };
    inline bool operator==(const MonitorQueryReq&l, const MonitorQueryReq&r)
    {
        return l.uid == r.uid && l.method == r.method && l.dataid == r.dataid && l.date == r.date && l.tflag1 == r.tflag1 && l.tflag2 == r.tflag2 && l.conditions == r.conditions && l.indexs == r.indexs && l.groupby == r.groupby;
    }
    inline bool operator!=(const MonitorQueryReq&l, const MonitorQueryReq&r)
    {
        return !(l == r);
    }
    inline ostream& operator<<(ostream & os,const MonitorQueryReq&r)
    {
        os << r.writeToJsonString();
        return os;
    }
    inline istream& operator>>(istream& is,MonitorQueryReq&l)
    {
        std::istreambuf_iterator<char> eos;
        std::string s(std::istreambuf_iterator<char>(is), eos);
        l.readFromJsonString(s);
        return is;
    }

    struct MonitorQueryRsp : public kant::KantStructBase
    {
    public:
        static string className()
        {
            return "kant.MonitorQueryRsp";
        }
        static string MD5()
        {
            return "18c701841e21d42906a222a66e48ce02";
        }
        MonitorQueryRsp()
        {
            resetDefautlt();
        }
        void resetDefautlt()
        {
            ret = 0;
            msg = "";
            lastTime = "";
            activeDb = 0;
            totalDb = 0;
            retThreads.clear();
            result.clear();
        }
        template<typename WriterT>
        void writeTo(kant::KantOutputStream<WriterT>& _os) const
        {
            _os.write(ret, 0);
            if (msg != "")
            {
                _os.write(msg, 1);
            }
            _os.write(lastTime, 2);
            _os.write(activeDb, 3);
            _os.write(totalDb, 4);
            _os.write(retThreads, 5);
            if (result.size() > 0)
            {
                _os.write(result, 6);
            }
        }
        template<typename ReaderT>
        void readFrom(kant::KantInputStream<ReaderT>& _is)
        {
            resetDefautlt();
            _is.read(ret, 0, true);
            _is.read(msg, 1, false);
            _is.read(lastTime, 2, true);
            _is.read(activeDb, 3, true);
            _is.read(totalDb, 4, true);
            _is.read(retThreads, 5, true);
            _is.read(result, 6, false);
        }
        kant::JsonValueObjPtr writeToJson() const
        {
            kant::JsonValueObjPtr p = std::make_shared<kant::JsonValueObj>();
            p->value["ret"] = kant::JsonOutput::writeJson(ret);
            p->value["msg"] = kant::JsonOutput::writeJson(msg);
            p->value["lastTime"] = kant::JsonOutput::writeJson(lastTime);
            p->value["activeDb"] = kant::JsonOutput::writeJson(activeDb);
            p->value["totalDb"] = kant::JsonOutput::writeJson(totalDb);
            p->value["retThreads"] = kant::JsonOutput::writeJson(retThreads);
            p->value["result"] = kant::JsonOutput::writeJson(result);
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
            kant::JsonInput::readJson(ret,pObj->value["ret"], true);
            kant::JsonInput::readJson(msg,pObj->value["msg"], false);
            kant::JsonInput::readJson(lastTime,pObj->value["lastTime"], true);
            kant::JsonInput::readJson(activeDb,pObj->value["activeDb"], true);
            kant::JsonInput::readJson(totalDb,pObj->value["totalDb"], true);
            kant::JsonInput::readJson(retThreads,pObj->value["retThreads"], true);
            kant::JsonInput::readJson(result,pObj->value["result"], false);
        }
        void readFromJsonString(const string & str)
        {
            readFromJson(kant::KT_Json::getValue(str));
        }
        ostream& display(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.display(ret,"ret");
            _ds.display(msg,"msg");
            _ds.display(lastTime,"lastTime");
            _ds.display(activeDb,"activeDb");
            _ds.display(totalDb,"totalDb");
            _ds.display(retThreads,"retThreads");
            _ds.display(result,"result");
            return _os;
        }
        ostream& displaySimple(ostream& _os, int _level=0) const
        {
            kant::KantDisplayer _ds(_os, _level);
            _ds.displaySimple(ret, true);
            _ds.displaySimple(msg, true);
            _ds.displaySimple(lastTime, true);
            _ds.displaySimple(activeDb, true);
            _ds.displaySimple(totalDb, true);
            _ds.displaySimple(retThreads, true);
            _ds.displaySimple(result, false);
            return _os;
        }
    public:
        kant::Int32 ret;
        std::string msg;
        std::string lastTime;
        kant::Int32 activeDb;
        kant::Int32 totalDb;
        vector<kant::Int32> retThreads;
        map<std::string, vector<kant::Double> > result;
    };
    inline bool operator==(const MonitorQueryRsp&l, const MonitorQueryRsp&r)
    {
        return l.ret == r.ret && l.msg == r.msg && l.lastTime == r.lastTime && l.activeDb == r.activeDb && l.totalDb == r.totalDb && l.retThreads == r.retThreads && l.result == r.result;
    }
    inline bool operator!=(const MonitorQueryRsp&l, const MonitorQueryRsp&r)
    {
        return !(l == r);
    }
    inline ostream& operator<<(ostream & os,const MonitorQueryRsp&r)
    {
        os << r.writeToJsonString();
        return os;
    }
    inline istream& operator>>(istream& is,MonitorQueryRsp&l)
    {
        std::istreambuf_iterator<char> eos;
        std::string s(std::istreambuf_iterator<char>(is), eos);
        l.readFromJsonString(s);
        return is;
    }


    /* callback of async proxy for client */
    class MonitorQueryPrxCallback: public kant::ServantProxyCallback
    {
    public:
        virtual ~MonitorQueryPrxCallback(){}
        virtual void callback_query(kant::Int32 ret,  const kant::MonitorQueryRsp& rsp)
        { throw std::runtime_error("callback_query() override incorrect."); }
        virtual void callback_query_exception(kant::Int32 ret)
        { throw std::runtime_error("callback_query_exception() override incorrect."); }

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
            static ::std::string __MonitorQuery_all[]=
            {
                "query"
            };
            pair<string*, string*> r = equal_range(__MonitorQuery_all, __MonitorQuery_all+1, string(msg->request.sFuncName));
            if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;
            switch(r.first - __MonitorQuery_all)
            {
                case 0:
                {
                    if (msg->response->iRet != kant::KANTSERVERSUCCESS)
                    {
                        callback_query_exception(msg->response->iRet);

                        return msg->response->iRet;
                    }
                    kant::KantInputStream<kant::BufferReader> _is;

                    _is.setBuffer(msg->response->sBuffer);
                    kant::Int32 _ret;
                    _is.read(_ret, 0, true);

                    kant::MonitorQueryRsp rsp;
                    _is.read(rsp, 2, true);
                    CallbackThreadData * pCbtd = CallbackThreadData::getData();
                    assert(pCbtd != NULL);

                    pCbtd->setResponseContext(msg->response->context);

                    callback_query(_ret, rsp);

                    pCbtd->delResponseContext();

                    return kant::KANTSERVERSUCCESS;

                }
            }
            return kant::KANTSERVERNOFUNCERR;
        }

    };
    typedef std::shared_ptr<MonitorQueryPrxCallback> MonitorQueryPrxCallbackPtr;

    //callback of promise async proxy for client
    class MonitorQueryPrxCallbackPromise: public kant::ServantProxyCallback
    {
    public:
        virtual ~MonitorQueryPrxCallbackPromise(){}
    public:
        struct Promisequery
        {
        public:
            kant::Int32 _ret;
            kant::MonitorQueryRsp rsp;
            map<std::string, std::string> _mRspContext;
        };
        
        typedef std::shared_ptr< MonitorQueryPrxCallbackPromise::Promisequery > PromisequeryPtr;

        MonitorQueryPrxCallbackPromise(const kant::Promise< MonitorQueryPrxCallbackPromise::PromisequeryPtr > &promise)
        : _promise_query(promise)
        {}
        
        virtual void callback_query(const MonitorQueryPrxCallbackPromise::PromisequeryPtr &ptr)
        {
            _promise_query.setValue(ptr);
        }
        virtual void callback_query_exception(kant::Int32 ret)
        {
            std::string str("");
            str += "Function:query_exception|Ret:";
            str += KT_Common::tostr(ret);
            _promise_query.setException(kant::copyException(str, ret));
        }

    protected:
        kant::Promise< MonitorQueryPrxCallbackPromise::PromisequeryPtr > _promise_query;

    public:
        virtual int onDispatch(kant::ReqMessagePtr msg)
        {
            static ::std::string __MonitorQuery_all[]=
            {
                "query"
            };

            pair<string*, string*> r = equal_range(__MonitorQuery_all, __MonitorQuery_all+1, string(msg->request.sFuncName));
            if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;
            switch(r.first - __MonitorQuery_all)
            {
                case 0:
                {
                    if (msg->response->iRet != kant::KANTSERVERSUCCESS)
                    {
                        callback_query_exception(msg->response->iRet);

                        return msg->response->iRet;
                    }
                    kant::KantInputStream<kant::BufferReader> _is;

                    _is.setBuffer(msg->response->sBuffer);

                    MonitorQueryPrxCallbackPromise::PromisequeryPtr ptr = std::make_shared<MonitorQueryPrxCallbackPromise::Promisequery>();

                    try
                    {
                        _is.read(ptr->_ret, 0, true);

                        _is.read(ptr->rsp, 2, true);
                    }
                    catch(std::exception &ex)
                    {
                        callback_query_exception(kant::KANTCLIENTDECODEERR);

                        return kant::KANTCLIENTDECODEERR;
                    }
                    catch(...)
                    {
                        callback_query_exception(kant::KANTCLIENTDECODEERR);

                        return kant::KANTCLIENTDECODEERR;
                    }

                    ptr->_mRspContext = msg->response->context;

                    callback_query(ptr);

                    return kant::KANTSERVERSUCCESS;

                }
            }
            return kant::KANTSERVERNOFUNCERR;
        }

    };
    typedef std::shared_ptr<MonitorQueryPrxCallbackPromise> MonitorQueryPrxCallbackPromisePtr;

    /* callback of coroutine async proxy for client */
    class MonitorQueryCoroPrxCallback: public MonitorQueryPrxCallback
    {
    public:
        virtual ~MonitorQueryCoroPrxCallback(){}
    public:
        virtual const map<std::string, std::string> & getResponseContext() const { return _mRspContext; }

        virtual void setResponseContext(const map<std::string, std::string> &mContext) { _mRspContext = mContext; }

    public:
        int onDispatch(kant::ReqMessagePtr msg)
        {
            static ::std::string __MonitorQuery_all[]=
            {
                "query"
            };

            pair<string*, string*> r = equal_range(__MonitorQuery_all, __MonitorQuery_all+1, string(msg->request.sFuncName));
            if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;
            switch(r.first - __MonitorQuery_all)
            {
                case 0:
                {
                    if (msg->response->iRet != kant::KANTSERVERSUCCESS)
                    {
                        callback_query_exception(msg->response->iRet);

                        return msg->response->iRet;
                    }
                    kant::KantInputStream<kant::BufferReader> _is;

                    _is.setBuffer(msg->response->sBuffer);
                    try
                    {
                        kant::Int32 _ret;
                        _is.read(_ret, 0, true);

                        kant::MonitorQueryRsp rsp;
                        _is.read(rsp, 2, true);
                        setResponseContext(msg->response->context);

                        callback_query(_ret, rsp);

                    }
                    catch(std::exception &ex)
                    {
                        callback_query_exception(kant::KANTCLIENTDECODEERR);

                        return kant::KANTCLIENTDECODEERR;
                    }
                    catch(...)
                    {
                        callback_query_exception(kant::KANTCLIENTDECODEERR);

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
    typedef std::shared_ptr<MonitorQueryCoroPrxCallback> MonitorQueryCoroPrxCallbackPtr;

    /* proxy for client */
    class MonitorQueryProxy : public kant::ServantProxy
    {
    public:
        typedef map<string, string> KANT_CONTEXT;
        MonitorQueryProxy(Communicator* pCommunicator, const string& name, const string& setName)
                : ServantProxy(pCommunicator, name, setName) {}

        kant::Int32 query(const kant::MonitorQueryReq & req,kant::MonitorQueryRsp &rsp,const map<string, string> &context = KANT_CONTEXT(),map<string, string> * pResponseContext = NULL)
        {
            kant::KantOutputStream<kant::BufferWriterVector> _os;
            _os.write(req, 1);
            _os.write(rsp, 2);
            std::map<string, string> _mStatus;
            shared_ptr<kant::ResponsePacket> rep = kant_invoke(kant::KANTNORMAL,"query", _os, context, _mStatus);
            if(pResponseContext)
            {
                pResponseContext->swap(rep->context);
            }

            kant::KantInputStream<kant::BufferReader> _is;
            _is.setBuffer(rep->sBuffer);
            kant::Int32 _ret;
            _is.read(_ret, 0, true);
            _is.read(rsp, 2, true);
            return _ret;
        }

        void async_query(MonitorQueryPrxCallbackPtr callback,const kant::MonitorQueryReq &req,const map<string, string>& context = KANT_CONTEXT())
        {
            kant::KantOutputStream<kant::BufferWriterVector> _os;
            _os.write(req, 1);
            std::map<string, string> _mStatus;
            kant_invoke_async(kant::KANTNORMAL,"query", _os, context, _mStatus, callback);
        }
        
        kant::Future< MonitorQueryPrxCallbackPromise::PromisequeryPtr > promise_async_query(const kant::MonitorQueryReq &req,const map<string, string>& context)
        {
            kant::Promise< MonitorQueryPrxCallbackPromise::PromisequeryPtr > promise;
            MonitorQueryPrxCallbackPromisePtr callback = std::make_shared<MonitorQueryPrxCallbackPromise>(promise);

            kant::KantOutputStream<kant::BufferWriterVector> _os;
            _os.write(req, 1);
            std::map<string, string> _mStatus;
            kant_invoke_async(kant::KANTNORMAL,"query", _os, context, _mStatus, callback);

            return promise.getFuture();
        }

        void coro_query(MonitorQueryCoroPrxCallbackPtr callback,const kant::MonitorQueryReq &req,const map<string, string>& context = KANT_CONTEXT())
        {
            kant::KantOutputStream<kant::BufferWriterVector> _os;
            _os.write(req, 1);
            std::map<string, string> _mStatus;
            kant_invoke_async(kant::KANTNORMAL,"query", _os, context, _mStatus, callback, true);
        }

        MonitorQueryProxy* kant_hash(int64_t key)
        {
            return (MonitorQueryProxy*)ServantProxy::kant_hash(key);
        }

        MonitorQueryProxy* kant_consistent_hash(int64_t key)
        {
            return (MonitorQueryProxy*)ServantProxy::kant_consistent_hash(key);
        }

        MonitorQueryProxy* kant_open_trace(bool traceParam = false)
        {
            return (MonitorQueryProxy*)ServantProxy::kant_open_trace(traceParam);
        }

        MonitorQueryProxy* kant_set_timeout(int msecond)
        {
            return (MonitorQueryProxy*)ServantProxy::kant_set_timeout(msecond);
        }

        static const char* kant_prxname() { return "MonitorQueryProxy"; }
    };
    typedef std::shared_ptr<MonitorQueryProxy> MonitorQueryPrx;

    /* servant for server */
    class MonitorQuery : public kant::Servant
    {
    public:
        virtual ~MonitorQuery(){}
        virtual kant::Int32 query(const kant::MonitorQueryReq & req,kant::MonitorQueryRsp &rsp,kant::KantCurrentPtr current) = 0;
        static void async_response_query(kant::KantCurrentPtr current, kant::Int32 _ret, const kant::MonitorQueryRsp &rsp)
        {
            if (current->getRequestVersion() == TUPVERSION )
            {
                UniAttribute<kant::BufferWriterVector, kant::BufferReader>  kantAttr;
                kantAttr.setVersion(current->getRequestVersion());
                kantAttr.put("", _ret);
                kantAttr.put("kant_ret", _ret);
                kantAttr.put("rsp", rsp);

                vector<char> sTupResponseBuffer;
                kantAttr.encode(sTupResponseBuffer);
                current->sendResponse(kant::KANTSERVERSUCCESS, sTupResponseBuffer);
            }
            else if (current->getRequestVersion() == JSONVERSION)
            {
                kant::JsonValueObjPtr _p = std::make_shared< kant::JsonValueObj>();
                _p->value["rsp"] = kant::JsonOutput::writeJson(rsp);
                _p->value["kant_ret"] = kant::JsonOutput::writeJson(_ret);
                vector<char> sJsonResponseBuffer;
                kant::KT_Json::writeValue(_p, sJsonResponseBuffer);
                current->sendResponse(kant::KANTSERVERSUCCESS, sJsonResponseBuffer);
            }
            else
            {
                kant::KantOutputStream<kant::BufferWriterVector> _os;
                _os.write(_ret, 0);

                _os.write(rsp, 2);

                current->sendResponse(kant::KANTSERVERSUCCESS, _os.getByteBuffer());
            }
        }

    public:
        int onDispatch(kant::KantCurrentPtr _current, vector<char> &_sResponseBuffer)
        {
            static ::std::string __kant__MonitorQuery_all[]=
            {
                "query"
            };

            pair<string*, string*> r = equal_range(__kant__MonitorQuery_all, __kant__MonitorQuery_all+1, _current->getFuncName());
            if(r.first == r.second) return kant::KANTSERVERNOFUNCERR;
            switch(r.first - __kant__MonitorQuery_all)
            {
                case 0:
                {
                    kant::KantInputStream<kant::BufferReader> _is;
                    _is.setBuffer(_current->getRequestBuffer());
                    kant::MonitorQueryReq req;
                    kant::MonitorQueryRsp rsp;
                    if (_current->getRequestVersion() == TUPVERSION)
                    {
                        UniAttribute<kant::BufferWriterVector, kant::BufferReader>  kantAttr;
                        kantAttr.setVersion(_current->getRequestVersion());
                        kantAttr.decode(_current->getRequestBuffer());
                        kantAttr.get("req", req);
                        kantAttr.getByDefault("rsp", rsp, rsp);
                    }
                    else if (_current->getRequestVersion() == JSONVERSION)
                    {
                        kant::JsonValueObjPtr _jsonPtr = std::dynamic_pointer_cast<kant::JsonValueObj>(kant::KT_Json::getValue(_current->getRequestBuffer()));
                        kant::JsonInput::readJson(req, _jsonPtr->value["req"], true);
                        kant::JsonInput::readJson(rsp, _jsonPtr->value["rsp"], false);
                    }
                    else
                    {
                        _is.read(req, 1, true);
                        _is.read(rsp, 2, false);
                    }
                    kant::Int32 _ret = query(req,rsp, _current);
                    if(_current->isResponse())
                    {
                        if (_current->getRequestVersion() == TUPVERSION)
                        {
                            UniAttribute<kant::BufferWriterVector, kant::BufferReader>  kantAttr;
                            kantAttr.setVersion(_current->getRequestVersion());
                            kantAttr.put("", _ret);
                            kantAttr.put("kant_ret", _ret);
                            kantAttr.put("rsp", rsp);
                            kantAttr.encode(_sResponseBuffer);
                        }
                        else if (_current->getRequestVersion() == JSONVERSION)
                        {
                            kant::JsonValueObjPtr _p = std::make_shared< kant::JsonValueObj>();
                            _p->value["rsp"] = kant::JsonOutput::writeJson(rsp);
                            _p->value["kant_ret"] = kant::JsonOutput::writeJson(_ret);
                            kant::KT_Json::writeValue(_p, _sResponseBuffer);
                        }
                        else
                        {
                            kant::KantOutputStream<kant::BufferWriterVector> _os;
                            _os.write(_ret, 0);
                            _os.write(rsp, 2);
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
