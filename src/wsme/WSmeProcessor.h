#ifndef SMSC_WSME_PROCESSOR
#define SMSC_WSME_PROCESSOR

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
#include <util/Logger.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>

#include <util/config/Config.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <sms/sms.h> // ???
#include <db/DataSource.h>

#include <util/Exception.hpp>

namespace smsc { namespace wsme
{
    using namespace smsc::db;
    using namespace smsc::sms; // ???
    using namespace smsc::core::buffers;
                              
    using smsc::util::Logger;
    using smsc::util::Exception;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    class ProcessException : public Exception
    {
    public:

        ProcessException() 
            : Exception("Processing failed !") {};
        ProcessException(const char* fmt,...)
            : Exception() 
        {
            SMSC_UTIL_EX_FILL(fmt);
        };
        ProcessException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~ProcessException() throw() {};
    };
    
    class WSmeProcessor
    {
    private:
        
        log4cpp::Category       &log;

    public:

        WSmeProcessor(ConfigView* config) 
            : log(Logger::getCategory("smsc.wsme.WSmeProcessor")) {};
        ~WSmeProcessor() {};
        
        virtual void processNotification(std::string& in, std::string& out)
            throw (ProcessException) {};
        virtual void processReceipt(std::string& in)
            throw (ProcessException) {};
    };

}}

#endif // SMSC_WSME_PROCESSOR


