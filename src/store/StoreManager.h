#ifndef STORE_MANAGER_DECLARATIONS
#define STORE_MANAGER_DECLARATIONS

#include <oci.h>
#include <core/synchronization/Mutex.hpp>

#include "StoreConfig.h"
#include "MessageStore.h"
#include "ConnectionManager.h"

namespace smsc { namespace store
{
    using namespace smsc::sms;
	using namespace smsc::core::synchronization;
    
	class IDGenerator 
	{
	private:
		
		SMSId id;
		Mutex mutex;

	public:

		IDGenerator(SMSId _id) : id(_id) {};
		~IDGenerator() {};
		
		inline SMSId getNextId() 
		{
			MutexGuard guard(mutex);
			return ++id;
		};
	};

    class StoreManager : public MessageStore
    {
    private:
		
		static Mutex mutex;

		static IDGenerator		*generator;
	    static StoreManager		*instance;
        static ConnectionPool	*pool;

    protected:
        
        StoreManager() : MessageStore() {};
        virtual ~StoreManager() {};

    public:    
		
		static void startup(StoreConfig* config) 
			throw(ConnectionFailedException);
		
		static void shutdown(); 
        
        static MessageStore* getMessageStore() {
			return ((MessageStore *)instance);
		};
        
        virtual SMSId store(SMS& sms)  
			throw(StorageException);
        virtual SMS& retrive(SMSId id)
			throw(StorageException, NoSuchMessageException);
    };

}}

#endif

