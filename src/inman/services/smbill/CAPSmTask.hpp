/* ************************************************************************* *
 * CAPSmTask: maintains a bunch of CAP3Sms dialogs (one per each destination
 * address) for given abonent. The overall result (permission for message(s)
 * transfer) is negative if at least one dialog results in negative
 * response (ReleaseSMS or some error).
 * ************************************************************************* */
#ifndef __INMAN_CAPSM_TASK_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_CAPSM_TASK_HPP

#include "inman/services/iapmgr/InScfCfg.hpp"
#include "inman/services/scheduler/TaskSchedulerDefs.hpp"

#include "inman/inap/cap_sms/DlgCapSMS.hpp"
#include "inman/inap/TCDspDefs.hpp"

namespace smsc {
namespace inman {
namespace smbill {

using smsc::inman::ScheduledTaskAC;

using smsc::inman::iapmgr::INScfCFG;

using smsc::inman::inap::CapSMSDlg;
using smsc::inman::inap::CapSMS_SSFhandlerITF;
using smsc::inman::inap::TCDialogID;
using smsc::inman::inap::TCAPDispatcherITF;
using smsc::inman::inap::TCAPUsr_CFG;


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

    explicit CAPSmDlgResult(CapSMSDlg * use_dlg = NULL)
        : rpCause(0), answered(false), completed(false), pDlg(use_dlg)
    { }
    ~CAPSmDlgResult()
    {
        if (pDlg) delete pDlg;
    }

    CapSMSDlg * releaseDlg(void)
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
    SMSConnectArg *     smsArg; //last ConnectSMSArg received
    CAPSmDlgResult *    dlgRes; //last CAPSmsDialog result

    explicit CAPSmDPResult(const TonNpiAddress & use_da)
        : dstAdr(use_da), attNum(0), doCharge(false), scfErr(0), smsArg(0), dlgRes(0)
    { }
    ~CAPSmDPResult()
    { 
        if (dlgRes) delete dlgRes;
    }

    CapSMSDlg * releaseDlg(void)
    {
        if (dlgRes) return dlgRes->releaseDlg();
        return NULL;
    }
    void resetRes(CAPSmDlgResult * use_dres = NULL)
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

    explicit CAPSmResult(const INScfCFG * use_scf)
        : abScf(use_scf), doCharge(false), scfErr(0), rejectRPC(false), allReplied(false)
    { }
    ~CAPSmResult()
    { }

    //Scans all dialogs and adjusts the charging result, SCF error and rejectRPC status
    //Returns true if all enqueued dialogs responded.
    bool Adjust(void)
    {
        doCharge = true;
        for (CAPSmDPList::const_iterator it = dpRes.begin(); it != dpRes.end(); ++it) {
            if (!it->doCharge) {
                doCharge = false; break;
            }
        }
        scfErr = 0;
        for (CAPSmDPList::const_iterator it = dpRes.begin(); it != dpRes.end(); ++it) {
            if (it->scfErr) {
                scfErr = it->scfErr; break;
            }
        }
        rejectRPC = false;
        CAPSmDPList::size_type cnt = 0;
        for (CAPSmDPList::const_iterator it = dpRes.begin();
              (it != dpRes.end()) && it->dlgRes && it->dlgRes->answered; ++it) {
            ++cnt;
            if (abScf->_capSms.rejectRPC.exist(it->dlgRes->rpCause))
                rejectRPC = true;
        }
        return (allReplied = (cnt == dpRes.size()) ? true : false);
    }

    //Returns true if all enqueued dialogs completed
    bool Completed(void) const
    {
        for (CAPSmDPList::const_iterator it = dpRes.begin(); it != dpRes.end(); ++it) {
            if (!it->dlgRes || !it->dlgRes->completed)
                return false;
        }
        return true;
    }
};


class CAPSmTaskAC : public CAPSmResult, public ScheduledTaskAC,
                    CapSMS_SSFhandlerITF {
public:
    enum CAPSmMode { idpMO = 0, idpMT};

    enum ProcMode {
        pmIdle = 0
        , pmInit            //initiating dialog(s) with SCF (IN-platform)
        , pmWaitingInstr    //waiting instructions from SCF (IN-platform)
        , pmInstructing     //giving instructions to SSF (Billing)
        , pmMonitoring      //monitoring for submission report from SSF (Billing)
        , pmReportingEvent  //reporting submission status to SCF ((IN-platform))
        , pmAcknowledging   //reporting dialog(s) completion status to Billing
    };

protected:
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

    const char *        logPfx;     //prefix for logging info
    const CAPSmTaskCFG  cfgSS7;
    const TonNpiAddress abNumber;

    mutable Mutex       _sync;
    volatile ProcMode   _pMode;
    std::auto_ptr<SMSInitialDPArg> _arg;
    DAList          daList;

    DLGList         corpses;
    TCSessionSR *   capSess;
    Logger *        logger;
    char            _logId[sizeof("CAPSmTask[%lu]") + sizeof(unsigned long)*3 + 1];


    void Reschedule(TaskSchedulerITF::PGSignal use_sig)
    {
        smsc_log_debug(logger, "%s: {%s, %s} signalling %s", _logId,
                       nmPMode(), nmPGState(), TaskSchedulerITF::nmPGSignal(use_sig));
        _Owner->SignalTask(_Id, use_sig);
    }
    void log_error(const char * nm_sig, bool has_arg = false, const char * aux_msg = "")
    {
        smsc_log_error(logger, "%s: {%s, %s} signal %s(%s)%s",
                       _logId, nmPMode(), nmPGState(), nm_sig, has_arg ? "arg" : "0", aux_msg);
    }
    void log_warn(const char * nm_sig, bool has_arg = false, const char * aux_msg = "")
    {
        smsc_log_warn(logger, "%s: {%s, %s} signal %s(%s)%s",
                       _logId, nmPMode(), nmPGState(), nm_sig, has_arg ? "arg" : "0", aux_msg);
    }
    void log_debug(const char * nm_sig, bool has_arg = false, const char * aux_msg = "")
    {
        smsc_log_debug(logger, "%s: {%s, %s} signal %s(%s)%s",
                       _logId, nmPMode(), nmPGState(), nm_sig, has_arg ? "arg" : "0", aux_msg);
    }

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
    CAPSmDPList::iterator lookUp(const TCDialogID & dlg_id)
    {
        CAPSmDPList::iterator it = dpRes.begin();
        for (; it != dpRes.end(); ++it) {
            if (it->dlgRes && it->dlgRes->pDlg
                && (it->dlgRes->pDlg->getId() == dlg_id))
                break;
        }
        return it;
    }

    RCHash reportSMSubmission(bool submitted)
    {
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
        return repErr;
    }


    // -- --------------------------------------
    // -- CapSMS_SSFhandlerITF interface methods
    // -- --------------------------------------
    friend class smsc::inman::inap::CapSMSDlg;
    void onDPSMSResult(TCDialogID dlg_id, unsigned char rp_cause,
                        std::auto_ptr<SMSConnectArg> & sms_params);
    void onEndCapDlg(TCDialogID dlg_id, RCHash errcode);

    // ----------------------------------
    // -- CAPSmTaskAC interface methods
    // ----------------------------------
    //Returns ScheduledTaskAC::PGState for pmMonitoring mode
//    virtual ProcMode monitoringFSM(void) const = 0;
    //Checks if task should be rescheduled in order to start next enqueued
    //dialog on receiving response from SCF
    virtual bool    scheduleNextRequired(void) const = 0;
    //Perfoms next step of task initialization.
    //Returns true on success, false otherwise.
    virtual bool    doNextInit(void) = 0;

public:
    CAPSmTaskAC(const TonNpiAddress & use_abn, const CAPSmTaskCFG & cfg_ss7,
                uint32_t serv_key, CAPSmMode idp_mode = idpMO,
                const char * log_pfx = "CAPSm", Logger * use_log = NULL)
        : CAPSmResult(cfg_ss7.abScf), logPfx(log_pfx), cfgSS7(cfg_ss7)
        , abNumber(use_abn), _pMode(pmIdle), capSess(0), logger(use_log)
    {
        _arg.reset(new SMSInitialDPArg(logger));
        _arg->setIDPParms(!idp_mode ? smsc::inman::comp::DeliveryMode_Originating :
                        smsc::inman::comp::DeliveryMode_Terminating , serv_key);
        _arg->setCallingPartyNumber(use_abn);
        snprintf(_logId, sizeof(_logId)-1, "%s[0x%lX]", logPfx, _Id);
        if (!logger)
            logger = Logger::getInstance("smsc.inman.CAPSm");
    }
    virtual ~CAPSmTaskAC()
    {
        MutexGuard tmp(_sync);
        corpses.cleanUp();
    }

    static const char * nmPMode(ProcMode p_mode)
    {
        switch (p_mode) {
        case pmInit:            return "pmInit";
        case pmWaitingInstr:    return "pmWaitingInstr";
        case pmInstructing:     return "pmInstructing";
        case pmMonitoring:      return "pmMonitoring";
        case pmReportingEvent:  return "pmReportingEvent";
        case pmAcknowledging:   return "pmAcknowledging";
        case pmIdle:            return "pmIdle";
        }
        return "pmUnknown";
    }
    //
    const char * nmPMode(void) const { return nmPMode(_pMode); }
    //
    ProcMode curPMode(void) const { return _pMode; }
    //
    SMSInitialDPArg & Arg(void) const { return *(_arg.get()); }
    //
    void enqueueDA(const TonNpiAddress & use_da)
    { 
        daList.push_back(dpRes.insert(dpRes.end(), CAPSmDPResult(use_da)));
    }

    //-- --------------------------------- --//
    //-- ScheduledTaskAC interface methods
    //-- --------------------------------- --//
    void Init(TaskId use_id, TaskSchedulerITF * owner)
    {
        ScheduledTaskAC::Init(use_id, owner);
        snprintf(_logId, sizeof(_logId)-1, "%s[0x%lX]", logPfx, _Id);
        _pMode = pmInit;
    }
    const char * TaskName(void) const { return _logId; }
    //Called on sigProc, switches task to next state.
    PGState Process(auto_ptr_utl<UtilizableObjITF> & use_dat);
    //Called on sigAbort, aborts task, switches task to pgDone state.
    PGState Abort(auto_ptr_utl<UtilizableObjITF> & use_dat);
    //Called on sigReport, requests task to report to the referee (use_ref != 0)
    //or perform some other reporting actions.
    PGState Report(auto_ptr_utl<UtilizableObjITF> & use_dat, TaskRefereeITF * use_ref = 0);

    //-- ----------------------------------- --//
    // -- UtilizableObjITF interface methods
    //-- ----------------------------------- --//
    void    Utilize(void) { delete this; }
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
    //Returns ScheduledTaskAC::PGState for pmMonitoring mode
//     ProcMode monitoringFSM(void) const { return ScheduledTaskAC::pgSuspend; }
    //Checks if task should be rescheduled in order to start next enqueued
    //dialog on receiving response from SCF
    bool scheduleNextRequired(void) const { return false; }
    //Perfoms next step of task initialization.
    //Returns true on success, false otherwise.
    bool    doNextInit(void)
    { //Start all required CapSMS dialogs at once
        do {
            if (startDialog(daList.front()))
                break;
            daList.pop_front();
        } while (!daList.empty());

        if (daList.empty()) {
            //all dialogs are succesfully started, waiting for SCF response
            _pMode = pmWaitingInstr;
            _fsmState = ScheduledTaskAC::pgCont;
            return true;
        }
        return false;
    }
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
        _Criterion += abScf->_scfAdr.getSignals();
        _Criterion += abNumber.toString();
    }
    ~CAPSmTaskSQ()
    { }

protected:
    //Returns ScheduledTaskAC::PGState for pmMonitoring mode
//    ProcMode monitoringFSM(void) const { return ScheduledTaskAC::pgCont; }
    //Checks if task should be rescheduled in order to start next enqueued
    //dialog on receiving response from SCF
    bool scheduleNextRequired(void) const { return !daList.empty(); }
    //Perfoms next step of task initialization.
    //Returns true on success, false otherwise.
    bool    doNextInit(void)
    { //Starts first one of enqueued dialog(s)
        if (!startDialog(daList.front())) {
            //dialog succesfully started, wait for SCF response
            //before starting next one.
            daList.pop_front();
            _pMode = pmWaitingInstr;
             //preserve task at top of scheduling queue
            _fsmState = ScheduledTaskAC::pgCont;
            return true;
        }
        return false;
    }
};


class CAPSmSubmit : public UtilizableObjITF {
private:
    bool doDel;

protected:
    bool smSubmit;

public:
    explicit CAPSmSubmit(bool sm_submmit = false, bool do_del = false)
      : doDel(do_del), smSubmit(sm_submmit)
    { }
    ~CAPSmSubmit()
    { }

    bool isSubmitted(void) const { return smSubmit; }

    //-- ----------------------------------- --//
    // -- UtilizableObjITF interface methods
    //-- ----------------------------------- --//
    void    Utilize(void) { if (doDel) delete this; }
};

extern CAPSmSubmit  _SMSubmitOK;
extern CAPSmSubmit  _SMSubmitNO;

} //smbill
} //inman
} //smsc
#endif /* __INMAN_CAPSM_TASK_HPP */

