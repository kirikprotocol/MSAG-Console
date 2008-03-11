#pragma ident "$Id$"
/* ************************************************************************* *
 * CAPSmTask: maintains a bunch of CAP3Sms dialogs (one per each destination
 * address) for given abonent. The overall result (permission for message(s)
 * transfer) is negative if at least one dialog results in negative
 * response (ReleaseSMS or some error).
 * ************************************************************************* */
#ifndef __INMAN_CAPSM_TASK_HPP
#define __INMAN_CAPSM_TASK_HPP

#include "inman/InScf.hpp"
using smsc::inman::INScfCFG;

#include "inman/services/scheduler/TaskSchedulerDefs.hpp"
using smsc::util::ScheduledTaskAC;

#include "inman/inap/cap_sms/DlgCapSMS.hpp"
using smsc::inman::inap::CapSMSDlg;
using smsc::inman::inap::CapSMS_SSFhandlerITF;

#include "inman/inap/TCDspDefs.hpp"
using smsc::inman::inap::TCAPDispatcherITF;

#include "inman/inap/TCDspDefs.hpp"
using smsc::inman::inap::TCAPUsr_CFG;

namespace smsc {
namespace inman {
namespace smbill {

struct CAPSmTaskCFG : public TCAPUsr_CFG {
    TCAPDispatcherITF * disp;
    const INScfCFG *    abScf;

    CAPSmTaskCFG(const TCAPUsr_CFG & use_cfg,
                 TCAPDispatcherITF * use_disp = NULL,
                 const INScfCFG *    use_scf = NULL)
        : TCAPUsr_CFG(use_cfg), disp(use_disp), abScf(use_scf)
    { }
};

class CAPSmDlgResult {
public:
    uint8_t         rpCause;
    bool            answered; //rpCause has been got from SCF
    bool            completed; //dialog has been finished
    CapSMSDlg *     pDlg;

    CAPSmDlgResult(CapSMSDlg * use_dlg = NULL)
        : rpCause(0), pDlg(use_dlg), answered(false), completed(false)
    { }
    ~CAPSmDlgResult()
    {
        if (pDlg) delete pDlg;
    }

    inline CapSMSDlg * releaseDlg(void)
    {
        CapSMSDlg * tmp = pDlg;
        pDlg = NULL;
        return tmp;
    }
};

class CAPSmDPResult {
public:
    const TonNpiAddress dstAdr;
    unsigned            attNum;     //attempt number
    bool                doCharge;
    RCHash              scfErr;
    ConnectSMSArg *     smsArg; //last ConnectSMSArg received
    CAPSmDlgResult *    dlgRes; //last CAPSmsDialog result

    CAPSmDPResult(const TonNpiAddress & use_da)
        : dstAdr(use_da), attNum(0), doCharge(false), scfErr(0), dlgRes(0), smsArg(0)
    { }
    ~CAPSmDPResult()
    { 
        if (dlgRes) delete dlgRes;
    }

    inline CapSMSDlg * releaseDlg(void)
    {
        if (dlgRes) return dlgRes->releaseDlg();
        return NULL;
    }
    inline void resetRes(CAPSmDlgResult * use_dres = NULL)
    {
        if (dlgRes) delete dlgRes;
        dlgRes = use_dres;
        if (smsArg) delete smsArg;
        smsArg = NULL;
    }
};

typedef std::list<CAPSmDPResult> CAPSmDPList;

class CAPSmResult {
public:
    const INScfCFG * abScf;     //
    bool             doCharge;  //charge permission
    RCHash           scfErr;    //first SCF error occured
    bool             rejectRPC; //charging denied due to RCPcause from rejectRPC list
    bool             allReplied;//
    CAPSmDPList      dpRes;     //

    CAPSmResult(const INScfCFG * use_scf)
        : abScf(use_scf), doCharge(false), rejectRPC(false), scfErr(0), allReplied(false)
    { }
    ~CAPSmResult()
    { }

    //Scans all dialogs and adjusts the charging result, SCF error and rejectRPC status
    //Returns true if all enqueued dialogs responded.
    bool Adjust(void)
    {
        doCharge = true;
        for (CAPSmDPList::iterator it = dpRes.begin(); it != dpRes.end(); ++it) {
            if (!it->doCharge) {
                doCharge = false; break;
            }
        }
        scfErr = 0;
        for (CAPSmDPList::iterator it = dpRes.begin(); it != dpRes.end(); ++it) {
            if (it->scfErr) {
                scfErr = it->scfErr; break;
            }
        }
        rejectRPC = false;
        CAPSmDPList::size_type cnt = 0;
        for (CAPSmDPList::iterator it = dpRes.begin();
              (it != dpRes.end()) && it->dlgRes && it->dlgRes->answered; ++it) {
            ++cnt;
            if (abScf->rejectRPC.exist(it->dlgRes->rpCause))
                rejectRPC = true;
        }
        return (allReplied = (cnt == dpRes.size()) ? true : false);
    }

    //Returns true if all enqueued dialogs completed
    bool Completed(void)
    {
        for (CAPSmDPList::iterator it = dpRes.begin(); it != dpRes.end(); ++it) {
            if (!it->dlgRes || !it->dlgRes->completed)
                return false;
        }
        return true;
    }
};


class CAPSmTaskAC : public CAPSmResult, public ScheduledTaskAC,
                    CapSMS_SSFhandlerITF {
protected:
    enum ProcMode { pmIdle = 0,
        pmWaitingInstr, //waiting instructions from IN-platform
        pmMonitoring    //monitoring for submission report from SMSC
    };

    typedef std::list<CAPSmDPList::iterator> DAList;

    class DLGList : public std::list<CapSMSDlg*> {
    public:
        void cleanUp(void)
        {
            for (DLGList::iterator it = begin(); it != end(); ++it) {
                if (*it) delete *it;
            }
            clear();
        }
    };

    const char *    logPfx;        //prefix for logging info
    const CAPSmTaskCFG  cfgSS7;
    const TonNpiAddress abNumber;

    mutable Mutex   _sync;
    ProcMode        _pMode;
    std::auto_ptr<InitialDPSMSArg> _arg;
    DAList          daList;
    volatile bool   reporting; //task is or awaits reporting to referee
    DLGList         corpses;
    TCSessionSR *   capSess;
    Logger *        logger;
    char            _logId[sizeof("CAPSmTask[%lu]") + sizeof(unsigned long)*3 + 1];

    // ----------------------------------
    // -- CAPSmTaskAC interface methods
    // ----------------------------------
    virtual PGState processTask(void) = 0;
    //returns true if rescheduling was requested
    virtual bool    onContinueRes(void) = 0;

public:
    enum CAPSmMode { idpMO = 0, idpMT};

    CAPSmTaskAC(const TonNpiAddress & use_abn, const CAPSmTaskCFG & cfg_ss7,
                uint32_t serv_key, CAPSmMode idp_mode = idpMO,
                const char * log_pfx = "CAPSm", Logger * use_log = NULL)
        : ScheduledTaskAC(true), CAPSmResult(cfg_ss7.abScf), cfgSS7(cfg_ss7)
        , abNumber(use_abn), capSess(0), _pMode(pmIdle)
        , logPfx(log_pfx), logger(use_log)
    {
        _arg.reset(new InitialDPSMSArg(logger));
        _arg->setIDPParms(!idp_mode ? smsc::inman::comp::DeliveryMode_Originating :
                        smsc::inman::comp::DeliveryMode_Terminating , serv_key);
        _arg->setCallingPartyNumber(use_abn);
        snprintf(_logId, sizeof(_logId)-1, "%s[0x%lX]", logPfx, _Id);
        if (!logger)
            logger = Logger::getInstance("smsc.inman.CAPSm");
    }
    virtual ~CAPSmTaskAC()
    { corpses.cleanUp(); }

    inline InitialDPSMSArg & Arg(void) const { return *(_arg.get()); }

    inline void enqueueDA(const TonNpiAddress & use_da)
    { 
        daList.push_back(dpRes.insert(dpRes.end(), CAPSmDPResult(use_da)));
    }

    RCHash reportSMSubmission(bool submitted)
    {
        MutexGuard tmp(_sync);
        RCHash repErr = 0;
        for (CAPSmDPList::iterator it = dpRes.begin();
              (it != dpRes.end()) && it->dlgRes && it->dlgRes->pDlg; ++it) {
            RCHash err = it->dlgRes->pDlg->reportSubmission(submitted);
            if (err) {
                if (!it->scfErr)
                    it->scfErr = err;
                if (!repErr)
                    repErr = err;
            }
        }
        _pMode = pmIdle;
        _fsmState = ScheduledTaskAC::pgSuspend;
        smsc_log_debug(logger, "%s: signalling %s", _logId,
                        TaskSchedulerITF::nmPGSignal(TaskSchedulerITF::sigSuspend));
        Signal(TaskSchedulerITF::sigSuspend);
        return repErr;
    }
    //-- --------------------------------- --//
    //-- ScheduledTaskAC interface methods
    //-- --------------------------------- --//
    inline const char * TaskName(void) const { return _logId; }
    void Init(TaskId use_id, TaskSchedulerITF * owner)
    {
        ScheduledTaskAC::Init(use_id, owner);
        snprintf(_logId, sizeof(_logId)-1, "%s[0x%lX]", logPfx, _Id);
    }
    PGState Process(bool do_abort = false)
    {
        return do_abort ? abortTask() : processTask();
    }

protected:
    //requires the _CAPSmsArg to be inited
    RCHash startDialog(CAPSmDPList::iterator & use_da);

    void abortDialogs(CAPSmDPList::iterator * it_exclude = NULL)
    {
        for (CAPSmDPList::iterator it = dpRes.begin(); it != dpRes.end(); ++it) {
            if (!it_exclude || (it != *it_exclude)) {
                if (!it->dlgRes) {
                    it->resetRes(new CAPSmDlgResult());
                } else {
                    if (it->dlgRes->pDlg) //dialog still active ?
                        it->dlgRes->pDlg->abortSMS();
                }
                it->dlgRes->completed = true;
            }
        }
    }

    PGState abortTask(void)
    {
        MutexGuard tmp(_sync);
        smsc_log_warn(logger, "%s: aborting at state %s", _logId,
                        ScheduledTaskAC::nmPGState(_fsmState));
        abortDialogs();
        return (_fsmState = ScheduledTaskAC::pgDone);
    }

    CAPSmDPList::iterator lookUp(unsigned dlg_id)
    {
        CAPSmDPList::iterator it = dpRes.begin();
        for (; it != dpRes.end(); ++it) {
            if (it->dlgRes && it->dlgRes->pDlg
                && (it->dlgRes->pDlg->getId() == dlg_id))
                break;
        }
        return it;
    }
    // -- --------------------------------------
    // -- CapSMS_SSFhandlerITF interface methods
    // -- --------------------------------------
    friend class smsc::inman::inap::CapSMSDlg;
    void onDPSMSResult(unsigned dlg_id, unsigned char rp_cause,
                        std::auto_ptr<ConnectSMSArg> & sms_params);
    void onEndCapDlg(unsigned dlg_id, RCHash errcode);
};

//MT (parallel mode) implementation of CAPSmTaskAC:
//all CAP dialogs are maintained simultaneously
//Scheduling criterion is not used
//Supposed to be scheduled by schedMT scheduler
class CAPSmTaskMT : public CAPSmTaskAC {
public: 
    CAPSmTaskMT(const TonNpiAddress & use_abn, const CAPSmTaskCFG & cfg_ss7,
                uint32_t serv_key, CAPSmMode idp_mode = idpMO, Logger * use_log = NULL)
        : CAPSmTaskAC(use_abn, cfg_ss7, serv_key,
                      idp_mode, (const char *)"CAPSmMT", use_log)
    { /* doesn't care about _Criterion */ }
    ~CAPSmTaskMT()
    { }
    
protected:
    //-- ScheduledTaskAC interface methods
    PGState processTask(void);
    //returns true if rescheduling was requested
    inline bool onContinueRes(void) { return false; }
};

//SQ (sequential mode) implementation of CAPSmTaskAC:
//All CAP dialogs are maintained sequentially (next dialog starts as
//current one gets ContinueSMS)
//Scheduling criterion is a pair {scfAddress, abonentAddress} 
//Supposed to be scheduled by schedSEQ scheduler
class CAPSmTaskSQ : public CAPSmTaskAC {
public: 
    CAPSmTaskSQ(const TonNpiAddress & use_abn, const CAPSmTaskCFG & cfg_ss7,
                uint32_t serv_key, CAPSmMode idp_mode = idpMO, Logger * use_log = NULL)
        : CAPSmTaskAC(use_abn, cfg_ss7, serv_key,
                      idp_mode, (const char *)"CAPSmSQ", use_log)
    { 
        _Criterion += abScf->scfAdr.getSignals();
        _Criterion += abNumber.toString();
    }
    ~CAPSmTaskSQ()
    { }
    
protected:
    //-- ScheduledTaskAC interface methods
    PGState processTask(void);
    //returns true if rescheduling was requested
    bool onContinueRes(void)
    {
        if (!daList.empty()) { //reschedule task: start next dialog
            _fsmState = ScheduledTaskAC::pgCont;
            smsc_log_debug(logger, "%s: signalling %s", _logId,
                            TaskSchedulerITF::nmPGSignal(TaskSchedulerITF::sigProc));
            Signal(TaskSchedulerITF::sigProc);
            return true;
        }
        return false; 
    }
};

} //smbill
} //inman
} //smsc
#endif /* __INMAN_CAPSM_TASK_HPP */

