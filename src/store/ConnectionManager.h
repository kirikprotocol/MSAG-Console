#ifndef CONNECTION_MANAGER_DECLARATIONS
#define CONNECTION_MANAGER_DECLARATIONS

/**
 * Файл содержит описание средств используемых для организации
 * взаимойствия с базами данных: 
 * Connection, ConnectionPool, Statement 
 *
 * Реализация основана на базе средств СУБД Oracle и 
 * с использованием средств разработки Oracle Call Interface. 
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
     * Класс реализует понятие соединения с базой данных.
     * Содержит сервисную информацию, необходимую для установления
     * логического соединения с СУБД Oracle, создания и выполнения
     * набора SQL операторов в контексте одной транзакции.
     * Транзакционность поддерживается методами commit() и rollback().
     * 
     * Используется для создания и выполнения SQL операторов
     * на указанной базе данных.
     * 
     * Реализация основана на базе средств СУБД Oracle и
     * с использованием средств разработки Oracle Call Interface.
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
         * Конструктор, инициализирует, но не создаёт логического
         * соединения с базой данных. Для получения реального соединения
         * необходимо использовать метод connect().
         * 
         * @param instance алиас для базы данных
         * @param user     имя пользователя
         * @param password пароль пользователя
         * @see Connection::connect()
         */
        Connection(const char* instance, 
                   const char* user, const char* password);
        
        /**
         * Деструктор, уничтожает логическое соединение с
         * базой данных, если таковое было открыто.
         * Также уничтожает все SQL операторы ассоциированные
         * с данным соединением.
         * 
         * @see Connection::disconnect()
         */
        virtual ~Connection();

        /**
         * В случае отсутствия реального соединения с базой данных
         * делается попытка его создания.
         * 
         * @exception ConnectionFailedException
         *                   возникает в случае неуспеха попытки
         *                   получения соединения
         */
        virtual void connect()
            throw(ConnectionFailedException);
        
        /**
         * В случае наличия реального соединения уничтожает его.
         * Также уничтожает все SQL операторы ассоциированные
         * с данным соединением.  
         */
        virtual void disconnect();
        
        /**
         * Сервисный метод, используется для проверки корректности
         * результата выполнения последней операции и генерации
         * соответствующей исключительной ситуации
         * 
         * @param status статус выполнения последней операции
         * @exception StorageException
         *                   исклучительная ситуация возникающая по ошибке
         *                   любого рода, идентифицированного параметром status
         */
        virtual void check(sword status) 
            throw(StorageException);
        
        /**
         * Реализует commit для текущей открытой транзакции
         * на соединении.
         * 
         * @exception StorageException
         *                   возникает в случае ошибки при commit'е
         *                   на соединении
         */
        virtual void commit()
            throw(StorageException);
        
        /**
         * Реализует rollback для текущей открытой транзакции
         * на соединении.
         * 
         * @exception StorageException
         *                   возникает в случае ошибки при rollback'е
         *                   на соединении
         */
        virtual void rollback()
            throw(StorageException);

        /**
         * Возвращает признак, есть ли реальное соединение с
         * базой данных. Можно ли использовать соединение без
         * реконнекта
         * 
         * @return признак, можно ли использовать соединение без
         *         реконнекта
         */
        inline bool isAvailable() {
            return (isConnected && !isDead);
        };

    };
    
    /**
     * Структура, используется внутренне в реализации
     * класса ConnectionPool для организации "справедливого"
     * распределения соединений по запрашивающим потокам.
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
     * Класс реализует контроль набора соединений с базой данных.
     * Обеспечивает выдачу и возврат соединений.
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
         * Конструктор, создаёт контроллер для набора соединений.
         * Извлекает набор конфигурационных параметров и инициализирует
         * несколько соединений с базой данных (по параметру).
         * 
         * @param config интерфес для получения конфигурационных параметров
         * @exception ConfigException
         *                   возникает в случае некорректности и/или
         *                   неполноты набора конфигурационных параметров.
         * @see Connection
         * @see smsc::util::config::Manager
         */
        ConnectionPool(Manager& config) 
            throw(ConfigException);
        
        /**
         * Деструктор, уничтожает контроллер соединений
         * и сами соединения с базой данных
         */
        virtual ~ConnectionPool(); 
        
        /**
         * Меняет размер пула соединений с хранилищем.
         * В один момент времени одно соединение может использоваться
         * только одним потоком управления.
         * 
         * @param size   новый размер пула соединений
         */
        void setSize(unsigned _size);
        
        /**
         * @return текущий размер пула соединений
         */
        inline unsigned getSize() {
            return size;
        }
        /**
         * @return текущее количество соединений в пуле
         */
        inline unsigned getConnectionsCount() {
            return count;
        }
        /**
         * @return текущее количество занятых соединений
         */
        inline unsigned getBusyConnectionsCount() {
            return (count-idleCount);
        }
        /**
         * @return текущее количество простаивающих соединений
         */
        inline unsigned getIdleConnectionsCount() {
            return idleCount;
        }
        /**
         * @return текущее количество запросов ожидающих обработку
         * @see ConnectionPool
         */
        inline unsigned getPendingQueueLength() {
            return queueLen;
        }
        
        /**
         * @return признак, есть ли свободные соединения
         */
        bool hasFreeConnections();
        
        /**
         * Возвращает свободное соединение для использования.
         * Соединения выдаются в порядке очерёдности запросов.
         * Блокируется до наличия свободного соединения.
         * После использования соединение должно быть возвращено
         * в пул посредством метода ConnectionPool::freeConnection()
         * 
         * @return свободное соединение для использования
         * @see ConnectionPool::freeConnection()
         */
        Connection* getConnection();
        
        /**
         * Возвращает соединение в пул, полученное посредством 
         * ConnectionPool::getConnection()
         * 
         * @param connection
         * @see ConnectionPool::getConnection()
         */
        void freeConnection(Connection* connection);
    };

    /**
     * Класс реализует понятие соединения с хранилищем сообщений.
     * В дополнение к функциональности предоставляемой базовым
     * классом содержит ряд подготовленных SQL операторов для
     * непосредственного взаимодейсвия с хранилищем.
     * 
     * Реализация основана на базе средств СУБД Oracle и
     * с использованием средств разработки Oracle Call Interface.
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
         * Конструктор, инициализирует, но не создаёт логического
         * соединения с хранилищем сообщений. Для получения реального соединения
         * необходимо использовать метод connect().
         * 
         * @param instance алиас для базы данных
         * @param user     имя пользователя
         * @param password пароль пользователя
         * @see StorageConnection::connect()
         */
        StorageConnection(const char* instance, 
                          const char* user, const char* password);
        /**
         * Деструктор, уничтожает логическое соединение с
         * базой данных, если таковое было открыто.
         * Также уничтожает все SQL операторы ассоциированные
         * с данным соединением.
         * 
         * @see Connection::~Connection()
         */
        virtual ~StorageConnection() {};
        
        /**
         * В случае отсутствия реального соединения с базой данных
         * делается попытка его создания.
         * Кроме того, метод создаёт набор предопределённых
         * (сохранённых) SQL операторов.
         *
         * @see Connection::connect() 
         * @exception ConnectionFailedException
         *                   возникает в случае неуспеха попытки
         *                   получения соединения
         */
        virtual void connect()
            throw(ConnectionFailedException);
        
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        NeedOverwriteSvcStatement* getNeedOverwriteSvcStatement() 
            throw(ConnectionFailedException);
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        NeedOverwriteStatement* getNeedOverwriteStatement() 
            throw(ConnectionFailedException);
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        NeedRejectStatement*    getNeedRejectStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        OverwriteStatement*     getOverwriteStatement() 
            throw(ConnectionFailedException);
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        StoreStatement*         getStoreStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        DestroyStatement*       getDestroyStatement() 
            throw(ConnectionFailedException);
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        RetrieveStatement*       getRetrieveStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        RetrieveBodyStatement*   getRetrieveBodyStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        ReplaceStatement*       getReplaceStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        ReplaceVTStatement*     getReplaceVTStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        ReplaceWTStatement*     getReplaceWTStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        ReplaceVWTStatement*    getReplaceVWTStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        ReplaceAllStatement*    getReplaceAllStatement() 
            throw(ConnectionFailedException); 
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        ToEnrouteStatement*         getToEnrouteStatement()
            throw(ConnectionFailedException); 
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        ToFinalStatement*       getToFinalStatement()
            throw(ConnectionFailedException); 
        
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        SetBodyStatement* getSetBodyStatement()
            throw(ConnectionFailedException);
        
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        GetBodyStatement* getGetBodyStatement()
            throw(ConnectionFailedException);
        
        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        DestroyBodyStatement* getDestroyBodyStatement()
            throw(ConnectionFailedException);

        /**
         * @return подготовленный (хранимый) SQL оператор
         * @exception ConnectionFailedException
         *                   возникает в случае потери реального соединения с
         *                   базой данных
         */
        UpdateSeqNumStatement* getUpdateSeqNumStatement()
            throw(ConnectionFailedException);

    };
    
    /**
     * Класс реализует контроль набора соединений с хранилищем сообщений.
     * Обеспечивает выдачу и возврат соединений.
     * Расширяет функциональность предоставляемую базовым классом
     * для работы только с экземплярами StorageConnection.
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
         * Создаёт экземпляр StorageConnection. 
         * Используется внутренне при помещении нового соединения в пул
         * 
         * @return новый экземпляр StorageConnection
         * @see StorageConnection
         */
        virtual Connection* newConnection();

    public:

        /**
         * Конструктор, создаёт контроллер для набора соединений
         * с хранилищем сообщений.
         * 
         * @param config интерфес для получения конфигурационных параметров
         * @exception ConfigException
         *                   возникает в случае некорректности и/или
         *                   неполноты набора конфигурационных параметров.
         * @see ConnectionPool
         * @see StorageConnection
         * @see smsc::util::config::Manager
         */
        StorageConnectionPool(Manager& config) 
            throw(ConfigException);
        
        /**
         * Деструктор, уничтожает контроллер соединений
         * и сами соединения с хранилищем сообщений.
         *
         * @see ConnectionPool
         */
        virtual ~StorageConnectionPool() {}; 
    };

}}

#endif


