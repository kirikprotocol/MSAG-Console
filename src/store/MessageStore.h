#ifndef MESSAGE_STORE_DECLARATIONS
#define MESSAGE_STORE_DECLARATIONS

#include <sms/sms.h>

#include "StoreExceptions.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;

    /**
     * Допустимые режимы создания SMS в контексте вызова createSMS()
     * 
     * @see MessageStore::createSMS()
     */
    typedef enum { 
        CREATE_NEW=0, SMPP_OVERWRITE_IF_PRESENT=1, ETSI_REJECT_IF_PRESENT=2 
    } CreateMode;
    
    class MessageStore 
    {
    public:    
        /* 
        Works only with these fields, are need to be defined in SMS:
        
        uint8_t msgRef,EService eService, 
        Address& oa, Descriptor& src, Address& da, Body& body
        uint8_t priority, uint8_t protocolId,
        bool needArchivate, bool statusReportRequested,
        time_t submitTime, time_t validTime, time_t waitTime
        */
        virtual SMSId createSms(SMS& sms, const CreateMode flag = CREATE_NEW)
                throw(StorageException, DuplicateMessageException) = 0;
        
        virtual void retriveSms(SMSId id, SMS &sms)
                throw(StorageException, NoSuchMessageException) = 0;
        
        /* В Body не нужно замещать UDHI & DCS */
        virtual void replaceSms(SMSId id, const Address& oa,
            const Body& newBody, uint8_t deliveryReport,
            time_t validTime = 0, time_t waitTime = 0)
                throw(StorageException, NoSuchMessageException) = 0; 
        
        virtual void changeSmsStateToEnroute(SMSId id,
            const Descriptor& dst, uint8_t failureCause, time_t nextTryTime) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /* Установить failureCause = 0 */
        virtual void changeSmsStateToDelivered(SMSId id, 
            const Descriptor& dst) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        virtual void changeSmsStateToUndeliverable(SMSId id,
            const Descriptor& dst, uint8_t failureCause) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        virtual void changeSmsStateToExpired(SMSId id) 
                throw(StorageException, NoSuchMessageException) = 0; 
    
        virtual void changeSmsStateToDeleted(SMSId id) 
                throw(StorageException, NoSuchMessageException) = 0; 
        
        /* Needed for test needs. Will be removed.*/
        virtual void destroySms(SMSId id) 
                throw(StorageException, NoSuchMessageException) = 0;
    
    protected:
        
        MessageStore() {};
        virtual ~MessageStore() {};
    };
}}

#endif

