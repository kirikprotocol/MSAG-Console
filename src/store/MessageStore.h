#ifndef MESSAGE_STORE_DECLARATIONS
#define MESSAGE_STORE_DECLARATIONS

#include <sms/sms.h>
#include "StoreExceptions.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;

    struct MessageStore 
    {
        virtual SMSId store(SMS& sms) 
            throw(StorageException) = 0;
        
        virtual SMS& retrive(SMSId id) 
            throw(StorageException, NoSuchMessageException) = 0;

		virtual ~MessageStore() {};

    protected:
	    MessageStore() {};
   };
}}

#endif

