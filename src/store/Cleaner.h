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
     * Подсистема работы с архивными таблицами в контексте SMS центра.
     * Работает во взаимодействии с подсистемой хранения сообщений.
     * 
     * Процесс работы с архивными таблицами реализован
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

        smsc::logger::Logger *log;

        Event       awake, exited;
        bool        bStarted, bNeedExit;
        Mutex       idLock, startLock, cleanupLock;

        time_t      ageInterval, awakeInterval, cleanupInterval;

        char* storageDBInstance;
        char* storageDBUserName;
        char* storageDBUserPassword;
        
        Connection*      cleanerConnection;
        Statement*       cleanerMinTimeStmt;
        Statement*       cleanerDeleteStmt;
        GetIdStatement*  cleanerNextIdStmt;

        OCIDate     dbTime;
        sb2         indDbTime;

        SMSId       currentId, sequenceId;

        char* loadDBInstance(Manager& config, const char* cat)
            throw(ConfigException);
        char* loadDBUserName(Manager& config, const char* cat)
            throw(ConfigException);
        char* loadDBUserPassword(Manager& config, const char* cat)
            throw(ConfigException);
        
        void loadCleanupInterval(Manager& config);
        void loadCleanupAgeInterval(Manager& config);
        void loadCleanupAwakeInterval(Manager& config);

        void prepareCleanerNextIdStmt() throw(StorageException);
        void prepareCleanerMinTimeStmt() throw(StorageException);
        void prepareCleanerDeleteStmt() throw(StorageException);

        void cleanup() throw(StorageException);
        void connect() throw(StorageException);

    public:

        /**
         * Конструктор, создаёт (но не запускает) систему работы с архивными таблицами
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
         * Деструктор, останавливает и уничтожает систему работы с архивными таблицами.
         * Ожидает завершения текущего минимального кванта работы.
         * Также, уничтожает соединения с хранилищем данных.
         * 
         * @see Connection
         */
        virtual ~Cleaner();

        /**
         * Возвращает следующий индекс для создания сообщений в хранилище.
         * 
         * @return следующий индекс
         * @exception StorageException
         *                   возникает в случае непредвиденных ошибок
         *                   при работе с хранилищами.
         */
        SMSId getNextId()
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
         * Запускает процесс работы с архивными таблицами.
         * Если уже запущен, то ничего не поисходит.
         * 
         * @exception StorageException
         *                   возникает в случае непредвиденных ошибок
         *                   при работе с хранилищами.
         */
        void Start();
        
        /**
         * Останавливает процесс работы с архивными таблицами.
         * Если ещё не запущен, то ничего не поисходит.
         * Ожидает завершения текущего минимального кванта работы.
         */
        void Stop();
        
        /**
         * @return признак, запущена ли система в настоящее время.
         */
        inline bool isStarted() {
            return bStarted;
        }
        
        /**
         * @return признак, происходит ли реальная работа в настоящее время.
         */
        inline bool isInProgress() {
            return awake.isSignaled();
        }

    };

}};

#endif // CLEANER_DECLARATIONS

