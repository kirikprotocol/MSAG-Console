#ifndef ARCHIVER_DECLARATIONS
#define ARCHIVER_DECLARATIONS

#include <oci.h>
#include <orl.h>

#include <sms/sms.h>
#include <core/threads/Thread.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include "ConnectionManager.h"

namespace smsc { namespace store 
{
    using smsc::core::threads::Thread;
    using smsc::core::synchronization::Event;
    using smsc::core::synchronization::Mutex;

    using namespace smsc::sms;
    
    class Archiver : public Thread
    {
    private:
        
        log4cpp::Category   &log;

        Event       job, exit, exited;
        Mutex       finalizedMutex;
        int         finalizedCount;
        unsigned    maxFinalizedCount;
        unsigned    maxUncommitedCount;
        unsigned    awakeInterval;
        
        bool        bStarted, bArchivation;
        Mutex       processLock, startLock;
        
        static const char*  storageCountSql;
        static const char*  storageMaxIdSql;
        static const char*  archiveMaxIdSql;
        static const char*  billingMaxIdSql;
        
        static const char*  storageSelectSql;
        static const char*  storageDeleteSql;
        static const char*  archiveInsertSql;
        static const char*  billingInsertSql;
        static const char*  billingCleanIdsSql;
        static const char*  billingLookIdSql;
        static const char*  billingPutIdSql;
        
        const char*     storageDBInstance;
        const char*     storageDBUserName;
        const char*     storageDBUserPassword;
        
        const char*     billingDBInstance;
        const char*     billingDBUserName;
        const char*     billingDBUserPassword;
        
        Connection*     storageConnection;
        Connection*     billingConnection;
        
        Statement*      storageSelectStmt;
        Statement*      storageDeleteStmt;
        Statement*      archiveInsertStmt;
        
        Statement*      billingCleanIdsStmt;
        Statement*      billingInsertStmt;
        Statement*      billingLookIdStmt;
        Statement*      billingPutIdStmt;
        
        ub4             idCounter; // for lookIdStmt

        SMSId           id;
        uint8_t         uState;
        uint8_t         msgReference;
        
        uint8_t         oaLenght, oaType, oaPlan;
        uint8_t         daLenght, daType, daPlan;
        AddressValue    oaValue, daValue;

        OCIDate         waitTime;
        OCIDate         validTime;
        OCIDate         submitTime;
        OCIDate         deliveryTime;
        
        uint8_t         priority;
        uint8_t         protocolIdentifier;
        uint8_t         failureCause;
        
        char            bNeedArchivate;
        char            bHeaderIndicator;
        
        uint8_t         dataScheme;
        uint8_t         dataLenght;
        SMSData         data;
        
        const char* loadDBInstance(Manager& config, const char* cat)
            throw(ConfigException);
        const char* loadDBUserName(Manager& config, const char* cat)
            throw(ConfigException);
        const char* loadDBUserPassword(Manager& config, const char* cat)
            throw(ConfigException);
        
        void loadAwakeInterval(Manager& config);
        void loadMaxFinalizedCount(Manager& config);
        void loadMaxUncommitedCount(Manager& config);

        void prepareStorageSelectStmt() throw(StorageException);
        void prepareStorageDeleteStmt() throw(StorageException);
        void prepareArchiveInsertStmt() throw(StorageException);
        void prepareBillingInsertStmt() throw(StorageException);
        void prepareBillingCleanIdsStmt() throw(StorageException);
        void prepareBillingLookIdStmt() throw(StorageException);
        void prepareBillingPutIdStmt() throw(StorageException);

        SMSId getMaxUsedId(Connection* connection, const char* sql)
            throw(StorageException);
        
        void count()
            throw(StorageException);
        void connect()
            throw(StorageException); 
        void startup()
            throw(StorageException); 
        void billing(bool check)
            throw(StorageException); 
        void archivate(bool first)
            throw(StorageException); 
    
    public:

        Archiver(Manager& config)
            throw(ConfigException, StorageException);
        virtual ~Archiver();
    
        SMSId getLastUsedId()
            throw(StorageException); 

        void incrementFinalizedCount(unsigned count=1);
        void decrementFinalizedCount(unsigned count=1);

        virtual int Execute();
        
        void Start() throw(StorageException);
        void Stop();

        inline bool isStarted() {
            return bStarted;
        }
        inline bool isInProgress() {
            return bArchivation;
        }
    };

}};

#endif


