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

        bool    isConnected, isDead;
        
        const char* dbInstance;
        const char* dbUserName;
        const char* dbUserPassword;
        
        inline void setNextConnection(Connection* connection) {
            next = connection;
        };
        
        inline Connection* getNextConnection(void) {
            return next;
        };
        
        OCIEnv*         envhp;  // OCI envirounment handle
        OCISvcCtx*      svchp;  // OCI service handle
        OCIServer*      srvhp;  // OCI server handle
        OCIError*       errhp;  // OCI error handle
        OCISession*     sesshp; // OCI session handle
        
        Array<Statement *>          statements;

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
        
        inline void assign(Statement* statement) {
            statements.Push(statement);
        };
    
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
         * ����� ����, ����� ������ ����� ���������������
         * (����������) SQL ����������.
         * 
         * @exception ConnectionFailedException
         *                   ��������� � ������ �������� �������
         *                   ��������� ����������
         */
        void connect()
            throw(ConnectionFailedException);
        
        /**
         * � ������ ������� ��������� ���������� ���������� ���.
         * ����� ���������� ��� SQL ��������� ���������������
         * � ������ �����������.  
         */
        void disconnect();
        
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
        void check(sword status) 
            throw(StorageException);
        
        /**
         * ��������� commit ��� ������� �������� ����������
         * �� ����������.
         * 
         * @exception StorageException
         *                   ��������� � ������ ������ ��� commit'�
         *                   �� ����������
         */
        void commit()
            throw(StorageException);
        
        /**
         * ��������� rollback ��� ������� �������� ����������
         * �� ����������.
         * 
         * @exception StorageException
         *                   ��������� � ������ ������ ��� rollback'�
         *                   �� ����������
         */
        void rollback()
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
        RetriveStatement*       getRetriveStatement() 
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
        ToEnrouteStatement*         getToEnrouteStatement()
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ToDeliveredStatement*       getToDeliveredStatement()
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ToUndeliverableStatement*   getToUndeliverableStatement()
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ToExpiredStatement*         getToExpiredStatement()
            throw(ConnectionFailedException); 
        /**
         * @return �������������� (��������) SQL ��������
         * @exception ConnectionFailedException
         *                   ��������� � ������ ������ ��������� ���������� �
         *                   ����� ������
         */
        ToDeletedStatement*         getToDeletedStatement()
            throw(ConnectionFailedException); 
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
        
        const char* dbInstance;
        const char* dbUserName;
        const char* dbUserPassword;

        EventMonitor    monitor;
        ConnectionQueue *head,*tail;
        unsigned        maxQueueSize;
        unsigned        queueLen;

        Array<Connection *> connections;
        unsigned        size;
        unsigned        count;
        
        Connection      *idleHead, *idleTail;
        unsigned        idleCount;
        
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

}}

#endif


