#ifndef _SCAG_PVSS_BASE_ERRORRESPONSE_H
#define _SCAG_PVSS_BASE_ERRORRESPONSE_H

#include "Response.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace pvss {

// should we inherit from BatchResponseComponent ?
class ErrorResponse : public Response
{
public:
    ErrorResponse() : Response() {}
    ErrorResponse( uint32_t seqNum ) : Response( seqNum ) {}
    ErrorResponse( uint32_t seqNum, StatusType status, const char* msg = 0) : Response(seqNum,status) {}

    virtual bool visit( ResponseVisitor& visitor ) /* throw (PvapException) */  {
        return visitor.visitErrResponse(*this);
    }

    virtual ErrorResponse* clone() const { return new ErrorResponse(*this); }

protected:
    virtual const char* typeToString() const { return "err_resp"; }

private:
    ErrorResponse( const ErrorResponse& other ) : Response(other) {}
    ErrorResponse& operator = ( const ErrorResponse& other );
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_ERRORRESPONSE_H */
