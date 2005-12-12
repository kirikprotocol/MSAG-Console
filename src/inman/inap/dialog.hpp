#ident "$Id$"
//TCAP dialog implementation

#ifndef __SMSC_INMAN_INAP_DIALOG__
#define __SMSC_INMAN_INAP_DIALOG__

#include <map>

#include "logger/Logger.h"
#include "inman/common/types.hpp"
#include "inman/inap/invoke.hpp"
#include "inman/inap/results.hpp"
#include "inman/common/observable.hpp"

using smsc::inman::common::ObservableT;
using smsc::logger::Logger;

namespace smsc {
namespace inman {
namespace inap {

class Session;

typedef std::map< UCHAR_T, Invoke* > InvokeMap;

class DialogListener
{
  public:
    virtual void onDialogInvoke(Invoke*, bool lastComp) = 0;
    virtual void onDialogContinue(bool compPresent) = 0;
    virtual void onDialogPAbort(UCHAR_T abortCause) = 0;
    virtual void onDialogREnd(bool compPresent) = 0;
};

static const USHORT_T   _DEFAULT_INVOKE_TIMER = 30; //seconds

class Dialog : public ObservableT< DialogListener >
{
public:
    typedef enum {
        dlgIdle = 0, dlgInited, dlgContinued, dlgConfirmed, dlgEnded
    } DialogState;

    Dialog(Session* session, USHORT_T id, unsigned dialog_ac_idx, Logger * uselog = NULL);
    virtual ~Dialog();

    //sets the default timeout for Invoke result waiting
    void setInvokeTimeout(USHORT_T timeout);

    //creates and registers Invoke, sets its response waiting timeout,
    //zero as timeout value sets Invoke timer equal to the Dialog default timeout value
    Invoke* initInvoke(UCHAR_T opcode, USHORT_T timeout = 0);
    void    releaseInvoke(UCHAR_T invId);
    //resets the Invoke timer, extending its lifetime (response waiting)
    void    resetInvokeTimer(UCHAR_T invokeId);

    void    sendInvoke(Invoke * inv);
    void    sendResultLast(InvokeResultLast* res);
    void    sendResultNotLast(InvokeResultNotLast* res);
    void    sendResultError(InvokeResultError* res);

    // Transaction layer
    void beginDialog(UCHAR_T* ui = NULL, USHORT_T uilen = 0);
    //start dialog with new remote address
    void beginDialog(const SCCP_ADDRESS_T& remote_addr, UCHAR_T* ui = NULL, USHORT_T uilen = 0);
    void continueDialog();
    void endDialog(USHORT_T termination);

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

    //own methods
    void     setId(USHORT_T did){ _dId = did; } //do not use this manually !
    USHORT_T getId()      const { return _dId;       }
    UCHAR_T  getNextInvokeId()  { return _lastInvId++;}
    USHORT_T getQSrv()    const { return qSrvc;     }
    USHORT_T getTimeout() const { return _timeout;  }
    Session* getSession() const { return session;   }

protected:
    InvokeMap       originating; //Invokes, which have result/errors defined
    InvokeMap       terminating; //
    Session*        session;
    SCCP_ADDRESS_T  ownAddr;
    SCCP_ADDRESS_T  remoteAddr;
    UCHAR_T         dSSN;   //SubSystemNumber dialog uses
    unsigned        _ac_idx; //ApplicationContext index, see acdefs.hpp
    APP_CONTEXT_T   ac;
    USHORT_T        _dId;
    USHORT_T        _timeout;
    UCHAR_T         qSrvc;
    UCHAR_T         priority;
    UCHAR_T         _lastInvId;
    Logger*         logger;
    DialogState     _state;
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_DIALOG__ */

