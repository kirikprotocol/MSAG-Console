#ifndef CONNECTION_MANAGER_DECLARATIONS
#define CONNECTION_MANAGER_DECLARATIONS

/**
 * ���� �������� �������� ������� ������������ ��� �����������
 * ������������ � ������ ������: 
 * Connection, ConnectionPool, Statement 
 *
 * ���������� �������� �� ���� ������� ���� Oracle � 
 * � �������������� ������� ���������� Oracle Call Interface. 
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see Statement
 */

#include <unistd.h>
#include <oci.h>
#include <orl.h>


#include <core/synchronization/EventMonitor.hpp>
#include <core/synchronization/Event.hpp>

#include <core/buffers/Array.hpp>
#include <util/config/Manager.h>
#include <logger/Logger.h>
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
    
    /**
     * ����� ��������� ������� ���������� � ����� ������.
     * �������� ��������� ����������, ����������� ��� ������������
     * ����������� ���������� � ���� Oracle, �������� � ����������
     * ������ SQL ���������� � ��������� ����� ����������.
     * ���������������� �������������� �������� commit() � rollback().
     * 
     * ������������ ��� �������� � ���������� SQL ����������
     * �� ��������� ���� ������.
     * 
     * ���������� �������� �� ���� ������� ���� Oracle �
     * � �������������� ������� ���������� Oracle Call Interface.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see Statement
     * @see ConnectionPool
     */
    class Connection
    {
        friend class Statement;
        friend class ConnectionPool;

    private:
        
        log4cpp::Category      &log;
        
        static Mutex    connectLock;

        Connection*     next;
        Mutex           mutex;
        
        Array<Statement *>          statements;

        inline void assign(Statement* statement) {
            statements.Push(statement);
        };
    
        inline void setNextConnection(Connection* connection) {
            next = connection;
        };
        
        inline Connection* getNextConnection(void) {
            return next;
        };
    
    protected:

        bool    isConnected, isDead;

        const char*     dbInstance;
        const char*     dbUserName;
        const char*     dbUserPassword;
        
        OCIEnv*         envhp;  // OCI envirounment handle
        OCISvcCtx*      svchp;  // OCI service handle
        OCIServer*      srvhp;  // OCI server handle
        OCIError*       errhp;  // OCI error handle
        OCISession*     sesshp; // OCI session handle

        Event           sleepOnReconnect;
        
    public:

        /**
         * �����������, ��������������, �� �� ������ �����������
         * ���������� � ����� ������. ��� ��������� ��������� ����������
         * ���������� ������������ ����� connect().
         * 
         * @param instance ����� ��� ���� ������
         * @param user     ��� ������������
         * @param password ������ ������������
         * @see Connection::connect()
         */
        Connection(const char* instance, 
                   const char* user, const char* password);
        
        /**
         * ����������, ���������� ���������� ���������� �
         * ����� ������, ���� ������� ���� �������.
         * ����� ���������� ��� SQL ��������� ���������������
         * � ������ �����������.
         * 
         * @see Connection::disconnect()
         */
        virtual ~Connection();

        /**
         * � ������ ���������� ��������� ���������� � ����� ������
         * �������� ������� ��� ��������.
         * 
         * @exception ConnectionFailedException
         *                   ��������� � ������ �������� �������
         *                   ��������� ����������
         */
        virtual void connect()
            throw(ConnectionFailedException);
        
        /**
         * � ������ ������� ��������� ���������� ���������� ���.
         * ����� ���������� ��� SQL ��������� ���������������
         * � ������ �����������.  
         */
        virtual void disconnect();
        
        /**
         * ��������� �����, ������������ ��� �������� ������������
         * ���������� ���������� ��������� �������� � ���������
         * ��������������� �������������� ��������
         * 
         * @param status ������ ���������� ��������� ��������
         * @exception StorageException
         *                   �������������� �������� ����������� �� ������
         *                   ������ ����, ������������������� ���������� status
         */
        virtual void check(sword status) 
            throw(StorageException);
        
        /**
         * ��������� commit ��� ������� �������� ����������
         * �� ����������.
         * 
         * @exception StorageException
         *                   ��������� � ������ ������ ��� commit'�
         *                   �� ����������
         */
        virtual void commit()
            throw(StorageException);
        
        /**
         * ��������� rollback ��� ������� �������� ����������
         * �� ����������.
         * 
         * @exception StorageException
         *                   ��������� � ������ ������ ��� rollback'�
         *                   �� ����������
         */
        virtual void rollback()
            throw(StorageException);

        /**
         * ���������� �������, ���� �� �������� ���������� �
         * ����� ������. ����� �� ������������ ���������� ���
         * ����������
         * 
         * @return �������, ����� �� ������������ ���������� ���
         *         ����������
         */
        inline bool isAvailable() {
            return (isConnected && !isDead);
        };

    };
    
    /**
     * ���������, ������������ ��������� � ����������
     * ������ ConnectionPool ��� ����������� "�������������"
     * ������������� ���������� �� ������������� �������.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see Connection
     * @see ConnectionPool
     */
    struct ConnectionQueue
    {
        cond_t              condition;
        Connection*         connection;
        ConnectionQueue*    next;
    };

    /**
     * ����� ��������� �������� ������ ���������� � ����� ������.
     * ������������ ������ � ������� ����������.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see Connection
     */
    class ConnectionPool
    {
    private:

        log4cpp::Category    &log;
        
        EventMonitor    monitor;
        ConnectionQueue *head,*tail;
        unsigned        maxQueueSize;
        unsigned        queueLen;
        
        Connection      *idleHead, *idleTail;
        unsigned        idleCount;
        
        void loadMaxSize(Manager& config);
        void loadInitSize(Manager& config);
        
        void loadDBInstance(Manager& config)
            throw(ConfigException);
        void loadDBUserName(Manager& config)
            throw(ConfigException);
        void loadDBUserPassword(Manager& config)
            throw(ConfigException);
    
    protected:
        
        char*   dbInstance;
        char*   dbUserName;
        char*   dbUserPassword;
        
        Array<Connection *> connections;

        unsigned    size;
        unsigned    count;

        void push(Connection* connection);
        Connection* pop(void);

        virtual Connection* newConnection();
    
    public:
    
        /**
         * �����������, ������ ���������� ��� ������ ����������.
         * ��������� ����� ���������������� ���������� � ��������������
         * ��������� ���������� � ����� ������ (�� ���������).
         * 
         * @param config �������� ��� ��������� ���������������� ����������
         * @exception ConfigException
         *                   ��������� � ������ �������������� �/���
         *                   ��������� ������ ���������������� ����������.
         * @see Connection
         * @see smsc::util::config::Manager
         */
        ConnectionPool(Manager& config) 
            throw(ConfigException);
        
        /**
         * ����������, ���������� ���������� ����������
         * � ���� ���������� � ����� ������
         */
        virtual ~ConnectionPool(); 
        
        /**
         * ������ ������ ���� ���������� � ����������.
         * � ���� ������ ������� ���� ���������� ����� ��������������
         * ������ ����� ������� ����������.
         * 
         * @param size   ����� ������ ���� ����������
         */
        void setSize(unsigned _size);
        
        /**
         * @return ������� ������ ���� ����������
         */
        inline unsigned getSize() {
            return size;
        }
        /**
         * @return ������� ���������� ���������� � ����
         */
        inline unsigned getConnectionsCount() {
            return count;
        }
        /**
         * @return ������� ���������� ������� ����������
         */
        inline unsigned getBusyConnectionsCount() {
            return (count-idleCount);
        }
        /**
         * @return ������� ���������� ������������� ����������
         */
        inline unsigned getIdleConnectionsCount() {
            return idleCount;
        }
        /**
         * @return ������� ���������� �������� ��������� ���������
         * @see ConnectionPool
         */
        inline unsigned getPendingQueueLength() {
            return queueLen;
        }
        
        /**
         * @return �������, ���� �� ��������� ����������
         */
        bool hasFreeConnections();
        
        /**
         * ���������� ��������� ���������� ��� �������������.
         * ���������� �������� � ������� ���������� ��������.
         * ����������� �� ������� ���������� ����������.
         * ����� ������������� ���������� ������ ���� ����������
         * � ��� ����������� ������ ConnectionPool::freeConnection()
         * 
         * @return ��������� ���������� ��� �������������
         * @see ConnectionPool::freeConnection()
         */
        Connection* getConnection();
        
        /**
         * ���������� ���������� � ���, ���������� ����������� 
         * ConnectionPool::getConnection()
         * 
         * @param connection
         * @see ConnectionPool::getConnection()
         */
        void freeConnection(Connection* connection);
    };

    /**
     * ����� ��������� ������� ���������� � ���������� ���������.
     * � ���������� � ���������������� ��������������� �������
     * ������� �������� ��� �������������� SQL ���������� ���
     * ����������������� ������������� � ����������.
     * 
     * ���������� �������� �� ���� ������� ���� Oracle �
     * � �������������� ������� ���������� Oracle Call Interface.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see Connection
     * @see StorageConnectionPool
     * @see Statement
     */
    class StorageConnection : public Connection
    {
    private:
        
        NeedOverwriteSvcStatement*  needOverwriteSvcStmt;
        NeedOverwriteStatement*     needOverwriteStmt;
        NeedRejectStatement*        needRejectStmt;
        OverwriteStatement*         overwriteStmt;
        StoreStatement*             storeStmt;
        RetrieveStatement*          retrieveStmt;
        RetrieveBodyStatement*      retrieveBodyStmt;
        DestroyStatement*           destroyStmt;
        
        ReplaceStatement*           replaceStmt;
        ReplaceVTStatement*         replaceVTStmt;
        ReplaceWTStatement*         replaceWTStmt;
        ReplaceVWTStatement*        replaceVWTStmt;
        ReplaceAllStatement*        replaceAllStmt;
        
        ToEnrouteStatement*         toEnrouteStmt;
        ToFinalStatement*           toFinalStmt;
        
        SetBodyStatement*           setBodyStmt;
        GetBodyStatement*           getBodyStmt;
        DestroyBodyStatement*       destroyBodyStmt;
        UpdateSeqNumStatement*      seqNumStmt;
        
    public:

        /**
         * �����������, ��������������, �� �� ������ �����������
         * ���������� � ���������� ���������. ��� ��������� ��������� ����������
         * ���������� ������������ ����� connect().
         * 
         * @param instance ����� ��� ���� ������
         * @param user     ��� ������������
         * @param password ������ ������������
         * @see StorageConnection::connect()
         */
        StorageConnection(const char* instance, 
                          const char* user, const char* password);
        /**
         * ����������, ���������� ���������� ���������� �
         * ����� ������, ���� ������� ���� �������.
         * ����� ���������� ��� SQL ��������� ���������������
         * � ������ �����������.
         * 
         * @see Connection::~Connection()
         */
        virtual ~StorageConnection() {};
        
        /**
         * � ������ ���������� ��������� ���������� � ����� ������
         * �������� ������� ��� ��������.
         * ����� ����, ����� ������ ����� ���������������
         * (����������) SQL ����������.
         *
         * @see Connection::connect() 
         * @exception ConnectionFailedException
         *                   ��������� � ������ �������� �������
         *                   ��������� ����������
         */
        virtual void connect()
            throw(ConnectionFailedException);
        
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        NeedOverwriteSvcStatement* getNeedOverwriteSvcStatement() 
            throw(ConnectionFailedException);
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        NeedOverwriteStatement* getNeedOverwriteStatement() 
            throw(ConnectionFailedException);
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        NeedRejectStatement*    getNeedRejectStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        OverwriteStatement*     getOverwriteStatement() 
            throw(ConnectionFailedException);
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        StoreStatement*         getStoreStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        DestroyStatement*       getDestroyStatement() 
            throw(ConnectionFailedException);
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        RetrieveStatement*       getRetrieveStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        RetrieveBodyStatement*   getRetrieveBodyStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ReplaceStatement*       getReplaceStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ReplaceVTStatement*     getReplaceVTStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ReplaceWTStatement*     getReplaceWTStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ReplaceVWTStatement*    getReplaceVWTStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ReplaceAllStatement*    getReplaceAllStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ToEnrouteStatement*         getToEnrouteStatement()
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ToFinalStatement*       getToFinalStatement()
            throw(ConnectionFailedException); 
        
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        SetBodyStatement* getSetBodyStatement()
            throw(ConnectionFailedException);
        
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        GetBodyStatement* getGetBodyStatement()
            throw(ConnectionFailedException);
        
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        DestroyBodyStatement* getDestroyBodyStatement()
            throw(ConnectionFailedException);

        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        UpdateSeqNumStatement* getUpdateSeqNumStatement()
            throw(ConnectionFailedException);

    };
    
    /**
     * ����� ��������� �������� ������ ���������� � ���������� ���������.
     * ������������ ������ � ������� ����������.
     * ��������� ���������������� ��������������� ������� �������
     * ��� ������ ������ � ������������ StorageConnection.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see ConnectionPool
     * @see StorageConnection
     */
    class StorageConnectionPool : public ConnectionPool
    {
    protected:
        
        /**
         * ������ ��������� StorageConnection. 
         * ������������ ��������� ��� ��������� ������ ���������� � ���
         * 
         * @return ����� ��������� StorageConnection
         * @see StorageConnection
         */
        virtual Connection* newConnection();

    public:

        /**
         * �����������, ������ ���������� ��� ������ ����������
         * � ���������� ���������.
         * 
         * @param config �������� ��� ��������� ���������������� ����������
         * @exception ConfigException
         *                   ��������� � ������ �������������� �/���
         *                   ��������� ������ ���������������� ����������.
         * @see ConnectionPool
         * @see StorageConnection
         * @see smsc::util::config::Manager
         */
        StorageConnectionPool(Manager& config) 
            throw(ConfigException);
        
        /**
         * ����������, ���������� ���������� ����������
         * � ���� ���������� � ���������� ���������.
         *
         * @see ConnectionPool
         */
        virtual ~StorageConnectionPool() {}; 
    };

}}

#endif


