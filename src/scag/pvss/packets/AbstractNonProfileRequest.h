#ifndef _SCAG_PVSS_BASE_ABSTRACTNONPROFILEREQUEST_H
#define _SCAG_PVSS_BASE_ABSTRACTNONPROFILEREQUEST_H

#include "Request.h"
#include "Command.h"

namespace scag2 {
namespace pvss {

///
/// Abstract class non profile request
///
class AbstractNonProfileRequest : public Request, public Command
{
protected:
    AbstractNonProfileRequest() : seqNum_(-1) {}
    AbstractNonProfileRequest( uint32_t seqNum ) : seqNum_(seqNum) {}
    
public:
    virtual uint32_t getSeqNum() const {
        return seqNum_;
    }
    virtual void setSeqNum( uint32_t seqNum ) {
        seqNum_ = seqNum;
    }
    virtual std::string toString() const {
        char buf[32];
        sprintf( buf, "seqNum=%d %s", seqNum_, typeToString() );
        return buf;
    }

protected:
    AbstractNonProfileRequest( const AbstractNonProfileRequest& other ) :
    Request(other), Command(other), seqNum_(other.seqNum_) {}

private:
    AbstractNonProfileRequest& operator = ( const AbstractNonProfileRequest& );

private:
    uint32_t seqNum_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_ABSTRACTNONPROFILEREQUEST_H */
