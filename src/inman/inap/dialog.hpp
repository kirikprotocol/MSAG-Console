#ident "$Id$"
// Диалог (транзакция) IN

#ifndef __SMSC_INMAN_INAP_DIALOG__
#define __SMSC_INMAN_INAP_DIALOG__

#include <list>
#include <vector>
#include <map>

#include "ss7cp.h"

#include "logger/Logger.h"
#include "inman/common/types.hpp"
#include "invoke.hpp"
#include "results.hpp"

using smsc::logger::Logger;

namespace smsc {
namespace inman {
namespace inap {

class Session;

typedef std::map< UCHAR_T, Invoke* > InvokeMap;

class Dialog;

class DialogListener
{
  public:
    virtual void onDialogInvoke(  Invoke*  ) = 0;
};

class Dialog : public ObservableT< DialogListener >
{
    friend class Session;

  public:

    Dialog(Session* session, USHORT_T id, unsigned dialog_ac_idx );
    virtual ~Dialog();

    virtual Invoke* invoke(UCHAR_T opcode);

    // Transaction layer
    virtual void beginDialog(const SCCP_ADDRESS_T& remote_addr, UCHAR_T* ui, USHORT_T uilen);
    virtual void beginDialog(const SCCP_ADDRESS_T& remote_addr);
    //called by client of this dialog instance
    virtual void beginDialog();
    virtual void beginDialog(UCHAR_T* ui, USHORT_T uilen);
    virtual void continueDialog();
    virtual void endDialog(USHORT_T termination);
    virtual void timerReset();

    // Transaction level callbacks
  virtual USHORT_T handleBeginDialog();
  virtual USHORT_T handleContinueDialog();
  virtual USHORT_T handleEndDialog();

    // Interaction level callbacks
    virtual USHORT_T handleInvoke(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm);
    virtual USHORT_T handleResultLast(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm);
    virtual USHORT_T handleResultNotLast(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen, const UCHAR_T *pm);
    virtual USHORT_T handleUserError(UCHAR_T invokeId, UCHAR_T tag, USHORT_T oplen, const UCHAR_T *op, USHORT_T pmlen,  const UCHAR_T *pm);

   public:
    USHORT_T getId()      const { return did;       }
    void     setId(USHORT_T id) { did = id;         }
    USHORT_T getNextInvokeId()  { return invokeId++;}
    USHORT_T getQSrv()    const { return qSrvc;     }
    USHORT_T getTimeout() const { return timeout;   }
    Session* getSession() const { return session;   }

   protected:

    InvokeMap       originating;
    InvokeMap       terminating;
    Session*        session;
    SCCP_ADDRESS_T  ownAddr;
    SCCP_ADDRESS_T  remoteAddr;
    unsigned        _ac_idx; //ApplicationContext index, see acdefs.hpp
    APP_CONTEXT_T   ac;
    USHORT_T        opid;
    USHORT_T        did;
    USHORT_T        timeout;
    UCHAR_T         qSrvc;
    UCHAR_T         priority;
    UCHAR_T         invokeId;
    Logger*         logger;
};

}
}
}

#endif
