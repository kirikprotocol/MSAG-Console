#ifndef STORE_MANAGER_DECLARATIONS
#define STORE_MANAGER_DECLARATIONS

#include <oci.h>

#include <core/synchronization/Mutex.hpp>
#include <util/config/Manager.h>
#include <util/Logger.h>

#include "StoreConfig.h"
#include "MessageStore.h"
#include "ConnectionManager.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;
    using namespace smsc::core::synchronization;
    using smsc::util::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;
    
    class IDGenerator 
    {
    private:
        
        SMSId id;
        Mutex mutex;

    public:

        IDGenerator(SMSId _id) : id(_id) {};
        ~IDGenerator() {};
        
        inline SMSId getNextId() 
        {
            MutexGuard guard(mutex);
            return ++id;
        };
    };

    class StoreManager : public MessageStore
    {
    private:
        
        static Mutex mutex;

        static IDGenerator          *generator;
        static StoreManager         *instance;
        static ConnectionPool       *pool;
        static log4cpp::Category    &log;
        
    protected:
        
        StoreManager() : MessageStore() {};
        virtual ~StoreManager() {};

    public:    
        
        static MessageStore* startup(Manager& config)
            throw(ConfigException, ConnectionFailedException);
        
        static void shutdown(); 
        
        static void setPoolSize(unsigned size) {
            __require__(pool);
            pool->setSize(size);
        }
        static unsigned getPoolSize() {
            __require__(pool);
            return pool->getSize();
        }
        
        static unsigned getConnectionsCount() {
            __require__(pool);
            return pool->getConnectionsCount();
        }
        static bool hasFreeConnections() {
            __require__(pool);
            return pool->hasFreeConnections();
        }
        static unsigned getBusyConnectionsCount() {
            __require__(pool);
            return pool->getBusyConnectionsCount();
        }
        static unsigned getIdleConnectionsCount() {
            __require__(pool);
            return pool->getIdleConnectionsCount();
        }
        static unsigned getDeadConnectionsCount() {
            __require__(pool);
            return pool->getDeadConnectionsCount();
        }

        virtual SMSId store(const SMS &sms)  
            throw(StorageException);
        virtual void retrive(SMSId id, SMS &sms)
            throw(StorageException, NoSuchMessageException);
        virtual void remove(SMSId id) 
            throw(StorageException, NoSuchMessageException);
    };

}}

#endif

