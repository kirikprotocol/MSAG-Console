#ident "$Id$"
#ifndef __SMSC_INMAN_COMMON_ERRORS_HPP__
#define __SMSC_INMAN_COMMON_ERRORS_HPP__

#include <string>
#include <string.h>
#include <errno.h>
#include <stdexcept>

#include "inman/common/util.hpp"

using smsc::inman::common::format;

namespace smsc {
namespace inman {
namespace common {

class CustomException : public std::exception
{
protected:
    int         errCode;
    std::string message;
    std::string desc;
    std::string exId;

public:
    CustomException(const char * msg, int err_code = 0, const char * err_desc = NULL)
        : errCode(err_code)
    {
        if (msg)
            message += msg;
        if (err_desc) {
            desc += err_desc;
            message += ": ";
            message += err_desc;
        }
        if (errCode)
            format(message, "(code: %u)", errCode);
    }
    ~CustomException() throw()
    {}

    void setExcId(const char * ids) { exId += ids; }
    const char * getExcId(void) const { return exId.c_str(); }

    const char* what() const throw() { return message.c_str(); }

    int getErrorCode(void) const { return errCode; }
    const char* getErrorDesc(void) const  { return desc.c_str(); }
};


//system errors based on 'errno'
class SystemError : public CustomException
{
public:
    SystemError(const char * msg, int err_code = errno)
        : CustomException(msg, err_code, err_code ? strerror(err_code) : NULL)
    {
        setExcId("SystemError");
    }

    SystemError(std::string & msg, int err_code = errno)
        : CustomException(msg.c_str(), err_code, err_code ? strerror(err_code) : NULL)
    {
        setExcId("SystemError");
    }
};

} //common
} //inman
} //smsc

#endif /* __SMSC_INMAN_COMMON_ERRORS_HPP__ */

