#ifndef _SCAG_PVSS_BASE_RESPONSE_H
#define _SCAG_PVSS_BASE_RESPONSE_H

#include "Packet.h"
#include "Request.h"
#include "StatusType.h"

namespace scag2 {
namespace pvss {

class ResponseVisitor;

///
/// Abstract class Request
///
class Response : public Packet, public StatusType
{
protected:
    Response() : seqNum_(uint32_t(-1)) {}
    Response( uint32_t seqNum ) : seqNum_(seqNum) {}
    // Response( const Response& other ); // default is ok

public:
    virtual ~Response() {}
    virtual bool isRequest() const { return false; }
    virtual uint32_t getSeqNum() const { return seqNum_; }
    virtual void setSeqNum( uint32_t seqNum ) { seqNum_ = seqNum; }

    virtual uint8_t getStatus() const = 0;
    // void setStatus( uint8_t status ) = 0;

    virtual std::string toString() const {
        char buf[64];
        snprintf( buf, sizeof(buf), "seqNum=%d %s status=%s",
                  seqNum_, typeToString(), statusToString(getStatus()) );
        return buf;
    }
    virtual void clear() = 0;
    virtual bool isValid() const { return true; }
    virtual bool isPing() const { return false; }

    virtual bool visit( ResponseVisitor& visitor ) /* throw (PvapException) */  = 0;

    bool correspondsRequest( const Request& request ) const {
        return request.getSeqNum() == getSeqNum();
        // FIXME: && request.matchResponseType(*this) );
    }

    // for serialization
    inline uint8_t getStatusValue() const { return getStatus(); }

    /// start timing measurements corresponding to request.
    virtual void startTiming( const Request& ) {}

protected:
    virtual const char* typeToString() const = 0;

protected:
    uint32_t   seqNum_;
    // uint8_t    status_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_RESPONSE_H */
