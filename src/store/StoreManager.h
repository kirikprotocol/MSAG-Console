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

#include <map>
#include <system/sched_timer.hpp>

#include <util/config/Manager.h>
#include <logger/Logger.h>

#include <core/synchronization/Mutex.hpp>
#include <core/threads/Thread.hpp>
#include <core/buffers/XHash.hpp>
#include <core/buffers/Array.hpp>

#include "MessageStore.h"
#include "ConnectionManager.h"
#include "FileStorage.h"

#undef SMSC_FAKE_MEMORY_MESSAGE_STORE
//#define SMSC_FAKE_MEMORY_MESSAGE_STORE

#ifdef SMSC_FAKE_MEMORY_MESSAGE_STORE
#include <core/buffers/IntHash.hpp>
    using namespace smsc::core::buffers;
#endif

namespace smsc { namespace store
{
    using namespace smsc::sms;
    using namespace smsc::core::synchronization;
    using namespace smsc::core::buffers;
    
    using smsc::logger::Logger;
    using smsc::core::threads::Thread;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;

    using smsc::system::SchedTimer;

    class RemoteStore : public MessageStore, public Thread
    {
    private:

    #ifdef SMSC_FAKE_MEMORY_MESSAGE_STORE
        static IntHash<SMS*>    fakeStore;
        static Mutex            fakeMutex;
    #endif

        smsc::logger::Logger *log;

        Mutex   startLock;
        Event   awake, exited;
        bool    bStarted, bNeedExit;
        
        Mutex       sequenceIdLock;
        SMSId       currentId, sequenceId;

        BillingStorage  billingStorage;
        ArchiveStorage  archiveStorage;
        
    protected:

        void loadMaxTriesCount(Manager& config);
        unsigned                maxTriesCount;

        StorageConnectionPool*  pool;
        SchedTimer*    scheduleTimer;

        class ReadyIdIterator : public TimeIdIterator
        {
        private:

            bool                        isNull;
            Connection*                 connection;
            ReadyByNextTimeStatement*   readyStmt;
            StorageConnectionPool*      pool;

        public:

            ReadyIdIterator(StorageConnectionPool* _pool, 
                            time_t retryTime, bool immediate=false)
                throw(StorageException);
            virtual ~ReadyIdIterator();

            virtual bool next()
                throw(StorageException);
            virtual SMSId  getId()
                throw(StorageException);
            virtual time_t getTime()
                throw(StorageException);
            virtual bool getDstSmeId(char* buffer) 
                throw(StorageException);
        };

        class CancelIdIterator : public IdIterator
        {
        private:

            bool                        isNull;
            Connection*                 connection;
            CancelIdsStatement*         cancelStmt;
            StorageConnectionPool*      pool;

        public:

            CancelIdIterator(StorageConnectionPool* _pool,
                const Address& oa, const Address& da, const char* svc=0)
                    throw(StorageException);
            virtual ~CancelIdIterator();

            virtual bool next()
                throw(StorageException);
            virtual SMSId  getId()
                throw(StorageException);
        };

        class DeliveryIdIterator : public IdIterator
        {
        private:

            bool                        isNull;
            Connection*                 connection;
            DeliveryIdsStatement*       deliveryStmt;
            StorageConnectionPool*      pool;

        public:

            DeliveryIdIterator(StorageConnectionPool* _pool,
                const Address& da)
                    throw(StorageException);
            virtual ~DeliveryIdIterator();

            virtual bool next()
                throw(StorageException);
            virtual SMSId  getId()
                throw(StorageException);
        };

        SMSId doCreateSms(StorageConnection* connection,
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
        void doReplaceSms(StorageConnection* connection,
                          SMSId id, SMS& sms)
                throw(StorageException, NoSuchMessageException);
        void doDestroySms(StorageConnection* connection, SMSId id)
                throw(StorageException, NoSuchMessageException);

        void doChangeSmsStateToEnroute(StorageConnection* connection,
            SMSId id, const Descriptor& dst,
            uint32_t failureCause, time_t nextTryTime, uint32_t attempts)
                throw(StorageException, NoSuchMessageException);
        void doFinalizeSms(SMSId id, SMS& sms, bool needDelete=true)
            throw(StorageException, NoSuchMessageException);
        
        void doChangeSmsConcatSequenceNumber(StorageConnection* connection,
                                             SMSId id, int8_t inc) 
                throw(StorageException, NoSuchMessageException); 
    public:

        RemoteStore(Manager& config, SchedTimer* sched)
            throw(ConfigException, StorageException);
        virtual ~RemoteStore();

        virtual int Execute();
        void Start();
        void Stop();
        
        void setPoolSize(unsigned size) {
            __require__(pool);
            pool->setSize(size);
        }
        unsigned getPoolSize() {
            __require__(pool);
            return pool->getSize();
        }
        unsigned getConnectionsCount() {
            __require__(pool);
            return pool->getConnectionsCount();
        }
        bool hasFreeConnections() {
            __require__(pool);
            return pool->hasFreeConnections();
        }
        unsigned getBusyConnectionsCount() {
            __require__(pool);
            return pool->getBusyConnectionsCount();
        }
        unsigned getIdleConnectionsCount() {
            __require__(pool);
            return pool->getIdleConnectionsCount();
        }
        unsigned getPendingQueueLength() {
            __require__(pool);
            return pool->getPendingQueueLength();
        }

        /**
         * Реализация метода MessageStore для внешней генерация ключа.
         *
         * @see MessageStore
         */
        virtual SMSId getNextId()
            throw(StorageException);

        /**
         * Реализация метода MessageStore
         * @see MessageStore
         */
        virtual SMSId createSms(SMS& sms, SMSId id,
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
        virtual void replaceSms(SMSId id, SMS& sms)
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
            const Descriptor& dst, uint32_t failureCause, 
                time_t nextTryTime, uint32_t attempts)
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
        virtual void createFinalizedSms(SMSId id, SMS& sms)
                throw(StorageException, DuplicateMessageException);
        /**
         * Реализация метода MessageStore
         * @see MessageStore
         */
        virtual void changeSmsConcatSequenceNumber(SMSId id, int8_t inc=1) 
                throw(StorageException, NoSuchMessageException); 

        /**
         * Реализация метода MessageStore
         * @see MessageStore
         */
        virtual int getConcatMessageReference(const Address& dda)
                throw(StorageException);
        
        /**
         * Реализация метода MessageStore
         * @see MessageStore
         */
        virtual IdIterator* getReadyForDelivery(const Address& da)
                throw(StorageException);
        /**
         * Реализация метода MessageStore
         * @see MessageStore
         */
        virtual IdIterator* getReadyForCancel(const Address& oa,
            const Address& da, const char* svcType = 0)
                throw(StorageException);
        /**
         * Реализация метода MessageStore
         * @see MessageStore
         */
        virtual TimeIdIterator* getReadyForRetry(time_t retryTime, bool immediate=false)
                throw(StorageException);

        /**
         * Реализация метода MessageStore
         * @see MessageStore
         */
        virtual time_t getNextRetryTime()
                throw(StorageException);

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
    class StoreManager
    {
    private:

        static Mutex mutex;

        static RemoteStore*             instance;
        static smsc::logger::Logger*    log;

        static bool needCache(Manager& config);

    public:

        /**
         * Метод создаёт и инициализирует подсистему хранения сообщений.
         * Должен быть вызван один раз, перед непосредственным использованием
         * подсистемы. Для получения интерфейса подсистемы следует
         * воспользоваться методом getMessageStore()
         *
         * @param config интерфес для получения конфигурационных параметров
         * @param sched интерфейс для изменения schedule time для sms 
         * @exception ConfigException
         *                   возникает в случае некорректности и/или
         *                   неполноты набора конфигурационных параметров.
         * @exception ConnectionFailedException
         *                   возникает при ошибке хранилища физической природы,
         *                   т.н когда хранилище недоступно.
         * @see StoreManager::getMessageStore()
         * @see smsc::util::config::Manager
         */
        static void startup(Manager& config, SchedTimer* sched)
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
        inline static MessageStore* getMessageStore() {
            return ((MessageStore *)instance);
        }

        inline static SMSId getNextId()
            throw(StorageException)
        {
            __require__(instance);
            return instance->getNextId();
        };

        /**
         * Меняет размер пула соединений с хранилищем.
         * В один момент времени одно соединение может использоваться
         * только одним потоком управления.
         *
         * @param size   новый размер пула соединений
         * @see ConnectionPool
         */
        static void setPoolSize(unsigned size) {
            __require__(instance);
            instance->setPoolSize(size);
        }
        /**
         * @return текущий размер пула соединений
         * @see ConnectionPool
         */
        static unsigned getPoolSize() {
            __require__(instance);
            return instance->getPoolSize();
        }
        /**
         * @return текущее количество соединений
         * @see ConnectionPool
         */
        static unsigned getConnectionsCount() {
            __require__(instance);
            return instance->getConnectionsCount();
        }
        /**
         * @return признак, есть ли свободные соединения
         * @see ConnectionPool
         */
        static bool hasFreeConnections() {
            __require__(instance);
            return instance->hasFreeConnections();
        }
        /**
         * @return текущее количество занятых соединений
         * @see ConnectionPool
         */
        static unsigned getBusyConnectionsCount() {
            __require__(instance);
            return instance->getBusyConnectionsCount();
        }
        /**
         * @return текущее количество простаивающих соединений
         * @see ConnectionPool
         */
        static unsigned getIdleConnectionsCount() {
            __require__(instance);
            return instance->getIdleConnectionsCount();
        }
        /**
         * @return текущее количество запросов ожидающих обработку
         * @see ConnectionPool
         */
        static unsigned getPendingQueueLength() {
            __require__(instance);
            return instance->getPendingQueueLength();
        }

    };

    struct UpdateRecord
    {
        SMSId       id;
        State       state;
        Descriptor  dst;
        uint32_t    fcs;
        time_t      nt;

        UpdateRecord(SMSId _id, State _state)
            : id(_id), state(_state), fcs(0), nt(0) {};
        UpdateRecord(SMSId _id, State _state, const Descriptor& _dst,
                     uint32_t _fcs = 0, time_t _nt = 0)
            : id(_id), state(_state), dst(_dst), fcs(_fcs), nt(_nt) {};
    };

    const int SMSC_MAX_SMS_CACHE_CAPACITY = 10000;

    class SmsCache
    {
    private:

        struct SMSIdIdx
        {
            static inline unsigned int CalcHash(const SMSId& id) {
                return (unsigned int)id;
            };
        };

        SMSId                        lastId;
        XHash<SMSId, SMS*, SMSIdIdx> idCache;
        unsigned                     cacheCapacity;

    public:

        SmsCache(unsigned capacity=SMSC_MAX_SMS_CACHE_CAPACITY,
                 unsigned initsize=SMSC_MAX_SMS_CACHE_CAPACITY);
        virtual ~SmsCache();

        void clean();
        bool delSms(SMSId id);
        void putSms(SMSId id, SMS* sm);
        SMS* getSms(SMSId id);
    };

    class CachedStore : public RemoteStore
    {
    protected:

        smsc::logger::Logger *log;

        Mutex       cacheMutex;
        SmsCache*   cache;

        int         maxCacheCapacity;
        void loadMaxCacheCapacity(Manager& config);

    public:

        CachedStore(Manager& config, SchedTimer* sched)
            throw(ConfigException, StorageException);
        virtual ~CachedStore();

        virtual SMSId createSms(SMS& sms, SMSId id,
            const CreateMode flag = CREATE_NEW)
                throw(StorageException, DuplicateMessageException);
        virtual void retriveSms(SMSId id, SMS &sms)
                throw(StorageException, NoSuchMessageException);
        virtual void replaceSms(SMSId id, const Address& oa,
            const uint8_t* newMsg, uint8_t newMsgLen,
            uint8_t deliveryReport, time_t validTime = 0, time_t waitTime = 0)
                throw(StorageException, NoSuchMessageException);
        virtual void replaceSms(SMSId id, SMS& sms)
            throw(StorageException, NoSuchMessageException); 
        virtual void destroySms(SMSId id)
                throw(StorageException, NoSuchMessageException);
        virtual void changeSmsStateToEnroute(SMSId id,
            const Descriptor& dst, uint32_t failureCause, 
                time_t nextTryTime, uint32_t attempts)
                throw(StorageException, NoSuchMessageException);
        virtual void changeSmsStateToDelivered(SMSId id,
            const Descriptor& dst)
                throw(StorageException, NoSuchMessageException);
        virtual void changeSmsStateToUndeliverable(SMSId id,
            const Descriptor& dst, uint32_t failureCause)
                throw(StorageException, NoSuchMessageException);
        virtual void changeSmsStateToExpired(SMSId id)
                throw(StorageException, NoSuchMessageException);
        virtual void changeSmsStateToDeleted(SMSId id)
                throw(StorageException, NoSuchMessageException);
        virtual void changeSmsConcatSequenceNumber(SMSId id, int8_t inc=1) 
                throw(StorageException, NoSuchMessageException); 

    };

}}

#endif
