#ident "$Id$"
// mapATIH service: ATSI dialog implementation (over TCAP dialog)

#ifndef __SMSC_INMAN_INAP_MAP_ATSI__
#define __SMSC_INMAN_INAP_MAP_ATSI__

#include "core/synchronization/Mutex.hpp"
#include "inman/inerrcodes.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/comp/map_atih/MapATSIComps.hpp"

using smsc::core::synchronization::Mutex;

using smsc::inman::InmanErrorType;
using smsc::inman::_InmanErrorSource;

using smsc::inman::comp::atih::ATSIArg;
using smsc::inman::comp::atih::ATSIRes;
using smsc::inman::inap::TCSessionMA;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::DialogListener;
using smsc::inman::inap::InvokeListener;

namespace smsc {
namespace inman {
namespace inap {
namespace atih {

class ATSIhandler { // SCF <- HLR */
public: 
    virtual void onATSIResult(ATSIRes* arg) = 0;
    //dialog finalization/error handling:
    //if errLayer != errOk, dialog is aborted by reason = errcode
    virtual void onEndATSI(unsigned short ercode, InmanErrorType errLayer) = 0;
};

typedef union {
    unsigned short value;
    struct {
        unsigned int ctrInited : 2;
        unsigned int ctrResulted : 1;
        unsigned int ctrAborted : 1;
        unsigned int ctrFinished : 1;
    } s;
} ATSIState;

//NOTE: DlgATSI doesn't maintain own timer for operations, it uses instead the 
//innate timer of the SS7 stack for Invoke lifetime.
class MapATSIDlg : DialogListener, InvokeListener { // SCF -> HLR
public:
    MapATSIDlg(TCSessionMA* pSession, ATSIhandler * atsi_handler, Logger * uselog = NULL);
    virtual ~MapATSIDlg();

    enum ATSIDlgError { atsiServiceResponse = 1 };
    enum MapOperState { operInited = 1, operFailed = 2, operDone = 3 };

    void subsciptionInterrogation(const char * subcr_adr,
                        bool imsi = false, USHORT_T timeout = 0) throw(CustomException);

    void endATSI(void);

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

    // InvokeListener interface (no OPs with returnResult defined)
    void onInvokeResult(Invoke* inv, TcapEntity* res);
    void onInvokeError(Invoke* inv, TcapEntity* resE);
    void onInvokeResultNL(Invoke* inv, TcapEntity* res) { }
    void onInvokeLCancel(Invoke* inv);

private:
    void endTCap(void); //ends TC dialog, releases Dialog()

    Mutex       _sync;
    unsigned    atsiId;
    Dialog*     dialog;     //TCAP dialog
    TCSessionMA* session;    //TCAP dialogs factory
    Logger*     logger;
    ATSIhandler * atsiHdl;
    ATSIState   _atsiState;  //current state of mapATSI dialog
};

} //atih
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_ATSI__ */

