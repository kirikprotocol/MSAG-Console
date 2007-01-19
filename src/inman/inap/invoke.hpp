#ident "$Id$"
/* ************************************************************************* *
 * Transaction Component: ROS::Invoke
 * ************************************************************************* */
#ifndef __SMSC_INMAN_TCAP_INVOKE__
#define __SMSC_INMAN_TCAP_INVOKE__

#include "inman/inap/entity.hpp"
#include <string>

namespace smsc  {
namespace inman {
namespace inap  {

class InvokeListener;

class Invoke : public TcapEntity {
public:
    typedef enum { resWait = 0, resNotLast, resLast, resError, resLCancel
    } InvokeStatus;

    typedef enum { respNone = 0, respError, respResultOrError
    } InvokeResponse;

    Invoke(UCHAR_T tId = 0, UCHAR_T tOpCode = 0,
           InvokeResponse resp = Invoke::respNone, InvokeListener * pListener = NULL)
        : TcapEntity(tId, TcapEntity::tceInvoke, tOpCode), _iResHdl(pListener)
        , _linkedTo(NULL), _timeout(0), _status(Invoke::resWait), _respType(resp)
    {}

    ~Invoke() {}

    //copies and takes ownership of original param
    void clone(Invoke * org) { *this = *org; org->setParam(NULL); }

    inline InvokeStatus   getStatus(void) const { return _status; }
    inline InvokeResponse getResultType(void) const { return _respType; }
    
    inline void  linkTo(Invoke * linkedInv) { _linkedTo = linkedInv; }
    inline const Invoke * getLinkedTo(void) const { return _linkedTo; }

    inline void setTimeout(USHORT_T timeOut) { _timeout = timeOut; }
    inline USHORT_T getTimeout(void) const   { return _timeout; }

    void notifyResultListener(TcapEntity* resp, InvokeStatus resKind);

    std::string strStatus(void)
    {
        char buf[sizeof("Invoke[%u]{%u}: respType: %u, status: %u") + (sizeof(unsigned)*3)*4];
        int n = snprintf(buf, sizeof(buf)-1, "Invoke[%u]{%u}: respType: %u, status: %u",
                         (unsigned)id, (unsigned)opcode, (unsigned)_respType, (unsigned)_status);
        buf[n]=0;
        return buf;
    }

protected:
    InvokeStatus     _status;   //
    InvokeListener * _iResHdl;  //optional
    Invoke *         _linkedTo; //invoke to which this one linked to
    USHORT_T         _timeout;  //response waiting timeout
    InvokeResponse   _respType; //
};

class InvokeListener {
public:
    //NOTE: below methods should not take ownership of TcapEntity::param
    virtual void onInvokeResultNL(Invoke* inv, TcapEntity* res) = 0;
    //NOTE: Invoke listener is reset upon return from these callbacks
    //NOTE: originating Invoke is released by Dialog upon return from these callbacks
    virtual void onInvokeResult(Invoke* inv, TcapEntity* res) = 0;
    virtual void onInvokeError(Invoke* inv, TcapEntity* resE) = 0;
    virtual void onInvokeLCancel(Invoke* inv) = 0; //Local TCAP timer is expired
};


} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCAP_INVOKE__ */

