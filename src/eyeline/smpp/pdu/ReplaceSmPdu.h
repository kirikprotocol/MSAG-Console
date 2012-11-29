#ifndef _EYELINE_SMPP_REPLACESMPDU_H
#define _EYELINE_SMPP_REPLACESMPDU_H

#include "RequestPdu.h"
#include "ResponsePdu.h"
#include "PduAddr.h"

namespace eyeline {
namespace smpp {

class ReplaceSmRespPdu : public ResponsePdu
{
public:
    ReplaceSmRespPdu( uint32_t status, uint32_t seqNum ) : ResponsePdu(status,seqNum) {
        pduTypeId_ = SUBMIT_SM_RESP;
    }
};


class ReplaceSmPdu : public RequestPdu
{
protected: // mandatory fields
    smsc::core::buffers::FixedLengthString<65> message_id;
    addr_params_t<ADDRLEN21> source_addr;
    smsc::core::buffers::FixedLengthString<17> schedule_delivery_time;
    smsc::core::buffers::FixedLengthString<17> validity_period;
    uint8_t registered_delivery;
    uint8_t sm_default_msg_id;
    uint8_t sm_length;
    smsc::core::buffers::FixedLengthString<256> short_message;  // 255 + trailing '\0'

public:
    ReplaceSmPdu() : RequestPdu(REPLACE_SM) {}

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const 
    {
        tb.setCString(message_id.c_str());
        source_addr.encode(tb);
        tb.setCString(schedule_delivery_time.c_str());
        tb.setCString(validity_period.c_str());
        tb.ncset8(registered_delivery);
        tb.ncset8(sm_default_msg_id);
        tb.ncset8(sm_length);
        tb.copy(size_t(sm_length),short_message.str);
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) 
    {
        message_id = fb.getCString();
        source_addr.decode(fb);
        schedule_delivery_time = fb.getCString();
        validity_period = fb.getCString();
        registered_delivery = fb.get8();
        sm_default_msg_id = fb.get8();
        const size_t sz = sm_length = fb.get8();
        short_message.assign(fb.skip(sz),sz);
    }

protected:
    size_t mandatorySize() const {
        return message_id.length() + 1
            + source_addr.size()
            + schedule_delivery_time.length() + 1 + validity_period.length() + 1
            + sizeof(registered_delivery) + sizeof(sm_default_msg_id) +
            sizeof(sm_length) + sm_length;
    }

public: // mandatory fields access
    inline const char* getMessageId() const { return message_id.c_str(); }
    inline const uint8_t getSourceAddrTon() const { return source_addr.ton; }
    inline const uint8_t getSourceAddrNpi() const { return source_addr.npi; }
    inline const char* getSourceAddr() const { return source_addr.addr.c_str(); }
    inline const char* getScheduleDeliveryTime() const { return schedule_delivery_time.c_str(); }
    inline const char* getValidityPeriod() const { return validity_period.c_str(); }
    inline const uint8_t getRegisteredDelivery() const { return registered_delivery; }
    inline const uint8_t getSmDefaultMsgId() const { return sm_default_msg_id; }
    inline const uint8_t getSmLength() const { return sm_length; }
    inline const char* getShortMessage() const { return short_message.str; }

    inline void setMessageId(const char* value) { message_id = value; }
    inline void setSourceAddrTon(const uint8_t value) { source_addr.ton = value; }
    inline void setSourceAddrNpi(const uint8_t value) { source_addr.npi = value; }
    inline void setSourceAddr(const char* value) { source_addr.addr = value; }
    inline void setScheduleDeliveryTime(const char* value) { schedule_delivery_time = value; }
    inline void setValidityPeriod(const char* value) { validity_period = value; }
    inline void setRegisteredDelivery(const uint8_t value) { registered_delivery = value; }
    inline void setSmDefaultMsgId(const uint8_t value) { sm_default_msg_id = value; }
    inline void setShortMessage(const char* value,
                                uint8_t len ) {
        const size_t sz = sm_length = len;
        short_message.assign(value,sz);
    }

public: // optional fields access, SMPP5.0 ¤4.5.3.3
    const void* getMessagePayload(size_t& len) { return optionals.getBinary(TAG_MESSAGE_PAYLOAD, len); }

    void setMessagePayload(size_t len, const void* value) { optionals.setBinary(TAG_MESSAGE_PAYLOAD, len, value); }
};

}
}

#endif
