#ifndef STORE_MANAGER_DECLARATIONS
#define STORE_MANAGER_DECLARATIONS

#include <oci.h>
#include <orl.h>

#include <core/synchronization/Mutex.hpp>
#include <util/config/Manager.h>
#include <util/Logger.h>

#include "StoreConfig.h"
#include "MessageStore.h"
#include "ConnectionManager.h"
#include "Archiver.h"

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
        
        SMSId   id;
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
        static Archiver             *archiver;
        static StoreManager         *instance;
        static ConnectionPool       *pool;
        static log4cpp::Category    &log;
        
        static unsigned             maxTriesCount;
        static void loadMaxTriesCount(Manager& config);

    protected:
        
        StoreManager() : MessageStore() {};
        virtual ~StoreManager() {};
        
        void doCreateSms(Connection* connection,
            SMS& sms, SMSId id, const CreateMode flag)
                throw(StorageException, DuplicateMessageException);
        void doRetriveSms(Connection* connection, 
            SMSId id, SMS& sms)
                throw(StorageException, NoSuchMessageException);
        void doReplaceSms(Connection* connection, 
            SMSId id, const Address& oa, 
            const Body& newBody, uint8_t deliveryReport,
            time_t validTime = 0, time_t waitTime = 0)
                throw(StorageException, NoSuchMessageException);
        void doDestroySms(Connection* connection, SMSId id) 
                throw(StorageException, NoSuchMessageException);
        
        void doChangeSmsStateToEnroute(Connection* connection,
            SMSId id, const Descriptor& dst,
            uint8_t failureCause, time_t nextTryTime)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToDelivered(Connection* connection, 
            SMSId id, const Descriptor& dst)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToUndeliverable(Connection* connection, 
            SMSId id, const Descriptor& dst, uint8_t failureCause)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToExpired(Connection* connection, 
            SMSId id)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToDeleted(Connection* connection,
            SMSId id) 
                throw(StorageException, NoSuchMessageException);
    
    public:    
        
        static void startup(Manager& config)
            throw(ConfigException, ConnectionFailedException);
        
        static MessageStore* getMessageStore() {
            return ((MessageStore *)instance);
        }
        
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
        static unsigned getPendingQueueLength() {
            __require__(pool);
            return pool->getPendingQueueLength();
        }
        
        static void startArchiver() 
            throw (StorageException)
        {
            __require__(archiver);
            return archiver->Start();
        }
        static void stopArchiver() {
            __require__(archiver);
            return archiver->Stop();
        }
        static bool isArchiverStarted() {
            __require__(archiver);
            return archiver->isStarted();
        }
        static bool isArchivationInProgress() {
            __require__(archiver);
            return archiver->isInProgress();
        }

        virtual SMSId createSms(SMS& sms, const CreateMode flag = CREATE_NEW)
                throw(StorageException, DuplicateMessageException);
        virtual void retriveSms(SMSId id, SMS &sms)
                throw(StorageException, NoSuchMessageException);
        virtual void replaceSms(SMSId id, const Address& oa,
            const Body& newBody, uint8_t deliveryReport,
            time_t validTime = 0, time_t waitTime = 0)
                throw(StorageException, NoSuchMessageException); 
        /* Needed for test needs. Will be removed.*/
        virtual void destroySms(SMSId id) 
                throw(StorageException, NoSuchMessageException);
        
        virtual void changeSmsStateToEnroute(SMSId id,
            const Descriptor& dst, uint8_t failureCause, time_t nextTryTime) 
                throw(StorageException, NoSuchMessageException); 
        virtual void changeSmsStateToDelivered(SMSId id, 
            const Descriptor& dst) 
                throw(StorageException, NoSuchMessageException); 
        virtual void changeSmsStateToUndeliverable(SMSId id,
            const Descriptor& dst, uint8_t failureCause) 
                throw(StorageException, NoSuchMessageException); 
        virtual void changeSmsStateToExpired(SMSId id) 
                throw(StorageException, NoSuchMessageException); 
        virtual void changeSmsStateToDeleted(SMSId id) 
                throw(StorageException, NoSuchMessageException); 
       
    };

}}

#endif

