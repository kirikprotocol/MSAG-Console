#ifndef MESSAGE_STORE_DECLARATIONS#define MESSAGE_STORE_DECLARATIONS

#include <sms/sms.h>#include "StoreExceptions.h"

namespace smsc { namespace store{
    struct MessageStore 
    {
        virtual void open()             throw(ResourceAllocationException, AuthenticationException) = 0;
        virtual void close() = 0;
        virtual sms::SMSId store(sms::SMS* message) 
            throw(ResourceAllocationException) = 0;
        virtual sms::SMS* retrive(sms::SMSId id) 
            throw(ResourceAllocationException, NoSuchMessageException) = 0;

	virtual ~MessageStore() {};

    protected:
	MessageStore() {};
   };

}}

#endif