#ifndef _SCAG_PVSS_BASE_PROFILERESPONSEVISITOR_H
#define _SCAG_PVSS_BASE_PROFILERESPONSEVISITOR_H

#include "scag/pvss/common/PvapException.h"

namespace scag2 {
namespace pvss {

class DelResponse;
class GetResponse;
class SetResponse;
class IncResponse;
class BatchResponse;
class GetProfileResponse;

///
/// interface ProfileResponseVisitor
///
class ProfileResponseVisitor
{
public:
    virtual ~ProfileResponseVisitor() {}
    virtual bool visitDelResponse( DelResponse& resp )     /* throw (PvapException) */  = 0;
    virtual bool visitSetResponse( SetResponse& resp )     /* throw (PvapException) */  = 0;
    virtual bool visitGetResponse( GetResponse& resp )     /* throw (PvapException) */  = 0;
    virtual bool visitIncResponse( IncResponse& resp )     /* throw (PvapException) */  = 0;
    virtual bool visitBatchResponse( BatchResponse& resp ) /* throw (PvapException) */  = 0;
    virtual bool visitGetProfileResponse( GetProfileResponse& resp ) { return false; }
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PROFILERESPONSEVISITOR_H */
