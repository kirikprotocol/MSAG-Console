#ifndef _EYELINE_SMPP_SMRESPONSEPDU_H
#define _EYELINE_SMPP_SMRESPONSEPDU_H

#include "ResponsePdu.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace smpp {


class SmRespPdu : public ResponsePdu
{
public:
    SmRespPdu(  PduType pt, uint32_t status, uint32_t seqNum ) : ResponsePdu(status,seqNum) {
        pduTypeId_ = pt;
    }

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const 
    {
        tb.setCString(message_id.c_str());
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) 
    {
        message_id = fb.getCString();
    }

protected:
    size_t mandatorySize() const { return message_id.length() + 1; }

protected:
    smsc::core::buffers::FixedLengthString<65> message_id;

public: // mandatory fields access
    inline const char* getMessageId() const { return message_id.c_str(); }
    inline void setMessageId(const char* value) { message_id = value; }

public: // tlv access, SMPP5.0 ¤4.2.5, ¤4.3.4
    const char* getAdditionalStatusInfoText() { return optionals.getString(TAG_ADDITIONAL_STATUS_INFO_TEXT); }
    const uint8_t getDeliveryFailureReason() { return optionals.getInt(TAG_DELIVERY_FAILURE_REASON); }
    const uint8_t getDpfResult() { return optionals.getInt(TAG_DPF_RESULT); }
    const void* getNetworkErrorCode(size_t& len) { return optionals.getBinary(TAG_NETWORK_ERROR_CODE, len); }

    void setAdditionalStatusInfoText(const char* value) { optionals.setString(TAG_ADDITIONAL_STATUS_INFO_TEXT, value); }
    void setDeliveryFailureReason(const uint8_t value) { optionals.setInt(TAG_DELIVERY_FAILURE_REASON, 1, uint32_t(value)); }
    void setDpfResult(const uint8_t value) { optionals.setInt(TAG_DPF_RESULT, 1, uint32_t(value)); }
    void setNetworkErrorCode(size_t len, const void* value) { optionals.setBinary(TAG_NETWORK_ERROR_CODE, len, value); }
};


}
}

#endif
