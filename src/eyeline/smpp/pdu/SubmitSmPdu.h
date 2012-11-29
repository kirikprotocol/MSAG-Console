#ifndef _EYELINE_SMPP_SUBMITSMPDU_H
#define _EYELINE_SMPP_SUBMITSMPDU_H

#include "TransportSmPdu.h"
#include "SmResponsePdu.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace smpp {

class SubmitSmPdu : public TransportSmPdu
{
public:
    SubmitSmPdu() : TransportSmPdu(SUBMIT_SM) {}

public: // optional fields access, SMPP5.0 ¤4.2.4
    const bool getAlertOnMsgDelivery() { return optionals.hasTag(TAG_ALERT_ON_MESSAGE_DELIVERY); }
    const uint8_t getDestBearerType() { return optionals.getInt(TAG_DEST_BEARER_TYPE); }
    const uint8_t getDestNetworkType() { return optionals.getInt(TAG_DEST_NETWORK_TYPE); }
    const uint8_t getDisplayTime() { return optionals.getInt(TAG_DISPLAY_TIME); }
    const uint8_t getMoreMessagesToSend() { return optionals.getInt(TAG_MORE_MESSAGES_TO_SEND); }
    const uint8_t getMsMsgWaitFacilities() { return optionals.getInt(TAG_MS_MSG_WAIT_FACILITIES); }
    const uint8_t getNumberOfMessages() { return optionals.getInt(TAG_NUMBER_OF_MESSAGES); }
    const uint8_t getSetDpf() { return optionals.getInt(TAG_SET_DPF); }
    const uint8_t getSourceBearerType() { return optionals.getInt(TAG_SOURCE_BEARER_TYPE); }
    const uint8_t getSourceNetworkType() { return optionals.getInt(TAG_SOURCE_NETWORK_TYPE); }
    const uint8_t getSourceTelematicsId() { return optionals.getInt(TAG_SOURCE_TELEMATICS_ID); }
    const uint16_t getSmsSignal() { return optionals.getInt(TAG_SMS_SIGNAL); }
    const uint16_t getDestTelematicsId() { return optionals.getInt(TAG_DEST_TELEMATICS_ID); }
    const uint32_t getQosTimeToLive() { return optionals.getInt(TAG_QOS_TIME_TO_LIVE); }
    const void* getBillingIdentification(size_t& len) { return optionals.getBinary(TAG_BILLING_IDENTIFICATION, len); }
    const void* getMsValidity(size_t& len) { return optionals.getBinary(TAG_MS_VALIDITY, len); }

    void setAlertOnMsgDelivery() { return optionals.setInt(TAG_ALERT_ON_MESSAGE_DELIVERY, 0, 0); }
    void setDestBearerType(const uint8_t value) { return optionals.setInt(TAG_DEST_BEARER_TYPE, 1, uint32_t(value)); }
    void setDestNetworkType(const uint8_t value) { return optionals.setInt(TAG_DEST_NETWORK_TYPE, 1, uint32_t(value)); }
    void setDisplayTime(const uint8_t value) { return optionals.setInt(TAG_DISPLAY_TIME, 1, uint32_t(value)); }
    void setMoreMessagesToSend(const uint8_t value) { return optionals.setInt(TAG_MORE_MESSAGES_TO_SEND, 1, uint32_t(value)); }
    void setMsMsgWaitFacilities(const uint8_t value) { return optionals.setInt(TAG_MS_MSG_WAIT_FACILITIES, 1, uint32_t(value)); }
    void setNumberOfMessages(const uint8_t value) { return optionals.setInt(TAG_NUMBER_OF_MESSAGES, 1, uint32_t(value)); }
    void setSetDpf(const uint8_t value) { return optionals.setInt(TAG_SET_DPF, 1, uint32_t(value)); }
    void setSourceBearerType(const uint8_t value) { return optionals.setInt(TAG_SOURCE_BEARER_TYPE, 1, uint32_t(value)); }
    void setSourceNetworkType(const uint8_t value) { return optionals.setInt(TAG_SOURCE_NETWORK_TYPE, 1, uint32_t(value)); }
    void setSourceTelematicsId(const uint8_t value) { return optionals.setInt(TAG_SOURCE_TELEMATICS_ID, 1, uint32_t(value)); }
    void setSmsSignal(const uint16_t value) { return optionals.setInt(TAG_SMS_SIGNAL, 2, uint32_t(value)); }
    void setDestTelematicsId(const uint16_t value) { return optionals.setInt(TAG_DEST_TELEMATICS_ID, 2, uint32_t(value)); }
    void setQosTimeToLive(const uint32_t value) { return optionals.setInt(TAG_QOS_TIME_TO_LIVE, 4, value); }

    void setBillingIdentification(size_t len, const void* value) { return optionals.setBinary(TAG_BILLING_IDENTIFICATION, len, value); }
    void setMsValidity(size_t len, const void* value) { return optionals.setBinary(TAG_MS_VALIDITY, len, value); }
};

}
}

#endif
