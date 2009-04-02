#ifndef _SCAG_PVSS_BASE_RESPONSEVISITOR_H
#define _SCAG_PVSS_BASE_RESPONSEVISITOR_H

#include "scag/pvss/common/PvapException.h"

namespace scag2 {
namespace pvss {

class ErrorResponse;
class PingResponse;
class AuthResponse;
class ProfileResponse;

///
/// interface ResponseVisitor
///
class ResponseVisitor
{
public:
    virtual ~ResponseVisitor() {}
    virtual bool visitErrResponse( ErrorResponse& resp )       /* throw (PvapException) */  = 0;
    virtual bool visitPingResponse( PingResponse& resp )       /* throw (PvapException) */  = 0;
    virtual bool visitAuthResponse( AuthResponse& resp )       /* throw (PvapException) */  = 0;
    virtual bool visitProfileResponse( ProfileResponse& resp ) /* throw (PvapException) */  = 0;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_RESPONSEVISITOR_H */
