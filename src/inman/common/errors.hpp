#ifndef __SMSC_INMAN_COMMON_ERRORS_HPP__
#define __SMSC_INMAN_COMMON_ERRORS_HPP__

#include <string>
#include <string.h>
#include <errno.h>
#include <stdexcept>

#include "inman/common/util.hpp"

namespace smsc {
namespace inman {
namespace common {

class CustomError : public std::exception
{
    protected:
        std::string message;

    public:	
        CustomError() throw()
        { }
        CustomError(const std::string& msg) throw()
            : message( msg )
        { }

        ~CustomError() throw()
        { }

        const char* what() const throw()
        {
            return message.c_str();
        }
};

class SystemError : public CustomError
{
    protected:
        int errorCode;

    public:
        SystemError(const char * msg) throw()
            : errorCode( errno )
        {
            format(message, "%s : %s (0x%02X)", msg, getErrorString(), errorCode);
        }

        SystemError(std::string & msg) throw()
            : errorCode( errno )
        {
            format(message, "%s : %s (0x%02X)", msg.c_str(), getErrorString(), errorCode);
        }

        void Init(const char * msg)
        {
            format(message, "%s : %s (0x%02X)", msg, getErrorString(), errorCode = errno);
        }

        int getErrorCode() const
        {
	    return errorCode;
        }

        const char* getErrorString() const
        {
            return strerror(errorCode);
        }
};

}
}
}

#endif
