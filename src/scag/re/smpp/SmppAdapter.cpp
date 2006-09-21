#include "SmppAdapter.h"
#include "scag/re/CommandAdapter.h"
#include "scag/re/CommandBrige.h"

#include "scag/util/encodings/Encodings.h"
#include "logger/Logger.h"


namespace scag { namespace re { namespace smpp 
{

using namespace scag::util::properties;
using namespace scag::util::encodings;


Hash<int> SmppCommandAdapter::SubmitFieldNames = SmppCommandAdapter::InitSubmitFieldNames();
Hash<int> SmppCommandAdapter::SubmitRespFieldNames = SmppCommandAdapter::InitSubmitRespFieldNames();
Hash<int> SmppCommandAdapter::DeliverRespFieldNames = SmppCommandAdapter::InitDeliverRespFieldNames();
Hash<int> SmppCommandAdapter::DeliverFieldNames = SmppCommandAdapter::InitDeliverFieldNames();

Hash<int> SmppCommandAdapter::DataSmFieldNames = SmppCommandAdapter::InitDataSmFieldNames();
Hash<int> SmppCommandAdapter::DataSmRespFieldNames = SmppCommandAdapter::InitDataSmRespFieldNames();


IntHash<AccessType> SmppCommandAdapter::SubmitFieldsAccess = SmppCommandAdapter::InitSubmitAccess();
IntHash<AccessType> SmppCommandAdapter::DeliverFieldsAccess = SmppCommandAdapter::InitDeliverAccess();
IntHash<AccessType> SmppCommandAdapter::DataSmFieldsAccess = SmppCommandAdapter::InitDataSmAccess();
IntHash<AccessType> SmppCommandAdapter::SubmitRespFieldsAccess = SmppCommandAdapter::InitSubmitRespAccess();
IntHash<AccessType> SmppCommandAdapter::DeliverRespFieldsAccess = SmppCommandAdapter::InitDeliverRespAccess();
IntHash<AccessType> SmppCommandAdapter::DataSmRespFieldsAccess = SmppCommandAdapter::InitDataSmRespAccess();


AccessType SmppCommandAdapter::CheckAccess(int handlerType, const std::string& name)
{
    int * pFieldId;
    AccessType * actype = 0;

    switch (handlerType) 
    {
    case EH_DELIVER_SM:
        pFieldId = DeliverFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return atNoAccess;

        actype = DeliverFieldsAccess.GetPtr(*pFieldId);

        if (actype) return *actype;

        return atRead;
        break;
    case EH_SUBMIT_SM:
        pFieldId = SubmitFieldNames.GetPtr(name.c_str());

        if (!pFieldId) return atNoAccess;

        actype = SubmitFieldsAccess.GetPtr(*pFieldId);

        if (actype) return *actype;
        return atRead;
    case EH_DELIVER_SM_RESP:
        pFieldId = DeliverRespFieldNames.GetPtr(name.c_str());

        if (!pFieldId) return atNoAccess;

        actype = DeliverRespFieldsAccess.GetPtr(*pFieldId);
        if (actype) return *actype;
        /*
        if (name =="status") return atReadWrite;
        if (name =="ussd_dialog") return atRead;
        return atNoAccess;
        */

    case EH_SUBMIT_SM_RESP:

        pFieldId = SubmitRespFieldNames.GetPtr(name.c_str());

        if (!pFieldId) return atNoAccess;

        actype = SubmitRespFieldsAccess.GetPtr(*pFieldId);
        if (actype) return *actype;

        /*if (name == "status") return atReadWrite;
        if (name == "message_id") return atRead;
        if (name == "ussd_dialog") return atRead;
        
        return atNoAccess;                       */
        break;
    case EH_DATA_SM:

        pFieldId = DataSmFieldNames.GetPtr(name.c_str());

        if (!pFieldId) return atNoAccess;

        actype = DataSmFieldsAccess.GetPtr(*pFieldId);
        if (actype) return *actype;
        return atRead;
        break;
    case EH_DATA_SM_RESP:
        pFieldId = DataSmRespFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return atNoAccess;

        actype = DataSmRespFieldsAccess.GetPtr(*pFieldId);
        if (actype) return *actype;

        break;
    }



    return atNoAccess;
}


IntHash<AccessType> SmppCommandAdapter::InitSubmitRespAccess()
{
    IntHash<AccessType> hs;
    hs.Insert(STATUS,atReadWrite);

    return hs;
}

IntHash<AccessType> SmppCommandAdapter::InitDeliverRespAccess()
{
    IntHash<AccessType> hs;
    hs.Insert(STATUS,atReadWrite);

    return hs;
}

IntHash<AccessType> SmppCommandAdapter::InitDataSmRespAccess()
{
    IntHash<AccessType> hs;
    hs.Insert(STATUS,atReadWrite);

    return hs;
}



IntHash<AccessType> SmppCommandAdapter::InitDataSmAccess()
{
    IntHash<AccessType> hs;
    hs.Insert(OA,atReadWrite);
    hs.Insert(DA,atReadWrite);

    //submit fields
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
    hs.Insert(SMS_MESSAGE_BODY, atReadWrite);

    hs.Insert(Tag::SMPP_USER_RESPONSE_CODE, atReadWrite);
    hs.Insert(Tag::SMPP_LANGUAGE_INDICATOR, atReadWrite);

    hs.Insert(SMS_SVC_TYPE, atReadWrite);

    return hs;
}

IntHash<AccessType> SmppCommandAdapter::InitSubmitAccess()
{
    IntHash<AccessType> hs;

    hs.Insert(OA,atReadWrite);
    hs.Insert(DA,atReadWrite);


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
/*
    hs.Insert(Tag::SMPP_SHORT_MESSAGE, atReadWrite);
    hs.Insert(Tag::SMPP_MESSAGE_PAYLOAD, atReadWrite);*/
    hs.Insert(SMS_MESSAGE_BODY, atReadWrite);

    hs.Insert(Tag::SMPP_USER_RESPONSE_CODE, atReadWrite);
    hs.Insert(Tag::SMPP_LANGUAGE_INDICATOR, atReadWrite);

    hs.Insert(SMS_SVC_TYPE, atReadWrite);

    return hs;
}

IntHash<AccessType> SmppCommandAdapter::InitDeliverAccess()
{
    IntHash<AccessType> hs;

    hs.Insert(OA,atReadWrite);
    hs.Insert(DA,atReadWrite);

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

/*    hs.Insert(Tag::SMPP_SHORT_MESSAGE, atReadWrite);
    hs.Insert(Tag::SMPP_MESSAGE_PAYLOAD, atReadWrite);*/
    hs.Insert(SMS_MESSAGE_BODY, atReadWrite);

    hs.Insert(Tag::SMPP_USER_RESPONSE_CODE, atReadWrite);
    hs.Insert(Tag::SMPP_LANGUAGE_INDICATOR, atReadWrite);

    hs.Insert(SMS_SVC_TYPE, atReadWrite);
    
    return hs;
}


Hash<int> SmppCommandAdapter::InitDataSmFieldNames()
{
    Hash<int> hs;

    hs["OA"]                            = OA;
    hs["DA"]                            = DA;

    hs["packet_direction"]              = PACKET_DIRECTION;

    hs["whoisd_charging"]                          = OPTIONAL_CHARGING;
    hs["whoisd_message_transport_type"]            = OPTIONAL_MESSAGE_TRANSPORT_TYPE;
    hs["whoisd_expected_message_transport_type"]   = OPTIONAL_EXPECTED_MESSAGE_TRANSPORT_TYPE;

    hs["whoisd_message_content_type"]              = OPTIONAL_MESSAGE_CONTENT_TYPE;
    hs["whoisd_expected_message_content_type"]     = OPTIONAL_EXPECTED_MESSAGE_CONTENT_TYPE;

    //Submit fields
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
    hs["priority_flag"]                 = Tag::SMPP_PRIORITY; 
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
    hs["message_text"]                  = SMS_MESSAGE_BODY;

    hs["source_port"]                   = Tag::SMPP_SOURCE_PORT; 
    hs["destination_port"]              = Tag::SMPP_DESTINATION_PORT; 
    hs["dest_addr_subunit"]             = Tag::SMPP_DEST_ADDR_SUBUNIT; 
    hs["sar_msg_ref_num"]               = Tag::SMPP_SAR_MSG_REF_NUM; 
    hs["sar_total_segments"]            = Tag::SMPP_SAR_TOTAL_SEGMENTS; 
    hs["sar_segment_seqnum"]            = Tag::SMPP_SAR_SEGMENT_SEQNUM; 
    hs["more_messages_to_send"]         = Tag::SMPP_MORE_MESSAGES_TO_SEND; 
    hs["payload_type"]                  = Tag::SMPP_PAYLOAD_TYPE; 
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

    hs["validity_period"]               = SMS_VALIDITY_PERIOD;
    hs["svc_type"]                      = SMS_SVC_TYPE;

    hs["ussd_dialog"]                   = USSD_DIALOG;

    return hs;
}

Hash<int> SmppCommandAdapter::InitDataSmRespFieldNames()
{
    Hash<int> hs;

    hs["OA"]                            = OA;
    hs["DA"]                            = DA;

    hs["packet_direction"]              = PACKET_DIRECTION;

    hs["status"] = STATUS;

    hs["message_id"] = MESSAGE_ID;
    hs["ussd_dialog"] = USSD_DIALOG;


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
    hs["priority_flag"]                 = Tag::SMPP_PRIORITY; 
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

/*    hs["short_message"]                 = Tag::SMPP_SHORT_MESSAGE; //*
    hs["message_payload"]               = Tag::SMPP_MESSAGE_PAYLOAD; //**/
    hs["message_text"]                  = SMS_MESSAGE_BODY;

    hs["source_port"]                   = Tag::SMPP_SOURCE_PORT; 
    hs["destination_port"]              = Tag::SMPP_DESTINATION_PORT; 
    hs["dest_addr_subunit"]             = Tag::SMPP_DEST_ADDR_SUBUNIT; 
    hs["sar_msg_ref_num"]               = Tag::SMPP_SAR_MSG_REF_NUM; 
    hs["sar_total_segments"]            = Tag::SMPP_SAR_TOTAL_SEGMENTS; 
    hs["sar_segment_seqnum"]            = Tag::SMPP_SAR_SEGMENT_SEQNUM; 
    hs["more_messages_to_send"]         = Tag::SMPP_MORE_MESSAGES_TO_SEND; 
    hs["payload_type"]                  = Tag::SMPP_PAYLOAD_TYPE; 
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

    hs["validity_period"]               = SMS_VALIDITY_PERIOD;
    hs["svc_type"]                      = SMS_SVC_TYPE;

    hs["ussd_dialog"]                   = USSD_DIALOG;

    hs["whoisd_charging"]                          = OPTIONAL_CHARGING;
    hs["whoisd_message_transport_type"]            = OPTIONAL_MESSAGE_TRANSPORT_TYPE;
    hs["whoisd_expected_message_transport_type"]   = OPTIONAL_EXPECTED_MESSAGE_TRANSPORT_TYPE;

    hs["whoisd_message_content_type"]              = OPTIONAL_MESSAGE_CONTENT_TYPE;
    hs["whoisd_expected_message_content_type"]     = OPTIONAL_EXPECTED_MESSAGE_CONTENT_TYPE;


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

    hs["status"] = STATUS;
    hs["message_id"] = MESSAGE_ID;
    hs["ussd_dialog"] = USSD_DIALOG;

    hs["OA"] = OA;
    hs["DA"] = DA;

    return hs;
}

Hash<int> SmppCommandAdapter::InitDeliverRespFieldNames()
{
    Hash<int> hs;

    hs["status"] = STATUS;
    hs["ussd_dialog"] = USSD_DIALOG;

    hs["OA"] = OA;
    hs["DA"] = DA;

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
    hs["priority_flag"]                 = Tag::SMPP_PRIORITY; 
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

/*    hs["short_message"]                 = Tag::SMPP_SHORT_MESSAGE; //*
    hs["message_payload"]               = Tag::SMPP_MESSAGE_PAYLOAD; //**/
    hs["message_text"]                  = SMS_MESSAGE_BODY;

    hs["source_port"]                   = Tag::SMPP_SOURCE_PORT; 
    hs["destination_port"]              = Tag::SMPP_DESTINATION_PORT; 
    hs["sar_msg_ref_num"]               = Tag::SMPP_SAR_MSG_REF_NUM; 
    hs["sar_total_segments"]            = Tag::SMPP_SAR_TOTAL_SEGMENTS; 
    hs["sar_segment_seqnum"]            = Tag::SMPP_SAR_SEGMENT_SEQNUM; 
    hs["user_response_code"]            = Tag::SMPP_USER_RESPONSE_CODE; //*
    hs["payload_type"]                  = Tag::SMPP_PAYLOAD_TYPE; 
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

    hs["svc_type"]                      = SMS_SVC_TYPE;

    hs["ussd_pssd_ind"]                 = USSD_PSSD_IND;
    hs["ussd_pssr_ind"]                 = USSD_PSSR_IND;
    hs["ussd_pssr_req"]                 = USSD_PSSR_REQ;
    hs["ussd_ussn_req"]                 = USSD_USSN_REQ;
    hs["ussd_pssd_resp"]                = USSD_PSSD_RESP;
    hs["ussd_pssr_resp"]                = USSD_PSSR_RESP;
    hs["ussd_ussr_conf"]                = USSD_USSR_CONF;
    hs["ussd_ussn_conf"]                = USSD_USSN_CONF;

    hs["ussd_dialog"]                   = USSD_DIALOG;

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

void SmppCommandAdapter::WriteDataSmField(SMS& data,int FieldId,AdapterProperty& property)
{
    /*
    if (FieldId == OA) 
        AssignAddress(data.originatingAddress, property.getStr().c_str());
    else if (FieldId == DA) 
        AssignAddress(data.destinationAddress, property.getStr().c_str());
        */
    WriteDeliveryField(data,FieldId,property);
}


void SmppCommandAdapter::WriteSubmitField(SMS& data,int FieldId,AdapterProperty& property)
{
    WriteDeliveryField(data,FieldId,property);
}



void SmppCommandAdapter::WriteDeliveryField(SMS& data,int FieldId,AdapterProperty& property)
{
    int len = 0;
    std::string str;

    if (FieldId == OA) 
        AssignAddress(data.originatingAddress, property.getStr().c_str());
    else if (FieldId == DA) 
        AssignAddress(data.destinationAddress, property.getStr().c_str());
    else if ((FieldId >= DC_BINARY)&&(FieldId <= DC_GSM_MSG_CC)) 
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
        case SMS_SVC_TYPE:
            str = property.getStr();

            if (str.size() >= MAX_ESERVICE_TYPE_LENGTH) 
                len = MAX_ESERVICE_TYPE_LENGTH;
            else
                len = str.size();

             memcpy(data.eServiceType, str.c_str(),str.size()); 
             data.eServiceType[str.size()] = 0;

            break;

        case SMS_MESSAGE_BODY:
            str = property.getStr();

            std::string resStr;

            int code = smsc::smpp::DataCoding::SMSC7BIT;

            if (data.hasIntProperty(Tag::SMPP_DATA_CODING)) 
                code = data.getIntProperty(Tag::SMPP_DATA_CODING);

            int resultLen = 0;

            switch (code) 
            {
            case smsc::smpp::DataCoding::SMSC7BIT:
                Convertor::UTF8ToGSM7Bit(str.data(), str.size(), resStr);
                break;
            case smsc::smpp::DataCoding::LATIN1:
                Convertor::UTF8ToKOI8R(str.data(), str.size(), resStr);
                break;
            case smsc::smpp::DataCoding::UCS2:
                Convertor::UTF8ToUCS2(str.data(), str.size(), resStr);
                break;
            default:
                Convertor::UTF8ToGSM7Bit(str.data(), str.size(), resStr);
            }
  
            //smsc::logger::Logger * logger = smsc::logger::Logger::getInstance("scag.test");

            if (IsShortSize(str.size())) 
            {

                unsigned len = 0;
                if (data.hasBinProperty(Tag::SMPP_SHORT_MESSAGE)) 
                {
                    data.getBinProperty(Tag::SMPP_SHORT_MESSAGE, &len);
                    if (len == 0) 
                    {
                        //smsc_log_debug(logger, "1 String size is: %d", resStr.size());
                        data.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, resStr.data(), resStr.size());
                        data.setIntProperty(Tag::SMPP_SM_LENGTH, 0);
                    } else
                    {
                        data.setBinProperty(Tag::SMPP_SHORT_MESSAGE, resStr.data(), resStr.size());
                        data.setIntProperty(Tag::SMPP_SM_LENGTH, resStr.size());
                    }
                }
                else
                {
                    data.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, resStr.data(), resStr.size());
                    data.setIntProperty(Tag::SMPP_SM_LENGTH, 0);
                }
            }
            else
            {
                if (data.hasBinProperty(Tag::SMPP_SHORT_MESSAGE))
                    data.setBinProperty(Tag::SMPP_SHORT_MESSAGE, 0, 0);

                data.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, str.data(), str.size());
                data.setIntProperty(Tag::SMPP_SM_LENGTH, 0);
            }

            //smsc_log_debug(logger, "MESSAGE BODY: %s", CommandBrige::getMessageBody(command).c_str());
            break;
        }
}



AdapterProperty * SmppCommandAdapter::getSubmitRespProperty(SMS& data, const std::string& name,int FieldId)
{

    AdapterProperty * property = 0;

    switch (FieldId) 
    {
    case STATUS:
        property = new AdapterProperty(name,this,command->get_resp()->get_status());
        break;
    case MESSAGE_ID:
        //TODO: Inmplement
        break;
    case USSD_DIALOG:
        property = new AdapterProperty(name,this,data.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP));
        break;
    case OA:
        property = new AdapterProperty(name,this,CommandBrige::getAbonentAddr(command).toString());
        break;
    case DA:
        property = new AdapterProperty(name,this,CommandBrige::getDestAddr(command).toString());
        break;

    }

    return property;
}

AdapterProperty * SmppCommandAdapter::getDeliverRespProperty(SMS& data, const std::string& name,int FieldId)
{

    AdapterProperty * property = 0;

    switch (FieldId) 
    {
    case STATUS:
        property = new AdapterProperty(name,this,command->get_resp()->get_status());
        break;
    case USSD_DIALOG:
        property = new AdapterProperty(name,this,data.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP));
        break;
    case OA:
        property = new AdapterProperty(name,this,CommandBrige::getAbonentAddr(command).toString());
        break;
    case DA:
        property = new AdapterProperty(name,this,CommandBrige::getDestAddr(command).toString());
        break;
    }

    return property;
}



AdapterProperty * SmppCommandAdapter::GetStrBitFromMask(SMS& data,const std::string& name,int tag,int mask)
{

    int num = data.getIntProperty(tag);

    AdapterProperty * property = new AdapterProperty(name,this,((num&mask)==mask));
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

    char buff[2] = {0,0};
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
        property = new AdapterProperty(name,this,(((num&192)==192)||(num&208)==208));
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
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,1);
        break;
    case USSD_PSSR_IND:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,2);
        break;
    case USSD_PSSR_REQ:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,4);
        break;
    case USSD_USSN_REQ:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,8); 
        break;
    case USSD_PSSD_RESP:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,65536); //2^16
        break;
    case USSD_PSSR_RESP:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,131072); //2^17
        break;
    case USSD_USSR_CONF:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,262144); //2^18
        break;
    case USSD_USSN_CONF:
        property = GetStrBitFromMask(data,name,Tag::SMPP_USSD_SERVICE_OP,524288); //2^16
        break;
    }

    return property;
}


AdapterProperty * SmppCommandAdapter::Get_Unknown_Property(SMS& data, const std::string& name,int FieldId)
{
    if (!data.hasBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS)) return 0;

    unsigned int len;
    const char * buff = data.getBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS, &len);

    uint16_t value;
    uint16_t valueLen;
    if (len < 5) return 0;

    int i = 0;
    while (i < (len - 4)) 
    {
        value = *((uint16_t *)(buff + i));
        valueLen = *((uint16_t *)(buff + i + 2));

        if (valueLen <=0) return 0;
        if ((i + 4 + valueLen) > len) return 0;

        if (value == FieldId) 
        {
            if (valueLen == 1)
            {
                char temp = buff[i + 4];
                return new AdapterProperty(name, this, (temp > 0));
            }

            std::string str;
            str.append((buff + i + 4),valueLen);
            return new AdapterProperty(name, this, str.c_str());
        }

        //if ((i + valueLen + 4 + 2) > (len-1)) return 0;
        i = i + valueLen + 4;

    }
    return 0;
}



AdapterProperty * SmppCommandAdapter::getMessageBodyProperty(SMS& data, std::string name)
{
    std::string str = CommandBrige::getMessageBody(command);
    return new AdapterProperty(name,this,str);
}


AdapterProperty * SmppCommandAdapter::getSubmitProperty(SMS& data,const std::string& name,int FieldId)
{
    AdapterProperty * property = 0;
    char buff[20];
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
    } else if (FieldId == Tag::SMPP_MESSAGE_PAYLOAD) 
    {
        property = new AdapterProperty(name,this,data.getState());
        //property->setPureInt(data.getState());
    } else if ((FieldId >= OPTIONAL_CHARGING)&&(FieldId <= OPTIONAL_EXPECTED_MESSAGE_CONTENT_TYPE))
    {
        property = Get_Unknown_Property(data, name, FieldId);
    } else
    switch (FieldId) 
    {
    case OA:
        property = new AdapterProperty(name,this,data.getOriginatingAddress().toString().c_str());
        break;
    case DA:
        property = new AdapterProperty(name,this,data.getDestinationAddress().toString().c_str());
        break;
    case SMS_VALIDITY_PERIOD:
        property = new AdapterProperty(name,this,data.validTime);
        break;
    case SMS_MESSAGE_BODY:
        property = getMessageBodyProperty(data,name);
        break;
    case SMS_SVC_TYPE:
        property = new AdapterProperty(name, this, data.eServiceType);
        break;
    case USSD_DIALOG:
        property = new AdapterProperty(name,this,data.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP));
        break;
    }

    int tagType = (FieldId >> 8);
    /*
    if (!property) 
    {
        if (tagType == SMS_STR_TAG) 
        {
            if (!data.hasStrProperty(FieldId)) return 0;
            property = new AdapterProperty(name,this,ConvertStrToWStr(data.getStrProperty(FieldId).c_str()));
        } else if (tagType == SMS_INT_TAG) 
        {
            if (!data.hasIntProperty(FieldId)) return 0;
            num = data.getIntProperty(FieldId);
            property = new AdapterProperty(name,this,num);
        }
    }
    */      
    return property;
}

AdapterProperty * SmppCommandAdapter::getDataSmRespProperty(SmsCommand& data,const std::string& name,int FieldId)
{
    SmsResp * smsResp = command->get_resp();
    if (!smsResp) return 0;
    SMS * sms = smsResp->get_sms();

    if (!sms) return 0;


    AdapterProperty * property = 0;

    switch (FieldId) 
    {
    case STATUS:
        property = new AdapterProperty(name,this,command->get_resp()->get_status());
        break;
    case MESSAGE_ID:
        //TODO: Inmplement
        break;
    case USSD_DIALOG:
        property = new AdapterProperty(name,this,sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP));
        break;
    case OA:
        property = new AdapterProperty(name,this,CommandBrige::getAbonentAddr(command).toString());
        break;
    case DA:
        property = new AdapterProperty(name,this,CommandBrige::getDestAddr(command).toString());
        break;
    case PACKET_DIRECTION:
        property = new AdapterProperty(name,this, (int)data.dir);
    }

    return property;
}

AdapterProperty * SmppCommandAdapter::getDataSmProperty(SmsCommand& data,const std::string& name,int FieldId)
{
    AdapterProperty * property = 0;
/*
    dsdSrv2Srv,
    dsdSrv2Sc,
    dsdSc2Srv,
    dsdSc2Sc
*/

    if (FieldId == PACKET_DIRECTION) 
    {
        property = new AdapterProperty(name,this,(int)data.dir);
    }
    else if (data.dir == dsdSc2Srv) 
    {
        property = getDeliverProperty(data.sms, name, FieldId);
    }
    else if (data.dir == dsdSrv2Sc) 
    {
        property = getSubmitProperty(data.sms, name, FieldId);
    }

   /* 
    if ((FieldId >= OPTIONAL_CHARGING)&&(FieldId <= OPTIONAL_EXPECTED_MESSAGE_CONTENT_TYPE))
    {
        property = Get_Unknown_Property(data.sms, name, FieldId);
    } else

    switch (FieldId)
    {
    case OA:
        property = new AdapterProperty(name,this,data.orgSrc.toString().c_str());
        break;
    case DA:
        property = new AdapterProperty(name,this,data.orgDst.toString().c_str());
        break;
    case PACKET_DIRECTION:
        property = new AdapterProperty(name,this,(int)data.dir);
        break;
    }
    */

    return property;
}



AdapterProperty * SmppCommandAdapter::getDeliverProperty(SMS& data,const std::string& name,int FieldId)
{
    AdapterProperty * property = 0;
    char buff[100];
    int num = 0;
    std::string tempStr;

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
    } else if (((FieldId >= ST_ENROUTE)&&(FieldId <= ST_REJECTED))||(FieldId == Tag::SMPP_MESSAGE_PAYLOAD)) 
    {
        property = new AdapterProperty(name,this,data.getState());
        //property->setPureInt(data.getState());
    } else
    switch (FieldId) 
    {
    case OA:
        property = new AdapterProperty(name,this,data.getOriginatingAddress().toString().c_str());
        break;
    case DA:
        property = new AdapterProperty(name,this,data.getDestinationAddress().toString().c_str());
        break;
    case SMS_MESSAGE_BODY:
        property = getMessageBodyProperty(data, name);
        break;
    case SMS_SVC_TYPE:
        property = new AdapterProperty(name, this, data.eServiceType);
        break;
    case USSD_DIALOG:
        property = new AdapterProperty(name,this,data.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP));
        break;
    }
    
    /*
    #define SMS_INT_TAG 0
    #define SMS_STR_TAG 1
    #define SMS_BIN_TAG 2
    */
    int tagType = (FieldId >> 8);
/*
    if (!property) 
    {
        if (tagType == SMS_STR_TAG) 
        {
            if (!data.hasStrProperty(FieldId)) return 0;
            property = new AdapterProperty(name,this,ConvertStrToWStr(data.getStrProperty(FieldId).c_str()));
        } else if (tagType == SMS_INT_TAG) 
        {
            if (!data.hasIntProperty(FieldId)) return 0;

            num = data.getIntProperty(FieldId);
            property = new AdapterProperty(name,this,num);
        }
    } 
*/          
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

    SMS * sms = 0;
    SmsCommand smsCmd;

    CommandId cmdid = command->get_commandId();

    int * pFieldId = 0;
    AdapterProperty * property = 0;

    AdapterProperty ** propertyPtr;
    SmsResp * smsResp = 0;

    switch (cmdid) 
    {
    case DELIVERY:
        sms = command->get_sms();
        if (!sms) return 0;

        pFieldId = DeliverFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return 0;

        propertyPtr = PropertyPul.GetPtr(*pFieldId);
        if (propertyPtr) return (*propertyPtr);

        property = getDeliverProperty(*sms,name,*pFieldId);
        break;
    case SUBMIT:
        sms = command->get_sms();
        if (!sms) return 0;

        pFieldId = SubmitFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return 0;

        propertyPtr = PropertyPul.GetPtr(*pFieldId);
        if (propertyPtr) return (*propertyPtr);

        property = getSubmitProperty(*sms,name,*pFieldId);
        break;
    case DELIVERY_RESP:

        pFieldId = DeliverRespFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return 0;

        propertyPtr = PropertyPul.GetPtr(*pFieldId);
        if (propertyPtr) return (*propertyPtr);

        smsResp = command->get_resp();
        if (!smsResp) return 0;
        sms = smsResp->get_sms();

        if (!sms) return 0;

        property = getDeliverRespProperty(*sms, name,*pFieldId);
        break;
    case SUBMIT_RESP:

        pFieldId = SubmitRespFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return 0;

        propertyPtr = PropertyPul.GetPtr(*pFieldId);
        if (propertyPtr) return (*propertyPtr);

        smsResp = command->get_resp();
        if (!smsResp) return 0;
        sms = smsResp->get_sms();
        if (!sms) return 0;

        property = getSubmitRespProperty(*sms, name,*pFieldId);
        break;
    case DATASM:
        smsCmd = command->get_smsCommand();

        pFieldId = DataSmFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return 0;

        propertyPtr = PropertyPul.GetPtr(*pFieldId);
        if (propertyPtr) return (*propertyPtr);

        property = getDataSmProperty(smsCmd,name,*pFieldId);
        break;
    case DATASM_RESP:

        pFieldId = DataSmRespFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return 0;

        propertyPtr = PropertyPul.GetPtr(*pFieldId);
        if (propertyPtr) return (*propertyPtr);

        property = getDataSmRespProperty(command->get_smsCommand(),name,*pFieldId);
        break;
    }

    if ((property)&&(pFieldId)) PropertyPul.Insert(*pFieldId, property);

    return property;
}


void SmppCommandAdapter::changed(AdapterProperty& property)
{

    SMS * sms = 0;

    CommandId cmdid = command->get_commandId();

    int * pFieldId = 0;
    int receiptMessageId;

    const std::string name = property.GetName();  

    switch (cmdid) 
    {
    case DELIVERY:
        sms = command->get_sms();
        if (sms == 0) return;

        pFieldId = DeliverFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return;

        //TODO: ensure
        receiptMessageId = atoi(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
        //TODO: check - what we must to do?
        if (receiptMessageId) return; 
        
        WriteDeliveryField(*sms,*pFieldId,property);
        break;
    case SUBMIT:
        sms = command->get_sms();
        if (sms == 0) return;

        pFieldId = SubmitFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return;

        WriteSubmitField(*sms,*pFieldId,property);
        break;
    case DELIVERY_RESP:
        if (name!="status") return;
        command->get_resp()->set_status(property.getInt());
        break;
    case SUBMIT_RESP:
        if (name!="status") return;
        command->get_resp()->set_status(property.getInt());
        break;
    case DATASM:
        sms = command->get_sms();
        if (sms == 0) return;

        pFieldId = DataSmFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return;

        WriteDataSmField(*sms,*pFieldId,property);

        break;
    case DATASM_RESP:
        if (name!= "status") return;
        command->get_resp()->set_status(property.getInt());
        break;




    default:
        return;
    }
    /*
    AdapterProperty ** propertyPtr = PropertyPul.GetPtr(FieldId);
    if (propertyPtr) (*propertyPtr)->setPureStr(property.getStr());*/
}

/*
SMS& SmppCommandAdapter::getSMS()
{
    return CommandBrige::getSMS(command);
}



void SmppCommandAdapter::fillChargeOperation(smsc::inman::interaction::ChargeSms& op, TariffRec& tariffRec)
{

    SMS& sms = CommandBrige::getSMS(command);

    char buff[128];
    sprintf(buff,"%d", tariffRec.ServiceNumber);
    std::string str(buff);
    //op.setDestinationSubscriberNumber(sms.getDealiasedDestinationAddress().toString());
    if (command->cmdid == DELIVERY) 
    {
        op.setDestinationSubscriberNumber(str);
        op.setCallingPartyNumber(sms.getOriginatingAddress().toString());
    } else if (command->cmdid == SUBMIT)
    {
        op.setDestinationSubscriberNumber(str);
        op.setCallingPartyNumber(sms.getDestinationAddress().toString());
    }

    op.setCallingIMSI(sms.getOriginatingDescriptor().imsi);
    //op.setSMSCAddress(INManComm::scAddr.toString());
    op.setSubmitTimeTZ(sms.getSubmitTime());
    op.setTPShortMessageSpecificInfo(0x11);
    op.setTPProtocolIdentifier(sms.getIntProperty(Tag::SMPP_PROTOCOL_ID));
    op.setTPDataCodingScheme(sms.getIntProperty(Tag::SMPP_DATA_CODING));
    op.setTPValidityPeriod(sms.getValidTime()-time(NULL));
    op.setLocationInformationMSC(sms.getOriginatingDescriptor().msc);
    op.setCallingSMEid(sms.getSourceSmeId());
    op.setRouteId(sms.getRouteId());

    op.setServiceId(command.getServiceId());

    op.setUserMsgRef(sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)?sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE):-1);
    //op.setMsgId(id);
    op.setServiceOp(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)?sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP):-1);
    op.setPartsNum(sms.getIntProperty(Tag::SMSC_ORIGINALPARTSNUM));

    if(sms.hasBinProperty(Tag::SMPP_SHORT_MESSAGE))
    {
        unsigned len;
        sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
        op.setMsgLength(len);
    }else
    {
        unsigned len;
        sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
        op.setMsgLength(len);
    }
}

void SmppCommandAdapter::fillRespOperation(smsc::inman::interaction::DeliverySmsResult& op, TariffRec& tariffRec)
{
 //   op.setDeliveryTime();
 //   op.setDestIMSI();
 //   op.setDestMSC();
 //   op.setDestSMEid();
 //   op.setDivertedAdr();
 //   op.setResultValue();
}
*/

}}}
