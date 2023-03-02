#include "util/kt_ex.h"
#include "util/kt_platform.h"
#if TARGET_PLATFORM_LINUX
#include <execinfo.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <cerrno>
#include <iostream>

namespace kant
{

KT_Exception::KT_Exception(const string &buffer)
: _code(0), _buffer(buffer)
{
}


KT_Exception::KT_Exception(const string &buffer, int err)
{
    if(err != 0)
    {
    	_buffer = buffer + " :" + parseError(err);
    }
    else
    {
        _buffer = buffer;
    }
    _code   = err;
}

KT_Exception::~KT_Exception() throw()
{
}

const char* KT_Exception::what() const throw()
{
    return _buffer.c_str();
}

void KT_Exception::getBacktrace()
{
#if TARGET_PLATFORM_LINUX
    void * array[64];
    int nSize = backtrace(array, 64);
    char ** symbols = backtrace_symbols(array, nSize);

    for (int i = 0; i < nSize; i++)
    {
        _buffer += symbols[i];
        _buffer += "\n";
    }
	free(symbols);
#endif
}

#if TARGET_PLATFORM_WINDOWS
static std::string Unicode2ANSI(LPCWSTR lpszSrc)
{
    std::string sResult;
    if (lpszSrc != NULL) {
        int  nANSILen = WideCharToMultiByte(CP_ACP, 0, lpszSrc, -1, NULL, 0, NULL, NULL);
        char* pANSI = new char[nANSILen + 1];
        if (pANSI != NULL) {
            ZeroMemory(pANSI, nANSILen + 1);
            WideCharToMultiByte(CP_ACP, 0, lpszSrc, -1, pANSI, nANSILen, NULL, NULL);
            sResult = pANSI;
            delete[] pANSI;
        }
    }
    return sResult;
}
#endif

string KT_Exception::parseError(int err)
{
    string errMsg;

#if TARGET_PLATFORM_LINUX || TARGET_PLATFORM_IOS
    errMsg = strerror(err);
#else
    // LPTSTR lpMsgBuf;
    LPSTR lpMsgBuf;

    FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, err, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            (LPTSTR) & lpMsgBuf, 0, NULL);

    // errMsg = Unicode2ANSI((LPCWSTR)lpMsgBuf);
    if(lpMsgBuf != NULL)
    {
        errMsg = lpMsgBuf;
    }
    LocalFree(lpMsgBuf);
#endif

    return errMsg;
}

int KT_Exception::getSystemCode()
{
#if TARGET_PLATFORM_WINDOWS        
    return GetLastError();
#else
    return errno; 
#endif
}

string KT_Exception::getSystemError()
{
    return parseError(getSystemCode());
}
}
