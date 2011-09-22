#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapSRI/IAPQuerySRI.hpp"

#include "inman/inap/HDSSnSession.hpp"
using smsc::inman::inap::SSNSession;

#include "inman/comp/map_chsri/MapCHSRIFactory.hpp"
using smsc::inman::comp::_ac_map_locInfoRetrieval_v3;
using smsc::inman::comp::chsri::initMAPCHSRI3Components;

using smsc::util::URCRegistry;

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

using smsc::inman::comp::UnifiedCSI;
using smsc::inman::iaprvd::CSIRecord;

/* ************************************************************************** *
 * class IAPQuerySRI implementation:
 * ************************************************************************** */
const IAPQuerySRI::TypeString_t IAPQuerySRI::_qryType("IAPQuerySRI");

// -------------------------------------------------------
// -- IAPQueryAC interface methods
// -------------------------------------------------------
//Starts query execution, in case of success switches FSM to qryStarted state,
//otherwise to qryDone state.
IAPQueryAC::ProcResult_e IAPQuerySRI::start(void) /*throw()*/
{
  //MAP_SRI serves only ISDN International numbers
  if (!_abId.interISDN()) {
    _qStatus = IAPQStatus::iqBadArg;
    setStageNotify(qryDone);
    return IAPQueryAC::procNeedReport;
  }

  try {
    _mapDlg.init().init(*_cfg._mapSess, *this, _logger);
    _mapDlg->reqRoutingInfo(_abId, _cfg._tmoSecs); //throws
    /* - */
  } catch (const std::exception & exc) {
    smsc_log_error(_logger, "%s(%s): %s", taskName(), _abId.getSignals(), exc.what());
    _qStatus = IAPQStatus::iqError;
    _exc = exc.what();
    if (_mapDlg.get())
      _mapDlg->unbindUser();
    setStageNotify(qryDone);
    return IAPQueryAC::procNeedReport;
  }
  setStage(qryStarted);
  _qStatus = IAPQStatus::iqOk;
  return IAPQueryAC::procOk;
}

//Blocks until all used resources are released.
//Called only at qryStoppping state.
IAPQueryAC::ProcResult_e IAPQuerySRI::finalize(void) /*throw()*/
{
  while (_mapDlg.get() && !_mapDlg->unbindUser())
    wait();  //sleep while MAPDlg refers this query
  
  if (_mapDlg.get())
    _mapDlg->endDialog(); //end dialog if it's still active, release TC Dialog

  _mapDlg.clear();
  return IAPQueryAC::procOk;
}

// -------------------------------------------------------
// -- CHSRIhandlerITF interface methods
// -------------------------------------------------------
//FSM switching:: qryStarted -> qryResulted
void IAPQuerySRI::onMapResult(CHSendRoutingInfoRes & res)
{
  MutexGuard  grd(*this);
  if (getStage() > qryStarted) {
    smsc_log_warn(_logger, "%s(%s): onMapResult() at stage %s", taskName(),
                  _abId.getSignals(), nmStage());
    return;
  }

  if (!res.getIMSI(_abInfo.abImsi)) //abonent is unknown
    smsc_log_warn(_logger, "%s(%s): IMSI not determined.", taskName(), _abId.getSignals());
  else {
    //NOTE: CH-SRI returns only O-Bcsm tDP serviceKeys
    if (!res.hasOCSI()) {
      _abInfo.abType = AbonentContractInfo::abtPostpaid;
      smsc_log_debug(_logger, "%s(%s): %s", taskName(), _abId.getSignals(),
                     _abInfo.toString().c_str());
    } else {
      _abInfo.abType = AbonentContractInfo::abtPrepaid;
      CSIRecord & csiRec = _abInfo.csiSCF[UnifiedCSI::csi_O_BC];
      csiRec.iapId = IAPProperty::iapCHSRI;
      csiRec.csiId = UnifiedCSI::csi_O_BC;
      res.getSCFinfo(&csiRec.scfInfo);
      res.getVLRN(_abInfo.vlrNum);
      smsc_log_debug(_logger, "%s(%s): %s", taskName(), _abId.getSignals(),
                     _abInfo.toString().c_str());
    }
  }
  setStage(qryResulted);
}

//FSM switching: [qryStarted | qryResulted] -> qryReporting
//               qryStopping -> qryStopping
ObjAllcStatus_e
  IAPQuerySRI::onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash err_code/* = 0*/)
{
  {
    MutexGuard  grd(*this);
    if (getStage() > qryReporting) {
      smsc_log_warn(_logger, "%s(%s): onDialogEnd() at stage %u, code 0x%x(%s)",
                     taskName(), _abId.getSignals(), (unsigned )getStage(), err_code,
                     err_code ? URCRegistry::explainHash(err_code).c_str()
                     : "Ok");
      return ObjFinalizerIface::objActive;
    }
  
    if (err_code) {
      _qStatus = IAPQStatus::iqError;
      _qError = err_code;
      _exc = URCRegistry::explainHash(err_code);
      smsc_log_error(_logger, "%s(%s): query failed: code 0x%x(%s)",
                      taskName(), _abId.getSignals(), err_code, _exc.c_str());
    } else
      smsc_log_debug(_logger, "%s(%s): query succeeded",
                      taskName(), _abId.getSignals());
  
    setStageNotify(qryReporting);
  }
  reportThis();
  return ObjFinalizerIface::objActive;
}

/* ************************************************************************** *
 * class IAPQueriesPoolSRI implementation:
 * ************************************************************************** */

// ------------------------------------------
// -- IAPQueriesPoolIface interface methods
// ------------------------------------------
IAPQueryRef IAPQueriesPoolSRI::allcQuery(void)
{
  IAPQueryRef pQry = mObjPool.allcObj();
  if (pQry.empty()) {
    smsc_log_error(mLogger, "%s: pool is exhausted: %u of %u", IAPQuerySRI::_qryType.c_str(),
                   (unsigned)mObjPool.usage(), (unsigned)mObjPool.capacity());
  } else {
    (static_cast <IAPQuerySRI*>(pQry.get()))->configure(mCfg, mLogger);
  }
  return pQry;
}


} //sri
} //iaprvd
} //inman
} //smsc

