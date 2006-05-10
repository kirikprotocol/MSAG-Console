#ident "$Id$"
// Session: TCAP dialogs factory, one per SSN

#ifndef __SMSC_INMAN_INAP_SESSION__
#define __SMSC_INMAN_INAP_SESSION__

#include <map>
#include <list>

#include "inman/common/TimeOps.hpp"
#include "inman/common/types.hpp"
#include "logger/Logger.h"
#include "inman/comp/acdefs.hpp"
using smsc::ac::ACOID;
using smsc::logger::Logger;

namespace smsc {
namespace inman {
namespace inap {

static const USHORT_T TCAP_DIALOG_MIN_ID   = 1;
static const USHORT_T TCAP_DIALOG_MAX_ID   = 1000;

class Dialog;

class SSNSession {
public:
    typedef enum { ssnIdle = 0, ssnBound, ssnError } SSNState;

    SSNSession(UCHAR_T ownssn, Logger * uselog = NULL);
    ~SSNSession();

    void    init(const char* own_addr, /*UCHAR_T rmt_ssn,*/
                    const char* rmt_addr, ACOID::DefinedOIDidx dialog_ac_idx);

    SSNState getState(void) const { return state; }

    /* TCAP Dialogs factory methods */
    Dialog* openDialog(void);
//    Dialog* openDialog(const char* own_addr, /*UCHAR_T rmt_ssn,*/
//                        const char* rmt_addr, unsigned dialog_ac_idx);
    Dialog* findDialog(USHORT_T did);
    void    releaseDialog(Dialog* pDlg);
    void    releaseDialogs(void);

protected:
    friend class TCAPDispatcher;
    void    setState(SSNState newState) { state = newState; }

private:
    typedef struct {
        struct timeval tms;
        Dialog *       dlg;
    } DlgTime;

    typedef std::map<USHORT_T, DlgTime> DlgTimesMap_T;
    typedef std::map<USHORT_T, Dialog*> DialogsMap_T;
    typedef std::list<Dialog*> DialogsLIST;

    bool    nextDialogId(USHORT_T & dId);
    void    cleanUpDialogs(void);
    Dialog* locateDialog(USHORT_T dId);
    void    dumpDialogs(void);

    Mutex           dlgGrd;
    DialogsMap_T    dialogs;
    DialogsLIST     pool;
    DlgTimesMap_T   pending; //released but not terminated Dialogs with timestamp

    UCHAR_T         SSN;
    SCCP_ADDRESS_T  locAddr;
    SCCP_ADDRESS_T  rmtAddr;
    ACOID::DefinedOIDidx  ac_idx; //default APPLICATION-CONTEXT index for dialogs, see acdefs.hpp

    SSNState        state;
    USHORT_T        lastDlgId;
    Logger*         logger;
};
} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_SESSION__ */

