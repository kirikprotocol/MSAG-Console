#ident "$Id$"
// Диалог (транзакция) MAP Supplementary services related services

#ifndef __SMSC_INMAN_USS_USSDSM__
#define __SMSC_INMAN_USS_USSDSM__

#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/comp/usscomp.hpp"
#include "inman/uss/ussmessages.hpp"
#include "inman/interaction/connect.hpp"


using smsc::inman::usscomp::ProcessUSSRequestRes;
using smsc::inman::usscomp::ProcessUSSRequestArg;
using smsc::inman::inap::Session;
using smsc::inman::inap::InvokeListener;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::DialogListener;
using smsc::inman::inap::Invoke;
using smsc::inman::inap::TcapEntity;
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::USSCommandHandler;
using smsc::inman::interaction::USSRequestMessage;


namespace smsc {
namespace inman {
namespace uss {

class USSDSM;
class USSTcapListener: public InvokeListener
{
public:
    USSTcapListener(Invoke * op, USSDSM * dsm);
    ~USSTcapListener() {}

    Invoke * getInvId() const;
    //Gets result from TCAP 
    void result(TcapEntity* resL);
    void error(TcapEntity* err);
    void resultNL(TcapEntity* resNL) {}
    void lcancel(void) {}

protected:
    Invoke * orgInv;    //originating Invoke
    USSDSM * _dSM;      //parent USSDSM
};


class USSResultHandler  //handles result got from sygnal system
{
    public:
        virtual void onUSSRequestResult(ProcessUSSRequestRes* res) = 0;
        virtual void onUSSRequestError(unsigned char ercode) = 0;
};

class USSDialog : public Dialog
{
public:
//    enum { dlg_idle = 0, dlg_inited, dlg_end };
    USSDialog(USSDSM * pDsm, Session* pSession);
    virtual ~USSDialog();
    // Transaction level callbacks
//    USHORT_T handleEndDialog();

protected:
    USSDSM * _pDsm; //parent state machine
};


//USSMan State Machine, created one per one TCP request
class VLR;
class USSDSM : public USSCommandHandler, public USSResultHandler
{
public:
//    enum { dsm_idle = 0, dsm_inited, dsm_processed, dsm_done };
    USSDSM(VLR* vlr, int dsmId, Session* sess, Connect* conn);
    virtual ~USSDSM();

    unsigned int getDSMId(void) const;

    //USSResultHandler interface
    //Gets response from Signal System, sends USSRequest result back to socket
    void onUSSRequestResult(ProcessUSSRequestRes* res);
    void onUSSRequestError(unsigned char ercode);

    //USSCommandHandler interface
    //Creates request to Signal System and initiates dialog over TCAP,
    //sets TCAP result listener.
    void onProcessUSSRequest(USSRequestMessage* req);
    //forms request denial packet
    void onDenyUSSRequest(USSRequestMessage* req);
    //
    //void onDialogEnd(Dialog * dlg);

protected:
    //const members:
    unsigned int    _dsmId;     //unique state machine id,
    Address         _msAdr;     //subscriber ISDN address from request
    VLR*            _vLR;       //parent VLR
    Session*        _session;   //TCAP dialog factory
    Connect*        _connect;   //TCP connect
    //
    USSDialog*       _dlg;      //TCAP dialog
    USSTcapListener* _ires;     //invoke result listener 
    Logger*          logger;
};

}//namespace uss
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_USS_USSDSM__ */

