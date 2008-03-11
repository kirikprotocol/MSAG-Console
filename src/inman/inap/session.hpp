#pragma ident "$Id$"
// SSNSession: TCAP dialogs/sessions factory (one per SSN)

#ifndef __SMSC_INMAN_INAP_SESSION__
#define __SMSC_INMAN_INAP_SESSION__

#include <map>
#include <list>

#include "logger/Logger.h"
using smsc::logger::Logger;

#include "core/synchronization/Event.hpp"
using smsc::core::synchronization::Event;

#include "inman/inap/TCDspDefs.hpp"
#include "inman/inap/SS7Types.hpp"

namespace smsc {
namespace inman {
namespace inap {

class Dialog;

class TCSessionAC;
class TCSessionSR;
class TCSessionMA;

struct TNoticeParms {
    uint16_t    relId;
    uint8_t     reportCause;

    TNoticeParms(uint16_t rel_id = 0, uint8_t report_cause = 0)
        : relId(rel_id), reportCause(report_cause)
    { }
};

/* ************************************************************************** *
 * class SSNSession (TCAP dialogs/sessions factory):
 * ************************************************************************** */
typedef enum { ssnIdle = 0, ssnBound, ssnError } SSNState;
typedef std::string TCSessionSUID;

class SSNSession: public Event {
private:
    typedef struct {
        struct timeval  tms;
        Dialog *        dlg;
    } DlgTime;

    typedef std::map<uint16_t, Dialog*>      DialogsMAP;
    typedef std::map<uint16_t, DlgTime>      DlgTimesMAP;
    typedef std::list<TCSessionAC*>          TCSessionsLIST;
    typedef std::map<std::string, TCSessionAC*> TCSessionsMAP;
    typedef std::map<uint16_t /*dlg_id*/, TNoticeParms> NoticedDLGs;

    DialogsMAP      dialogs;
    NoticedDLGs     ntcdDlgs;
    DlgTimesMAP     pending; //released but not terminated Dialogs with timestamp
    TCSessionsMAP   tcSessions;
    TCSessionsLIST  deadSess;

    uint8_t         _SSN;
    TCAPDispatcherITF * tcDsp;  //parent TCAP dispatcher
    uint16_t        msgUserId;  //Common part message port user id
    SSNState        state;
    uint16_t        maxId;
    uint16_t        minId;
    uint16_t        lastDlgId;
    uint16_t        lastTCSUId;
    Logger*         logger;

    void    dischargeDlg(Dialog * pDlg, const TCSessionSUID * tc_suid = 0);
    bool    nextDialogId(uint16_t & dId);
    void    cleanUpDialogs(void);
    Dialog* locateDialog(uint16_t dId);
    void    dumpDialogs(void);

protected:
    friend class TCAPDispatcher;
    SSNSession(TCAPDispatcherITF * use_dsp, uint8_t ssn_id, uint16_t msg_user_id,
               uint16_t max_dlg_id = 2000, uint16_t min_dlg_id = 1, Logger * uselog = NULL);
    ~SSNSession();

    inline void    setState(SSNState newState) { state = newState; }
    inline void    incMaxDlgs(uint16_t max_dlg_id) { if (max_dlg_id > maxId) maxId = max_dlg_id; }

//protected:
    friend class TCSessionAC;
    friend class TCSessionSR;
    friend class TCSessionMA;
    bool    getDialogId(uint16_t & dId);
    void    markDialog(Dialog * p_dlg);
    void    closeTCSession(TCSessionAC * p_sess);

public:
    inline SSNState    getState(void)  const { return state; }
    inline uint8_t     getSSN(void)    const { return _SSN; }
    inline uint16_t    getMsgUserId(void) const { return msgUserId; }

    // -- TCAP Sessions factory methods -- //
    //NOTE: fake_ssn is substituted in TCAP dialog originating address,
    //but sending still perfomed by behalf on SSN from owning SSNSession
    TCSessionMA * newMAsession(const char* own_addr, const EncodedOID & dlg_ac,
                                uint8_t rmt_ssn, uint8_t fake_ssn = 0);
    TCSessionMA * newMAsession(const TonNpiAddress & onpi, const EncodedOID & dlg_ac,
                                uint8_t rmt_ssn, uint8_t fake_ssn = 0);

    TCSessionSR * newSRsession(const char* own_addr, const EncodedOID & dlg_ac,
                                uint8_t rmt_ssn, const char* rmt_addr, uint8_t fake_ssn = 0);
    TCSessionSR * newSRsession(const TonNpiAddress & onpi, const EncodedOID & dlg_ac,
                                uint8_t rmt_ssn, const TonNpiAddress & rnpi, uint8_t fake_ssn = 0);

    std::string mkSignature(const TonNpiAddress & onpi, const EncodedOID & dlg_ac,
                            uint8_t rmt_ssn, const TonNpiAddress * rnpi) const;
    // -- TCAP Dialogs factory methods -- //
    Dialog* findDialog(uint16_t did);
    void    releaseDialog(uint16_t dId);
    void    releaseDialog(Dialog* pDlg, const TCSessionSUID * tc_suid = 0);
    void    releaseDialogs(const TCSessionSUID * tc_suid = 0);

    //Special methods used only by TCAP API Callbacks
    void    noticeInd(uint16_t dlg_id, uint16_t rel_id, uint8_t reportCause);
    bool    noticeParms(uint16_t dlg_id, TNoticeParms & parms);
};

/* ************************************************************************** *
 * Various TCAP Session classes (TCAP dialogs factory):
 * ************************************************************************** */

class TCSessionAC { //Base for all types of TC Sessions
protected:
    typedef std::list<Dialog*> TCDialogsLIST;

    Mutex           dlgGrd;
    SSNSession *    _owner;
    uint16_t        tcUID;
    TonNpiAddress   ownAdr;
    SCCP_ADDRESS_T  locAddr;
    uint8_t         senderSsn;
    TCDialogsLIST   pool;
    TCSessionSUID   sign;   //must be inited by successor
    const ROSComponentsFactory * acFab; //application constext operations factory

    Dialog * initDialog(const SCCP_ADDRESS_T & rmtAddr, Logger * use_log = NULL);

protected:
    friend class SSNSession;
    //NOTE: fake_ssn is substituted in TCAP dialog originating address,
    //but sending still perfomed by behalf on SSN from owning SSNSession
    TCSessionAC(uint16_t uid, SSNSession * owner, uint8_t fake_ssn,
                const TonNpiAddress & own_addr, const ROSComponentsFactory * use_fact);
    virtual ~TCSessionAC();

    void toPool(Dialog * p_dlg);

public:
    inline uint16_t     getUID(void)        const { return tcUID; }
    inline uint8_t      getSSNid(void)      const { return _owner->getSSN(); }
    inline SSNSession * getSSNSession(void) const { return _owner; }
    inline SSNState     getState(void)      const { return _owner->getState(); }

    inline const TonNpiAddress & getOwnAdr(void) const { return ownAdr; }
    inline const TCSessionSUID & Signature(void) const { return sign; }

    // -- TCAP Dialogs factory methods -- //
    void    releaseDialog(Dialog* pDlg);
    void    releaseDialogs(void);
    void    release(void);
};

//multiAddress TC session (opened with remote ssn specified, remote addr may vary)
class TCSessionMA: public TCSessionAC {
private:
    uint8_t     rmtSSN;

public:
    // -- TCAP Dialogs factory methods -- //
    Dialog* openDialog(const char* rmt_addr, Logger * use_log = NULL);
    Dialog* openDialog(const TonNpiAddress & rnpi, Logger * use_log = NULL);

protected:
    friend class SSNSession; 
    TCSessionMA(uint16_t uid, SSNSession * owner, uint8_t fake_ssn,
                const TonNpiAddress & own_addr, const ROSComponentsFactory * use_fact,
                uint8_t rmt_ssn);
    ~TCSessionMA()
    { }
};

//singleRoute TC session (opened with remote ssn & addr specified)
class TCSessionSR: public TCSessionAC {
private:
    TonNpiAddress   rmtNpi;
    SCCP_ADDRESS_T  rmtAddr;

public:
    // -- TCAP Dialogs factory methods -- //
    Dialog* openDialog(Logger * use_log = NULL);

protected:
    friend class SSNSession; 
    TCSessionSR(uint16_t uid, SSNSession * owner, uint8_t fake_ssn,
                const TonNpiAddress & own_addr, const ROSComponentsFactory * use_fact,
                uint8_t rmt_ssn, const TonNpiAddress & rmt_addr);
    ~TCSessionSR()
    { }
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_SESSION__ */

