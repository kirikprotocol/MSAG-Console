/* ************************************************************************** *
 * SubSystem Session that supports HD version of TCAP BE
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INAP_SESSION__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_SESSION__

#include <set>
#include <map>
#include <list>

#include "logger/Logger.h"
#include "core/synchronization/Event.hpp"
#include "util/TonNpiAddress.hpp"

#include "inman/inap/ACRegDefs.hpp"
#include "inman/inap/SS7Types.hpp"
#include "inman/inap/TCDlgDefs.hpp"
#include "inman/inap/SS7UnitsDefs.hpp"
#include "inman/inap/HDSSnDefs.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::logger::Logger;
using smsc::util::TonNpiAddress;
using smsc::core::synchronization::Event;


class Dialog;

class TCSessionAC;
class TCSessionSR;
class TCSessionMA;

struct TNoticeParms {
    TCDialogID  relId;
    uint8_t     reportCause;

    TNoticeParms() : reportCause(0)
    { }
    TNoticeParms(const TCDialogID & rel_id, uint8_t report_cause)
        : relId(rel_id), reportCause(report_cause)
    { }
};

/* ************************************************************************** *
 * class SSNSession (TCAP dialogs registry/factory):
 * ************************************************************************** */
typedef std::string TCSessionSUID;

class SSNSession {
private:
    typedef struct {
        struct timeval  tms;
        Dialog *        dlg;
    } DlgTime;

    typedef std::map<TCDialogID, Dialog*>        DialogsMAP;
    typedef std::map<TCDialogID, DlgTime>        DlgTimesMAP;
    typedef std::list<TCSessionAC*>             TCSessionsLIST;
    typedef std::map<std::string, TCSessionAC*> TCSessionsMAP;
    typedef std::map<TCDialogID, TNoticeParms>   NoticedDLGs;

    mutable Mutex   _sync;
    volatile bool   _stopping;

    DialogsMAP      dialogs;
    NoticedDLGs     ntcdDlgs;
    DlgTimesMAP     pending; //released but not terminated Dialogs with timestamp
    TCSessionsMAP   tcSessions;
    TCSessionsLIST  deadSess;

    uint8_t         _SSN;
    uint16_t        msgUserId;  //Common part message port user id
    SSNBinding      _cfg;
    uint16_t        lastTCSUId;
    const ApplicationContextRegistryITF * acReg;  //ApplicationContexts Registry
    Logger *        logger;

    void    dischargeDlg(Dialog * pDlg, const TCSessionSUID * tc_suid = 0);
    bool    nextDialogId(TCDialogID & dId);
    void    cleanUpDialogs(void);
    Dialog* locateDialog(const TCDialogID & dId) const;
    void    dumpDialogs(void) const;

protected:
    friend class TCAPDispatcher;
    SSNSession(const ApplicationContextRegistryITF * use_acReg, uint8_t ssn_id,
               uint16_t user_id, const SS7UnitInstsMap & tcap_inst_ids,
               uint16_t max_dlg_id = 2000, Logger * uselog = NULL);
    ~SSNSession();

    void Stop(void)
    {
      MutexGuard  tmp(_sync);
      _stopping = true;
    }
    //
    bool incMaxDlgs(uint16_t max_dlg_num) { return _cfg.incMaxDlgNum(max_dlg_num); }
    //
    int  Wait(int timeout_msec) //timeout unit: millisecs
    {
        return _cfg.Wait(timeout_msec);
    }
    //Returns false if no TCAP BE instance exists with given instId
    bool SignalUnit(uint8_t unit_inst_id, UNITStatus::BindStatus_e unit_status)
    {
        MutexGuard  tmp(_sync);
        return _cfg.SignalUnit(unit_inst_id, unit_status);
    }
    //
    void setUnitStatus(uint8_t unit_inst_id, UNITStatus::BindStatus_e inst_status)
    {
        MutexGuard  tmp(_sync);
        _cfg.setUnitStatus(unit_inst_id, inst_status);
    }
    //
    void ResetUnit(uint8_t unit_inst_id, UNITStatus::BindStatus_e use_status = UNITStatus::unitIdle)
    {
        MutexGuard  tmp(_sync);
        _cfg.ResetUnit(unit_inst_id, use_status);
    }

    bool getUnitsStatus(UnitsStatus & unit_set) const
    {
        MutexGuard  tmp(_sync);
        return _cfg.getUnitsStatus(unit_set);
    }
    //
    const UNITBinding * findUnit(uint8_t unit_inst_id) const
    {
      MutexGuard  tmp(_sync);
      return _cfg.findUnit(unit_inst_id);
    }
    //Returns number of unbinded TCAP BE instances
    unsigned unbindedUnits(void) const
    {
        MutexGuard  tmp(_sync);
        return _cfg.unbindedUnits();
    }


//protected:
    friend class TCSessionAC;
    friend class TCSessionSR;
    friend class TCSessionMA;
    bool    getDialogId(TCDialogID & dId);
    void    markDialog(Dialog * p_dlg);
    void    closeTCSession(TCSessionAC * p_sess);

public:
    SSNBinding::SSBindStatus    bindStatus(void)  const
    {
        MutexGuard  tmp(_sync);
        return _cfg.getStatus(); 
    }
    uint8_t     getSSN(void)    const { return _SSN; }
    uint16_t    getMsgUserId(void) const { return msgUserId; }


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
    Dialog* findDialog(const TCDialogID & did) const;
    void    releaseDialog(const TCDialogID & dId);
    void    releaseDialog(Dialog* pDlg, const TCSessionSUID * tc_suid = 0);
    void    releaseDialogs(const TCSessionSUID * tc_suid = 0);

    //Special methods used only by TCAP API Callbacks
    void    noticeInd(const TCDialogID & dlg_id, const TCDialogID & rel_id, uint8_t reportCause);
    bool    noticeParms(const TCDialogID & dlg_id, TNoticeParms & parms);
};

/* ************************************************************************** *
 * Various TCAP Session classes (TCAP dialogs factory):
 * ************************************************************************** */

class TCSessionAC { //Base for all types of TC Sessions
protected:
    typedef std::list<Dialog*> TCDialogsLIST;

    mutable Mutex   dlgGrd;
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
    uint16_t     getUID(void)        const { return tcUID; }
    uint8_t      getSSNid(void)      const { return _owner->getSSN(); }
    SSNSession * getSSNSession(void) const { return _owner; }
    SSNBinding::SSBindStatus  bindStatus(void) const { return _owner->bindStatus(); }

    const TonNpiAddress & getOwnAdr(void) const { return ownAdr; }
    const TCSessionSUID & Signature(void) const { return sign; }

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

