#ifndef SMSC_DBSME_EXCEPTIONS
#define SMSC_DBSME_EXCEPTIONS

#include <util/Exception.hpp>

namespace smsc { namespace db
{
    using smsc::util::Exception;

    class ServiceNotFoundException : public Exception
    {
    public:

        ServiceNotFoundException() 
            : Exception("Exception: Service Not Found !") {};
        virtual ~ServiceNotFoundException() throw() {};
    };
    
    class CommandProcessException : public Exception
    {
    public:

        CommandProcessException() 
            : Exception("Exception: Command processing failed !") {};
        virtual ~CommandProcessException() throw() {};
    };

}}

#endif
