#ifndef SMSC_WSME_EXCEPTIONS
#define SMSC_WSME_EXCEPTIONS

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <util/debug.h>
#include <logger/Logger.h>
#include <util/Exception.hpp>

namespace smsc { namespace wsme
{
    using smsc::util::Logger;
    using smsc::util::Exception;

    class InitException : public Exception
    {
    public:

        InitException(const char* fmt,...)
            : Exception() 
        { 
            SMSC_UTIL_EX_FILL(fmt);
        };
        InitException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~InitException() throw() {};
    };
    
    class ProcessException : public Exception
    {
    public:

        ProcessException(const char* fmt,...)
            : Exception() 
        { 
            SMSC_UTIL_EX_FILL(fmt);
        };
        ProcessException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~ProcessException() throw() {};
    };

}}

#endif // SMSC_WSME_EXCEPTIONS


