/* ************************************************************************* *
 * MAP-PROCESS-UNSTRUCTURED-SS-REQUEST service:
 * dialog implementation (over TCAP dialog)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_MAP_USS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_MAP_USS__

#include "core/synchronization/EventMonitor.hpp"
using smsc::core::synchronization::EventMonitor;

#include "inman/inap/HDSSnSession.hpp"
#include "inman/inap/dialog.hpp"
using smsc::inman::inap::TCSessionSR;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::TCDialogUserITF;

#include "inman/comp/map_uss/MapUSSComps.hpp"
using smsc::inman::comp::uss::MAPUSS2CompAC;
using smsc::inman::comp::uss::ProcessUSSRequestArg;
using smsc::util::RCHash;

namespace smsc {
namespace inman {
namespace inap {
namespace uss {

//USS Dialog Handler
class USSDhandlerITF { //  <- gsmSCF */
protected:
    virtual ~USSDhandlerITF() //forbid interface destruction
    { }

public:
    virtual void onMapResult(MAPUSS2CompAC* arg) = 0;
    //dialog finalization/error handling:
    //if ercode != 0, no result has been got from MAP service,
    //NOTE: MAP dialog may be deleted only from this callback !!!
    virtual void onEndMapDlg(RCHash ercode = 0) = 0;
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
class MapUSSDlg : TCDialogUserITF { //  -> gsmSCF
public:
    MapUSSDlg(TCSessionSR* pSession, USSDhandlerITF * res_handler, Logger * uselog = NULL);
    virtual ~MapUSSDlg();

    enum MapOperState   { operInited = 1, operFailed = 2, operDone = 3 };

    //composes SS request data from plain text(ASCIIZ BY default).
    void requestSS(const char * txt_data,
                    const TonNpiAddress * subsc_adr = NULL, const char * subscr_imsi = NULL) throw (CustomException);
    //composes SS request data from preencoded binary data which encoding is identified by dcs.
    void requestSS(const std::vector<unsigned char> & rq_data, unsigned char dcs,
                    const TonNpiAddress * subsc_adr = NULL, const char * subscr_imsi = NULL) throw (CustomException);

    void endMapDlg(void);

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

    void onInvokeResult(InvokeRFP pInv, TcapEntity* res);
    void onInvokeError(InvokeRFP pInv, TcapEntity* resE);
    void onInvokeResultNL(InvokeRFP pInv, TcapEntity* res);
    void onInvokeLCancel(InvokeRFP pInv);
    //
    void Awake(void) { _sync.notify(); }

private:
    EventMonitor    _sync;
    TCDialogID      dlgId;
    //prefix for logging info
    const char *    _logPfx; //"MapUSS"
    char            _logId[sizeof("MapUSS[%u:%Xh]") + 2*sizeof(unsigned)*3 + 1];
    Dialog *        dialog;     //TCAP dialog
    TCSessionSR *   session;    //TCAP dialogs factory
    USSDhandlerITF * resHdl;    //request result handler
    USSDState       dlgState;   //current state of dialog
    std::auto_ptr<MAPUSS2CompAC> reqRes;
    Logger *        logger;

    void initSSDialog(ProcessUSSRequestArg & arg, const TonNpiAddress * subsc_adr = NULL,
                      const char * subscr_imsi = NULL) throw (CustomException);

    void endTCap(void); //ends TC dialog, releases Dialog()
};

} //uss
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_MAP_USS__ */

