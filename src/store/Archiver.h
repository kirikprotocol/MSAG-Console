#ifndef ARCHIVER_DECLARATIONS
#define ARCHIVER_DECLARATIONS

/**
 * Файл содержит описание подсистемы архивации
 * и создания биллинговых записей в контексте SMS центра.
 * Подсистема работает во взаимодействии с подсистемой хранения сообщений.
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see StoreManager
 */

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
    
    /**
     * Подсистема архивации и создания биллинговых записей
     * в контексте SMS центра.
     * Работает во взаимодействии с подсистемой хранения сообщений.
     * Переодически извлекает сообщения из хранилища, переносит их в
     * архив и создаёт биллинговую запись для каждого. 
     * 
     * Процесс архивации и создания биллинговых записей реализован
     * в виде отдельного потока управления в системе SMS центра.
     * Система активизируется посредством двух событий:
     * 1) Истёк интервал времени простоя;
     * 2) Количество сообщений в финальном состоянии в хранилище
     *    превышает определённый уровень.
     * Указанные параметры задаются при кофигурировании системы.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see StoreManager
     */
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
        
        bool        bStarted;
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
        
        const char* storageDBInstance;
        const char* storageDBUserName;
        const char* storageDBUserPassword;
        
        const char* billingDBInstance;
        const char* billingDBUserName;
        const char* billingDBUserPassword;
        
        Connection* storageConnection;
        Connection* billingConnection;
        
        Statement*  storageSelectStmt;
        Statement*  storageDeleteStmt;
        Statement*  archiveInsertStmt;
        
        Statement*  billingCleanIdsStmt;
        Statement*  billingInsertStmt;
        Statement*  billingLookIdStmt;
        Statement*  billingPutIdStmt;
        
        ub4         idCounter;

        SMSId       id;
        SMS         sms;
        
        OCIDate     waitTime;
        OCIDate     validTime;
        OCIDate     submitTime;
        OCIDate     lastTime;
        OCIDate     nextTime;
        
        uint8_t     uState;
        char        bHeaderIndicator;
        char        bNeedArchivate;

        sb2         indOA, indSrcMsc, indSrcImsi, indSrcSme;
        sb2         indDA, indDstMsc, indDstImsi, indDstSme;
        sb2         indWaitTime, indLastTime;
        
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

        /**
         * Конструктор, создаёт (но не запускает) систему архивации
         * и создания биллинговых записей.
         * Извлекает набор конфигурационных параметров и создаёт два
         * соединения с хранилищами: сообщений и биллинговых записей.
         * 
         * @param config интерфес для получения конфигурационных параметров
         * @exception ConfigException
         *                   возникает в случае некорректности и/или
         *                   неполноты набора конфигурационных параметров.
         * @see Connection
         * @see smsc::util::config::Manager
         */
        Archiver(Manager& config)
            throw(ConfigException);
        
        /**
         * Деструктор, останавливает и уничтожает систему архивации
         * и создания биллинговых записей.
         * Ожидает завершения текущего минимального кванта работы.
         * Также, уничтожает соединения с хранилищами данных.
         * 
         * @see Connection
         */
        virtual ~Archiver();
    
        /**
         * Возвращает последний (максимальный) использованный индекс
         * для сообщений в хранилищах.
         * Используется при старте системы хранения.
         * 
         * @return последний (максимальный) использованный индекс
         * @exception StorageException
         *                   возникает в случае непредвиденных ошибок
         *                   при работе с хранилищами.
         */
        SMSId getLastUsedId()
            throw(StorageException); 

        /**
         * Увеличивает внутренний счётчик числа сообщений
         * в финальном состоянии.
         * 
         * @param count на сколько нужно увеличить счётчик, default=1
         */
        void incrementFinalizedCount(unsigned count=1);
        
        /**
         * Уменьшает внутренний счётчик числа сообщений
         * в финальном состоянии.
         * 
         * @param count на сколько нужно уменьшить счётчик, default=1
         */
        void decrementFinalizedCount(unsigned count=1);

        /**
         * Используется внутренне со стороны Thread
         * для создания и запуска выделенного потока управления
         * 
         * @return код завершения, не используется
         * @see Thread
         */
        virtual int Execute();
        
        /**
         * Запускает процесс архивации и создания биллинговых записей.
         * Если уже запущен, то ничего не поисходит.
         * Перед непосредственным запуском обновляет значение счётчика
         * числа сообщений в финальном состоянии.
         * 
         * @exception StorageException
         *                   возникает в случае непредвиденных ошибок
         *                   при работе с хранилищами.
         */
        void Start() throw(StorageException);
        
        /**
         * Останавливает процесс архивации и создания биллинговых записей.
         * Если ещё не запущен, то ничего не поисходит.
         * Ожидает завершения текущего минимального кванта работы.
         */
        void Stop();

        /**
         * @return Возвращает признак, запущена ли подсистема архивации
         *         и создания биллинговых записей в настоящее время.
         */
        inline bool isStarted() {
            return bStarted;
        }
        
        /**
         * @return Возвращает признак, работает ли реально процесс архивации
         *         и создания биллинговых записей в настоящее время.
         */
        inline bool isInProgress() {
            return job.isSignaled();
        }
    };

}};

#endif


