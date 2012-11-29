#ifndef _EYELINE_SMPP_QUERYSMPDU_H
#define _EYELINE_SMPP_QUERYSMPDU_H

#include "RequestPdu.h"
#include "ResponsePdu.h"
#include "PduAddr.h"
#include "informer/io/IOConverter.h"

namespace eyeline {
namespace smpp {

class QuerySmRespPdu : public ResponsePdu
{
public:
    QuerySmRespPdu( uint32_t status, uint32_t seqNum ) : ResponsePdu(status,seqNum) {
        pduTypeId_ = QUERY_SM_RESP;
    }

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const 
    {
        tb.setCString(message_id.c_str());
        tb.setCString(final_date.c_str());
        tb.ncset8(message_state);
        tb.ncset8(error_code);
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) 
    {
        message_id = fb.getCString();
        final_date = fb.getCString();
        message_state = fb.get8();
        error_code = fb.get8();
    }

protected:
    size_t mandatorySize() const 
    {
        return message_id.length() + 1 + final_date.length() + 1 + sizeof(message_state) + sizeof(error_code);
    }

protected:
    smsc::core::buffers::FixedLengthString<65> message_id;
    smsc::core::buffers::FixedLengthString<17> final_date;
    uint8_t message_state;
    uint8_t error_code;

public: // mandatory fields access
    inline const char* getMessageId() const { return message_id.c_str(); }
    inline const char* getFinalDate() const { return final_date.c_str(); }
    inline const uint8_t getMessageState() const { return message_state; }
    inline const uint8_t getErrorCode() const { return error_code; }

    inline void setMessageId(const char* value) { message_id = value; }
    inline void setFinalDate(const char* value) { final_date = value; }
    inline void setMessageState(const uint8_t value) { message_state = value; }
    inline void setErrorCode(const uint8_t value) { error_code = value; }
};


class QuerySmPdu : public RequestPdu
{
protected: //mandatory fields
    smsc::core::buffers::FixedLengthString<65> message_id;
    addr_params_t<ADDRLEN21> source_addr;

public:
    QuerySmPdu() : RequestPdu(QUERY_SM) {}

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const 
    {
        tb.setCString(message_id.c_str());
        source_addr.encode(tb);
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) 
    {
        message_id = fb.getCString();
        source_addr.decode(fb);
    }

protected:
    size_t mandatorySize() const 
    {
        return message_id.length() + 1 + source_addr.size();
    }

public: // mandatory fields access
    inline const char* getMessageId() const { return message_id.c_str(); }
    inline const uint8_t getSourceAddrTon() const { return source_addr.ton; }
    inline const uint8_t getSourceAddrNpi() const { return source_addr.npi; }
    inline const char* getSourceAddr() const { return source_addr.addr.c_str(); }

    void setMessageId(const char* value) { message_id = value; }
    inline void setSourceAddr(uint8_t ton, uint8_t npi, const char* value)
    {
        source_addr.ton = ton;
        source_addr.npi = npi;
        source_addr.addr = value;
    }

    void setSourceAddr(const char* value)
    {
        source_addr.setAddr(value);
    }
};

}
}

#endif
