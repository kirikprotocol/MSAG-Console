#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapATSI/IAPQueryATSI.hpp"

#include "inman/inap/HDSSnSession.hpp"
using smsc::inman::inap::SSNSession;

#include "inman/comp/map_atih/MapATIHFactory.hpp"
using smsc::inman::comp::_ac_map_anyTimeInfoHandling_v3;
using smsc::inman::comp::atih::initMAPATIH3Components;

using smsc::util::URCRegistry;

namespace smsc {
namespace inman {
namespace iaprvd {
namespace atih {

/* ************************************************************************** *
 * class IAPQueryATSIFactory implementation:
 * ************************************************************************** */
IAPQueryATSIFactory::IAPQueryATSIFactory(const IAPQueryATSI_CFG & in_cfg,
                            unsigned timeout_secs, Logger * use_log/* = NULL*/)
  : _cfg(in_cfg)
{
  logger = use_log ? use_log : Logger::getInstance("smsc.inman.iaprvd.atsi");
}

IAPQueryAC * IAPQueryATSIFactory::newQuery(unsigned q_id, IAPQueryManagerITF * owner,
                                         Logger * use_log)
{
  return new IAPQueryATSI(q_id, owner, use_log, _cfg);
}

/* ************************************************************************** *
 * class IAPQueryATSI implementation:
 * ************************************************************************** */
const IAPQueryATSI::TypeString_t IAPQueryATSI::_qryType("IAPQueryATSI");

IAPQueryATSI::IAPQueryATSI(unsigned q_id, IAPQueryManagerITF * owner, 
                        Logger * use_log, const IAPQueryATSI_CFG & use_cfg)
  : IAPQueryAC(q_id, owner, use_cfg.mapTimeout, use_log), _cfg(use_cfg)
{
  logger = use_log ? use_log : Logger::getInstance("smsc.inman.iaprvd.atsi");
  mkTaskName();
}

int IAPQueryATSI::Execute(void)
{ 
  {
    MutexGuard tmp(_mutex);
    if (isStopping || !_owner->hasListeners(abonent)) {
      //query was cancelled by either QueryManager or ThreadPool
      return (_qStatus = IAPQStatus::iqCancelled);
    }
    //MAP_ATSI serves only ISDN International numbers
    if (!abonent.interISDN() || !_cfg.hasCSI())
      return (_qStatus = IAPQStatus::iqBadArg);

    try {
      _mapDlg = new MapATSIDlg(_cfg.mapSess, this); //binds this as user
      _mapDlg->subsciptionInterrogation(_cfg, abonent, _cfg.mapTimeout);  //throws

      if (_mutex.wait(_cfg.mapTimeout*1000 + 100) != 0) //Unlocks, waits, locks
        _qStatus = IAPQStatus::iqTimeout;
    } catch (const std::exception & exc) {
      smsc_log_error(logger, "%s(%s): %s", taskName(), abonent.getSignals(),
                     exc.what());
      _qStatus = IAPQStatus::iqError;
      _exc = exc.what();
    }
    if (_mapDlg) {
      while (!_mapDlg->Unbind()) //MAPDlg refers this query
        _mutex.wait();
    }
  }
  if (_mapDlg) {
    _mapDlg->destroy();  //synchronization point, waits for _mapDlg mutex
    _mapDlg = NULL;
  }
  return _qStatus;
}

void IAPQueryATSI::stop(void)
{
  MutexGuard  grd(_mutex);
  isStopping = true;
  _qStatus = IAPQStatus::iqCancelled;
  _mutex.notify();
}
// ****************************************
// -- ATSIhandlerITF implementation:
// ****************************************
void IAPQueryATSI::onATSIResult(ATSIRes & res)
{
  MutexGuard  grd(_mutex);
  if (res.isCSIpresent(_cfg.getCSI())) {
    abInfo.abType = AbonentContractInfo::abtPrepaid;
    CSIRecord & csiRec = abInfo.csiSCF[_cfg.getCSI()];
    csiRec.iapId = IAPProperty::iapATSI;
    csiRec.csiId = _cfg.getCSI();
    csiRec.scfInfo = *res.getSCFinfo(_cfg.getCSI());
  } else {
    abInfo.abType = AbonentContractInfo::abtPostpaid;
  }
  smsc_log_debug(logger, "%s(%s): %s", taskName(), abonent.getSignals(),
                 abInfo.toString().c_str());
}

void IAPQueryATSI::onEndATSI(RCHash ercode/* = 0*/)
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
}

} //atih
} //iaprvd
} //inman
} //smsc

