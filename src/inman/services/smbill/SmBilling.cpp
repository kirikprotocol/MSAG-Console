#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/smbill/SmBilling.hpp"
using smsc::inman::iaprvd::IAProviderAC;
using smsc::inman::iaprvd::CSIRecord;
using smsc::inman::iaprvd::IAPProperty;

using smsc::inman::iapmgr::IAProviderInfo;
using smsc::inman::iapmgr::INParmsCapSms;

using smsc::inman::interaction::SerializerException;
using smsc::inman::interaction::INPBilling;
using smsc::inman::interaction::SPckChargeSmsResult;
using smsc::inman::interaction::DeliveredSmsData;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
//using smsc::inman::interaction::CsBillingHdr_dlg;
using smsc::core::synchronization::MutexTryGuard;
//using smsc::core::synchronization::TimeSlice;
using smsc::util::URCRegistry;
using smsc::util::format;
using smsc::util::CustomException;

using smsc::inman::smbill::_SMSubmitOK;
using smsc::inman::smbill::_SMSubmitNO;

using smsc::inman::comp::UnifiedCSI;
using smsc::inman::comp::ODBGeneralData;

#include "inman/comp/cap_sms/MOSM_RPCauses.hpp"
using smsc::inman::comp::_RCS_MOSM_RPCause;

#include "inman/comp/MapOpErrors.hpp"
using smsc::inman::comp::_RCS_MAPOpErrors;
using smsc::inman::comp::MAPOpErrorId;

//#include "inman/INManErrors.hpp"
using smsc::inman::inap::_RCS_TC_Dialog;
using smsc::inman::inap::TC_DlgError;

#ifdef SMSEXTRA
#define SMSX_WEB_GT "GT SMSCX"
#endif /* SMSEXTRA */

namespace smsc {
namespace inman {
namespace smbill {

const char * const _BILLmodes[] = {"OFF", "CDR", "IN"};
const char * const _MSGtypes[] = {"unknown", "SMS", "USSD", "XSMS"};
const char * const _CDRmodes[] = {"none", "billMode", "all"};

/* ************************************************************************** *
 * class Billing implementation:
 * ************************************************************************** */
Billing::~Billing()
{
  MutexGuard grd(_sync);
  doCleanUp();
  smsc_log_debug(_logger, "%s: Deleted", _logId);
}

const char * Billing::state2Str(BillingState bil_state)
{
  switch (bil_state) {
  case bilStarted:        return "bilStarted";
  case bilQueried:        return "bilQueried";
  case bilInited:         return "bilInited";
  case bilReleased:       return "bilReleased";
  case bilContinued:      return "bilContinued";
  case bilAborted:        return "bilAborted";
  case bilSubmitted:      return "bilSubmitted";
  case bilReported:       return "bilReported";
  case bilComplete:       return "bilComplete";
  case bilIdle:           return "bilIdle";
  }
  return "billUnknown";
}

void Billing::configure(const SmBillingCFG & use_cfg, const char * id_pfx)
{
  _cfg = &use_cfg;
  snprintf(_logId, sizeof(_logId)-1, "Billing[%s:%u]", id_pfx, _wrkId);
}

//NOTE: it's the processing graph entry point, so locks _sync !!!
void Billing::wrkHandlePacket(SmBillRequestMsg & recv_pck, PacketBufferAC * pck_buf/* = NULL*/) /*throw()*/
{
  {
    MutexGuard      grd(_sync);
    PGraphState     pgState = Billing::pgCont;
  
    smsc_log_debug(_logger, "%s: <-- %s at state %s", _logId,
                   INPBilling::nameOfCmd(recv_pck._cmdId), state2Str());

    switch (_pState) {
    case Billing::bilIdle: {
      switch (recv_pck._cmdId) {
      case INPBilling::CHARGE_SMS_TAG: {
        _pState = Billing::bilStarted;
        bool badPdu = false;
        ChargeSms & chgReq = recv_pck.u._chgSms->_Cmd;
        //complete the command deserialization
        try {
          chgReq.load(*pck_buf);
          chgReq.export2CDR(_cdr);
          chgReq.exportCAPInfo(_csInfo);
          _chrgFlags = chgReq.getChargingFlags();
        } catch (const SerializerException & exc) {
          smsc_log_error(_logger, "%s: %s", _logId, exc.what());
          badPdu = true;
        }
        if (badPdu || (badPdu = !verifyChargeSms())) {
          SPckChargeSmsResult spck;
          spck._Cmd.setValue(ChargeSmsResult::CHARGING_NOT_POSSIBLE);
          spck._Cmd.setError(
              _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData),
              _RCS_INManErrors->explainCode(INManErrorId::protocolInvalidData).c_str());
          spck._Hdr.dlgId = _wrkId;
          sendCmd(spck); //ignore sending result here
          pgState = Billing::pgEnd;
        } else { //process charging request
          pgState = onChargeSms();
        }
      } break;
  
      case INPBilling::DELIVERED_SMS_DATA_TAG: { 
        _pState = Billing::bilStarted;
        bool badPdu = false;
        DeliveredSmsData & chgReq = recv_pck.u._chgDSms->_Cmd;
        //complete the command deserialization
        try {
          chgReq.load(*pck_buf);
          chgReq.export2CDR(_cdr);
          _chrgFlags = chgReq.getChargingFlags();
        } catch (const SerializerException & exc) {
          smsc_log_error(_logger, "%s: %s", _logId, exc.what());
          badPdu = true;
        }
        if (badPdu || (badPdu = !verifyChargeSms())) {
          _billErr = _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData);
          pgState = Billing::pgEnd;
        } else { //process charging request
          pgState = onChargeSms();
        }
      } break;
  
      default: {
        smsc_log_error(_logger, "%s: protocol error: cmd %u, state %u",
                       _logId, (unsigned)recv_pck._cmdId, _pState);
        pgState = Billing::pgEnd;
      }
      } /* eosw */
    } break;
  
    case Billing::bilContinued: {
      if (recv_pck._cmdId == INPBilling::DELIVERY_SMS_RESULT_TAG) {
        stopTimer(_pState);
        bool badPdu = false;
        DeliverySmsResult & smsRes = recv_pck.u._dlvrRes->_Cmd;
        //complete the command deserialization
        try { 
          smsRes.load(*pck_buf);
          smsRes.export2CDR(_cdr);
        } catch (const SerializerException & exc) {
          smsc_log_error(_logger, "%s: %s", _logId, exc.what());
          badPdu = true;
          _pState = Billing::bilAborted;
        }
        pgState = badPdu ? Billing::pgEnd : onDeliverySmsResult();
        break;
      }
    } //no break, fall into default !!!
    default: //ignore unknown/illegal command
      smsc_log_error(_logger, "%s: protocol error: cmd %u, state %u",
                     _logId, (unsigned)recv_pck._cmdId, _pState);
    } /* eosw */
    if (pgState != Billing::pgEnd)
      return; //worker continues processing
    doFinalize();
  }
  _wrkMgr->workerDone(*this);
}

/* -------------------------------------------------------------------------- *
 * WorkerIface interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks _sync !!!
void Billing::wrkInit(smsc::inman::interaction::WorkerID w_id,
                      smsc::inman::interaction::WorkerManagerIface * use_mgr,
                      Logger * use_log/* = NULL*/)
{
  _wrkId = w_id; _wrkMgr = use_mgr;
  if (!(_logger = use_log))
    _logger = Logger::getInstance("smsc.inman.smbill");
}

//Prints some information about worker state/status
void Billing::wrkLogState(std::string & use_str) const
{
  MutexGuard grd(_sync);
  format(use_str, "%u{%u}", _wrkId, (unsigned)_pState);
}

//NOTE: it's the processing graph entry point, so locks Mutex !!!
void Billing::wrkAbort(const char * abrt_reason/* = NULL*/)
{
  bool wDone = false;
  {
    MutexGuard grd(_sync);
    if ((_pState != bilAborted) && (_pState != bilComplete)) {
      abortThis(abrt_reason);
      wDone = doFinalize();
    } else {
      smsc_log_warn(_logger, "%s: Abort requested at state(%s)%s%s",
                    _logId, state2Str(), abrt_reason ? ", reason: " : "",
                    abrt_reason ? abrt_reason : "");
    }
  }
  if (wDone)
    _wrkMgr->workerDone(*this);
}

/* ---------------------------------------------------------------------------------- *
 * Protected/Private methods:
 * NOTE: these methods are not the processing graph entries, so never lock _sync,
 *       it's a caller responsibility to lock _sync !!!
 * ---------------------------------------------------------------------------------- */
//returns true if required (depending on chargeMode) CDR data fullfilled
bool Billing::isCDRComplete(void) const
{
  if (_cdr._chargePolicy == CDRRecord::ON_SUBMIT)
    return (_cdr._finalized >= CDRRecord::dpSubmitted) ? true : false;
  return (_cdr._finalized == CDRRecord::dpCollected)  ? true : false;
}

//returns true if all billing stages are completed
bool Billing::isBillComplete(void) const
{
  return ((_pState >= Billing::bilReported) && isCDRComplete()) ? true : false;
}

//Returns true if next configured IAProvider has specified ability.
bool Billing::nextIAProviderHas(IAPAbility::Option_e op_val) const
{
  unsigned i = _curIAPrvd;

  while (i < AbonentPolicy::iapSecondary) {
    const IAProviderInfo * pPrvd = _iapRule._iaPolicy->getIAProvider(static_cast<IAPPrio_e>(++i));
    if (pPrvd && pPrvd->hasAbility(op_val))
      return true;
  }
  return false;
}

bool Billing::sendCmd(const SPckChargeSmsResult & use_res) const
{
  bool srlzRes = true;
  smsc::inman::interaction::PacketBuffer_T<512>  pckBuf;

  try {
    use_res.serialize(pckBuf);
    pckBuf.setPos(0);
  } catch (const std::exception & exc) {
    smsc_log_fatal(_logger, "%s: --> ChargeSmsResult serialization exception - %s",
                   _logId, exc.what());
    srlzRes = false;
  }
  if (srlzRes) {
    if (_wrkMgr->sendPck(pckBuf))
      return true;

    CustomException sndExc;
    srlzRes = _wrkMgr->hasExceptionOnSend(&sndExc);
    smsc_log_error(_logger, "%s: --> ChargeSmsResult sending failure: %s", _logId,
                    srlzRes ? sndExc.what() : "");
  }
  return false;
}

//Returns true if query is started, so execution will continue in another thread.
bool Billing::startIAPQuery(void)
{
  unsigned i = _curIAPrvd;
  bool prvdQueried = false;

  while (i < AbonentPolicy::iapSecondary) {
    const IAProviderInfo * pPrvd = _iapRule._iaPolicy->getIAProvider(static_cast<IAPPrio_e>(++i));
    if (pPrvd) {
      _curIAPrvd = static_cast<IAPPrio_e>(i);

      if ((prvdQueried = pPrvd->_iface->startQuery(_abNumber, *this))) {
        if ((prvdQueried = startTimer(_cfg->abtTimeout)))
          break; //problems with timer facility
        
        while (pPrvd && !pPrvd->_iface->cancelQuery(_abNumber, *this)) {
          addRef(refIdItself);
          _sync.wait();
          unRef(refIdItself);
        }
        _billErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
        break;
        /* */
      } else {
        _billErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
        smsc_log_error(_logger, "%s: %s.startQuery(%s) failed!", _logId, pPrvd->_ident.c_str(),
                       _abNumber.getSignals());
      }
    }
  }
  if (prvdQueried)
    addRef(refIdIAProvider);
  return prvdQueried;
}

void Billing::cancelIAPQuery(void)
{
  if (hasRef(refIdIAProvider)) {  //check for pending query to AbonentProvider
    const IAProviderInfo * pPrvd = _iapRule._iaPolicy->getIAProvider(_curIAPrvd);
    while (pPrvd && !pPrvd->_iface->cancelQuery(_abNumber, *this)) {
      addRef(refIdItself);
      _sync.wait();
      unRef(refIdItself);
    }
    unRef(refIdIAProvider);
  }
}

void Billing::doCleanUp(void)
{
  //check for pending query to AbonentProvider
  cancelIAPQuery();
  //release active timers
  for (unsigned i = 0; i < _timers.size(); ++i) {
    if (!_timers[i].empty()) {
      _timers[i]._hdl.Stop();
      smsc_log_debug(_logger, "%s: Released timer[%s] at state(%s)", _logId,
                     _timers[i]._hdl.IdStr(), state2Str());
      _timers[i].clear();
    }
  }
  //abort dialog with IN-platform
  abortCAPSmTask(true);
}

//NOTE: CDR must be finalized, cdrMode != none, returns number of CDRs created
unsigned Billing::writeCDR(void)
{
  unsigned cnt = 0;
  _cdr._contract = static_cast<CDRRecord::ContractType>((unsigned)_abCsi.abType);
  //set the location MSC of charged abonent to last known one
  if (_cdr._chargeType) {  //MT: from delivery report
    if (_cdr._dstMSC.empty() && !_abCsi.vlrNum.empty())
      _cdr._dstMSC = _abCsi.vlrNum.toString();
  } else {                //MO: from chargeReq or abonentProvider
    if (!_abCsi.vlrNum.empty())
      _cdr._srcMSC = _abCsi.vlrNum.toString();
  }
  //separate undetermined IMSI of known abonent from empty IMSI of unknown abonent
  if (_abCsi.abImsi.empty() && !_abCsi.isUnknown()) {
    memset(_cdr._srcIMSI.str, '0', IMSIString::MAX_SZ-1);
    _cdr._srcIMSI.str[IMSIString::MAX_SZ-1] = 0;
  }
  if (!_cdr._inBilled || (_cfg->prm->cdrMode == ChargeParm::cdrALL)) {
    //remove TonNpi for MSCs ids
    TonNpiAddress tna;
    if (tna.fromText(_cdr._srcMSC.c_str()))
      _cdr._srcMSC = tna.getSignals();
    else if ((_cdr._chargeType == CDRRecord::MO_Charge) && _abNumber.interISDN()) {
      smsc_log_warn(_logger, "%s: empty MSC for %s", _logId, _abNumber.toString().c_str());
    }

    if (tna.fromText(_cdr._dstMSC.c_str()))
      _cdr._dstMSC = tna.getSignals();
    else if ((_cdr._chargeType == CDRRecord::MT_Charge) && _abNumber.interISDN()) {
      smsc_log_warn(_logger, "%s: empty MSC for %s", _logId, _abNumber.toString().c_str());
    }
    _cfg->bfs->bill(_cdr); cnt++;
    smsc_log_info(_logger, "%s: TDR written: msgId: %llu, status: %u, IN billed: %s, charged: %s",
                  _logId, _cdr._msgId, _cdr._dlvrRes, _cdr._inBilled ? "true": "false",
                  _abNumber.toString().c_str());
  }
  return cnt;
}

bool Billing::doFinalize(void)
{
  unsigned cdrs = 0;
  if (_cfg->prm->cdrMode && isBillComplete() && _cfg->bfs.get())
    cdrs = writeCDR(); //Note: interacts with OS FileSystem, may take time

  smsc_log_info(_logger, "%s: %scomplete(%s, %s), %s --> %s(cause %u),"
                        " abonent(%s), type %s, TDR(s) written: %u", _logId,
          isBillComplete() ? "" : "IN", _cdr._chargeType ? "MT" : "MO",
          _cdr.nmPolicy(), _cdr.dpType().c_str(),
          _cdr._inBilled ? _cfgScf->Ident() : _cfg->prm->billModeStr(_billMode), _billErr,
          _abNumber.getSignals(), _abCsi.type2Str(), cdrs);

  doCleanUp();
  if (_pState != bilAborted)
    _pState = bilComplete;
  return true;
}

//FSM switching:
//  entry:  [ any state ]
//  return: [ bilAborted, bilReported ]
void Billing::abortThis(const char * reason/* = NULL*/)
{
  smsc_log_error(_logger, "%s: Aborting at state %s%s%s",
                 _logId, state2Str(), reason ? ", reason: " : "", reason ? reason : "");
  if (_pState < bilReported)
    _pState = Billing::bilAborted;
}


//
RCHash Billing::startCAPSmTask(void)
{
  if (_msgType == ChargeParm::msgSMS) { // _billMode == ChargeParm::bill2IN
    smsc_log_warn(_logger, "%s: %s(%s, %s): double CDR is also created by'%s' for %s",
                  _logId, _cdr.dpType().c_str(), _cdr._chargeType ? "MT" : "MO",
                  _cdr.nmPolicy(), _cfgScf->_ident.c_str(), _cdr._srcAdr.c_str());
  }

  _capSched = _cfg->schedMgr->getScheduler((_cfgScf->_prm->_capSms.idpReqMode == INParmsCapSms::idpReqMT) ? 
                      TaskSchedulerITF::schedMT : TaskSchedulerITF::schedSEQ);
  if (!_capSched) {
    smsc_log_error(_logger, "%s: TaskScheduler is not srarted", _logId);
    return _RCS_INManErrors->mkhash(INManErrorId::internalError);
  }
  CAPSmTaskCFG        cfgSS7(*(_cfg->prm->capSms.get()), _cfg->tcDisp, _cfgScf);
  const GsmSCFinfo *  smScf = _abCsi.getSCFinfo(UnifiedCSI::csi_MO_SM);
  std::auto_ptr<CAPSmTaskAC> smTask;

  if (_cfgScf->_prm->_capSms.idpReqMode == INParmsCapSms::idpReqMT)
    smTask.reset(new CAPSmTaskMT(_abNumber, cfgSS7, smScf->serviceKey, CAPSmTaskAC::idpMO, _logger));
  else
    smTask.reset(new CAPSmTaskSQ(_abNumber, cfgSS7, smScf->serviceKey, CAPSmTaskAC::idpMO, _logger));

  //determine number of required CAP dialogs and corresponding destination addresses
  if (!_xsmsSrv || _xsmsSrv->chargeBearer) {
    TonNpiAddress dstAdr;
    if (_cdr._chargeType) {  //MT
      //charge target abonent as if it attempts to send a SMS
      if (!dstAdr.fromText(_cdr._srcAdr.c_str())) {
        smsc_log_error(_logger, "%s: invalid Call.Adr '%s'", _logId, _cdr._srcAdr.c_str());
        return _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData);
      }
    } else {                //MO
      if (!dstAdr.fromText(_cdr._dstAdr.c_str())) {
        smsc_log_error(_logger, "%s: invalid Dest.Adr '%s'", _logId, _cdr._dstAdr.c_str());
        return _RCS_INManErrors->mkhash(INManErrorId::protocolInvalidData);
      }
    }
    smTask->enqueueDA(dstAdr);
  }
  if (_xsmsSrv)
    smTask->enqueueDA(_xsmsSrv->adr);
  
  RCHash rval = 0;
  try { //compose InitialDPSMS argument
    switch (_cfgScf->_prm->_capSms.idpLiAddr) {
    case INParmsCapSms::idpLiSSF: {
        smTask->Arg().setLocationInformationMSC(cfgSS7.ownAddr);
        if (!_cfgScf->_prm->_capSms._cellGId.empty())
          smTask->Arg().setCellGlobalId(_cfgScf->_prm->_capSms._cellGId);
    } break;
    case INParmsCapSms::idpLiSMSC:
        smTask->Arg().setLocationInformationMSC(_csInfo.smscAddress.c_str()); break;
    default:
      if (!_abCsi.vlrNum.empty())
        smTask->Arg().setLocationInformationMSC(_abCsi.vlrNum);
    }
    if (!_abCsi.abImsi.empty())
      smTask->Arg().setIMSI(_abCsi.abImsi);
    smTask->Arg().setSMSCAddress(_csInfo.smscAddress.c_str());
    smTask->Arg().setTimeAndTimezone(_cdr._submitTime);
    smTask->Arg().setTPShortMessageSpecificInfo(_csInfo.tpShortMessageSpecificInfo);
    smTask->Arg().setTPValidityPeriod(_csInfo.tpValidityPeriod, smsc::inman::comp::tp_vp_relative);
    smTask->Arg().setTPProtocolIdentifier(_csInfo.tpProtocolIdentifier);
    smTask->Arg().setTPDataCodingScheme(_csInfo.tpDataCodingScheme);

  } catch (const CustomException & c_exc) {
    smsc_log_error(_logger, "%s: %s", _logId, c_exc.what());
    rval = (RCHash)(c_exc.errorCode());
  } catch (const std::exception& exc) {
    smsc_log_error(_logger, "%s: %s", _logId, exc.what());
    rval = _RCS_TC_Dialog->mkhash(INManErrorId::protocolInvalidData);
  }
  if (!rval) {
    if (!(_capTask = _capSched->StartTask(smTask.get(), this))) {
      smsc_log_error(_logger, "%s: Failed to start %s", _logId, smTask->TaskName());
      rval = _RCS_TC_Dialog->mkhash(INManErrorId::logicTimedOut);
    } else {
      _pState = Billing::bilInited;
      _capName = smTask->TaskName();
      smsc_log_debug(_logger, "%s: Initiated %s", _logId, _capName.c_str());
      smTask.release();
      addRef(refIdCapSmTask);
      //execution will continue in onTaskReport() or in onTimerEvent() by another thread.
      if (!startTimer(_cfg->maxTimeout)) { //fatal problems
        abortCAPSmTask();
        rval = _RCS_INManErrors->mkhash(INManErrorId::internalError);
      }
    }
  }
  return rval; //!rval means Billing::pgCont;
}

//NOTE: _sync should be locked upon entry!
bool Billing::startTimer(const TimeoutHDL & tmo_hdl)
{
  TimerInfo * pTmr = NULL;
  if (!_timers.alloc(_pState, pTmr)) {
    smsc_log_fatal(_logger, "%s: timer[%s] is already active at state %s", _logId,
                   pTmr->_hdl.IdStr(), state2Str());
    return false;
  }
  OPAQUE_OBJ            timerArg((unsigned)_pState);
  TimeWatcherITF::Error tErr = TimeWatcherITF::errBadTimer;

  pTmr->_hdl = tmo_hdl.CreateTimer(this, &timerArg);
  if (pTmr->_hdl.empty()) {
    smsc_log_fatal(_logger, "%s: failed to allocate timer at state '%s'", _logId,
                   state2Str());
  } else {
    if ((tErr = pTmr->_hdl.Start()) == TimeWatcherITF::errOk) {
      smsc_log_debug(_logger, "%s: started timer[%s] at state %s", _logId,
                     pTmr->_hdl.IdStr(), state2Str());
      pTmr->_wrkGrd = _wrkMgr->getWorkerGuard(*this);
      return true;
    }
    smsc_log_fatal(_logger, "%s: failed to start timer[%s]:%u", _logId,
                   pTmr->_hdl.IdStr(), _pState);
  }
  pTmr->clear();
  return false;
}

//NOTE: _sync should be locked upon entry!
void Billing::stopTimer(Billing::BillingState bil_state)
{
  TimerInfo * pTmr = _timers.find(bil_state);
  if (pTmr) {
    smsc_log_debug(_logger, "%s: releasing timer[%s]:%s at state(%s)", _logId,
                   pTmr->_hdl.IdStr(), state2Str(bil_state), state2Str());
    pTmr->_hdl.Stop();
    pTmr->clear(); //WorkerGuard is also released here
  } else {
    smsc_log_warn(_logger, "%s: no active timer for state(%s)", _logId, state2Str());
  }
}

//returns false if PDU contains invalid data preventing request processing
//NOTE: _sync should be locked upon entry!
bool Billing::verifyChargeSms(void)
{
  //determine which abonent should be charged
  if (!_abNumber.fromText(_cdr._chargeType ? _cdr._dstAdr.c_str() : _cdr._srcAdr.c_str())) {
    smsc_log_error(_logger, "%s: invalid %s.Adr '%s'", _logId,
                   _cdr._chargeType ? "Dest" : "Call",
                   _cdr._chargeType ? _cdr._dstAdr.c_str() : _cdr._srcAdr.c_str());
    return false;
  }
  //remember IMSI of abonent is to charge (if present)
  _abCsi.abImsi = _cdr._chargeType ? _cdr._dstIMSI : _cdr._srcIMSI;
  //remember location MSC of abonent is to charge (if present)
  _abCsi.vlrNum.fromText(_cdr._chargeType ? _cdr._dstMSC.c_str() : _cdr._srcMSC.c_str());

  smsc_log_info(_logger, "%s: %s(%s, %s): '%s' -> '%s'", _logId,
                _cdr.dpType().c_str(), _cdr._chargeType ? "MT" : "MO",
                _cdr.nmPolicy(), _cdr._srcAdr.c_str(), _cdr._dstAdr.c_str());

  uint32_t smsXMask = _cdr._smsXMask & ~SMSX_RESERVED_MASK;
  if (smsXMask) {
    if (_cdr._bearer != CDRRecord::dpSMS) {
      smsc_log_error(_logger, "%s: invalid bearer for SMS Extra service", _logId);
      return false;
    }
    if (!_cfg->prm->smsXMap.get()) {
      smsc_log_error(_logger, "%s: SMS Extra services are not configured!", _logId);
      return false;
    }
    SmsXServiceMap::const_iterator it = _cfg->prm->smsXMap->find(smsXMask);
    if (it != _cfg->prm->smsXMap->end()) {
      _xsmsSrv = &(it->second);
      _cdr._serviceId = _xsmsSrv->svcCode;
      smsc_log_info(_logger, "%s: %s[0x%x]: %u, %s%s", _logId,
                    _xsmsSrv->name.empty() ? "SMSExtra service" : _xsmsSrv->name.c_str(),
                    smsXMask, _xsmsSrv->svcCode, _xsmsSrv->adr.toString().c_str(),
                    _xsmsSrv->chargeBearer ? ", chargeBearer" : "");
    } else {
      smsc_log_error(_logger, "%s: SMSExtra service[0x%x] misconfigured, ignoring!",
                     _logId, smsXMask);
      _cdr._smsXMask &= SMSX_RESERVED_MASK;
    }
  }

  if (_xsmsSrv)
    _msgType = ChargeParm::msgXSMS;
  else
    _msgType = (_cdr._bearer == CDRRecord::dpUSSD) ? ChargeParm::msgUSSD : ChargeParm::msgSMS;

  //determine billmode
  _billPrio = _cdr._chargeType ? _cfg->prm->mt_billMode.modeFor(_msgType) : 
                                  _cfg->prm->mo_billMode.modeFor(_msgType);
  _billMode = _billPrio->first;

  //according to #B2501:
/*
    if (_msgType == ChargeParm::msgSMS) {
      //only bill2CDR & billOFF allowed for ordinary SMS
      _cdr._smsXMask &= ~SMSX_INCHARGE_SRV;
      if (_billMode == ChargeParm::bill2IN) {
        smsc_log_error(_logger, "%s: incompatible billingMode and messageType", _logId);
        return false; 
      }
    }
*/
  return true;
}

//NOTE: _sync should be locked upon entry!
Billing::PGraphState Billing::onChargeSms(void)
{
  if ((_billMode == ChargeParm::bill2IN) && !_cfg->prm->capSms.get())
    _billMode = _billPrio->second;

  if (_billMode == ChargeParm::billOFF)
    return chargeResult(false, _RCS_INManErrors->mkhash(INManErrorId::cfgMismatch));

  //Here goes either bill2IN or bill2CDR ..
  if ((_cdr._chargePolicy == CDRRecord::ON_DATA_COLLECTED)
      || (_cdr._chargePolicy == CDRRecord::ON_SUBMIT_COLLECTED))
    _billMode = ChargeParm::bill2CDR;
  else if (_cdr._smsXMask & SMSX_NOCHARGE_SRV)
    _billMode = ChargeParm::bill2CDR;
  else if (_cdr._smsXMask & SMSX_INCHARGE_SRV)
    _billMode = ChargeParm::bill2IN;
  else if (_chrgFlags & ChargeSms::chrgCDR)
    _billMode = ChargeParm::bill2CDR;

  //check for SMS extra sevice number being set
  if ((_xsmsSrv && _xsmsSrv->adr.empty())) {
    _billMode = ChargeParm::bill2CDR;
    _billErr = _RCS_INManErrors->mkhash(INManErrorId::cfgSpecific);
  }

  if (_cfg->abCache) {
    //NOTE: vlrNum doesn't stored in cache
    AbonentSubscription cacheRec;
    _cfg->abCache->getAbonentInfo(_abNumber, &cacheRec);
    _abCsi.Merge(cacheRec);     //merge available abonent info
  }

  bool askProvider = false;
#ifdef SMSEXTRA
  //Special processing for SMSX WEB gateway: 
  // 1) billing mode bill2CDR
  // 2) callBarred state denies processing 
  if (!strcmp(_abCsi.vlrNum.getSignals(), SMSX_WEB_GT)) {
    _billMode = ChargeParm::bill2CDR;
    if (_abCsi.isUnknown())
      askProvider = true; //check for callBarred state
  }
#endif /* SMSEXTRA */

  if (_abCsi.isPostpaid() && _abCsi.getImsi()) {
    _billMode = ChargeParm::bill2CDR;
    //do not interact IN platform, just create CDR
    return chargeResult(true);
  }

  const AbonentPolicy * iapPolicy = determinePolicy();
  const IAProviderInfo *
    prmPrvd = iapPolicy ? iapPolicy->getIAProvider(AbonentPolicy::iapPrimary) : NULL;

  if (!_abCsi.isUnknown()) {
    //check for IMSI being defined
    if (!_abCsi.getImsi() && prmPrvd && prmPrvd->hasAbility(IAPAbility::abIMSI))
      askProvider = true;

    if (_abCsi.isPrepaid())
      configureMOSM(); //check if cache data enough to determine gsmSCF params
  }
  //check if AbonentProvider should be requested for contract type/gsmSCF params

  /* **************************************************** */
  /* conditions which switch ON provider request         */
  /* **************************************************** */
  askProvider |= ((_cfg->prm->cntrReq == ChargeParm::reqAlways)
                  || ( (_billMode == ChargeParm::bill2IN)
                        && (_abCsi.isUnknown() || !getServiceKey(UnifiedCSI::csi_MO_SM)) )
                  );

  //check if AbonentProvider should be requested for current abonent location
  if (_abCsi.vlrNum.empty() && prmPrvd && prmPrvd->hasAbility(IAPAbility::abVLR))
    askProvider = true;

  /* **************************************************** */
  /* conditions which switch OFF IAProvider request       */
  /* **************************************************** */

  //verify that abonent number is in ISDN international format
  if (!_abNumber.interISDN() || (_abNumber.length < 10)) //HOT-PATCH for short SME ISDN numbers
    askProvider = false;

  if (_chrgFlags & ChargeSms::chrgCDR)
    askProvider = false;

  if (askProvider) {
    if (!prmPrvd) {
      _billErr = _RCS_INManErrors->mkhash(INManErrorId::cfgInconsistency);
      smsc_log_error(_logger, "%s: no IAProvider configured for %s", _logId, _abNumber.getSignals());
    } else  if (startIAPQuery()) {
      _pState = bilStarted;
      //execution will continue in onIAPQueried()/onTimerEvent() by another thread.
      return Billing::pgCont;
    }
  }
  return configureSCFandCharge();
}

//Here goes either bill2IN or bill2CDR ..
Billing::PGraphState Billing::configureSCFandCharge(void)
{
  if (_abCsi.isPostpaid()) {
    _billMode = ChargeParm::bill2CDR;
    return chargeResult(true);
  }
  //Here goes either abtPrepaid or abtUnknown ..
  RCHash err = 0;
  if (_billMode == ChargeParm::bill2IN) {
    INManErrorId::Code_e rc = configureSCF();
    if (rc) {
      err = _RCS_INManErrors->mkhash(rc);
      if ((_billMode = _billPrio->second) == ChargeParm::billOFF)
        return chargeResult(false, err);
      //else //billCDR
      smsc_log_info(_logger, "%s: switching to CDR mode for abonent(%s)",
                    _logId, _abNumber.toString().c_str());
    }
  }
  if (_billMode == ChargeParm::bill2IN) {
    err = startCAPSmTask();
    if (!err)
      return pgCont; //awaiting response from IN point

    smsc_log_error(_logger, "%s: %s", _logId, URCRegistry::explainHash(err).c_str());
    if ((_billMode = _billPrio->second) == ChargeParm::billOFF)
      return chargeResult(false, err);
    //else //billCDR
    smsc_log_info(_logger, "%s: switching to CDR mode for abonent(%s)",
                  _logId, _abNumber.toString().c_str());
  }
  //_billMode == ChargeParm::bill2CDR
  return chargeResult(true, err);
}

INManErrorId::Code_e Billing::configureSCF(void)
{
  configureMOSM();
  const GsmSCFinfo * smScf = _abCsi.getSCFinfo(UnifiedCSI::csi_MO_SM);

  //verify gsmSCF & serviceKey
  if (smScf && !smScf->serviceKey) {
    smsc_log_error(_logger, "%s: misconfigured serviceKey(%s) for gsmSCF(%s)", _logId,
                   UnifiedCSI::nmTDP(UnifiedCSI::csi_MO_SM),
                   smScf->scfAddress.toString().c_str());
    return INManErrorId::cfgInconsistency;
  }
  if (!smScf) {
    smsc_log_error(_logger, "%s: unable to determine gsmSCF for abonent(%s)", _logId,
                   _abNumber.toString().c_str());
    return INManErrorId::cfgInconsistency;
  }

  //verify IMSI
  if (!_abCsi.getImsi() && _cfgScf && !_cfgScf->_prm->_dfltImsi.empty())
    _abCsi.setImsi(_cfgScf->_prm->_dfltImsi.c_str());
  if (!_abCsi.getImsi()) {
    smsc_log_warn(_logger, "%s: unable to determine IMSI for abonent(%s)", _logId,
                   _abNumber.toString().c_str());
//    smsc_log_error(_logger, "%s: unable to determine IMSI for abonent(%s)", _logId,
//                   _abNumber.toString().c_str());
//    return INManErrorId::cfgInconsistency;
  }

  //verify abonent location MSC
  if (_abCsi.vlrNum.empty()) {
    smsc_log_warn(_logger, "%s: unable to determine location MSC for abonent(%s)",
                   _logId, _abNumber.toString().c_str());
/*
    smsc_log_error(_logger, "%s: unable to determine location MSC for abonent(%s)",
                   _logId, _abNumber.toString().c_str());
    return INManErrorId::cfgInconsistency;
*/
  }
  return INManErrorId::noErr;
}

//Adjusts the MO-SM gsmSCF parameters combining cache/IAProvider CSIs
//and gsmSCF parameters from config.xml
void Billing::configureMOSM(void)
{
  uint32_t          keyMOSM = 0;
  const CSIRecord * pCsi = _abCsi.csiSCF.getCSIRecord(UnifiedCSI::csi_MO_SM);

  if (pCsi)
    keyMOSM = pCsi->scfInfo.serviceKey;
  else //check if SCF for MO-BC may be used
    pCsi = _abCsi.csiSCF.getCSIRecord(UnifiedCSI::csi_O_BC);

  //check if MO_SM parameters are configured for serving gsmSCF
  //serviceKey from config.xml has a higher priority
  if (!_cfgScf && pCsi && determinePolicy())
    _cfgScf = _iapRule._iaPolicy->getSCFparms(pCsi->scfInfo.scfAddress);

  if (_cfgScf) {
    uint32_t  keyCfg = _cfgScf->getSKey(UnifiedCSI::csi_MO_SM, &_abCsi.csiSCF);

    if (keyCfg && (keyCfg != keyMOSM)) { //update/create MO_SM record
      CSIRecord & csiRec = _abCsi.csiSCF[UnifiedCSI::csi_MO_SM];
      csiRec.iapId = IAPProperty::iapUnknown;
      csiRec.scfInfo.serviceKey = keyCfg;
      if (pCsi->csiId != UnifiedCSI::csi_MO_SM)
        csiRec.scfInfo.scfAddress = pCsi->scfInfo.scfAddress;
    }
  }
}

uint32_t Billing::getServiceKey(CSIUid_e csi_id) const
{
  if (_cfgScf) //serviceKey from config.xml has a higher priority
    return _cfgScf->getSKey(csi_id, &_abCsi.csiSCF);

  const CSIRecord * pCsi = _abCsi.csiSCF.getCSIRecord(csi_id);
  return pCsi ? pCsi->scfInfo.serviceKey : 0;
}

const AbonentPolicy * Billing::determinePolicy(void)
{
  if (!_iapRule._iaPolicy && _cfg->iapMgr) {
    //determine policy rule
    const IAPRule * pRule = _cfg->iapMgr->getPolicy(_abNumber);
    if (pRule) {
      _iapRule = *pRule;
    } else {
      _iapRule._nmPool = "<default>";
      _iapRule._iaPolicy = _cfg->iapMgr->getPolicy(_cfg->policyNm);
    }
    if (_iapRule._iaPolicy) {
      smsc_log_debug(_logger, "%s: using policy %s for %s", _logId, _iapRule.toString().c_str(),
                      _abNumber.toString().c_str());
    } else {
      smsc_log_error(_logger, "%s: no IAPolicy configured for %s", _logId, _abNumber.getSignals());
    }
  }
  return _iapRule._iaPolicy;
}

//NOTE: _sync should be locked upon entry
//FSM switching:
//  entry:  billContinued
//  return: [ bilAborted, bilSubmitted, bilReported ]
Billing::PGraphState Billing::onDeliverySmsResult(void)
{
  RCHash rval = 0;
  smsc_log_info(_logger, "%s: <-- DELIVERY_%s (code: %u)", _logId,
                  (!_cdr._dlvrRes) ? "SUCCEEDED" : "FAILED", _cdr._dlvrRes);
  
  if (_capTask) { //report message submission to SCF
    TaskSchedulerITF::SchedulerRC
      schedRc = _capSched->SignalTask(_capTask, TaskSchedulerITF::sigProc,
                                     !_cdr._dlvrRes ? &_SMSubmitOK : &_SMSubmitNO);
    if (TaskSchedulerITF::rcOk == schedRc) {
      _pState = bilSubmitted;
      //execution will continue either in onTaskReport() or in onTimerEvent()
      startTimer(_cfg->maxTimeout);
      return Billing::pgCont;
    } //else task cann't be interacted!
    smsc_log_error(_logger, "%s: sending signal to %s is failed: %s(%u) ", _logId,
                   _capName.c_str(), _capSched->nmRCode(schedRc), (unsigned)schedRc);
    unrefCAPSmTask(false);
    rval = _RCS_INManErrors->mkhash(INManErrorId::internalError);
  }
  onSubmitReport(rval);
  return Billing::pgEnd;
}

//NOTE: _sync should be locked upon entry
//FSM switching:
//  entry:  [ bilStarted, bilQueried, bilInited ]
//  return: [ bilReleased, bilContinued ]
Billing::PGraphState Billing::chargeResult(bool do_charge, RCHash last_err /* = 0*/)
{
  _pState = do_charge ? Billing::bilContinued : Billing::bilReleased;

  if (do_charge && !((_cdr._chargePolicy == CDRRecord::ON_DATA_COLLECTED)
                     || (_cdr._chargePolicy == CDRRecord::ON_SUBMIT_COLLECTED))) {
    //DeliverySmsResult is awaited
    if (!startTimer(_cfg->maxTimeout)) { //fatal problems, reject charging
      last_err = _RCS_INManErrors->mkhash(INManErrorId::internalError);
      do_charge = false;
      abortCAPSmTask(false);
    }
  }

  if (last_err)
    _billErr = last_err;

  if (_logger->isInfoEnabled()) {
    std::string reply = !do_charge ? 
                  format("NOT_POSSIBLE (cause %u", _billErr) :
                  format("POSSIBLE (via %s, cause %u", (_billMode == ChargeParm::bill2CDR) ?
                         "CDR" : _cfgScf->Ident(), _billErr);
    if (_billErr) {
      reply += ": ";
      reply += URCRegistry::explainHash(_billErr);
    }
    reply += ")";
    _logger->log_(smsc::logger::Logger::LEVEL_INFO, 
                 "%s: --> %s %s CHARGING_%s, abonent(%s) type: %s (%u)",
                 _logId, _cdr.dpType().c_str(), _cdr._chargeType ? "MT" : "MO",
                 reply.c_str(), _abNumber.getSignals(),
                 _abCsi.type2Str(), (unsigned)_abCsi.abType);
  }

  if ((_cdr._chargePolicy == CDRRecord::ON_DATA_COLLECTED)
      || (_cdr._chargePolicy == CDRRecord::ON_SUBMIT_COLLECTED))
    return do_charge ? onDeliverySmsResult() : Billing::pgEnd;

  SPckChargeSmsResult res;
  res._Cmd.setValue(do_charge ? ChargeSmsResult::CHARGING_POSSIBLE : 
                              ChargeSmsResult::CHARGING_NOT_POSSIBLE);
  _cdr._contract = static_cast<CDRRecord::ContractType>((unsigned)_abCsi.abType);
  res._Cmd.setContract(_cdr._contract);
  if (_billErr)
    res._Cmd.setError(_billErr, URCRegistry::explainHash(_billErr).c_str());
  res._Hdr.dlgId = _wrkId;

  if (sendCmd(res))
    return do_charge ? Billing::pgCont : Billing::pgEnd;

  abortThis();
  return Billing::pgEnd;
}

//NOTE: _sync should be locked upon entry
//FSM switching:
//  entry:  billSubmitted
//  return: [ bilAborted, bilReported ]
void Billing::onSubmitReport(RCHash scf_err, bool in_billed/* = false*/)
{
  bool submitted = _cdr._dlvrRes ? false : true;
  bool smscAborted = ((_cdr._chargePolicy == CDRRecord::ON_SUBMIT) && _cdr._dlvrRes);

  if (scf_err) {
    _billErr = scf_err;
    //if message has been already delivered, then create CDR
    //even if secondary billing mode is OFF
    if (submitted) 
      _billMode = ChargeParm::bill2CDR;
    smsc_log_error(_logger, "%s: %s%s interaction failure: %s",
                   _logId, submitted ? "switching to CDR mode: " : "", _cfgScf->Ident(),
                   URCRegistry::explainHash(scf_err).c_str());
  } else
    _cdr._inBilled = in_billed;

  _pState = smscAborted ? bilAborted : bilReported;
}

//Returns false if task currently awaits for Billing::_sync to call onTaskReport()
bool Billing::unrefCAPSmTask(bool wait_report/* = true */)
{
  bool rval = _capSched->UnrefTask(_capTask, this);
  if (rval) {
    unRef(refIdCapSmTask);
  } else if (wait_report) {
    //task currently awaits for Billing::_sync, so pass control to onTaskReport()
    addRef(refIdItself);
    int res = _capEvent.WaitOn(_sync, _cfg->prm->capSms->rosTimeout);
    unRef(refIdItself);
    if (!res)
      rval = true;
    else {
      smsc_log_error(_logger, "%s: %s unbind failed, code %u", _logId,
                     _capName.c_str(), res);
    }
  }
  return rval;
}

void Billing::abortCAPSmTask(bool wait_report/* = true */)
{
  if (_capTask) {
    unrefCAPSmTask(wait_report);
    _capSched->AbortTask(_capTask);
    _capTask = 0;
  }
}

/* -------------------------------------------------------------------------- *
 * TimerListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks _sync !!!
TimeWatcherITF::SignalResult
    Billing::onTimerEvent(const TimerHdl & tm_hdl, OPAQUE_OBJ * opaque_obj)
{
  bool wDone = false;
  {
    MutexTryGuard grd(_sync);
    if (!grd.tgtLocked()) //billing is busy, request resignalling
      return TimeWatcherITF::evtResignal;

    smsc_log_debug(_logger, "%s: timer[%s] signaled, states: %u -> %u",
                   _logId, tm_hdl.IdStr(), opaque_obj->val.ui, (unsigned)_pState);

    TimerInfo * pInf = _timers.find(static_cast<BillingState>(opaque_obj->val.ui));
    if (pInf) {
      pInf->clear(); //deletes handle (unrefs timer), releases WorkerGuard
    } else {
      smsc_log_warn(_logger, "%s: timer[%s] signaled, but isn't registered",
                     _logId, tm_hdl.IdStr());
    }

    if (opaque_obj->val.ui == (unsigned)_pState) {
      //target operation doesn't complete yet.
      switch (_pState) {
      case Billing::bilStarted: { //abonent provider query is expired
        cancelIAPQuery();
        //check if next IAProvider may ne requested
        if (!startIAPQuery()) {
          if (Billing::pgEnd == configureSCFandCharge())
            wDone = doFinalize();
        } //else: keep bilStarted state
      } break;

      case Billing::bilInited: { //CapSMTask lasts too long
        //CapSMTask suspends while awaiting Continue/Release from SCF
        bool doCharge = ((_billMode = _billPrio->second) 
                              == ChargeParm::billOFF) ? false : true;
        smsc_log_error(_logger, "%s: %s%s is timed out (RRSM)", _logId, 
                     doCharge ? "switching to CDR mode: " : "", _capName.c_str());
        abortCAPSmTask();
        if (Billing::pgEnd == chargeResult(doCharge,
                  _RCS_INManErrors->mkhash(INManErrorId::logicTimedOut)))
          wDone = doFinalize();
      } break;

      case Billing::bilContinued: { //SMSC doesn't respond with DeliveryResult
        abortThis("SMSC DeliverySmsResult is timed out");
        wDone = doFinalize();
      } break;

      case Billing::bilSubmitted: { //CapSMTask suspends while reporting submission to SCF
        smsc_log_error(_logger, "%s: %s is timed out (ERSM)", _logId, _capName.c_str());
        abortCAPSmTask();
        onSubmitReport(_RCS_INManErrors->mkhash(INManErrorId::logicTimedOut));
        wDone = doFinalize();
      } break;

      default:;
      } //eosw
    } //else: state, guarded by timer, is already finished

    if (!wDone)
      return TimeWatcherITF::evtOk;
  }
  _wrkMgr->workerDone(*this);
  return TimeWatcherITF::evtOk;
}

/* -------------------------------------------------------------------------- *
 * IAPQueryListenerITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: it's the processing graph entry point, so locks _sync !!!
bool Billing::onIAPQueried(const AbonentId & ab_number, const AbonentSubscription & ab_info,
                            RCHash qry_status)
{
  bool wDone = false;
  {
    MutexGuard grd(_sync);
  
    if (_pState > bilStarted) {
      smsc_log_warn(_logger, "%s: onIAPQueried() at state: %u", _logId, _pState);
      goto _ret_Unlock;
    }
    stopTimer(_pState);
  
    if (qry_status) {
      _billErr = qry_status;
      if (startIAPQuery()) //check if next IAProvider may ne requested
        goto _ret_Unlock;  //keep bilStarted state
    } else {
  #ifdef SMSEXTRA
      //Special processing in case of SMSX WEB gateway
      if (!strcmp(_abCsi.vlrNum.getSignals(), SMSX_WEB_GT)) {
        TonNpiAddress orgVLR = _abCsi.vlrNum;
  
        _abCsi.Merge(ab_info); //merge known abonent info
        if (!ab_info.vlrNum.empty()) {
          //Keep MSC address in case of special value reserved for SMSX WEB gateway
          _abCsi.vlrNum = orgVLR;
        }
      } else
  #endif /* SMSEXTRA */
        _abCsi.Merge(ab_info); //merge known abonent info
  
      //Forbid billing in case of barred state
      //NOTE: Barring data doesn't stored in cache!
      if (ab_info.odbGD.hasBit(ODBGeneralData::bit_allOG_CallsBarred)) {
        _pState = bilQueried;
        smsc_log_info(_logger, "%s: allOG_CallsBarred is set for %s", _logId,
                      _abNumber.toString().c_str());
        if (chargeResult(false, _RCS_MAPOpErrors->mkhash(MAPOpErrorId::callBarred)) == Billing::pgEnd)
          wDone = doFinalize();
        goto _ret_Unlock;
      }
  
      //NOTE: Migration to ATSI - in case of postpaid abonent check if secondary
      //      provider able to provide IMSI
      if (_abCsi.isPostpaid() && !_abCsi.getImsi() && nextIAProviderHas(IAPAbility::abIMSI)) {
        if (startIAPQuery())
          goto _ret_Unlock; //keep bilStarted state
      }
  
      if (_cfg->abCache) {
        try {
          _cfg->abCache->setAbonentInfo(_abNumber, _abCsi); //throws
        } catch (const std::exception & exc) {
          smsc_log_error(_logger, "%s: abCache exception: %s", _logId, exc.what());
        }
      }
    }
    _pState = bilQueried;
    if (configureSCFandCharge() == Billing::pgEnd)
      wDone = doFinalize();

_ret_Unlock:
    unRef(refIdIAProvider);
    if (!wDone) //this worker continues processing
      return true;
  }
  _wrkMgr->workerDone(*this);
  return true;
}

/* -------------------------------------------------------------------------- *
 * TaskRefereeITF interface implementation:
 * -------------------------------------------------------------------------- */
//NOTE: whithin this method _capSched->UnrefTask() always returns false
//      because of Billing is already targeted by task for reporting.
void Billing::onTaskReport(TaskSchedulerITF * sched, const ScheduledTaskAC * task)
{
  bool wDone = false;
  {
    MutexGuard grd(_sync);
    const CAPSmTaskAC * sm_res = static_cast<const CAPSmTaskAC *>(task);
    RCHash capErr = sm_res->scfErr;
  
    if (_pState == bilInited) {
      stopTimer(_pState);
      bool doCharge = sm_res->doCharge;
      bool scfCharge = sm_res->doCharge;
  
      if (sm_res->curPMode() != CAPSmTaskAC::pmInstructing) {
        smsc_log_error(_logger, "%s: %s dissynchonization: %s <- %s", _logId,
                       _capName.c_str(), state2Str(), sm_res->nmPMode());
        scfCharge = false;
        capErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
        if (_billPrio->second == ChargeParm::bill2CDR) {
            smsc_log_error(_logger, "%s: switching to CDR mode: %s interaction failure",
                           _logId, _cfgScf->Ident());
            _billMode = ChargeParm::bill2CDR;
            doCharge = true;
        } else
            doCharge = false;
      } else if (!sm_res->doCharge && !sm_res->rejectRPC
            && (_billPrio->second == ChargeParm::bill2CDR)) {
        smsc_log_error(_logger, "%s: switching to CDR mode: %s interaction failure",
                    _logId, _cfgScf->Ident());
        _billMode = ChargeParm::bill2CDR;
        doCharge = true;
      }
      if (!scfCharge) { //CapSMS task will be released by itself
        unrefCAPSmTask(false);
        _capTask = 0; //omit CAPTask releasing while cleanUp phase
      }
      if (Billing::pgEnd == chargeResult(doCharge, capErr))
        wDone = doFinalize();
    } else if (_pState == bilSubmitted) {
      //submission status was reported to SCF
      stopTimer(_pState);
      if (sm_res->curPMode() != CAPSmTaskAC::pmAcknowledging) {
        smsc_log_error(_logger, "%s: %s dissynchonization: %s <- %s", _logId,
                       _capName.c_str(), state2Str(), sm_res->nmPMode());
        capErr = _RCS_INManErrors->mkhash(INManErrorId::internalError);
      }
      if (capErr) {
        //Note1: it's unknown whether the SCF finalized transaction or 
        //      not, so unconditionally create TDR and rise in_billed flag
        smsc_log_error(_logger, "%s: switching to CDR mode: %s interaction failure",
                       _logId, _cfgScf->Ident());
        _billMode = ChargeParm::bill2CDR;
      }
      unrefCAPSmTask(false);
      _capTask = 0; //omit CAPTask releasing while cleanUp phase
      //Note2: considering Note1 rise in_billed flag despite of capErr value
      onSubmitReport(capErr, true);
      wDone = doFinalize();
    } else if (_pState == bilContinued) {
      //abnormal CAPSmTask termination, charging was allowed so create TDR
      //despite of secondary billmode setting
      unrefCAPSmTask(false);
      smsc_log_error(_logger, "%s: switching to CDR mode: %s interaction failure: %s",
                  _logId, _cfgScf->Ident(), URCRegistry::explainHash(capErr).c_str());
      _billMode = ChargeParm::bill2CDR;
      _billErr = sm_res->scfErr;
    } else {
      smsc_log_warn(_logger, "%s: %s reported: %s <- %s", _logId,
                    _capName.c_str(), state2Str(), sm_res->nmPMode());
      if ((sm_res->curPMode() == CAPSmTaskAC::pmInstructing) && sm_res->doCharge) {
        //dissynchonization -> abort CAPSmTask
        abortCAPSmTask(false);
      }
    }

//_ret_Unlock:
    unRef(refIdCapSmTask);
    if (!wDone)
      return;
  }
  _wrkMgr->workerDone(*this);
  return;
}

} //smbill
} //inman
} //smsc

