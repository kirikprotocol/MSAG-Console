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

        static bool needCache(Manager& config);
        static bool needArchiver(Manager& config);

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
        State       state;
        Descriptor  dst;
        uint32_t    fcs;
        time_t      nt;

        UpdateRecord(State _state) 
            : state(_state), fcs(0), nt(0) {};
        UpdateRecord(State _state, const Descriptor& _dst,
                     uint32_t _fcs = 0, time_t _nt = 0)
            : state(_state), dst(_dst), fcs(_fcs), nt(_nt) {};
    };

    struct IdxSMS : public SMS
    {
        SMSId id;

        IdxSMS(SMSId _id, const SMS& sm) : SMS(sm), id(_id) {};
        virtual ~IdxSMS() {};
    };

    class SmsCache
    {
    private:
    
        struct SMSIdIdx
        {
            static inline unsigned int CalcHash(const SMSId& id) {
                return (unsigned int)id;
            };
        };
        struct AddressIdx
        {
            Address oa, da; 
            
            AddressIdx(const Address& _oa, const Address& _da) 
                : oa(_oa), da(_da) {};
            AddressIdx(const AddressIdx& idx) 
                : oa(idx.oa), da(idx.da) {};

            static unsigned getStrHash(const char* key)
            {
                char* curr = (char *)key;
                unsigned count = *curr;
                while(*curr) {
                  count += 37 * count + *curr; curr++;
                }
                count=(unsigned)(( ( count * (unsigned)19L ) + 
                                   (unsigned)12451L ) % (unsigned)8882693L);
                return count;
            };
            
            inline AddressIdx& operator =(const AddressIdx& idx) {
                oa = idx.oa; da = idx.da;
                return (*this);
            };
            inline int operator ==(const AddressIdx& idx) {
                return (oa == idx.oa && da == idx.da);
            };
        };
        struct ComplexStIdx : public AddressIdx
        {
            EService st;

            void setSt(const char* _st) {
                st[0] = '\0';
                if (_st) strncpy(st, _st, sizeof(st)-1);
            };

            ComplexStIdx(const Address& _oa, const Address& _da, 
                         const char* _st) 
                : AddressIdx(_oa, _da) { setSt(_st); };
            ComplexStIdx(const ComplexStIdx& idx) 
                : AddressIdx(idx) { setSt(idx.st); };
            
            inline ComplexStIdx& operator =(const ComplexStIdx& idx) {
                AddressIdx::operator =(idx); setSt(idx.st);
                return (*this);
            };
            inline int operator ==(const ComplexStIdx& idx) {
                return (AddressIdx::operator ==(idx) && 
                        strcmp(st, idx.st) == 0);
            };
            static inline unsigned int CalcHash(const ComplexStIdx& idx) {
                char buff[128];
                sprintf(buff, ".%d.%d.%s+.%d.%d.%s+%s", 
                        idx.oa.type, idx.oa.plan, idx.oa.value,
                        idx.da.type, idx.da.plan, idx.da.value, idx.st);
                return AddressIdx::getStrHash(buff);
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
                
            inline ComplexMrIdx& operator =(const ComplexMrIdx& idx) {
                AddressIdx::operator =(idx); mr = idx.mr;
                return (*this);
            };
            inline int operator ==(const ComplexMrIdx& idx) {
                return (AddressIdx::operator ==(idx) && 
                        mr == idx.mr);
            };
            static inline unsigned int CalcHash(const ComplexMrIdx& idx) {
                char buff[128];
                sprintf(buff, ".%d.%d.%s+.%d.%d.%s+%d", 
                        idx.oa.type, idx.oa.plan, idx.oa.value,
                        idx.da.type, idx.da.plan, idx.da.value, idx.mr);
                return AddressIdx::getStrHash(buff);
            };
        };

        XHash<SMSId,        IdxSMS*, SMSIdIdx>      idCache;
        XHash<ComplexMrIdx, IdxSMS*, ComplexMrIdx>  mrCache;
        XHash<ComplexStIdx, IdxSMS*, ComplexStIdx>  stCache;

    public:

        SmsCache(int capacity=0);
        virtual ~SmsCache();

        bool delSms(SMSId id);
        void putSms(IdxSMS* sm);
        
        SMS* getSms(SMSId id);
        SMS* getSms(const Address& oa, const Address& da, 
                    const char* svc, SMSId& id);
        SMS* getSms(const Address& oa, const Address& da, 
                    uint16_t mr, SMSId& id);
    };

    typedef std::multimap<SMSId, UpdateRecord*> UpdatesIdMap;

    class CachedStore : public RemoteStore, public Thread
    {
    protected:

        SmsCache        cache;
        Mutex           cacheMutex;

        UpdatesIdMap    updates;
        
        int maxUncommitedCount, maxSleepInterval;
        void loadMaxUncommitedCount(Manager& config);
        void loadSleepInterval(Manager& config); 

        Mutex   startLock;
        bool    bStarted, bNeedExit;
        Event   processEvent, exitedEvent;

        static log4cpp::Category    &log;
        
        inline SMS* _retriveSms(SMSId id)
            throw(StorageException, NoSuchMessageException);
        
        void addUpdate(SMSId id, UpdateRecord* update)
            throw(StorageException, NoSuchMessageException);
        void actualizeUpdate(SMSId id, UpdateRecord* update)
            throw(StorageException, NoSuchMessageException);
        void processUpdate(StorageConnection* connection,
                           SMSId id, UpdateRecord* update)
            throw(StorageException, NoSuchMessageException);
        bool delUpdates(SMSId id);
        
        void processUpdates(SMSId forId = 0);

    public:

        CachedStore(Manager& config) 
            throw(ConfigException, StorageException);
        virtual ~CachedStore();

        virtual int Execute();
        void Start();
        void Stop();
        
        virtual SMSId createSms(SMS& sms, SMSId id,
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
