#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/facility2/IAPQueryFacility.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {

/* ************************************************************************** *
 * class IAPQueryFacility implementation:
 * ************************************************************************** */
const TimeSlice  IAPQueryFacility::_dflt_ShutdownTmo(300, TimeSlice::tuMSecs);

//Finally stops facility. Cancels all existing queries.
//If shutdown timeout isn't set the default one is used.
void IAPQueryFacility::stop(const TimeSlice * use_tmo/* = NULL*/)
{
  cancelAllQueries();
  _iapNtfr.stop(use_tmo ? use_tmo : &_dflt_ShutdownTmo);
}

// ----------------------------------------------
// IAPQueryProcessorITF interface implementation:
// ----------------------------------------------
//Starts query and binds listener to it.
//Returns true if query succesfully started, false otherwise
bool IAPQueryFacility::startQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb)
{
  IAPQueryGuard pQry = _qrsStore.getQuery(ab_number, true);
  if (!pQry.get()) {
    smsc_log_error(_logger, "%s: queries pool exhausted", _logId);
    return false; //no more queries available
  }

  bool  rval = true;
  {
    MutexGuard grd(*pQry);
    if (!pQry->addListener(pf_cb)) { //query already done, just rereport it
      if (!_iapNtfr.onQueryEvent(ab_number)) {
        smsc_log_error(_logger, "%s: failed to report %s(%s)", _logId,
                       pQry->taskName(), ab_number.getSignals());
        pQry->removeListener(pf_cb);
        rval = false;
      } else {
        smsc_log_debug(_logger, "%s: rereporting %s(%s)", _logId,
                       pQry->taskName(), ab_number.getSignals());
      }
    } else {
      smsc_log_debug(_logger, "%s: listener is added to %s(%s)", _logId,
                     pQry->taskName(), ab_number.getSignals());
      if (pQry.isStatus(IAPQueriesStore::qryNew)) {
        if (!(rval = (pQry->start(_logger) == IAPQStatus::iqOk))) {
          smsc_log_error(_logger, "%s: failed to start %s(%s)", _logId,
                         pQry->taskName(), ab_number.getSignals());
          pQry->removeListener(pf_cb);
          pQry.rlseQuery();
        } else {
          smsc_log_debug(_logger, "%s: started %s(%s)", _logId,
                         pQry->taskName(), ab_number.getSignals());
        }
      }
    }
  }
  return rval;
}

//Unbinds query listener, cancels query if no listeners remain.
bool IAPQueryFacility::cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb)
{
  IAPQueryGuard   pQry = _qrsStore.getQuery(ab_number, false);

  if (pQry.get()) {
    MutexGuard grd(*pQry);
    if (!pQry->removeListener(pf_cb))
      return false;

    smsc_log_debug(_logger, "%s: listener is removed from %s(%s)", _logId,
                   pQry->taskName(), ab_number.getSignals());

    if (!pQry->hasListener()) {
      smsc_log_debug(_logger, "%s: cancelling %s(%s)", _logId,
                     pQry->taskName(), ab_number.getSignals());
      pQry->cancel(); //cann't fail here
      if (pQry->isToRelease()) { //release query immediately
        smsc_log_debug(_logger, "%s: releasing %s(%s)", _logId,
                       pQry->taskName(), ab_number.getSignals());
        pQry.rlseQuery();
      } else {
        _iapNtfr.onQueryEvent(ab_number);
      }
    }
  }
  return true;
}

//
bool IAPQueryFacility::cancelAllQueries(void)
{
  bool rval = true;
  for (IAPQueriesStore::iterator qIt = _qrsStore.begin(); qIt.get(); ++qIt) {
    MutexGuard grd(*qIt.get());
    smsc_log_debug(_logger, "%s: cancelling %s(%s)", _logId,
                   qIt->taskName(), qIt->getAbonentId().getSignals());
    rval &= qIt->cancel();
  }
  return rval;
}

} //iaprvd
} //inman
} //smsc


