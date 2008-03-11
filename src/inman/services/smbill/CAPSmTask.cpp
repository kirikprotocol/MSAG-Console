#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

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
using smsc::inman::inap::TCSessionAC;

namespace smsc {
namespace inman {
namespace smbill {
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
        SSNSession * ssnSess = cfgSS7.disp->findSession(cfgSS7.ownSsn);
        if (!ssnSess) //attempt to open SSN
            ssnSess = cfgSS7.disp->openSSN(cfgSS7.ownSsn, cfgSS7.maxDlgId, 1, logger);

        if (!(capSess = ssnSess->newSRsession(cfgSS7.ownAddr,
                                _ac_cap3_sms, 146, abScf->scfAdr, cfgSS7.fakeSsn))) {
            std::string sid =
                ssnSess->mkSignature(cfgSS7.ownAddr, _ac_cap3_sms, 146, &(abScf->scfAdr));
            smsc_log_error(logger, "%s: Unable to init TCSR session: %s", _logId, sid.c_str());
            return (use_da->scfErr = _RCS_TC_Dialog->mkhash(TC_DlgError::dlgInit));
        }
        smsc_log_debug(logger, "%s: using TCSR[%u]: %s", _logId,
                       capSess->getUID(), capSess->Signature().c_str());
    }
    if (!capSess || (capSess->getState() != smsc::inman::inap::ssnBound)) {
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

// -- --------------------------------------
// -- CapSMS_SSFhandlerITF interface methods
// -- --------------------------------------
void CAPSmTaskAC::onDPSMSResult(unsigned dlg_id, unsigned char rp_cause,
                        std::auto_ptr<ConnectSMSArg> & sms_params)
{
    {
        MutexGuard tmp(_sync);
        CAPSmDPList::iterator res = lookUp(dlg_id);
        if (res == dpRes.end()) {
            smsc_log_warn(logger, "%s: RES_IND for unhandled Dialog[0x%x]", _logId, dlg_id);
            return;
        }
    
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
        if (Adjust()) {
            _pMode = doCharge ? pmMonitoring : pmIdle;
            smsc_log_debug(logger, "%s: reporting ..", _logId);
            _Owner->ReportTask(_Id);
        }
    }
    return;
}

void CAPSmTaskAC::onEndCapDlg(unsigned dlg_id, RCHash errcode)
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
    bool doReport = false;
    if (_pMode == pmWaitingInstr) {
        //abnormal termination: task still hasn't reported result to referee
        abortDialogs(&res); //abort all other dialogs and mark them as completed
        res->doCharge = false;
        Adjust();
        doReport = true;
    } else if (_pMode == pmMonitoring) {
        //abnormal termination: task already has reported successfful result to referee
        abortDialogs(&res); //abort all other dialogs and mark them as completed
        doReport = true;
    } //else if (_pMode == pmIdle) //normal end

    if (doReport) {
        smsc_log_debug(logger, "%s: reporting ..", _logId);
        _Owner->ReportTask(_Id);
    }
    if (Completed()) {
        _pMode = pmIdle;
        _fsmState = ScheduledTaskAC::pgDone;
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
            if (daList.empty()) { //all dialogs are succesfully started
                _pMode = pmWaitingInstr;
                return _fsmState = ScheduledTaskAC::pgCont;
            }
            
            abortDialogs();
            _pMode = pmIdle;
            _fsmState = ScheduledTaskAC::pgDone;
            smsc_log_debug(logger, "%s: reporting ..", _logId);
            _Owner->ReportTask(_Id);
        } break;
    
        default: 
            smsc_log_warn(logger, "%s: activated at state %s", _logId,
                            ScheduledTaskAC::nmPGState(_fsmState));
        }
    }
    return _fsmState;
}

/* ************************************************************************* *
 * class CAPSmTaskSQ implementation
 * ************************************************************************* */
ScheduledTaskAC::PGState CAPSmTaskSQ::processTask(void)
{
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
                _pMode = pmWaitingInstr;
                return _fsmState = ScheduledTaskAC::pgCont;
            }
            abortDialogs();
            _pMode = pmIdle;
            _fsmState = ScheduledTaskAC::pgDone;
            smsc_log_debug(logger, "%s: reporting ..", _logId);
            _Owner->ReportTask(_Id);

        } break;
    
        default: 
            smsc_log_warn(logger, "%s: activated at state %s", _logId,
                            ScheduledTaskAC::nmPGState(_fsmState));
        }
    }
    return _fsmState;
}

} //smbill
} //inman
} //smsc

