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
    
    class Connection
    {
    friend class Statement;
    private:

        Connection* next;

        static Mutex connectLock;

        bool    isConnected, isDead;
        log4cpp::Category      &log;
        
        const char* dbInstance;
        const char* dbUserName;
        const char* dbUserPassword;
    
    protected:
        
        OCIEnv*         envhp;  // OCI envirounment handle
        OCISvcCtx*      svchp;  // OCI service handle
        OCIServer*      srvhp;  // OCI server handle
        OCIError*       errhp;  // OCI error handle
        OCISession*     sesshp; // OCI session handle
        
        StoreStatement*     StoreStmt;
        RemoveStatement*    RemoveStmt;
        RetriveStatement*   RetriveStmt;
        ReplaceStatement*   ReplaceStmt;

        Mutex           mutex;
        
        /*void checkConnErr(sword status) 
            throw(ConnectionFailedException);*/
        
        void checkErr(sword status) 
            throw(StorageException);
        
        void connect()
            throw(ConnectionFailedException);
        void disconnect();

    public:

        Connection(const char* instance, 
                   const char* user, const char* password);
        virtual ~Connection();

        inline void setNextConnection(Connection* connection) {
            next = connection;
        };
        inline Connection* getNextConnection(void) {
            return next;
        };

        SMSId getMessagesCount()
            throw(ConnectionFailedException, StorageException);
        void store(const SMS &sms, SMSId id) 
            throw(ConnectionFailedException, StorageException);
        void retrive(SMSId id, SMS &sms) 
            throw(ConnectionFailedException, StorageException, 
                  NoSuchMessageException);
        void remove(SMSId id) 
            throw(ConnectionFailedException, StorageException, 
                  NoSuchMessageException);
        void replace(SMSId id, const SMS &sms) 
            throw(ConnectionFailedException, StorageException,
                  NoSuchMessageException);
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
        
        Connection* getConnection()
            throw(TooLargeQueueException);
        void freeConnection(Connection* connection);
    };

}}

#endif


