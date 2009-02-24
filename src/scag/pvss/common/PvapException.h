#ifndef _SCAG_PVSS_COMMON_PVAPEXCEPTION_H
#define _SCAG_PVSS_COMMON_PVAPEXCEPTION_H

#include "util/int.h"
#include "PvssException.h"

namespace scag2 {
namespace pvss {

class PvapException : public PvssException
{
protected:
    static Type getExcType( bool isreq ) {
        return isreq ? BAD_REQUEST : BAD_RESPONSE;
    }

protected:
    PvapException( Type type, uint32_t seqNum = uint32_t(-1) ) :
    PvssException(type), seqNum_(seqNum) {}

    PvapException( bool isreq, uint32_t seqNum, int tag = 0 ) :
    PvssException(getExcType(isreq)), seqNum_(seqNum), tag_(tag) {}


public:
    virtual ~PvapException() throw () {}
    uint32_t getSeqNum() const { return seqNum_; }

private:
    uint32_t seqNum_;
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
