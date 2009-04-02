#ifndef _SCAG_PVSS_BASE_PINGRESPONSE_H
#define _SCAG_PVSS_BASE_PINGRESPONSE_H

#include "AbstractNonProfileResponse.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace pvss {

class PingResponse : public AbstractNonProfileResponse
{
public:
    PingResponse() : AbstractNonProfileResponse() { initLog(); }
    // PingResponse( uint32_t seqNum ) : Response(seqNum), status_(UNKNOWN) {}
    PingResponse( uint32_t seqNum, uint8_t status = UNKNOWN ) : 
    AbstractNonProfileResponse(seqNum,status) { initLog(); }
    virtual ~PingResponse() { logDtor(); }

    virtual bool visit( ResponseVisitor& visitor ) /* throw (PvapException) */  {
        return visitor.visitPingResponse(*this);
    }

    virtual PingResponse* clone() const { return new PingResponse(*this); }
    virtual bool isPing() const { return true; }

protected:
    virtual const char* typeToString() const { return "ping_resp"; }

private:
    PingResponse( const PingResponse& other ) : AbstractNonProfileResponse(other) {
        initLog();
    }
    PingResponse& operator = ( const PingResponse& other );

};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PINGRESPONSE_H */
