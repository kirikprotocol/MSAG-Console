#ifndef SMSC_MSC_MAN_EXCEPTIONS
#define SMSC_MSC_MAN_EXCEPTIONS

#include <util/Exception.hpp>

namespace smsc { namespace mscman
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

}}

#endif // SMSC_MSC_MAN_EXCEPTIONS


