#ifndef _EYELINE_SMPP_PDU_PDUINPUTBUF_H
#define _EYELINE_SMPP_PDU_PDUINPUTBUF_H

#include "Types.h"

namespace eyeline {
namespace smpp {

class Pdu;
class ResponsePdu;

/// this is an input buffer which can be used to decode pdu
class PduInputBuf
{
    friend class PduBuffer;
public:

    /// may throw an exception
    PduInputBuf( const size_t streamlen, const char* stream);

    // decode a pdu
    Pdu* decode() const;

    bool isRequest() const { return !(pduTypeId_ & 0x80000000); }
    uint32_t getPduTypeId() const { return pduTypeId_; }
    uint32_t getSeqNum() const { return seqNum_; }
    uint32_t getStatus() const { return status_; }

    /// create a response based on the original pdu
    ResponsePdu* createResponse( uint32_t status = Status::OK ) const
    {
        return createResponse( PduType(pduTypeId_),
                               seqNum_,
                               status );
    }

    static ResponsePdu* createResponse(PduType  pduType,
                                       uint32_t seqNum,
                                       uint32_t status = Status::OK );

private:
    Pdu* createPdu() const;

private:
    PduInputBuf();
    PduInputBuf( const PduInputBuf& );
    PduInputBuf& operator = ( const PduInputBuf& );

private:
    size_t      size_;      // size of the buffer
    uint32_t    pduTypeId_;
    uint32_t    status_;
    uint32_t    seqNum_;
    const char* buf_;       // the buffer, not owned!
};

}
}

#endif
