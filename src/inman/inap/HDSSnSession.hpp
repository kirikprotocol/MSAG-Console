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
class UNITStatus {
public:
    uint8_t         instId;     //SS7 communication unit instanceId, [1..255]
    enum BindStatus_e { unitIdle = 0, unitError, unitAwaited, unitBound };

    BindStatus_e    bindStatus;

    UNITStatus(uint8_t inst_id = 0, BindStatus_e inst_status = unitIdle)
        : instId(inst_id), bindStatus(inst_status)
    { }

    UNITStatus(const SS7UnitInstance & unit_inst)
        : instId(unit_inst.instId)
        , bindStatus((unit_inst.connStatus == SS7UnitInstance::uconnError) ? unitError : unitIdle)
    { }

    void Reset(BindStatus_e use_status = UNITStatus::unitIdle)
    {
        bindStatus = use_status;
    }
    bool operator< (const UNITStatus & obj2) const
    {
        return (instId < obj2.instId);
    }
};
typedef std::set<UNITStatus> UNITsStatus;

class UNITBinding {
private:
    bool            signaled;

public:
    UNITStatus  unit;
    uint16_t    maxId;      //maximum dlgId for TCAP BE instance
    uint16_t    lastDlgId;  //dlgId of last allocated dialog
    uint16_t    numOfDlgs;  //number of active dialogs controlled by this unit

    UNITBinding(uint8_t inst_id = 0, uint16_t max_id = 0)
        : unit(inst_id), maxId(max_id), lastDlgId(0), numOfDlgs(0)
    { }
    UNITBinding(const UNITStatus & unit_inst, uint16_t max_id = 0)
        : unit(unit_inst), maxId(max_id), lastDlgId(0), numOfDlgs(0)
    { }

    void Rise(void) { signaled = true; }
    bool isSignaled(void) const { return signaled; }
    void Reset(UNITStatus::BindStatus_e use_status = UNITStatus::unitIdle)
    {
      signaled = false; unit.Reset(use_status);
    }
};

struct SSNBinding : public Event {
protected:
    typedef std::map<uint8_t /*instId*/, UNITBinding> UNITsBinding;

    uint16_t        maxDlgNum;  //maximum number of dialogs per SSN
    UNITsBinding    units;

public:
    //NOTE: that's a sorted enum !!!
    enum SSBindStatus { ssnIdle = 0, ssnError, ssnPartiallyBound, ssnBound };

    //NOTE: tcap_inst_ids cann't be empty!
    SSNBinding(const SS7UnitInstsMap & tcap_inst_ids, uint16_t max_dlg_num)
        : maxDlgNum(max_dlg_num)
    {
        uint16_t maxId = max_dlg_num/(uint16_t)tcap_inst_ids.size();
        for (SS7UnitInstsMap::const_iterator cit = tcap_inst_ids.begin();
                                            cit != tcap_inst_ids.end(); ++cit) {
            units.insert(UNITsBinding::value_type(cit->first,
                                        UNITBinding(UNITStatus(cit->second), maxId)));
        }
        units.begin()->second.maxId += max_dlg_num % tcap_inst_ids.size();
    }

    uint16_t MaxDlgNum(void) const { return maxDlgNum; }

    bool incMaxDlgNum(uint16_t max_dlg_num)
    {
         if (max_dlg_num > maxDlgNum) {
             uint16_t maxId = max_dlg_num/(uint16_t)units.size();

             maxDlgNum = max_dlg_num;
             for (UNITsBinding::iterator cit = units.begin(); cit != units.end(); ++cit)
                 cit->second.maxId = maxId;
             units.begin()->second.maxId += max_dlg_num % (uint16_t)units.size();
             return true;
         }
         return false;
    }
    //Returns true if at least one UNIT instanceId is present
    bool getUnitsStatus(UNITsStatus & unit_set) const
    {
        unit_set.clear();
        for (UNITsBinding::const_iterator cit = units.begin();
                                        cit != units.end(); ++cit)
            unit_set.insert(cit->second.unit);
        return !unit_set.empty();
    }

    //Returns number of unbinded TCAP BE instances
    unsigned unbindedUnits(void) const
    {
        unsigned rval = 0;
        for (UNITsBinding::const_iterator cit = units.begin();
                                        cit != units.end(); ++cit) {
            if (cit->second.unit.bindStatus != UNITStatus::unitBound)
                ++rval;
        }
        return rval;
    }
    //Searches for bound TCAP BE with lesser number of dialogs
    //Returns NULL if no TCAP BE available at that time
    UNITBinding * getUnitForDialog(void)
    {
        uint16_t    numOfDlgs = (uint16_t)-1;
        UNITsBinding::iterator  uIt = units.end();
        //NOTE: set cann't be empty!
        for (UNITsBinding::iterator cit = units.begin(); cit != units.end(); ++cit) {
            if ((cit->second.unit.bindStatus == UNITStatus::unitBound)
                && (cit->second.numOfDlgs < cit->second.maxId)
                && (cit->second.numOfDlgs < numOfDlgs)) {
                uIt = cit;
                numOfDlgs = cit->second.numOfDlgs;
            }
        }
        return (uIt == units.end()) ? NULL : &(uIt->second);
    }

    //
    UNITBinding * getUnit(uint8_t unit_inst_id)
    {
        UNITsBinding::iterator it = units.find(unit_inst_id);
        return (it != units.end()) ? &(it->second) : 0;
    }
    //
    const UNITBinding * findUnit(uint8_t unit_inst_id) const
    {
        UNITsBinding::const_iterator it = units.find(unit_inst_id);
        return (it != units.end()) ? &(it->second) : 0;
    }

    SSBindStatus getStatus(void) const
    {
        uint8_t mask = 0x00; //'0000 0,Bound,Error,Idle'
        
        for (UNITsBinding::const_iterator cit = units.begin(); cit != units.end(); ++cit) {
            if (cit->second.unit.bindStatus == UNITStatus::unitIdle) {
                mask |= 0x01;
            } else if (cit->second.unit.bindStatus == UNITStatus::unitError) {
                mask |= 0x02;
            } else if (cit->second.unit.bindStatus == UNITStatus::unitBound)
                mask |= 0x04;
        }
//        if ((mask & 0x07) == 0x01)  //all units are idle
//            return ssnIdle;
        if ((mask & 0x07) == 0x02)  //all units faced error
            return ssnError;
        if ((mask & 0x07) == 0x04)  //all units are bound
            return ssnBound;
        if (mask & 0x04)            //at least one unit is bound
            return ssnPartiallyBound;
        return ssnIdle;
    }

    bool setUnitStatus(uint8_t unit_inst_id, UNITStatus::BindStatus_e use_status)
    {
        UNITBinding * unb = getUnit(unit_inst_id);
        if (unb) {
            unb->unit.bindStatus = use_status;
            return true;
        }
        return false;
    }

    bool isUnitSignaled(uint8_t unit_inst_id) const
    {
        const UNITBinding * unb = findUnit(unit_inst_id);
        return  unb ? unb->isSignaled() : false;
    }
    bool isAllSignaled(void) const
    {
        //using just a linear search because of tiny number of elements
        for (UNITsBinding::const_iterator cit = units.begin(); cit != units.end(); ++cit) {
            if (!cit->second.isSignaled())
                return false;
        }
        return true;
    }
    //Returns false if no TCAP BE instance exists with given instId
    bool SignalUnit(uint8_t unit_inst_id, UNITStatus::BindStatus_e unit_status)
    {
        UNITBinding * unb = getUnit(unit_inst_id);
        if (!unb)
            return false;

        unb->Rise();
        unb->unit.bindStatus = unit_status;
        if (isAllSignaled())
            Signal(); //all units either are bound or faced an error
        return true;
    }
    //
    void ResetUnit(uint8_t unit_inst_id, UNITStatus::BindStatus_e use_status = UNITStatus::unitIdle)
    {
        UNITBinding * unb = getUnit(unit_inst_id);
        if (unb)
            unb->Reset(use_status);
    }
};


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

    bool getUnitsStatus(UNITsStatus & unit_set) const
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

