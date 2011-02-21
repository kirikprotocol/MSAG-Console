#ifndef __UTIL_EXCEPTION_HPP__
#define __UTIL_EXCEPTION_HPP__

#include <exception>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include "util/vformat.hpp"

#define SMSC_UTIL_EX_FILL(fmt) \
    va_list arglist;\
    va_start(arglist,fmt);\
    fill(fmt,arglist);\
    va_end(arglist);


namespace smsc {
namespace util {

class Exception: public std::exception {
public:
    Exception() { }
    Exception(const char* const fmt,...)
    {
        SMSC_UTIL_EX_FILL(fmt);
    }

    inline void fill(const char* const &fmt, va_list &arglist)
    {
        smsc::util::vformat(message, fmt, arglist);
    }

    virtual ~Exception() throw() { }
    virtual const char* what() const throw() { return message.c_str(); }
protected:
    std::string message;
};

class CustomException : public Exception {
protected:
    int32_t     errCode;
    std::string exId;

public:
    CustomException() : Exception(), errCode(0), exId("")
    { }
    CustomException(int32_t err_code, const char * msg, const char * err_desc = NULL)
        : Exception(), errCode(err_code), exId("CustomException")
    {
        if (msg)
            message += msg;
        if (errCode)
            format(message, "%scode %d", msg ? ", " : "", errCode);
        if (err_desc) {
            if (!message.empty())
                message += ": ";
            message += err_desc;
        }
    }
    CustomException(const char * fmt, ...)
        : Exception(), errCode(-1), exId("CustomException")
    {
        SMSC_UTIL_EX_FILL(fmt);
    }
    CustomException(const CustomException & org_exc)
        : Exception(), errCode(org_exc.errCode), exId(org_exc.exId)
    {
        message = org_exc.message;
    }

    ~CustomException() throw()
    {}

    int32_t errorCode(void) const { return errCode; }
    const char * excId(void) const { return exId.c_str(); }
    void setExcId(const char * ids) { exId = ids ? ids : ""; }
};

//system errors based on 'errno'
class SystemError : public CustomException {
public:
    SystemError(const char * msg, int err_code = errno)
        : CustomException(err_code, msg, err_code ? strerror(err_code) : NULL)
    { setExcId("SystemError"); }

    SystemError(std::string & msg, int err_code = errno)
        : CustomException(err_code, msg.c_str(), err_code ? strerror(err_code) : NULL)
    { setExcId("SystemError"); }
};


}//util
}//smsc

#endif /* __UTIL_EXCEPTION_HPP__ */

