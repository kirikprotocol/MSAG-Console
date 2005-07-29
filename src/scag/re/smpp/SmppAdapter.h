#ifndef SCAG_RULE_ENGINE_SMPP_ADAPTER
#define SCAG_RULE_ENGINE_SMPP_ADAPTER

#include <scag/transport/smpp/SmppCommand.h> 
#include <scag/util/properties/Properties.h>
#include <core/buffers/IntHash.hpp>


namespace scag { namespace re { namespace smpp 
{
    using namespace scag::util::properties;
    using namespace scag::transport::smpp;
    using smsc::core::buffers::IntHash;

    class SmppCommandAdapter : public PropertyManager
    {
    private:

        enum FieldsId
        {
            ADDRESS,
            DIALOG_ID,
            MODE,
            PRIORITY,
            SME_INDEX,
            STATUS,
    
            ATTEMPTS_COUNT,
            BILLING_RECORD,
            CONCAT_MSG_REF,
            CONCAT_SEQ_NUM,
            DEALIASED_DEST_ADDR,
            DELIVERY_REPORT,
            DEST_ADDR,
            DEST_SME_ID,
            ESERVICE_TYPE,
            LAST_RESULT,
            LAST_TIME,
            MSG_REFERENCE,
            NEXT_TIME,
            ORIGIN_ADDR,
            ROUTE_ID,
            SERVICE_ID,
            SOURCE_SME_ID,
            STATE,
            SUBMIT_TIME,
            VALID_TIME,
            ARCHIV_REQUEST,
        
            NUM_DESTS,
        
            DATA_SM,
            MSG_ID,
        
            SCHEDULE_DELIVERY_TIME,
            DEFAULT_MSG_ID,
            SM_LEN,
            SOURCE_ADDR,
            VALID_PERIOD,
        
            MSG_STATE,
            NETWORK_CODE,
        
            FORCE,
            INTERNAL,
            SERVICE_TYPE,
       
            PASS,
            SYS_ID
        };


        static Hash<int> FieldNames;
        static Hash<int> InitFieldNames();

        SmppCommand&    command;
        IntHash<AdapterProperty *>  PropertyPul;

        AdapterProperty * getPropertyFromPul(int FieldId,const std::string& InitName,int InitValue);
        AdapterProperty * getPropertyFromPul(int FieldId,const std::string& InitName,const std::string& InitValue);
                                    
        AdapterProperty * getProperty(const SMS& data,const std::string& name,int FieldId);
        AdapterProperty * getProperty(const SubmitMultiSm& data,const std::string& name,int FieldId);
        AdapterProperty * getProperty(const SubmitMultiResp& data,const std::string& name,int FieldId);
        AdapterProperty * getProperty(const SmsResp& data,const std::string& name,int FieldId);
        AdapterProperty * getProperty(const ReplaceSm& data,const std::string& name,int FieldId);
        AdapterProperty * getProperty(const QuerySm& data,const std::string& name,int FieldId);
        AdapterProperty * getProperty(const QuerySmResp& data,const std::string& name,int FieldId);
        AdapterProperty * getProperty(const CancelSm& data,const std::string& name,int FieldId);
        AdapterProperty * getProperty(const BindCommand& data,const std::string& name,int FieldId);
                                                                                        
    public:

        SmppCommandAdapter(SmppCommand& _command) 
            : PropertyManager(), command(_command) {};

        // TODO: Implement PropertyManager interface (Access to command fields)
        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);
        virtual ~SmppCommandAdapter();
    };

}}}

#endif // SCAG_RULE_ENGINE_SMPP_ADAPTER
