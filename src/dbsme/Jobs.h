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
#include <string>

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

    static const char* SMSC_DBSME_DEFAULT_JOB_NAME = "default";
    
    class MessageSet
    {
    protected:
        
        Hash<std::string>   messages;

    public:

        virtual ~MessageSet() {};
        
        MessageSet(ConfigView* config = 0) 
            throw(ConfigException) 
        {
            init(config);
        };
        MessageSet(const MessageSet& set, ConfigView* config = 0)
            throw(ConfigException)
        {
            init(set, config);
        };
        
        void init(const MessageSet& set, ConfigView* config = 0)
            throw(ConfigException)
        {
            messages = set.messages;
            init(config);
        };

        void init(ConfigView* config = 0)
            throw(ConfigException) 
        {
            if (!config) return;
            
            // Load up messages as parameters here !!!

            /*
            ConfigView* messageConfig = config->getSubConfig("MessageSet");
            std::set<std::string>* set = messageConfig->getSectionNames();

            for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
            {

            }*/
        };

        const char* get(const char* key) 
        {
            return (messages.Exists(key)) ? messages.Get(key).c_str() : 0;
        };
        void set(const char* key, const char* message)
        {
            if (messages.Exists(key)) messages.Delete(key);
            messages.Insert(key, std::string(message));
        };
    };

    class Job
    {
    protected:

        log4cpp::Category   &log;

        MessageSet  messages;
        char*       name;

        Job() : log(Logger::getCategory("smsc.dbsme.Job")), name(0) {};

    public:

        virtual ~Job() 
        { 
            if (name) delete name; 
        };
        
        void init(ConfigView* config, const MessageSet& set)
            throw(ConfigException) 
        {
            name = config->getString("name", "SQLJob name wasn't defined !");
            messages.init(set, config);
            init(config);
        };
        
        const char* getName() const { return name ? name:""; };
        
        virtual void error(const char* key, 
                           const char* details = 0, 
                           bool search = true)
            throw(CommandProcessException)
        {
            __require__(key);
            
            if (details) log.error("%s > %s", name, details); 
            const char* message = (search) ? messages.get(key):0;
            throw CommandProcessException(message ? message:key);
        };
        
        virtual void init(ConfigView* config)
            throw(ConfigException) = 0; 
        virtual void process(Command& command, DataSource& ds)
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


