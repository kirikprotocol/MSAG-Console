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
            throw(StorageException) = 0;
        
        virtual const SMS& retrive(SMSId id) 
            throw(StorageException, NoSuchMessageException) = 0;

	protected:

	    MessageStore() {};
		virtual ~MessageStore() {};
   };
}}

#endif

