#ifndef ARCHIVER_DECLARATIONS
#define ARCHIVER_DECLARATIONS

/**
 * ���� �������� �������� ���������� ���������
 * � �������� ����������� ������� � ��������� SMS ������.
 * ���������� �������� �� �������������� � ����������� �������� ���������.
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
     * ���������� ��������� � �������� ����������� �������
     * � ��������� SMS ������.
     * �������� �� �������������� � ����������� �������� ���������.
     * ������������ ��������� ��������� �� ���������, ��������� �� �
     * ����� � ������ ����������� ������ ��� �������. 
     * 
     * ������� ��������� � �������� ����������� ������� ����������
     * � ���� ���������� ������ ���������� � ������� SMS ������.
     * ������� �������������� ����������� ���� �������:
     * 1) ���� �������� ������� �������;
     * 2) ���������� ��������� � ��������� ��������� � ���������
     *    ��������� ����������� �������.
     * ��������� ��������� �������� ��� ��������������� �������.
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
         * �����������, ������ (�� �� ���������) ������� ���������
         * � �������� ����������� �������.
         * ��������� ����� ���������������� ���������� � ������ ���
         * ���������� � �����������: ��������� � ����������� �������.
         * 
         * @param config �������� ��� ��������� ���������������� ����������
         * @exception ConfigException
         *                   ��������� � ������ �������������� �/���
         *                   ��������� ������ ���������������� ����������.
         * @see Connection
         * @see smsc::util::config::Manager
         */
        Archiver(Manager& config)
            throw(ConfigException);
        
        /**
         * ����������, ������������� � ���������� ������� ���������
         * � �������� ����������� �������.
         * ������� ���������� �������� ������������ ������ ������.
         * �����, ���������� ���������� � ����������� ������.
         * 
         * @see Connection
         */
        virtual ~Archiver();
    
        /**
         * ���������� ��������� (������������) �������������� ������
         * ��� ��������� � ����������.
         * ������������ ��� ������ ������� ��������.
         * 
         * @return ��������� (������������) �������������� ������
         * @exception StorageException
         *                   ��������� � ������ �������������� ������
         *                   ��� ������ � �����������.
         */
        SMSId getLastUsedId()
            throw(StorageException); 

        /**
         * ����������� ���������� ������� ����� ���������
         * � ��������� ���������.
         * 
         * @param count �� ������� ����� ��������� �������, default=1
         */
        void incrementFinalizedCount(unsigned count=1);
        
        /**
         * ��������� ���������� ������� ����� ���������
         * � ��������� ���������.
         * 
         * @param count �� ������� ����� ��������� �������, default=1
         */
        void decrementFinalizedCount(unsigned count=1);

        /**
         * ������������ ��������� �� ������� Thread
         * ��� �������� � ������� ����������� ������ ����������
         * 
         * @return ��� ����������, �� ������������
         * @see Thread
         */
        virtual int Execute();
        
        /**
         * ��������� ������� ��������� � �������� ����������� �������.
         * ���� ��� �������, �� ������ �� ���������.
         * ����� ���������������� �������� ��������� �������� ��������
         * ����� ��������� � ��������� ���������.
         * 
         * @exception StorageException
         *                   ��������� � ������ �������������� ������
         *                   ��� ������ � �����������.
         */
        void Start() throw(StorageException);
        
        /**
         * ������������� ������� ��������� � �������� ����������� �������.
         * ���� ��� �� �������, �� ������ �� ���������.
         * ������� ���������� �������� ������������ ������ ������.
         */
        void Stop();

        /**
         * @return ���������� �������, �������� �� ���������� ���������
         *         � �������� ����������� ������� � ��������� �����.
         */
        inline bool isStarted() {
            return bStarted;
        }
        
        /**
         * @return ���������� �������, �������� �� ������� ������� ���������
         *         � �������� ����������� ������� � ��������� �����.
         */
        inline bool isInProgress() {
            return job.isSignaled();
        }
    };

}};

#endif


