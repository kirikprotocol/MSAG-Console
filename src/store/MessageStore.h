#ifndef MESSAGE_STORE_DECLARATIONS
#define MESSAGE_STORE_DECLARATIONS

#include <sms.h>
#include "StoreExceptions.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;

    struct MessageStore 
    {
        virtual SMSId store(SMS* sms) 
            throw(ResourceAllocationException) = 0;
        
        virtual SMS* retrive(SMSId id) 
            throw(ResourceAllocationException, NoSuchMessageException) = 0;

	virtual ~MessageStore() {};

    protected:
	    MessageStore() {};
   };
}}

#endif