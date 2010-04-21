#ifndef SCAG_RULE_ENGINE_SMPP_ADAPTER2
#define SCAG_RULE_ENGINE_SMPP_ADAPTER2

#include <map>
#include "scag/transport/smpp/base/SmppCommand2.h"
#include "ActionContext2.h"
#include "CommandBridge.h"

#include <core/buffers/IntHash.hpp>

namespace scag2 {
namespace re {
namespace smpp {

using namespace util::properties;
using namespace transport::smpp;
using smsc::core::buffers::IntHash;
using bill::infrastruct::TariffRec;
using re::actions::CommandAccessor;

class SmppCommandAdapter : public CommandAccessor
{
    enum AdditionTag
    {
            SMALLEST_ADDITIONAL_TAG = 100,
            OA                      = 100,
            DA                      = 101,

            ESM_MM_SMSC_DEFAULT     = 102,
            ESM_MM_DATAGRAM         = 103,
            ESM_MM_FORWARD          = 104,
            ESM_MM_S_AND_F          = 105,
            ESM_MT_DEFAULT          = 106,
            ESM_MT_RECEIPT          = 107,
            ESM_MT_DELIVERY_ACK     = 108,
            ESM_MT_MANUAL_ACK       = 109,
            ESM_NSF_NONE            = 110,
            ESM_NSF_UDHI            = 111,
            ESM_NSF_SRP             = 112,
            ESM_NSF_BOTH            = 113,

            RD_RECEIPT_OFF          = 114,
            RD_RECEIPT_ON           = 115,
            RD_RECEIPT_FAILURE      = 116,
            RD_ACK_OFF              = 117,
            RD_ACK_ON               = 118,
            RD_ACK_MAN_ON           = 119,
            RD_RD_ACK_BOTH_ON       = 120,
            RD_I_NOTIFICATION       = 121,


            DC_BINARY               = 122,
            DC_TEXT                 = 123,
            DC_SMSC_DEFAULT         = 124,
            DC_ASCII_X34            = 125,
            DC_LATIN1               = 126,
            DC_JIS                  = 127,
            DC_CYRILIC              = 128,
            DC_LATIN_HEBREW         = 129,
            DC_UCS2                 = 130,
            DC_PICTOGRAM_ENC        = 131,
            DC_ISO_MUSIC_CODES      = 132,
            DC_E_KANJI_JIS          = 133,
            DC_KS_C_5601            = 134,
            DC_GSM_MWI              = 135,
            DC_GSM_MSG_CC           = 136,

            USSD_PSSD_IND           = 137,
            USSD_PSSR_IND           = 138,
            USSD_USSR_REQ           = 139,
            USSD_USSN_REQ           = 140,
            USSD_PSSD_RESP          = 141,
            USSD_PSSR_RESP          = 142,
            USSD_USSR_CONF          = 143,
            USSD_USSN_CONF          = 144,

            ST_ENROUTE              = 145,
            ST_DELIVERED            = 146, 
            ST_EXPIRED              = 147,
            ST_DELETED              = 148,
            ST_UNDELIVERABLE        = 149,
            ST_ACCEPTED             = 150,
            ST_UNKNOWN              = 151,
            ST_REJECTED             = 152,

            SMS_VALIDITY_PERIOD     = 153,
            SMS_MESSAGE_BODY        = 154,
            SMS_SVC_TYPE            = 155,

            USSD_DIALOG             = 156,
            MESSAGE_ID              = 157,
            STATUS                  = 158,
            PACKET_DIRECTION        = 159,

            STATUS_OK               = 160,
            STATUS_PERM_ERROR       = 161,
            STATUS_TEMP_ERROR       = 162,

            SMPP_DELIVERY_FAILURE_REASON     = 163,
            SMPP_NETWORK_ERROR_CODE          = 164,
            SMPP_ADDITIONAL_STATUS_INFO_TEXT = 165,
            SMPP_DPF_RESULT                  = 166,

            SLICING_REF_NUM                  = 167,
            SLICING_TOTAL_SEGMENTS           = 168,
            SLICING_SEGMENT_SEQNUM           = 169,
            BIGGEST_ADDITIONAL_TAG           = 169,

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

public:
    static AccessType CheckAccess(int handlerType,const std::string& name);

    SmppCommandAdapter( SmppCommand& _command ) :
    command(_command), src_sme_id(0), dst_sme_id(0)
    {
    }

    virtual ~SmppCommandAdapter();

    virtual void changed(AdapterProperty& property);
    virtual Property* getProperty(const std::string& name);
    //virtual void fillChargeOperation(smsc::inman::interaction::ChargeSms& op, TariffRec& tariffRec);
    //virtual void fillRespOperation(smsc::inman::interaction::DeliverySmsResult& op, TariffRec& tariffRec);

    virtual transport::SCAGCommand& getSCAGCommand() { return command; }
        
    bool hasServiceOp()
    {
        return CommandBridge::getSMS(command).hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP);
    }

    int getServiceOp()
    {
        return CommandBridge::getSMS(command).getIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP);
    }

    void setServiceOp(int value)
    {
        CommandBridge::getSMS(command).setIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP, value);
    }

    void setOA(Address& addr)
    {
        CommandBridge::getSMS(command).setOriginatingAddress(addr);
    }

    void setDA(Address& addr)
    {
        CommandBridge::getSMS(command).setDestinationAddress(addr);
    }

private:

    // invoked from getProperty
    // AdapterProperty* getDeliverProperty(SMS& data,const std::string& name,int FieldId);
    // AdapterProperty* getSubmitProperty(SMS& data,const std::string& name,int FieldId);
    // AdapterProperty* getDataSmProperty(SmsCommand& data,const std::string& name,int FieldId);
    // AdapterProperty* getRespProperty(SMS& data, const std::string& name,int FieldId);
    // AdapterProperty* getDataSmRespProperty(const std::string& name, int fieldId);

    // AdapterProperty* getGenericProperty(SMS& data, const std::string& name, int fieldId );
    AdapterProperty* createSmProperty(const std::string& name, int fieldId);
    AdapterProperty* createRespProperty(const std::string& name, int fieldId);

    // low level getters
    AdapterProperty* getESMBitProperty(SMS& data, const std::string& name,int FieldId);
    AdapterProperty* getRDBitProperty(SMS& data, const std::string& name,int FieldId);
    AdapterProperty* getDCBitProperty(SMS& data, const std::string& name,int FieldId);
    AdapterProperty* getUSSDBoolProperty(SMS& data, const std::string& name,int FieldId);
    AdapterProperty* getUnknownProperty(SMS& data, const std::string& name,int FieldId);
    AdapterProperty* getMessageBodyProperty(SMS& data, std::string name);
    AdapterProperty* getSlicingProperty(SMS& data, const std::string& name, int fieldId);

    AdapterProperty* getStrBitFromMask(int data,const std::string& name,int maskVal);

    // invoked from changed
    void writeSmField(int fieldId,AdapterProperty& property);
    // void writeDataSmField(SMS& data,int FieldId,AdapterProperty& property);
    // void writeDeliveryField( SMS& data, int fieldId, AdapterProperty& property);
    void writeRespField(int fieldId, AdapterProperty& property);

    bool isShortSize(int size) {return (size <= 254);}

    // void Set_DC_BIT_Property(SMS& data, int FieldId, bool value);
    // void SetBitMask(SMS& data, int tag, int mask);
    void setBitField(SMS& data, int smppTag, bool value, int tag);

private:
    // static fields
    static IntHash<AccessType> SubmitFieldsAccess;
    static IntHash<AccessType> DeliverFieldsAccess;
    static IntHash<AccessType> SubmitRespFieldsAccess;
    static IntHash<AccessType> DeliverRespFieldsAccess;

    static Hash<int> SubmitFieldNames;
    static Hash<int> DeliverFieldNames;
    static Hash<int> RespFieldNames;
    static Hash<int> DataSmRespFieldNames;

    // 16-bits values in hash: mask (8msb) + value (8lsb).
    static IntHash<int> tagToMaskAndValue;
    static std::multimap<int,int> exclusiveBitTags_;

    // field initializers
    static Hash<int> initFieldNames( EventHandlerType handlerType );
    static IntHash<AccessType> initFieldsAccess( EventHandlerType handlerType );

    // static void initCommonSmNames( Hash<int>& hs );
    // static Hash<int> InitSubmitFieldNames();
    // static Hash<int> InitDeliverFieldNames();
    // static void initCommonRespNames( Hash<int>& hs );
    // static Hash<int> InitRespFieldNames();
    // static Hash<int> InitDataSmRespFieldNames();

    // static void initCommonSmAccess( IntHash<AccessType>& hs );
    // static void initCommonRespAccess( IntHash<AccessType>& hs );
    // static IntHash<AccessType> InitSubmitAccess();
    // static IntHash<AccessType> InitDeliverAccess();
    // static IntHash<AccessType> InitSubmitRespAccess();
    // static IntHash<AccessType> InitDeliverRespAccess();

    static IntHash<int> initTagToMaskAndValue();
    static int bitFieldType( int tag );
    static std::multimap<int,int> initExclusiveBitTags();

private:
    SmppCommand&    command;
    IntHash<AdapterProperty *>  PropertyPul;
    AdapterProperty *src_sme_id, *dst_sme_id;
};

}}}

#endif // SCAG_RULE_ENGINE_SMPP_ADAPTER
