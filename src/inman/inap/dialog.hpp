#ident "$Id$"
//TCAP dialog implementation

#ifndef __SMSC_INMAN_INAP_DIALOG__
#define __SMSC_INMAN_INAP_DIALOG__

#include <map>
#include <list>

#include "logger/Logger.h"
#include "inman/common/types.hpp"
#include "inman/inap/invoke.hpp"
#include "inman/inap/results.hpp"
#include "inman/common/errors.hpp"
#include "inman/comp/acdefs.hpp"
using smsc::ac::ACOID;
using smsc::logger::Logger;
using smsc::inman::common::CustomException;



namespace smsc {
namespace inman {
namespace inap {

class DialogListener
{
  public:
    virtual void onDialogInvoke(Invoke*, bool lastComp) = 0;
    virtual void onDialogContinue(bool compPresent) = 0;
    virtual void onDialogPAbort(UCHAR_T abortCause) = 0;
    virtual void onDialogREnd(bool compPresent) = 0;
};

static const USHORT_T   _DEFAULT_INVOKE_TIMER = 30; //seconds

class Dialog {
public:
    typedef enum {
        dlgIdle = 0, dlgInited, dlgContinued, dlgConfirmed, dlgEnded
    } DialogState;

    Dialog(USHORT_T dlgId, ACOID::DefinedOIDidx dialog_ac_idx, const SCCP_ADDRESS_T & loc_addr,
           const SCCP_ADDRESS_T & rmt_addr, Logger * uselog = NULL);
    virtual ~Dialog();
    //reinitializes Dialog to be reused with other id
    void reset(USHORT_T new_id);

    void addListener(DialogListener* pListener);
    void removeListener(DialogListener* pListener);

    //sets the default timeout for Invoke result waiting
    void    setInvokeTimeout(USHORT_T timeout);

    //creates and registers Invoke, sets its response waiting timeout,
    //zero as timeout value sets Invoke timer equal to the Dialog default timeout value
    Invoke* initInvoke(UCHAR_T opcode, USHORT_T timeout = 0);
    void    releaseInvoke(UCHAR_T invId);
    //returns true is dialog has some Invokes pending (awaiting Result or LCancel)
    unsigned  pendingInvokes(void);
    //resets the Invoke timer, extending its lifetime (response waiting)
    void    resetInvokeTimer(UCHAR_T invokeId) throw (CustomException);

    void    sendInvoke(Invoke * inv) throw(CustomException);
    void    sendResultLast(InvokeResultLast* res) throw(CustomException);
    void    sendResultNotLast(InvokeResultNotLast* res) throw(CustomException);
    void    sendResultError(InvokeResultError* res) throw(CustomException);

    // Transaction layer
    void beginDialog(UCHAR_T* ui = NULL, USHORT_T uilen = 0) throw(CustomException);
    //start dialog with new remote address
    void beginDialog(const SCCP_ADDRESS_T& remote_addr,
                     UCHAR_T* ui = NULL, USHORT_T uilen = 0) throw(CustomException);
    void continueDialog() throw(CustomException);
    void endDialog(USHORT_T termination) throw(CustomException);

    // Transaction level callbacks
    USHORT_T handleBeginDialog();
    USHORT_T handleContinueDialog(bool compPresent);
    USHORT_T handleEndDialog(bool compPresent);
    USHORT_T handlePAbortDialog(UCHAR_T abortCause);

    // Interaction level callbacks
    USHORT_T handleInvoke(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen,
                                const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm,
                                bool lastComp);
    USHORT_T handleLCancelInvoke(UCHAR_T invokeId);
    USHORT_T handleResultLast(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen,
                                const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm);
    USHORT_T handleResultNotLast(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen,
                                const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm);
    USHORT_T handleUserError(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen,
                                const UCHAR_T *op, USHORT_T pmlen,  const UCHAR_T *pm);

    
    USHORT_T getId(void)      const { return _dId;     }
    USHORT_T getTimeout(void) const { return _timeout; }

protected:
    void checkSS7res(const char * descr, USHORT_T result) throw(CustomException);
    UCHAR_T  getNextInvokeId(void)  { return _lastInvId++; }

    typedef std::list<DialogListener*> ListenerList;
    typedef std::map<UCHAR_T, Invoke*> InvokeMap;

    Mutex           invGrd;      //invokes and listeners guard
    InvokeMap       originating; //Invokes, which have result/errors defined
    InvokeMap       terminating; //
    ListenerList    listeners;   //

    SCCP_ADDRESS_T  ownAddr;
    SCCP_ADDRESS_T  rmtAddr;
    UCHAR_T         dSSN;   //SubSystemNumber dialog uses
    APP_CONTEXT_T   ac;
    USHORT_T        _dId;
    USHORT_T        _timeout;
    UCHAR_T         qSrvc;
    UCHAR_T         priority;
    UCHAR_T         _lastInvId;
    Logger*         logger;
    DialogState     _state;
    ACOID::DefinedOIDidx   _ac_idx; //ApplicationContext index, see acdefs.hpp
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_DIALOG__ */

