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
    PvssException(type), seqNum_(seqNum) {
        // showSeqNum();
    }

    PvapException( Type type, uint32_t seqNum, const char* fmt, ... ) :
    PvssException(type), seqNum_(seqNum) {
        SMSC_UTIL_EX_FILL(fmt);
        showSeqNum();
    }

public:
    virtual ~PvapException() throw () {}
    uint32_t getSeqNum() const { return seqNum_; }

protected:
    void showSeqNum() {
        char buf[32];
        snprintf(buf,sizeof(buf),",seqNum=%u",seqNum_);
        message += buf;
    }

private:
    uint32_t seqNum_;
};

} // namespace pvss
} // namespace scag2

namespace scag {
namespace pvss {

using scag2::pvss::PvapException;

}
}

#endif /* !_SCAG_PVSS_COMMON_PVAPEXCEPTION_H */
