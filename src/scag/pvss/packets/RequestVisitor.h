#ifndef _SCAG_PVSS_BASE_REQUESTVISITOR_H
#define _SCAG_PVSS_BASE_REQUESTVISITOR_H

#include "scag/pvss/common/PvapException.h"

namespace scag2 {
namespace pvss {

class PingRequest;
class AuthRequest;
class AbstractProfileRequest;

class RequestVisitor
{
public:
    virtual ~RequestVisitor() {}
    virtual bool visitPingRequest( PingRequest& req ) throw(PvapException) = 0;
    virtual bool visitAuthRequest( AuthRequest& req ) throw(PvapException) = 0;
    virtual bool visitProfileRequest( AbstractProfileRequest& req ) throw(PvapException) = 0;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_REQUESTVISITOR_H */
