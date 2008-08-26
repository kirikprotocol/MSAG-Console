#include "SmppAdapter2.h"
#include "logger/Logger.h"
#include "CommandAdapter2.h"
#include "CommandBridge.h"
#include "scag/transport/smpp/SmppEntity2.h"
#include "scag/util/encodings/Encodings.h"
#include "scag/sessions/base/Session2.h"
#include "system/status.h"


namespace scag2 {
namespace re {
namespace smpp {

using namespace scag::util::properties;
using namespace scag::util::encodings;
using namespace smsc::system::Status;

Hash<int> SmppCommandAdapter::SubmitFieldNames = SmppCommandAdapter::InitSubmitFieldNames();
Hash<int> SmppCommandAdapter::DeliverFieldNames = SmppCommandAdapter::InitDeliverFieldNames();

Hash<int> SmppCommandAdapter::DataSmFieldNames = SmppCommandAdapter::InitDataSmFieldNames();

Hash<int> SmppCommandAdapter::RespFieldNames = SmppCommandAdapter::InitRespFieldNames();

Hash<int> SmppCommandAdapter::DataSmRespFieldNames = SmppCommandAdapter::InitDataSmRespFieldNames();

IntHash<AccessType> SmppCommandAdapter::SubmitFieldsAccess = SmppCommandAdapter::InitSubmitAccess();
IntHash<AccessType> SmppCommandAdapter::DeliverFieldsAccess = SmppCommandAdapter::InitDeliverAccess();
IntHash<AccessType> SmppCommandAdapter::DataSmFieldsAccess = SmppCommandAdapter::InitDataSmAccess();

IntHash<AccessType> SmppCommandAdapter::RespFieldsAccess = SmppCommandAdapter::InitRespAccess();
IntHash<AccessType> SmppCommandAdapter::DataSmRespFieldsAccess = SmppCommandAdapter::InitDataSmRespAccess();


AccessType SmppCommandAdapter::CheckAccess(int handlerType, const std::string& name)
{
    int * pFieldId;
    AccessType * actype = 0;

    if(!strcmp(name.c_str(), "src_sme_id") || !strcmp(name.c_str(), "dst_sme_id")) return atRead;

    switch (handlerType) 
    {
    case EH_DATA_SM:

        pFieldId = DataSmFieldNames.GetPtr(name.c_str());

        if (!pFieldId) return atNoAccess;

        actype = DataSmFieldsAccess.GetPtr(*pFieldId);
        if (actype) return *actype;

        return atRead;
        break;
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
    case EH_DATA_SM_RESP:
        pFieldId = DataSmRespFieldNames.GetPtr(name.c_str());
        if (pFieldId) {
          actype = DataSmRespFieldsAccess.GetPtr(*pFieldId);
          return actype ? *actype : atRead;
        }
    case EH_DELIVER_SM_RESP:
    case EH_SUBMIT_SM_RESP:
        pFieldId = RespFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return atNoAccess;

        actype = RespFieldsAccess.GetPtr(*pFieldId);
        if (actype) return *actype;

        return atRead;
        break;
    }


    return atNoAccess;
}


IntHash<AccessType> SmppCommandAdapter::InitRespAccess()
{
    IntHash<AccessType> hs;
    hs.Insert(STATUS,atReadWrite);

    return hs;
}

IntHash<AccessType> SmppCommandAdapter::InitDataSmRespAccess()
{
    IntHash<AccessType> hs;
    hs.Insert(SMPP_ADDITIONAL_STATUS_INFO_TEXT, atReadWrite);
    hs.Insert(SMPP_DELIVERY_FAILURE_REASON, atReadWrite);
    hs.Insert(SMPP_DPF_RESULT, atReadWrite);
    hs.Insert(SMPP_NETWORK_ERROR_CODE, atReadWrite);
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

    hs.Insert(Tag::SMPP_SOURCE_PORT, atReadWrite);
    hs.Insert(Tag::SMPP_DESTINATION_PORT, atReadWrite);

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
    hs.Insert(Tag::SMPP_USER_MESSAGE_REFERENCE, atReadWrite);
/*
    hs.Insert(Tag::SMPP_SHORT_MESSAGE, atReadWrite);
    hs.Insert(Tag::SMPP_MESSAGE_PAYLOAD, atReadWrite);*/
    hs.Insert(SMS_MESSAGE_BODY, atReadWrite);

    hs.Insert(Tag::SMPP_USER_RESPONSE_CODE, atReadWrite);
    hs.Insert(Tag::SMPP_LANGUAGE_INDICATOR, atReadWrite);

    hs.Insert(SMS_SVC_TYPE, atReadWrite);

    hs.Insert(Tag::SMPP_SOURCE_PORT, atReadWrite);
    hs.Insert(Tag::SMPP_DESTINATION_PORT, atReadWrite);

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
    hs.Insert(Tag::SMPP_USER_MESSAGE_REFERENCE, atReadWrite );

/*    hs.Insert(Tag::SMPP_SHORT_MESSAGE, atReadWrite);
    hs.Insert(Tag::SMPP_MESSAGE_PAYLOAD, atReadWrite);*/
    hs.Insert(SMS_MESSAGE_BODY, atReadWrite);

    hs.Insert(Tag::SMPP_USER_RESPONSE_CODE, atReadWrite);
    hs.Insert(Tag::SMPP_LANGUAGE_INDICATOR, atReadWrite);

    hs.Insert(SMS_SVC_TYPE, atReadWrite);

    hs.Insert(Tag::SMPP_SOURCE_PORT, atReadWrite);
    hs.Insert(Tag::SMPP_DESTINATION_PORT, atReadWrite);
    
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


    hs["sm_length"]                     = Tag::SMPP_SM_LENGTH; // *
    hs["message_text"]                  = SMS_MESSAGE_BODY;

    hs["source_port"]                   = Tag::SMPP_SOURCE_PORT; 
    hs["destination_port"]              = Tag::SMPP_DESTINATION_PORT; 
    hs["dest_addr_subunit"]             = Tag::SMPP_DEST_ADDR_SUBUNIT; 
    hs["sar_msg_ref_num"]               = Tag::SMPP_SAR_MSG_REF_NUM; 
    hs["sar_total_segments"]            = Tag::SMPP_SAR_TOTAL_SEGMENTS; 
    hs["sar_segment_seqnum"]            = Tag::SMPP_SAR_SEGMENT_SEQNUM; 
    hs["more_messages_to_send"]         = Tag::SMPP_MORE_MESSAGES_TO_SEND; 
    hs["payload_type"]                  = Tag::SMPP_PAYLOAD_TYPE; 
    hs["user_response_code"]            = Tag::SMPP_USER_RESPONSE_CODE; // *
    hs["ms_validity"]                   = Tag::SMPP_MS_VALIDITY; 
    hs["number_of_messages"]            = Tag::SMPP_NUMBER_OF_MESSAGES; 
    hs["language_indicator"]            = Tag::SMPP_LANGUAGE_INDICATOR; // *


    //hs[""] = Tag::SMPP_USSD_SERVICE_OP //mask +

    hs["ussd_pssd_ind"]                 = USSD_PSSD_IND;
    hs["ussd_pssr_ind"]                 = USSD_PSSR_IND;
    hs["ussd_ussr_req"]                 = USSD_USSR_REQ;
    hs["ussd_ussn_req"]                 = USSD_USSN_REQ;
    hs["ussd_pssd_resp"]                = USSD_PSSD_RESP;
    hs["ussd_pssr_resp"]                = USSD_PSSR_RESP;
    hs["ussd_ussr_conf"]                = USSD_USSR_CONF;
    hs["ussd_ussn_conf"]                = USSD_USSN_CONF;

    hs["validity_period"]               = SMS_VALIDITY_PERIOD;
    hs["svc_type"]                      = SMS_SVC_TYPE;

    hs["ussd_dialog"]                   = USSD_DIALOG;

//    hs["whoisd_phone_model"]     = OPTIONAL_PHONE_MODEL;

    return hs;
}


Hash<int> SmppCommandAdapter::InitSubmitFieldNames()
{
    Hash<int> hs;

    hs["packet_direction"]              = PACKET_DIRECTION;

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
    hs["umr"]                           = Tag::SMPP_USER_MESSAGE_REFERENCE;
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
    

    hs["sm_length"]                     = Tag::SMPP_SM_LENGTH;

//   hs["short_message"]                 = Tag::SMPP_SHORT_MESSAGE;
//   hs["message_payload"]               = Tag::SMPP_MESSAGE_PAYLOAD;
    hs["message_text"]                  = SMS_MESSAGE_BODY;

    hs["source_port"]                   = Tag::SMPP_SOURCE_PORT; 
    hs["destination_port"]              = Tag::SMPP_DESTINATION_PORT; 
    hs["dest_addr_subunit"]             = Tag::SMPP_DEST_ADDR_SUBUNIT; 
    hs["sar_msg_ref_num"]               = Tag::SMPP_SAR_MSG_REF_NUM; 
    hs["sar_total_segments"]            = Tag::SMPP_SAR_TOTAL_SEGMENTS; 
    hs["sar_segment_seqnum"]            = Tag::SMPP_SAR_SEGMENT_SEQNUM; 
    hs["more_messages_to_send"]         = Tag::SMPP_MORE_MESSAGES_TO_SEND; 
    hs["payload_type"]                  = Tag::SMPP_PAYLOAD_TYPE; 
    hs["user_response_code"]            = Tag::SMPP_USER_RESPONSE_CODE; // *
    hs["ms_validity"]                   = Tag::SMPP_MS_VALIDITY; 
    hs["number_of_messages"]            = Tag::SMPP_NUMBER_OF_MESSAGES; 
    hs["language_indicator"]            = Tag::SMPP_LANGUAGE_INDICATOR; // *


    //hs[""] = Tag::SMPP_USSD_SERVICE_OP //mask +

    hs["ussd_pssd_ind"]                 = USSD_PSSD_IND;
    hs["ussd_pssr_ind"]                 = USSD_PSSR_IND;
    hs["ussd_ussr_req"]                 = USSD_USSR_REQ;
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

//    hs["whoisd_phone_model"]     = OPTIONAL_PHONE_MODEL;

/*
Tag::SMPP_ESM_CLASS //mask +
Tag::SMPP_PROTOCOL_ID //+
Tag::SMPP_PRIORITY // +
Tag::SMPP_SCHEDULE_DELIVERY_TIME //+
Tag::SMPP_REGISTRED_DELIVERY//+
Tag::SMPP_REPLACE_IF_PRESENT_FLAG//+

Tag::SMPP_DATA_CODING //mask *
Tag::SMPP_SM_LENGTH // *
Tag::SMPP_SHORT_MESSAGE // *
Tag::SMPP_SOURCE_PORT //+
Tag::SMPP_DESTINATION_PORT //+
Tag::SMPP_DEST_ADDR_SUBUNIT //+
Tag::SMPP_SAR_MSG_REF_NUM //+
Tag::SMPP_SAR_TOTAL_SEGMENTS //+
Tag::SMPP_SAR_SEGMENT_SEQNUM //+
Tag::SMPP_MORE_MESSAGES_TO_SEND //+
Tag::SMPP_PAYLOAD_TYPE //+
Tag::SMPP_MESSAGE_PAYLOAD // *
Tag::SMPP_USER_RESPONSE_CODE // *
Tag::SMPP_MS_VALIDITY //+
Tag::SMPP_NUMBER_OF_MESSAGES // +
Tag::SMPP_LANGUAGE_INDICATOR // *
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

Hash<int> SmppCommandAdapter::InitDataSmRespFieldNames() {
  Hash<int> hs;

  hs["delivery_failure_reason"]      = SMPP_DELIVERY_FAILURE_REASON;
  hs["network_error_code"]           = SMPP_NETWORK_ERROR_CODE;
  hs["additional_status_info_text"]  = SMPP_ADDITIONAL_STATUS_INFO_TEXT;
  hs["dpf_result"]                   = SMPP_DPF_RESULT;

  return hs;
}


Hash<int> SmppCommandAdapter::InitRespFieldNames()
{
    Hash<int> hs;

    hs["OA"]                            = OA;
    hs["DA"]                            = DA;

    hs["packet_direction"]              = PACKET_DIRECTION;

    hs["status"]                        = STATUS;

    hs["message_id"]                    = MESSAGE_ID;
    hs["ussd_dialog"]                   = USSD_DIALOG;

    hs["status_ok"]                     = STATUS_OK;
    hs["status_perm_error"]             = STATUS_PERM_ERROR;
    hs["status_temp_error"]             = STATUS_TEMP_ERROR;

    return hs;
}


Hash<int> SmppCommandAdapter::InitDeliverFieldNames()
{
    Hash<int> hs;

    hs["packet_direction"]              = PACKET_DIRECTION;

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
    hs["umr"]                           = Tag::SMPP_USER_MESSAGE_REFERENCE;
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


    hs["sm_length"]                     = Tag::SMPP_SM_LENGTH; // *

/*    hs["short_message"]                 = Tag::SMPP_SHORT_MESSAGE; // *
    hs["message_payload"]               = Tag::SMPP_MESSAGE_PAYLOAD; // * */
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
    hs["ussd_ussr_req"]                 = USSD_USSR_REQ;
    hs["ussd_ussn_req"]                 = USSD_USSN_REQ;
    hs["ussd_pssd_resp"]                = USSD_PSSD_RESP;
    hs["ussd_pssr_resp"]                = USSD_PSSR_RESP;
    hs["ussd_ussr_conf"]                = USSD_USSR_CONF;
    hs["ussd_ussn_conf"]                = USSD_USSN_CONF;

    hs["ussd_dialog"]                   = USSD_DIALOG;

//    hs["whoisd_phone_model"]     = OPTIONAL_PHONE_MODEL;

    return hs;
}


void SmppCommandAdapter::Set_DC_BIT_Property(SMS& data,int FieldId,bool value)
{
    
    if(FieldId == DC_BINARY) 
        data.setIntProperty(Tag::SMPP_DATA_CODING,value ? 2 : 4); //00000010
    else if(FieldId == DC_TEXT) {
        data.setIntProperty(Tag::SMPP_DATA_CODING, value ? 4 : 2); //00000100
    } else if(FieldId >= DC_SMSC_DEFAULT && FieldId <= DC_GSM_MSG_CC) 
    {
        //TODO: check DC_TEXT field first!
        if(!(data.getIntProperty(Tag::SMPP_DATA_CODING) & 4)) return;
        
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
                case DC_GSM_MWI:        dc = 192 | (data.getIntProperty(Tag::SMPP_DATA_CODING) & 15); break;
                case DC_GSM_MSG_CC:     dc = 240 | (data.getIntProperty(Tag::SMPP_DATA_CODING) & 15); break; //1111xxxx
            }
        }
        else if(FieldId == DC_GSM_MWI)
            dc = 240 | (data.getIntProperty(Tag::SMPP_DATA_CODING) & 15);
        else if(FieldId == DC_GSM_MSG_CC)
            dc = 192|(data.getIntProperty(Tag::SMPP_DATA_CODING) & 15);

        data.setIntProperty(Tag::SMPP_DATA_CODING, dc);
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
    unsigned len = 0;
    std::string str;

    if (FieldId == OA) 
        AssignAddress(data.originatingAddress, property.getStr().c_str());
    else if (FieldId == DA) 
        AssignAddress(data.destinationAddress, property.getStr().c_str());
    else if ((FieldId >= DC_BINARY)&&(FieldId <= DC_GSM_MSG_CC)) 
        Set_DC_BIT_Property(data,FieldId,property.getBool());
    else 
        switch (FieldId)
        {
        case Tag::SMPP_USER_MESSAGE_REFERENCE:
            // we also need to switch session ussd ref
            if ( ! command.getSession() )
                throw SCAGException( "command has no session to change umr in" );
            command.getSession()->setUSSDref( unsigned(property.getInt()) );
        case Tag::SMPP_SM_LENGTH:
        case Tag::SMPP_USER_RESPONSE_CODE:
        case Tag::SMPP_LANGUAGE_INDICATOR:
        case Tag::SMPP_SOURCE_PORT:
        case Tag::SMPP_DESTINATION_PORT:
            data.setIntProperty(FieldId, unsigned(property.getInt()));
            break;
        case SMS_SVC_TYPE:
            str = property.getStr();

            if (str.size() >= unsigned(MAX_ESERVICE_TYPE_LENGTH))
                len = MAX_ESERVICE_TYPE_LENGTH;
            else
                len = str.size();

             memcpy(data.eServiceType, str.c_str(),str.size()); 
             data.eServiceType[str.size()] = 0;

            break;

        case SMS_MESSAGE_BODY:
            str = property.getStr();

            std::string resStr;

            int code = smsc::smpp::DataCoding::UCS2;

            if(!CommandBridge::hasMSB(str.data(), str.size()))
                code = smsc::smpp::DataCoding::SMSC7BIT;
                
            data.setIntProperty(Tag::SMPP_DATA_CODING, code);
            
            switch (code) 
            {
                case smsc::smpp::DataCoding::UCS2:
                    Convertor::UTF8ToUCS2(str.data(), str.size(), resStr);
                    break;
                default:
                    Convertor::UTF8ToGSM7Bit(str.data(), str.size(), resStr);
            }
  
            if (IsShortSize(resStr.size()) && data.hasBinProperty(Tag::SMPP_SHORT_MESSAGE)) 
            {
                unsigned mlen;
                data.getBinProperty(Tag::SMPP_SHORT_MESSAGE, &mlen);
                if (mlen) 
                {
                   data.setBinProperty(Tag::SMPP_SHORT_MESSAGE, resStr.data(), resStr.size());
                   data.setIntProperty(Tag::SMPP_SM_LENGTH, resStr.size());
                   return;
                }
            }

            if (data.hasBinProperty(Tag::SMPP_SHORT_MESSAGE))
                data.setBinProperty(Tag::SMPP_SHORT_MESSAGE, 0, 0);

            data.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, resStr.data(), resStr.size());
            data.setIntProperty(Tag::SMPP_SM_LENGTH, 0);

            break;
        }
}

void SmppCommandAdapter::WriteDataSmRespField(int fieldId, AdapterProperty& property) {
    SmsResp* resp = command.get_resp();
    if (!resp) {
        return;
    }
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
  }
}

AdapterProperty * SmppCommandAdapter::getDataSmRespProperty(const std::string& name, int fieldId) {
  SmsResp* resp = command.get_resp();
  if (!resp) {
    return 0;
  }
  AdapterProperty * property = 0;
  switch (fieldId) {
  case SMPP_ADDITIONAL_STATUS_INFO_TEXT:
    property = new AdapterProperty(name, this, resp->getAdditionalStatusInfoText());
    break;
  case SMPP_DELIVERY_FAILURE_REASON: 
    {
      int val = resp->hasDeliveryFailureReason() ? resp->getDeliveryFailureReason() : -1;
      property = new AdapterProperty(name, this, val);
      break;
    }
  case SMPP_DPF_RESULT:
    property = new AdapterProperty(name, this, resp->getDpfResult());
    break;
  case SMPP_NETWORK_ERROR_CODE:
    property = new AdapterProperty(name, this, resp->getNetworkErrorCode());
    break;
  }
  return property;
}

AdapterProperty * SmppCommandAdapter::getRespProperty(SMS& data,const std::string& name,int FieldId)
{
    
    AdapterProperty * property = 0;

    switch (FieldId) 
    {
    case STATUS:
        property = new AdapterProperty(name,this,command.get_status());
        break;
    case PACKET_DIRECTION:
        property = new AdapterProperty(name,this,command.get_resp()->get_dir());
        break;
    case MESSAGE_ID:
        property = new AdapterProperty(name,this,command.get_resp()->get_messageId());
        break;
    case USSD_DIALOG:
        property = new AdapterProperty(name,this,data.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP));
        break;
    case OA:
        property = new AdapterProperty(name,this,CommandBridge::getAbonentAddr(command).toString());
        break;
    case DA:
        property = new AdapterProperty(name,this,CommandBridge::getDestAddr(command).toString());
        break;
    case STATUS_OK:
        property = new AdapterProperty(name,this,(command.get_status() == 0));
        break;
    case STATUS_PERM_ERROR:
        if (command.get_status() > 0)
            property = new AdapterProperty(name,this,isErrorPermanent(command.get_status()));
        else
            property = new AdapterProperty(name,this,0);

        break;
    case STATUS_TEMP_ERROR:
        if (command.get_status() > 0) 
            property = new AdapterProperty(name,this,!isErrorPermanent(command.get_status()));
        else
            property = new AdapterProperty(name,this,0);
        break;
    }

    return property;
}


AdapterProperty * SmppCommandAdapter::GetStrBitFromMask(SMS& data,const std::string& name,int tag,int mask)
{
    return new AdapterProperty(name, this, int(data.getIntProperty(tag) & mask) == mask);
}

void SmppCommandAdapter::SetBitMask(SMS& data, int tag, int mask)
{
    if (data.hasIntProperty(tag)) 
        data.setIntProperty(tag, data.getIntProperty(tag) & mask);
}

AdapterProperty * SmppCommandAdapter::Get_ESM_BIT_Property(SMS& data, const std::string& name,int FieldId)
{
    int esm = 0; // ESM_SMSC_DEFAULT, ESM_MT_DEFAULT, ESM_NSF_NONE
    switch (FieldId) 
    {
        case ESM_MM_DATAGRAM: esm = 1; break;
        case ESM_MM_FORWARD: esm = 2; break;
        case ESM_MM_S_AND_F: esm = 3; break;
        case ESM_MT_DELIVERY_ACK: esm = 0x8; break;
        case ESM_MT_MANUAL_ACK: esm = 0x16; break;
        case ESM_NSF_UDHI: esm = 64; break;
        case ESM_NSF_SRP: esm = 128; break;
        case ESM_NSF_BOTH: esm = 192; break;
    }
    return GetStrBitFromMask(data, name, Tag::SMPP_ESM_CLASS, esm);
}

AdapterProperty * SmppCommandAdapter::Get_RD_BIT_Property(SMS& data, const std::string& name,int FieldId)
{
    int rd = 0; // RD_RECEIPT_OFF, RD_ACK_OFF
    switch (FieldId) 
    {
        case RD_RECEIPT_ON: rd = 1; break;
        case RD_RECEIPT_FAILURE: rd = 3; break;
        case RD_ACK_ON: rd = 4; break;
        case RD_ACK_MAN_ON: rd = 8; break;
        case RD_RD_ACK_BOTH_ON: rd = 12; break;
        case RD_I_NOTIFICATION: rd = 16; break;
    }

    return GetStrBitFromMask(data, name, Tag::SMPP_REGISTRED_DELIVERY, rd);
}


AdapterProperty * SmppCommandAdapter::Get_DC_BIT_Property(SMS& data, const std::string& name,int FieldId)
{
    int dc = 0; // DC_SMSC_DEFAULT
    // char buff[2] = {0,0};
    int num = 0;

    switch (FieldId) 
    {
        case DC_BINARY:     dc = 2; break;
        case DC_TEXT:       dc = 4; break;
        case DC_ASCII_X34:  dc = 1; break;
        case DC_LATIN1:     dc = 3; break;
        case DC_JIS:        dc = 5; break;
        case DC_CYRILIC:    dc = 6; break;
        case DC_LATIN_HEBREW: dc = 7; break;
        case DC_UCS2:       dc = 8; break;
        case DC_PICTOGRAM_ENC: dc = 9; break;
        case DC_ISO_MUSIC_CODES: dc = 10; break;
        case DC_E_KANJI_JIS: dc = 13; break;
        case DC_KS_C_5601:  dc = 14; break;
        case DC_GSM_MWI:
            num = data.getIntProperty(Tag::SMPP_DATA_CODING);
            dc = ((num & 192) == 192) || (num & 208) == 208;
            break;
        case DC_GSM_MSG_CC: dc = 240; break;
    }

    return GetStrBitFromMask(data,name,Tag::SMPP_DATA_CODING, dc);
}

AdapterProperty * SmppCommandAdapter::Get_USSD_BOOL_Property(SMS& data, const std::string& name,int FieldId)
{
    if (!data.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) return 0;

    int sop = data.getIntProperty(Tag::SMPP_USSD_SERVICE_OP);

    /*
    ussd_pssd_ind  0
    ussd_pssr_ind  1
    ussd_ussr_req  2
    ussd_ussn_req  3
    ussd_pssd_resp 16
    ussd_pssr_resp 17
    ussd_ussr_conf 18
    ussd_ussn_conf 19


  static const int8_t PSSD_INDICATION        = 0;
  static const int8_t PSSR_INDICATION        = 1;
  static const int8_t USSR_REQUEST           = 2;
  static const int8_t USSN_REQUEST           = 3;
  static const int8_t PSSD_RESPONSE          = 16;
  static const int8_t PSSR_RESPONSE          = 17;
  static const int8_t USSR_CONFIRM           = 18;
  static const int8_t USSN_CONFIRM           = 19;
    
    */

    bool b = false;
    switch (FieldId) 
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

    return new AdapterProperty(name, this, b);
}


AdapterProperty * SmppCommandAdapter::Get_Unknown_Property(SMS& data, const std::string& name,int FieldId)
{
    if (!data.hasBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS)) return 0;

    unsigned int len;
    const char * buff = data.getBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS, &len);

    uint16_t value;
    uint16_t valueLen;
    if (len < 5) return 0;

    unsigned int i = 0;
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
                return new AdapterProperty(name, this, temp);
//                return new AdapterProperty(name, this, (temp > 0));
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
    return new AdapterProperty(name,this,CommandBridge::getMessageBody(command));
}


AdapterProperty * SmppCommandAdapter::getSubmitProperty(SMS& data,const std::string& name,int FieldId)
{
    AdapterProperty * property = 0;

    if ((FieldId >= ESM_MM_SMSC_DEFAULT)&&(FieldId <= ESM_NSF_BOTH)) 
        property = Get_ESM_BIT_Property(data,name,FieldId);
    else if ((FieldId >= RD_RECEIPT_OFF)&&(FieldId <= RD_I_NOTIFICATION)) 
        property = Get_RD_BIT_Property(data,name,FieldId);
    else if ((FieldId >= DC_BINARY)&&(FieldId <= DC_GSM_MSG_CC)) 
        property = Get_DC_BIT_Property(data,name,FieldId);
    else if ((FieldId >= USSD_PSSD_IND)&&(FieldId <= USSD_USSN_CONF)) 
        property = Get_USSD_BOOL_Property(data,name,FieldId);
    else if (FieldId == Tag::SMPP_MESSAGE_PAYLOAD) 
        property = new AdapterProperty(name,this,data.getState());
        //property->setPureInt(data.getState());
    else if ((FieldId >= OPTIONAL_CHARGING)&&(FieldId <= OPTIONAL_EXPECTED_MESSAGE_CONTENT_TYPE))
        property = Get_Unknown_Property(data, name, FieldId);
    else
    switch (FieldId) 
    {
    case PACKET_DIRECTION:
        property = new AdapterProperty(name,this,command.get_smsCommand().dir);
        break;
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
    
    if (!property) 
    {
        if (tagType == SMS_STR_TAG)
            property = new AdapterProperty(name, this, data.hasStrProperty(FieldId) ? data.getStrProperty(FieldId).c_str() : "");
        else if (tagType == SMS_INT_TAG)
            property = new AdapterProperty(name, this, data.hasIntProperty(FieldId) ? data.getIntProperty(FieldId) : 0);
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
        property = new AdapterProperty(name,this,(int)data.dir);
    else if (data.dir == dsdSc2Srv) 
        property = getDeliverProperty(data.sms, name, FieldId);
    else if (data.dir == dsdSrv2Sc) 
        property = getSubmitProperty(data.sms, name, FieldId);

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
        property = Get_USSD_BOOL_Property(data,name,FieldId);
    } else if (((FieldId >= ST_ENROUTE)&&(FieldId <= ST_REJECTED))||(FieldId == Tag::SMPP_MESSAGE_PAYLOAD)) 
    {
        property = new AdapterProperty(name,this,data.getState());
        //property->setPureInt(data.getState());
    } else
    switch (FieldId) 
    {
    case PACKET_DIRECTION:
        property = new AdapterProperty(name,this,command.get_smsCommand().dir);
        break;
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

    if (!property) 
    {
        if (tagType == SMS_STR_TAG) 
            property = new AdapterProperty(name,this, data.hasStrProperty(FieldId) ? data.getStrProperty(FieldId).c_str() : "");
        else if (tagType == SMS_INT_TAG) 
            property = new AdapterProperty(name, this, data.hasIntProperty(FieldId) ? data.getIntProperty(FieldId) : 0);
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
    if(src_sme_id) delete src_sme_id;
    if(dst_sme_id) delete dst_sme_id;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////



Property* SmppCommandAdapter::getProperty(const std::string& name)
{

    SMS * sms = 0;

    CommandId cmdid = CommandId(command.getCommandId());

    int * pFieldId = 0;
    AdapterProperty * property = 0;

    AdapterProperty ** propertyPtr;
    SmsResp * smsResp = 0;

    if(!strcmp(name.c_str(), "src_sme_id"))
    {
        if(!src_sme_id)
            src_sme_id = new AdapterProperty(name, this, command.getEntity()->getSystemId());
        return src_sme_id;
    }
    else if(!strcmp(name.c_str(), "dst_sme_id"))
    {
        if(!dst_sme_id)
            dst_sme_id = new AdapterProperty(name, this, command.getDstEntity()->getSystemId());
        return dst_sme_id;
    }

    switch (cmdid) 
    {
    case DELIVERY:
        sms = command.get_sms();
        if (!sms) return 0;

        pFieldId = DeliverFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return 0;

        propertyPtr = PropertyPul.GetPtr(*pFieldId);
        if (propertyPtr) return (*propertyPtr);

        property = getDeliverProperty(*sms,name,*pFieldId);
        break;
    case SUBMIT:
        sms = command.get_sms();
        if (!sms) return 0;

        pFieldId = SubmitFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return 0;

        propertyPtr = PropertyPul.GetPtr(*pFieldId);
        if (propertyPtr) return (*propertyPtr);

        property = getSubmitProperty(*sms,name,*pFieldId);
        break;

    case DATASM:
        pFieldId = DataSmFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return 0;

        propertyPtr = PropertyPul.GetPtr(*pFieldId);
        if (propertyPtr) return (*propertyPtr);

        property = getDataSmProperty(command.get_smsCommand(),name,*pFieldId);
        break;

    case DATASM_RESP:
        pFieldId = DataSmRespFieldNames.GetPtr(name.c_str());
        if (pFieldId) {
          propertyPtr = PropertyPul.GetPtr(*pFieldId);
          if (propertyPtr) return (*propertyPtr);
          property = getDataSmRespProperty(name, *pFieldId);
          break;
        }
    case SUBMIT_RESP:
    case DELIVERY_RESP:

        pFieldId = RespFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return 0;

        propertyPtr = PropertyPul.GetPtr(*pFieldId);
        if (propertyPtr) return (*propertyPtr);

        smsResp = command.get_resp();
        if (!smsResp) return 0;
        sms = smsResp->get_sms();
        if (!sms) return 0;

        property = getRespProperty(*sms, name,*pFieldId);
        break;
    default:
        break;
    }

    if ((property)&&(pFieldId)) PropertyPul.Insert(*pFieldId, property);

    return property;
}

void SmppCommandAdapter::changed(AdapterProperty& property)
{

    SMS * sms = 0;

    CommandId cmdid = CommandId(command.getCommandId());

    int * pFieldId = 0;
    int receiptMessageId;

    const std::string name = property.GetName();  

    switch (cmdid) 
    {
    case DELIVERY:
        sms = command.get_sms();
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
        sms = command.get_sms();
        if (sms == 0) return;

        pFieldId = SubmitFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return;

        WriteSubmitField(*sms,*pFieldId,property);
        break;
    case DATASM:
        sms = command.get_sms();
        if (sms == 0) return;

        pFieldId = DataSmFieldNames.GetPtr(name.c_str());
        if (!pFieldId) return;

        WriteDataSmField(*sms,*pFieldId,property);

        break;
    case DATASM_RESP:
        pFieldId = DataSmRespFieldNames.GetPtr(name.c_str());
        if (pFieldId) {
          WriteDataSmRespField(*pFieldId, property);
          break;
        }
    case DELIVERY_RESP:
    case SUBMIT_RESP:
        if (name!= "status") return;
        // if (!command.get_resp()) return;
        command.set_status( int(property.getInt()) );
        break;

    default:
        return;
    }
}

}}}
