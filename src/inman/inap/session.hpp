#ident "$Id$"
// INAP cессия

#ifndef __SMSC_INMAN_INAP_SESSION__
#define __SMSC_INMAN_INAP_SESSION__

#include <map>

#include "ss7cp.h"

#include "inman/inap/types.hpp"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"

using smsc::core::threads::Thread;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;

namespace smsc {
namespace inman {
namespace inap {

static const USHORT_T MSG_RECV_TIMEOUT     = 1000;
static const USHORT_T MSG_INIT_MAX_ENTRIES = 512;
static const USHORT_T TCAP_DIALOG_MIN_ID   = 1;
static const USHORT_T TCAP_DIALOG_MAX_ID   = 1000;

class Dialog;

class Session : public Thread
{
        friend class Factory;

    public:
        typedef enum { IDLE, BOUNDED, ERROR } State_T;

        State_T     getState() const;
        void        setState(State_T newState);
        UCHAR_T     getSSN() const;

        SCCP_ADDRESS_T scfAddr;
        SCCP_ADDRESS_T inmanAddr;
        APP_CONTEXT_T  ac;

        virtual     Dialog*  openDialog(USHORT_T id);
        virtual     Dialog*  findDialog(USHORT_T id);
        virtual     void     closeDialog(Dialog* pDlg);
        virtual     void     closeAllDialogs();

    protected:
        typedef      std::map<USHORT_T, Dialog*> DialogsMap_T;

        Session(UCHAR_T SSN);
        Session(UCHAR_T ssn, const char* scfNum, const char* inmanNum);
        DialogsMap_T dialogs;

        virtual     ~Session();
        virtual int  Execute();

        USHORT_T      nextDialogId();
        State_T       state;
        UCHAR_T       SSN;
        USHORT_T      lastDialogId;
        Event         started;
        Event         stopped;
        Mutex         lock;
        volatile BOOLEAN_T running;
};

}
}
}

#endif
