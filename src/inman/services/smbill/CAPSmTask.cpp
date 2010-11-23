#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/smbill/CAPSmTask.hpp"
using smsc::inman::common::RPCauseATT;

#include "inman/inap/TCDlgErrors.hpp"
using smsc::inman::inap::_RCS_TC_Dialog;
using smsc::inman::inap::TC_DlgError;

#include "inman/comp/cap_sms/CapSMSFactory.hpp"
using smsc::inman::comp::_ac_cap3_sms;

#include "inman/comp/cap_sms/MOSM_RPCauses.hpp"
using smsc::inman::comp::_RCS_MOSM_RPCause;

#include "inman/inap/dispatcher.hpp"
using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::inap::SSNSession;
using smsc::inman::inap::SSNBinding;
using smsc::inman::inap::TCSessionAC;

namespace smsc {
namespace inman {
namespace smbill {

CAPSmSubmit  _SMSubmitOK(true);
CAPSmSubmit  _SMSubmitNO(false);

/* ************************************************************************* *
 * class CAPSmResult implementation
 * ************************************************************************* */
//Scans all dialogs and adjusts the charging result, SCF error and rejectRPC status
//Returns true if all enqueued dialogs responded.
bool CAPSmResult::Adjust(void)
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
    if (abScf->_prm->_capSms.rejectRPC.exist(it->dlgRes->rpCause))
      rejectRPC = true;
  }
  return(allReplied = (cnt == dpRes.size()) ? true : false);
}

//Returns true if all enqueued dialogs completed
bool CAPSmResult::Completed(void) const
{
  for (CAPSmDPList::const_iterator it = dpRes.begin(); it != dpRes.end(); ++it) {
    if (!it->dlgRes || !it->dlgRes->completed)
      return false;
  }
  return true;
}


/* ************************************************************************* *
 * class CAPSmTaskAC partial implementation
 * ************************************************************************* */
CAPSmTaskAC::CAPSmTaskAC(const TonNpiAddress & use_abn,
              const CAPSmTaskCFG & cfg_ss7,
              uint32_t serv_key, CAPSmMode idp_mode/* = idpMO*/,
              const char * log_pfx/* = "CAPSm"*/, Logger * use_log/* = NULL*/)
  : CAPSmResult(cfg_ss7.abScf), logPfx(log_pfx), cfgSS7(cfg_ss7)
  , abNumber(use_abn), _pMode(pmIdle), capSess(0), logger(use_log)
{
  if (!logger)
      logger = Logger::getInstance("smsc.inman.CAPSm");

  _arg.reset(new SMSInitialDPArg(logger));
  _arg->setIDPParms(!idp_mode ? smsc::inman::comp::DeliveryMode_Originating :
                  smsc::inman::comp::DeliveryMode_Terminating , serv_key);
  _arg->setCallingPartyNumber(use_abn);
  snprintf(_logId, sizeof(_logId)-1, "%s[0x%lX]", logPfx, _Id);
}

CAPSmTaskAC::~CAPSmTaskAC()
{
  MutexGuard tmp(_sync);
  corpses.cleanUp();
}

const char * CAPSmTaskAC::nmPMode(CAPSmTaskAC::ProcMode p_mode)
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

//requires the _CAPSmsArg to be inited
RCHash CAPSmTaskAC::startDialog(CAPSmDPList::iterator & use_da)
{
    CapSMSDlg * pDlg = use_da->releaseDlg();
    if (pDlg)
        corpses.push_back(pDlg);
    use_da->attNum++;

    if (!capSess) {
        SSNSession * ssnSess = cfgSS7.disp->findSession(cfgSS7.ownSsn);
        if (!ssnSess) //attempt to open SSN
            ssnSess = cfgSS7.disp->openSSN(cfgSS7.ownSsn, cfgSS7.maxDlgId, logger);

        if (!(capSess = ssnSess->newSRsession(cfgSS7.ownAddr,
                                _ac_cap3_sms, 146, abScf->_scfAdr, cfgSS7.fakeSsn))) {
            std::string sid =
                ssnSess->mkSignature(cfgSS7.ownAddr, _ac_cap3_sms, 146, &(abScf->_scfAdr));
            smsc_log_error(logger, "%s: Unable to init TCSR session: %s", _logId, sid.c_str());
            return (use_da->scfErr = _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit));
        }
        smsc_log_debug(logger, "%s: using TCSR[%u]: %s", _logId,
                       capSess->getUID(), capSess->Signature().c_str());
    }
    if (!capSess || (capSess->bindStatus() < SSNBinding::ssnPartiallyBound)) {
        smsc_log_error(logger, "%s: SSN[%u] session is not available/bound", _logId,
                        cfgSS7.ownSsn);
        return (use_da->scfErr = _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit));
    }

    pDlg = new CapSMSDlg(capSess, this, cfgSS7.rosTimeout, abScf->Ident(), logger);
    use_da->resetRes(new CAPSmDlgResult(pDlg));
    _arg->setDestinationSubscriberNumber(use_da->dstAdr);
    pDlg->Init();
    smsc_log_debug(logger, "%s: Initiation(#%u) of %s: %s -> %s", _logId, use_da->attNum,
        pDlg->Ident(), abNumber.toString().c_str(), use_da->dstAdr.toString().c_str());
    return (use_da->scfErr = pDlg->initialDPSMS(_arg.get())); //begins TCAP dialog
}

void CAPSmTaskAC::abortDialogs(CAPSmDPList::iterator * it_exclude/* = NULL*/)
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

CAPSmDPList::iterator CAPSmTaskAC::lookUp(const TCDialogID & dlg_id)
{
  CAPSmDPList::iterator it = dpRes.begin();
  for (; it != dpRes.end(); ++it) {
    if (it->dlgRes && it->dlgRes->pDlg
        && (it->dlgRes->pDlg->getId() == dlg_id))
      break;
  }
  return it;
}

RCHash CAPSmTaskAC::reportSMSubmission(bool submitted)
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
void CAPSmTaskAC::onDPSMSResult(TCDialogID dlg_id, unsigned char rp_cause,
                        std::auto_ptr<SMSConnectArg> & sms_params)
{
    {
        MutexGuard tmp(_sync);
        CAPSmDPList::iterator res = lookUp(dlg_id);
        if (res == dpRes.end()) {
            smsc_log_warn(logger, "%s: DPSMRes_IND{%u} for unhandled Dialog[%u:%Xh]",
                          _logId, (unsigned)dlg_id.tcInstId, dlg_id.dlgId);
            return;
        }
        smsc_log_debug(logger, "%s: DPSMRes_IND{%u} for Dialog[%u:%Xh]",
                       _logId, (unsigned)rp_cause,
                       (unsigned)dlg_id.tcInstId, dlg_id.dlgId);

    
        res->dlgRes->answered = true;
        if (!(res->dlgRes->rpCause = rp_cause)) {    //ContinueSMS
            res->doCharge = true;
            if (sms_params.get()) {
                if (sms_params->paramsMask() & SMSConnectArg::connDSN) {
                    smsc_log_info(logger, "%s: %s set DA: %s", _logId, abScf->Ident(),
                        sms_params->destinationSubscriberNumber().toString().c_str());
                }
                if (sms_params->paramsMask() & SMSConnectArg::connCPN) {
                    smsc_log_info(logger, "%s: %s set OA: %s", _logId, abScf->Ident(),
                        sms_params->callingPartyNumber().toString().c_str());
                }
                if (sms_params->paramsMask() & SMSConnectArg::connSMSC) {
                    smsc_log_info(logger, "%s: %s set SMSC: %s", _logId, abScf->Ident(),
                        sms_params->SMSCAddress().toString().c_str());
                }
            }
            res->smsArg = sms_params.release();
            if (scheduleNextRequired()) {
                _fsmState = ScheduledTaskAC::pgCont;
                Reschedule(TaskSchedulerITF::sigProc);
                return;
            }
        } else {            //ReleaseSMS
            res->doCharge = false;
            //check first for RPCause that forces interaction retrying
            const RPCauseATT * rAtt = abScf->_prm->_capSms.retryRPC.exist(rp_cause);
            if (rAtt && (res->attNum < (rAtt->_att + 1U))) {
                daList.push_front(res);
                corpses.push_back(res->dlgRes->releaseDlg());
                res->resetRes();
                //reschedule task
                _fsmState = ScheduledTaskAC::pgCont;
                Reschedule(TaskSchedulerITF::sigProc);
                return;
            }
            res->scfErr = _RCS_MOSM_RPCause->mkhash(rp_cause);
            abortDialogs(&res); //abort all other dialogs and mark them as completed
        }
        if (Adjust()) {
            _pMode = pmInstructing;
            //reschedule task
            _fsmState = ScheduledTaskAC::pgCont;
            Reschedule(TaskSchedulerITF::sigReport);
        }
    }
    return;
}

void CAPSmTaskAC::onEndCapDlg(TCDialogID dlg_id, RCHash errcode)
{
    MutexGuard tmp(_sync);
    //smsc_log_debug(logger, "%s: END_IND { Dialog[0x%x] }", _logId, dlg_id);
    CAPSmDPList::iterator res = lookUp(dlg_id);
    if (res == dpRes.end())
        return; //END_IND for one of corpses

    res->dlgRes->completed = true;
    corpses.push_back(res->dlgRes->releaseDlg());
    if (errcode && !res->scfErr)
        res->scfErr = errcode;

    //select further actions depending on processing mode
    switch (_pMode) {
//    case pmIdle: //task is finished and just waits for TCAP dialogs being ended
    //initiating dialog(s) with SCF (IN-platform)
    case pmInit:
    case pmWaitingInstr: {
        //abnormal termination: task still hasn't reported result to referee
        abortDialogs(&res); //abort all other dialogs and mark them as completed
        Adjust();
        //reschedule task
        _pMode = pmInstructing;
        _fsmState = ScheduledTaskAC::pgCont;
        Reschedule(TaskSchedulerITF::sigReport);
        return;
    } //break;

    //giving instructions to SSF (Billing)
    case pmInstructing: {
        if (!Completed()) { //abnormal termination: task is reporting positive result to referee
            abortDialogs(&res); //abort all other dialogs and mark them as completed
            Adjust();
        } // else //normal termination: task is reporting negative result to referee
        return; //rescheduling will be done in CAPSmTaskAC::Report()
    } //break;

    //monitoring for submission report from SSF (Billing)
    case pmMonitoring: {
        //abnormal termination: task already has reported successfful result to referee
        abortDialogs(&res); //abort all other dialogs and mark them as completed
        //reschedule task
        _pMode = pmAcknowledging;
        _fsmState = ScheduledTaskAC::pgCont;
        Reschedule(TaskSchedulerITF::sigReport);
        return;
    } //break;

    case pmReportingEvent: {
        if (Completed()) {
            //reschedule task
            _pMode = pmAcknowledging;
            _fsmState = ScheduledTaskAC::pgCont;
            Reschedule(TaskSchedulerITF::sigReport);
            return;
        }
    } break;

    case pmAcknowledging:
        return; //rescheduling will be done in CAPSmTaskAC::Report()

    default:; //normal end
    } //eosw

    if (Completed()) {
        //reschedule task
        _pMode = pmIdle;
        _fsmState = ScheduledTaskAC::pgDone;
        Reschedule(TaskSchedulerITF::sigRelease);
    }
    return;
}

// -- --------------------------------------
// -- ScheduledTaskAC interface methods
// -- --------------------------------------

//Called on sigProc, switches task to next state.
ScheduledTaskAC::PGState CAPSmTaskAC::Process(auto_ptr_utl<UtilizableObjITF> & use_dat)
{
    MutexGuard tmp(_sync);
//    log_debug("Process", use_dat.get());

    corpses.cleanUp();
    switch (_pMode) {
    //initiating dialog(s) with SCF (IN-platform)
    case pmIdle:
    case pmInit:
    case pmWaitingInstr: {
        if (daList.empty()) { //inconsistency of signal and internal state
            log_error("Process", use_dat.get(), ", no DA set!");
            //keep task in suspended state for easier core file analyzis :)
            _fsmState = ScheduledTaskAC::pgSuspend;
        } else {
            if (!doNextInit()) {
                //abnormal initialization -> report CHARGE_NOT_POSSIBLE to Billing
                abortDialogs();
                _pMode = pmInstructing;
                _fsmState = ScheduledTaskAC::pgSuspend;
                Reschedule(TaskSchedulerITF::sigReport);
            }
        }
    } break;

//        case pmInstructing:
    //monitoring for submission report from SSF (Billing)
    case pmMonitoring: {
        CAPSmSubmit * res = static_cast<CAPSmSubmit*>(use_dat.get());
        if (!res) { //inconsistency of signal and internal state
            log_error("Process", use_dat.get(), ", arg missed!");
            //keep task in suspended state for easier core file analyzis :)
            _fsmState = ScheduledTaskAC::pgSuspend;
        } else {
            _pMode = pmReportingEvent;
            _fsmState = ScheduledTaskAC::pgCont;
            if (reportSMSubmission(res->isSubmitted())) {
                //submission report to SCF failed -> abort dialog(s) and report to Billing
                abortDialogs();
                _pMode = pmAcknowledging;
                _fsmState = ScheduledTaskAC::pgSuspend;
                Reschedule(TaskSchedulerITF::sigReport);
            } //else execution continues in onEndCapDlg()
        }
    } break;
//        case pmReportingEvent:
//        case pmAcknowledging:

    default:
        log_error("Process", use_dat.get(), ", inconsistent state");
    } //eosw
    return _fsmState;
}

//Called on sigAbort, aborts task, switches task to pgDone state.
ScheduledTaskAC::PGState CAPSmTaskAC::Abort(auto_ptr_utl<UtilizableObjITF> & use_dat)
{
    MutexGuard tmp(_sync);
    log_error("Abort", use_dat.get());

    corpses.cleanUp();
    abortDialogs();
    _pMode = pmIdle;
    return _fsmState = ScheduledTaskAC::pgDone;
}
//Called on sigReport, requests task to report to the referee (use_ref != 0)
//or perform some other reporting actions.
ScheduledTaskAC::PGState CAPSmTaskAC::Report(auto_ptr_utl<UtilizableObjITF> & use_dat,
                                                        TaskRefereeITF * use_ref/* = 0*/)
{
    MutexGuard tmp(_sync);
    log_debug("Report", use_dat.get(), use_ref ? ", referee(ON)" : ", referee(OFF)");

    corpses.cleanUp();
    Adjust();

    switch (_pMode) {
//    case pmIdle:
//    case pmInit:
//    case pmWaitingInstr:
    case pmInstructing: {
        if (use_ref) {
            use_ref->onTaskReport(_Owner, this);
            if (doCharge) { //CHARGING_POSSIBLE -> wait for SM submission status
              _pMode = pmMonitoring;
              _fsmState = ScheduledTaskAC::pgCont;
            } else {        //CHARGING_NOT_POSSIBLE -> normal completion
              _pMode = pmIdle; //if all TCAP dialog(s) ended immediately release
                               //itself, otherwise wait for all dialog(s) end
              _fsmState = Completed() ? ScheduledTaskAC::pgDone : ScheduledTaskAC::pgSuspend;
            }
        } else {
            //Billing is died or no longer interested in charging -> abort task
            if (doCharge) {
                log_error("Report", use_ref, ", no referee set, aborting ..");
                abortDialogs();
            } else //CHARGING_NOT_POSSIBLE -> normal completion
                log_warn("Report", use_ref, ", no referee set ..");
            _pMode = pmIdle; //if all TCAP dialog(s) ended immediately release
                             //itself, otherwise wait for all dialog(s) end
            _fsmState = Completed() ? ScheduledTaskAC::pgDone : ScheduledTaskAC::pgSuspend;
        }
    } break;
//    case pmMonitoring:
//    case pmReportingEvent:
    case pmAcknowledging: { //do a final report, completing ..
        if (use_ref)
            use_ref->onTaskReport(_Owner, this);
        _pMode = pmIdle;
        _fsmState = ScheduledTaskAC::pgDone;
    } break;
    //
    default: //inconsistent procMode
        log_warn("Report", use_dat.get(), use_ref ? ", referee(ON)" : ", referee(OFF)");
    } //eosw
    return _fsmState;
}


/* ************************************************************************* *
 * class CAPSmTaskMT implementation
 * ************************************************************************* */
bool  CAPSmTaskMT::doNextInit(void)
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

/* ************************************************************************* *
 * class CAPSmTaskSQ implementation
 * ************************************************************************* */
bool CAPSmTaskSQ::doNextInit(void)
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

} //smbill
} //inman
} //smsc

