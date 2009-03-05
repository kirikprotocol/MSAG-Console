#ifndef _SCAG_PVSS_BASE_PINGRESPONSE_H
#define _SCAG_PVSS_BASE_PINGRESPONSE_H

#include "Response.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace pvss {

class PingResponse : public Response
{
public:
    PingResponse() : Response() {}
    PingResponse( uint32_t seqNum ) : Response(seqNum) {}

    virtual bool visit( ResponseVisitor& visitor ) throw (PvapException) {
        return visitor.visitPingResponse(*this);
    }
    virtual PingResponse* clone() const { return new PingResponse(*this); }
    virtual bool isPing() const { return true; }

protected:
    virtual const char* typeToString() const { return "ping_resp"; }

private:
    PingResponse( const PingResponse& other ) : Response(other) {}
    PingResponse& operator = ( const PingResponse& other );
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PINGRESPONSE_H */
