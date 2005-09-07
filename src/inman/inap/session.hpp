#ident "$Id$"
// INAP cессия

#ifndef __SMSC_INMAN_INAP_SESSION__
#define __SMSC_INMAN_INAP_SESSION__

#include <map>

#include "ss7cp.h"

#include "inman/common/types.hpp"
#include "inman/common/observable.hpp"

#include "dispatcher.hpp"

using smsc::inman::common::ObservableT;

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

class Session : public ObservableT< SessionListener >
{
        friend class Factory;
        friend class TcapDialog;

    public:
        typedef enum { IDLE, BOUNDED, ERROR } SessionState;

        SessionState getState() const;
        void         setState(SessionState newState);

        UCHAR_T      getSSN() const;


        virtual     TcapDialog*  openDialog(USHORT_T id);
        virtual     TcapDialog*  findDialog(USHORT_T id);
        virtual     void    	 closeDialog(TcapDialog* pDlg);
        virtual     void    	 closeAllDialogs();

    protected:
        typedef     std::map<USHORT_T, TcapDialog*> DialogsMap_T;

        Session(UCHAR_T ssn, const char* scfNum, const char* inmanNum);
        virtual     	~Session();
        USHORT_T      	nextDialogId();

        SCCP_ADDRESS_T 	ssfAddr;
        SCCP_ADDRESS_T 	scfAddr;
        APP_CONTEXT_T  	ac;
        DialogsMap_T   	dialogs;
        SessionState   	state;
        UCHAR_T        	SSN;
        USHORT_T       	lastDialogId;
        Logger*		   	logger;
};

}
}
}

#endif
