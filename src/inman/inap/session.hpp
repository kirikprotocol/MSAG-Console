#ident "$Id$"
// INAP cессия

#ifndef __SMSC_INMAN_INAP_SESSION__
#define __SMSC_INMAN_INAP_SESSION__

#include <map>

#include "ss7cp.h"

#include "dispatcher.hpp"
#include "inman/common/types.hpp"
#include "inman/common/observable.hpp"
using smsc::inman::common::ObservableT;

namespace smsc {
namespace inman {
namespace inap {

static const USHORT_T MSG_RECV_TIMEOUT     = 1000;
static const USHORT_T MSG_INIT_MAX_ENTRIES = 512;
static const USHORT_T TCAP_DIALOG_MIN_ID   = 1;
static const USHORT_T TCAP_DIALOG_MAX_ID   = 1000;

class Dialog;

class SessionListener
{
public:
  virtual void onDialogBegin(Dialog*) = 0;
  virtual void onDialogEnd(Dialog*)   = 0;
};

class Session : public ObservableT< SessionListener >
{
        friend class InSessionFactory;
        friend class Dialog;

    public:
        typedef enum { IDLE, BOUNDED, ERROR } SessionState;

        SessionState getState() const;
        void         setState(SessionState newState);

        UCHAR_T      getSSN() const;
        //sets default APPLICATION-CONTEXT index for dialogs, see acdefs.hpp
        void         setDialogsAC(const unsigned dialog_ac_idx);

        // register dialog in session, it's ad hoc method for using
        // Session functionality for Dialog successors.
        // NOTE: forcedly sets dialogId
        virtual     Dialog*  registerDialog(Dialog* pDlg);

        virtual     Dialog*  openDialog();
        virtual     Dialog*  openDialog(const unsigned dialog_ac_idx);
        virtual     Dialog*  findDialog(USHORT_T id);
        virtual     void     closeDialog(Dialog* pDlg);
        virtual     void     closeAllDialogs();

    protected:
        Dialog* registerDialog(Dialog* pDlg, USHORT_T id);
        typedef     std::map<USHORT_T, Dialog*> DialogsMap_T;

        Session(UCHAR_T ownssn, const char* own, const char* remote);
        Session(UCHAR_T    ownssn, const char*    ownaddr,
                UCHAR_T remotessn, const char* remoteaddr);
        Session(UCHAR_T   userssn,
                UCHAR_T    ownssn, const char*    ownaddr,
                UCHAR_T remotessn, const char* remoteaddr);
        virtual       ~Session();
        USHORT_T        nextDialogId();

        SCCP_ADDRESS_T  ssfAddr;
        SCCP_ADDRESS_T  scfAddr;
        unsigned        _ac_idx; //default APPLICATION-CONTEXT index for dialogs, see acdefs.hpp
        DialogsMap_T    dialogs;
        SessionState    state;
        UCHAR_T         SSN;
        USHORT_T        lastDialogId;
        Logger*       logger;
};

}
}
}

#endif
