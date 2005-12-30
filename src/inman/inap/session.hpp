#ident "$Id$"
// INAP cессия

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

class SessionListener
{
public:
  virtual void onDialogBegin(Dialog*) = 0;
  virtual void onDialogEnd(Dialog*)   = 0;
};

class Session : public ObservableT< SessionListener >
{
friend class TCAPDispatcher;
friend class Dialog;

public:
    typedef enum { IDLE, BOUND, ERROR } SessionState;

    Session(UCHAR_T ownssn, const char* ownaddr,
           UCHAR_T remotessn, const char* remoteaddr, Logger * uselog = NULL);

    Session(UCHAR_T ownssn, const char* ownadr,
            const char* remoteadr, Logger * uselog = NULL);
    ~Session();

    UCHAR_T      getSSN(void) const;
    SessionState getState(void) const;
    USHORT_T     nextDialogId(void);
    //sets default APPLICATION-CONTEXT index for dialogs, see acdefs.hpp
    void    setDialogsAC(const unsigned dialog_ac_idx);
    // register dialog in session, it's ad hoc method for using
    // Session functionality for Dialog successors.
    // NOTE: forcedly sets dialogId
    Dialog*  registerDialog(Dialog* pDlg);

    /* TCAP Dialogs factory methods */
    Dialog*  openDialog(void);
    Dialog*  openDialog(const unsigned dialog_ac_idx);
    Dialog*  findDialog(USHORT_T id);
    void     closeDialog(Dialog* pDlg);
    void     closeAllDialogs(void);

protected:
    typedef std::map<USHORT_T, Dialog*> DialogsMap_T;

    //NOTE: do not use this method manually
    void    setState(SessionState newState);
    Dialog* registerDialog(Dialog* pDlg, USHORT_T id);

    SCCP_ADDRESS_T  ssfAddr;
    SCCP_ADDRESS_T  scfAddr;
    unsigned        _ac_idx; //default APPLICATION-CONTEXT index for dialogs, see acdefs.hpp
    DialogsMap_T    dialogs;
    SessionState    state;
    UCHAR_T         SSN;
    USHORT_T        lastDialogId;
    Logger*         logger;
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_SESSION__ */

