#include "util/kt_ex.h"

using namespace kant;

/**
* @brief 配置文件异常类
* @brief Profile Exception Class
*/
struct KT_Cgi_Exception : public KT_Exception
{
    KT_Cgi_Exception(const string &buffer) : KT_Exception(buffer){};
    KT_Cgi_Exception(const string &buffer, int err) : KT_Exception(buffer, err){};
    ~KT_Cgi_Exception() throw(){};
};

int main(int argc, char *argv[]) {
    THROW_EXCEPTION_SYSCODE(KT_Cgi_Exception, "parseFormData error!");
    return 0;
}