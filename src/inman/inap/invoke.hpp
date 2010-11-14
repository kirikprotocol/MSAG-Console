/* ************************************************************************* *
 * Transaction Component: ROS::Invoke
 * ************************************************************************* */
#ifndef __SMSC_INMAN_TCAP_INVOKE__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TCAP_INVOKE__

#include <string>

#include "inman/inap/entity.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

class Invoke : public TcapEntity {
public:
    typedef enum { resWait = 0, resNotLast, resLast, resError, resLCancel
    } InvokeStatus;

    typedef enum { respNone = 0, respError, respResultOrError
    } InvokeResponse;

    Invoke(uint8_t tId = 0, uint8_t tOpCode = 0, InvokeResponse resp = Invoke::respNone)
        : TcapEntity(tId, TcapEntity::tceInvoke, tOpCode)
        , _status(Invoke::resWait), _linkedTo(NULL)
        , _timeout(0), _respType(resp)
    {}

    ~Invoke()
    {}

    void           setStatus(InvokeStatus use_st) { _status = use_st; }
    InvokeStatus   getStatus(void) const { return _status; }
    InvokeResponse getResultType(void) const { return _respType; }
    
    void  linkTo(Invoke * linkedInv) { _linkedTo = linkedInv; }
    const Invoke * getLinkedTo(void) const { return _linkedTo; }

    void setTimeout(uint16_t timeOut) { _timeout = timeOut; }
    uint16_t getTimeout(void) const   { return _timeout; }

    std::string strStatus(void)
    {
        char buf[sizeof("Invoke[%u]{%u}: respType: %u, status: %u") + (sizeof(unsigned)*3)*4];
        int n = snprintf(buf, sizeof(buf)-1, "Invoke[%u]{%u}: respType: %u, status: %u",
                         (unsigned)id, (unsigned)opcode, (unsigned)_respType, (unsigned)_status);
        if ((n < 1) || (n >= (int)sizeof(buf))) {
            buf[0]='?'; buf[1]=0;
        }
        return buf;
    }

protected:
    InvokeStatus     _status;   //
    Invoke *         _linkedTo; //invoke to which this one linked to
    uint16_t         _timeout;  //response waiting timeout
    InvokeResponse   _respType; //
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCAP_INVOKE__ */

