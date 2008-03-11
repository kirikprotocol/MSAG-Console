#pragma ident "$Id$"
/* ************************************************************************* *
 * TCAP dialog implementation (initiated by local point).
 * ************************************************************************* */
#ifndef __SMSC_INMAN_INAP_DIALOG__
#define __SMSC_INMAN_INAP_DIALOG__

#include <map>
#include <list>

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "inman/inap/ACRegistry.hpp"
#include "inman/inap/invoke.hpp"
#include "inman/inap/TCDlgErrors.hpp"
#include "inman/inap/SS7Types.hpp"

#include "inman/common/XRefPtr.hpp"
#include "inman/common/MTRefWrapper.hpp"

namespace smsc {
namespace inman {
namespace inap {

typedef union {
    unsigned short value;
    struct {
#define TCAP_DLG_COMP_LAST  2
#define TCAP_DLG_COMP_WAIT  1
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
#define TC_DLG_ENDED_MASK       0x0280
#define TC_DLG_ABORTED_MASK     0x0070
#define TC_DLG_CLOSED_MASK      (TC_DLG_ENDED_MASK|TC_DLG_ABORTED_MASK) //0x03F0
    } s;
} TC_DlgState;


typedef smsc::core::synchronization::URefPtr_T<Invoke> InvokeRFP;

class TCDialogUserITF {
public:
    // -- Transaction layer indications 
    virtual void onDialogInvoke(Invoke*, bool lastComp) = 0;
    virtual void onDialogContinue(bool compPresent) = 0;
    virtual void onDialogREnd(bool compPresent) = 0;
    virtual void onDialogPAbort(uint8_t abortCause) = 0;
    virtual void onDialogUAbort(uint16_t abortInfo_len, uint8_t *pAbortInfo,
                                    uint16_t userInfo_len, uint8_t *pUserInfo) = 0;
    virtual void onDialogNotice(uint8_t reportCause,
                                TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                                uint8_t invId = 0, uint8_t opCode = 0) = 0;
    // -- Component layer indications 

    //NOTE: below methods should either take ownership of TcapEntity::param or copy it
    virtual void onInvokeResultNL(InvokeRFP pInv, TcapEntity* res) = 0;
    //NOTE: originating Invoke is released by Dialog upon return from these callbacks
    virtual void onInvokeResult(InvokeRFP pInv, TcapEntity* res) = 0;
    virtual void onInvokeError(InvokeRFP pInv, TcapEntity* resE) = 0;
    virtual void onInvokeLCancel(InvokeRFP pInv) = 0; //Local invoke timer is expired

    virtual void Awake(void) = 0;
};

class SSNSession;

//NOTE: All thrown CustomExceptions has errcode set to RCHash
class Dialog {
private:
    typedef smsc::core::synchronization::MTRefWrapper_T<TCDialogUserITF>     DlgUserRef;
    typedef std::map<uint8_t, InvokeRFP> InvokeMap;

    std::string     _tcSUId;    //TC session signature
    InvokeMap       invMap;     //locally initiated Invokes
    mutable Mutex   dlgGrd;     //state, listeners guard
    DlgUserRef      pUser;    //
    TC_DlgState     _state;

    SCCP_ADDRESS_T  ownAddr;
    SCCP_ADDRESS_T  rmtAddr;
    uint8_t         dSSN;       //TC SubSystem Number dialog uses
    uint16_t        msgUserId;  //EINSS7 Common part message port user id
    const EncodedOID & ac;
    const ROSComponentsFactory * acFab; //applicatiojn context Components factory
    uint16_t        _dId;
                    //prefix for logging info
    char            _logId[sizeof("Dialog[0x%X]") + sizeof(unsigned int)*3 + 1];
    uint16_t        _timeout;
    uint8_t         qSrvc;
    uint8_t         priority;
    uint8_t         _lastInvId;
    Logger*         logger;



    void clearInvokes(void);
    //
    void checkSS7res(const char * descr, uint16_t result) throw(CustomException);
    //
    uint8_t  getNextInvokeId(void)
    {
        MutexGuard  tmp(dlgGrd);
        return ++_lastInvId;
    }
    //Updates dialog state on reception of last component of T_END or T_CONTINUE
    //Returns true if dialog is ended and no more components expected
    void updateState(bool doClear = false)
    {
        if (_state.s.dlgRContinued == TCAP_DLG_COMP_WAIT)
            _state.s.dlgRContinued = TCAP_DLG_COMP_LAST;
        else if (_state.s.dlgREnded == TCAP_DLG_COMP_WAIT) {
            _state.s.dlgREnded = TCAP_DLG_COMP_LAST;
            //if Dialog is remotedly ended, no LCancel or Result/Error
            //indications will arise for invokes
            if (doClear)
                clearInvokes();
        }
    }

    //if Dialog is remotedly ended, no LCancel or Result/Error
    //indications will arise for invokes
    inline void clenUpInvokes(void)
    {
        if (_state.s.dlgREnded == TCAP_DLG_COMP_LAST)
            clearInvokes();
    }

    void unlockUser(void)
    {
        MutexGuard tmp(dlgGrd);
        pUser.UnLock();
        if (pUser.get())
            pUser->Awake();
        return;
    }

protected:
    friend class TCSessionAC;
    Dialog(const std::string & sess_uid, uint16_t dlg_id, uint16_t msg_user_id,
           const ROSComponentsFactory * use_fact, const SCCP_ADDRESS_T & loc_addr,
           uint8_t sender_ssn = 0, Logger * uselog = NULL);
    //reinitializes Dialog to be reused with other id and remote address
    void reset(uint16_t new_id, const SCCP_ADDRESS_T * rmt_addr);

protected:
    friend class SSNSession;
    virtual ~Dialog();

public:
    static const uint16_t   _DFLT_INVOKE_TIMER = 30; //seconds
    enum Ending { endBasic = 0, endPrearranged, endUAbort };

    //Returns pointer to previously set user.
    //(non-zero value means that calling code has big problem :))
    TCDialogUserITF * bindUser(TCDialogUserITF * p_user)
    {
        MutexGuard  tmp(dlgGrd);
        TCDialogUserITF * old = pUser.get();
        pUser.Reset(p_user);
        return old;
    }
    //Attempts to unbind TC User.
    //Returns true on succsess, false result means that this dialog has
    //established references to user
    bool unbindUser(void)
    {
        MutexGuard  tmp(dlgGrd);
        return pUser.Unref();
    }
    //
    TC_DlgState getState(void) const
    { 
        MutexGuard  tmp(dlgGrd);
        return _state; 
    }
    inline uint16_t     getId(void)     const  { return _dId; }
    inline const std::string & getSUId(void)  const { return _tcSUId;  }

    //Returns false if there are invokes pending or dialog has
    //unfinished calls to user.
    bool isFinished(unsigned * p_invNum = NULL) const
    {
        MutexGuard  tmp(dlgGrd);
        if (p_invNum) 
            *p_invNum = invMap.size();
        return ((_state.value & TC_DLG_CLOSED_MASK)
                && invMap.empty() && !pUser.get()) ? true : false;
    }
    //Returns true is dialog has some Invokes pending (awaiting Result or LCancel)
    unsigned  pendingInvokes(void) const
    {
        MutexGuard  tmp(dlgGrd);
        return invMap.size();
    }

    //Returns the default timeout for Invokes
    inline uint16_t     getTimeout(void) const { return _timeout; }
    //Sets the default timeout for Invoke result waiting
    void                setInvokeTimeout(uint16_t timeout);

    void    releaseInvoke(uint8_t invId);
    void    releaseAllInvokes(void);
    //Resets the Invoke timer, extending its lifetime (response waiting),
    //Returns TC_APIError
    uint16_t  resetInvokeTimer(uint8_t inv_id);

    // -- Component layer requests -- //
    //Creates and sends Invoke, timeout value equal to zero forces the
    //Invoke timer being set to the Dialog default timeout value,
    //doesn't take ownership of Component
    uint8_t /*inv_id*/ sendInvoke(uint8_t opcode, const Component *p_arg, uint16_t timeout = 0)
                        throw(CustomException);
    void    sendResultLast(TcapEntity* res) throw(CustomException);
    void    sendResultNotLast(TcapEntity* res) throw(CustomException);
    void    sendResultError(TcapEntity* res) throw(CustomException);

    // Transaction layer requests
    void beginDialog(uint8_t* ui = NULL, uint16_t uilen = 0) throw(CustomException);
    //start dialog with new remote address
    void beginDialog(const SCCP_ADDRESS_T& remote_addr,
                     uint8_t* ui = NULL, uint16_t uilen = 0) throw(CustomException);
    void continueDialog(void) throw(CustomException);
    void endDialog(Dialog::Ending type = endBasic) throw(CustomException);

    // Transaction layer callbacks (indications)
    // uint16_t handleBeginDialog(); //NOTE: Unimplemented yet!
    uint16_t handleContinueDialog(bool compPresent);
    uint16_t handleEndDialog(bool compPresent);
    uint16_t handlePAbortDialog(uint8_t abortCause);
    uint16_t handleUAbort(uint16_t abortInfo_len, uint8_t *pAbortInfo,
                          uint16_t userInfo_len, uint8_t *pUserInfo);

    // Component layer callbacks (indications)
    uint16_t handleInvoke(uint8_t invokeId, uint8_t tag, uint16_t oplen,
                                const uint8_t *op, uint16_t pmlen, const uint8_t *pm,
                                bool lastComp);
    uint16_t handleResultLast(uint8_t invokeId, uint8_t tag, uint16_t oplen,
                                const uint8_t *op, uint16_t pmlen, const uint8_t *pm);
    uint16_t handleResultNotLast(uint8_t invokeId, uint8_t tag, uint16_t oplen,
                                const uint8_t *op, uint16_t pmlen, const uint8_t *pm);
    uint16_t handleResultError(uint8_t invokeId, uint8_t tag, uint16_t oplen,
                                const uint8_t *op, uint16_t pmlen,  const uint8_t *pm);
    uint16_t handleLCancelInvoke(uint8_t invokeId);
    uint16_t handleNoticeInd(uint8_t reportCause, TcapEntity::TCEntityKind comp_kind = TcapEntity::tceNone,
                             uint8_t invokeId = 0, uint16_t oplen = 0, const uint8_t *op = NULL);
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_DIALOG__ */

