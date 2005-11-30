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

    using smsc::logger::Logger;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    static const char* PROVIDER_NOT_FOUND  = "PROVIDER_NOT_FOUND";
    static const char* SERVICE_NOT_AVAIL   = "SERVICE_NOT_AVAIL";
    static const char* JOB_NOT_FOUND       = "JOB_NOT_FOUND";

    class DataProvider;
    class ProviderGuard;
    class CommandProcessor
    {
    friend class DataProvider;
    private:

        smsc::logger::Logger    *log;
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

        void setProviderEnabled(std::string providerId, bool enabled);
    };

    class DataProvider
    {
    friend class ProviderGuard;
    protected:

        smsc::logger::Logger    *log;
        MessageSet              messages;
        std::string             id;

        Event       usersCountEvent;
        Mutex       usersCountLock;
        long        usersCount[2];

        Mutex       finalizingLock, enabledLock, setEnabledLock;
        bool        bFinalizing, bEnabled;

        CommandProcessor*       owner;
        DataSource*             ds;

        Mutex                   jobsLock;
        Hash<Job *>             allJobs;        // jobs by id
        Hash<Job *>             jobsByAddress;  // jobs by address
        Hash<Job *>             jobsByName;     // jobs by sybolic & digital name

        inline long* getUserCounter() {
            MutexGuard guard(enabledLock);
            return bEnabled ? &usersCount[0]:&usersCount[1];
        };

        void doWait(bool total=true)
        {
            while (true) {
                usersCountEvent.Wait(10);
                MutexGuard guard(usersCountLock);
                if (total && usersCount[0] <= 0 && usersCount[1] <= 0) break;
                else if (!total && usersCount[0] <= 0) break;
            }
        };

        void createDataSource(ConfigView* config)
            throw(ConfigException);
        void registerJob(Job* job, const char* id, const char* address,
                         const char* alias, const char* name)
            throw(ConfigException);

        JobGuard getJob(const Address& address);
        JobGuard getJob(const char* name);

        virtual ~DataProvider();

    public:

        DataProvider(CommandProcessor* root, ConfigView* config,
                     const char* providerId, const MessageSet& set)
            throw(ConfigException);

        virtual void finalize()
        {
            MutexGuard guard(setEnabledLock);
            {
                MutexGuard guard2(finalizingLock);
                if (bFinalizing) return;
                bFinalizing = true;
            }
            doWait(true);
            delete this;
        }
        inline bool isFinalizing() {
            MutexGuard guard(finalizingLock);
            return bFinalizing;
        }
        virtual void process(Command& command)
            throw(CommandProcessException);

        void createJob(const char* jobId, ConfigView* jobConfig);
        void changeJob(const char* jobId, ConfigView* jobConfig);
        void removeJob(const char* jobId);

        void setEnabled(bool enabled)
        {
            MutexGuard guard(setEnabledLock);
            {
                MutexGuard guard2(enabledLock);
                if (bEnabled == enabled) return;
                bEnabled = enabled;
                if (bEnabled) return;
            }
            doWait(false);
            if (ds) ds->closeConnections();
        }
        inline bool isEnabled() {
            MutexGuard guard(enabledLock);
            return bEnabled;
        }
    };

    class ProviderGuard
    {
    private:

        ProviderGuard& operator=(const ProviderGuard& pg) {
            return *this;
        };

    protected:

        DataProvider* provider;
        long*         counter;

    public:

        ProviderGuard(DataProvider* _provider=0) : provider(_provider), counter(0) {
            if (!provider) return;
            MutexGuard guard(provider->usersCountLock);
            counter = provider->getUserCounter();
            if (counter) (*counter)++;
        }
        ProviderGuard(const ProviderGuard& pg) : provider(pg.provider), counter(0) {
            if (!provider) return;
            MutexGuard guard(provider->usersCountLock);
            counter = provider->getUserCounter();
            if (counter) (*counter)++;
        }
        virtual ~ProviderGuard() {
            if (!provider || !counter) return;
            MutexGuard guard(provider->usersCountLock);
            (*counter)--; provider->usersCountEvent.Signal();
        }

        inline DataProvider* get() {
            return provider;
        }
    };


}}

#endif
