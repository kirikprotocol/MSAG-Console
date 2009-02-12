#ifndef _SCAG_PVSS_COMMON_PVAPEXCEPTION_H
#define _SCAG_PVSS_COMMON_PVAPEXCEPTION_H

#include "util/int.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag2 {
namespace pvss {

class PvapException : public scag::exceptions::SCAGException
{
protected:
    PvapException( uint32_t seqNum = uint32_t(-1) ) :seqNum_(seqNum), isRequest_(true), tag_(0) {}
    PvapException( bool isreq, uint32_t seqNum, int tag = 0 ) :
    SCAGException(), seqNum_(seqNum), isRequest_(isreq), tag_(tag) {}

public:
    virtual ~PvapException() throw () {}
    uint32_t getSeqNum() const { return seqNum_; }

private:
    uint32_t seqNum_;
    bool     isRequest_;
    int      tag_;
};

} // namespace pvss
} // namespace scag2

namespace scag {
namespace pvss {

using scag2::pvss::PvapException;

}
}

#endif /* !_SCAG_PVSS_COMMON_PVAPEXCEPTION_H */
