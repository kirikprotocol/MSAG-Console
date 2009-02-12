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

    virtual bool visit( ResponseVisitor& visitor ) throw (PvapException) {
        return visitor.visitErrResponse(*this);
    }

protected:
    virtual const char* typeToString() const { return "err_resp"; }

private:
    ErrorResponse( const ErrorResponse& other );
    ErrorResponse& operator = ( const ErrorResponse& other );
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_ERRORRESPONSE_H */
