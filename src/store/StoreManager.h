#ifndef STORE_MANAGER_DECLARATIONS
#define STORE_MANAGER_DECLARATIONS

#include <oci.h>

#include "MessageStore.h"
#include "ConnectionManager.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;
    
    class StoreManager : public MessageStore
    {
    private:

	    static MessageStore* instance;
	
    protected:
        
	    ConnectionPool*	pool;

        StoreManager()
            throw(ResourceAllocationException, AuthenticationException);

    public:    

        static MessageStore* getInstance()
            throw(ResourceAllocationException, AuthenticationException);
        
        virtual ~StoreManager();

        virtual SMSId store(SMS* message) 
            throw(ResourceAllocationException);
        virtual SMS* retrive(SMSId id) 
            throw(ResourceAllocationException, NoSuchMessageException);
    };

}}

#endif

