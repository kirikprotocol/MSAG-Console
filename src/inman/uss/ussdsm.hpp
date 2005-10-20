#ident "$Id$"
// Диалог (транзакция) MAP Supplementary services related services

#ifndef __SMSC_INMAN_USS_USSDSM__
#define __SMSC_INMAN_USS_USSDSM__

#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/comp/usscomp.hpp"
#include "inman/interaction/ussmessages.hpp"
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

    //Gets result from TCAP 
    void result(TcapEntity* resL);
    void error(TcapEntity* err) {}
    void resultNL(TcapEntity* resNL) {};

protected:
    Invoke * orgInv;    //originating Invoke
    USSDSM * _dSM;      //parent USSDSM
};


class USSResultHandler  //handles result got from sygnal system
{
    public:
        virtual void onUSSRequestResult(ProcessUSSRequestRes* res) = 0;
};

//USSMan State Machine, created one per one TCP request
class VLR;
class USSDSM : public USSCommandHandler, USSResultHandler
{
public:
  
    USSDSM(VLR* vlr, int dsmId, Session* sess, Connect* conn);
    virtual ~USSDSM();

    //USSResultHandler interface
    //Gets response from Signal System, sends USSRequest result back to socket
    void onUSSRequestResult(ProcessUSSRequestRes* res);

    //USSCommandHandler interface
    //Creates request to Signal System and initiates dialog over TCAP,
    //sets TCAP result listener.
    void onProcessUSSRequest(USSRequestMessage* req);
    //sends request denial back to TCP socket
    void onDenyUSSRequest(USSRequestMessage* req);

protected:
    int         _dsmId;     //unique state machine id,
    Address     _msAdr;     //subscriber ISDN address from request
    VLR*        _vLR;       //parent VLR
    Session*    _session;   //TCAP dialog factory
    Connect*    _connect;   //TCP connect
    Dialog*     _dlg;       //TCAP dialog
    Logger*     logger;
};

//    USHORT_T handleEndDialog();


}//namespace uss
}//namespace inman
}//namespace smsc

#endif
