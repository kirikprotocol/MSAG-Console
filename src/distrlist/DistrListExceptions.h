#ifndef SMSC_DISTRIBUTION_LIST_EXCEPTIONS
#define SMSC_DISTRIBUTION_LIST_EXCEPTIONS

#include <util/Exception.hpp>

namespace smsc { namespace distrlist
{
    using smsc::util::Exception;

    class InitException : public Exception
    {
    public:

        InitException() : Exception() {};
        InitException(const char* fmt,...) 
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        InitException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~InitException() throw() {};
    };

    class IllegalSubmitterException : public Exception
    {
    public:

        IllegalSubmitterException() : Exception() {};
        IllegalSubmitterException(const char* fmt,...) 
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        IllegalSubmitterException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~IllegalSubmitterException() throw() {};
    };

}}

#endif // SMSC_DISTRIBUTION_LIST_EXCEPTIONS


