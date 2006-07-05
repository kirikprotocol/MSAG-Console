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
using smsc::inman::inap::SSNSession;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::DialogListener;
using smsc::inman::inap::InvokeListener;

namespace smsc {
namespace inman {
namespace inap {
namespace chsri {

class CHSRIhandler { // GMSC/SCF <- HLR */
public: 
    virtual void onMapResult(CHSendRoutingInfoRes* arg) = 0;
    //dialog finalization/error handling:
    //if errLayer != errOk, dialog is aborted by reason = errcode
    virtual void onEndMapDlg(unsigned short ercode, InmanErrorType errLayer) = 0;
};

#define MAP_OPER_INITED 0x2 //'10'B  on BIG-ENDIAN
#define MAP_OPER_FAIL   0x3 //'11'B  on BIG-ENDIAN 
#define MAP_OPER_DONE   0x3 //'11'B  on BIG-ENDIAN 
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
    MapCHSRIDlg(SSNSession* pSession, CHSRIhandler * sri_handler, Logger * uselog = NULL);
    virtual ~MapCHSRIDlg();

    enum {
        chsriServiceResponse = 0
    };
    void reqRoutingInfo(const char * subcr_adr, USHORT_T timeout = 0) throw(CustomException);

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
    SSNSession* session;    //TCAP dialogs factory
    Logger*     logger;
    CHSRIhandler * sriHdl;
    CHSRIState   _sriState;  //current state of dialog
    CHSendRoutingInfoRes reqRes;
};

} //chsri
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_CHSRI__ */

