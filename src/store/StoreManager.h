#ifndef STORE_MANAGER_DECLARATIONS
#define STORE_MANAGER_DECLARATIONS

/**
 * ���� �������� �������� ���������� ���������� �������� ���������
 * � ��������� SMS ������.
 * ��������� ��������� ������� �������� MessageStore.
 * ���������� �������� �� �������������� � ����������� ��������� �
 * �������� ����������� �������.
 *
 * ���������� ����������� �� ���� ������� ���� Oracle �
 * � �������������� ������� ���������� Oracle Call Interface.
 *
 * @author Victor V. Makarov
 * @version 1.0
 * @see MessageStore
 * @see SMS
 */

#include <core/synchronization/Mutex.hpp>
#include <util/config/Manager.h>
#include <util/Logger.h>

#include <core/buffers/XHash.hpp>
#include <core/buffers/Array.hpp>
#include <map>

#include <system/sched_timer.hpp>

#include "MessageStore.h"
#include "ConnectionManager.h"
#include "Cleaner.h"

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
    
    using smsc::util::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;

    using smsc::system::SchedTimer;

    class RemoteStore : public MessageStore
    {
    private:

    #ifdef SMSC_FAKE_MEMORY_MESSAGE_STORE
        static IntHash<SMS*>    fakeStore;
        static Mutex            fakeMutex;
    #endif

        static log4cpp::Category    &log;

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
            uint32_t failureCause, time_t nextTryTime, bool skipAttempt=false)
                throw(StorageException, NoSuchMessageException);
        void doFinalizeSms(SMSId id, SMS& sms)
            throw(StorageException);
        
        void doChangeSmsConcatSequenceNumber(StorageConnection* connection,
                                             SMSId id, int8_t inc) 
                throw(StorageException, NoSuchMessageException); 

    public:

        RemoteStore(Manager& config, SchedTimer* sched)
            throw(ConfigException, StorageException);
        virtual ~RemoteStore();

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
         * ���������� ������ MessageStore ��� ������� ��������� �����.
         *
         * @see MessageStore
         * @see IdGenerator
         */
        virtual SMSId getNextId();

        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual SMSId createSms(SMS& sms, SMSId id,
            const CreateMode flag = CREATE_NEW)
                throw(StorageException, DuplicateMessageException);
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void retriveSms(SMSId id, SMS &sms)
                throw(StorageException, NoSuchMessageException);
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void replaceSms(SMSId id, const Address& oa,
            const uint8_t* newMsg, uint8_t newMsgLen,
            uint8_t deliveryReport, time_t validTime = 0, time_t waitTime = 0)
                throw(StorageException, NoSuchMessageException);
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void replaceSms(SMSId id, SMS& sms)
                throw(StorageException, NoSuchMessageException); 
        
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void destroySms(SMSId id)
                throw(StorageException, NoSuchMessageException);

        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void changeSmsStateToEnroute(SMSId id,
            const Descriptor& dst, uint32_t failureCause, time_t nextTryTime,
                bool skipAttempt=false)
                    throw(StorageException, NoSuchMessageException);
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void changeSmsStateToDelivered(SMSId id,
            const Descriptor& dst)
                throw(StorageException, NoSuchMessageException);
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void changeSmsStateToUndeliverable(SMSId id,
            const Descriptor& dst, uint32_t failureCause)
                throw(StorageException, NoSuchMessageException);
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void changeSmsStateToExpired(SMSId id)
                throw(StorageException, NoSuchMessageException);
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void changeSmsStateToDeleted(SMSId id)
                throw(StorageException, NoSuchMessageException);
        
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void changeSmsConcatSequenceNumber(SMSId id, int8_t inc=1) 
                throw(StorageException, NoSuchMessageException); 

        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual int getConcatMessageReference(const Address& dda)
                throw(StorageException);
        
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual IdIterator* getReadyForDelivery(const Address& da)
                throw(StorageException);
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual IdIterator* getReadyForCancel(const Address& oa,
            const Address& da, const char* svcType = 0)
                throw(StorageException);
        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual TimeIdIterator* getReadyForRetry(time_t retryTime, bool immediate=false)
                throw(StorageException);

        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual time_t getNextRetryTime()
                throw(StorageException);

    };

    /**
     * ��������� ����� ������������ ��� ��������� ����������
     * ������������������ ������ ��� ���������.
     * ������������ ���������.
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
         * �����������, ������ ��������� �����
         *
         * @param _id    ��������� �������������� �����
         *               ��� ������������� ���������
         */
        IDGenerator(SMSId _id) : id(_id) {};
        /**
         * ������ ����������
         */
        ~IDGenerator() {};
        /**
         *
         * @return ���������� ��������� ����������������� ������ ���������.
         */
        inline SMSId getNextId() {
            MutexGuard guard(mutex);
            return ++id;
        };
    };

    /**
     * ����� ��������� ���������� �������� ��������� � ��������� SMS ������.
     * � ������, ��������� ��������� MessageStore ��������� � ������������
     * ����������.��������� � ���� ������� � ���������� ������������.
     *
     * ����� �������� ����� ����������� ������� ��� ��� �����������
     * ������ ����������, ��� � ��� ��������� ��������� ����������.
     *
     * ����� ����, ���������� ������������ ���������� ��������� �
     * �������� ����������� �������.
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

        static IDGenerator              *generator;
        static Cleaner                  *cleaner;
        static RemoteStore              *instance;

        static log4cpp::Category        &log;

        static bool needCache(Manager& config);
        static bool needCleaner(Manager& config);

    public:

        /**
         * ����� ������ � �������������� ���������� �������� ���������.
         * ������ ���� ������ ���� ���, ����� ���������������� ��������������
         * ����������. ��� ��������� ���������� ���������� �������
         * ��������������� ������� getMessageStore()
         *
         * @param config �������� ��� ��������� ���������������� ����������
         * @param sched ��������� ��� ��������� schedule time ��� sms 
         * @exception ConfigException
         *                   ��������� � ������ �������������� �/���
         *                   ��������� ������ ���������������� ����������.
         * @exception ConnectionFailedException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @see StoreManager::getMessageStore()
         * @see smsc::util::config::Manager
         */
        static void startup(Manager& config, SchedTimer* sched)
            throw(ConfigException, ConnectionFailedException);
        /**
         * ����� ������������� � ���������� ���������� �������� ���������.
         * ������ ���� ������ ���� ���, ��� ���������� ������.
         *
         * ������� ���������� �������� ������ ������, ���� ������� �
         * ���������� ��� ��������� � �������� ���������.
         */
        static void shutdown();

        /**
         * ������ ���������� ������ ����� ��������� ������ ������ startup()
         *
         * @return ���������� ��������� ��� ���������������� ������
         *         � ����������� �������� ���������
         * @see StoreManager::startup()
         * @see MessageStore
         */
        static MessageStore* getMessageStore() {
            return ((MessageStore *)instance);
        }

        static SMSId getNextId()
        {
            __require__(generator);
            return generator->getNextId();
        };

        /**
         * ������ ������ ���� ���������� � ����������.
         * � ���� ������ ������� ���� ���������� ����� ��������������
         * ������ ����� ������� ����������.
         *
         * @param size   ����� ������ ���� ����������
         * @see ConnectionPool
         */
        static void setPoolSize(unsigned size) {
            __require__(instance);
            instance->setPoolSize(size);
        }
        /**
         * @return ������� ������ ���� ����������
         * @see ConnectionPool
         */
        static unsigned getPoolSize() {
            __require__(instance);
            return instance->getPoolSize();
        }
        /**
         * @return ������� ���������� ����������
         * @see ConnectionPool
         */
        static unsigned getConnectionsCount() {
            __require__(instance);
            return instance->getConnectionsCount();
        }
        /**
         * @return �������, ���� �� ��������� ����������
         * @see ConnectionPool
         */
        static bool hasFreeConnections() {
            __require__(instance);
            return instance->hasFreeConnections();
        }
        /**
         * @return ������� ���������� ������� ����������
         * @see ConnectionPool
         */
        static unsigned getBusyConnectionsCount() {
            __require__(instance);
            return instance->getBusyConnectionsCount();
        }
        /**
         * @return ������� ���������� ������������� ����������
         * @see ConnectionPool
         */
        static unsigned getIdleConnectionsCount() {
            __require__(instance);
            return instance->getIdleConnectionsCount();
        }
        /**
         * @return ������� ���������� �������� ��������� ���������
         * @see ConnectionPool
         */
        static unsigned getPendingQueueLength() {
            __require__(instance);
            return instance->getPendingQueueLength();
        }

        /**
         * ��������� ������������� �������������� ���������� �������
         *
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @see Cleaner
         */
        static void startCleaner()
            throw (StorageException)
        {
            __require__(cleaner);
            cleaner->Start();
        }
        /**
         * ��������� ������������� ���������� ���������� �������
         *
         * ������� ���������� �������� ������������ ������ ������.
         * @see Cleaner
         */
        static void stopCleaner() {
            __require__(cleaner);
            cleaner->Stop();
        }
        /**
         * @return ���������� �������, �������� �� ���������� �������
         *
         * @see Cleaner
         */
        static bool isCleanerStarted() {
            __require__(cleaner);
            return cleaner->isStarted();
        }
        /**
         * @return ���������� �������, �������� �� ������� ������� �������
         *
         * @see Cleaner
         */
        static bool isCleaningInProgress() {
            __require__(cleaner);
            return cleaner->isInProgress();
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

        Mutex       cacheMutex;
        SmsCache*   cache;

        int         maxCacheCapacity;
        void loadMaxCacheCapacity(Manager& config);

        static log4cpp::Category    &log;

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
            const Descriptor& dst, uint32_t failureCause, time_t nextTryTime,
                bool skipAttempt=false)
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
