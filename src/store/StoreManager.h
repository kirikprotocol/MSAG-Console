#ifndef MESSAGE_MANAGER_DECLARATIONS
#define MESSAGE_MANAGER_DECLARATIONS

#include "MessageStore.h"

namespace smsc { namespace store
{
    class StoreConfig
    {
    protected:
        char*   userName;
        char*   userPwd;
        char*   dbName;

        StoreConfig(const char* db, const char* usr, const char* pwd);

    public:

        virtual ~StoreConfig();

        inline const char* getUserName() { return userName; };
        inline const char* getUserPwd() { return userPwd; };
        inline const char* getDbName() { return dbName; };
    };

    
    class ConnectionPool

    class StoreManager : public MessageStore
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

}}

#endif

