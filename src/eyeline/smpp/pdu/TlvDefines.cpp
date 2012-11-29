#include "TlvDefines.h"
#include <algorithm>
#include <strings.h>

namespace eyeline {
namespace smpp {

const unsigned TlvUtil::KNOWN_TLVS; // to weed dumb compilers

const TlvUtil::TlvDef TlvUtil::tlv_defs[] = {
TlvUtil::TlvDef( TAG_DEST_ADDR_SUBUNIT           ,      1,      TLVUINT8,      "dest_addr_subunit",               "4.8.4.23 (5.3.2.1)"),
TlvUtil::TlvDef( TAG_DEST_NETWORK_TYPE           ,      1,      TLVUINT8,      "dest_network_type",               "4.8.4.26 (5.3.2.3)"),
TlvUtil::TlvDef( TAG_DEST_BEARER_TYPE            ,      1,      TLVUINT8,      "dest_bearer_type",                "4.8.4.24 (5.3.2.5)"),
TlvUtil::TlvDef( TAG_DEST_TELEMATICS_ID          ,      2,      TLVUINT16,     "dest_telematics_id",              "4.8.4.29 (5.3.2.7)"),
TlvUtil::TlvDef( TAG_SOURCE_ADDR_SUBUNIT         ,      1,      TLVUINT8,      "source_addr_subunit",             "4.8.4.54 (5.3.2.2)"),
TlvUtil::TlvDef( TAG_SOURCE_NETWORK_TYPE         ,      1,      TLVUINT8,      "source_network_type",             "4.8.4.57 (5.3.2.4)"),
TlvUtil::TlvDef( TAG_SOURCE_BEARER_TYPE          ,      1,      TLVUINT8,      "source_bearer_type",              "4.8.4.55 (5.3.2.6)"),
TlvUtil::TlvDef( TAG_SOURCE_TELEMATICS_ID        ,      1,      TLVUINT8,      "source_telematics_id",            "4.8.4.61 (5.3.2.8)"),
TlvUtil::TlvDef( TAG_QOS_TIME_TO_LIVE            ,      4,      TLVUINT32,     "qos_time_to_live",                "4.8.4.46 (5.3.2.9)"),
TlvUtil::TlvDef( TAG_PAYLOAD_TYPE                ,      1,      TLVUINT8,      "payload_type",                    "4.8.4.44 (5.3.2.10)"),
TlvUtil::TlvDef( TAG_ADDITIONAL_STATUS_INFO_TEXT ,      256,    TLVCSTRING,    "additional_status_info_text",     "4.8.4.1 (5.3.2.11)"),
TlvUtil::TlvDef( TAG_RECEIPTED_MESSAGE_ID        ,      65,     TLVCSTRING,    "receipted_message_id",            "4.8.4.47 (5.3.2.12)"),
TlvUtil::TlvDef( TAG_MS_MSG_WAIT_FACILITIES      ,      1,      TLVUINT8,      "ms_msg_wait_facilities",          "4.8.4.40 (5.3.2.13)"),
TlvUtil::TlvDef( TAG_PRIVACY_INDICATOR           ,      1,      TLVUINT8,      "privacy_indicator",               "4.8.4.45 (5.3.2.14)"),
TlvUtil::TlvDef( TAG_SOURCE_SUBADDRESS           ,      23,     TLVOCTET,      "source_subaddress",               "4.8.4.60 (5.3.2.15)"),
TlvUtil::TlvDef( TAG_DEST_SUBADDRESS             ,      23,     TLVOCTET,      "dest_subaddress",                 "4.8.4.28 (5.3.2.16)"),
TlvUtil::TlvDef( TAG_USER_MESSAGE_REFERENCE      ,      2,      TLVUINT16,     "user_message_reference",          "4.8.4.62 (5.3.2.17)"),
TlvUtil::TlvDef( TAG_USER_RESPONSE_CODE          ,      1,      TLVUINT8,      "user_response_code",              "4.8.4.63 (5.3.2.18)"),
TlvUtil::TlvDef( TAG_SOURCE_PORT                 ,      2,      TLVUINT16,     "source_port",                     "4.8.4.59 (5.3.2.20)"),
TlvUtil::TlvDef( TAG_DEST_PORT                   ,      2,      TLVUINT16,     "dest_port",                       "4.8.4.30 (5.3.2.21)"),
TlvUtil::TlvDef( TAG_SAR_MSG_REF_NUM             ,      2,      TLVUINT16,     "sar_msg_ref_num",                 "4.8.4.48 (5.3.2.22)"),
TlvUtil::TlvDef( TAG_LANGUAGE_INDICATOR          ,      1,      TLVUINT8,      "language_indicator",              "4.8.4.35 (5.3.2.19)"),
TlvUtil::TlvDef( TAG_SAR_TOTAL_SEGMENTS          ,      1,      TLVUINT8,      "sar_total_segments",              "4.8.4.50 (5.3.2.23)"),
TlvUtil::TlvDef( TAG_SAR_SEGMENT_SEQNUM          ,      1,      TLVUINT8,      "sar_segment_seqnum",              "4.8.4.49 (5.3.2.24)"),
TlvUtil::TlvDef( TAG_SC_INTERFACE_VERSION        ,      1,      TLVUINT8,      "sc_interface_version",            "4.8.4.51 (5.3.2.25)"),
TlvUtil::TlvDef( TAG_CALLBACK_NUM_PRES_IND       ,      1,      TLVUINT8,      "callback_num_pres_ind",           "4.8.4.17 (5.3.2.37)"),
TlvUtil::TlvDef( TAG_CALLBACK_NUM_ATAG           ,      65,     TLVOCTET,      "callback_num_atag",               "4.8.4.16 (5.3.2.38)"),
TlvUtil::TlvDef( TAG_NUMBER_OF_MESSAGES          ,      1,      TLVUINT8,      "number_of_messages",              "4.8.4.43 (5.3.2.39)"),
TlvUtil::TlvDef( TAG_CALLBACK_NUM                ,      19,     TLVOCTET,      "callback_num",                    "4.8.4.15 (5.3.2.36)"),
TlvUtil::TlvDef( TAG_DPF_RESULT                  ,      1,      TLVUINT8,      "dpf_result",                      "4.8.4.32 (5.3.2.28)"),
TlvUtil::TlvDef( TAG_SET_DPF                     ,      1,      TLVUINT8,      "set_dpf",                         "4.8.4.52 (5.3.2.29)"),
TlvUtil::TlvDef( TAG_MS_AVAILABILITY_STATUS      ,      1,      TLVUINT8,      "ms_availability_status",          "4.8.4.39 (5.3.2.30)"),
TlvUtil::TlvDef( TAG_NETWORK_ERROR_CODE          ,      3,      TLVOCTET,      "network_error_code",              "4.8.4.42 (5.3.2.31)"),
TlvUtil::TlvDef( TAG_MESSAGE_PAYLOAD             ,      0xffff, TLVOCTET,      "message_payload",                 "4.8.4.36 (5.3.2.32)"),
TlvUtil::TlvDef( TAG_DELIVERY_FAILURE_REASON     ,      1,      TLVUINT8,      "delivery_failure_reason",         "4.8.4.19 (5.3.2.33)"),
TlvUtil::TlvDef( TAG_MORE_MESSAGES_TO_SEND       ,      1,      TLVUINT8,      "more_messages_to_send",           "4.8.4.38 (5.3.2.34)"),
TlvUtil::TlvDef( TAG_MESSAGE_STATE               ,      1,      TLVUINT8,      "message_state",                   "4.8.4.37 (5.3.2.35)"),
TlvUtil::TlvDef( TAG_CONGESTION_STATE            ,      1,      TLVUINT8,      "congestion_state",                "4.8.4.18 ()"),
TlvUtil::TlvDef( TAG_USSD_SERVICE_OP             ,      1,      TLVUINT8,      "ussd_service_op",                 "4.8.4.64 (5.3.2.44)"),
TlvUtil::TlvDef( TAG_BROADCAST_CHANNEL_INDICATOR ,      1,      TLVUINT8,      "broadcast_channel_indicator",     "4.8.4.7 ()"),
TlvUtil::TlvDef( TAG_BROADCAST_CONTENT_TYPE      ,      3,      TLVOCTET,      "broadcast_content_type",          "4.8.4.8 ()"),
TlvUtil::TlvDef( TAG_BROADCAST_CONTENT_TYPE_INFO ,      255,    TLVOCTET,      "broadcast_content_type_info",     "4.8.4.6 ()"),
TlvUtil::TlvDef( TAG_BROADCAST_MESSAGE_CLASS     ,      1,      TLVUINT8,      "broadcast_message_class",         "4.8.4.12 ()"),
TlvUtil::TlvDef( TAG_BROADCAST_REP_NUM           ,      2,      TLVUINT16,     "broadcast_rep_num",               "4.8.4.13 ()"),
TlvUtil::TlvDef( TAG_BROADCAST_FREQUENCY_INTERVAL,      3,      TLVOCTET,      "broadcast_frequency_interval",    "4.8.4.11 ()"),
TlvUtil::TlvDef( TAG_BROADCAST_AREA_IDENTIFIER   ,      0xffff, TLVOCTET,      "broadcast_area_identifier",       "4.8.4.4 ()"),
TlvUtil::TlvDef( TAG_BROADCAST_ERROR_STATUS      ,      4,      TLVUINT32,     "broadcast_error_status",          "4.8.4.10 ()"),
TlvUtil::TlvDef( TAG_BROADCAST_AREA_SUCCESS      ,      1,      TLVUINT8,      "broadcast_area_success",          "4.8.4.5 ()"),
TlvUtil::TlvDef( TAG_BROADCAST_END_TIME          ,      16,     TLVCSTRING,    "broadcast_end_time",              "4.8.4.9 ()"),
TlvUtil::TlvDef( TAG_BROADCAST_SERVICE_GROUP     ,      255,    TLVOCTET,      "broadcast_service_group",         "4.8.4.14 ()"),
TlvUtil::TlvDef( TAG_BILLING_IDENTIFICATION      ,      1024,   TLVOCTET,      "billing_identification",          "4.8.4.3 ()"),
TlvUtil::TlvDef( TAG_SOURCE_NETWORK_ID           ,      65,     TLVCSTRING,    "source_network_id",               "4.8.4.56 ()"),
TlvUtil::TlvDef( TAG_DEST_NETWORK_ID             ,      65,     TLVCSTRING,    "dest_network_id",                 "4.8.4.25 ()"),
TlvUtil::TlvDef( TAG_SOURCE_NODE_ID              ,      6,      TLVOCTET,      "source_node_id",                  "4.8.4.58 ()"),
TlvUtil::TlvDef( TAG_DEST_NODE_ID                ,      6,      TLVOCTET,      "dest_node_id",                    "4.8.4.27 ()"),
TlvUtil::TlvDef( TAG_DEST_ADDR_NP_RESOLUTION     ,      1,      TLVUINT8,      "dest_addr_np_resolution",         "4.8.4.22 ()"),
TlvUtil::TlvDef( TAG_DEST_ADDR_NP_INFORMATION    ,      10,     TLVOCTET,      "dest_addr_np_information",        "4.8.4.21 ()"),
TlvUtil::TlvDef( TAG_DEST_ADDR_NP_COUNTRY        ,      5,      TLVOCTET,      "dest_addr_np_country",            "4.8.4.20 ()"),
TlvUtil::TlvDef( TAG_DISPLAY_TIME                ,      1,      TLVUINT8,      "display_time",                    "4.8.4.31 (5.3.2.26)"),
TlvUtil::TlvDef( TAG_SMS_SIGNAL                  ,      2,      TLVUINT16,     "sms_signal",                      "4.8.4.53 (5.3.2.40)"),
TlvUtil::TlvDef( TAG_MS_VALIDITY                 ,      4,      TLVOCTET,      "ms_validity",                     "4.8.4.41 (5.3.2.27)"),
TlvUtil::TlvDef( TAG_ALERT_ON_MESSAGE_DELIVERY   ,      0,      TLVUNDEFINED,  "alert_on_message_delivery",       "4.8.4.2 (5.3.2.41)"),
TlvUtil::TlvDef( TAG_ITS_REPLY_TYPE              ,      1,      TLVUINT8,      "its_reply_type",                  "4.8.4.33 (5.3.2.42)"),
TlvUtil::TlvDef( TAG_ITS_SESSION_INFO            ,      2,      TLVOCTET,      "its_session_info",                "4.8.4.34 (5.3.2.43)"),
// SMSC_specific
TlvUtil::TlvDef( TAG_SUPPORTED_LOCALE            ,      31,     TLVUINT8,      "supported_locale",                ""),
TlvUtil::TlvDef( TAG_SUPPORTED_CODESET           ,      2,      TLVUINT8,      "supported_codeset",               ""),
TlvUtil::TlvDef( TAG_IMSI_ADDRESS                ,      31,     TLVUINT8,      "imsi_address",                    ""),
TlvUtil::TlvDef( TAG_MSC_ADDRESS                 ,      31,     TLVUINT8,      "msc_address",                     ""),
TlvUtil::TlvDef( TAG_PROTOCOL_ID                 ,      1,      TLVUINT8,      "protocol_id",                     ""),
TlvUtil::TlvDef( TAG_SCCP_OA                     ,      21,     TLVUINT8,      "sccp_oa",                         ""),
TlvUtil::TlvDef( TAG_SCCP_DA                     ,      21,     TLVUINT8,      "sccp_da",                         ""),
// SMPP+ specific
TlvUtil::TlvDef( TAG_USSD_SESSION_ID             ,      4,      TLVUINT8,      "ussd_session_id",                 ""),
TlvUtil::TlvDef( TAG_IMSI                        ,      15,     TLVUINT8,      "imsi",                            ""),
TlvUtil::TlvDef( TAG_HLR_ADDRESS_TON             ,      1,      TLVUINT8,      "hlr_address_ton",                 ""),
TlvUtil::TlvDef( TAG_HLR_ADDRESS_NPI             ,      1,      TLVUINT8,      "hlr_address_npi",                 ""),
TlvUtil::TlvDef( TAG_HLR_ADDRESS                 ,      21,     TLVUINT8,      "hlr_address",                     ""),
TlvUtil::TlvDef( TAG_VLR_NUMBER_TON              ,      1,      TLVUINT8,      "vlr_number_ton",                  ""),
TlvUtil::TlvDef( TAG_VLR_NUMBER_NPI              ,      1,      TLVUINT8,      "vlr_number_npi",                  ""),
TlvUtil::TlvDef( TAG_VLR_NUMBER                  ,      21,     TLVUINT8,      "vlr_number",                      ""),
// Additional, the last array element TlvDef[KNOWN_TLVS+1].
// Contains values for "not found". To avoid of search errors.
TlvUtil::TlvDef( TAG_UNKNOWN                     ,      0xffff, TLVUNDEFINED,  "unknown",                         "")
};

smsc::core::buffers::Hash<uint16_t> TlvUtil::tlv_defs_hash;

const uint16_t TlvUtil::tlvMaxLength(uint16_t tag) {
    const TlvDef* it = 
        std::lower_bound(&tlv_defs[0], &tlv_defs[KNOWN_TLVS],
                         tag, TlvDef::CompareTag());
    return it->maxlen_;
}

const char* TlvUtil::tlvName(uint16_t tag) {
    const TlvDef* it =
        std::lower_bound(&tlv_defs[0], &tlv_defs[KNOWN_TLVS],
                         tag, TlvDef::CompareTag());
    return it->name_;
}

const char* TlvUtil::tlvManRef(uint16_t tag) {
    const TlvDef* it =
        std::lower_bound(&tlv_defs[0], &tlv_defs[KNOWN_TLVS],
                         tag, TlvDef::CompareTag());
    return it->manref_;
}

const TlvDataType TlvUtil::tlvDataType(uint16_t tag) {
    const TlvDef* it = 
        std::lower_bound(&tlv_defs[0], &tlv_defs[KNOWN_TLVS],
                         tag, TlvDef::CompareTag());
    return it->type_;
}

const uint16_t TlvUtil::tlvTag(const char* name) {
    uint16_t result = 0;
    try {
        result = tlv_defs_hash.Get(name);
    }
    catch (...) {
        result = 0;
    }
    return result;
}

void TlvUtil::initHash() {
    tlv_defs_hash.Empty();
    const TlvDef* it = &tlv_defs[0];
    while ( it->tag_ > 0 ) {
        tlv_defs_hash[it->name_] = it->tag_;
        ++it;
    }
}

}
}

