#ident "$Id$"
// INAP cессия

#ifndef __SMSC_INMAN_INAP_SESSION__
#define __SMSC_INMAN_INAP_SESSION__

#include <map>

#include "ss7cp.h"

#include "inman/common/types.hpp"
#include "core/threads/Thread.hpp"
#include "core/network/Socket.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "inman/common/observable.hpp"

using smsc::core::network::Socket;
using smsc::inman::common::ObservableT;
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

class TcapDialog;

class SessionListener
{
public:
	virtual void onDialogBegin(TcapDialog*) = 0;
	virtual void onDialogEnd(TcapDialog*)   = 0;
};

class Session : public Thread, public ObservableT< SessionListener >
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

        virtual     TcapDialog*  openDialog(USHORT_T id);
        virtual     TcapDialog*  findDialog(USHORT_T id);
        virtual     void    	 closeDialog(TcapDialog* pDlg);
        virtual     void    	 closeAllDialogs();
        virtual 	void		 run();

    protected:
        typedef      std::map<USHORT_T, TcapDialog*> DialogsMap_T;

        Session(UCHAR_T ssn, const char* scfNum, const char* inmanNum, const char* host, int port);

        DialogsMap_T dialogs;
        Socket		 socket;

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
