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
#include <logger/Logger.h>
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
    using namespace smsc::sms;

    using namespace smsc::core::buffers;
    using namespace smsc::db;
    
    using smsc::util::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    static const char* PROVIDER_NOT_FOUND  = "PROVIDER_NOT_FOUND";
    static const char* JOB_NOT_FOUND       = "JOB_NOT_FOUND";

    class DataProvider;
    class ProviderGuard;
    class CommandProcessor
    {
    friend class DataProvider;
    private:

        log4cpp::Category       &log;
        MessageSet              messages;

        int     protocolId;
        char*   svcType;
        
        Mutex                   providersLock;
        Hash<DataProvider *>    allProviders;   // all providers by id
        Hash<DataProvider *>    idxProviders;   // by provider address
        
        ProviderGuard getProvider(const char* id);
        ProviderGuard getProvider(const Address& address);
        
        bool hasProvider(const char* id);
        bool hasProvider(const Address& address);
        
        bool addProvider(const char* id, const Address& address, DataProvider *provider);
        bool addProviderIndex(const Address& address, DataProvider *provider);
        bool delProviderIndex(const char* idx);
    
    public:

        CommandProcessor();
        CommandProcessor(ConfigView* config)
            throw(ConfigException);
        virtual ~CommandProcessor();

        void clean();
        void init(ConfigView* config)
            throw(ConfigException);
        void process(Command& command)
            throw(CommandProcessException);

        const char* getSvcType() { return (svcType) ? svcType:"DbSme"; };
        int getProtocolId() { return protocolId; };

        /* ---------------- Admin interface implementation ----------------- */

        void addJob   (std::string providerId, std::string jobId);
        void removeJob(std::string providerId, std::string jobId);
        void changeJob(std::string providerId, std::string jobId);
    };

    class DataProvider
    {
    friend class ProviderGuard;
    protected:

        log4cpp::Category       &log;
        MessageSet              messages;
        
        Event       usersCountEvent;
        Mutex       usersCountLock;
        long        usersCount;
        
        Mutex       finalizingLock;
        bool        bFinalizing;

        CommandProcessor*       owner;
        DataSource*             ds;
        
        Mutex                   jobsLock;
        Hash<Job *>             allJobs;        // jobs by id
        Hash<Job *>             jobsByAddress;  // jobs by address
        Hash<Job *>             jobsByName;     // jobs by sybolic & digital name

        void doFinalization()
        {
            {
                MutexGuard guard(finalizingLock);
                bFinalizing = true;
            }
            
            while (true) {
                usersCountEvent.Wait(10);
                MutexGuard guard(usersCountLock);
                if (usersCount <= 0) break;
            }
        }
        
        void createDataSource(ConfigView* config) 
            throw(ConfigException);
        void registerJob(Job* job, const char* id, const char* address, 
                         const char* alias, const char* name)
            throw(ConfigException);

        JobGuard getJob(const Address& address);
        JobGuard getJob(const char* name);
    
        virtual ~DataProvider();

    public:
        
        DataProvider(CommandProcessor* root, ConfigView* config, const MessageSet& set)
            throw(ConfigException);
        
        virtual void finalize() {
            doFinalization();
            delete this;
        }
        inline bool isFinalizing() {
            MutexGuard guard(finalizingLock);
            return bFinalizing;
        }

        virtual void process(Command& command)
            throw(CommandProcessException);
        
        void createJob(const char* id, ConfigView* jobConfig) 
            throw(ConfigException);
        void removeJob(const char* id);
    };

    class ProviderGuard
    {
    private:
        
        ProviderGuard& operator=(const ProviderGuard& pg) {
            changeProviderCounter(false);
            provider = pg.provider;
            changeProviderCounter(true);
            return *this;
        };

    protected:
        
        DataProvider* provider;
        
        inline void changeProviderCounter(bool increment) {
           if (!provider) return;
           MutexGuard guard(provider->usersCountLock);
           if (increment) provider->usersCount++;
           else provider->usersCount--;
           provider->usersCountEvent.Signal();
        }
        
    public:
        
        ProviderGuard(DataProvider* provider=0) : provider(provider) {
            changeProviderCounter(true);
        }
        ProviderGuard(const ProviderGuard& pg) : provider(pg.provider) {
            changeProviderCounter(true);
        }
        virtual ~ProviderGuard() {
            changeProviderCounter(false);
        }
    
        inline DataProvider* get() {
            return provider;
        }
    };


}}

#endif


