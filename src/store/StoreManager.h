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

#include "MessageStore.h"
#include "ConnectionManager.h"
#include "Archiver.h"

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
        
        class ReadyIdIterator : public IdIterator
        {
        private:

            Connection*                 connection;
            StorageConnectionPool*      pool;
            ReadyByNextTimeStatement*   readyStmt;

        public:

            ReadyIdIterator(StorageConnectionPool* pool, time_t retryTime)
                throw(StorageException);
            virtual ~ReadyIdIterator();

            virtual bool getNextId(SMSId &id)
                throw(StorageException);
        };

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

        RemoteStore(Manager& config)
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
        virtual void createSms(SMS& sms, SMSId id,
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
        virtual void destroySms(SMSId id)
                throw(StorageException, NoSuchMessageException);

        /**
         * ���������� ������ MessageStore
         * @see MessageStore
         */
        virtual void changeSmsStateToEnroute(SMSId id,
            const Descriptor& dst, uint32_t failureCause, time_t nextTryTime)
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
        virtual IdIterator* getReadyForRetry(time_t retryTime)
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
        static Archiver                 *archiver;
        static RemoteStore              *instance;
        
        static log4cpp::Category        &log;

    public:

        /**
         * ����� ������ � �������������� ���������� �������� ���������.
         * ������ ���� ������ ���� ���, ����� ���������������� ��������������
         * ����������. ��� ��������� ���������� ���������� �������
         * ��������������� ������� getMessageStore()
         *
         * @param config �������� ��� ��������� ���������������� ����������
         * @exception ConfigException
         *                   ��������� � ������ �������������� �/���
         *                   ��������� ������ ���������������� ����������.
         * @exception ConnectionFailedException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @see StoreManager::getMessageStore()
         * @see smsc::util::config::Manager
         */
        static void startup(Manager& config)
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
         * ��������� ������������� �������������� ���������� ��������� �
         * �������� ����������� �������.
         *
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @see Archiver
         */
        static void startArchiver()
            throw (StorageException) 
        {
            __require__(archiver);
            return archiver->Start();
        }
        /**
         * ��������� ������������� ���������� ���������� ��������� �
         * �������� ����������� �������.
         *
         * ������� ���������� �������� ������������ ������ ������ ����������.
         * @see Archiver
         */
        static void stopArchiver() {
            __require__(archiver);
            return archiver->Stop();
        }
        /**
         * @return ���������� �������, �������� �� ���������� ���������
         *         � �������� ����������� ������� � ��������� �����.
         * @see Archiver
         */
        static bool isArchiverStarted() {
            __require__(archiver);
            return archiver->isStarted();
        }
        /**
         * @return ���������� �������, �������� �� ������� ������� ���������
         *         � �������� ����������� ������� � ��������� �����.
         * @see Archiver
         */
        static bool isArchivationInProgress() {
            __require__(archiver);
            return archiver->isInProgress();
        }

        static void incrementFinalizedCount(unsigned count=1) {
            __require__(archiver);
            archiver->incrementFinalizedCount();
        }
        static void decrementFinalizedCount(unsigned count=1) {
            __require__(archiver);
            archiver->decrementFinalizedCount();
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

    class SmsCache
    {
    private:
    
        struct SMSIdIdx
        {
            static inline int CalcHash(const SMSId& id) {
                return (int)id;
            };
        };
        struct AddressIdx
        {
            Address oa, da; 
            
            AddressIdx(const Address& _oa, const Address& _da) 
                : oa(_oa), da(_da) {};
            AddressIdx(const AddressIdx& idx) 
                : oa(idx.oa), da(idx.da) {};

            inline int operator ==(const AddressIdx& idx) {
                return (oa == idx.oa && da == idx.da);
            };
            static inline int addressHash() {
                // Add real hash calculation code here !!!
                return 0; 
            };
        };
        struct ComplexStIdx : public AddressIdx
        {
            const char* st;

            ComplexStIdx(const Address& _oa, const Address& _da, 
                         const char* _st) 
                : AddressIdx(_oa, _da), st(_st) {};
            ComplexStIdx(const ComplexStIdx& idx) 
                : AddressIdx(idx), st(idx.st) {};
            
            inline int operator ==(const ComplexStIdx& idx) {
                return (AddressIdx::operator ==(idx) && 
                        ((!st && !idx.st) || 
                         (st && idx.st && strcmp(st, idx.st) == 0)));
            };
            static inline int CalcHash(const ComplexStIdx& idx) {
                // Add real hash calculation code here !!!
                return AddressIdx::addressHash()+(int)idx.st;
            };
        };
        struct ComplexMrIdx : public AddressIdx
        {
            uint16_t mr;

            ComplexMrIdx(const Address& _oa, const Address& _da, 
                         uint16_t _mr) 
                : AddressIdx(_oa, _da), mr(_mr) {};
            ComplexMrIdx(const ComplexMrIdx& idx) 
                : AddressIdx(idx), mr(idx.mr) {};
                
            inline int operator ==(const ComplexMrIdx& idx) {
                return (AddressIdx::operator ==(idx) && 
                        mr == idx.mr);
            };
            static inline int CalcHash(const ComplexMrIdx& idx) {
                // Add real hash calculation code here !!!
                return AddressIdx::addressHash()+idx.mr;
            };
        };

        XHash<SMSId, SMS*, SMSIdIdx>                idCache;
        XHash<ComplexMrIdx, SMS*, ComplexMrIdx>     mrCache;
        XHash<ComplexStIdx, SMS*, ComplexStIdx>     stCache;

    public:

        SmsCache(int capacity=0);
        virtual ~SmsCache();

        bool delSms(SMSId id);
        void putSms(SMSId id, SMS* sm);
        
        SMS* getSms(SMSId id);
        SMS* getSms(const Address& oa, const Address& da, 
                    const char* svc, SMSId& id);
        SMS* getSms(const Address& oa, const Address& da, 
                    uint16_t mr, SMSId& id);
    };

    class CachedStore : public RemoteStore
    {
    protected:

        SmsCache                cache;
        Mutex                   cacheMutex;

        StorageConnection*      connection;
    
    public:

        CachedStore(Manager& config) 
            throw(ConfigException, StorageException);
        virtual ~CachedStore();
        
        virtual void createSms(SMS& sms, SMSId id,
                               const CreateMode flag = CREATE_NEW)
                throw(StorageException, DuplicateMessageException);
        virtual void retriveSms(SMSId id, SMS &sms)
                throw(StorageException, NoSuchMessageException);
        virtual void replaceSms(SMSId id, const Address& oa,
            const uint8_t* newMsg, uint8_t newMsgLen,
            uint8_t deliveryReport, time_t validTime = 0, time_t waitTime = 0)
                throw(StorageException, NoSuchMessageException);
        virtual void destroySms(SMSId id)
                throw(StorageException, NoSuchMessageException);
        virtual void changeSmsStateToEnroute(SMSId id,
            const Descriptor& dst, uint32_t failureCause, time_t nextTryTime)
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
        virtual IdIterator* getReadyForRetry(time_t retryTime)
                throw(StorageException);
        virtual time_t getNextRetryTime()
                throw(StorageException);
    };

}}

#endif
