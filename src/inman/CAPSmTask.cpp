#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/CAPSmTask.hpp"
using smsc::inman::common::RPCauseATT;

#include "inman/inap/TCDlgErrors.hpp"
using smsc::inman::inap::_RCS_TC_Dialog;
using smsc::inman::inap::TC_DlgError;

#include "inman/comp/cap_sms/MOSM_RPCauses.hpp"
using smsc::inman::comp::_RCS_MOSM_RPCause;

#include "inman/inap/dispatcher.hpp"
using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::inap::SSNSession;
using smsc::inman::inap::TCSessionAC;

namespace smsc {
namespace inman {
/* ************************************************************************* *
 * class CAPSmTaskAC partial implementation
 * ************************************************************************* */
//requires the _CAPSmsArg to be inited
RCHash CAPSmTaskAC::startDialog(CAPSmDPList::iterator & use_da)
{
    CapSMSDlg * pDlg = use_da->releaseDlg();
    if (pDlg)
        corpses.push_back(pDlg);
    use_da->attNum++;

    if (!capSess) {
        TCAPDispatcher * disp = TCAPDispatcher::getInstance();
        SSNSession * ssnSess = disp->findSession(cfgSS7.own_ssn);
        if (!ssnSess) //attempt to open SSN
            ssnSess = disp->openSSN(cfgSS7.own_ssn, cfgSS7.maxDlgId);

        if (!(capSess = ssnSess->newSRsession(cfgSS7.ssf_addr,
                ACOID::id_ac_cap3_sms_AC, 146, abScf->scfAdr))) {
            std::string sid;
            TCSessionAC::mkSignature(sid, cfgSS7.own_ssn, cfgSS7.ssf_addr,
                                     ACOID::id_ac_cap3_sms_AC, 146, &(abScf->scfAdr));
            smsc_log_error(logger, "%s: Unable to init TCSR session: %s", _logId, sid.c_str());
            return (use_da->scfErr = _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit));
        }
        smsc_log_debug(logger, "%s: using TCSR[%u]: %s", _logId,
                       capSess->getUID(), capSess->Signature().c_str());
    }
    if (!capSess || (capSess->getState() != smsc::inman::inap::ssnBound)) {
        smsc_log_error(logger, "%s: SSN[%u] session is not available/bound", _logId,
                        cfgSS7.own_ssn);
        return (use_da->scfErr = _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit));
    }

    pDlg = new CapSMSDlg(capSess, this, cfgSS7.capTimeout, abScf->Ident(), logger);
    use_da->resetRes(new CAPSmDlgResult(pDlg));
    _arg->setDestinationSubscriberNumber(use_da->dstAdr);
    pDlg->Init();
    smsc_log_debug(logger, "%s: Initiation(#%u) of %s: %s -> %s", _logId, use_da->attNum,
        pDlg->Ident(), abNumber.toString().c_str(), use_da->dstAdr.toString().c_str());
    return (use_da->scfErr = pDlg->initialDPSMS(_arg.get())); //begins TCAP dialog
}

// -- --------------------------------------
// -- CapSMS_SSFhandlerITF interface methods
// -- --------------------------------------
void CAPSmTaskAC::onDPSMSResult(unsigned dlg_id, unsigned char rp_cause,
                        std::auto_ptr<ConnectSMSArg> & sms_params)
{
    bool doReport = false;
    {
        MutexGuard tmp(_sync);
//        smsc_log_debug(logger, "%s: RES_IND { Dialog[0x%x] }", _logId, dlg_id);
        CAPSmDPList::iterator res = lookUp(dlg_id);
        if (res == dpRes.end())
            return;
    
        res->dlgRes->answered = true;
        if (!(res->dlgRes->rpCause = rp_cause)) {    //ContinueSMS
            res->doCharge = true;
            if (sms_params.get()) {
                if (sms_params->paramsMask() & ConnectSMSArg::connDSN) {
                    smsc_log_info(logger, "%s: %s set DA: %s", _logId, abScf->Ident(),
                        sms_params->destinationSubscriberNumber().toString().c_str());
                }
                if (sms_params->paramsMask() & ConnectSMSArg::connCPN) {
                    smsc_log_info(logger, "%s: %s set OA: %s", _logId, abScf->Ident(),
                        sms_params->callingPartyNumber().toString().c_str());
                }
                if (sms_params->paramsMask() & ConnectSMSArg::connSMSC) {
                    smsc_log_info(logger, "%s: %s set SMSC: %s", _logId, abScf->Ident(),
                        sms_params->SMSCAddress().toString().c_str());
                }
            }
            res->smsArg = sms_params.release();
            if (onContinueRes())
                return;
        } else {            //ReleaseSMS
            res->doCharge = false;
            //check first for RPCause that forces interaction retrying
            const RPCauseATT * rAtt = abScf->retryRPC.exist(rp_cause);
            if (rAtt && (res->attNum < (rAtt->second + 1))) {
                daList.push_front(res);
                corpses.push_back(res->dlgRes->releaseDlg());
                res->resetRes();
                //reschedule task
                _fsmState = ScheduledTaskAC::pgCont;
                smsc_log_debug(logger, "%s: signalling %s", _logId,
                                TaskSchedulerITF::nmPGSignal(TaskSchedulerITF::sigProc));
                Signal(TaskSchedulerITF::sigProc);
                return;
            }
            res->scfErr = _RCS_MOSM_RPCause->mkhash(rp_cause);
            abortDialogs(&res); //abort all other dialogs and mark them as completed
        }
        doReport = reported = Adjust();
    }
    if (doReport) {
        smsc_log_debug(logger, "%s: reporting ..", _logId);
        _Owner->ReportTask(_Id);
    }
    return;
}

void CAPSmTaskAC::onEndCapDlg(unsigned dlg_id, RCHash errcode)
{
    bool done = false;
    bool doReport = false;
    {
        MutexGuard tmp(_sync);
//        smsc_log_debug(logger, "%s: END_IND { Dialog[0x%x] }", _logId, dlg_id);
        CAPSmDPList::iterator res = lookUp(dlg_id);
        if (res == dpRes.end())
            return;
    
        res->dlgRes->completed = true;
        if (errcode)
            res->scfErr = errcode;
        corpses.push_back(res->dlgRes->releaseDlg());

        if ((done = Completed())) {
            Adjust();
            _fsmState = ScheduledTaskAC::pgDone;
            if (!reported) 
                doReport = reported = true;
        }
    }
    if (doReport) {
        smsc_log_debug(logger, "%s: reporting ..", _logId);
        _Owner->ReportTask(_Id);
    }
    if (done) {
        smsc_log_debug(logger, "%s: signalling %s", _logId,
                        TaskSchedulerITF::nmPGSignal(TaskSchedulerITF::sigRelease));
        Signal(TaskSchedulerITF::sigRelease);
    }
    return;
}


/* ************************************************************************* *
 * class CAPSmTaskMT implementation
 * ************************************************************************* */
ScheduledTaskAC::PGState CAPSmTaskMT::processTask(void)
{
    bool doReport = false;
    {
        MutexGuard tmp(_sync);
        corpses.cleanUp();
    
        switch (_fsmState) {
        case ScheduledTaskAC::pgIdle:
        case ScheduledTaskAC::pgCont: { //start all required CapSMS dialogs
            if (daList.empty()) {
                smsc_log_error(logger, "%s: no dest.addresses set!!!", _logId);
                return (_fsmState = ScheduledTaskAC::pgSuspend);
            }
            //start all enqueued dialogs
            do {
                if (startDialog(daList.front()))
                    break;
                daList.pop_front();
            } while (!daList.empty());
            if (daList.empty()) //all dialogs are succesfully started
                return _fsmState = ScheduledTaskAC::pgCont;
            
            abortDialogs();
            _fsmState = ScheduledTaskAC::pgDone;
            doReport = true;
        } break;
    
        default: 
            smsc_log_warn(logger, "%s: activated at state %s", _logId,
                            ScheduledTaskAC::nmPGState(_fsmState));
        }
    }
    if (doReport) {
        smsc_log_debug(logger, "%s: reporting ..", _logId);
        _Owner->ReportTask(_Id);
    }
    return _fsmState;
}

/* ************************************************************************* *
 * class CAPSmTaskSQ implementation
 * ************************************************************************* */
ScheduledTaskAC::PGState CAPSmTaskSQ::processTask(void)
{
    bool doReport = false;
    {
        MutexGuard tmp(_sync);
        corpses.cleanUp();
    
        switch (_fsmState) {
        case ScheduledTaskAC::pgIdle:
        case ScheduledTaskAC::pgCont: { //start all required CapSMS dialogs
            if (daList.empty()) {
                smsc_log_error(logger, "%s: no dest.addresses set!!!", _logId);
                return (_fsmState = ScheduledTaskAC::pgSuspend);
            }
            //start first enqueued dialog
            if (!startDialog(daList.front())) {
                daList.pop_front();
                return _fsmState = ScheduledTaskAC::pgCont;
            }
            abortDialogs();
            _fsmState = ScheduledTaskAC::pgDone;
            doReport = true;
        } break;
    
        default: 
            smsc_log_warn(logger, "%s: activated at state %s", _logId,
                            ScheduledTaskAC::nmPGState(_fsmState));
        }
    }
    if (doReport) {
        smsc_log_debug(logger, "%s: reporting ..", _logId);
        _Owner->ReportTask(_Id);
    }
    return _fsmState;
}

} //inman
} //smsc

