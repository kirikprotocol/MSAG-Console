#ident "$Id$"
// MAP_SEND_ROUTING_INFO service: dialog implementation (over TCAP dialog)

#ifndef __SMSC_INMAN_INAP_MAP_CHSRI__
#define __SMSC_INMAN_INAP_MAP_CHSRI__

#include "core/synchronization/Mutex.hpp"
#include "inman/inerrcodes.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/comp/map_chsri/MapCHSRIComps.hpp"

using smsc::core::synchronization::Mutex;

using smsc::inman::InmanErrorType;
using smsc::inman::_InmanErrorSource;

using smsc::inman::comp::chsri::CHSendRoutingInfoRes;
using smsc::inman::inap::TCSessionMA;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::DialogListener;
using smsc::inman::inap::InvokeListener;

namespace smsc {
namespace inman {
namespace inap {
namespace chsri {

class CHSRIhandlerITF { // GMSC/SCF <- HLR
public: 
    virtual void onMapResult(CHSendRoutingInfoRes* arg) = 0;
    //Dialog finalization/error handling:
    //if errLayer != errOk, dialog is aborted by reason = errcode
    //NOTE: MAP diaog may be deleted only from this callback !!!
    virtual void onEndMapDlg(unsigned short ercode, InmanErrorType errLayer) = 0;
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
class MapCHSRIDlg : DialogListener, InvokeListener { // GMSC/SCF -> HLR
public:
    MapCHSRIDlg(TCSessionMA* pSession, CHSRIhandlerITF * sri_handler, Logger * uselog = NULL);
    virtual ~MapCHSRIDlg();

    enum CHSRIDlgError { chsriServiceResponse = 1 };
    enum MapOperState  { operInited = 1, operFailed = 2, operDone = 3 };

    void reqRoutingInfo(const char * subcr_adr, USHORT_T timeout = 0) throw(CustomException);
    void reqRoutingInfo(const TonNpiAddress & tnpi_adr, USHORT_T timeout = 0) throw(CustomException);

    void endMapDlg(void);

protected:
    friend class smsc::inman::inap::Dialog;
    // DialogListener interface
    void onDialogInvoke(Invoke* op, bool lastComp) { }
    void onDialogContinue(bool compPresent);
    void onDialogPAbort(UCHAR_T abortCause);
    void onDialogREnd(bool compPresent);
    void onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                        USHORT_T userInfo_len, UCHAR_T *pUserInfo);
    void onDialogNotice(UCHAR_T reportCause,
                        TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                        UCHAR_T invId = 0, UCHAR_T opCode = 0);

    // InvokeListener interface
    void onInvokeResult(Invoke* inv, TcapEntity* res);
    void onInvokeError(Invoke* inv, TcapEntity* resE);
    void onInvokeResultNL(Invoke* inv, TcapEntity* res);
    void onInvokeLCancel(Invoke* inv);

private:
    void endTCap(void); //ends TC dialog, releases Dialog()

    Mutex       _sync;
    unsigned    sriId;
    Dialog*     dialog;     //TCAP dialog
    TCSessionMA* session;    //TCAP dialogs factory
    Logger*     logger;
    CHSRIhandlerITF * sriHdl;
    CHSRIState   _sriState;  //current state of dialog
    CHSendRoutingInfoRes reqRes;
};

} //chsri
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_CHSRI__ */

