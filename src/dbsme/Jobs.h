#ifndef SMSC_DBSME_JOBS
#define SMSC_DBSME_JOBS

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
#include <core/buffers/Hash.hpp>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSource.h>

#include "DBSmeExceptions.h"
#include "Command.h"

namespace smsc { namespace dbsme 
{
    using namespace smsc::core::buffers;
    using namespace smsc::db;
    
    using smsc::util::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    class Job
    {
    protected:

        char*   sql;
        char*   inputFormat;
        char*   outputFormat;
        
        bool    isQuery;

        Job() {};

    public:

        virtual ~Job();
        virtual void init(ConfigView* config)
            throw(ConfigException);
        
        void process(Command& command, DataSource& ds)
            throw(CommandProcessException);
        virtual void process(Command& command, Statement& stmt) 
            throw(CommandProcessException) = 0;
    };

    class JobFactory
    {
    protected:

        static Hash<JobFactory *>*   registry;
        
        JobFactory() {};
        virtual ~JobFactory() {};
        
        virtual Job* createJob() = 0;
        
    public:

        static void registerFactory(JobFactory* jf, const char* key)
        {
            if (!registry)
            {
                registry = new Hash<JobFactory *>();
            }
            registry->Insert(key, jf);
        };
        
        static Job* getJob(const char* key)
        {
            JobFactory* jf = (registry) ? 
                ((registry->Exists(key)) ? registry->Get(key):0):0;
            return ((jf) ? jf->createJob():0);
        };
    };

}}

#endif


