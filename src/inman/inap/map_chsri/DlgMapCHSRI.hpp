#pragma ident "$Id$"
/* ************************************************************************* *
 * MAP_SEND_ROUTING_INFO service: dialog implementation (over TCAP dialog)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_MAP_CHSRI__
#define __SMSC_INMAN_INAP_MAP_CHSRI__

#include "core/synchronization/EventMonitor.hpp"
using smsc::core::synchronization::EventMonitor;

#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
using smsc::inman::inap::TCSessionMA;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::TCDialogUserITF;

#include "inman/comp/map_chsri/MapCHSRIComps.hpp"
using smsc::inman::comp::chsri::CHSendRoutingInfoRes;
using smsc::util::RCHash;

namespace smsc {
namespace inman {
namespace inap {
namespace chsri {

class CHSRIhandlerITF { // GMSC/SCF <- HLR
public: 
    virtual void onMapResult(CHSendRoutingInfoRes* arg) = 0;
    //Dialog finalization/error handling:
    //if ercode != 0, no result has been got from MAP service,
    //NOTE: MAP dialog may be deleted only from this callback !!!
    virtual void onEndMapDlg(RCHash ercode = 0) = 0;

    virtual void Awake(void) = 0;
};

typedef union {
    unsigned short value;
    struct {
        unsigned int ctrInited : 2;
        unsigned int ctrResulted : 2; // INITED - ResultNL, DONE - ResultL
        unsigned int ctrAborted : 1;
        unsigned int ctrFinished : 1;
    } s;
} CHSRIState;

//NOTE: MapCHSRI doesn't maintain own timer for operations, it uses instead the 
//innate timer of the SS7 stack for Invoke lifetime.
class MapCHSRIDlg : TCDialogUserITF { // GMSC/SCF -> HLR
public:
    MapCHSRIDlg(TCSessionMA* pSession, CHSRIhandlerITF * sri_handler, Logger * uselog = NULL);
    //May be called only after successfull Unbind() call
    virtual ~MapCHSRIDlg();

    enum MapOperState  { operInited = 1, operFailed = 2, operDone = 3 };

    void reqRoutingInfo(const char * subcr_adr, USHORT_T timeout = 0) throw(CustomException);
    void reqRoutingInfo(const TonNpiAddress & tnpi_adr, USHORT_T timeout = 0) throw(CustomException);

    //Attempts to unbind TC User.
    //Returns true on succsess, false result means that this object has 
    //established references to handler.
    bool Unbind(void);
    //May be called only after successfull Unbind() call
    void endMapDlg(void);

protected:
    friend class smsc::inman::inap::Dialog;
    // TCDialogUserITF interface
    void onDialogInvoke(Invoke* op, bool lastComp) { }
    void onDialogContinue(bool compPresent);
    void onDialogPAbort(UCHAR_T abortCause);
    void onDialogREnd(bool compPresent);
    void onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                        USHORT_T userInfo_len, UCHAR_T *pUserInfo);
    void onDialogNotice(UCHAR_T reportCause,
                        TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                        UCHAR_T invId = 0, UCHAR_T opCode = 0);

    void onInvokeResult(InvokeRFP pInv, TcapEntity* res);
    void onInvokeError(InvokeRFP pInv, TcapEntity* resE);
    void onInvokeResultNL(InvokeRFP pInv, TcapEntity* res);
    void onInvokeLCancel(InvokeRFP pInv);
    //
    inline void Awake(void) { _sync.notify(); }

private:
    typedef smsc::core::synchronization::MTRefWrapper_T<CHSRIhandlerITF>    SRIUserRef;

    void endTCap(void); //ends TC dialog, releases Dialog()
    void unRefHdl(void);

    EventMonitor    _sync;
    unsigned        sriId;
    Dialog *        dialog;     //TCAP dialog
    TCSessionMA *   session;    //TCAP dialogs factory
    Logger*         logger;
    SRIUserRef      sriHdl;
    CHSRIState      _sriState;  //current state of dialog
    CHSendRoutingInfoRes reqRes;
};

} //chsri
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_CHSRI__ */

