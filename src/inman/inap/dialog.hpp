#ident "$Id$"
// Диалог (транзакция) IN

#ifndef __SMSC_INMAN_INAP_DIALOG__
#define __SMSC_INMAN_INAP_DIALOG__

#include <list>
#include <vector>
#include <map>

#include "ss7cp.h"

#include "logger/Logger.h"
#include "inman/comp/comps.hpp"
#include "invoke.hpp"
#include "results.hpp"

using smsc::logger::Logger;

namespace smsc {
namespace inman {
namespace inap {

class Session;

typedef std::map< UCHAR_T, Invoke* > InvokeMap;

class TcapDialog;

class TcapDialogListener
{
	public:
		virtual void onDialogInvoke( TcapDialog*, Invoke*  ) = 0;
};

class TcapDialog : public ObservableT< TcapDialogListener > 
{
    friend class Session;

  public:

    TcapDialog(Session* session, USHORT_T id);
    virtual ~TcapDialog();

    virtual Invoke* invoke(UCHAR_T opcode);

    // Transaction layer
    virtual void beginDialog();//called by client of this dialog instance
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
   	USHORT_T getId()      const { return did;     	}
   	USHORT_T getNextInvokeId()  { return invokeId++;}
   	USHORT_T getQSrv()    const { return qSrvc;     }
   	USHORT_T getTimeout() const { return timeout;   }
   	Session* getSession() const { return session;   }

   protected:

    InvokeMap	  	  originating;
    InvokeMap	  	  terminating;
    Session*          session;
    USHORT_T          opid;
    USHORT_T          did;
    USHORT_T          timeout;
    UCHAR_T           qSrvc;
    UCHAR_T           priority;
    UCHAR_T           acShort;
    UCHAR_T			  invokeId;
    Logger*			  logger;
};

}
}
}

#endif
