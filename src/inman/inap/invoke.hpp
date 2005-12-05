#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_INVOKE__
#define __SMSC_INMAN_INAP_INVOKE__

#include "inman/inap/entity.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

class InvokeListener
{
    public:
        //NOTE: below methods should not take ownership of TcapEntity::param
        virtual void result(TcapEntity*) = 0;
        virtual void error(TcapEntity*) = 0;
        virtual void resultNL(TcapEntity*) = 0;
        virtual void lcancel(void) = 0; //timeout is expired
};

class Dialog;

//Implemets ROS::Invoke over TCAP, 
class Invoke : public TcapEntity
{
public:
    typedef enum { resWait = 0, resLast, resNotLast, resError, resLCancel
    } InvokeStatus;

    typedef enum { respNone = 0, respError, respResultOrError
    } InvokeResponse;

    Invoke(Dialog * dlg, UCHAR_T tId, UCHAR_T tTag = 0,
           UCHAR_T tOpCode = 0, InvokeResponse resp = Invoke::respNone)
        : TcapEntity(tId, tTag, tOpCode), _dlg(dlg), _iResHdl(NULL)
        , _linkedTo(NULL), _timeout(0), _status(Invoke::resWait), _respType(resp)
    {}

    ~Invoke() {}

    InvokeStatus   getStatus(void) const { return _status; }
    InvokeResponse getResponseType(void) const { return _respType; }

    void  setListener(InvokeListener * plistener) { _iResHdl = plistener; }
    const InvokeListener * getListener(void) const { return _iResHdl; }

    void  linkTo(Invoke * linkedInv) { _linkedTo = linkedInv; }
    const Invoke * getLinkedTo(void) const { return _linkedTo; }

    void setTimeout(USHORT_T timeOut) { _timeout = timeOut; }
    USHORT_T getTimeout(void) const   { return _timeout; }

    virtual void send(void); //throws runtime_error

    void notifyResultListener(TcapEntity* resp);
    void notifyResultNListener(TcapEntity* resp);
    void notifyErrorListener(TcapEntity* resp);
    void notifyLCancelListener(void);

protected:
    Dialog *         _dlg;      //parent Dialog
    InvokeListener * _iResHdl;  //optional
    Invoke *         _linkedTo; //invoke to which this one linked to
    USHORT_T         _timeout;  //response waiting timeout
    InvokeStatus     _status;   //
    InvokeResponse   _respType; //
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_INVOKE__ */

