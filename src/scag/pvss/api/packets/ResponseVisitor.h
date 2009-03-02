#ifndef _SCAG_PVSS_BASE_RESPONSEVISITOR_H
#define _SCAG_PVSS_BASE_RESPONSEVISITOR_H

#include "scag/pvss/common/PvapException.h"

namespace scag2 {
namespace pvss {

class ErrorResponse;
class DelResponse;
class GetResponse;
class SetResponse;
class IncResponse;
class PingResponse;
class AuthResponse;
class BatchResponse;

///
/// interface ResponseVisitor
///
class ResponseVisitor
{
public:
    virtual ~ResponseVisitor() {}
    virtual bool visitErrResponse( ErrorResponse& resp ) throw (PvapException) = 0;
    virtual bool visitDelResponse( DelResponse& resp ) throw (PvapException) = 0;
    virtual bool visitSetResponse( SetResponse& resp ) throw (PvapException) = 0;
    virtual bool visitGetResponse( GetResponse& resp ) throw (PvapException) = 0;
    virtual bool visitIncResponse( IncResponse& resp ) throw (PvapException) = 0;
    virtual bool visitPingResponse( PingResponse& resp ) throw (PvapException) = 0;
    virtual bool visitAuthResponse( AuthResponse& resp ) throw (PvapException) = 0;
    virtual bool visitBatchResponse( BatchResponse& resp ) throw (PvapException) = 0;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_RESPONSEVISITOR_H */
