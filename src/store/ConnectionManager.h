#ifndef CONNECTION_MANAGER_DECLARATIONS
#define CONNECTION_MANAGER_DECLARATIONS

#include <oci.h>
#include <orl.h>

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/Array.hpp>
#include <sms/sms.h>
#include "StoreExceptions.h"

using namespace smsc::core::synchronization;
//using namespace smsc::core::buffers;

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
		
		int   			connectionsCount;
		Mutex			connectionsLock;
		Mutex			idleLock;

		Array<Connection*>	idle;
		Array<Connection*>	busy;
		Array<Connection*>	dead;

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
		OCIError*       errhp;  // OCI error handle
        OCISession*     sesshp; // OCI session handle
        
		// OCI prepared 'store' statements
		OCIStmt* 		stmtStoreLock;
		OCIStmt* 		stmtStoreMaxId;
        OCIStmt* 		stmtStoreInsert;
		OCIStmt* 		stmtRetriveAll;

		OCIDefine	*defhp;
		OCIBind		*bndSt, *bndMsgRef, *bndMsgInd;
		OCIBind		*bndOALen, *bndOATon, *bndOANpi, *bndOAVal;
		OCIBind		*bndDALen, *bndDATon, *bndDANpi, *bndDAVal;
		OCIBind		*bndVTime, *bndWTime, *bndSTime, *bndDTime;
		OCIBind		*bndSrr, *bndRd, *bndMsgPri, *bndMsgPid;
		OCIBind		*bndFcs, *bndDcs, *bndUdhi, *bndUdl, *bndUd;

		OCIDefine	*defSt, *defMsgRef, *defMsgInd;
		OCIDefine   *defOALen, *defOATon, *defOANpi, *defOAVal;
		OCIDefine	*defDALen, *defDATon, *defDANpi, *defDAVal;
		OCIDefine	*defVTime, *defWTime, *defSTime, *defDTime;
		OCIDefine	*defSrr, *defRd, *defMsgPri, *defMsgPid;
		OCIDefine	*defFcs, *defDcs, *defUdhi, *defUdl, *defUd;
		OCIBind		*bndRetriveId;
		OCIBind		*bndStoreId;

        SMS				sms;
		SMSId			smsId;
        
		OCIDate			waitTime;
		OCIDate     	validTime;
		OCIDate			submitTime;
		OCIDate     	deliveryTime;
		OCIRaw*			rawUd;	

		uint8_t			uState;
		char			bStatusReport;
		char			bRejectDuplicates;
		char			bHeaderIndicator;
		
		Mutex        	mutex;

		void checkErr(sword status) 
			throw(StoreException);

		void setSMS(SMS& _sms) throw(StoreException);
		SMS& getSMS() throw(StoreException);

	public:

        Connection(ConnectionPool* pool, int _id = 0)
			throw(StoreException);
		virtual ~Connection();

        virtual SMSId store(SMS& sms) 
            throw(StoreException);

        virtual SMS& retrive(SMSId id) 
            throw(StoreException);

		friend class ConnectionPool;
    };
    
}}

#endif


