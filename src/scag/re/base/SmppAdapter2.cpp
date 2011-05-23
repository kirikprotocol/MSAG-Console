#include "SmppAdapter2.h"
#include "logger/Logger.h"
#include "CommandAdapter2.h"
#include "CommandBridge.h"
#include "scag/transport/smpp/base/SmppEntity2.h"
#include "scag/util/encodings/Encodings.h"
#include "scag/util/io/EndianConverter.h"
#include "scag/sessions/base/Session2.h"
#include "system/status.h"


namespace scag2 {
namespace re {
namespace smpp {

using namespace util::properties;
using namespace util::encodings;
using namespace smsc::system::Status;
using util::io::EndianConverter;

// === fields

Hash<int> SmppCommandAdapter::SubmitFieldNames = SmppCommandAdapter::initFieldNames(EH_SUBMIT_SM);
Hash<int> SmppCommandAdapter::DeliverFieldNames = SmppCommandAdapter::initFieldNames(EH_DELIVER_SM);
Hash<int> SmppCommandAdapter::RespFieldNames = SmppCommandAdapter::initFieldNames(EH_SUBMIT_SM_RESP);
Hash<int> SmppCommandAdapter::DataSmRespFieldNames = SmppCommandAdapter::initFieldNames(EH_DATA_SM_RESP);

IntHash<AccessType> SmppCommandAdapter::SubmitFieldsAccess = SmppCommandAdapter::initFieldsAccess(EH_SUBMIT_SM);
IntHash<AccessType> SmppCommandAdapter::DeliverFieldsAccess = SmppCommandAdapter::initFieldsAccess(EH_DELIVER_SM);
IntHash<AccessType> SmppCommandAdapter::SubmitRespFieldsAccess = SmppCommandAdapter::initFieldsAccess(EH_SUBMIT_SM_RESP);
IntHash<AccessType> SmppCommandAdapter::DeliverRespFieldsAccess = SmppCommandAdapter::initFieldsAccess(EH_DELIVER_SM_RESP);

IntHash<int>  SmppCommandAdapter::tagToMaskAndValue = SmppCommandAdapter::initTagToMaskAndValue();

std::multimap<int,int> SmppCommandAdapter::exclusiveBitTags_ = initExclusiveBitTags();

// === public

AccessType SmppCommandAdapter::CheckAccess(int handlerType, const std::string& name)
{
    if(!strcmp(name.c_str(), "src_sme_id") || !strcmp(name.c_str(), "dst_sme_id")) return atRead;

    Hash<int> *nameHash = 0, *nameHash2 = 0;
    IntHash<AccessType> *accessHash = 0, *accessHash2 = 0;
    switch (handlerType) {
    case EH_DATA_SM:
        nameHash = &SubmitFieldNames;
        accessHash = &SubmitFieldsAccess;
        nameHash2 = &DeliverFieldNames;
        accessHash2 = &DeliverFieldsAccess;
        break;
    case EH_DELIVER_SM:
        nameHash = &DeliverFieldNames;
        accessHash = &DeliverFieldsAccess;
        break;
    case EH_SUBMIT_SM:
        nameHash = &SubmitFieldNames;
        accessHash = &SubmitFieldsAccess;
        break;
    case EH_DATA_SM_RESP:
        nameHash = &DataSmRespFieldNames;
        accessHash = &SubmitRespFieldsAccess;
        accessHash2 = &DeliverRespFieldsAccess;
        break;
    case EH_DELIVER_SM_RESP:
        nameHash = &RespFieldNames;
        accessHash = &DeliverRespFieldsAccess;
        break;
    case EH_SUBMIT_SM_RESP:
        nameHash = &RespFieldNames;
        accessHash = &SubmitRespFieldsAccess;
        break;
    default:
        return atNoAccess;
    }

    int* pFieldId = 0;
    if (nameHash) { pFieldId = nameHash->GetPtr(name.c_str()); }
    if (!pFieldId && nameHash2 ) { pFieldId = nameHash2->GetPtr(name.c_str()); }
    if (!pFieldId) return atNoAccess;

    AccessType* actype1 = accessHash ? accessHash->GetPtr(*pFieldId) : 0;
    AccessType* actype2 = accessHash2 ? accessHash2->GetPtr(*pFieldId) : 0;
    if (actype1 && actype2) {
        // we are taking higher access
        return std::max(*actype1,*actype2);
    }
    if (actype1) return *actype1;
    if (actype2) return *actype2;
    return atReadWrite;
}


SmppCommandAdapter::~SmppCommandAdapter()
{
    int key = 0;
    AdapterProperty * value = 0;
    
    for (IntHash <AdapterProperty *>::Iterator it = PropertyPul.First(); it.Next(key, value);)
    {
        delete value;
    }
    if(src_sme_id) delete src_sme_id;
    if(dst_sme_id) delete dst_sme_id;
}


Property* SmppCommandAdapter::getProperty(const std::string& name)
{
    if(!strcmp(name.c_str(), "src_sme_id"))
    {
        if(!src_sme_id)
            src_sme_id = new AdapterProperty(name.c_str(), this, command.getEntity()->getSystemId());
        return src_sme_id;
    }
    else if(!strcmp(name.c_str(), "dst_sme_id"))
    {
        if(!dst_sme_id)
            dst_sme_id = new AdapterProperty(name.c_str(), this, command.getDstEntity()->getSystemId());
        return dst_sme_id;
    }

    const CommandId cmdid = CommandId(command.getCommandId());
    Hash<int>* nameHash;
    IntHash<AccessType>* accessHash;
    switch (cmdid) 
    {
    case DELIVERY:
        nameHash = &DeliverFieldNames;
        accessHash = &DeliverFieldsAccess;
        break;
    case SUBMIT:
        nameHash = &SubmitFieldNames;
        accessHash = &SubmitFieldsAccess;
        break;
    case DATASM: {
        const DataSmDirection dir = command.get_smsCommand().dir;
        if ( dir == dsdSc2Srv ) {
            nameHash = &DeliverFieldNames;
            accessHash = &DeliverFieldsAccess;
        } else if ( dir == dsdSrv2Sc ) {
            nameHash = &SubmitFieldNames;
            accessHash = &SubmitFieldsAccess;
        } else {
            return 0;
        }
        break;
    }
    case DATASM_RESP: {
        nameHash = &DataSmRespFieldNames;
        SmsResp* resp = command.get_resp();
        if (!resp) return 0;
        const DataSmDirection dir = resp->get_dir();
        if ( dir == dsdSc2Srv ) {
            accessHash = &DeliverRespFieldsAccess;
        } else if (dir == dsdSrv2Sc) {
            accessHash = &SubmitRespFieldsAccess;
        } else {
            return 0;
        }
        break;
    }
    case SUBMIT_RESP:
        nameHash = &RespFieldNames;
        accessHash = &SubmitRespFieldsAccess;
        break;
    case DELIVERY_RESP:
        nameHash = &RespFieldNames;
        accessHash = &DeliverRespFieldsAccess;
        break;
    default:
        return 0;
    } // switch cmdid

    int* pFieldId = nameHash->GetPtr(name.c_str());
    if (!pFieldId) return 0;

    AdapterProperty** propertyPtr = PropertyPul.GetPtr(*pFieldId);
    if (propertyPtr) return *propertyPtr;

    // checking access
    AccessType* actype = accessHash->GetPtr(*pFieldId);
    if (actype && *actype == atNoAccess) return 0;

    // property was not accessed yet, creating
    AdapterProperty* property = 0;
    switch (cmdid) {
    case SUBMIT:
    case DELIVERY:
    case DATASM: {
        property = createSmProperty(name,*pFieldId);
        break;
    }
    case SUBMIT_RESP:
    case DELIVERY_RESP:
    case DATASM_RESP: {
        property = createRespProperty(name,*pFieldId);
        break;
    }
    default:
        return 0;
    }

    if (property) PropertyPul.Insert(*pFieldId,property);
    return property;
}


void SmppCommandAdapter::delProperty( const std::string& name )
{
    smsc::logger::Logger* logr = smsc::logger::Logger::getInstance("smpp.adapt");
    smsc_log_warn(logr,"delProperty(%s) is not implemented",name.c_str());
}


AdapterProperty* SmppCommandAdapter::createSmProperty( const std::string& name, int fieldId )
{
    AdapterProperty* property = 0;

    do { // fake do

        if (fieldId == PACKET_DIRECTION) {
            property = new AdapterProperty(name.c_str(),this,command.get_smsCommand().dir);
            break;
        }

        SMS* sms = command.get_sms();
        if (!sms) break;
        SMS& data = *sms;

        switch ( fieldId ) {
        case OA:
            property = new AdapterProperty(name.c_str(),this,data.getOriginatingAddress().toString().c_str());
            break;
        case DA:
            property = new AdapterProperty(name.c_str(),this,data.getDestinationAddress().toString().c_str());
            break;
        case SMS_MESSAGE_BODY:
            property = getMessageBodyProperty(data, name);
            break;
        case SMS_SVC_TYPE:
            property = new AdapterProperty(name.c_str(),this,data.eServiceType);
            break;
        case USSD_DIALOG:
            property = new AdapterProperty(name.c_str(),this,data.hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP));
            break;
        default: break;
        } // switch
    
        if ( property ) { break; }

        // generic
        if ( (fieldId >= ESM_MM_SMSC_DEFAULT) && (fieldId <= ESM_NSF_BOTH) ) {
            property = getESMBitProperty(data,name,fieldId);
            break;
        }

        if ( (fieldId >= RD_RECEIPT_OFF) && (fieldId <= RD_I_NOTIFICATION) ) {
            property = getRDBitProperty(data,name,fieldId);
            break;
        }

        if ( (fieldId >= DC_BINARY) && (fieldId <= DC_GSM_MSG_CC) ) {
            property = getDCBitProperty(data,name,fieldId);
            break;
        }

        if ( (fieldId >= USSD_PSSD_IND) && (fieldId <= USSD_USSN_CONF) ) {
            property = getUSSDBoolProperty(data,name,fieldId);
            break;
        }

        if ( (fieldId >= SLICING_REF_NUM) && (fieldId <= SLICING_SEGMENT_SEQNUM) ) {
            property = getSlicingProperty(data,name,fieldId);
            break;
        }

        // delivery specific
        if ( (fieldId >= ST_ENROUTE) && (fieldId <= ST_REJECTED) ) {
            property = new AdapterProperty(name.c_str(),this,data.getState());
            break;
        }

        // submit specific
        if ((fieldId >= OPTIONAL_CHARGING)&&(fieldId <= OPTIONAL_EXPECTED_MESSAGE_CONTENT_TYPE)) {
            property = getUnknownProperty(data, name, fieldId);
            break;
        }
        if (fieldId == SMS_VALIDITY_PERIOD ) {
            property = new AdapterProperty(name.c_str(),this,data.validTime);
            break;
        }
    
        if (fieldId >= SMALLEST_ADDITIONAL_TAG && fieldId <= BIGGEST_ADDITIONAL_TAG) {
            smsc::logger::Logger* logr = smsc::logger::Logger::getInstance("smpp.adapt");
            smsc_log_warn(logr,"requested field #%u/%s is not handled",fieldId,name.c_str());
            break;
        }

        // sms tag property
        int tagType = (fieldId >> 8);
        if (tagType == SMS_STR_TAG) 
            property = new AdapterProperty(name.c_str(),this, data.hasStrProperty(fieldId) ? data.getStrProperty(fieldId).c_str() : "");
        else if (tagType == SMS_INT_TAG) 
            property = new AdapterProperty(name.c_str(), this, data.hasIntProperty(fieldId) ? data.getIntProperty(fieldId) : 0);

    } while (false); // fake do
    return property;
}


AdapterProperty* SmppCommandAdapter::createRespProperty(const std::string& name, int fieldId)
{
    AdapterProperty* property = 0;
    do {

        SmsResp* resp = command.get_resp();
        if (!resp) break;

        // data sm resp
        switch (fieldId) {

        case PACKET_DIRECTION:
            property = new AdapterProperty(name.c_str(),this,resp->get_dir());
            break;

        case SMPP_ADDITIONAL_STATUS_INFO_TEXT:
            property = new AdapterProperty(name.c_str(),this,resp->getAdditionalStatusInfoText());
            break;
        case SMPP_DELIVERY_FAILURE_REASON: {
            int val = resp->hasDeliveryFailureReason() ? resp->getDeliveryFailureReason() : -1;
            property = new AdapterProperty(name.c_str(),this,val);
            break;
        }
        case SMPP_DPF_RESULT:
            property = new AdapterProperty(name.c_str(),this,resp->getDpfResult());
            break;
        case SMPP_NETWORK_ERROR_CODE:
            property = new AdapterProperty(name.c_str(),this,resp->getNetworkErrorCode());
            break;
        case STATUS:
            property = new AdapterProperty(name.c_str(),this,command.get_status());
            break;
        case MESSAGE_ID:
            property = new AdapterProperty(name.c_str(),this,resp->get_messageId());
            break;
        case USSD_DIALOG: {
            SMS* data = resp->get_sms();
            if (data) {
                property = new AdapterProperty(name.c_str(),this,data->hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP));
            }
            break;
        }
        case OA: {
            SMS* data = resp->get_sms();
            if (data) {
                property = new AdapterProperty(name.c_str(),this,data->getOriginatingAddress().toString());
            }
            break;
        }
        case DA: {
            SMS* data = resp->get_sms();
            if (data) {
                property = new AdapterProperty(name.c_str(),this,data->getDestinationAddress().toString());
            }
            break;
        }
        case STATUS_OK:
            property = new AdapterProperty(name.c_str(),this,(command.get_status() == 0));
            break;
        case STATUS_PERM_ERROR:
            if (command.get_status() > 0)
                property = new AdapterProperty(name.c_str(),this,isErrorPermanent(command.get_status()));
            else
                property = new AdapterProperty(name.c_str(),this,0);
            break;
        case STATUS_TEMP_ERROR:
            if (command.get_status() > 0) 
                property = new AdapterProperty(name.c_str(),this,!isErrorPermanent(command.get_status()));
            else
                property = new AdapterProperty(name.c_str(),this,0);
            break;

        default: break;
        } // switch
        
        if (property) break;

        if (fieldId >= SMALLEST_ADDITIONAL_TAG && fieldId <= BIGGEST_ADDITIONAL_TAG) {
            smsc::logger::Logger* logr = smsc::logger::Logger::getInstance("smpp.adapt");
            smsc_log_warn(logr,"requested resp field #%u/%s is not handled",fieldId,name.c_str());
            break;
        }

    } while (false);
    return property;
}


void SmppCommandAdapter::changed(AdapterProperty& property)
{
    CommandId cmdid = CommandId(command.getCommandId());

    Hash<int>* nameHash;
    IntHash<AccessType>* accessHash;
    switch (cmdid) 
    {
    case DELIVERY:
        nameHash = &DeliverFieldNames;
        accessHash = &DeliverFieldsAccess;
        break;
    case SUBMIT:
        nameHash = &SubmitFieldNames;
        accessHash = &SubmitFieldsAccess;
        break;
    case DATASM: {
        const DataSmDirection dir = command.get_smsCommand().dir;
        if (dir == dsdSc2Srv) {
            nameHash = &DeliverFieldNames;
            accessHash = &DeliverFieldsAccess;
        } else if (dir == dsdSrv2Sc) {
            nameHash = &SubmitFieldNames;
            accessHash = &SubmitFieldsAccess;
        } else {
            return;
        }
        break;
    }
    case SUBMIT_RESP:
        nameHash = &RespFieldNames;
        accessHash = &SubmitRespFieldsAccess;
        break;
    case DELIVERY_RESP:
        nameHash = &RespFieldNames;
        accessHash = &DeliverRespFieldsAccess;
        break;
    case DATASM_RESP: {
        nameHash = &DataSmRespFieldNames;
        SmsResp* resp = command.get_resp();
        if (!resp) return;
        const DataSmDirection dir = resp->get_dir();
        if ( dir == dsdSc2Srv ) {
            accessHash = &DeliverRespFieldsAccess;
        } else if (dir == dsdSrv2Sc) {
            accessHash = &SubmitRespFieldsAccess;
        } else {
            return;
        }
        break;
    }
    default: break;
    }
        
    const Property::string_type& name = property.getName();
    int* pFieldId = nameHash->GetPtr(name.c_str());
    if (!pFieldId) return;

    AccessType* actype = accessHash->GetPtr(*pFieldId);
    if (actype && *actype <= atRead) return;

    // allowed to write
    switch (cmdid) {
    case SUBMIT:
    case DELIVERY:
    case DATASM:
        writeSmField(*pFieldId,property);
        break;
    case SUBMIT_RESP:
    case DELIVERY_RESP:
    case DATASM_RESP:
        writeRespField(*pFieldId,property);
        break;
    default: break;
    } // switch
}


AdapterProperty* SmppCommandAdapter::getESMBitProperty(SMS& data, const std::string& name,int fieldId)
{
    const int* maskValue = tagToMaskAndValue.GetPtr(fieldId);
    if (!maskValue) return 0;
    return getStrBitFromMask(data.getIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS),name,*maskValue);
}


AdapterProperty* SmppCommandAdapter::getRDBitProperty(SMS& data, const std::string& name,int fieldId)
{
    const int* maskValue = tagToMaskAndValue.GetPtr(fieldId);
    if (!maskValue) return 0;
    return getStrBitFromMask(data.getIntProperty(smsc::sms::Tag::SMPP_REGISTRED_DELIVERY),name,*maskValue);
}


AdapterProperty* SmppCommandAdapter::getDCBitProperty(SMS& data, const std::string& name,int fieldId)
{
    const int val = data.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING);
    if ( (fieldId == DC_BINARY) || (fieldId == DC_TEXT) ) {
        const bool binary = ((val == 0x02) || (val == 0x04));
        // we use boolean comparison instead of xor
        return new AdapterProperty(name.c_str(),this, (fieldId == DC_BINARY) == binary );
    }
    const int* maskValue = tagToMaskAndValue.GetPtr(fieldId);
    if (!maskValue) return 0;
    return getStrBitFromMask(val,name,*maskValue);
}


AdapterProperty* SmppCommandAdapter::getUSSDBoolProperty(SMS& data, const std::string& name,int fieldId)
{
    if (!data.hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP)) return 0;

    int sop = data.getIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP);

    bool b = false;
    switch (fieldId) 
    {
        case USSD_PSSD_IND:b = sop == PSSD_INDICATION; break;
        case USSD_PSSR_IND: b = sop ==  PSSR_INDICATION; break;
        case USSD_USSR_REQ: b = sop ==  USSR_REQUEST; break;
        case USSD_USSN_REQ:b = sop ==  USSN_REQUEST; break;
        case USSD_PSSD_RESP: b = sop ==  PSSD_RESPONSE; break;
        case USSD_PSSR_RESP: b = sop ==  PSSR_RESPONSE; break;
        case USSD_USSR_CONF: b = sop ==  USSR_CONFIRM; break;
        case USSD_USSN_CONF: b = sop ==  USSN_CONFIRM; break;
    }
    return new AdapterProperty(name.c_str(), this, b);
}


AdapterProperty* SmppCommandAdapter::getUnknownProperty(SMS& data, const std::string& name,int FieldId)
{
    if (!data.hasBinProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS)) return 0;

    unsigned int len;
    const char * buff = data.getBinProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS, &len);

    uint16_t value;
    uint16_t valueLen;
    if (len < 5) return 0;

    unsigned int i = 0;
    while (i < (len - 4)) 
    {
        value = EndianConverter::get16(buff + i);
        valueLen = EndianConverter::get16(buff + i + 2);

        if (valueLen <=0) return 0;
        if ((i + 4 + valueLen) > len) return 0;

        if (value == FieldId) 
        {
            if (valueLen == 1)
            {
                char temp = buff[i + 4];
                return new AdapterProperty(name.c_str(), this, temp);
//                return new AdapterProperty(name.c_str(), this, (temp > 0));
            }

            std::string str;
            str.append((buff + i + 4),valueLen);
            return new AdapterProperty(name.c_str(), this, str.c_str());
        }

        //if ((i + valueLen + 4 + 2) > (len-1)) return 0;
        i = i + valueLen + 4;

    }
    return 0;
}



AdapterProperty * SmppCommandAdapter::getMessageBodyProperty(SMS& data, std::string name)
{
    return new AdapterProperty(name.c_str(),this,CommandBridge::getMessageBody(command));
}


AdapterProperty* SmppCommandAdapter::getSlicingProperty( SMS& data,
                                                         const std::string& name,
                                                         int FieldId )
{
    AdapterProperty* property = 0;
    int sarmr;
    int lastIndex;
    int currentIndex;
    SmppCommand::getSlicingParameters(data,sarmr,currentIndex,lastIndex);
    switch (FieldId) {
    case SLICING_REF_NUM :
        property = new AdapterProperty(name.c_str(),this,sarmr);
        break;
    case SLICING_TOTAL_SEGMENTS :
        property = new AdapterProperty(name.c_str(),this,lastIndex);
        break;
    case SLICING_SEGMENT_SEQNUM :
        property = new AdapterProperty(name.c_str(),this,currentIndex);
        break;
    }
    return property;
}


AdapterProperty* SmppCommandAdapter::getStrBitFromMask(int data, const std::string& name, int maskVal)
{
    const int mask = (maskVal >> 8) & 0xff;
    return new AdapterProperty(name.c_str(),this, (data & mask) == (maskVal & 0xff));
}


/*
void SmppCommandAdapter::Set_DC_BIT_Property(SMS& data,int FieldId,bool value)
{
    
    if(FieldId == DC_BINARY) 
        data.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,value ? 2 : 4); //00000010
    else if(FieldId == DC_TEXT) {
        data.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, value ? 4 : 2); //00000100
    } else if(FieldId >= DC_SMSC_DEFAULT && FieldId <= DC_GSM_MSG_CC) 
    {
        //TODO: check DC_TEXT field first!
        if(!(data.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING) & 4)) return;
        
        int dc = 0;

        if(value)
        {
            switch (FieldId) 
            {
                case DC_ASCII_X34:      dc = 1; break;
                case DC_LATIN1:         dc = 3; break; //00000011
                case DC_JIS:            dc = 5; break; //00000101
                case DC_CYRILIC:        dc = 6; break; //00000110
                case DC_LATIN_HEBREW:   dc = 7; break; //00000111
                case DC_UCS2:           dc = 8; break; //00001000
                case DC_PICTOGRAM_ENC:  dc = 9; break; //00001001
                case DC_ISO_MUSIC_CODES: dc = 10; break; //00001010
                case DC_E_KANJI_JIS:    dc = 11; break; //00001101
                case DC_KS_C_5601:      dc = 12; break; //00001110
                case DC_GSM_MWI:        dc = 192 | (data.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING) & 15); break;
                case DC_GSM_MSG_CC:     dc = 240 | (data.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING) & 15); break; //1111xxxx
            }
        }
        else if(FieldId == DC_GSM_MWI)
            dc = 240 | (data.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING) & 15);
        else if(FieldId == DC_GSM_MSG_CC)
            dc = 192|(data.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING) & 15);

        data.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, dc);
    }
}
 */


/*
void SmppCommandAdapter::writeDataSmField(SMS& data,int FieldId,AdapterProperty& property)
{
    writeDeliveryField(data,FieldId,property);
}


void SmppCommandAdapter::writeSubmitField(SMS& data,int FieldId,AdapterProperty& property)
{
    writeDeliveryField(data,FieldId,property);
}
 */


void SmppCommandAdapter::writeSmField(int fieldId,AdapterProperty& property)
{
    SMS* sms = command.get_sms();
    if (!sms) return;
    SMS& data = *sms;

    if (fieldId == OA) 
        AssignAddress(data.originatingAddress, property.getStr().c_str());
    else if (fieldId == DA) 
        AssignAddress(data.destinationAddress, property.getStr().c_str());
    /*
    else if ((fieldId >= DC_BINARY)&&(fieldId <= DC_GSM_MSG_CC)) 
        Set_DC_BIT_Property(data,fieldId,property.getBool());
     */
    else if ( (fieldId >= ESM_MM_SMSC_DEFAULT) && (fieldId <= ESM_NSF_BOTH) ) {

        setBitField(data,smsc::sms::Tag::SMPP_ESM_CLASS,property.getBool(),fieldId);

    } else if ( (fieldId >= RD_RECEIPT_OFF) && (fieldId <= RD_I_NOTIFICATION) ) {

        setBitField(data,smsc::sms::Tag::SMPP_REGISTRED_DELIVERY,property.getBool(),fieldId);

    } else {

        switch (fieldId) {
        case smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE: {
            uint32_t flag = 0;
            if ( data.hasIntProperty(fieldId) ) {
                flag = data.getIntProperty(fieldId) & 0x80000000;
            }
            const uint32_t umr = (uint32_t(property.getInt()) & 0x7fffffff) | flag;
            data.setIntProperty(fieldId,umr);
            break;
        }
        case smsc::sms::Tag::SMPP_SM_LENGTH:
        case smsc::sms::Tag::SMPP_USER_RESPONSE_CODE:
        case smsc::sms::Tag::SMPP_LANGUAGE_INDICATOR:
        case smsc::sms::Tag::SMPP_SOURCE_PORT:
        case smsc::sms::Tag::SMPP_DESTINATION_PORT:
            data.setIntProperty(fieldId, unsigned(property.getInt()));
            break;
        case smsc::sms::Tag::SMPP_RECEIPTED_MESSAGE_ID:
            data.setStrProperty(fieldId, property.getStr().c_str());
            break;
        case SMS_SVC_TYPE: {
            const Property::string_type& str = property.getStr();
            size_t len = 0;

            if (str.size() >= unsigned(smsc::sms::MAX_ESERVICE_TYPE_LENGTH))
                len = smsc::sms::MAX_ESERVICE_TYPE_LENGTH;
            else
                len = str.size();

            memcpy( data.eServiceType, str.c_str(), len );
            data.eServiceType[len] = 0;
            break;
        }

        case SMS_MESSAGE_BODY: {
            const Property::string_type& str = property.getStr();
            std::string resStr;

            int code = smsc::smpp::DataCoding::UCS2;

            if(!CommandBridge::hasMSB(str.data(), str.size()))
                code = smsc::smpp::DataCoding::SMSC7BIT;
                
            data.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, code);
            
            switch (code) 
            {
                case smsc::smpp::DataCoding::UCS2:
                    Convertor::UTF8ToUCS2(str.data(), str.size(), resStr);
                    break;
                default:
                    Convertor::UTF8ToGSM7Bit(str.data(), str.size(), resStr);
            }
  
            if (isShortSize(resStr.size()) && data.hasBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE)) 
            {
                unsigned mlen;
                data.getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &mlen);
                if (mlen) 
                {
                   data.setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, resStr.data(), resStr.size());
                   data.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH, resStr.size());
                   return;
                }
            }

            if (data.hasBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE))
                data.setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, 0, 0);

            data.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, resStr.data(), resStr.size());
            data.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH, 0);

            break;
        }

        default: {
            smsc::logger::Logger* logr = smsc::logger::Logger::getInstance("smpp.adapt");
            smsc_log_warn(logr,"requested field(w) #%u/%s is not handled",fieldId,property.getName().c_str());
            break;
        }
        } // switch
    } // if
}


void SmppCommandAdapter::writeRespField(int fieldId, AdapterProperty& property) {
    if (fieldId == STATUS) {
        command.set_status(int(property.getInt()));
        return;
    }

    SmsResp* resp = command.get_resp();
    if (!resp) { return; }

    switch (fieldId) {
    case SMPP_ADDITIONAL_STATUS_INFO_TEXT:
        resp->setAdditionalStatusInfoText(property.getStr().c_str());
        break;
    case SMPP_DELIVERY_FAILURE_REASON:
        resp->setDeliveryFailureReason(uint8_t(property.getInt()));
        break;
    case SMPP_DPF_RESULT:
        resp->setDpfResult(uint8_t(property.getInt()));
        break;
    case SMPP_NETWORK_ERROR_CODE:
        resp->setNetworkErrorCode(unsigned(property.getInt()));
        break;
    case MESSAGE_ID:
        resp->set_messageId(property.getStr().c_str());
        break;
    default: {
        smsc::logger::Logger* logr = smsc::logger::Logger::getInstance("smpp.adapt");
        smsc_log_warn(logr,"requested resp field(w) #%u/%s is not handled",fieldId,property.getName().c_str());
        break;
    }
    }
}


void SmppCommandAdapter::setBitField(SMS& data, int smppTag, bool value, int tag )
{
    int* maskValue = tagToMaskAndValue.GetPtr(tag);
    if (!maskValue) return;
    // we cannot unset complex values
    if ( value == false && (*maskValue & 0x10000) == 0 ) return;

    int oldVal = data.getIntProperty(smppTag);
    oldVal &= (~(*maskValue >> 8) & 0xff);
    if ( value ) oldVal |= (*maskValue & 0xff);
    data.setIntProperty(smppTag,oldVal);

    // resetting exclusive fields
    typedef std::multimap<int,int>::const_iterator eiter;
    std::pair< eiter, eiter > range = exclusiveBitTags_.equal_range(tag);
    AdapterProperty** ptr;
    for ( eiter i = range.first; i != range.second; ++i ) {
        ptr = PropertyPul.GetPtr(i->second);
        if (ptr) (*ptr)->setBool(false);
    }
}


/*
void SmppCommandAdapter::SetBitMask(SMS& data, int tag, int mask)
{
    if (data.hasIntProperty(tag)) 
        data.setIntProperty(tag, data.getIntProperty(tag) & mask);
}
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


IntHash<AccessType> SmppCommandAdapter::initFieldsAccess( EventHandlerType eh )
{
    IntHash<AccessType> hs;

    hs.Insert(PACKET_DIRECTION,atRead);

    switch (eh) {
    case EH_SUBMIT_SM_RESP:
    case EH_DATA_SM_RESP:
        hs.Insert(MESSAGE_ID,atReadWrite);
    case EH_DELIVER_SM_RESP:
        hs.Insert(OA,atRead);
        hs.Insert(DA,atRead);
        hs.Insert(STATUS_OK,atRead);
        hs.Insert(STATUS_PERM_ERROR,atRead);
        hs.Insert(STATUS_TEMP_ERROR,atRead);
        hs.Insert(USSD_DIALOG,atRead);
        break;
        
    case EH_SUBMIT_SM:
    case EH_DELIVER_SM:
    case EH_DATA_SM:
        hs.Insert(smsc::sms::Tag::SMPP_PROTOCOL_ID,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_PRIORITY,atRead);
        // hs.Insert(smsc::sms::Tag::SMPP_SCHEDULE_DELIVERY_TIME ,atRead);
        hs.Insert(SMS_VALIDITY_PERIOD              ,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_REPLACE_IF_PRESENT_FLAG,atRead);
        hs.Insert(DC_BINARY         ,atRead);
        hs.Insert(DC_TEXT           ,atRead);
        hs.Insert(DC_SMSC_DEFAULT   ,atRead);
        hs.Insert(DC_ASCII_X34      ,atRead);
        hs.Insert(DC_LATIN1         ,atRead);
        hs.Insert(DC_JIS            ,atRead);
        hs.Insert(DC_CYRILIC        ,atRead);
        hs.Insert(DC_LATIN_HEBREW   ,atRead);
        hs.Insert(DC_UCS2           ,atRead);
        hs.Insert(DC_PICTOGRAM_ENC  ,atRead);
        hs.Insert(DC_ISO_MUSIC_CODES,atRead);
        hs.Insert(DC_E_KANJI_JIS    ,atRead);
        hs.Insert(DC_KS_C_5601      ,atRead);
        hs.Insert(DC_GSM_MWI        ,atRead);
        hs.Insert(DC_GSM_MSG_CC     ,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_SM_LENGTH,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_DEST_ADDR_SUBUNIT    ,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_SAR_MSG_REF_NUM      ,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_SAR_TOTAL_SEGMENTS   ,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_SAR_SEGMENT_SEQNUM   ,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_PAYLOAD_TYPE         ,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_MORE_MESSAGES_TO_SEND,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_MS_VALIDITY          ,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_NUMBER_OF_MESSAGES   ,atRead);
        hs.Insert(smsc::sms::Tag::SMPP_NETWORK_ERROR_CODE   ,atRead);
        hs.Insert(ST_ENROUTE      ,atRead);
        hs.Insert(ST_DELIVERED    ,atRead);
        hs.Insert(ST_EXPIRED      ,atRead);
        hs.Insert(ST_DELETED      ,atRead);
        hs.Insert(ST_UNDELIVERABLE,atRead);
        hs.Insert(ST_ACCEPTED     ,atRead);
        hs.Insert(ST_UNKNOWN      ,atRead);
        hs.Insert(ST_REJECTED     ,atRead);
        // hs.Insert(smsc::sms::Tag::SMPP_RECEIPTED_MESSAGE_ID,atRead);
        hs.Insert(USSD_DIALOG   ,atRead);
        hs.Insert(USSD_PSSD_IND ,atRead);
        hs.Insert(USSD_PSSR_IND ,atRead);
        hs.Insert(USSD_USSR_REQ ,atRead);
        hs.Insert(USSD_USSN_REQ ,atRead);
        hs.Insert(USSD_PSSD_RESP,atRead);
        hs.Insert(USSD_PSSR_RESP,atRead);
        hs.Insert(USSD_USSR_CONF,atRead);
        hs.Insert(USSD_USSN_CONF,atRead);
        hs.Insert(SLICING_REF_NUM       ,atRead);
        hs.Insert(SLICING_TOTAL_SEGMENTS,atRead);
        hs.Insert(SLICING_SEGMENT_SEQNUM,atRead);
        break;

    default:
        break;
    }

    // additional fields
    if ( eh == EH_DELIVER_SM_RESP ) {
        hs.Insert(MESSAGE_ID,atNoAccess);
    }
    return hs;
}

/*
IntHash<AccessType> SmppCommandAdapter::InitRespAccess()
{
    IntHash<AccessType> hs;
    initCommonRespAccess(hs);
    return hs;
}

IntHash<AccessType> SmppCommandAdapter::InitDataSmRespAccess()
{
    IntHash<AccessType> hs;
    initCommonRespAccess(hs);
    // hs.Insert(SMPP_ADDITIONAL_STATUS_INFO_TEXT, atReadWrite);
    // hs.Insert(SMPP_DELIVERY_FAILURE_REASON, atReadWrite);
    // hs.Insert(SMPP_DPF_RESULT, atReadWrite);
    // hs.Insert(SMPP_NETWORK_ERROR_CODE, atReadWrite);
    return hs;
}
 */


Hash<int> SmppCommandAdapter::initFieldNames( EventHandlerType eh )
{
    Hash<int> hs;

    switch (eh) {

    case EH_SUBMIT_SM:
    case EH_DELIVER_SM:
    case EH_DATA_SM:

        hs["packet_direction"]              = PACKET_DIRECTION;
        hs["svc_type"]                      = SMS_SVC_TYPE;
        hs["OA"]                            = OA;
        hs["DA"]                            = DA;

        hs["esm_mm_smsc_default"]           = ESM_MM_SMSC_DEFAULT;
        hs["esm_mm_datagram"]               = ESM_MM_DATAGRAM;
        hs["esm_mm_forward"]                = ESM_MM_FORWARD;
        hs["esm_mm_s_and_f"]                = ESM_MM_S_AND_F;
        hs["esm_mt_default"]                = ESM_MT_DEFAULT;
        hs["esm_mt_delivery_ack"]           = ESM_MT_DELIVERY_ACK;
        hs["esm_mt_manual_ack"]             = ESM_MT_MANUAL_ACK;
        hs["esm_nsf_none"]                  = ESM_NSF_NONE;
        hs["esm_nsf_udhi"]                  = ESM_NSF_UDHI;
        hs["esm_nsf_srp"]                   = ESM_NSF_SRP;
        hs["esm_nsf_both"]                  = ESM_NSF_BOTH;

        hs["protocol_id"]                   = smsc::sms::Tag::SMPP_PROTOCOL_ID; 
        hs["priority_flag"]                 = smsc::sms::Tag::SMPP_PRIORITY; 

        hs["rd_receipt_off"]                = RD_RECEIPT_OFF;
        hs["rd_receipt_on"]                 = RD_RECEIPT_ON;
        hs["rd_receipt_failure"]            = RD_RECEIPT_FAILURE;
        hs["rd_ack_off"]                    = RD_ACK_OFF;
        hs["rd_ack_on"]                     = RD_ACK_ON;
        hs["rd_ack_man_on"]                 = RD_ACK_MAN_ON;
        hs["rd_ack_both_on"]                = RD_RD_ACK_BOTH_ON;
        hs["rd_i_notification"]             = RD_I_NOTIFICATION;

        hs["dc_binary"]                     = DC_BINARY;
        hs["dc_text"]                       = DC_TEXT;
        hs["dc_smsc_default"]               = DC_SMSC_DEFAULT;
        hs["dc_ascii_x34"]                  = DC_ASCII_X34;
        hs["dc_latin1"]                     = DC_LATIN1;
        hs["dc_jis"]                        = DC_JIS;
        hs["dc_cyrilic"]                    = DC_CYRILIC;
        hs["dc_latin_hebrew"]               = DC_LATIN_HEBREW;
        hs["dc_ucs2"]                       = DC_UCS2;
        hs["dc_pictogram_enc"]              = DC_PICTOGRAM_ENC;
        hs["dc_iso_music_codes"]            = DC_ISO_MUSIC_CODES;
        hs["dc_e_kanji_jis"]                = DC_E_KANJI_JIS;
        hs["dc_ks_c_5601"]                  = DC_KS_C_5601;
        hs["dc_gsm_mwi"]                    = DC_GSM_MWI;
        hs["dc_gsm_msg_cc"]                 = DC_GSM_MSG_CC;    

        hs["sm_length"]                     = smsc::sms::Tag::SMPP_SM_LENGTH;
        hs["message_text"]                  = SMS_MESSAGE_BODY;

        // --- optional

        hs["umr"]                           = smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE;
        hs["source_port"]                   = smsc::sms::Tag::SMPP_SOURCE_PORT; 
        hs["destination_port"]              = smsc::sms::Tag::SMPP_DESTINATION_PORT; 

        hs["sar_msg_ref_num"]               = smsc::sms::Tag::SMPP_SAR_MSG_REF_NUM;
        hs["sar_total_segments"]            = smsc::sms::Tag::SMPP_SAR_TOTAL_SEGMENTS; 
        hs["sar_segment_seqnum"]            = smsc::sms::Tag::SMPP_SAR_SEGMENT_SEQNUM;

        hs["payload_type"]                  = smsc::sms::Tag::SMPP_PAYLOAD_TYPE; 
        // hs["privacy_indicator"]
        // hs["callback_num"]

        hs["user_response_code"]            = smsc::sms::Tag::SMPP_USER_RESPONSE_CODE;

        hs["language_indicator"]            = smsc::sms::Tag::SMPP_LANGUAGE_INDICATOR;
        hs["its_session_info"]              = smsc::sms::Tag::SMPP_ITS_SESSION_INFO;

        hs["ussd_dialog"]                   = USSD_DIALOG;
        hs["ussd_pssd_ind"]                 = USSD_PSSD_IND;
        hs["ussd_pssr_ind"]                 = USSD_PSSR_IND;
        hs["ussd_ussr_req"]                 = USSD_USSR_REQ;
        hs["ussd_ussn_req"]                 = USSD_USSN_REQ;
        hs["ussd_pssd_resp"]                = USSD_PSSD_RESP;
        hs["ussd_pssr_resp"]                = USSD_PSSR_RESP;
        hs["ussd_ussr_conf"]                = USSD_USSR_CONF;
        hs["ussd_ussn_conf"]                = USSD_USSN_CONF;

        // --- generated

        hs["slicing_ref_num"]               = SLICING_REF_NUM;
        hs["slicing_total_segments"]        = SLICING_TOTAL_SEGMENTS;
        hs["slicing_segment_seqnum"]        = SLICING_SEGMENT_SEQNUM;

        break;

    case EH_SUBMIT_SM_RESP:
    case EH_DELIVER_SM_RESP:
    case EH_DATA_SM_RESP:
        hs["packet_direction"]              = PACKET_DIRECTION;
        hs["OA"]                            = OA;
        hs["DA"]                            = DA;
        hs["status"]                        = STATUS;
        hs["message_id"]                    = MESSAGE_ID; // submit_resp
        hs["ussd_dialog"]                   = USSD_DIALOG;
        hs["status_ok"]                     = STATUS_OK;
        hs["status_perm_error"]             = STATUS_PERM_ERROR;
        hs["status_temp_error"]             = STATUS_TEMP_ERROR;
        break;

    default : break;
    }

    // --- additional fields
    switch (eh) {

    case EH_SUBMIT_SM:
        
        // hs["schedule_delivery_time"]        = smsc::sms::Tag::SMPP_SCHEDULE_DELIVERY_TIME;
        hs["validity_period"]               = SMS_VALIDITY_PERIOD;
        hs["replace_if_present_flag"]       = smsc::sms::Tag::SMPP_REPLACE_IF_PRESENT_FLAG;
        // hs["sm_default_msg_id"]

        // --- optional

        // hs["source_addr_subunit"]
        hs["dest_addr_subunit"]             = smsc::sms::Tag::SMPP_DEST_ADDR_SUBUNIT;
        hs["more_messages_to_send"]         = smsc::sms::Tag::SMPP_MORE_MESSAGES_TO_SEND;
        hs["ms_validity"]                   = smsc::sms::Tag::SMPP_MS_VALIDITY;         

        // hs["callback_num_pres_ind"]
        // hs["callback_num_atag"]
        // hs["display_time"]
        // hs["sms_signal"]
        // hs["ms_msg_wait_facilities"]

        hs["number_of_messages"]            = smsc::sms::Tag::SMPP_NUMBER_OF_MESSAGES;

        // hs["alert_on_msg_delivery"]
        // hs["its_reply_type"]
    
        hs["whoisd_charging"]                          = OPTIONAL_CHARGING;
        hs["whoisd_message_transport_type"]            = OPTIONAL_MESSAGE_TRANSPORT_TYPE;
        hs["whoisd_expected_message_transport_type"]   = OPTIONAL_EXPECTED_MESSAGE_TRANSPORT_TYPE;
        hs["whoisd_message_content_type"]              = OPTIONAL_MESSAGE_CONTENT_TYPE;
        hs["whoisd_expected_message_content_type"]     = OPTIONAL_EXPECTED_MESSAGE_CONTENT_TYPE;
        break;

    case EH_DELIVER_SM:

        hs["esm_mt_receipt"]                = ESM_MT_RECEIPT;

        // --- optional

        hs["network_error_code"]            = smsc::sms::Tag::SMPP_NETWORK_ERROR_CODE;

        hs["st_enroute"]                    = ST_ENROUTE;
        hs["st_delivered"]                  = ST_DELIVERED;
        hs["st_expired"]                    = ST_EXPIRED;
        hs["st_deleted"]                    = ST_DELETED;
        hs["st_undeliverable"]              = ST_UNDELIVERABLE;
        hs["st_accepted"]                   = ST_ACCEPTED;
        hs["st_unknown"]                    = ST_UNKNOWN;
        hs["st_rejected"]                   = ST_REJECTED;    

        hs["receipted_message_id"]          = smsc::sms::Tag::SMPP_RECEIPTED_MESSAGE_ID;
        break;

    case EH_DATA_SM_RESP:
        hs["delivery_failure_reason"]      = SMPP_DELIVERY_FAILURE_REASON;
        hs["network_error_code"]           = SMPP_NETWORK_ERROR_CODE;
        hs["additional_status_info_text"]  = SMPP_ADDITIONAL_STATUS_INFO_TEXT;
        hs["dpf_result"]                   = SMPP_DPF_RESULT;
        break;
    default:
        break;
    }

    return hs;
}


IntHash<int> SmppCommandAdapter::initTagToMaskAndValue()
{
    IntHash<int> hs;
    //  mask*0x100 + value
    hs.Insert(ESM_MM_SMSC_DEFAULT,0x0300);
    hs.Insert(ESM_MM_DATAGRAM    ,0x0301);
    hs.Insert(ESM_MM_FORWARD     ,0x0302);
    hs.Insert(ESM_MM_S_AND_F     ,0x0303);
    hs.Insert(ESM_MT_DEFAULT     ,0x3c00);
    hs.Insert(ESM_MT_RECEIPT     ,0x3c04); // added on 2010-03-09
    hs.Insert(ESM_MT_DELIVERY_ACK,0x3c08);
    hs.Insert(ESM_MT_MANUAL_ACK  ,0x3c10);
    hs.Insert(ESM_NSF_NONE       ,0xc000);
    hs.Insert(ESM_NSF_UDHI       ,0xc040);
    hs.Insert(ESM_NSF_SRP        ,0xc080);
    hs.Insert(ESM_NSF_BOTH       ,0xc0c0);

    hs.Insert(RD_RECEIPT_OFF     ,0x0300);
    hs.Insert(RD_RECEIPT_ON      ,0x0301);
    hs.Insert(RD_RECEIPT_FAILURE ,0x0302);
    hs.Insert(RD_ACK_OFF         ,0x0c00);
    hs.Insert(RD_ACK_ON          ,0x0c04);
    hs.Insert(RD_ACK_MAN_ON      ,0x0c08);
    hs.Insert(RD_RD_ACK_BOTH_ON  ,0x0c0c);
    hs.Insert(RD_I_NOTIFICATION  ,0x1010 + 0x10000); // value may be written

    // hs.Insert(DC_BINARY          ,0x0c0c);   // special
    // hs.Insert(DC_TEXT            ,0x0c0c);   // special
    hs.Insert(DC_SMSC_DEFAULT    ,0xff00);
    hs.Insert(DC_ASCII_X34       ,0xff01);
    hs.Insert(DC_LATIN1          ,0xff03);
    hs.Insert(DC_JIS             ,0xff05);
    hs.Insert(DC_CYRILIC         ,0xff06);
    hs.Insert(DC_LATIN_HEBREW    ,0xff07);
    hs.Insert(DC_UCS2            ,0xff08);
    hs.Insert(DC_PICTOGRAM_ENC   ,0xff09);
    hs.Insert(DC_ISO_MUSIC_CODES ,0xff0a);
    hs.Insert(DC_E_KANJI_JIS     ,0xff0d);
    hs.Insert(DC_KS_C_5601       ,0xff0e);
    hs.Insert(DC_GSM_MWI         ,0xe0c0);
    hs.Insert(DC_GSM_MSG_CC      ,0xf0f0);
    return hs;
}


int SmppCommandAdapter::bitFieldType( int fieldId )
{
    if ( (fieldId >= ESM_MM_SMSC_DEFAULT) && (fieldId <= ESM_NSF_BOTH) ) {
        return 1;
    } else if ( (fieldId >= RD_RECEIPT_OFF) && (fieldId <= RD_I_NOTIFICATION) ) {
        return 2;
    } else if ( (fieldId >= DC_BINARY) && (fieldId <= DC_GSM_MSG_CC) ) {
        return 3;
    }
    return 0;
}


std::multimap<int,int> SmppCommandAdapter::initExclusiveBitTags()
{
    std::multimap<int,int> res;
    IntHash<int> taghash = initTagToMaskAndValue();
    int tag, tag2;
    int maskValue, maskVal2;
    for ( IntHash<int>::Iterator iter(taghash); iter.Next(tag,maskValue); ) {
        const int mask = (maskValue & 0xff00);
        const int tagType = bitFieldType(tag);
        for ( IntHash<int>::Iterator jter(iter); jter.Next(tag2,maskVal2); ) {
            if (tag == tag2) continue;
            if (tagType != bitFieldType(tag2)) continue;
            if ((maskVal2 & 0xff00) == mask) {
                res.insert(std::make_pair(tag,tag2));
                res.insert(std::make_pair(tag2,tag));
            }
        }
    }
    return res;
}

}}}
