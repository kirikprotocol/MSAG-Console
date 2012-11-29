#ifndef _EYELINE_SMPP_DELIVERSMPDU_H
#define _EYELINE_SMPP_DELIVERSMPDU_H

#include "TransportSmPdu.h"
#include "SmResponsePdu.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace smpp {

class DeliverSmPdu : public TransportSmPdu
{
public:
    DeliverSmPdu() : TransportSmPdu(DELIVER_SM) {}

public: // optional fields access, SMPP5.0 ¤4.3.3
    const uint8_t getDpfResult() { return optionals.getInt(TAG_DPF_RESULT); }
    const uint8_t getMessageState() { return optionals.getInt(TAG_MESSAGE_STATE); }
    const void* getNetworkErrorCode(size_t& len) { return optionals.getBinary(TAG_NETWORK_ERROR_CODE, len); }
    const void* getReceiptedMessageId(size_t& len) { return optionals.getBinary(TAG_RECEIPTED_MESSAGE_ID, len); }

    void setDpfResult(const uint8_t value) { optionals.setInt(TAG_DPF_RESULT, TlvUtil::tlvMaxLength(TAG_DPF_RESULT), uint32_t(value)); }
    void setMessageState(const uint8_t value) { optionals.setInt(TAG_MESSAGE_STATE, TlvUtil::tlvMaxLength(TAG_MESSAGE_STATE), uint32_t(value)); }
    void setNetworkErrorCode(size_t len, const void* value) { optionals.setBinary(TAG_NETWORK_ERROR_CODE, len, value); }
    void setReceiptedMessageId(size_t len, const void* value) { optionals.setBinary(TAG_RECEIPTED_MESSAGE_ID, len, value); }
};

}
}

#endif
