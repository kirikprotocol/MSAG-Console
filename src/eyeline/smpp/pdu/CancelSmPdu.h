#ifndef _EYELINE_SMPP_CANCELSMPDU_H
#define _EYELINE_SMPP_CANCELSMPDU_H

#include "RequestPdu.h"
#include "ResponsePdu.h"
#include "PduAddr.h"


namespace eyeline {
namespace smpp {

class CancelSmRespPdu : public ResponsePdu
{
public:
    CancelSmRespPdu( uint32_t status, uint32_t seqNum ) : ResponsePdu(status,seqNum) {
        pduTypeId_ = CANCEL_SM_RESP;
    }
};


class CancelSmPdu : public RequestPdu
{
protected: //mandatory fields
    smsc::core::buffers::FixedLengthString<6> service_type;
    smsc::core::buffers::FixedLengthString<65> message_id;
    addr_params_t<ADDRLEN21> source_addr;
    addr_params_t<ADDRLEN21> dest_addr;

public:
    CancelSmPdu() : RequestPdu(CANCEL_SM) {}

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const 
    {
        tb.setCString(service_type.c_str());
        tb.setCString(message_id.c_str());
        source_addr.encode(tb);
        dest_addr.encode(tb);
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) 
    {
        service_type = fb.getCString();
        message_id = fb.getCString();
        source_addr.decode(fb);
        dest_addr.decode(fb);
    }

    inline const char* getServiceType() const { return service_type.c_str(); }
    inline const char* getMessageId() const { return message_id.c_str(); }
    inline const uint8_t getSourceAddrTon() const { return source_addr.ton; }
    inline const uint8_t getSourceAddrNpi() const { return source_addr.npi; }
    inline const char* getSourceAddr() const { return source_addr.addr.c_str(); }
    inline const uint8_t getDestAddrTon() const { return dest_addr.ton; }
    inline const uint8_t getDestAddrNpi() const { return dest_addr.npi; }
    inline const char* getDestAddr() const { return dest_addr.addr.c_str(); }

    inline void setServiceType(const char* value) { service_type = value; }
    inline void setMessageId(const char* value) { message_id = value; }
    inline void setSourceAddrTon(const uint8_t value) { source_addr.ton = value; }
    inline void setSourceAddrNpi(const uint8_t value) { source_addr.npi = value; }
    inline void setSourceAddr(const char* value) { source_addr.addr = value; }
    inline void setDestAddrTon(const uint8_t value) { dest_addr.ton = value; }
    inline void setDestAddrNpi(const uint8_t value) { dest_addr.npi = value; }
    inline void setDestAddr(const char* value) { dest_addr.addr = value; }

protected:
    size_t mandatorySize() const 
    {
        return service_type.length() + 1 + message_id.length() + 1
            + source_addr.size() + dest_addr.size();
    }

};

}
}

#endif
