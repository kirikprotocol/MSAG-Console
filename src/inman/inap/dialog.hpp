#ident "$Id$"
/* ************************************************************************* *
 * TCAP dialog implementation (initiated by local point).
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_DIALOG__
#define __SMSC_INMAN_INAP_DIALOG__

#include <map>
#include <list>

#include "inman/comp/acdefs.hpp"
using smsc::ac::ACOID;

#include "inman/comp/operfactory.hpp"
using smsc::inman::comp::OperationFactory;

#include "inman/inap/invoke.hpp"
#include "inman/inap/TCDlgErrors.hpp"

namespace smsc {
namespace inman {
namespace inap {

class SSNSession;

typedef union {
    unsigned short value;
    struct {
        unsigned int dlgLInited : 1;    //1: T_BEGIN_REQ [+ INVOKE_REQ]
        unsigned int dlgRInited : 2;    //1: T_CONT_IND, 2: + INVOKE_IND
        unsigned int dlgLContinued : 1; //1: T_CONT_REQ [+ INVOKE_REQ]
        /* ... */
        unsigned int dlgRContinued : 2; //1: T_CONT_IND, 2: + INVOKE_IND
        unsigned int dlgREnded : 2;     //1: T_END_IND, 2: + INVOKE_IND
        /* ... */
        unsigned int dlgLEnded : 1;     //1: T_END_REQ
        unsigned int dlgLUAborted : 1;  //1: T_U_ABORT_REQ
        unsigned int dlgRUAborted : 1;  //1: T_U_ABORT_IND
        unsigned int dlgPAborted : 1;   //1: T_P_ABORT_IND
        /* ... */
        unsigned int reserved : 4;
//NOTE: These masks are for BIG_ENDIAN platforms
#define TC_DLG_CONTINUED_MASK   0x1C00
#define TC_DLG_ENDED_MASK       0x0380
#define TC_DLG_ABORTED_MASK     0x0070
#define TC_DLG_CLOSED_MASK      (TC_DLG_ENDED_MASK|TC_DLG_ABORTED_MASK) //0x03F0
    } s;
} TC_DlgState;

#define TCAP_DLG_COMP_LAST  2
#define TCAP_DLG_COMP_WAIT  1

class DialogListener {
public:
    virtual void onDialogInvoke(Invoke*, bool lastComp) = 0;
    virtual void onDialogContinue(bool compPresent) = 0;
    virtual void onDialogREnd(bool compPresent) = 0;
    virtual void onDialogPAbort(UCHAR_T abortCause) = 0;
    virtual void onDialogUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                                    USHORT_T userInfo_len, UCHAR_T *pUserInfo) = 0;
    virtual void onDialogNotice(UCHAR_T reportCause,
                                TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                                UCHAR_T invId = 0, UCHAR_T opCode = 0) = 0;
};

static const USHORT_T   _DEFAULT_INVOKE_TIMER = 30; //seconds

//NOTE: All thrown CustomExceptions has errcode set to RCHash
class Dialog {
public:
    enum Ending { endBasic = 0, endPrearranged, endUAbort };

    void addListener(DialogListener* pListener);
    void removeListener(DialogListener* pListener);

    inline const std::string & getSUId(void)  const { return _tcSUId;  }

    TC_DlgState         getState(void);
    inline USHORT_T     getId(void)     const  { return _dId; }
    //Returns the default timeout for Invokes
    inline USHORT_T     getTimeout(void) const { return _timeout; }
    //Sets the default timeout for Invoke result waiting
    void                setInvokeTimeout(USHORT_T timeout);

    void    releaseInvoke(UCHAR_T invId);
    void    releaseAllInvokes(void);
    //Resets the Invoke timer, extending its lifetime (response waiting),
    //Returns TC_APIError
    USHORT_T  resetInvokeTimer(UCHAR_T inv_id);
    //Returns true is dialog has some Invokes pending (awaiting Result or LCancel)
    unsigned  pendingInvokes(void);

    // -- Component layer requests -- //
    //Creates and sends Invoke, timeout value equal to zero forces the
    //Invoke timer being set to the Dialog default timeout value,
    //doesn't take ownership of Component
    UCHAR_T /*inv_id*/ sendInvoke(UCHAR_T opcode, const Component *p_arg, InvokeListener * pLst = NULL,
                        USHORT_T timeout = 0) throw(CustomException);
    void    sendResultLast(TcapEntity* res) throw(CustomException);
    void    sendResultNotLast(TcapEntity* res) throw(CustomException);
    void    sendResultError(TcapEntity* res) throw(CustomException);

    // Transaction layer requests
    void beginDialog(UCHAR_T* ui = NULL, USHORT_T uilen = 0) throw(CustomException);
    //start dialog with new remote address
    void beginDialog(const SCCP_ADDRESS_T& remote_addr,
                     UCHAR_T* ui = NULL, USHORT_T uilen = 0) throw(CustomException);
    void continueDialog(void) throw(CustomException);
    void endDialog(Dialog::Ending type = endBasic) throw(CustomException);

    // Transaction layer callbacks (indications)
    // USHORT_T handleBeginDialog(); //NOTE: Unimplemented yet!
    USHORT_T handleContinueDialog(bool compPresent);
    USHORT_T handleEndDialog(bool compPresent);
    USHORT_T handlePAbortDialog(UCHAR_T abortCause);
    USHORT_T handleUAbort(USHORT_T abortInfo_len, UCHAR_T *pAbortInfo,
                          USHORT_T userInfo_len, UCHAR_T *pUserInfo);

    // Component layer callbacks (indications)
    USHORT_T handleInvoke(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen,
                                const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm,
                                bool lastComp);
    USHORT_T handleResultLast(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen,
                                const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm);
    USHORT_T handleResultNotLast(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen,
                                const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm);
    USHORT_T handleResultError(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen,
                                const UCHAR_T *op, USHORT_T pmlen,  const UCHAR_T *pm);
    USHORT_T handleLCancelInvoke(UCHAR_T invokeId);
    USHORT_T handleNoticeInd(UCHAR_T reportCause, TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                             UCHAR_T invokeId = 0, USHORT_T oplen = 0, const UCHAR_T *op = NULL);

protected:
    friend class TCSessionAC;
    Dialog(const std::string & sess_uid, USHORT_T dlg_id, USHORT_T msg_user_id,
           ACOID::DefinedOIDidx dialog_ac_idx, const SCCP_ADDRESS_T & loc_addr,
           UCHAR_T sender_ssn = 0, Logger * uselog = NULL);
    //reinitializes Dialog to be reused with other id and remote address
    void reset(USHORT_T new_id, const SCCP_ADDRESS_T * rmt_addr);

protected:
    friend class SSNSession;
    virtual ~Dialog();

private:
    void clearInvokes(bool reqTC = false);
    void checkSS7res(const char * descr, USHORT_T result) throw(CustomException);
    UCHAR_T  getNextInvokeId(void)  { MutexGuard  tmp(invGrd);  return ++_lastInvId; }

    typedef std::list<DialogListener*> ListenerList;
    typedef std::map<UCHAR_T, Invoke*> InvokeMap;

    std::string     _tcSUId;     //TC session signature
    Mutex           invGrd;      //invokes guard
    InvokeMap       originating; //Invokes, which have result/errors defined
    InvokeMap       terminating; //
    Mutex           dlgGrd;      //state, listeners guard
    ListenerList    listeners;   //
    TC_DlgState     _state;

    SCCP_ADDRESS_T  ownAddr;
    SCCP_ADDRESS_T  rmtAddr;
    UCHAR_T         dSSN;   //SubSystemNumber dialog uses
    USHORT_T        msgUserId;  //Common part message port user id
    APP_CONTEXT_T   ac;
    USHORT_T        _dId;
    USHORT_T        _timeout;
    UCHAR_T         qSrvc;
    UCHAR_T         priority;
    UCHAR_T         _lastInvId;
    Logger*         logger;
    ACOID::DefinedOIDidx   _ac_idx; //ApplicationContext index, see acdefs.hpp
    OperationFactory * ac_fact;
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_DIALOG__ */

