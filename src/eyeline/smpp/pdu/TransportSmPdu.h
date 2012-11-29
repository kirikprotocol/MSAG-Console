#ifndef _EYELINE_SMPP_TRANSPORTSMPDU_H
#define _EYELINE_SMPP_TRANSPORTSMPDU_H

#include "BaseSmPdu.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace smpp {

class TransportSmPdu : public BaseSmPdu<ADDRLEN21>
{
public:
    TransportSmPdu( uint32_t thetype ) : BaseSmPdu<ADDRLEN21>(thetype),
    esm_class(0), protocol_id(0), priority_flag(0), registered_delivery(0),
    replace_if_present_flag(0), data_coding(0), sm_default_msg_id(0),
    sm_length(0) {}

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const;
    void decodeMandatoryFields( eyeline::informer::FromBuf& fb );

    const char* toString( char* buf, size_t bsz, size_t* written = 0 ) const;

protected:
    size_t mandatorySize() const
    {
        return BaseSmPdu<ADDRLEN21>::mandatorySize()
            + sizeof(esm_class) + sizeof(protocol_id) + sizeof(priority_flag)
                + schedule_delivery_time.length() + 1 + validity_period.length() + 1
            + sizeof(registered_delivery) + sizeof(replace_if_present_flag) + sizeof(data_coding)
                + sizeof(sm_default_msg_id) +
            sizeof(sm_length) + sm_length;
    }

protected:
    uint8_t esm_class;
    uint8_t protocol_id;
    uint8_t priority_flag;
    smsc::core::buffers::FixedLengthString<17> schedule_delivery_time;
    smsc::core::buffers::FixedLengthString<17> validity_period;
    uint8_t registered_delivery;
    uint8_t replace_if_present_flag;
    uint8_t data_coding;
    uint8_t sm_default_msg_id;
    uint8_t sm_length;
    smsc::core::buffers::FixedLengthString<256> short_message;  // 255 + trailing '\0'

public:    //mandatory fields access
    const uint8_t getSourceAddrTon() const; // { return source_addr.ton; }
    const uint8_t getSourceAddrNpi() const; // { return source_addr.npi; }
    const char* getSourceAddr() const; // { return source_addr.addr.c_str(); }
    const uint8_t getDestAddrTon() const; // { return dest_addr.ton; }
    const uint8_t getDestAddrNpi() const; // { return dest_addr.npi; }
    const char* getDestAddr() const; // { return dest_addr.addr.c_str(); }

    const uint8_t getEsmClass() const { return esm_class; }
    const uint8_t getProtocolId() const { return protocol_id; }
    const uint8_t getPriorityFlag() const { return priority_flag; }
    const char* getScheduleDeliveryTime() const { return schedule_delivery_time.c_str(); }
    const char* getValidityPeriod() const { return validity_period.c_str(); }
    const uint8_t getRegisteredDelivery() const { return registered_delivery; }
    const uint8_t getReplaceIfPresentFlag() const { return replace_if_present_flag; }
    const uint8_t getDataCoding() const { return data_coding; }
    const uint8_t getSmDefaultMsgId() const { return sm_default_msg_id; }
    const uint8_t getSmLength() const { return sm_length; }
    const char* getShortMessage() const { return short_message.str; }

    void setEsmClass(const uint8_t value) { esm_class = value; }
    void setProtocolId(const uint8_t value) { protocol_id = value; }
    void setPriorityFlag(const uint8_t value) { priority_flag = value; }
    void setScheduleDeliveryTime(const char* value) { schedule_delivery_time = value; }
    void setValidityPeriod(const char* value) { validity_period = value; }
    void setRegisteredDelivery(const uint8_t value) { registered_delivery = value; }
    void setReplaceIfPresentFlag(const uint8_t value) { replace_if_present_flag = value; }
    void setDataCoding(const uint8_t value) { data_coding = value; }
    void setSmDefaultMsgId(const uint8_t value) { sm_default_msg_id = value; }
    void setShortMessage(const char* value, uint8_t len ) {
        const size_t sz = sm_length = len;
        short_message.assign(value,sz);
    }

public: // optional fields access, SMPP5.0 ¤4.2.4, ¤4.3.3
    const uint8_t getCallbackNumPresInd() { return optionals.getInt(TAG_CALLBACK_NUM_PRES_IND); }
    const uint8_t getDestAddrNpResolution() { return optionals.getInt(TAG_DEST_ADDR_NP_RESOLUTION); }
    const uint8_t getDestAddrSubunit() { return optionals.getInt(TAG_DEST_ADDR_SUBUNIT); }
    const uint8_t getItsReplyType() { return optionals.getInt(TAG_ITS_REPLY_TYPE); }
    const uint8_t getLanguageIndicator() { return optionals.getInt(TAG_LANGUAGE_INDICATOR); }
    const uint8_t getPayloadType() { return optionals.getInt(TAG_PAYLOAD_TYPE); }
    const uint8_t getPrivacyIndicator() { return optionals.getInt(TAG_PRIVACY_INDICATOR); }
    const uint8_t getSarSegmentSeqnum() { return optionals.getInt(TAG_SAR_SEGMENT_SEQNUM); }
    const uint8_t getSarTotalSegments() { return optionals.getInt(TAG_SAR_TOTAL_SEGMENTS); }
    const uint8_t getSourceAddrSubunit() { return optionals.getInt(TAG_SOURCE_ADDR_SUBUNIT); }
    const uint8_t getUserResponseCode() { return optionals.getInt(TAG_USER_RESPONSE_CODE); }
    const uint8_t getUssdServiceOp() { return optionals.getInt(TAG_USSD_SERVICE_OP); }
    const uint16_t getSarMsgRefNum() { return optionals.getInt(TAG_SAR_MSG_REF_NUM); }
    const uint16_t getDestPort() { return optionals.getInt(TAG_DEST_PORT); }
    const uint16_t getSourcePort() { return optionals.getInt(TAG_SOURCE_PORT); }
    const uint16_t getUserMessageReference() { return optionals.getInt(TAG_USER_MESSAGE_REFERENCE); }

    const char* getDestNetworkId() { return optionals.getString(TAG_DEST_NETWORK_ID); }
    const char* getSourceNetworkId() { return optionals.getString(TAG_SOURCE_NETWORK_ID); }

    const void* getCallbackNum(size_t& len) { return optionals.getBinary(TAG_CALLBACK_NUM, len); }
    const void* getCallbackNumAtag(size_t& len) { return optionals.getBinary(TAG_CALLBACK_NUM_ATAG, len); }
    const void* getDestAddrNpCountry(size_t& len) { return optionals.getBinary(TAG_DEST_ADDR_NP_COUNTRY, len); }
    const void* getDestAddrNpInformation(size_t& len) { return optionals.getBinary(TAG_DEST_ADDR_NP_INFORMATION, len); }
    const void* getDestNodeId(size_t& len) { return optionals.getBinary(TAG_DEST_NODE_ID, len); }
    const void* getDestSubaddress(size_t& len) { return optionals.getBinary(TAG_DEST_SUBADDRESS, len); }
    const void* getItsSessionInfo(size_t& len) { return optionals.getBinary(TAG_ITS_SESSION_INFO, len); }
    const void* getMessagePayload(size_t& len) { return optionals.getBinary(TAG_MESSAGE_PAYLOAD, len); }
    const void* getSourceNodeId(size_t& len) { return optionals.getBinary(TAG_SOURCE_NODE_ID, len); }
    const void* getSourceSubaddress(size_t& len) { return optionals.getBinary(TAG_SOURCE_SUBADDRESS, len); }


    void setCallbackNumPresInd(const uint8_t value) { optionals.setInt(TAG_CALLBACK_NUM_PRES_IND, 1, uint32_t(value)); }
    void setDestAddrNpResolution(const uint8_t value) { optionals.setInt(TAG_DEST_ADDR_NP_RESOLUTION, 1, uint32_t(value)); }
    void setDestAddrSubunit(const uint8_t value) { optionals.setInt(TAG_DEST_ADDR_SUBUNIT, 1, uint32_t(value)); }
    void setItsReplyType(const uint8_t value) { optionals.setInt(TAG_ITS_REPLY_TYPE, 1, uint32_t(value)); }
    void setLanguageIndicator(const uint8_t value) { optionals.setInt(TAG_LANGUAGE_INDICATOR, 1, uint32_t(value)); }
    void setPayloadType(const uint8_t value) { optionals.setInt(TAG_PAYLOAD_TYPE, 1, uint32_t(value)); }
    void setPrivacyIndicator(const uint8_t value) { optionals.setInt(TAG_PRIVACY_INDICATOR, 1, uint32_t(value)); }
    void setSarSegmentSeqnum(const uint8_t value) { optionals.setInt(TAG_SAR_SEGMENT_SEQNUM, 1, uint32_t(value)); }
    void setSarTotalSegments(const uint8_t value) { optionals.setInt(TAG_SAR_TOTAL_SEGMENTS, 1, uint32_t(value)); }
    void setSourceAddrSubunit(const uint8_t value) { optionals.setInt(TAG_SOURCE_ADDR_SUBUNIT, 1, uint32_t(value)); }
    void setUserResponseCode(const uint8_t value) { optionals.setInt(TAG_USER_RESPONSE_CODE, 1, uint32_t(value)); }
    void setUssdServiceOp(const uint8_t value) { optionals.setInt(TAG_USSD_SERVICE_OP, 1, uint32_t(value)); }
    void setSarMsgRefNum(const uint16_t value) { optionals.setInt(TAG_SAR_MSG_REF_NUM, 2, uint32_t(value)); }
    void setDestPort(const uint16_t value) { optionals.setInt(TAG_DEST_PORT, 2, uint32_t(value)); }
    void setSourcePort(const uint16_t value) { optionals.setInt(TAG_SOURCE_PORT, 2, uint32_t(value)); }
    void setUserMessageReference(const uint16_t value) { optionals.setInt(TAG_USER_MESSAGE_REFERENCE, 2, uint32_t(value)); }

    void setDestNetworkId(const char* value) { optionals.setString(TAG_DEST_NETWORK_ID, value); }
    void setSourceNetworkId(const char* value) { optionals.setString(TAG_SOURCE_NETWORK_ID, value); }

    void setCallbackNum(size_t len, const void* value) { optionals.setBinary(TAG_CALLBACK_NUM, len, value); }
    void setCallbackNumAtag(size_t len, const void* value) { optionals.setBinary(TAG_CALLBACK_NUM_ATAG, len, value); }
    void setDestAddrNpCountry(size_t len, const void* value) { optionals.setBinary(TAG_DEST_ADDR_NP_COUNTRY, len, value); }
    void setDestAddrNpInformation(size_t len, const void* value) { optionals.setBinary(TAG_DEST_ADDR_NP_INFORMATION, len, value); }
    void setDestNodeId(size_t len, const void* value) { optionals.setBinary(TAG_DEST_NODE_ID, len, value); }
    void setDestSubaddress(size_t len, const void* value) { optionals.setBinary(TAG_DEST_SUBADDRESS, len, value); }
    void setItsSessionInfo(size_t len, const void* value) { optionals.setBinary(TAG_ITS_SESSION_INFO, len, value); }
    void setMessagePayload(size_t len, const void* value) { optionals.setBinary(TAG_MESSAGE_PAYLOAD, len, value); }
    void setSourceNodeId(size_t len, const void* value) { optionals.setBinary(TAG_SOURCE_NODE_ID, len, value); }
    void setSourceSubaddress(size_t len, const void* value) { optionals.setBinary(TAG_SOURCE_SUBADDRESS, len, value); }
};

}
}

#endif
