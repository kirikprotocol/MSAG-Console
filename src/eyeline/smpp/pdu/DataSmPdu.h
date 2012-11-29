#ifndef _EYELINE_SMPP_DATASMPDU_H
#define _EYELINE_SMPP_DATASMPDU_H

#include "BaseSmPdu.h"
#include "Types.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace smpp {

class DataSmPdu : public BaseSmPdu<ADDRLEN65>   // RequestPdu
{
public:
    DataSmPdu() : BaseSmPdu<ADDRLEN65>(DATA_SM),
        esm_class(0), registered_delivery(0), data_coding(0) {}

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const {
        BaseSmPdu<ADDRLEN65>::encodeMandatoryFields(tb);

        tb.ncset8(esm_class);
        tb.ncset8(registered_delivery);
        tb.ncset8(data_coding);
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) {
        BaseSmPdu<ADDRLEN65>::decodeMandatoryFields(fb);

        esm_class = fb.get8();
        registered_delivery = fb.get8();
        data_coding = fb.get8();
    }

    const char* toString( char* buf, size_t bsz, size_t* written = 0) const
    {
        size_t wr;
        BaseSmPdu<ADDRLEN65>::toString(buf,bsz,&wr);
        bsz -= wr;
        if (bsz > 0) {
            int res = snprintf(buf+wr,bsz,",esm=%u,regdlv=%u,dc=%u",
                               esm_class,
                               registered_delivery,
                               data_coding);
            if (res<=0) {
                buf[wr] = '\0';
                res = 0;
            } else if ( size_t(res) >= bsz ) {
                res = int(bsz-1);
            }
            wr += res;
        }
        if (written) {
            *written = wr;
        }
        return buf;
    }

protected:
    size_t mandatorySize() const {
        return BaseSmPdu<ADDRLEN65>::mandatorySize()
            + sizeof(esm_class) + sizeof(registered_delivery) + sizeof(data_coding);
    }

protected:
    uint8_t esm_class;
    uint8_t registered_delivery;
    uint8_t data_coding;

public: // mandatory fields access

    inline const uint8_t getSourceAddrTon() const; // { return source_addr.ton; }
    inline const uint8_t getSourceAddrNpi() const; // { return source_addr.npi; }
    inline const char* getSourceAddr() const; // { return source_addr.addr.c_str(); }
    inline const uint8_t getDestAddrTon() const; // { return dest_addr.ton; }
    inline const uint8_t getDestAddrNpi() const; // { return dest_addr.npi; }
    inline const char* getDestAddr() const; // { return dest_addr.addr.c_str(); }

    inline const uint8_t getEsmClass() const { return esm_class; }
    inline const uint8_t getRegisteredDelivery() const { return registered_delivery; }
    inline const uint8_t getDataCoding() const { return data_coding; }


    inline void setEsmClass(const uint8_t value) { esm_class = value; }
    inline void setRegisteredDelivery(const uint8_t value) { registered_delivery = value; }
    inline void setDataCoding(const uint8_t value) { data_coding = value; }

public: // optional fields access, SMPP5.0 ¤4.2.4
    inline const bool getAlertOnMsgDelivery() { return optionals.hasTag(TAG_ALERT_ON_MESSAGE_DELIVERY); }

    inline const uint8_t getCallbackNumPresInd() { return optionals.getInt(TAG_CALLBACK_NUM_PRES_IND); }
    inline const uint8_t getDestAddrNpResolution() { return optionals.getInt(TAG_DEST_ADDR_NP_RESOLUTION); }
    inline const uint8_t getDestAddrSubunit() { return optionals.getInt(TAG_DEST_ADDR_SUBUNIT); }
    inline const uint8_t getDestBearerType() { return optionals.getInt(TAG_DEST_BEARER_TYPE); }
    inline const uint8_t getDestNetworkType() { return optionals.getInt(TAG_DEST_NETWORK_TYPE); }
    inline const uint8_t getDisplayTime() { return optionals.getInt(TAG_DISPLAY_TIME); }
    inline const uint8_t getItsReplyType() { return optionals.getInt(TAG_ITS_REPLY_TYPE); }
    inline const uint8_t getLanguageIndicator() { return optionals.getInt(TAG_LANGUAGE_INDICATOR); }
    inline const uint8_t getMoreMessagesToSend() { return optionals.getInt(TAG_MORE_MESSAGES_TO_SEND); }
    inline const uint8_t getMsMsgWaitFacilities() { return optionals.getInt(TAG_MS_MSG_WAIT_FACILITIES); }
    inline const uint8_t getNumberOfMessages() { return optionals.getInt(TAG_NUMBER_OF_MESSAGES); }
    inline const uint8_t getPayloadType() { return optionals.getInt(TAG_PAYLOAD_TYPE); }
    inline const uint8_t getPrivacyIndicator() { return optionals.getInt(TAG_PRIVACY_INDICATOR); }
    inline const uint8_t getSarSegmentSeqnum() { return optionals.getInt(TAG_SAR_SEGMENT_SEQNUM); }
    inline const uint8_t getSarTotalSegments() { return optionals.getInt(TAG_SAR_TOTAL_SEGMENTS); }
    inline const uint8_t getSetDpf() { return optionals.getInt(TAG_SET_DPF); }
    inline const uint8_t getSourceAddrSubunit() { return optionals.getInt(TAG_SOURCE_ADDR_SUBUNIT); }
    inline const uint8_t getSourceBearerType() { return optionals.getInt(TAG_SOURCE_BEARER_TYPE); }
    inline const uint8_t getSourceNetworkType() { return optionals.getInt(TAG_SOURCE_NETWORK_TYPE); }
    inline const uint8_t getSourceTelematicsId() { return optionals.getInt(TAG_SOURCE_TELEMATICS_ID); }
    inline const uint8_t getUserResponseCode() { return optionals.getInt(TAG_USER_RESPONSE_CODE); }
    inline const uint8_t getUssdServiceOp() { return optionals.getInt(TAG_USSD_SERVICE_OP); }
    inline const uint16_t getSarMsgRefNum() { return optionals.getInt(TAG_SAR_MSG_REF_NUM); }
    inline const uint16_t getSmsSignal() { return optionals.getInt(TAG_SMS_SIGNAL); }
    inline const uint16_t getDestTelematicsId() { return optionals.getInt(TAG_DEST_TELEMATICS_ID); }
    inline const uint16_t getDestPort() { return optionals.getInt(TAG_DEST_PORT); }
    inline const uint16_t getSourcePort() { return optionals.getInt(TAG_SOURCE_PORT); }
    inline const uint16_t getUserMessageReference() { return optionals.getInt(TAG_USER_MESSAGE_REFERENCE); }
    inline const uint32_t getQosTimeToLive() { return optionals.getInt(TAG_QOS_TIME_TO_LIVE); }

    inline const char* getDestNetworkId() { return optionals.getString(TAG_DEST_NETWORK_ID); }
    inline const char* getSourceNetworkId() { return optionals.getString(TAG_SOURCE_NETWORK_ID); }

    inline const void* getBillingIdentification(size_t& len) { return optionals.getBinary(TAG_BILLING_IDENTIFICATION, len); }
    inline const void* getCallbackNum(size_t& len) { return optionals.getBinary(TAG_CALLBACK_NUM, len); }
    inline const void* getCallbackNumAtag(size_t& len) { return optionals.getBinary(TAG_CALLBACK_NUM_ATAG, len); }
    inline const void* getDestAddrNpCountry(size_t& len) { return optionals.getBinary(TAG_DEST_ADDR_NP_COUNTRY, len); }
    inline const void* getDestAddrNpInformation(size_t& len) { return optionals.getBinary(TAG_DEST_ADDR_NP_INFORMATION, len); }
    inline const void* getDestNodeId(size_t& len) { return optionals.getBinary(TAG_DEST_NODE_ID, len); }
    inline const void* getDestSubaddress(size_t& len) { return optionals.getBinary(TAG_DEST_SUBADDRESS, len); }
    inline const void* getItsSessionInfo(size_t& len) { return optionals.getBinary(TAG_ITS_SESSION_INFO, len); }
    inline const void* getMessagePayload(size_t& len) { return optionals.getBinary(TAG_MESSAGE_PAYLOAD, len); }
    inline const void* getMsValidity(size_t& len) { return optionals.getBinary(TAG_MS_VALIDITY, len); }
    inline const void* getSourceNodeId(size_t& len) { return optionals.getBinary(TAG_SOURCE_NODE_ID, len); }
    inline const void* getSourceSubaddress(size_t& len) { return optionals.getBinary(TAG_SOURCE_SUBADDRESS, len); }

    inline void setAlertOnMsgDelivery() { optionals.setInt(TAG_ALERT_ON_MESSAGE_DELIVERY, 0, 0); }

    inline void setCallbackNumPresInd(const uint8_t value) { optionals.setInt(TAG_CALLBACK_NUM_PRES_IND, 1, uint32_t(value)); }
    inline void setDestAddrNpResolution(const uint8_t value) { optionals.setInt(TAG_DEST_ADDR_NP_RESOLUTION, 1, uint32_t(value)); }
    inline void setDestAddrSubunit(const uint8_t value) { optionals.setInt(TAG_DEST_ADDR_SUBUNIT, 1, uint32_t(value)); }
    inline void setDestBearerType(const uint8_t value) { optionals.setInt(TAG_DEST_BEARER_TYPE, 1, uint32_t(value)); }
    inline void setDestNetworkType(const uint8_t value) { optionals.setInt(TAG_DEST_NETWORK_TYPE, 1, uint32_t(value)); }
    inline void setDisplayTime(const uint8_t value) { optionals.setInt(TAG_DISPLAY_TIME, 1, uint32_t(value)); }
    inline void setItsReplyType(const uint8_t value) { optionals.setInt(TAG_ITS_REPLY_TYPE, 1, uint32_t(value)); }
    inline void setLanguageIndicator(const uint8_t value) { optionals.setInt(TAG_LANGUAGE_INDICATOR, 1, uint32_t(value)); }
    inline void setMoreMessagesToSend(const uint8_t value) { optionals.setInt(TAG_MORE_MESSAGES_TO_SEND, 1, uint32_t(value)); }
    inline void setMsMsgWaitFacilities(const uint8_t value) { optionals.setInt(TAG_MS_MSG_WAIT_FACILITIES, 1, uint32_t(value)); }
    inline void setNumberOfMessages(const uint8_t value) { optionals.setInt(TAG_NUMBER_OF_MESSAGES, 1, uint32_t(value)); }
    inline void setPayloadType(const uint8_t value) { optionals.setInt(TAG_PAYLOAD_TYPE, 1, uint32_t(value)); }
    inline void setPrivacyIndicator(const uint8_t value) { optionals.setInt(TAG_PRIVACY_INDICATOR, 1, uint32_t(value)); }
    inline void setSarSegmentSeqnum(const uint8_t value) { optionals.setInt(TAG_SAR_SEGMENT_SEQNUM, 1, uint32_t(value)); }
    inline void setSarTotalSegments(const uint8_t value) { optionals.setInt(TAG_SAR_TOTAL_SEGMENTS, 1, uint32_t(value)); }
    inline void setSetDpf(const uint8_t value) { optionals.setInt(TAG_SET_DPF, 1, uint32_t(value)); }
    inline void setSourceAddrSubunit(const uint8_t value) { optionals.setInt(TAG_SOURCE_ADDR_SUBUNIT, 1, uint32_t(value)); }
    inline void setSourceBearerType(const uint8_t value) { optionals.setInt(TAG_SOURCE_BEARER_TYPE, 1, uint32_t(value)); }
    inline void setSourceNetworkType(const uint8_t value) { optionals.setInt(TAG_SOURCE_NETWORK_TYPE, 1, uint32_t(value)); }
    inline void setSourceTelematicsId(const uint8_t value) { optionals.setInt(TAG_SOURCE_TELEMATICS_ID, 1, uint32_t(value)); }
    inline void setUserResponseCode(const uint8_t value) { optionals.setInt(TAG_USER_RESPONSE_CODE, 1, uint32_t(value)); }
    inline void setUssdServiceOp(const uint8_t value) { optionals.setInt(TAG_USSD_SERVICE_OP, 1, uint32_t(value)); }
    inline void setSarMsgRefNum(const uint16_t value) { optionals.setInt(TAG_SAR_MSG_REF_NUM, 1, uint32_t(value)); }
    inline void setSmsSignal(const uint16_t value) { optionals.setInt(TAG_SMS_SIGNAL, 1, uint32_t(value)); }
    inline void setDestTelematicsId(const uint16_t value) { optionals.setInt(TAG_DEST_TELEMATICS_ID, 1, uint32_t(value)); }
    inline void setDestPort(const uint16_t value) { optionals.setInt(TAG_DEST_PORT, 1, uint32_t(value)); }
    inline void setSourcePort(const uint16_t value) { optionals.setInt(TAG_SOURCE_PORT, 1, uint32_t(value)); }
    inline void setUserMessageReference(const uint16_t value) { optionals.setInt(TAG_USER_MESSAGE_REFERENCE, 1, uint32_t(value)); }
    inline void setQosTimeToLive(const uint32_t value) { optionals.setInt(TAG_QOS_TIME_TO_LIVE, 1, value); }

    inline void setDestNetworkId(const char* value) { optionals.setString(TAG_DEST_NETWORK_ID, value); }
    inline void setSourceNetworkId(const char* value) { optionals.setString(TAG_SOURCE_NETWORK_ID, value); }

    inline void setBillingIdentification(size_t len, const void* value) { optionals.setBinary(TAG_BILLING_IDENTIFICATION, len, value); }
    inline void setCallbackNum(size_t len, const void* value) { optionals.setBinary(TAG_CALLBACK_NUM, len, value); }
    inline void setCallbackNumAtag(size_t len, const void* value) { optionals.setBinary(TAG_CALLBACK_NUM_ATAG, len, value); }
    inline void setDestAddrNpCountry(size_t len, const void* value) { optionals.setBinary(TAG_DEST_ADDR_NP_COUNTRY, len, value); }
    inline void setDestAddrNpInformation(size_t len, const void* value) { optionals.setBinary(TAG_DEST_ADDR_NP_INFORMATION, len, value); }
    inline void setDestNodeId(size_t len, const void* value) { optionals.setBinary(TAG_DEST_NODE_ID, len, value); }
    inline void setDestSubaddress(size_t len, const void* value) { optionals.setBinary(TAG_DEST_SUBADDRESS, len, value); }
    inline void setItsSessionInfo(size_t len, const void* value) { optionals.setBinary(TAG_ITS_SESSION_INFO, len, value); }
    inline void setMessagePayload(size_t len, const void* value) { optionals.setBinary(TAG_MESSAGE_PAYLOAD, len, value); }
    inline void setMsValidity(size_t len, const void* value) { optionals.setBinary(TAG_MS_VALIDITY, len, value); }
    inline void setSourceNodeId(size_t len, const void* value) { optionals.setBinary(TAG_SOURCE_NODE_ID, len, value); }
    inline void setSourceSubaddress(size_t len, const void* value) { optionals.setBinary(TAG_SOURCE_SUBADDRESS, len, value); }
};

}
}

#endif
