#ifndef SMSC_DB_DATASOURCE
#define SMSC_DB_DATASOURCE

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <time.h>
#include <string.h>
#include <inttypes.h>
#include <map>

#include <util/debug.h>
#include <logger/Logger.h>
#include <util/config/ConfigView.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>
#include <core/threads/Thread.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include <db/exceptions/DataSourceExceptions.h>

namespace smsc { namespace db
{
    using namespace smsc::core::synchronization;
    
    using smsc::logger::Logger;
    using smsc::core::buffers::Hash;
    using smsc::core::buffers::Array;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    using smsc::core::threads::Thread;
    using smsc::core::synchronization::Event;
    using smsc::core::synchronization::Mutex;

    class GetPosQuery
    {
    protected:
        GetPosQuery() {};
    public:
        virtual ~GetPosQuery() {};
        
        virtual bool isNull(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual const char* getString(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual int8_t getInt8(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual int16_t getInt16(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual int32_t getInt32(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual int64_t getInt64(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual uint8_t getUint8(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual uint16_t getUint16(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual uint32_t getUint32(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual uint64_t getUint64(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual float getFloat(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual double getDouble(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual long double getLongDouble(int pos)
            throw(SQLException, InvalidArgumentException) = 0;

        virtual time_t getDateTime(int pos)
            throw(SQLException, InvalidArgumentException) = 0;
    };
    
    class GetKeyQuery
    {
    protected:
        GetKeyQuery() {};
    public:
        virtual ~GetKeyQuery() {};
        
        virtual bool isNull(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual const char* getString(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual int8_t getInt8(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual int16_t getInt16(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual int32_t getInt32(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual int64_t getInt64(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual uint8_t getUint8(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual uint16_t getUint16(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual uint32_t getUint32(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual uint64_t getUint64(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual float getFloat(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual double getDouble(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual long double getLongDouble(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;

        virtual time_t getDateTime(const char* key)
            throw(SQLException, InvalidArgumentException) = 0;
    };

    class SetPosQuery
    {
    protected:
        SetPosQuery() {};
    public:
        virtual ~SetPosQuery() {};

        virtual void setString(int pos, const char* str, bool null=false)
            throw(SQLException) = 0;
        
        virtual void setInt8(int pos, int8_t val, bool null=false)
            throw(SQLException) = 0;
        virtual void setInt16(int pos, int16_t val, bool null=false)
            throw(SQLException) = 0;
        virtual void setInt32(int pos, int32_t val, bool null=false)
            throw(SQLException) = 0;
        virtual void setInt64(int pos, int64_t val, bool null=false)
            throw(SQLException) = 0;
        
        virtual void setUint8(int pos, uint8_t val, bool null=false)
            throw(SQLException) = 0;
        virtual void setUint16(int pos, uint16_t val, bool null=false)
            throw(SQLException) = 0;
        virtual void setUint32(int pos, uint32_t val, bool null=false)
            throw(SQLException) = 0;
        virtual void setUint64(int pos, uint64_t val, bool null=false)
            throw(SQLException) = 0;
        
        virtual void setFloat(int pos, float val, bool null=false)
            throw(SQLException) = 0;
        virtual void setDouble(int pos, double val, bool null=false)
            throw(SQLException) = 0;
        virtual void setLongDouble(int pos, long double val, bool null=false)
            throw(SQLException) = 0;
        
        virtual void setDateTime(int pos, time_t time, bool null=false)
            throw(SQLException) = 0;
    };
    
    class SetKeyQuery
    {
    protected:
        SetKeyQuery() {};
    public:
        virtual ~SetKeyQuery() {};

        virtual void setString(const char* key, const char* str, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual void setInt8(const char* key, int8_t val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual void setInt16(const char* key, int16_t val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual void setInt32(const char* key, int32_t val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual void setInt64(const char* key, int64_t val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual void setUint8(const char* key, uint8_t val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual void setUint16(const char* key, uint16_t val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual void setUint32(const char* key, uint32_t val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual void setUint64(const char* key, uint64_t val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual void setFloat(const char* key, float val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual void setDouble(const char* key, double val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        virtual void setLongDouble(const char* key, long double val, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
        
        virtual void setDateTime(const char* key, time_t time, bool null=false)
            throw(SQLException, InvalidArgumentException) = 0;
    };

    class ResultSet : public GetPosQuery
    {
    protected:
        ResultSet() {};
    public:
        virtual ~ResultSet() {};
        
        virtual bool fetchNext()
            throw(SQLException) = 0;
    };

    class Statement : public SetPosQuery
    {
    protected:
        Statement() {};
    public:
        virtual ~Statement() {};

        virtual void execute() 
            throw(SQLException) = 0;
        virtual uint32_t executeUpdate() 
            throw(SQLException) = 0;
        virtual ResultSet* executeQuery() 
            throw(SQLException) = 0;
    };

    class Routine : public SetKeyQuery, public GetKeyQuery
    {
    protected:
        Routine() {};
    public:
        virtual ~Routine() {};
        
        virtual void execute() 
            throw(SQLException) = 0;
    };

    class ConnectionPool;
    class Connection
    {
    friend class ConnectionPool;
    private:
        
        Connection*         next;

        inline void setNextConnection(Connection* connection) {
            next = connection;
        };
        
        inline Connection* getNextConnection(void) {
            return next;
        };
    
    protected:
        
		smsc::logger::Logger *log;

        bool                isConnected, isDead;
        
        Mutex               statementsRegistryLock, routinesRegistryLock;
        Hash<Statement *>   statementsRegistry;
        Hash<Routine *>     routinesRegistry;
        
        Connection() : next(0), log(Logger::getInstance("smsc.db.Connection")), 
            isConnected(false), isDead(false) {};

        Statement* _getStatement(const char* id);
        bool _registerStatement(const char* id, Statement* statement);
        Routine* _getRoutine(const char* id);
        bool _registerRoutine(const char* id, Routine* routine);

    public:
        
        virtual ~Connection() {};

        inline bool isAvailable() {
            return (isConnected && !isDead);
        };
    
        bool registerStatement(const char* id, Statement* statement);
        bool unregisterStatement(const char* id);
        Statement* getStatement(const char* id);
        Statement* getStatement(const char* id, const char* sql);
        virtual Statement* createStatement(const char* sql) 
            throw(SQLException) = 0;

        bool registerRoutine(const char* id, Routine* routine);
        bool unregisterRoutine(const char* id);
        Routine* getRoutine(const char* id);
        Routine* getRoutine(const char* id, const char* call, bool func=false);
        virtual Routine* createRoutine(const char* call, bool func=false) 
            throw(SQLException) = 0;

        virtual void connect() 
            throw(SQLException) = 0;
        virtual void disconnect();
        
        virtual void commit() 
            throw(SQLException) = 0;
        virtual void rollback() 
            throw(SQLException) = 0;
        virtual void abort() 
            throw(SQLException) = 0;
    };
    
    class DBDriver
    {
    protected:

        DBDriver(ConfigView* config)
            throw (ConfigException) {};

    public:
        
        virtual ~DBDriver() {};
        virtual Connection* newConnection() = 0;
    };
    
    struct ConnectionDeadline
    {
        Connection* connection;
        time_t      deadline;

        ConnectionDeadline(Connection* _connection=0, time_t _deadline=0)
            : connection(_connection), deadline(_deadline) {};
        ConnectionDeadline(const ConnectionDeadline& cd)
            : connection(cd.connection), deadline(cd.deadline) {};
        
        ConnectionDeadline& operator =(const ConnectionDeadline& cd) {
            connection = cd.connection; deadline = cd.deadline;
            return (*this);
        };
    };

    typedef std::map<int, ConnectionDeadline>  TimersMap;
    typedef std::pair<int, ConnectionDeadline> TimersPair;
    typedef TimersMap::iterator                TimersIterator;

    class WatchDog : public Thread
    {
    private:
        
        Event   awake, exited;
        bool    bStarted, bNeedExit;
        Mutex   startLock, timersLock;

        TimersMap   timers;

    public:

        WatchDog() : Thread(), bStarted(false), bNeedExit(false) {};
        virtual ~WatchDog() { Stop(); };

        virtual int Execute();
        void Start();
        void Stop();

        int startTimer(Connection* connection, uint32_t timeout);
        void stopTimer(int timer);
    };

    class DataSource
    {
    protected:
        
        DBDriver*       driver;
        WatchDog*       watchDog;
       
        DataSource() : driver(0), watchDog(0) {};
    
    public:

        virtual ~DataSource() 
        {
            if (watchDog) delete watchDog;
            if (driver) delete driver;
        };
        
        inline Connection* newConnection()
        {
            return ((driver) ? driver->newConnection():0);
        };

        int startTimer(Connection* connection, uint32_t timeout)
        {
            return (watchDog && timeout > 0) ? 
                watchDog->startTimer(connection, timeout) : -1;
        }
        void stopTimer(int timer)
        {
            if (watchDog && timer >= 0)
                watchDog->stopTimer(timer); 
        }
        
        virtual void init(ConfigView* config) 
            throw(ConfigException);
        
        virtual Connection* getConnection() = 0;
        virtual void freeConnection(Connection* connection) = 0;
        virtual void closeConnections() = 0;
        virtual void closeRegisteredQueries(const char* id) = 0;
    };

    class DataSourceFactory 
    {
    protected:

        static Hash<DataSourceFactory *>*   registry;
        static Mutex                        registryLock;
        
        DataSourceFactory() {};
        virtual ~DataSourceFactory() {};
        
        virtual DataSource* createDataSource() = 0;
        
    public:

        static void registerFactory(DataSourceFactory* dsf, const char* key)
        {
            MutexGuard guard(registryLock);

            if (!registry) registry = new Hash<DataSourceFactory *>();
            registry->Insert(key, dsf);
        };
        static void unregisterFactories()
        {
            MutexGuard guard(registryLock);
            
            if (registry) registry->Empty();
        };
        
        static DataSource* getDataSource(const char* key)
        {
            MutexGuard guard(registryLock);

            DataSourceFactory* dsf = (registry) ? 
                ((registry->Exists(key)) ? registry->Get(key):0):0;
            return ((dsf) ? dsf->createDataSource():0);
        };
    };
    
    class ConnectionPool
    {
    private:

        DataSource           &ds;
        smsc::logger::Logger *log;

        EventMonitor         monitor;
        
        struct ConnectionQueue
        {
            pthread_cond_t      condition;
            Connection*         connection;
            ConnectionQueue*    next;
        };

        ConnectionQueue    *head,*tail;
        unsigned            queueLen;
        
        Connection         *idleHead, *idleTail;
        unsigned            idleCount;

        void loadPoolSize(ConfigView* config);
        
    protected:

        Array<Connection *> connections;
        unsigned            size, count;
        
        void push(Connection* connection);
        Connection* pop(void);

    public:

        ConnectionPool(DataSource& ds, ConfigView* config) 
            throw(ConfigException);

        virtual ~ConnectionPool(); 

        Connection* getConnection();
        void freeConnection(Connection* connection);
        void closeConnections();
        void closeRegisteredQueries(const char* id);
    };

    class PoolledDataSource : public DataSource
    {
    protected:
        
        ConnectionPool* pool;
        
        PoolledDataSource() : DataSource() {};
        
        virtual void init(ConfigView* config)
            throw(ConfigException)
        {
            DataSource::init(config);
            pool = new ConnectionPool(*((DataSource *)this), config);
            if (watchDog) watchDog->Start();
        };

    public:

        virtual ~PoolledDataSource()
        {
            if (watchDog) watchDog->Stop();
            if (pool) delete pool;
        };

        Connection* getConnection() {
            return pool->getConnection();
        };
        void freeConnection(Connection* connection) {
            pool->freeConnection(connection);
        };
        void closeConnections() {
            pool->closeConnections();
        };
        void closeRegisteredQueries(const char* id) {
            pool->closeRegisteredQueries(id);
        }
    };
    
}}

#endif
