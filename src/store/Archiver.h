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
        
        static const char*  countSql;
        static const char*  storageMaxIdSql;
        static const char*  archiveMaxIdSql;

        static const char*  selectSql;
        static const char*  insertSql;
        static const char*  deleteSql;
        static const char*  lookIdSql;

        const char*     storageDBInstance;
        const char*     storageDBUserName;
        const char*     storageDBUserPassword;
        
        const char*     archiveDBInstance;
        const char*     archiveDBUserName;
        const char*     archiveDBUserPassword;
        
        Connection*     storageConnection;
        Connection*     archiveConnection;
        
        Statement*      selectStmt;
        Statement*      insertStmt;
        Statement*      deleteStmt;
        Statement*      lookIdStmt;
        
        ub4             idCounter; // for lookIdStmt

        SMSId           id;
        uint8_t         uState;
        uint8_t         msgReference;
        uint8_t         msgIdentifier;

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
        
        char            bStatusReport;
        char            bNeedArchivate;
        char            bHeaderIndicator;
        char            bRejectDuplicates;
        
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

        void prepareSelectStmt() throw(StorageException);
        void prepareInsertStmt() throw(StorageException);
        void prepareDeleteStmt() throw(StorageException);
        void prepareLookIdStmt() throw(StorageException);

        void connect()
            throw(StorageException); 
        void startup()
            throw(StorageException); 
        void archivate(bool first)
            throw(StorageException); 
    
    public:

        Archiver(Manager& config)
            throw(ConfigException, StorageException);
        virtual ~Archiver();
    
        SMSId getMaxId()
            throw(StorageException); 

        void incrementFinalizedCount(unsigned count=1);
        void decrementFinalizedCount(unsigned count=1);

        virtual int Execute();
    };

}};

#endif


