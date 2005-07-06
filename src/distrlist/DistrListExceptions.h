#ifndef SMSC_DISTRIBUTION_LIST_EXCEPTIONS
#define SMSC_DISTRIBUTION_LIST_EXCEPTIONS

#include <util/Exception.hpp>

namespace smsc { namespace distrlist
{
    using smsc::util::Exception;

    class InitException : public Exception
    {
    public:

        InitException() : Exception("InitException") {};
        InitException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        InitException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~InitException() throw() {};
    };

    class IllegalSubmitterException : public Exception
    {
    public:

        IllegalSubmitterException() : Exception("IllegalSubmitterException") {};
        IllegalSubmitterException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        IllegalSubmitterException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~IllegalSubmitterException() throw() {};
    };

    class SubmitterAlreadyExistsException : public Exception
    {
    public:

        SubmitterAlreadyExistsException() : Exception("SubmitterAlreadyExistsException") {};
        SubmitterAlreadyExistsException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        SubmitterAlreadyExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~SubmitterAlreadyExistsException() throw() {};
    };

    class SubmitterNotExistsException : public Exception
    {
    public:

        SubmitterNotExistsException() : Exception("SubmitterNotExistsException") {};
        SubmitterNotExistsException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        SubmitterNotExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~SubmitterNotExistsException() throw() {};
    };

    class PrincipalAlreadyExistsException : public Exception
    {
    public:

        PrincipalAlreadyExistsException() : Exception("PrincipalAlreadyExistsException") {};
        PrincipalAlreadyExistsException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        PrincipalAlreadyExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~PrincipalAlreadyExistsException() throw() {};
    };

    class PrincipalNotExistsException : public Exception
    {
    public:

        PrincipalNotExistsException() : Exception("PrincipalNotExistsException") {};
        PrincipalNotExistsException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        PrincipalNotExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~PrincipalNotExistsException() throw() {};
    };

    class PrincipalInUseException : public Exception
    {
    public:

        PrincipalInUseException() : Exception("PrincipalInUseException") {};
        PrincipalInUseException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        PrincipalInUseException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~PrincipalInUseException() throw() {};
    };

    class IllegalPrincipalException : public Exception
    {
    public:

        IllegalPrincipalException() : Exception("IllegalPrincipalException") {};
        IllegalPrincipalException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        IllegalPrincipalException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~IllegalPrincipalException() throw() {};
    };

    class ListNotExistsException : public Exception
    {
    public:

        ListNotExistsException() : Exception("ListNotExistsException") {};
        ListNotExistsException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        ListNotExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~ListNotExistsException() throw() {};
    };

    class ListAlreadyExistsException : public Exception
    {
    public:

        ListAlreadyExistsException() : Exception("ListAlreadyExistsException") {};
        ListAlreadyExistsException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        ListAlreadyExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~ListAlreadyExistsException() throw() {};
    };

    class ListCountExceededException : public Exception
    {
    public:

        ListCountExceededException() : Exception("ListCountExceededException") {};
        ListCountExceededException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        ListCountExceededException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~ListCountExceededException() throw() {};
    };

    class MemberNotExistsException : public Exception
    {
    public:

        MemberNotExistsException() : Exception("MemberNotExistsException") {};
        MemberNotExistsException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        MemberNotExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~MemberNotExistsException() throw() {};
    };


    class MemberAlreadyExistsException : public Exception
    {
    public:

        MemberAlreadyExistsException() : Exception("MemberAlreadyExistsException") {};
        MemberAlreadyExistsException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        MemberAlreadyExistsException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~MemberAlreadyExistsException() throw() {};
    };

    class MemberCountExceededException : public Exception
    {
    public:

        MemberCountExceededException() : Exception("MemberCountExceededException") {};
        MemberCountExceededException(const char* fmt,...)
            : Exception() { SMSC_UTIL_EX_FILL(fmt); };
        MemberCountExceededException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~MemberCountExceededException() throw() {};
    };

}}

#endif // SMSC_DISTRIBUTION_LIST_EXCEPTIONS
