#ifndef SCAG_RULE_ENGINE_SMPP_ADAPTER
#define SCAG_RULE_ENGINE_SMPP_ADAPTER

#include <scag/transport/smpp/SmppCommand.h> 
//#include <scag/util/properties/Properties.h>
#include <scag/re/actions/ActionContext.h>

#include <core/buffers/IntHash.hpp>

  
namespace scag { namespace re { namespace smpp 
{
    using namespace scag::util::properties;
    using namespace scag::transport::smpp;
    using smsc::core::buffers::IntHash;
    using scag::bill::infrastruct::TariffRec;
    using scag::re::actions::CommandAccessor;


    class SmppCommandAdapter : public CommandAccessor
    {
        enum AdditionTag
        {
            OA                      = 100,
            DA                      = 101,

            ESM_MM_SMSC_DEFAULT     = 102,
            ESM_MM_DATAGRAM         = 103,
            ESM_MM_FORWARD          = 104,
            ESM_MM_S_AND_F          = 105,
            ESM_MT_DEFAULT          = 106,
            ESM_MT_DELIVERY_ACK     = 107,
            ESM_MT_MANUAL_ACK       = 108,
            ESM_NSF_NONE            = 109,
            ESM_NSF_UDHI            = 110,
            ESM_NSF_SRP             = 111,
            ESM_NSF_BOTH            = 112,

            RD_RECEIPT_OFF          = 113,
            RD_RECEIPT_ON           = 114,
            RD_RECEIPT_FAILURE      = 115,
            RD_ACK_OFF              = 116,
            RD_ACK_ON               = 117,    
            RD_ACK_MAN_ON           = 118,
            RD_RD_ACK_BOTH_ON       = 119,
            RD_I_NOTIFICATION       = 120,


            DC_BINARY               = 121,
            DC_TEXT                 = 122,
            DC_SMSC_DEFAULT         = 123,
            DC_ASCII_X34            = 124,
            DC_LATIN1               = 125,
            DC_JIS                  = 126,
            DC_CYRILIC              = 127,
            DC_LATIN_HEBREW         = 128,
            DC_UCS2                 = 129,
            DC_PICTOGRAM_ENC        = 130,
            DC_ISO_MUSIC_CODES      = 131,
            DC_E_KANJI_JIS          = 132,
            DC_KS_C_5601            = 133,
            DC_GSM_MWI              = 134,
            DC_GSM_MSG_CC           = 135,

            USSD_PSSD_IND           = 136,
            USSD_PSSR_IND           = 137,
            USSD_USSR_REQ           = 138,
            USSD_USSN_REQ           = 139,
            USSD_PSSD_RESP          = 140,
            USSD_PSSR_RESP          = 141,
            USSD_USSR_CONF          = 142,
            USSD_USSN_CONF          = 143,

            ST_ENROUTE              = 144,
            ST_DELIVERED            = 145, 
            ST_EXPIRED              = 146,
            ST_DELETED              = 147,
            ST_UNDELIVERABLE        = 148,
            ST_ACCEPTED             = 149,
            ST_UNKNOWN              = 150,
            ST_REJECTED             = 151,

            SMS_VALIDITY_PERIOD     = 152,
            SMS_MESSAGE_BODY        = 153,
            SMS_SVC_TYPE            = 154,

            USSD_DIALOG             = 155,
            MESSAGE_ID              = 156,
            STATUS                  = 157,
            PACKET_DIRECTION        = 158,

            STATUS_OK               = 159,
            STATUS_PERM_ERROR       = 160,
            STATUS_TEMP_ERROR       = 161,

            //Binary tags
            OPTIONAL_CHARGING                           = 0x4901,
            OPTIONAL_MESSAGE_TRANSPORT_TYPE             = 0x4902,
            //String tags
            OPTIONAL_MESSAGE_CONTENT_TYPE               = 0x4903,
            //Binary tags
            OPTIONAL_EXPECTED_MESSAGE_TRANSPORT_TYPE    = 0x4904,
            //String tags
            OPTIONAL_EXPECTED_MESSAGE_CONTENT_TYPE      = 0x4905

        };

        //bool m_hasPayloadText;

        bool IsShortSize(int size) {return (size <= 254);}
        AdapterProperty * getMessageBodyProperty(SMS& data, std::string name);


        AdapterProperty * getSubmitProperty(SMS& data,const std::string& name,int FieldId);
        AdapterProperty * getDeliverProperty(SMS& data,const std::string& name,int FieldId);
        AdapterProperty * getDataSmProperty(SmsCommand& data,const std::string& name,int FieldId);
        AdapterProperty * getRespProperty(SMS& data, const std::string& name,int FieldId);


        AdapterProperty * Get_ESM_BIT_Property(SMS& data, const std::string& name,int FieldId);
        AdapterProperty * Get_RD_BIT_Property(SMS& data, const std::string& name,int FieldId);
        AdapterProperty * Get_DC_BIT_Property(SMS& data, const std::string& name,int FieldId);
        AdapterProperty * Get_USSD_BOOL_Property(SMS& data, const std::string& name,int FieldId);
        AdapterProperty * Get_Unknown_Property(SMS& data, const std::string& name,int FieldId);

        void Set_DC_BIT_Property(SMS& data, int FieldId, bool value);
        void SetBitMask(SMS& data, int tag, int mask);


        AdapterProperty * GetStrBitFromMask(SMS& data,const std::string& name,int tag,int mask);

        void WriteDeliveryField(SMS& data,int FieldId,AdapterProperty& property);
        void WriteSubmitField(SMS& data,int FieldId,AdapterProperty& property);
        void WriteDataSmField(SMS& data,int FieldId,AdapterProperty& property);


        SmppCommand&    command;
        IntHash<AdapterProperty *>  PropertyPul;
        AdapterProperty *src_sme_id, *dst_sme_id;

        static IntHash<AccessType> SubmitFieldsAccess;
        static IntHash<AccessType> DeliverFieldsAccess;
        static IntHash<AccessType> DataSmFieldsAccess;
        static IntHash<AccessType> RespFieldsAccess;

        static Hash<int> SubmitFieldNames;
        static Hash<int> DeliverFieldNames;
        static Hash<int> DataSmFieldNames;
        static Hash<int> RespFieldNames;

        static Hash<int> InitSubmitFieldNames();
        static Hash<int> InitDeliverFieldNames();

        static Hash<int> InitDataSmFieldNames();

        static Hash<int> InitRespFieldNames();

        static IntHash<AccessType> InitSubmitAccess();
        static IntHash<AccessType> InitDeliverAccess();
        static IntHash<AccessType> InitDataSmAccess();
        static IntHash<AccessType> InitRespAccess();
                                                            
    public:

        SmppCommandAdapter(SmppCommand& _command) : command(_command), src_sme_id(0), dst_sme_id(0) 
        {
        }

        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);
        //virtual void fillChargeOperation(smsc::inman::interaction::ChargeSms& op, TariffRec& tariffRec);
        //virtual void fillRespOperation(smsc::inman::interaction::DeliverySmsResult& op, TariffRec& tariffRec);


        virtual SCAGCommand& getSCAGCommand() { return command; }
        
        virtual ~SmppCommandAdapter();
        static AccessType CheckAccess(int handlerType,const std::string& name);
        //virtual SMS& getSMS();

        bool hasServiceOp()
        {
            return CommandBrige::getSMS(command).hasIntProperty(Tag::SMPP_USSD_SERVICE_OP);
        }

        int getServiceOp()
        {
            return CommandBrige::getSMS(command).getIntProperty(Tag::SMPP_USSD_SERVICE_OP);
        }

        void setServiceOp(int value)
        {
            CommandBrige::getSMS(command).setIntProperty(Tag::SMPP_USSD_SERVICE_OP, value);
        }

        void setOA(Address& addr)
        {
            CommandBrige::getSMS(command).setOriginatingAddress(addr);
        }

        void setDA(Address& addr)
        {
            CommandBrige::getSMS(command).setDestinationAddress(addr);
        }
    };

}}}

#endif // SCAG_RULE_ENGINE_SMPP_ADAPTER
