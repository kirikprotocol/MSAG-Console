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
 * class IAPQuerySRIFactory implementation:
 * ************************************************************************** */
IAPQuerySRIFactory::IAPQuerySRIFactory(const IAPQuerySRI_CFG & in_cfg,
                            unsigned timeout_secs, Logger * use_log/* = NULL*/)
  : _cfg(in_cfg)
{
  logger = use_log ? use_log : Logger::getInstance("smsc.inman.iaprvd.sri");
}

IAPQueryAC * IAPQuerySRIFactory::newQuery(unsigned q_id, IAPQueryManagerITF * owner,
                                         Logger * use_log)
{
  return new IAPQuerySRI(q_id, owner, use_log, _cfg);
}

/* ************************************************************************** *
 * class IAPQuerySRI implementation:
 * ************************************************************************** */
const IAPQuerySRI::TypeString_t IAPQuerySRI::_qryType("IAPQuerySRI");

IAPQuerySRI::IAPQuerySRI(unsigned q_id, IAPQueryManagerITF * owner, 
                        Logger * use_log, const IAPQuerySRI_CFG & use_cfg)
  : IAPQueryAC(q_id, owner, use_cfg.mapTimeout, use_log), _cfg(use_cfg)
{
  logger = use_log ? use_log : Logger::getInstance("smsc.inman.iaprvd.sri");
  mkTaskName();
}

int IAPQuerySRI::Execute(void)
{ 
  {
    MutexGuard tmp(_mutex);
    if (isStopping || !_owner->hasListeners(abonent)) {
      //query was cancelled by either QueryManager or ThreadPool
      return (_qStatus = IAPQStatus::iqCancelled);
    }
    //MAP_SRI serves only ISDN International numbers
    if (!abonent.interISDN())
      return (_qStatus = IAPQStatus::iqBadArg);

    try {
      _mapDlg.init().init(*_cfg.mapSess, *this, logger);
      _mapDlg->reqRoutingInfo(abonent, _cfg.mapTimeout); //throws

      if (_mutex.wait(_cfg.mapTimeout*1000 + 100) != 0) //Unlocks, waits, locks
          _qStatus = IAPQStatus::iqTimeout;
    } catch (const std::exception & exc) {
      smsc_log_error(logger, "%s(%s): %s", taskName(), abonent.getSignals(),
                     exc.what());
      _qStatus = IAPQStatus::iqError;
      _exc = exc.what();
    }
    while (_mapDlg.get() && !_mapDlg->unbindUser()) //MAPDlg refers this query
      _mutex.wait();
  }
  if (_mapDlg.get())
    _mapDlg->endDialog(); //end dialog if it's still active, release TC Dialog
  _mapDlg.clear();
  return _qStatus;
}

void IAPQuerySRI::stop(void)
{
  MutexGuard  grd(_mutex);
  isStopping = true;
  _qStatus = IAPQStatus::iqCancelled;
  _mutex.notify();
}
// ****************************************
// -- CHSRIhandlerITF implementation:
// ****************************************
void IAPQuerySRI::onMapResult(CHSendRoutingInfoRes & res)
{
  MutexGuard  grd(_mutex);
  if (!res.getIMSI(abInfo.abImsi)) //abonent is unknown
    smsc_log_warn(logger, "%s(%s): IMSI not determined.", taskName(), abonent.getSignals());
  else {
    //NOTE: CH-SRI returns only O-Bcsm tDP serviceKeys
    if (!res.hasOCSI()) {
      abInfo.abType = AbonentContractInfo::abtPostpaid;
      smsc_log_debug(logger, "%s(%s): %s", taskName(), abonent.getSignals(),
                     abInfo.toString().c_str());
    } else {
      abInfo.abType = AbonentContractInfo::abtPrepaid;
      CSIRecord & csiRec = abInfo.csiSCF[UnifiedCSI::csi_O_BC];
      csiRec.iapId = IAPProperty::iapCHSRI;
      csiRec.csiId = UnifiedCSI::csi_O_BC;
      res.getSCFinfo(&csiRec.scfInfo);
      res.getVLRN(abInfo.vlrNum);
      smsc_log_debug(logger, "%s(%s): %s", taskName(), abonent.getSignals(),
                     abInfo.toString().c_str());
    }
  }
}

ObjAllcStatus_e
  IAPQuerySRI::onDialogEnd(ObjFinalizerIface & use_finalizer, RCHash ercode/* = 0*/)
{
  MutexGuard  grd(_mutex);
  if (ercode) {
    _qStatus = IAPQStatus::iqError;
    _qError = ercode;
    _exc = URCRegistry::explainHash(ercode);
    smsc_log_error(logger, "%s(%s): query failed: code 0x%x, %s",
                    taskName(), abonent.getSignals(), ercode, _exc.c_str());
  } else
    smsc_log_debug(logger, "%s(%s): query succeeded",
                    taskName(), abonent.getSignals());

  return ObjFinalizerIface::objActive;
}

} //sri
} //iaprvd
} //inman
} //smsc

