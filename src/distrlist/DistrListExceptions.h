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

    class PrincipalAlreadyExistsException : public Exception
    {
    public:

        PrincipalAlreadyExistsException() : Exception() {};
        PrincipalAlreadyExistsException(const char* fmt,...) 
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        PrincipalAlreadyExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~PrincipalAlreadyExistsException() throw() {};
    };
    
    class ListNotExistsException : public Exception
    {
    public:

        ListNotExistsException() : Exception() {};
        ListNotExistsException(const char* fmt,...) 
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        ListNotExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~ListNotExistsException() throw() {};
    };
    
    class ListAlreadyExistsException : public Exception
    {
    public:

        ListAlreadyExistsException() : Exception() {};
        ListAlreadyExistsException(const char* fmt,...) 
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        ListAlreadyExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~ListAlreadyExistsException() throw() {};
    };
    
    class MemberNotExistsException : public Exception
    {
    public:

        MemberNotExistsException() : Exception() {};
        MemberNotExistsException(const char* fmt,...) 
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        MemberNotExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~MemberNotExistsException() throw() {};
    };


    class MemberAlreadyExistsException : public Exception
    {
    public:

        MemberAlreadyExistsException() : Exception() {};
        MemberAlreadyExistsException(const char* fmt,...) 
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        MemberAlreadyExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~MemberAlreadyExistsException() throw() {};
    };


}}

#endif // SMSC_DISTRIBUTION_LIST_EXCEPTIONS


