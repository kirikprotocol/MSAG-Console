#ifndef OCI_MESSAGE_STORE_DECLARATIONS
#define OCI_MESSAGE_STORE_DECLARATIONS

#include <oci.h>
#include <store/MessageStore.h>

namespace smsc { namespace store { namespace oci 
{
    using namespace smsc::store;
    
    const int ociStoreId = 100; 
    
    struct OCIStoreConfig : public StoreConfig 
    {
        OCIStoreConfig(const char* db,const char* user, const char* pwd) : 
           StoreConfig(ociStoreId, db, user, pwd) {}; 
        virtual ~OCIStoreConfig() {};
    };

    class OCIMessageStore : public MessageStore 
    {
    private:   
        bool            isOpened;
        
	//OCIEnv 		*envhp; /* the environment handle */
	//OCIServer 	*srvhp; /* the server handle */
	//OCIError 	*errhp; /* the error handle */
	//OCISession 	*usrhp; /* user session handle */
	//OCISvcCtx 	*svchp; /* the service handle */
        
	OCIEnv*         envhp;    /* environment handle  */
        OCIError*       errhp;    /* error handle        */
        OCISvcCtx*      svchp;    /* service handle      */

    protected:
        void checkerror(OCIError *errhp, sword status);
	
    public:    
        OCIMessageStore(OCIStoreConfig* _config);
        virtual ~OCIMessageStore();
        
        virtual void open()
            throw(ResourceAllocationException, AuthenticationException);
        virtual void close();
        
        virtual sms::SMSId store(sms::SMS* message)
            throw(ResourceAllocationException);
        virtual sms::SMS* retrive(sms::SMSId id)
            throw(ResourceAllocationException, NoSuchMessageException);
    };

    struct OCIMessageStoreFactory : public MessageStoreFactory 
    {
        virtual MessageStore* createMessageStore(StoreConfig* config) 
	{
	    return ((config && config->getType() == ociStoreId) ? 
                new OCIMessageStore((OCIStoreConfig *)config) : 0);
        };
    };


}}}

#endif


