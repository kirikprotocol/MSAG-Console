#ifndef SMSC_DBSME_COMMANDPROCESSOR
#define SMSC_DBSME_COMMANDPROCESSOR

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

#include <sms/sms.h>
#include <db/DataSource.h>

#include "DBSmeExceptions.h"
#include "Command.h"
#include "Jobs.h"

namespace smsc { namespace dbsme 
{
    using namespace smsc::core::buffers;
    using namespace smsc::db;
    
    using smsc::sms::Address;
    using smsc::sms::AddressValue;

    using smsc::util::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    static const char* PROVIDER_NOT_FOUND  = "PROVIDER_NOT_FOUND";
    static const char* JOB_NOT_FOUND       = "JOB_NOT_FOUND";

    class DataProvider
    {
    protected:

        log4cpp::Category       &log;
        MessageSet              messages;

        DataSource*             ds;
        Hash<Job *>             jobs;       // by job name
    
    public:
        
        virtual ~DataProvider();

        DataProvider(ConfigView* config, const MessageSet& set)
            throw(ConfigException);
        
        virtual void process(Command& command)
            throw(CommandProcessException);
    };
    
    class CommandProcessor
    {
    private:

        log4cpp::Category       &log;
        MessageSet              messages;

        int     protocolId;
        char*   svcType;
        
        Hash<DataProvider *>    providers;  // by provider address
        
        
    public:

        CommandProcessor();
        CommandProcessor(ConfigView* config)
            throw(ConfigException);
        virtual ~CommandProcessor();

        void init(ConfigView* config)
            throw(ConfigException);
        void process(Command& command)
            throw(CommandProcessException);

        const char* getSvcType() { return (svcType) ? svcType:"DbSme"; };
        int getProtocolId() { return protocolId; };
    };

}}

#endif


