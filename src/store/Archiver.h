#ifndef ARCHIVER_DECLARATIONS
#define ARCHIVER_DECLARATIONS

#include <oci.h>
#include <orl.h>

#include <sms/sms.h>

#include "ConnectionManager.h"

namespace smsc { namespace store 
{
    using namespace smsc::sms;
    
    class Archiver
    {
    private:
        
        log4cpp::Category   &log;

        static const char*  selectSql;
        static const char*  insertSql;
        static const char*  deleteSql;

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
    
    public:

        Archiver(Manager& config)
            throw(ConfigException);
        virtual ~Archiver();

        void archivate()
            throw(StorageException); 
    };

}};

#endif


