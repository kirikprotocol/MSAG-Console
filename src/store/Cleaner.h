#ifndef CLEANER_DECLARATIONS
#define CLEANER_DECLARATIONS

/**
 * Файл содержит описание подсистемы очистки архива
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
     * Подсистема очистки архива в контексте SMS центра.
     * Работает во взаимодействии с подсистемой хранения сообщений.
     * Проводит переодическую очистку архивированных сообщений в хранилище, 
     * 
     * Процесс очистки архива реализован
     * в виде отдельного потока управления в системе SMS центра.
     * Необходимые параметры задаются при кофигурировании системы.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see StoreManager
     */
    class Cleaner : public Thread
    {
    private:

        log4cpp::Category   &log;

        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock, cleanupLock;

        time_t      ageInterval, awakeInterval, cleanupInterval;

        char* storageDBInstance;
        char* storageDBUserName;
        char* storageDBUserPassword;
        
        static const char*  storageMaxIdSql;
        static const char*  archiveMaxIdSql;
        static const char*  billingMaxIdSql;
        
        Connection* cleanerConnection;
        Statement*  cleanerMinTimeStmt;
        Statement*  cleanerDeleteStmt;

        OCIDate     dbTime;
        sb2         indDbTime;

        char* loadDBInstance(Manager& config, const char* cat)
            throw(ConfigException);
        char* loadDBUserName(Manager& config, const char* cat)
            throw(ConfigException);
        char* loadDBUserPassword(Manager& config, const char* cat)
            throw(ConfigException);
        
        void loadCleanupInterval(Manager& config);
        void loadCleanupAgeInterval(Manager& config);
        void loadCleanupAwakeInterval(Manager& config);

        static const char* cleanerMinTimeSql;
        static const char* cleanerDeleteSql;
        void prepareCleanerMinTimeStmt() throw(StorageException);
        void prepareCleanerDeleteStmt() throw(StorageException);

        void cleanup() throw(StorageException);
        void connect() throw(StorageException);

        SMSId getMaxUsedId(Connection* connection, const char* sql)
            throw(StorageException);
    
    public:

        /**
         * Конструктор, создаёт (но не запускает) систему очистки хранилища
         * Извлекает набор конфигурационных параметров и создаёт 
         * соединение с хранилищем.
         * 
         * @param config интерфес для получения конфигурационных параметров
         * @exception ConfigException
         *                   возникает в случае некорректности и/или
         *                   неполноты набора конфигурационных параметров.
         * @see Connection
         * @see smsc::util::config::Manager
         */
        Cleaner(Manager& config) 
            throw(ConfigException); 
        /**
         * Деструктор, останавливает и уничтожает систему очистки хранилища.
         * Ожидает завершения текущего минимального кванта работы.
         * Также, уничтожает соединения с хранилищем данных.
         * 
         * @see Connection
         */
        virtual ~Cleaner();

        /**
         * Возвращает последний (максимальный) использованный индекс
         * для сообщений в хранилище.
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
         * Используется внутренне со стороны Thread
         * для создания и запуска выделенного потока управления
         * 
         * @return код завершения, не используется
         * @see Thread
         */
        virtual int Execute();
        
        /**
         * Запускает процесс очистки хранилища.
         * Если уже запущен, то ничего не поисходит.
         * 
         * @exception StorageException
         *                   возникает в случае непредвиденных ошибок
         *                   при работе с хранилищами.
         */
        void Start();
        /**
         * Останавливает процесс очистки.
         * Если ещё не запущен, то ничего не поисходит.
         * Ожидает завершения текущего минимального кванта работы.
         */
        void Stop();
        /**
         * @return Возвращает признак, запущена ли подсистема очистки
         * в настоящее время.
         */
        inline bool isStarted() {
            return bStarted;
        }
        /**
         * @return Возвращает признак, работает ли реально процесс очистки
         * в настоящее время.
         */
        inline bool isInProgress() {
            return awake.isSignaled();
        }
    };

}};

#endif // CLEANER_DECLARATIONS

