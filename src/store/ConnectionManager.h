#ifndef CONNECTION_MANAGER_DECLARATIONS
#define CONNECTION_MANAGER_DECLARATIONS

#include <unistd.h>
#include <oci.h>
#include <orl.h>

#include <core/synchronization/EventMonitor.hpp>
#include <core/buffers/Array.hpp>
#include <util/config/Manager.h>
#include <util/Logger.h>
#include <sms/sms.h>

#include "StoreExceptions.h"
#include "Statement.h"

namespace smsc { namespace store 
{
    using smsc::util::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;
    using smsc::core::buffers::Array;

    using namespace smsc::core::synchronization;
    using namespace smsc::sms;
    
    class ConnectionPool;
    class Connection
    {
        friend class Statement;
        friend class ConnectionPool;

    private:

        Connection* next;

        static Mutex    connectLock;
        Mutex           mutex;

        bool    isConnected, isDead;
        log4cpp::Category      &log;
        
        const char* dbInstance;
        const char* dbUserName;
        const char* dbUserPassword;
        
        inline void setNextConnection(Connection* connection) {
            next = connection;
        };
        
        inline Connection* getNextConnection(void) {
            return next;
        };
    
    protected:
        
        OCIEnv*         envhp;  // OCI envirounment handle
        OCISvcCtx*      svchp;  // OCI service handle
        OCIServer*      srvhp;  // OCI server handle
        OCIError*       errhp;  // OCI error handle
        OCISession*     sesshp; // OCI session handle
        
        Array<Statement *>      statements;

        NeedOverwriteSvcStatement*  needOverwriteSvcStmt;
        NeedOverwriteStatement*     needOverwriteStmt;
        NeedRejectStatement*        needRejectStmt;
        OverwriteStatement*         overwriteStmt;
        StoreStatement*             storeStmt;
        RetriveStatement*           retriveStmt;
        DestroyStatement*           destroyStmt;
        
        ReplaceStatement*           replaceStmt;
        ReplaceVTStatement*         replaceVTStmt;
        ReplaceWTStatement*         replaceWTStmt;
        ReplaceVWTStatement*        replaceVWTStmt;
        
        ToEnrouteStatement*         toEnrouteStmt;
        ToDeliveredStatement*       toDeliveredStmt;
        ToUndeliverableStatement*   toUndeliverableStmt;
        ToExpiredStatement*         toExpiredStmt;
        ToDeletedStatement*         toDeletedStmt;
        
    public:

        Connection(const char* instance, 
                   const char* user, const char* password);
        virtual ~Connection();

        void check(sword status) 
            throw(StorageException);
    
        void connect()
            throw(ConnectionFailedException);
        void disconnect();
        
        void commit()
            throw(StorageException);
        void rollback()
            throw(StorageException);

        inline void assign(Statement* statement) {
            statements.Push(statement);
        };
        
        inline bool isAvailable() {
            return (isConnected && !isDead);
        };

        NeedOverwriteSvcStatement* getNeedOverwriteSvcStatement() 
            throw(ConnectionFailedException);
        NeedOverwriteStatement* getNeedOverwriteStatement() 
            throw(ConnectionFailedException);
        NeedRejectStatement*    getNeedRejectStatement() 
            throw(ConnectionFailedException); 
        OverwriteStatement*     getOverwriteStatement() 
            throw(ConnectionFailedException);
        StoreStatement*         getStoreStatement() 
            throw(ConnectionFailedException); 
        DestroyStatement*       getDestroyStatement() 
            throw(ConnectionFailedException);
        RetriveStatement*       getRetriveStatement() 
            throw(ConnectionFailedException); 
        
        ReplaceStatement*       getReplaceStatement() 
            throw(ConnectionFailedException); 
        ReplaceVTStatement*     getReplaceVTStatement() 
            throw(ConnectionFailedException); 
        ReplaceWTStatement*     getReplaceWTStatement() 
            throw(ConnectionFailedException); 
        ReplaceVWTStatement*    getReplaceVWTStatement() 
            throw(ConnectionFailedException); 
        
        ToEnrouteStatement*         getToEnrouteStatement()
            throw(ConnectionFailedException); 
        ToDeliveredStatement*       getToDeliveredStatement()
            throw(ConnectionFailedException); 
        ToUndeliverableStatement*   getToUndeliverableStatement()
            throw(ConnectionFailedException); 
        ToExpiredStatement*         getToExpiredStatement()
            throw(ConnectionFailedException); 
        ToDeletedStatement*         getToDeletedStatement()
            throw(ConnectionFailedException); 
    };
    
    struct ConnectionQueue
    {
        cond_t              condition;
        Connection*         connection;
        ConnectionQueue*    next;
    };

    class ConnectionPool
    {
    private:

        log4cpp::Category    &log;
        
        const char* dbInstance;
        const char* dbUserName;
        const char* dbUserPassword;

        EventMonitor    monitor;
        ConnectionQueue *head,*tail;
        unsigned        maxQueueSize;
        unsigned        queueLen;

        Array<Connection *> connections;
        unsigned    size;
        unsigned    count;
        
        Connection  *idleHead, *idleTail;
        unsigned     idleCount;
        
        void push(Connection* connection);
        Connection* pop(void);

        void loadMaxSize(Manager& config);
        void loadInitSize(Manager& config);
        
        void loadDBInstance(Manager& config)
            throw(ConfigException);
        void loadDBUserName(Manager& config)
            throw(ConfigException);
        void loadDBUserPassword(Manager& config)
            throw(ConfigException);

    public:
    
        ConnectionPool(Manager& config) 
            throw(ConfigException);
        virtual ~ConnectionPool(); 
        
        void setSize(unsigned _size);

        inline unsigned getSize() {
            return size;
        }
        inline unsigned getConnectionsCount() {
            return count;
        }
        inline unsigned getBusyConnectionsCount() {
            return (count-idleCount);
        }
        inline unsigned getIdleConnectionsCount() {
            return idleCount;
        }
        inline unsigned getPendingQueueLength() {
            return queueLen;
        }
        
        bool hasFreeConnections();
        
        Connection* getConnection();
        void freeConnection(Connection* connection);
    };

}}

#endif


