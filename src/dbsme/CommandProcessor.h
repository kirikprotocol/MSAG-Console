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
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

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
    using smsc::util::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    class DataProvider
    {
    protected:

        DataSource*     ds;
        Hash<Job *>     jobs; // by job name
    
    public:
        
        DataProvider(ConfigView* config)
            throw(ConfigException); 
        virtual ~DataProvider();

        virtual void process(Command& command)
            throw(ServiceNotFoundException, CommandProcessException);
    };
    
    class CommandProcessor
    {
    private:

        Hash<DataProvider *>    providers;  // by provider address
        
    public:

        CommandProcessor(ConfigView* config)
            throw(ConfigException);
        virtual ~CommandProcessor();

        void process(Command& command)
            throw(ServiceNotFoundException, CommandProcessException);
    };

}}

#endif


