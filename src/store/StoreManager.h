#ifndef STORE_MANAGER_DECLARATIONS
#define STORE_MANAGER_DECLARATIONS

#include <oci.h>

#include "StoreConfig.h"
#include "MessageStore.h"
#include "ConnectionManager.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;
    
    class StoreManager : public MessageStore
    {
    private:

	    static MessageStore	*instance;
		static StoreConfig	*config;
	
    protected:
        
	    ConnectionPool*	pool;

        StoreManager()
			throw(ConnectionFailedException);

    public:    
		
		static void init(StoreConfig* config) {
			StoreManager::config = config;
		};

        static MessageStore* getInstance()
			throw(ConnectionFailedException);
        
        virtual ~StoreManager();

        virtual SMSId store(SMS& sms)  
			throw(StorageException);
        virtual SMS& retrive(SMSId id)
			throw(StorageException, NoSuchMessageException);
    };

}}

#endif

