#pragma ident "$Id$"
/* ************************************************************************* *
 * mapATIH service: ATSI dialog implementation (over TCAP dialog)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_MAP_ATSI__
#define __SMSC_INMAN_INAP_MAP_ATSI__

#include "core/synchronization/EventMonitor.hpp"
using smsc::core::synchronization::EventMonitor;

#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
using smsc::inman::inap::TCSessionMA;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::TCDialogUserITF;

#include "inman/comp/map_atih/MapATSIComps.hpp"
using smsc::inman::comp::atih::ATSIArg;
using smsc::inman::comp::atih::ATSIRes;
using smsc::util::RCHash;

namespace smsc {
namespace inman {
namespace inap {
namespace atih {

class ATSIhandlerITF { // SCF <- HLR */
public: 
    virtual void onATSIResult(ATSIRes* arg) = 0;
    //dialog finalization/error handling:
    //if ercode != 0, no result has been got from MAP service,
    //NOTE: MAP dialog may be deleted only from this callback !!!
    virtual void onEndATSI(RCHash ercode = 0) = 0;
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
class MapATSIDlg : TCDialogUserITF { // SCF -> HLR
public:
    MapATSIDlg(TCSessionMA* pSession, ATSIhandlerITF * atsi_handler, Logger * uselog = NULL);
    virtual ~MapATSIDlg();

    enum MapOperState { operInited = 1, operFailed = 2, operDone = 3 };

    void subsciptionInterrogation(const char * subcr_adr, bool imsi = false,
                                uint16_t timeout = 0) throw(CustomException);
    void endATSI(void);

protected:
    friend class smsc::inman::inap::Dialog;
    // TCDialogUserITF interface
    void onDialogInvoke(Invoke* op, bool lastComp) { }
    void onDialogContinue(bool compPresent);
    void onDialogPAbort(uint8_t abortCause);
    void onDialogREnd(bool compPresent);
    void onDialogUAbort(uint16_t abortInfo_len, uint8_t *pAbortInfo,
                        uint16_t userInfo_len, uint8_t *pUserInfo);
    void onDialogNotice(uint8_t reportCause,
                        TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                        uint8_t invId = 0, uint8_t opCode = 0);

    // no OPs with returnResult defined
    void onInvokeResult(InvokeRFP pInv, TcapEntity* res);
    void onInvokeError(InvokeRFP pInv, TcapEntity* resE);
    void onInvokeResultNL(InvokeRFP pInv, TcapEntity* res) { }
    void onInvokeLCancel(InvokeRFP pInv);
    //
    inline void Awake(void) { _sync.notify(); }

private:
    EventMonitor    _sync;
    unsigned        atsiId;
    Dialog *        dialog;     //TCAP dialog
    TCSessionMA *   session;    //TCAP dialogs factory
    Logger *        logger;
    ATSIhandlerITF * atsiHdl;
    ATSIState       _atsiState;  //current state of mapATSI dialog

    void endTCap(void); //ends TC dialog, releases Dialog()
};

} //atih
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_ATSI__ */

