#ifndef SMSC_DBSME_EXCEPTIONS
#define SMSC_DBSME_EXCEPTIONS

#include <util/Exception.hpp>

namespace smsc { namespace db
{
    using smsc::util::Exception;

    class ServiceNotFoundException : public Exception
    {
    public:

        ServiceNotFoundException(const char* name) 
            : Exception("Service for '%s' key not found !", name) {};
        virtual ~ServiceNotFoundException() throw() {};
    };
    
    class CommandProcessException : public Exception
    {
    public:

        CommandProcessException() 
            : Exception("Command processing failed !") {};
        virtual ~CommandProcessException() throw() {};
    };

}}

#endif
