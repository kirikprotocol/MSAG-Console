#ifndef MESSAGE_STORE_DECLARATIONS
#define MESSAGE_STORE_DECLARATIONS

#include <sms/sms.h>

#include "StoreExceptions.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;

    struct MessageStore 
    {
        virtual SMSId store(const SMS &sms)  
            throw(StorageException, DuplicateMessageException) = 0;
        
        virtual void retrive(SMSId id, SMS &sms)
            throw(StorageException, NoSuchMessageException) = 0;
        
        virtual void remove(SMSId id) 
            throw(StorageException, NoSuchMessageException) = 0;
        
        virtual void replace(SMSId id, const SMS &sms) 
            throw(StorageException, NoSuchMessageException) = 0; 
        
        virtual void update(SMSId id, const State state, 
                            time_t operationTime=0, uint8_t fcs=0) 
            throw(StorageException, NoSuchMessageException) = 0; 
    
    protected:
        
        MessageStore() {};
        virtual ~MessageStore() {};
    };
}}

#endif

