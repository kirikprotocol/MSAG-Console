#ifndef SMSC_DBSME_EXCEPTIONS
#define SMSC_DBSME_EXCEPTIONS

#include <util/Exception.hpp>

namespace smsc { namespace db
{
    using smsc::util::Exception;

    class ServiceNotFoundException : public Exception
    {
    public:

        ServiceNotFoundException(const Exception& exc)
            : Exception(exc.what()) {};
        ServiceNotFoundException(const char* name) 
            : Exception("Service for '%s' key not found !", name) {};
        virtual ~ServiceNotFoundException() throw() {};
    };
    
    class CommandProcessException : public Exception
    {
    public:

        CommandProcessException() 
            : Exception("Command processing failed !") {};
        CommandProcessException(const char* fmt,...)
            : Exception() 
        {
            SMSC_UTIL_EX_FILL(fmt);
        };
        CommandProcessException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~CommandProcessException() throw() {};
    };

}}

#endif
