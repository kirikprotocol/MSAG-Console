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
IAPQStatus::Code
  IAPQuerySRI::start(Logger * use_log/* = NULL*/) /*throw()*/
{
  if (use_log)
    _logger = use_log;

  //MAP_SRI serves only ISDN International numbers
  if (!_abId.interISDN()) {
    setStageNotify(qryDone);
    return (_qStatus = IAPQStatus::iqBadArg);
  }

  try {
    _mapDlg.init().init(*_cfg._mapSess, *this, _logger);
    _mapDlg->reqRoutingInfo(_abId, _cfg._tmoSecs); //throws
    /* - */
  } catch (const std::exception & exc) {
    smsc_log_error(_logger, "%s(%s): %s", taskName(), _abId.getSignals(), exc.what());
    _exc = exc.what();
    setStage(qryDone);
    if (_mapDlg.get())
      _mapDlg->unbindUser();
    return (_qStatus = IAPQStatus::iqError);
  }
  setStage(qryStarted);
  return (_qStatus = IAPQStatus::iqOk);
}

//Cancels query execution, switches FSM to qryDone state.
//Note: Listeners aren't notified.
//Returns false if listener is already targeted and query
//waits for its mutex.
bool IAPQuerySRI::cancel(void) /*throw()*/
{
  if (hasListenerAimed())
    return false;

  if (getStage() < qryStopping)
    setStage(qryStopping);
  
  if (getStage() < qryDone) {
    while (_mapDlg.get() && !_mapDlg->unbindUser())
      wait();  //sleep while MAPDlg refers this query
  
    if (_mapDlg.get())
      _mapDlg->endDialog(); //end dialog if it's still active, release TC Dialog
  
    _lsrList.clear();
    _qStatus = IAPQStatus::iqCancelled;
    setStageNotify(qryDone);
  }
  return true;
}

//Returns true if query object may be released.
//Note: should be at least equal to isCompleted()
bool IAPQuerySRI::isToRelease(void) /*throw()*/
{
  return isCompleted() && (!_mapDlg.get() || _mapDlg->unbindUser());
}

//Releases all used resources. May be called only at qryDone state.
//Switches FSM to qryIdle state.
void IAPQuerySRI::cleanup(void) /*throw()*/
{
  _mapDlg.clear();
  _stages.clear();
}

// -------------------------------------------------------
// -- CHSRIhandlerITF interface methods
// -------------------------------------------------------
//FSM switching:: qryStarted -> qryResulted
void IAPQuerySRI::onMapResult(CHSendRoutingInfoRes & res)
{
  MutexGuard  grd(*this);
  if (getStage() > qryStarted) {
    smsc_log_warn(_logger, "%s(%s): onMapResult() at stage %u", taskName(),
                  _abId.getSignals(), (unsigned)getStage());
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
  
    setStage(qryReporting);
    _owner->onQueryEvent(_abId);
  }
  return ObjFinalizerIface::objActive;
}

/* ************************************************************************** *
 * class IAPQueriesPoolSRI implementation:
 * ************************************************************************** */
void IAPQueriesPoolSRI::init(const IAPQuerySRI_CFG & use_cfg)
{
  MutexGuard grd(_sync);
  _cfg = use_cfg;
}

// ------------------------------------------
// -- IAPQueriesPoolIface interface methods
// ------------------------------------------
void IAPQueriesPoolSRI::reserveObj(IAPQueryId num_obj) /*throw()*/
{
  MutexGuard  grd(_sync);
  return _objPool.reserve(num_obj);
}

IAPQueryAC * IAPQueriesPoolSRI::allcQuery(void)
{
  MutexGuard  grd(_sync);
  IAPQuerySRI * pQry = _objPool.allcObj();
  if (pQry)
    pQry->configure(_cfg);
  return pQry;
}

void IAPQueriesPoolSRI::rlseQuery(IAPQueryAC & use_qry)
{
  MutexGuard  grd(_sync);
  _objPool.rlseObj(static_cast<QueriesPool::PooledObj&>(use_qry));
}

} //sri
} //iaprvd
} //inman
} //smsc

