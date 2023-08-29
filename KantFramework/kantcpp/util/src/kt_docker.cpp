
#include "util/kt_docker.h"
#include "util/kt_http.h"
#include "util/kt_http_async.h"
#include "util/kt_base64.h"
#include "util/kt_json.h"

namespace kant {

class AsyncHttpCallback : public KT_HttpAsync::RequestCallback {
 public:
  AsyncHttpCallback()  //std::mutex &m, std::condition_variable &cond ): _m(m), _cond(cond)
  {}

  virtual void onSucc(KT_HttpResponse &stHttpResponse) {
    //		LOG_CONSOLE_DEBUG << stHttpResponse.getContent() << endl;
    _succ = true;
    _rsp = stHttpResponse;
  }

  virtual void onFailed(FAILED_CODE ret, const string &info) {
    //		LOG_CONSOLE_DEBUG << ret << ":" << info << endl;

    _errMessage = info;
    std::unique_lock<std::mutex> lock(_m);
    _terminate = true;
    _cond.notify_one();
  }

  virtual void onClose() {
    //		LOG_CONSOLE_DEBUG << _rsp.getContent() << endl;

    std::unique_lock<std::mutex> lock(_m);
    _terminate = true;
    _cond.notify_one();
  }

  void wait() {
    std::unique_lock<std::mutex> lock(_m);

    if (_terminate) {
      return;
    }
    _cond.wait(lock);
  }

 public:
  std::mutex _m;
  std::condition_variable _cond;

  bool _succ = false;
  bool _terminate = false;
  string _errMessage;
  KT_HttpResponse _rsp;
};

void KT_Docker::setDockerUnixLocal(const string &unixSocket) {
  _dockerUrl = KT_Common::replace(unixSocket, FILE_SEP, "$") + ":0";
}

bool KT_Docker::createHttpRequest(KT_HttpRequest &request) {
  KT_HttpAsync ast;
  ast.setTimeout(_requestTimeout);
  ast.start();

  AsyncHttpCallback *callback = new AsyncHttpCallback();

  KT_HttpAsync::RequestCallbackPtr p(callback);

  request.setHeader("X-Registry-Auth", this->_authenticationStr);
  if (!request.hasHeader("Content-Type")) {
    request.setHeader("Content-Type", "application/tar");
  }

  request.setHost("localhost");
  //		LOG_CONSOLE_DEBUG << request.encode() << endl;

  ast.doAsyncRequest(request, p);

  callback->wait();
  ast.waitForAllDone();

  //		LOG_CONSOLE_DEBUG << callback->_rsp.encode() << endl;

  _responseMessage = KT_Common::trimright(callback->_rsp.getContent());

  if (callback->_succ && (callback->_rsp.getStatus() >= 200 && callback->_rsp.getStatus() < 300)) {
    return true;
  } else {
    if (!callback->_rsp.getContent().empty()) {
      _errMessage = KT_Common::trimright(callback->_rsp.getContent());
    } else if (!callback->_errMessage.empty()) {
      _errMessage = "{ \"message\": \"" + callback->_errMessage + "\"}";
    } else {
      _errMessage = "{ \"message\": \"" + callback->_rsp.getAbout() + "\"}";
    }

    return false;
  }
}

void KT_Docker::setAuthentication(const string &useName, const string &password, const string &serverAddress) {
  _authentication.useName = useName;
  _authentication.password = password;
  _authentication.serverAddress = serverAddress;

  JsonValueObjPtr obj = std::make_shared<JsonValueObj>();
  obj->value["username"] = std::make_shared<JsonValueString>(useName);
  obj->value["password"] = std::make_shared<JsonValueString>(password);
  obj->value["serveraddress"] = std::make_shared<JsonValueString>(serverAddress);

  _authenticationStr = KT_Base64::encode(KT_Json::writeValue(obj));
}

bool KT_Docker::login(const string &useName, const string &password, const string &serverAddress) {
  JsonValueObjPtr obj = std::make_shared<JsonValueObj>();
  obj->value["username"] = std::make_shared<JsonValueString>(useName);
  obj->value["password"] = std::make_shared<JsonValueString>(password);
  obj->value["serveraddress"] = std::make_shared<JsonValueString>(serverAddress);

  KT_HttpRequest request;

  request.setPostRequest(_dockerUrl + "/" + _dockerApiVersion + "/auth", KT_Json::writeValue(obj));

  bool flag = createHttpRequest(request);

  if (flag) {
    JsonValueObjPtr result = std::dynamic_pointer_cast<JsonValueObj>(KT_Json::getValue(getResponseMessage()));

    auto it = result->value.find("Status");
    if (it != result->value.end()) {
      JsonValueStringPtr v = std::dynamic_pointer_cast<JsonValueString>(it->second);

      if (v && v->value == "Login Succeeded") {
        return true;
      }
    } else {
      this->_errMessage = std::dynamic_pointer_cast<JsonValueString>(result->value["message"])->value;
    }
  }

  return false;
}

bool KT_Docker::pull(const string &image) {
  KT_HttpRequest request;

  request.setPostRequest(_dockerUrl + "/" + _dockerApiVersion + "/images/create?fromImage=" + image,
                         _authenticationStr);

  return createHttpRequest(request);
}

bool KT_Docker::create(const string &name, const string &image, const vector<string> &entrypoint,
                       const vector<string> &commands, const vector<string> &envs, const map<string, string> &mounts,
                       const map<string, pair<string, int>> &ports, const string &restartPolicy, int maximumRetryCount,
                       const string &networkMode, const string &ipcMode, bool autoRemove, bool privileged) {
  KT_HttpRequest request;

  JsonValueObjPtr obj = std::make_shared<JsonValueObj>();

  obj->value["Image"] = std::make_shared<JsonValueString>(image);

  if (!entrypoint.empty()) {
    JsonValueArrayPtr ePtr = std::make_shared<JsonValueArray>();
    obj->value["Entrypoint"] = ePtr;

    for (auto &e : entrypoint) {
      ePtr->value.push_back(std::make_shared<JsonValueString>(e));
    }
  }

  if (!commands.empty()) {
    JsonValueArrayPtr cmd = std::make_shared<JsonValueArray>();
    obj->value["Cmd"] = cmd;

    for (auto &c : commands) {
      cmd->value.push_back(std::make_shared<JsonValueString>(c));
    }
  }

  if (!envs.empty()) {
    JsonValueArrayPtr env = std::make_shared<JsonValueArray>();
    obj->value["Env"] = env;

    for (auto &e : envs) {
      env->value.push_back(std::make_shared<JsonValueString>(e));
    }
  }

  JsonValueObjPtr hostConfig = std::make_shared<JsonValueObj>();
  obj->value["HostConfig"] = hostConfig;

  hostConfig->value["NetworkMode"] = std::make_shared<JsonValueString>(networkMode);

  if (!ports.empty()) {
    JsonValueObjPtr espec = std::make_shared<JsonValueObj>();
    obj->value["ExposedPorts"] = espec;

    JsonValueObjPtr portBindings = std::make_shared<JsonValueObj>();
    hostConfig->value["PortBindings"] = portBindings;

    for (auto e : ports) {
      espec->value[e.first] = std::make_shared<JsonValueObj>();

      JsonValueArrayPtr hostPort = std::make_shared<JsonValueArray>();

      JsonValueObjPtr hObj = std::make_shared<JsonValueObj>();
      hObj->value["HostIp"] = std::make_shared<JsonValueString>(e.second.first);
      hObj->value["HostPort"] = std::make_shared<JsonValueString>(KT_Common::tostr(e.second.second));

      hostPort->push_back(hObj);

      portBindings->value[e.first] = hostPort;
    }
  }

  if (!mounts.empty()) {
    JsonValueArrayPtr Mounts = std::make_shared<JsonValueArray>();
    hostConfig->value["Mounts"] = Mounts;

    for (auto e : mounts) {
      JsonValueObjPtr mObj = std::make_shared<JsonValueObj>();
      mObj->value["Source"] = std::make_shared<JsonValueString>(e.first);
      mObj->value["Target"] = std::make_shared<JsonValueString>(e.second);
      mObj->value["Type"] = std::make_shared<JsonValueString>("bind");

      Mounts->push_back(mObj);
    }
  }

  if (!restartPolicy.empty()) {
    JsonValueObjPtr rObj = std::make_shared<JsonValueObj>();
    rObj->value["RestartPolicy"] = std::make_shared<JsonValueString>(restartPolicy);
    rObj->value["maximumRetryCount"] = std::make_shared<JsonValueNum>((int64_t)maximumRetryCount, true);

    hostConfig->value["RestartPolicy"] = rObj;
  }

  //	JsonValueArrayPtr shell = std::make_shared<JsonValueArray>();
  //	shell->value.push_back(std::make_shared<JsonValueString>("bash"));
  //	shell->value.push_back(std::make_shared<JsonValueString>("bash"));
  //	shell->value.push_back(std::make_shared<JsonValueString>("bash"));
  //	hostConfig->value["Shell"] = shell;

  hostConfig->value["WorkingDir"] = std::make_shared<JsonValueBoolean>("/");
  hostConfig->value["AutoRemove"] = std::make_shared<JsonValueBoolean>(autoRemove);
  hostConfig->value["Privileged"] = std::make_shared<JsonValueBoolean>(privileged);

  string json = KT_Json::writeValue(obj);

  request.setPostRequest(_dockerUrl + "/" + _dockerApiVersion + "/containers/create?name=" + name, json);

  request.setHeader("Content-Type", "application/json");

  return createHttpRequest(request);
}

bool KT_Docker::start(const string &containerId) {
  KT_HttpRequest request;

  request.setPostRequest(_dockerUrl + "/" + _dockerApiVersion + "/containers/" + containerId + "/start", "");

  return createHttpRequest(request);
}

bool KT_Docker::stop(const string &containerId, size_t waitSeconds) {
  KT_HttpRequest request;

  request.setPostRequest(
    _dockerUrl + "/" + _dockerApiVersion + "/containers/" + containerId + "/stop?t=" + KT_Common::tostr(waitSeconds),
    "");

  return createHttpRequest(request);
}

bool KT_Docker::remove(const string &containerId, bool force) {
  KT_HttpRequest request;

  request.setDeleteRequest(
    _dockerUrl + "/" + _dockerApiVersion + "/containers/" + containerId + "?force=" + string(force ? "true" : "false"),
    "");

  return createHttpRequest(request);
}

bool KT_Docker::inspectContainer(const string &containerId) {
  KT_HttpRequest request;

  request.setGetRequest(_dockerUrl + "/" + _dockerApiVersion + "/containers/" + containerId + "/json");

  return createHttpRequest(request);
}

bool KT_Docker::inspectImage(const string &imageId) {
  KT_HttpRequest request;

  request.setGetRequest(_dockerUrl + "/" + _dockerApiVersion + "/images/" + imageId + "/json");

  return createHttpRequest(request);
}

}  // namespace kant
