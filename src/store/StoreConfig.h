#ifndef STORE_CONFIG_DECLARATIONS
#define STORE_CONFIG_DECLARATIONS

namespace smsc { namespace store
{
    class StoreConfig
    {
	public:

        virtual ~StoreConfig() {};

        virtual const char* getDBInstance() = 0;
		virtual const char* getDBUserName() = 0;
		virtual const char* getDBUserPassword() = 0;
		
		virtual int getMaxConnectionsCount() = 0;
		virtual int getInitConnectionsCount() = 0;
    };
}}

#endif


