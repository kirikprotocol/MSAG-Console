#ifndef CLEANER_DECLARATIONS
#define CLEANER_DECLARATIONS

/**
 * ���� �������� �������� ���������� ������� ������
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
     * ���������� ������ � ��������� ��������� � ��������� SMS ������.
     * �������� �� �������������� � ����������� �������� ���������.
     * 
     * ������� ������ � ��������� ��������� ����������
     * � ���� ���������� ������ ���������� � ������� SMS ������.
     * ����������� ��������� �������� ��� ��������������� �������.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see StoreManager
     */
    class Cleaner : public Thread
    {
    private:

        smsc::logger::Logger log;

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
         * �����������, ������ (�� �� ���������) ������� ������ � ��������� ���������
         * ��������� ����� ���������������� ���������� � ������ 
         * ���������� � ����������.
         * 
         * @param config �������� ��� ��������� ���������������� ����������
         * @exception ConfigException
         *                   ��������� � ������ �������������� �/���
         *                   ��������� ������ ���������������� ����������.
         * @see Connection
         * @see smsc::util::config::Manager
         */
        Cleaner(Manager& config) 
            throw(ConfigException); 
        /**
         * ����������, ������������� � ���������� ������� ������ � ��������� ���������.
         * ������� ���������� �������� ������������ ������ ������.
         * �����, ���������� ���������� � ���������� ������.
         * 
         * @see Connection
         */
        virtual ~Cleaner();

        /**
         * ���������� ��������� ������ ��� �������� ��������� � ���������.
         * 
         * @return ��������� ������
         * @exception StorageException
         *                   ��������� � ������ �������������� ������
         *                   ��� ������ � �����������.
         */
        SMSId getNextId()
            throw(StorageException); 
        
        /**
         * ������������ ��������� �� ������� Thread
         * ��� �������� � ������� ����������� ������ ����������
         * 
         * @return ��� ����������, �� ������������
         * @see Thread
         */
        virtual int Execute();
        
        /**
         * ��������� ������� ������ � ��������� ���������.
         * ���� ��� �������, �� ������ �� ���������.
         * 
         * @exception StorageException
         *                   ��������� � ������ �������������� ������
         *                   ��� ������ � �����������.
         */
        void Start();
        
        /**
         * ������������� ������� ������ � ��������� ���������.
         * ���� ��� �� �������, �� ������ �� ���������.
         * ������� ���������� �������� ������������ ������ ������.
         */
        void Stop();
        
        /**
         * @return �������, �������� �� ������� � ��������� �����.
         */
        inline bool isStarted() {
            return bStarted;
        }
        
        /**
         * @return �������, ���������� �� �������� ������ � ��������� �����.
         */
        inline bool isInProgress() {
            return awake.isSignaled();
        }

    };

}};

#endif // CLEANER_DECLARATIONS

