#ident "$Id$"
// Session: TCAP dialogs factory

#ifndef __SMSC_INMAN_INAP_SESSION__
#define __SMSC_INMAN_INAP_SESSION__

#include <map>

#include "inman/common/types.hpp"
#include "inman/common/observable.hpp"
#include "logger/Logger.h"

using smsc::logger::Logger;
using smsc::inman::common::ObservableT;

namespace smsc {
namespace inman {
namespace inap {

static const USHORT_T MSG_RECV_TIMEOUT     = 1000;

static const USHORT_T TCAP_DIALOG_MIN_ID   = 1;
static const USHORT_T TCAP_DIALOG_MAX_ID   = 1000;

class Dialog;

class Session {
public:
    typedef enum { IDLE, BOUND, ERROR } SessionState;

    Session(UCHAR_T ownssn, const char* ownaddr,
           UCHAR_T remotessn, const char* remoteaddr, Logger * uselog = NULL);

    Session(UCHAR_T ownssn, const char* ownadr,
            const char* remoteadr, Logger * uselog = NULL);
    ~Session();

    UCHAR_T      getSSN(void) const { return SSN; }
    SessionState getState(void) const { return state; }
    void         getRoute(SCCP_ADDRESS_T & ownAddr, SCCP_ADDRESS_T & rmtAddr) const;
    USHORT_T     nextDialogId(void);
    //sets default APPLICATION-CONTEXT index for dialogs, see acdefs.hpp
    void         setDialogsAC(const unsigned dialog_ac_idx);

    /* TCAP Dialogs factory methods */
    Dialog*  openDialog(void);
    Dialog*  openDialog(const unsigned dialog_ac_idx);
    Dialog*  findDialog(USHORT_T id);
    void     releaseDialog(Dialog* pDlg);
    void     releaseDialogs(void);

protected:
    friend class TCAPDispatcher;
    void    setState(SessionState newState);

private:
    typedef std::map<USHORT_T, Dialog*> DialogsMap_T;
    typedef std::list<Dialog*> DialogsLIST;

    void cleanUpDialogs(void);

    Mutex           dlgGrd;
    DialogsMap_T    dialogs;
    DialogsLIST     pool;
    DialogsMap_T    pending; //released Dialogs, which have Invokes pending

    UCHAR_T         SSN;
    SCCP_ADDRESS_T  ssfAddr;
    SCCP_ADDRESS_T  scfAddr;
    unsigned        _ac_idx; //default APPLICATION-CONTEXT index for dialogs, see acdefs.hpp

    SessionState    state;
    USHORT_T        lastDialogId;
    Logger*         logger;

    // register dialog in session, it's ad hoc method for using
    // Session functionality for Dialog successors.
    // NOTE: forcedly sets dialogId
    //Dialog*  registerDialog(Dialog* pDlg);
};
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_SESSION__ */

