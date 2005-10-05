#include "SmppAdapter.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace smpp 
{

using namespace scag::util::properties;


Hash<int> SmppCommandAdapter::SubmitFieldNames = SmppCommandAdapter::InitSubmitFieldNames();
Hash<int> SmppCommandAdapter::SubmitRespFieldNames = SmppCommandAdapter::InitSubmitRespFieldNames();
Hash<int> SmppCommandAdapter::DeliverFieldNames = SmppCommandAdapter::InitDeliverFieldNames();

IntHash<AccessType> SmppCommandAdapter::SubmitFieldsAccess = SmppCommandAdapter::InitSubmitAccess();
IntHash<AccessType> SmppCommandAdapter::DeliverFieldsAccess = SmppCommandAdapter::InitDeliverAccess();


AccessType SmppCommandAdapter::CheckAccess(int handlerType,const std::string& name)
{
    int FieldId;

    switch (handlerType) 
    {
    case DELIVERY:
        if (!DeliverFieldNames.Exists(name.c_str())) return atNoAccess;
        FieldId = DeliverFieldNames[name.c_str()];
        if (DeliverFieldsAccess.Exist(FieldId)) return DeliverFieldsAccess.Get(FieldId);
        return atRead;
        break;
    case SUBMIT:
        if (!SubmitFieldNames.Exists(name.c_str())) return atNoAccess;
        FieldId = SubmitFieldNames[name.c_str()];
        if (SubmitFieldsAccess.Exist(FieldId)) return SubmitFieldsAccess.Get(FieldId);
        return atRead;
    case DELIVERY_RESP:
        if (name!="status") return atNoAccess;
        else return atReadWrite;
    case SUBMIT_RESP:
        if (name == "status") return atReadWrite;
        if (name == "message_id") return atRead;
        return atNoAccess;
    }



    return atNoAccess;
}

IntHash<AccessType> SmppCommandAdapter::InitSubmitAccess()
{
    IntHash<AccessType> hs;

    hs.Insert(DC_BINARY,atReadWrite);
    hs.Insert(DC_TEXT, atReadWrite);
    hs.Insert(DC_SMSC_DEFAULT, atReadWrite);
    hs.Insert(DC_ASCII_X34, atReadWrite);
    hs.Insert(DC_LATIN1, atReadWrite);
    hs.Insert(DC_JIS, atReadWrite);
    hs.Insert(DC_CYRILIC, atReadWrite);
    hs.Insert(DC_LATIN_HEBREW, atReadWrite);
    hs.Insert(DC_UCS2, atReadWrite);
    hs.Insert(DC_PICTOGRAM_ENC, atReadWrite);
    hs.Insert(DC_ISO_MUSIC_CODES, atReadWrite);
    hs.Insert(DC_E_KANJI_JIS, atReadWrite);
    hs.Insert(DC_KS_C_5601, atReadWrite);
    hs.Insert(DC_GSM_MWI, atReadWrite);
    hs.Insert(DC_GSM_MSG_CC, atReadWrite);

    hs.Insert(Tag::SMPP_SM_LENGTH, atReadWrite);
    hs.Insert(Tag::SMPP_SHORT_MESSAGE, atReadWrite);
    hs.Insert(Tag::SMPP_MESSAGE_PAYLOAD, atReadWrite);
    hs.Insert(Tag::SMPP_USER_RESPONSE_CODE, atReadWrite);
    hs.Insert(Tag::SMPP_LANGUAGE_INDICATOR, atReadWrite);

    return hs;
}

IntHash<AccessType> SmppCommandAdapter::InitDeliverAccess()
{
    IntHash<AccessType> hs;

    hs.Insert(DC_BINARY, atReadWrite);
    hs.Insert(DC_TEXT, atReadWrite);
    hs.Insert(DC_SMSC_DEFAULT, atReadWrite);
    hs.Insert(DC_ASCII_X34, atReadWrite);
    hs.Insert(DC_LATIN1, atReadWrite);
    hs.Insert(DC_JIS, atReadWrite);
    hs.Insert(DC_CYRILIC, atReadWrite);
    hs.Insert(DC_LATIN_HEBREW, atReadWrite);
    hs.Insert(DC_UCS2, atReadWrite);
    hs.Insert(DC_PICTOGRAM_ENC, atReadWrite);
    hs.Insert(DC_ISO_MUSIC_CODES, atReadWrite);
    hs.Insert(DC_E_KANJI_JIS, atReadWrite);
    hs.Insert(DC_KS_C_5601, atReadWrite);
    hs.Insert(DC_GSM_MWI, atReadWrite);
    hs.Insert(DC_GSM_MSG_CC, atReadWrite);

    hs.Insert(Tag::SMPP_SM_LENGTH, atReadWrite);
    hs.Insert(Tag::SMPP_SHORT_MESSAGE, atReadWrite);
    hs.Insert(Tag::SMPP_MESSAGE_PAYLOAD, atReadWrite);
    hs.Insert(Tag::SMPP_USER_RESPONSE_CODE, atReadWrite);
    hs.Insert(Tag::SMPP_LANGUAGE_INDICATOR, atReadWrite);


    return hs;
}



Hash<int> SmppCommandAdapter::InitSubmitFieldNames()
{
    Hash<int> hs;

    hs["OA"]                            = OA;
    hs["DA"]                            = DA;


    //hs[""] = Tag::SMPP_ESM_CLASS //mask 

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


    hs["protocol_id"]                   = Tag::SMPP_PROTOCOL_ID; 
    hs["priority"]                      = Tag::SMPP_PRIORITY; 
    hs["schedule_delivery_time"]        = Tag::SMPP_SCHEDULE_DELIVERY_TIME;


    //hs["registred_delivery"]            = Tag::SMPP_REGISTRED_DELIVERY; //mask +
    hs["rd_receipt_off"]                = RD_RECEIPT_OFF;
    hs["rd_receipt_on"]                 = RD_RECEIPT_ON;
    hs["rd_receipt_failure"]            = RD_RECEIPT_FAILURE;
    hs["rd_ack_off"]                    = RD_ACK_OFF;
    hs["rd_ack_on"]                     = RD_ACK_ON;
    hs["rd_ack_man_on"]                 = RD_ACK_MAN_ON;
    hs["rd_ack_both_on"]                = RD_RD_ACK_BOTH_ON;
    hs["rd_i_notification"]             = RD_I_NOTIFICATION;


    hs["replace_if_present_flag"]       = Tag::SMPP_REPLACE_IF_PRESENT_FLAG;


    //hs[""] = Tag::SMPP_DATA_CODING //mask *
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

    /*
    dc_binary !
    dc_text   !
    */
    

    hs["sm_length"]                     = Tag::SMPP_SM_LENGTH; //*
    hs["short_message"]                 = Tag::SMPP_SHORT_MESSAGE; //*
    hs["source_port"]                   = Tag::SMPP_SOURCE_PORT; 
    hs["destination_port"]              = Tag::SMPP_DESTINATION_PORT; 
    hs["dest_addr_subunit"]             = Tag::SMPP_DEST_ADDR_SUBUNIT; 
    hs["sar_msg_ref_num"]               = Tag::SMPP_SAR_MSG_REF_NUM; 
    hs["sar_total_segments"]            = Tag::SMPP_SAR_TOTAL_SEGMENTS; 
    hs["sar_segment_seqnum"]            = Tag::SMPP_SAR_SEGMENT_SEQNUM; 
    hs["more_messages_to_send"]         = Tag::SMPP_MORE_MESSAGES_TO_SEND; 
    hs["payload_type"]                  = Tag::SMPP_PAYLOAD_TYPE; 
    hs["message_payload"]               = Tag::SMPP_MESSAGE_PAYLOAD; //*
    hs["user_response_code"]            = Tag::SMPP_USER_RESPONSE_CODE; //*
    hs["ms_validity"]                   = Tag::SMPP_MS_VALIDITY; 
    hs["number_of_messages"]            = Tag::SMPP_NUMBER_OF_MESSAGES; 
    hs["language_indicator"]            = Tag::SMPP_LANGUAGE_INDICATOR; //*


    //hs[""] = Tag::SMPP_USSD_SERVICE_OP //mask +

    hs["ussd_pssd_ind"]                 = USSD_PSSD_IND;
    hs["ussd_pssr_ind"]                 = USSD_PSSR_IND;
    hs["ussd_pssr_req"]                 = USSD_PSSR_REQ;
    hs["ussd_ussn_req"]                 = USSD_USSN_REQ;
    hs["ussd_pssd_resp"]                = USSD_PSSD_RESP;
    hs["ussd_pssr_resp"]                = USSD_PSSR_RESP;
    hs["ussd_ussr_conf"]                = USSD_USSR_CONF;
    hs["ussd_ussn_conf"]                = USSD_USSN_CONF;




/*
Tag::SMPP_ESM_CLASS //mask +
Tag::SMPP_PROTOCOL_ID //+
Tag::SMPP_PRIORITY // +
Tag::SMPP_SCHEDULE_DELIVERY_TIME //+
Tag::SMPP_REGISTRED_DELIVERY//+
Tag::SMPP_REPLACE_IF_PRESENT_FLAG//+

Tag::SMPP_DATA_CODING //mask *
Tag::SMPP_SM_LENGTH //*
Tag::SMPP_SHORT_MESSAGE //*
Tag::SMPP_SOURCE_PORT //+
Tag::SMPP_DESTINATION_PORT //+
Tag::SMPP_DEST_ADDR_SUBUNIT //+
Tag::SMPP_SAR_MSG_REF_NUM //+
Tag::SMPP_SAR_TOTAL_SEGMENTS //+
Tag::SMPP_SAR_SEGMENT_SEQNUM //+
Tag::SMPP_MORE_MESSAGES_TO_SEND //+
Tag::SMPP_PAYLOAD_TYPE //+
Tag::SMPP_MESSAGE_PAYLOAD //*
Tag::SMPP_USER_RESPONSE_CODE //*
Tag::SMPP_MS_VALIDITY //+
Tag::SMPP_NUMBER_OF_MESSAGES // +
Tag::SMPP_LANGUAGE_INDICATOR //*
Tag::SMPP_USSD_SERVICE_OP //mask +




  SMPP_USER_MESSAGE_REFERENCE
  SMPP_USSD_SERVICE_OP
  SMPP_RECEIPTED_MESSAGE_ID
  SMPP_NUMBER_OF_MESSAGES
  SMPP_DATA_SM
  SMPP_MSG_STATE
  
  SMPP_MORE_MESSAGES_TO_SEND
  SMPP_DEST_NETWORK_TYPE
  SMPP_DEST_BEARER_TYPE
  SMPP_QOS_TIME_TO_LIVE
  SMPP_SET_DPF
  SMPP_SOURCE_NETWORK_TYPE
  SMPP_SOURCE_BEARER_TYPE
  
 

*/

    //Setting submit command fields access

    return hs;
}


Hash<int> SmppCommandAdapter::InitSubmitRespFieldNames()
{
    Hash<int> hs;

    hs["status"] = 0;
    hs["message_id"] = 1;


    return hs;
}


Hash<int> SmppCommandAdapter::InitDeliverFieldNames()
{
    Hash<int> hs;

    hs["OA"]                            = OA;
    hs["DA"]                            = DA;


    //hs[""] = Tag::SMPP_ESM_CLASS //mask +

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


    hs["protocol_id"]                   = Tag::SMPP_PROTOCOL_ID; 
    hs["priority"]                      = Tag::SMPP_PRIORITY; 
    hs["schedule_delivery_time"]        = Tag::SMPP_SCHEDULE_DELIVERY_TIME;


    //hs["registred_delivery"]            = Tag::SMPP_REGISTRED_DELIVERY; //mask +
    hs["rd_receipt_off"]                = RD_RECEIPT_OFF;
    hs["rd_receipt_on"]                 = RD_RECEIPT_ON;
    hs["rd_receipt_failure"]            = RD_RECEIPT_FAILURE;
    hs["rd_ack_off"]                    = RD_ACK_OFF;
    hs["rd_ack_on"]                     = RD_ACK_ON;
    hs["rd_ack_man_on"]                 = RD_ACK_MAN_ON;
    hs["rd_ack_both_on"]                = RD_RD_ACK_BOTH_ON;
    hs["rd_i_notification"]             = RD_I_NOTIFICATION;


    //hs[""] = Tag::SMPP_DATA_CODING //mask *
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

    /*
    dc_binary !
    dc_text   !
    */


    hs["sm_length"]                     = Tag::SMPP_SM_LENGTH; //*
    hs["short_message"]                 = Tag::SMPP_SHORT_MESSAGE; //*
    hs["source_port"]                   = Tag::SMPP_SOURCE_PORT; 
    hs["destination_port"]              = Tag::SMPP_DESTINATION_PORT; 
    hs["sar_msg_ref_num"]               = Tag::SMPP_SAR_MSG_REF_NUM; 
    hs["sar_total_segments"]            = Tag::SMPP_SAR_TOTAL_SEGMENTS; 
    hs["sar_segment_seqnum"]            = Tag::SMPP_SAR_SEGMENT_SEQNUM; 
    hs["user_response_code"]            = Tag::SMPP_USER_RESPONSE_CODE; //*
    hs["payload_type"]                  = Tag::SMPP_PAYLOAD_TYPE; 
    hs["message_payload"]               = Tag::SMPP_MESSAGE_PAYLOAD; //*
    hs["language_indicator"]            = Tag::SMPP_LANGUAGE_INDICATOR; //*

    //mask
    hs["st_enroute"]                    = ST_ENROUTE;
    hs["st_delivered"]                  = ST_DELIVERED;
    hs["st_expired"]                    = ST_EXPIRED;
    hs["st_deleted"]                    = ST_DELETED;
    hs["st_undeliverable"]              = ST_UNDELIVERABLE;
    hs["st_accepted"]                   = ST_ACCEPTED;
    hs["st_unknown"]                    = ST_UNKNOWN;
    hs["st_rejected"]                   = ST_REJECTED;


    return hs;
}


void SmppCommandAdapter::Set_DC_BIT_Property(SMS& data,int FieldId,bool value)
{
    if (FieldId == DC_BINARY) 
    {
        //00000010
        if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,2);
        else data.setIntProperty(Tag::SMPP_DATA_CODING,4);
        return;
    } else if (FieldId == DC_TEXT) {
        //00000100
        if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,4);
        else data.setIntProperty(Tag::SMPP_DATA_CODING,2);
        return;
    } else if ((FieldId >= DC_SMSC_DEFAULT)&&(FieldId <=DC_GSM_MSG_CC)) 
    {
        //TODO: check DC_TEXT field first!
        int num = data.getIntProperty(Tag::SMPP_DATA_CODING);
        if (!((num&4)==4)) return;

        switch (FieldId) 
        {

        case DC_SMSC_DEFAULT:
            //00000000
            data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_ASCII_X34:
            //00000001
            if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,1);
            else data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_LATIN1:
            //00000011
            if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,3);
            else data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_JIS:
            //00000101
            if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,5);
            else data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_CYRILIC:
            //00000110
            if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,6);
            else data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_LATIN_HEBREW:
            //00000111
            if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,7);
            else data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_UCS2:
            //00001000
            if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,8);
            else data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_PICTOGRAM_ENC:
            //00001001
            if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,9);
            else data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_ISO_MUSIC_CODES:
            //00001010
            if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,10);
            else data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_E_KANJI_JIS:
            //00001101
            if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,11);
            else data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_KS_C_5601:
            //00001110
            if (value) data.setIntProperty(Tag::SMPP_DATA_CODING,12);
            else data.setIntProperty(Tag::SMPP_DATA_CODING,0);
            break;
        case DC_GSM_MWI:
            //1100xxxx
            //1101xxxx


            //11000000 = 192
            //00001111 = 15
            //11110000 = 240
            if (value)
                data.setIntProperty(Tag::SMPP_DATA_CODING,
                                    ((192|(data.getIntProperty(Tag::SMPP_DATA_CODING)&15))));
            else data.setIntProperty(Tag::SMPP_DATA_CODING,
                                    ((240|(data.getIntProperty(Tag::SMPP_DATA_CODING)&15))));
            break;
        case DC_GSM_MSG_CC:
            //1111xxxx
            if (value)
                data.setIntProperty(Tag::SMPP_DATA_CODING,
                                    ((240|(data.getIntProperty(Tag::SMPP_DATA_CODING)&15))));
            else data.setIntProperty(Tag::SMPP_DATA_CODING,
                                    ((192|(data.getIntProperty(Tag::SMPP_DATA_CODING)&15))));
            break;
        }
    }
}


void SmppCommandAdapter::WriteSubmitField(SMS& data,int FieldId,AdapterProperty& property)
{
    WriteDeliveryField(data,FieldId,property);
}



void SmppCommandAdapter::WriteDeliveryField(SMS& data,int FieldId,AdapterProperty& property)
{

    if ((FieldId >= DC_BINARY)&&(FieldId <= DC_GSM_MSG_CC)) 
    {
        Set_DC_BIT_Property(data,FieldId,property.getBool());
    } 
    else 
        switch (FieldId)
        {
        case Tag::SMPP_SM_LENGTH:
        case Tag::SMPP_USER_RESPONSE_CODE:
        case Tag::SMPP_LANGUAGE_INDICATOR:
            data.setIntProperty(FieldId,property.getInt());
            break;
        case Tag::SMPP_SHORT_MESSAGE:
        case Tag::SMPP_MESSAGE_PAYLOAD:
            data.setStrProperty(FieldId,property.getStr().c_str());
            break;
        }



 /*       switch (FieldId) 
        {

        case Tag::SMPP_SM_LENGTH:
            data.setIntProperty(Tag::SMPP_SM_LENGTH,property.getInt());
            break;
        case Tag::SMPP_SHORT_MESSAGE:
    
            break;
        case Tag::SMPP_USER_RESPONSE_CODE:
    
            break;
        case Tag::SMPP_MESSAGE_PAYLOAD:
    
            break;
        case Tag::SMPP_LANGUAGE_INDICATOR:
    
            break;
        }*/
}



Property * SmppCommandAdapter::getSubmitRespProperty(const std::string& name,int FieldId)
{
    if (PropertyPul.Exist(FieldId)) return PropertyPul.Get(FieldId);

    AdapterProperty * property = 0;

    switch (FieldId) 
    {
    case 0:
        property = new AdapterProperty(name,this,"");
        property->setPureInt(command->get_status());
    }

    if (property) 
    {
        PropertyPul.Insert(FieldId, property);
        return property;
    }


    return 0;
}



AdapterProperty * SmppCommandAdapter::GetStrBitFromMask(SMS& data,const std::string& name,int tag,int mask)
{
    char buff[100];
    int num = data.getIntProperty(tag);
    sprintf(buff,"%d",((num&mask)==mask));
    AdapterProperty * property = new AdapterProperty(name,this,buff);
    return property;
}

AdapterProperty * SmppCommandAdapter::Get_ESM_BIT_Property(SMS& data, const std::string& name,int FieldId)
{
    AdapterProperty * property = 0;

    switch (FieldId) 
    {
    case ESM_MM_SMSC_DEFAULT:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,0);
        break;
    case ESM_MM_DATAGRAM:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,0x1);
        break;
    case ESM_MM_FORWARD:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,0x2);
        break;
    case ESM_MM_S_AND_F:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,0x3);
        break;
    case ESM_MT_DEFAULT:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,0x0);
        break;
    case ESM_MT_DELIVERY_ACK:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,0x8);
        break;
    case ESM_MT_MANUAL_ACK:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,0x16);
        break;
    case ESM_NSF_NONE:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,0x0);
        break;
    case ESM_NSF_UDHI:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,64);
        break;
    case ESM_NSF_SRP:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,128);
        break;
    case ESM_NSF_BOTH:
        property = GetStrBitFromMask(data,name,Tag::SMPP_ESM_CLASS,192);
        break;
    }

    return property;
}

AdapterProperty * SmppCommandAdapter::Get_RD_BIT_Property(SMS& data, const std::string& name,int FieldId)
{
    AdapterProperty * property = 0;

    switch (FieldId) 
    {
    case RD_RECEIPT_OFF:
        property = GetStrBitFromMask(data,name,Tag::SMPP_REGISTRED_DELIVERY,0);
        break;
    case RD_RECEIPT_ON:
        property = GetStrBitFromMask(data,name,Tag::SMPP_REGISTRED_DELIVERY,1);
        break;
    case RD_RECEIPT_FAILURE:
        property = GetStrBitFromMask(data,name,Tag::SMPP_REGISTRED_DELIVERY,3);
        break;
    case RD_ACK_OFF:
        property = GetStrBitFromMask(data,name,Tag::SMPP_REGISTRED_DELIVERY,0);
        break;
    case RD_ACK_ON:
        property = GetStrBitFromMask(data,name,Tag::SMPP_REGISTRED_DELIVERY,4);
        break;
    case RD_ACK_MAN_ON:
        property = GetStrBitFromMask(data,name,Tag::SMPP_REGISTRED_DELIVERY,8);
        break;
    case RD_RD_ACK_BOTH_ON:
        property = GetStrBitFromMask(data,name,Tag::SMPP_REGISTRED_DELIVERY,12);
        break;
    case RD_I_NOTIFICATION:
        property = GetStrBitFromMask(data,name,Tag::SMPP_REGISTRED_DELIVERY,16);
        break;
    }

    return property;
}


AdapterProperty * SmppCommandAdapter::Get_DC_BIT_Property(SMS& data, const std::string& name,int FieldId)
{
    AdapterProperty * property = 0;
    char buff[100];
    int num = 0;

    switch (FieldId) 
    {
    case DC_BINARY:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,2);
        break;
    case DC_TEXT:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,4);
        break;
    
    case DC_SMSC_DEFAULT:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,0);
        break;
    case DC_ASCII_X34:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,1);
        break;
    case DC_LATIN1:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,3);
        break;
    case DC_JIS:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,5);
        break;
    case DC_CYRILIC:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,6);
        break;
    case DC_LATIN_HEBREW:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,7);
        break;
    case DC_UCS2:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,8);
        break;
    case DC_PICTOGRAM_ENC:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,9);
        break;
    case DC_ISO_MUSIC_CODES:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,10);
        break;
    case DC_E_KANJI_JIS:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,13);
        break;
    case DC_KS_C_5601:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,14);
        break;
    case DC_GSM_MWI:
        num = data.getIntProperty(Tag::SMPP_DATA_CODING);
        sprintf(buff,"%d",(((num&192)==192)||(num&208)==208));
        property = new AdapterProperty(name,this,buff);
        break;
    case DC_GSM_MSG_CC:
        property = GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING,240);
        break;
    }

    return property;
}

AdapterProperty * SmppCommandAdapter::Get_USSD_BIT_Property(SMS& data, const std::string& name,int FieldId)
{
    AdapterProperty * property = 0;

    switch (FieldId) 
    {
    case USSD_PSSD_IND:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,0);
        break;
    case USSD_PSSR_IND:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,1);
        break;
    case USSD_PSSR_REQ:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,2);
        break;
    case USSD_USSN_REQ:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,3);
        break;
    case USSD_PSSD_RESP:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,16);
        break;
    case USSD_PSSR_RESP:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,17);
        break;
    case USSD_USSR_CONF:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,18);
        break;
    case USSD_USSN_CONF:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,19);
        break;
    }

    return property;
}


Property * SmppCommandAdapter::getSubmitProperty(SMS& data,const std::string& name,int FieldId)
{
    if (PropertyPul.Exist(FieldId)) return PropertyPul.Get(FieldId);

    AdapterProperty * property = 0;
    char buff[100];
    int num = 0;


    if ((FieldId >= ESM_MM_SMSC_DEFAULT)&&(FieldId <= ESM_NSF_BOTH)) 
    {
        property = Get_ESM_BIT_Property(data,name,FieldId);
    } else if ((FieldId >= RD_RECEIPT_OFF)&&(FieldId <= RD_I_NOTIFICATION)) 
    {
        property = Get_RD_BIT_Property(data,name,FieldId);
    } else if ((FieldId >= DC_BINARY)&&(FieldId <= DC_GSM_MSG_CC)) 
    {
        property = Get_DC_BIT_Property(data,name,FieldId);
    } else if ((FieldId >= USSD_PSSD_IND)&&(FieldId <= USSD_USSN_CONF)) 
    {
        property = Get_USSD_BIT_Property(data,name,FieldId);
    } else

    switch (FieldId) 
    {
    case OA:
        property = new AdapterProperty(name,this,data.getOriginatingAddress().toString());
        break;
    case DA:
        property = new AdapterProperty(name,this,data.getDestinationAddress().toString());
        break;
    }

   /* if (data.hasBinProperty(FieldId))
    {
        unsigned int len = 0;
        std::string str = data.getBinProperty(FieldId,&len);
        return new AdapterProperty(name,this,str);
    } else if (data.hasIntProperty(FieldId)) 
    {

    } else*/ 

    int tagType = (FieldId >> 8);

    if (property) 
    {
        PropertyPul.Insert(FieldId, property);
        return property;
    } else if (FieldId == SMS_STR_TAG) 
    {
        std::string str = data.getStrProperty(FieldId);
        property = new AdapterProperty(name,this,str);
        PropertyPul.Insert(FieldId, property);
    } else if (FieldId == SMS_INT_TAG) 
    {
        num = data.getIntProperty(FieldId);
        sprintf(buff,"%d",num);
        property = new AdapterProperty(name,this,buff);
        PropertyPul.Insert(FieldId, property);
    }
          
    return property;
}


Property * SmppCommandAdapter::getDeliverProperty(SMS& data,const std::string& name,int FieldId)
{
    if (PropertyPul.Exist(FieldId)) return PropertyPul.Get(FieldId);

    AdapterProperty * property = 0;
    char buff[100];
    int num = 0;

    if ((FieldId >= ESM_MM_SMSC_DEFAULT)&&(FieldId <= ESM_NSF_BOTH)) 
    {
        property = Get_ESM_BIT_Property(data,name,FieldId);
    } else if ((FieldId >= RD_RECEIPT_OFF)&&(FieldId <= RD_I_NOTIFICATION)) 
    {
        property = Get_RD_BIT_Property(data,name,FieldId);
    } else if ((FieldId >= DC_BINARY)&&(FieldId <= DC_GSM_MSG_CC)) 
    {
        property = Get_DC_BIT_Property(data,name,FieldId);
    } else if ((FieldId >= ST_ENROUTE)&&(FieldId <= ST_REJECTED)) 
    {
        property = new AdapterProperty(name,this,"");
        property->setPureInt(data.getState());
    } else

    switch (FieldId) 
    {
    case OA:
        property = new AdapterProperty(name,this,data.getOriginatingAddress().toString());
        break;
    case DA:
        property = new AdapterProperty(name,this,data.getDestinationAddress().toString());
        break;
    }

    /*
    #define SMS_INT_TAG 0
    #define SMS_STR_TAG 1
    #define SMS_BIN_TAG 2
    */
    int tagType = (FieldId >> 8);

    if (property) 
    {
        PropertyPul.Insert(FieldId, property);
        return property;
    } else if (tagType == SMS_STR_TAG) 
    {
        std::string str = data.getStrProperty(FieldId);
        property = new AdapterProperty(name,this,str);
        PropertyPul.Insert(FieldId, property);
    } else if (tagType == SMS_INT_TAG) 
    {
        num = data.getIntProperty(FieldId);
        sprintf(buff,"%d",num);
        property = new AdapterProperty(name,this,buff);
        PropertyPul.Insert(FieldId, property);
    }
          
    return property;
}


SmppCommandAdapter::~SmppCommandAdapter()
{
    int key = 0;
    AdapterProperty * value = 0;
    
    for (IntHash <AdapterProperty *>::Iterator it = PropertyPul.First(); it.Next(key, value);)
    {
        delete value;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////



Property* SmppCommandAdapter::getProperty(const std::string& name)
{
    _SmppCommand * cmd = command.operator ->();
    if (!cmd) return 0;

    CommandId cmdid = cmd->get_commandId();
    void * dta = cmd->dta;

    if (!dta) return 0;

    int FieldId = -1;
    AdapterProperty * property = 0;

    switch (cmdid) 
    {
    case DELIVERY:
        if (!DeliverFieldNames.Exists(name.c_str())) return 0;
        FieldId = DeliverFieldNames[name.c_str()];
        return getDeliverProperty(*(SMS*)dta,name,FieldId);
    case SUBMIT:
        if (!SubmitFieldNames.Exists(name.c_str())) return 0;
        FieldId = SubmitFieldNames[name.c_str()];
        return getSubmitProperty(*(SMS*)dta,name,FieldId);
    case DELIVERY_RESP:
        if (PropertyPul.Count()) 
        {
            return PropertyPul.Get(0);
        }
        property = new AdapterProperty(name,this,"");
        property->setPureInt(command->status);
        PropertyPul.Insert(0,property);
        return property;

    case SUBMIT_RESP:
        if (!SubmitRespFieldNames.Exists(name.c_str())) return 0;
        FieldId = SubmitRespFieldNames[name.c_str()];
        return getSubmitRespProperty(name,FieldId);
    }
    return 0;
}


void SmppCommandAdapter::changed(AdapterProperty& property)
{

    _SmppCommand * cmd = command.operator ->();
    if (!cmd) return;
    
    CommandId cmdid = cmd->get_commandId();
    int FieldId;

    SMS * data = (SMS *) cmd->dta;

    const std::string name = property.GetName();  

    switch (cmdid) 
    {
    case DELIVERY:
        if (!DeliverFieldNames.Exists(name.c_str())) return;
        FieldId = DeliverFieldNames[name.c_str()];
        WriteDeliveryField(*data,FieldId,property);
        break;
    case SUBMIT:
        if (!SubmitFieldNames.Exists(name.c_str())) return;
        FieldId = SubmitFieldNames[name.c_str()];
        WriteSubmitField(*data,FieldId,property);
        break;
    case DELIVERY_RESP:
        if (name!="status") return;
        cmd->set_status(property.getInt());
        break;
    case SUBMIT_RESP:
        if (name!="status") return;
        cmd->set_status(property.getInt());
        break;
    }

    if (PropertyPul.Exist(FieldId)) 
    {
        (PropertyPul.Get(FieldId))->setPureStr(property.getStr());
    }

}


}}}
