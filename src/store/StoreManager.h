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
        inline SMSId getNextId() 
        {
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
        void doRetriveSms(StorageConnection* connection, 
            SMSId id, SMS& sms)
                throw(StorageException, NoSuchMessageException);
        void doReplaceSms(StorageConnection* connection, 
            SMSId id, const Address& oa, 
            const Body& newBody, uint8_t deliveryReport,
            time_t validTime = 0, time_t waitTime = 0)
                throw(StorageException, NoSuchMessageException);
        void doDestroySms(StorageConnection* connection, SMSId id) 
                throw(StorageException, NoSuchMessageException);
        
        void doChangeSmsStateToEnroute(StorageConnection* connection,
            SMSId id, const Descriptor& dst,
            uint8_t failureCause, time_t nextTryTime)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToDelivered(StorageConnection* connection, 
            SMSId id, const Descriptor& dst)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToUndeliverable(StorageConnection* connection, 
            SMSId id, const Descriptor& dst, uint8_t failureCause)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToExpired(StorageConnection* connection, 
            SMSId id)
                throw(StorageException, NoSuchMessageException);
        void doChangeSmsStateToDeleted(StorageConnection* connection,
            SMSId id) 
                throw(StorageException, NoSuchMessageException);
    
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
        
        /**
         * ������ ������ ���� ���������� � ����������.
         * � ���� ������ ������� ���� ���������� ����� ��������������
         * ������ ����� ������� ����������.
         * 
         * @param size   ����� ������ ���� ����������
         * @see ConnectionPool
         */
        static void setPoolSize(unsigned size) {
            __require__(pool);
            pool->setSize(size);
        }
        /**
         * @return ������� ������ ���� ����������
         * @see ConnectionPool
         */
        static unsigned getPoolSize() {
            __require__(pool);
            return pool->getSize();
        }
        
        /**
         * @return ������� ���������� ����������
         * @see ConnectionPool
         */
        static unsigned getConnectionsCount() {
            __require__(pool);
            return pool->getConnectionsCount();
        }
        /**
         * @return �������, ���� �� ��������� ����������
         * @see ConnectionPool
         */
        static bool hasFreeConnections() {
            __require__(pool);
            return pool->hasFreeConnections();
        }
        /**
         * @return ������� ���������� ������� ����������
         * @see ConnectionPool
         */
        static unsigned getBusyConnectionsCount() {
            __require__(pool);
            return pool->getBusyConnectionsCount();
        }
        /**
         * @return ������� ���������� ������������� ����������
         * @see ConnectionPool
         */
        static unsigned getIdleConnectionsCount() {
            __require__(pool);
            return pool->getIdleConnectionsCount();
        }
        /**
         * @return ������� ���������� �������� ��������� ���������
         * @see ConnectionPool
         */
        static unsigned getPendingQueueLength() {
            __require__(pool);
            return pool->getPendingQueueLength();
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

        /**
         * ���������� ������ MessageStore ��� ������� ��������� �����.
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
            const Body& newBody, uint8_t deliveryReport,
            time_t validTime = 0, time_t waitTime = 0)
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
            const Descriptor& dst, uint8_t failureCause, time_t nextTryTime) 
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
            const Descriptor& dst, uint8_t failureCause) 
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

}}

#endif

