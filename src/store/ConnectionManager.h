#ifndef CONNECTION_MANAGER_DECLARATIONS
#define CONNECTION_MANAGER_DECLARATIONS

#include <oci.h>

#include "StoreExceptions.h"

namespace smsc { namespace store 
{
    class StoreConfig
    {
    private:
        
        char*   userName;
        char*   userPwd;
        char*   dbName;

    public:

        StoreConfig(const char* db, const char* usr, const char* pwd);
        virtual ~StoreConfig();

        inline const char* getUserName() { return userName; };
        inline const char* getUserPwd() { return userPwd; };
        inline const char* getDbName() { return dbName; };
    };
    
    const int MAX_STORE_STATEMENTS = 3;

    struct Connection
    {
	    OCIEnv*         envhp;  // OCI envirounment handle
        OCISvcCtx*      svchp;  // OCI service handle
	    OCIError*       errhp;  // OCI error handle
        OCIServer*      srvhp;  // OCI server handle
        OCISession*     sesshp; // OCI session handle
        
        // OCI prepared 'store' statements
        OCIStmt*        storeStmthps[MAX_STORE_STATEMENTS]; 
		
	    Connection(int _id = 0) 
	        : id(_id), envhp(0L), svchp(0L), errhp(0L) {};
	    Connection(int _id, OCIEnv* _envhp, OCISvcCtx* _svchp, OCIError* _errhp)
	        : id(_id), envhp(_envhp), svchp(_svchp), errhp(_errhp) {};
	
	    virtual ~Connection() {};
    
    private:
	    int	id;
    };
    
    class ConnectionPool
    {
    protected:

	    StoreConfig*	config;
        
	    ConnectionPool(StoreConfig* _config) 
            throw(ResourceAllocationException, AuthenticationException) 
                : config(_config) {};

    public:
	
	    virtual ~ConnectionPool() { 
            if (config) delete config; 
        };
	
	    virtual void connect() 
            throw(ResourceAllocationException, AuthenticationException) = 0;
	    virtual void disconnect() = 0;

	    virtual Connection* getConnection() = 0;
	    virtual void freeConnection(Connection* connection) = 0;
    };
    
    class SingleConnectionPool : public ConnectionPool
    {
    private:
    
	    Connection*	conn;
	    bool        lock; // Replace with mutex

    public:
    
	    SingleConnectionPool(StoreConfig* _config)
            throw(ResourceAllocationException, AuthenticationException);

	    virtual ~SingleConnectionPool();

	    virtual void connect()
            throw(ResourceAllocationException, AuthenticationException);
	    virtual void disconnect();

	    virtual Connection* getConnection();
	    virtual void freeConnection(Connection* connection);
    };

}}

#endif


