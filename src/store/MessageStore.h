#ifndef MESSAGE_STORE_DECLARATIONS
#define MESSAGE_STORE_DECLARATIONS

/**
 * ���� �������� �������� ���������� ���������� �������� ���������
 * � ��������� SMS ������.
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see SMS
 * @see StoreManager
 */

#include <sms/sms.h>
#include "StoreExceptions.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;

    class IdIterator
    {
    protected:

        /**
         * ���������� �����������.
         * ��������� ���������� IdIterator ����� ���� ������
         * ������ ����� ����������� �����, �������� ReadyIdIterator
         *
         * @see StoreManager::ReadyIdIterator
         */
        IdIterator() {};
    
    public:

        /**
         * ����������, ���������� ������ � ����������� ����������
         * � ����������. ������ ���� ���������� ����������� �������,
         * �������� ReadyIdIterator
         *
         * @see StoreManager::ReadyIdIterator
         */
        virtual ~IdIterator() {};

        /**
         * ������������ ��� ��������� ���������� id'� ���������.
         * ����������� ����������� �������, �������� ReadyIdIterator.
         * 
         * @param id     ������ ��� �������� id � ������ ������
         * @return �������, ��� �� �������� id ��� ������ id'�� ���
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         *                   
         * @see StoreManager::ReadyIdIterator
         */
        virtual bool getNextId(SMSId& id)
            throw(StorageException) = 0;
    };

    /**
     * ���������� ������ �������� SMS � ��������� ������ createSMS()
     *
     * CREATE_NEW                -  ������ ��������� ����� ��������� � ��;
     * SMPP_OVERWRITE_IF_PRESENT -  �������� ����������� ������������
     *                              ��������� ������� ���� ��� ����
     *                              ������� �� OA, DA & SVC_TYPE;
     * ETSI_REJECT_IF_PRESENT    -  �������� ����������� �������� �
     *                              �������� ��������� ���� ���� ���������
     *                              � ������������ OA, DA & MR.
     *
     * @see MessageStore::createSMS()
     */
    typedef enum { 
        CREATE_NEW=0, SMPP_OVERWRITE_IF_PRESENT=1, ETSI_REJECT_IF_PRESENT=2 
    } CreateMode;
    
    /**
     * ��������� ���������� �������� ��������� � ��������� SMS ������.
     * 
     * @author Victor V. Makarov
     * @version 1.0
     * @see SMS
     * @see StoreManager
     */
    class MessageStore 
    {
    public:    
        
        /**
         * ���������� ��������� id ��� �������� SMS � ���������
         * 
         * @return ��������� id ��� �������� SMS � ���������
         */
        virtual SMSId getNextId() = 0;
        
        /**
         * �������� SMS � ��������� ENROUTE � ��������� ���������.
         * ���������� ����� ����������� sms ���� ���������
         * �������������������� ����� ���� ����� ����� setter'��.
         * 
         * @param sms    SMS ������� ����� ��������� � ���������.
         * @param flag   ����-����� ����������, default - CREATE_NEW
         * @return id ��������� � ���������
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @exception DuplicateMessageException
         *                   ��������� ��� ������������� �����
         *                   ETSI_REJECT_IF_PRESENT � ���������� OA, DA & MR
         *                   � ���������� ������������� ��������� � ���������
         *                   � ��������� ENROUTE.
         * @see CreateMode
         * @see SMS
         */
        virtual SMSId createSms(SMS& sms, SMSId id,
                                const CreateMode flag = CREATE_NEW)
                throw(StorageException, DuplicateMessageException) = 0;
        
        /**
         * ��������� SMS �� ��������� ���������.
         * 
         * @param id     ����������������� ����� ��������� � ���������
         * @param sms    ��������� SMS ��� ���������� ����������� �������.
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @exception NoSuchMessageException
         *                   ��������� ���� ��������� � ��������� id
         *                   �� ���������� � ���������.
         * @see SMS
         */
        virtual void retriveSms(SMSId id, SMS &sms)
                throw(StorageException, NoSuchMessageException) = 0;
        
        /**
         * �������� ���� ��������� SMS � ��������� ���������.
         * �� ������ ����� ����������� ��������� � ������� ������� ��������� (?)
         * 
         * @param id        ����������������� ����� ��������� � ���������
         * @param oa        ���������-����� �����������
         * @param newMsg    ����� ���� ���������
         * @param newMsgLen ������ ������ ���� ���������
         * @param deliveryReport
         *                  ����� ������� ������ � �������� ���������
         * @param validTime ����� ����/����� �� �������� ��������� �������
         *                  ���� �� ���������, �� ����� ���������� � 0
         * @param waitTime  ����� ����/����� ��� ������ ������� ��������,
         *                  ���� �� ���������, �� ����� ���������� � 0
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @exception NoSuchMessageException
         *                   ��������� ���� ��������� � ��������� id
         *                   �� ���������� � ���������.
         * @see SMS
         */
        virtual void replaceSms(SMSId id, const Address& oa,
            const uint8_t* newMsg, uint8_t newMsgLen,
            uint8_t deliveryReport, time_t validTime = 0, time_t nextTime = 0)
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /**
         * �������� ��������� SMS � ��������� ���������.
         * ������������ � ������ ���������� ������� ��������.
         * 
         * @param id     ����������������� ����� ��������� � ���������
         * @param dst    ��������� Descriptor, ����������� ������
         *               MSC, IMSI � ����� SME ���������� SMS.
         * @param failureCause
         *               ������� ������� ��� ������� ��������
         * @param nextTryTime
         *               ����/����� ��������� ������� ��������
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @exception NoSuchMessageException
         *                   ��������� ���� ��������� � ��������� id
         *                   �� ���������� � ���������.
         * @see SMS
         */
        virtual void changeSmsStateToEnroute(SMSId id,
            const Descriptor& dst, uint32_t failureCause, time_t nextTryTime) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /**
         * �������� ��������� � DELIVERED � ��������� SMS � ��������� ���������.
         * ������������ � ������ �������� �������� ���������.
         * ����� �������� ������� ��������� ��������� �������.
         * 
         * @param id     ����������������� ����� ��������� � ���������
         * @param dst    ��������� Descriptor, ����������� ������
         *               MSC, IMSI � ����� SME ���������� SMS.
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @exception NoSuchMessageException
         *                   ��������� ���� ��������� � ��������� id
         *                   �� ���������� � ���������.
         * @see SMS
         */
        virtual void changeSmsStateToDelivered(SMSId id, 
            const Descriptor& dst) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /**
         * �������� ��������� � UNDELIVERABLE � ��������� SMS
         * � ��������� ���������.
         * ������������ � ������ �������������� ������������� ��������
         * ���������. �.�., ���� �������� � ����� ������� �� ����������.
         * 
         * @param id     ����������������� ����� ��������� � ���������
         * @param dst    ��������� Descriptor, ����������� ������
         *               MSC, IMSI � ����� SME ���������� SMS.
         * @param failureCause
         *               ������� ������� ��� ������� ��������
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @exception NoSuchMessageException
         *                   ��������� ���� ��������� � ��������� id
         *                   �� ���������� � ���������.
         * @see SMS
         */
        virtual void changeSmsStateToUndeliverable(SMSId id,
            const Descriptor& dst, uint32_t failureCause) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /**
         * �������� ��������� SMS � EXPIRED � ��������� ���������.
         * ������������ � ������ ��������� ����� ����� ��������� (validTime).
         * 
         * @param id     ����������������� ����� ��������� � ���������
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @exception NoSuchMessageException
         *                   ��������� ���� ��������� � ��������� id
         *                   �� ���������� � ���������.
         * @see SMS
         */
        virtual void changeSmsStateToExpired(SMSId id) 
                throw(StorageException, NoSuchMessageException) = 0; 
    
        /**
         * �������� ��������� SMS � DELETED � ��������� ���������.
         * ������������ � ������ ������� ������������ �� �������� ���������.
         * 
         * @param id     ����������������� ����� ��������� � ���������
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @exception NoSuchMessageException
         *                   ��������� ���� ��������� � ��������� id
         *                   �� ���������� � ���������.
         * @see SMS
         */
        virtual void changeSmsStateToDeleted(SMSId id) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /**
         * ������� ��������� SMS �� ��������� ���������.
         * ������������ ������ ��� ���������� ���� ������������ �������.
         * 
         * @param id     ����������������� ����� ��������� � ���������
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @exception NoSuchMessageException
         *                   ��������� ���� ��������� � ��������� id
         *                   �� ���������� � ���������.
         * @see SMS
         */
        virtual void destroySms(SMSId id) 
                throw(StorageException, NoSuchMessageException) = 0;
    
        /**
         * ���������� �������� ��� ������� id ���������, �������
         * � ��������� ������� ��������, �.�. �� � �������
         * ����� ��������� ������� �������� �� ��������� ����������.
         * 
         * @param retryTime ����/����� ��� ������� (������� �����)
         * @return �������� ��� ������� id ���������, �������
         *         � ��������� ������� ��������
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         * @see IdIterator
         */
        virtual IdIterator* getReadyForRetry(time_t retryTime) 
                throw(StorageException) = 0;
        
        /**
         * ���������� ����������� ����� ��� ��������� �������
         * �������� ��������� ����������� � ���������.
         * � ������ ���������� ���������, ��������� ��������� 
         * ������� ��������, ���������� 0.
         * 
         * @return ����������� ����� ��� ��������� �������
         *         �������� ��������� ����������� � ���������
         * @exception StorageException
         *                   ��������� ��� ������ ��������� ���������� �������,
         *                   �.� ����� ��������� ����������.
         */
        virtual time_t getNextRetryTime() 
                throw(StorageException) = 0;
    
    protected:
        
        /**
         * ���������� �����������.
         * ��������� ���������� MessageStore ����� ���� ������
         * ������ � ������� StoreManager
         *
         * @see StoreManager
         */
        MessageStore() {};

        /**
         * ���������� ����������.
         * ��������� ���������� MessageStore ����� ���� ���������
         * ������ � ������� StoreManager
         *
         * @see StoreManager
         */
        virtual ~MessageStore() {};
    };
}}

#endif

