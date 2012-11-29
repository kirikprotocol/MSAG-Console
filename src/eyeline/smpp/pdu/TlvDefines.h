#ifndef _EYELINE_SMPP_PDU_TLVDEFINES_H
#define _EYELINE_SMPP_PDU_TLVDEFINES_H

#include "util/int.h"
#include "core/buffers/Hash.hpp"

namespace eyeline {
namespace smpp {

enum TlvDataType {
    TLVUNDEFINED,
    TLVUINT8,
    TLVUINT16,
    TLVUINT32,
    TLVUINT64,
    TLVCSTRING,
    TLVOCTET
};

enum TlvTag {
    TAG_DEST_ADDR_SUBUNIT            = 0x0005,
    TAG_DEST_NETWORK_TYPE            = 0x0006,
    TAG_DEST_BEARER_TYPE             = 0x0007,
    TAG_DEST_TELEMATICS_ID           = 0x0008,
    TAG_SOURCE_ADDR_SUBUNIT          = 0x000d,
    TAG_SOURCE_NETWORK_TYPE          = 0x000e,
    TAG_SOURCE_BEARER_TYPE           = 0x000f,
    TAG_SOURCE_TELEMATICS_ID         = 0x0010,
    TAG_QOS_TIME_TO_LIVE             = 0x0017,
    TAG_PAYLOAD_TYPE                 = 0x0019,
    TAG_ADDITIONAL_STATUS_INFO_TEXT  = 0x001d,
    TAG_RECEIPTED_MESSAGE_ID         = 0x001e,
    TAG_MS_MSG_WAIT_FACILITIES       = 0x0030,
    TAG_PRIVACY_INDICATOR            = 0x0201,
    TAG_SOURCE_SUBADDRESS            = 0x0202,
    TAG_DEST_SUBADDRESS              = 0x0203,
    TAG_USER_MESSAGE_REFERENCE       = 0x0204,
    TAG_USER_RESPONSE_CODE           = 0x0205,
    TAG_SOURCE_PORT                  = 0x020a,
    TAG_DEST_PORT                    = 0x020b,
    TAG_SAR_MSG_REF_NUM              = 0x020c,
    TAG_LANGUAGE_INDICATOR           = 0x020d,
    TAG_SAR_TOTAL_SEGMENTS           = 0x020e,
    TAG_SAR_SEGMENT_SEQNUM           = 0x020f,
    TAG_SC_INTERFACE_VERSION         = 0x0210,
    TAG_CALLBACK_NUM_PRES_IND        = 0x0302,
    TAG_CALLBACK_NUM_ATAG            = 0x0303,
    TAG_NUMBER_OF_MESSAGES           = 0x0304,
    TAG_CALLBACK_NUM                 = 0x0381,
    TAG_DPF_RESULT                   = 0x0420,
    TAG_SET_DPF                      = 0x0421,
    TAG_MS_AVAILABILITY_STATUS       = 0x0422,
    TAG_NETWORK_ERROR_CODE           = 0x0423,
    TAG_MESSAGE_PAYLOAD              = 0x0424,
    TAG_DELIVERY_FAILURE_REASON      = 0x0425,
    TAG_MORE_MESSAGES_TO_SEND        = 0x0426,
    TAG_MESSAGE_STATE                = 0x0427,
    TAG_CONGESTION_STATE             = 0x0428,
    TAG_USSD_SERVICE_OP              = 0x0501,
    TAG_BROADCAST_CHANNEL_INDICATOR  = 0x0600,
    TAG_BROADCAST_CONTENT_TYPE       = 0x0601,
    TAG_BROADCAST_CONTENT_TYPE_INFO  = 0x0602,
    TAG_BROADCAST_MESSAGE_CLASS      = 0x0603,
    TAG_BROADCAST_REP_NUM            = 0x0604,
    TAG_BROADCAST_FREQUENCY_INTERVAL = 0x0605,
    TAG_BROADCAST_AREA_IDENTIFIER    = 0x0606,
    TAG_BROADCAST_ERROR_STATUS       = 0x0607,
    TAG_BROADCAST_AREA_SUCCESS       = 0x0608,
    TAG_BROADCAST_END_TIME           = 0x0609,
    TAG_BROADCAST_SERVICE_GROUP      = 0x060a,
    TAG_BILLING_IDENTIFICATION       = 0x060b,
    TAG_SOURCE_NETWORK_ID            = 0x060d,
    TAG_DEST_NETWORK_ID              = 0x060e,
    TAG_SOURCE_NODE_ID               = 0x060f,
    TAG_DEST_NODE_ID                 = 0x0610,
    TAG_DEST_ADDR_NP_RESOLUTION      = 0x0611,
    TAG_DEST_ADDR_NP_INFORMATION     = 0x0612,
    TAG_DEST_ADDR_NP_COUNTRY         = 0x0613,
    TAG_DISPLAY_TIME                 = 0x1201,
    TAG_SMS_SIGNAL                   = 0x1203,
    TAG_MS_VALIDITY                  = 0x1204,
    TAG_ALERT_ON_MESSAGE_DELIVERY    = 0x130c,
    TAG_ITS_REPLY_TYPE               = 0x1380,
    TAG_ITS_SESSION_INFO             = 0x1383,
    // SMSC_specific
    TAG_SUPPORTED_LOCALE             = 0x4100,
    TAG_SUPPORTED_CODESET            = 0x4101,
    TAG_IMSI_ADDRESS                 = 0x4102,
    TAG_MSC_ADDRESS                  = 0x4103,
    TAG_PROTOCOL_ID                  = 0x4104,
    TAG_SCCP_OA                      = 0x4105,
    TAG_SCCP_DA                      = 0x4106,
    // SMPP+ specific
    TAG_USSD_SESSION_ID              = 0x1501,
    TAG_IMSI                         = 0x1502,
    TAG_HLR_ADDRESS_TON              = 0x1800,
    TAG_HLR_ADDRESS_NPI              = 0x1801,
    TAG_HLR_ADDRESS                  = 0x1802,
    TAG_VLR_NUMBER_TON               = 0x1503,
    TAG_VLR_NUMBER_NPI               = 0x1504,
    TAG_VLR_NUMBER                   = 0x1505,
    // Additional, the last array element TlvDef[KNOWN_TLVS+1].
    // Contains values for "not found". To avoid of search errors.
    TAG_UNKNOWN                      = 0x0000
};


struct TlvUtil 
{
public:
    struct TlvDef {
        uint16_t tag_;
        uint16_t maxlen_;
        TlvDataType type_;
        const char* name_;
        const char* manref_;

        TlvDef(const uint16_t t, const uint16_t m, const TlvDataType y, const char* n, const char* r)
            : tag_(t), maxlen_(m), type_(y), name_(n), manref_(r) {}
        TlvDef(const TlvDef& t)
            : tag_(t.tag_), maxlen_(t.maxlen_), type_(t.type_), name_(t.name_), manref_(t.manref_) {}
        struct CompareTag
        {
            bool operator () (const TlvDef& left, const uint16_t tag) {
                return left.tag_ < tag;
            }
        };
    };

    static const unsigned KNOWN_TLVS = 79;
    static const TlvDef tlv_defs[KNOWN_TLVS+1];

    /// return max length the field may have
    static const uint16_t tlvMaxLength(uint16_t tag);

    static const char* tlvName(uint16_t tag);
    static const char* tlvManRef(uint16_t tag);
    static const TlvDataType tlvDataType(uint16_t tag);
    static const uint16_t tlvTag(const char* name);
    static void initHash();

protected:
    static smsc::core::buffers::Hash<uint16_t> tlv_defs_hash;
};

}
}
#endif
