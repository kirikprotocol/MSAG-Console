#ifndef CONNECTION_MANAGER_DECLARATIONS
#define CONNECTION_MANAGER_DECLARATIONS

#include <oci.h>
#include <orl.h>

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/Array.hpp>
#include <sms/sms.h>

#include "StoreConfig.h"
#include "StoreExceptions.h"

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;

namespace smsc { namespace store 
{
	struct Connection;
    class ConnectionPool
	{
    protected:

        int 	maxConnectionsCount;
		int		curConnectionsCount;
		Mutex	connectionsLock;
		Mutex	idleLock;

		StoreConfig*	config;
		
        Array<Connection*>	idle;
		Array<Connection*>	busy;
		Array<Connection*>	dead;

	public:
	
	    ConnectionPool(StoreConfig* _config) 
			throw(ConnectionFailedException);
	    virtual ~ConnectionPool(); 
        
	    inline StoreConfig*	getConfig() {
			return config;
		};

		void checkErr(sword status, Connection* conn) 
			throw(StorageException);

		Connection* getConnection() 
			throw(ConnectionFailedException);
	    void freeConnection(Connection* connection);
    };
	
    using namespace smsc::sms;

	class Connection
    {
	private:

	static sb4 _failoverCallback(dvoid *svchp, dvoid *envhp,
						 dvoid *fo_ctx, ub4 fo_type, ub4 fo_event) 
	{
		printf("Connection failure !!!\n");
		return ((fo_ctx) ? (((Connection *)fo_ctx)->failoverCallback(
			svchp, envhp, fo_ctx, fo_type, fo_event)) : ((sb4) 0));
	};
	
	protected:
		
        static text* 	sqlGetMessagesCount;
        static text* 	sqlStoreInsert;
		static text* 	sqlRetriveAll;
	    
        ConnectionPool*	owner;

		OCIEnv*         envhp;  // OCI envirounment handle
        OCISvcCtx*      svchp;  // OCI service handle
        OCIServer*      srvhp;  // OCI server handle
		OCIError*       errhp;  // OCI error handle
        OCISession*     sesshp; // OCI session handle
        
		// OCI prepared statements
		OCIStmt* 		stmtGetMessagesCount;
        OCIStmt* 		stmtStoreInsert;
		OCIStmt* 		stmtRetriveAll;

        OCIBind			*bndSt, *bndMsgRef, *bndMsgInd;
		OCIBind			*bndOALen, *bndOATon, *bndOANpi, *bndOAVal;
		OCIBind			*bndDALen, *bndDATon, *bndDANpi, *bndDAVal;
		OCIBind			*bndVTime, *bndWTime, *bndSTime, *bndDTime;
		OCIBind			*bndSrr, *bndRd, *bndMsgPri, *bndMsgPid;
		OCIBind			*bndFcs, *bndDcs, *bndUdhi, *bndUdl, *bndUd;

		OCIDefine		*defMaxId, *defSt, *defMsgRef, *defMsgInd;
		OCIDefine   	*defOALen, *defOATon, *defOANpi, *defOAVal;
		OCIDefine		*defDALen, *defDATon, *defDANpi, *defDAVal;
		OCIDefine		*defVTime, *defWTime, *defSTime, *defDTime;
		OCIDefine		*defSrr, *defRd, *defMsgPri, *defMsgPid;
		OCIDefine		*defFcs, *defDcs, *defUdhi, *defUdl, *defUd;
		OCIBind			*bndStoreId, *bndRetriveId;

        SMS				sms;
		SMSId			smsId;
        
		OCIDate			waitTime;
		OCIDate     	validTime;
		OCIDate			submitTime;
		OCIDate     	deliveryTime;
        
		uint8_t			uState;
		char			bStatusReport;
		char			bRejectDuplicates;
		char			bHeaderIndicator;
		
		Mutex        	mutex;

		void checkConnErr(sword status) 
			throw(ConnectionFailedException);
		
		void checkErr(sword status) 
			throw(StorageException);

		void setSMS(const SMS &_sms) 
			throw(StorageException);
		const SMS& getSMS() 
			throw(StorageException);
        
        OCIFocbkStruct failover;
		sb4 failoverCallback(dvoid *svchp, dvoid *envhp,
							 dvoid *fo_ctx, ub4 fo_type, ub4 fo_event);

	public:

        Connection(ConnectionPool* pool)
			throw(ConnectionFailedException);
		virtual ~Connection();

        SMSId getMessagesCount()
			throw(ConnectionFailedException);

		void store(const SMS &sms, SMSId id) 
            throw(StorageException);

        const SMS& retrive(SMSId id) 
            throw(StorageException, NoSuchMessageException);

		friend class ConnectionPool; // ???
    };
    
}}

#endif


