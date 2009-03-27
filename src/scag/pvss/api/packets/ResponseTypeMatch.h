#ifndef _SCAG_PVSS_BASE_RESPONSEMATCH_H
#define _SCAG_PVSS_BASE_RESPONSEMATCH_H

#include "ResponseVisitor.h"

namespace scag2 {
namespace pvss {

///
/// Abstract class Request
///
class ResponseTypeMatch : public ResponseVisitor
{
public:
    virtual bool visitErrResponse( ErrorResponse& resp ) /* throw (PvapException) */  {
        return true;
    }
    virtual bool visitDelResponse( DelResponse& resp ) /* throw (PvapException) */  {
        return false;
    }
    virtual bool visitSetResponse( SetResponse& resp ) /* throw (PvapException) */  {
        return false;
    }
    virtual bool visitGetResponse( GetResponse& resp ) /* throw (PvapException) */  {
        return false;
    }
    virtual bool visitIncResponse( IncResponse& resp ) /* throw (PvapException) */  {
        return false;
    }
    virtual bool visitPingResponse( PingResponse& resp ) /* throw (PvapException) */  {
        return false;
    }
    virtual bool visitAuthResponse( AuthResponse& resp ) /* throw (PvapException) */  {
        return false;
    }
    virtual bool visitBatchResponse( BatchResponse& resp ) /* throw (PvapException) */  {
        return false;
    }
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_RESPONSEMATCH_H */
