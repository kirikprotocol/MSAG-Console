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

class Dialog;

class SSNSession {
public:
    typedef enum { ssnIdle = 0, ssnBound, ssnError } SSNState;

    SSNState getState(void) const { return state; }
    const char* getOwnAdr(void) const { return ownAdr.c_str(); }

    /* TCAP Dialogs factory methods */
    Dialog* openDialog(void);
    Dialog* findDialog(USHORT_T did);
    void    releaseDialog(Dialog* pDlg);
    void    releaseDialogs(void);

protected:
    friend class TCAPDispatcher;
    SSNSession(UCHAR_T ownssn, USHORT_T user_id, Logger * uselog = NULL);
    ~SSNSession();

    void    setState(SSNState newState) { state = newState; }
    void    init(const char* own_addr, UCHAR_T rmt_ssn,
                const char* rmt_addr, ACOID::DefinedOIDidx dialog_ac_idx,
                USHORT_T max_id = 2000, USHORT_T min_id = 1);

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
    USHORT_T        userId;
    USHORT_T        maxId;
    USHORT_T        minId;
    std::string     ownAdr;
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

