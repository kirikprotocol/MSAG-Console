#ifndef STORE_MANAGER_DECLARATIONS
#define STORE_MANAGER_DECLARATIONS

/**
 * Файл содержит описание реализации подсистемы хранения сообщений
 * в контексте SMS центра.
 * Реализует интерфейс системы хранения MessageStore.
 * Подсистема работает во взаимодействии с подсистемой архивации и
 * создания биллинговых записей.
 *
 * Подсистема реализована на базе средств СУБД Oracle и 
 * с использованием средств разработки Oracle Call Interface.
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see MessageStore
 * @see SMS
 */

#include <core/synchronization/Mutex.hpp>
#include <util/config/Manager.h>
#include <util/Logger.h>

#include "MessageStore.h"
#include "ConnectionManager.h"
#include "Archiver.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;
    using namespace smsc::core::synchronization;
    using smsc::util::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;
    
    /**
     * Сервисный класс используемый для генерации следующего 
     * идентификационного номера для сообщения.
     * Используется внутренне.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     */
    class IDGenerator 
    {
    private:
        
        SMSId   id;
        Mutex mutex;

    public:

        /**
         * Конструктор, создаёт сервисный класс
         * 
         * @param _id    последний использованный номер
         *               для идентификации сообщений
         */
        IDGenerator(SMSId _id) : id(_id) {};
        
        /**
         * Пустой деструктор
         */
        ~IDGenerator() {};
        
        /**
         * 
         * @return возвращает следующий идентификационный номера сообщения.
         */
        inline SMSId getNextId() 
        {
            MutexGuard guard(mutex);
            return ++id;
        };
    };

    /**
     * Класс реализует подсистему хранения сообщений в контексте SMS центра.
     * А именно, реализует интерфейс MessageStore доступный в единственном 
     * экземпляре.Выступает в роли фабрики и синглетона одновременно.
     *
     * Также содержит набор статических методов как для мониторинга
     * работы подсистемы, так и для изменения некоторых параметров.
     * 
     * Кроме того, подсистема контролирует подсистему архивации и
     * создания биллинговых записей.
     *
     * @author Victor V. Makarov
     * @version 1.0
     * @see MessageStore
     * @see SMS
     */
    class StoreManager : public MessageStore
    {
    private:
        
        static Mutex mutex;

        static IDGenerator              *generator;
        static Archiver                 *archiver;
        static StoreManager             *instance;
        static StorageConnectionPool    *pool;
        static log4cpp::Category        &log;
        
        static unsigned             maxTriesCount;
        static void loadMaxTriesCount(Manager& config);

        class ReadyIdIterator : public IdIterator
        {
        private:
            
            Connection*                 connection;
            ReadyByNextTimeStatement*   readyStmt;
        
        public:
            
            ReadyIdIterator(time_t retryTime)
                throw(StorageException);
            virtual ~ReadyIdIterator();

            virtual bool getNextId(SMSId &id) 
                throw(StorageException);
        };

    protected:
        
        StoreManager() : MessageStore() {};
        virtual ~StoreManager() {};
        
        void doCreateSms(StorageConnection* connection,
            SMS& sms, SMSId id, const CreateMode flag)
                throw(StorageException, DuplicateMessageException);
        void doRetrieveSms(StorageConnection* connection, 
            SMSId id, SMS& sms)
                throw(StorageException, NoSuchMessageException);
        void doReplaceSms(StorageConnection* connection, 
            SMSId id, const Address& oa, 
            const uint8_t* newMsg, uint8_t newMsgLen,
            uint8_t deliveryReport, time_t validTime = 0, time_t waitTime = 0)
                throw(StorageException, NoSuchMessageException);
        void doDestroySms(StorageConnection* connection, SMSId id) 
                throw(StorageException, NoSuchMessageException);
        
        void doChangeSmsStateToEnroute(StorageConnection* connection,
            SMSId id, const Descriptor& dst,
            uint32_t failureCause, time_t nextTryTime)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToDelivered(StorageConnection* connection, 
            SMSId id, const Descriptor& dst)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToUndeliverable(StorageConnection* connection, 
            SMSId id, const Descriptor& dst, uint32_t failureCause)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToExpired(StorageConnection* connection, 
            SMSId id)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToDeleted(StorageConnection* connection,
            SMSId id) 
                throw(StorageException, NoSuchMessageException);
    
    public:    
        
        /**
         * Метод создаёт и инициализирует подсистему хранения сообщений.
         * Должен быть вызван один раз, перед непосредственным использованием
         * подсистемы. Для получения интерфейса подсистемы следует
         * воспользоваться методом getMessageStore()
         * 
         * @param config интерфес для получения конфигурационных параметров
         * @exception ConfigException
         *                   возникает в случае некорректности и/или
         *                   неполноты набора конфигурационных параметров.
         * @exception ConnectionFailedException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @see StoreManager::getMessageStore()
         * @see smsc::util::config::Manager
         */
        static void startup(Manager& config)
            throw(ConfigException, ConnectionFailedException);
        /**
         * Метод останавливает и уничтожает подсистему хранения сообщений.
         * Должен быть вызван один раз, при завершении работы.
         *
         * Ожидает завершения текущего кванта работы, если запросы к
         * подсистеме ещё находятся в процессе обработки.
         */
        static void shutdown(); 

        /**
         * Должен вызываться только после успешного вызова метода startup()
         * 
         * @return возвращает интерфейс для непосредственной работы
         *         с подсистемой хранения сообщений
         * @see StoreManager::startup()
         * @see MessageStore
         */
        static MessageStore* getMessageStore() {
            return ((MessageStore *)instance);
        }
        
        /**
         * Меняет размер пула соединений с хранилищем.
         * В один момент времени одно соединение может использоваться
         * только одним потоком управления.
         * 
         * @param size   новый размер пула соединений
         * @see ConnectionPool
         */
        static void setPoolSize(unsigned size) {
            __require__(pool);
            pool->setSize(size);
        }
        /**
         * @return текущий размер пула соединений
         * @see ConnectionPool
         */
        static unsigned getPoolSize() {
            __require__(pool);
            return pool->getSize();
        }
        
        /**
         * @return текущее количество соединений
         * @see ConnectionPool
         */
        static unsigned getConnectionsCount() {
            __require__(pool);
            return pool->getConnectionsCount();
        }
        /**
         * @return признак, есть ли свободные соединения
         * @see ConnectionPool
         */
        static bool hasFreeConnections() {
            __require__(pool);
            return pool->hasFreeConnections();
        }
        /**
         * @return текущее количество занятых соединений
         * @see ConnectionPool
         */
        static unsigned getBusyConnectionsCount() {
            __require__(pool);
            return pool->getBusyConnectionsCount();
        }
        /**
         * @return текущее количество простаивающих соединений
         * @see ConnectionPool
         */
        static unsigned getIdleConnectionsCount() {
            __require__(pool);
            return pool->getIdleConnectionsCount();
        }
        /**
         * @return текущее количество запросов ожидающих обработку
         * @see ConnectionPool
         */
        static unsigned getPendingQueueLength() {
            __require__(pool);
            return pool->getPendingQueueLength();
        }
        
        /**
         * Позволяет принудительно активизировать подсистему архивации и
         * создания биллинговых записей.
         * 
         * @exception StorageException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @see Archiver
         */
        static void startArchiver() 
            throw (StorageException)
        {
            __require__(archiver);
            return archiver->Start();
        }
        /**
         * Позволяет принудительно остановить подсистему архивации и
         * создания биллинговых записей.
         *
         * Ожидает завершения текущего минимального кванта работы архиватора.
         * @see Archiver
         */
        static void stopArchiver() {
            __require__(archiver);
            return archiver->Stop();
        }
        /**
         * @return Возвращает признак, запущена ли подсистема архивации
         *         и создания биллинговых записей в настоящее время.
         * @see Archiver
         */
        static bool isArchiverStarted() {
            __require__(archiver);
            return archiver->isStarted();
        }
        /**
         * @return Возвращает признак, работает ли реально процесс архивации
         *         и создания биллинговых записей в настоящее время.
         * @see Archiver
         */
        static bool isArchivationInProgress() {
            __require__(archiver);
            return archiver->isInProgress();
        }

        /**
         * Реализация метода MessageStore для внешней генерация ключа.
         *
         * @see MessageStore 
         * @see IdGenerator
         */
        virtual SMSId getNextId() 
        {
            __require__(generator);
            return generator->getNextId();
        };

        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual void createSms(SMS& sms, SMSId id,
                               const CreateMode flag = CREATE_NEW)
                throw(StorageException, DuplicateMessageException);
        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual void retriveSms(SMSId id, SMS &sms)
                throw(StorageException, NoSuchMessageException);
        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual void replaceSms(SMSId id, const Address& oa,
            const uint8_t* newMsg, uint8_t newMsgLen,
            uint8_t deliveryReport, time_t validTime = 0, time_t waitTime = 0)
                throw(StorageException, NoSuchMessageException); 
        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual void destroySms(SMSId id) 
                throw(StorageException, NoSuchMessageException);
        
        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual void changeSmsStateToEnroute(SMSId id,
            const Descriptor& dst, uint32_t failureCause, time_t nextTryTime) 
                throw(StorageException, NoSuchMessageException); 
        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual void changeSmsStateToDelivered(SMSId id, 
            const Descriptor& dst) 
                throw(StorageException, NoSuchMessageException); 
        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual void changeSmsStateToUndeliverable(SMSId id,
            const Descriptor& dst, uint32_t failureCause) 
                throw(StorageException, NoSuchMessageException); 
        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual void changeSmsStateToExpired(SMSId id) 
                throw(StorageException, NoSuchMessageException); 
        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual void changeSmsStateToDeleted(SMSId id) 
                throw(StorageException, NoSuchMessageException); 
       
        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual IdIterator* getReadyForRetry(time_t retryTime) 
                throw(StorageException);
        
        /**
         * Реализация метода MessageStore
         * @see MessageStore 
         */
        virtual time_t getNextRetryTime() 
                throw(StorageException);
    };

}}

#endif

