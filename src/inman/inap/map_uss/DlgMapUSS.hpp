#ident "$Id$"
/* ************************************************************************* *
 * MAP-PROCESS-UNSTRUCTURED-SS-REQUEST service:
 * dialog implementation (over TCAP dialog)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_MAP_USS__
#define __SMSC_INMAN_INAP_MAP_USS__

#include "inman/inerrcodes.hpp"
using smsc::inman::InmanErrorType;
using smsc::inman::_InmanErrorSource;

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::Mutex;

#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
using smsc::inman::inap::TCSessionSR;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::DialogListener;
using smsc::inman::inap::InvokeListener;

#include "inman/comp/map_uss/MapUSSComps.hpp"
using smsc::inman::comp::uss::MAPUSS2CompAC;

#define PLAIN_LATIN1_DCS 0xF4 // 1111 0100

namespace smsc {
namespace inman {
namespace inap {
namespace uss {

//USS Dialog Handler
class USSDhandlerITF { //  <- gsmSCF */
public: 
    virtual void onMapResult(MAPUSS2CompAC* arg) = 0;
    //dialog finalization/error handling:
    //if errLayer != errOk, dialog is aborted by reason = errcode
    virtual void onEndMapDlg(unsigned short ercode, InmanErrorType errLayer) = 0;
};

//USS Dialog state
typedef union {
    unsigned short value;
    struct {
        unsigned int ctrInited : 2;
        unsigned int ctrResulted : 2; // INITED - ResultNL, DONE - ResultL
        unsigned int ctrAborted : 1;
        unsigned int ctrFinished : 1;
    } s;
} USSDState;

//NOTE: MapUSSDlg doesn't maintain own timer for operations, it uses instead the 
//innate timer of the SS7 stack for Invoke lifetime.
class MapUSSDlg : DialogListener, InvokeListener { //  -> gsmSCF
public:
    MapUSSDlg(TCSessionSR* pSession, USSDhandlerITF * res_handler, Logger * uselog = NULL);
    virtual ~MapUSSDlg();

    enum MapUSSDlgError { ussServiceResponse = 0xFF };
    enum MapOperState   { operInited = 1, operFailed = 2, operDone = 3 };

    void requestSS(const std::vector<unsigned char> & rq_data, unsigned char dcs,
                    const char * subsc_adr = NULL) throw (CustomException);
    void requestSS(const std::vector<unsigned char> & rq_data, unsigned char dcs,
                    const TonNpiAddress * subsc_adr = NULL) throw (CustomException);

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
    unsigned    dlgId;
    Dialog*     dialog;     //TCAP dialog
    TCSessionSR* session;   //TCAP dialogs factory
    Logger*     logger;
    USSDhandlerITF * resHdl;   //request result handler
    USSDState   dlgState;   //current state of dialog
    std::auto_ptr<MAPUSS2CompAC> reqRes;
};

} //uss
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_USS__ */

