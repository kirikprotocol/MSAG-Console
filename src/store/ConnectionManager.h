#ifndef CONNECTION_MANAGER_DECLARATIONS
#define CONNECTION_MANAGER_DECLARATIONS

#include <oci.h>
#include <orl.h>

#include <sms/sms.h>
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
    
	struct Connection;
    class ConnectionPool
	{
    protected:

	    StoreConfig*	config;
	    Connection*		conn;
	    bool        	lock; // Replace with mutex
        
	public:
	
	    ConnectionPool(StoreConfig* _config) 
			throw(StoreException);
	    virtual ~ConnectionPool(); 
        
	    inline StoreConfig*	getConfig() {
			return config;
		};

		void checkErr(sword status, Connection* conn) 
			throw(StoreException);

		virtual Connection* getConnection();
	    virtual void freeConnection(Connection* connection);
    };
	
    using namespace smsc::sms;

	class Connection
    {
    protected:
		
		static text* 	sqlStoreLock;
		static text* 	sqlStoreMaxId;
        static text* 	sqlStoreInsert;
		static text* 	sqlRetriveAll;
	    
		int				id;
		ConnectionPool*	owner;

		OCIEnv*         envhp;  // OCI envirounment handle
        OCISvcCtx*      svchp;  // OCI service handle
        OCIServer*      srvhp;  // OCI server handle
        OCISession*     sesshp; // OCI session handle
        
		// OCI prepared 'store' statements
		OCIStmt* 		stmtStoreLock;
		OCIStmt* 		stmtStoreMaxId;
        OCIStmt* 		stmtStoreInsert;
		OCIStmt* 		stmtRetriveAll;
        
		SMS				sms;
		SMSId			smsId;
        
		OCIDate			waitTime;
		OCIDate     	validTime;
		OCIDate			submitTime;
		OCIDate     	deliveryTime;

		char			bStatusReport;
		char			bRejectDuplicates;
		char			bHeaderIndicator;
		
		void checkErr(sword status) 
			throw(StoreException);

		void setSMS(SMS& _sms);
		SMS& getSMS();

	public:

		OCIError*       errhp;  // OCI error handle

		Connection(ConnectionPool* pool, int _id = 0)
			throw(StoreException);
		virtual ~Connection();

        virtual SMSId store(SMS& sms) 
            throw(StoreException);

        virtual SMS& retrive(SMSId id) 
            throw(StoreException);
	
    };
    
}}

#endif


