#ifndef STORE_MANAGER_DECLARATIONS
#define STORE_MANAGER_DECLARATIONS

#include <oci.h>
#include <core/synchronization/Mutex.hpp>
#include <util/Logger.h>

#include "StoreConfig.h"
#include "MessageStore.h"
#include "ConnectionManager.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;
    using namespace smsc::core::synchronization;
    using smsc::util::Logger;
    
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

        static IDGenerator      *generator;
        static StoreManager     *instance;
        static ConnectionPool   *pool;
        log4cpp::Category       &log;

    protected:
        
        StoreManager() : MessageStore(), 
            log(Logger::getCategory("smsc.store.StoreManager")) {};
        virtual ~StoreManager() {};

    public:    
        
        static MessageStore* startup(const char* db, const char* user, 
                                     const char* password, 
                                     unsigned size, unsigned init)
            throw(ConnectionFailedException);
        
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

