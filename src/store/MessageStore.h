#ifndef MESSAGE_STORE_DECLARATIONS
#define MESSAGE_STORE_DECLARATIONS

#include <sms/sms.h>
#include "StoreExceptions.h"

namespace smsc { namespace store
{
    class StoreConfig
    {
    protected:
        int     type;
        char*   userName;
        char*   userPwd;
        char*   dbName;

        StoreConfig(int _type, 
	    const char* db, const char* usr, const char* pwd);
    
    public:
        virtual ~StoreConfig();
        
        inline int getType() { return type; };
        
        inline const char* getUserName() { return userName; };
        inline const char* getUserPwd() { return userPwd; };
        inline const char* getDbName() { return dbName; };
    };
    
    class MessageStore 
    {
    protected:
        StoreConfig* config;
        MessageStore(StoreConfig* _config) : config(_config) {};
    
    public:    
        virtual ~MessageStore() {};

        virtual void open() 
            throw(ResourceAllocationException, AuthenticationException) = 0;
        virtual void close() = 0;

        virtual sms::SMSId store(sms::SMS* message) 
            throw(ResourceAllocationException) = 0;
        virtual sms::SMS* retrive(sms::SMSId id) 
            throw(ResourceAllocationException, NoSuchMessageException) = 0;
    };
    
    struct MessageStoreFactory {
        virtual MessageStore* createMessageStore(StoreConfig* config) = 0;
    };

}}

#endif


