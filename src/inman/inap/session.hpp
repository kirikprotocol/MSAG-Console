#ident "$Id$"
// SSNSession: TCAP dialogs/sessions factory (one per SSN)

#ifndef __SMSC_INMAN_INAP_SESSION__
#define __SMSC_INMAN_INAP_SESSION__

#include <map>
#include <list>

#include "core/synchronization/Event.hpp"
using smsc::core::synchronization::Event;

#include "inman/common/TimeOps.hpp"
#include "inman/comp/acdefs.hpp"
using smsc::ac::ACOID;

#include "inman/common/adrutil.hpp"

#include "logger/Logger.h"
using smsc::logger::Logger;

namespace smsc {
namespace inman {
namespace inap {

class Dialog;

class TCSessionAC;
class TCSessionSR;
class TCSessionMR;
class TCSessionMA;

struct TNoticeParms {
    USHORT_T    relId;
    UCHAR_T     reportCause;

    TNoticeParms(USHORT_T rel_id = 0, UCHAR_T report_cause = 0)
        : relId(rel_id), reportCause(report_cause)
    { }
};

/* ************************************************************************** *
 * class SSNSession (TCAP dialogs/sessions factory):
 * ************************************************************************** */
typedef enum { ssnIdle = 0, ssnBound, ssnError } SSNState;
typedef std::string TCSessionSUID;

class SSNSession: public Event {
public:
    SSNState    getState(void)  const { return state; }
    UCHAR_T     getSSN(void)    const { return _SSN; }
    USHORT_T    getMsgUserId(void) const { return msgUserId; }

    // -- TCAP Sessions factory methods -- //
    //NOTE: fake_ssn is substituted in TCAP dialog originating address,
    //but sending still perfomed by behalf on SSN from owning SSNSession
    TCSessionMR * newMRsession(const char* own_addr, ACOID::DefinedOIDidx dlg_ac_idx,
                               UCHAR_T fake_ssn = 0);
    TCSessionMR * newMRsession(const TonNpiAddress & onpi, ACOID::DefinedOIDidx dlg_ac_idx,
                               UCHAR_T fake_ssn = 0);

    TCSessionMA * newMAsession(const char* own_addr, ACOID::DefinedOIDidx dlg_ac_idx,
                                UCHAR_T rmt_ssn, UCHAR_T fake_ssn = 0);
    TCSessionMA * newMAsession(const TonNpiAddress & onpi, ACOID::DefinedOIDidx dlg_ac_idx,
                                UCHAR_T rmt_ssn, UCHAR_T fake_ssn = 0);

    TCSessionSR * newSRsession(const char* own_addr, ACOID::DefinedOIDidx dlg_ac_idx,
                                UCHAR_T rmt_ssn, const char* rmt_addr, UCHAR_T fake_ssn = 0);
    TCSessionSR * newSRsession(const TonNpiAddress & onpi, ACOID::DefinedOIDidx dlg_ac_idx,
                                UCHAR_T rmt_ssn, const TonNpiAddress & rnpi, UCHAR_T fake_ssn = 0);
    
    // -- TCAP Dialogs factory methods -- //
    Dialog* findDialog(USHORT_T did);
    void    releaseDialog(USHORT_T dId);
    void    releaseDialog(Dialog* pDlg, const TCSessionSUID * tc_suid = 0);
    void    releaseDialogs(const TCSessionSUID * tc_suid = 0);

    //Special methods used only by TCAP API Callbacks
    void    noticeInd(USHORT_T dlg_id, USHORT_T rel_id, UCHAR_T reportCause);
    bool    noticeParms(USHORT_T dlg_id, TNoticeParms & parms);

protected:
    friend class TCAPDispatcher;
    SSNSession(UCHAR_T ssn_id, USHORT_T msg_user_id, USHORT_T max_dlg_id = 2000,
               USHORT_T min_dlg_id = 1, Logger * uselog = NULL);
    ~SSNSession();

    void    setState(SSNState newState) { state = newState; }
    void    incMaxDlgs(USHORT_T max_dlg_id) { if (max_dlg_id > maxId) maxId = max_dlg_id; }

protected:
    friend class TCSessionAC;
    friend class TCSessionSR;
    friend class TCSessionMR;
    friend class TCSessionMA;
    bool    getDialogId(USHORT_T & dId);
    void    markDialog(Dialog * p_dlg);
    void    closeTCSession(TCSessionAC * p_sess);

private:
    typedef struct {
        struct timeval  tms;
        Dialog *        dlg;
    } DlgTime;

    typedef std::map<USHORT_T, Dialog*>      DialogsMAP;
    typedef std::map<USHORT_T, DlgTime>      DlgTimesMAP;
    typedef std::list<TCSessionAC*>          TCSessionsLIST;
    typedef std::map<std::string, TCSessionAC*> TCSessionsMAP;
    typedef std::map<USHORT_T /*dlg_id*/, TNoticeParms> NoticedDLGs;

    void    dischargeDlg(Dialog * pDlg, const TCSessionSUID * tc_suid = 0);
    bool    nextDialogId(USHORT_T & dId);
    void    cleanUpDialogs(void);
    Dialog* locateDialog(USHORT_T dId);
    void    dumpDialogs(void);

    DialogsMAP      dialogs;
    NoticedDLGs     ntcdDlgs;
    DlgTimesMAP     pending; //released but not terminated Dialogs with timestamp
    TCSessionsMAP   tcSessions;
    TCSessionsLIST  deadSess;

    USHORT_T        msgUserId;   //Common part message port user id
    UCHAR_T         _SSN;
    SSNState        state;
    USHORT_T        maxId;
    USHORT_T        minId;
    USHORT_T        lastDlgId;
    USHORT_T        lastTCSUId;
    Logger*         logger;
};

/* ************************************************************************** *
 * Various TCAP Session classes (TCAP dialogs factory):
 * ************************************************************************** */

class TCSessionAC { //Base for all types of TC Sessions
public:
    USHORT_T     getUID(void)        const { return tcUID; }
    UCHAR_T      getSSNid(void)      const { return _owner->getSSN(); }
    SSNSession * getSSNSession(void) const { return _owner; }
    SSNState     getState(void)      const { return _owner->getState(); }
    const TonNpiAddress & getOwnAdr(void) const { return ownAdr; }
    const TCSessionSUID & Signature(void)   const { return sign; }

    static void mkSignature(std::string & sid, UCHAR_T own_ssn, const TonNpiAddress & onpi,
                             ACOID::DefinedOIDidx dlg_ac_idx,
                             UCHAR_T rmt_ssn, const TonNpiAddress * rnpi);

    // -- TCAP Dialogs factory methods -- //
    void    releaseDialog(Dialog* pDlg);
    void    releaseDialogs(void);
    void    release(void);

protected:
    friend class SSNSession;
    //NOTE: fake_ssn is substituted in TCAP dialog originating address,
    //but sending still perfomed by behalf on SSN from owning SSNSession
    TCSessionAC(USHORT_T uid, SSNSession * owner, UCHAR_T fake_ssn,
                const TonNpiAddress & own_addr, ACOID::DefinedOIDidx dlg_ac_idx);
    virtual ~TCSessionAC();

    void toPool(Dialog * p_dlg);

protected:
    typedef std::list<Dialog*> TCDialogsLIST;

    Dialog * initDialog(const SCCP_ADDRESS_T & rmtAddr);

    SSNSession *    _owner;
    USHORT_T        tcUID;
    TonNpiAddress   ownAdr;
    SCCP_ADDRESS_T  locAddr;
    UCHAR_T         senderSsn;
    ACOID::DefinedOIDidx  ac_idx; //default APPLICATION-CONTEXT index for dialogs
    Mutex           dlgGrd;
    TCDialogsLIST   pool;
    TCSessionSUID   sign;
};

//multiRoute TC session (both remote ssn & addr may vary)
class TCSessionMR: public TCSessionAC {
public:
    // -- TCAP Dialogs factory methods -- //
    Dialog* openDialog(UCHAR_T rmt_ssn, const char* rmt_addr);

protected:
    friend class SSNSession; 
    TCSessionMR(USHORT_T uid, SSNSession * owner, UCHAR_T fake_ssn,
                const TonNpiAddress & own_addr, ACOID::DefinedOIDidx dlg_ac_idx);
    ~TCSessionMR() { }
};

//multiAddress TC session (opened with remote ssn specified, remote addr may vary)
class TCSessionMA: public TCSessionAC {
public:
    // -- TCAP Dialogs factory methods -- //
    Dialog* openDialog(const char* rmt_addr);
    Dialog* openDialog(const TonNpiAddress & rnpi);

protected:
    friend class SSNSession; 
    TCSessionMA(USHORT_T uid, SSNSession * owner, UCHAR_T fake_ssn,
                const TonNpiAddress & own_addr, ACOID::DefinedOIDidx dlg_ac_idx,
                UCHAR_T rmt_ssn);
    ~TCSessionMA() { }

private:
    UCHAR_T     rmtSSN;
};

//singleRoute TC session (opened with remote ssn & addr specified)
class TCSessionSR: public TCSessionAC {
public:
    // -- TCAP Dialogs factory methods -- //
    Dialog* openDialog(void);

protected:
    friend class SSNSession; 
    TCSessionSR(USHORT_T uid, SSNSession * owner, UCHAR_T fake_ssn,
                const TonNpiAddress & own_addr, ACOID::DefinedOIDidx dlg_ac_idx,
                UCHAR_T rmt_ssn, const TonNpiAddress & rmt_addr);
    ~TCSessionSR() { }

private:
    TonNpiAddress   rmtNpi;
    SCCP_ADDRESS_T  rmtAddr;
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_SESSION__ */

