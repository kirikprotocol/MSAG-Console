#ifndef _SCAG_PVSS_BASE_ABSTRACTNONPROFILERESPONSE_H
#define _SCAG_PVSS_BASE_ABSTRACTNONPROFILERESPONSE_H

#include "Response.h"

namespace scag2 {
namespace pvss {

///
/// Abstract class non profile request
///
class AbstractNonProfileResponse : public Response
{
protected:
    AbstractNonProfileResponse() : Response(), status_(UNKNOWN) {}
    AbstractNonProfileResponse( uint32_t seqNum, uint8_t status = UNKNOWN ) : Response(seqNum), status_(status) {}
    
public:
    virtual uint8_t getStatus() const { return status_; }
    void setStatus( uint8_t status ) { status_ = status; }
    virtual void clear() { status_ = UNKNOWN; }

    // for serialization
    inline void setStatusValue( uint8_t status ) { status_ = status; }

private:
    uint8_t status_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_ABSTRACTNONPROFILERESPONSE_H */
