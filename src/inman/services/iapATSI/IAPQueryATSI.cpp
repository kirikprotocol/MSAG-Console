#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapATSI/IAPQueryATSI.hpp"

#include "inman/inap/HDSSnSession.hpp"
using smsc::inman::inap::SSNSession;

#include "inman/comp/map_atih/MapATIHFactory.hpp"
using smsc::inman::comp::_ac_map_anyTimeInfoHandling_v3;
using smsc::inman::comp::atih::initMAPATIH3Components;

#include "inman/comp/MapOpErrors.hpp"
using smsc::inman::comp::_RCS_MAPOpErrors;
using smsc::inman::comp::MAPOpErrorId;

using smsc::util::URCRegistry;

namespace smsc {
namespace inman {
namespace iaprvd {
namespace atih {
/* ************************************************************************** *
 * class IAPQueryATSI implementation:
 * ************************************************************************** */
const IAPQueryATSI::TypeString_t IAPQueryATSI::_qryType("IAPQueryATSI");

// -------------------------------------------------------
// -- IAPQueryAC interface methods
// -------------------------------------------------------
//Starts query execution, in case of success switches FSM to qryStarted state,
//otherwise to qryDone state.
IAPQueryAC::ProcResult_e  IAPQueryATSI::start(void) /*throw()*/
{
  //MAP_ATSI serves only ISDN International numbers
  if (!_abId.interISDN() || !_cfg.hasCSI()) {
    _qStatus = IAPQStatus::iqBadArg;
    setStageNotify(qryDone);
    return IAPQueryAC::procNeedReport;
  }

  try {
    _mapDlg.init().init(*_cfg._mapSess, *this, _logger);
    _mapDlg->subsciptionInterrogation(_cfg, _abId, _cfg._tmoSecs);  //throws
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
IAPQueryAC::ProcResult_e IAPQueryATSI::finalize(void) /*throw()*/
{
  while (_mapDlg.get() && !_mapDlg->unbindUser())
    wait();  //sleep while MAPDlg refers this query

  if (_mapDlg.get())
    _mapDlg->endDialog(); //end dialog if it's still active, release TC Dialog

  _mapDlg.clear();
  return IAPQueryAC::procOk;
}

// -------------------------------------------------------
// -- ATSIhandlerITF interface methods
// -------------------------------------------------------
//FSM switching:: qryStarted -> qryResulted
void IAPQueryATSI::onATSIResult(ATSIRes & res)
{
  MutexGuard  grd(*this);
  if (getStage() > qryStarted) {
    smsc_log_warn(_logger, "%s(%s): onMapResult() at stage %u", taskName(),
                  _abId.getSignals(), (unsigned)getStage());
    return;
  }

  if (res.isCSIpresent(_cfg.getCSI())) {
    _abInfo.abType = AbonentContractInfo::abtPrepaid;
    CSIRecord & csiRec = _abInfo.csiSCF[_cfg.getCSI()];
    csiRec.iapId = IAPProperty::iapATSI;
    csiRec.csiId = _cfg.getCSI();
    csiRec.scfInfo = *res.getSCFinfo(_cfg.getCSI());
  } else {
    _abInfo.abType = AbonentContractInfo::abtPostpaid;
  }
  if (res.getODB())
    _abInfo.odbGD = *res.getODB();
  smsc_log_debug(_logger, "%s(%s): %s", taskName(), _abId.getSignals(),
                 _abInfo.toString().c_str());
  setStage(qryResulted);
}

//FSM switching: [qryStarted | qryResulted] -> qryReporting
//               qryStopping -> qryStopping
ObjAllcStatus_e
  IAPQueryATSI::onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash err_code/* = 0*/)
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
  
    if (!err_code) {
      smsc_log_debug(_logger, "%s(%s): query succeeded",
                      taskName(), _abId.getSignals());
    } else if (err_code == _RCS_MAPOpErrors->mkhash(MAPOpErrorId::informationNotAvailable)) {
      //non-barred non-prepaid abonent
      _abInfo.abType = AbonentContractInfo::abtPostpaid;
      smsc_log_debug(_logger, "%s(%s): query failed: code 0x%x, %s",
                      taskName(), _abId.getSignals(), err_code, URCRegistry::explainHash(err_code).c_str());
    } else {
      _qStatus = IAPQStatus::iqError;
      _qError = err_code;
      _exc = URCRegistry::explainHash(err_code);
      smsc_log_error(_logger, "%s(%s): query failed: code 0x%x, %s",
                      taskName(), _abId.getSignals(), err_code, _exc.c_str());
    }
    setStageNotify(qryReporting);
  }
  reportThis();
  return ObjFinalizerIface::objActive;
}

/* ************************************************************************** *
 * class IAPQueriesPoolATSI implementation:
 * ************************************************************************** */
// ------------------------------------------
// -- IAPQueriesPoolIface interface methods
// ------------------------------------------
IAPQueryRef IAPQueriesPoolATSI::allcQuery(void)
{
  IAPQueryRef pQry = mObjPool.allcObj();
  if (pQry.empty()) {
    smsc_log_error(mLogger, "%s: pool is exhausted: %u of %u", IAPQueryATSI::_qryType.c_str(),
                   (unsigned)mObjPool.usage(), (unsigned)mObjPool.capacity());
  } else {
    (static_cast <IAPQueryATSI*>(pQry.get()))->configure(mCfg, mLogger);
  }
  return pQry;
}

} //atih
} //iaprvd
} //inman
} //smsc

