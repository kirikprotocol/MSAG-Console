#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/facility2/inc/IAPQuery.hpp"

using smsc::util::URCRegistry;
using smsc::core::synchronization::ReverseMutexGuard;

namespace smsc {
namespace inman {
namespace iaprvd {
/* ************************************************************************** *
 * class IAPQueryAC implementation:
 * ************************************************************************** */
void IAPQueryAC::init(IAPQueryRefereeIface & use_owner, const AbonentId & ab_id)
{
  _owner = &use_owner;
  _abId = ab_id;
  _stages.clear();
  _lsrList.clear();
  _qStatus = IAPQStatus::iqOk;
  _qError = 0;
  _abInfo.clear();
  _exc.clear();
  ++_usage;
  mkTaskName();
  _logger = Logger::getInstance(IAPROVIDER_DFLT_LOGGER);
}

void IAPQueryAC::mkTaskName(void)
{
  char buf[_idTaskSZ + 1];
  snprintf(buf, sizeof(buf)-1, "[%u:%lu]", this->getUIdx(), _usage);
  _tName = taskType().c_str();
  _tName += buf;
}

bool IAPQueryAC::addListener(IAPQueryListenerITF & pf_cb)
{
  _lsrList.push_back(ListenerInfo(&pf_cb));
  if (_stages._current <= qryReporting)
    return true;
  _stages.rollback(qryReporting, qryDone);
  this->notify();
  return false;
}

//returns false if listener is already targeted and query waits for it mutex.
bool IAPQueryAC::removeListener(IAPQueryListenerITF & pf_cb)
{
  QueryListeners::iterator it = _lsrList.begin();
  for (; it != _lsrList.end(); ++it) {
    if (it->_ptr == &pf_cb) {
      if (it->_isAimed)
        return false;
      _lsrList.erase(it);
      return true;
    }
  }
  return true;
}

void IAPQueryAC::notifyListeners(void) /*throw()*/
{
  if (getStage() == qryDone)
    return;

  while (!_lsrList.empty() && (getStage() == qryReporting)) {
    QueryListeners::iterator it = _lsrList.begin();
    bool res = it->_isAimed = true;
    {
      ReverseMutexGuard rGrd(*this);
      try { res = it->_ptr->onIAPQueried(_abId, _abInfo, _qError);
      } catch (const std::exception & exc) {
        smsc_log_error(_logger, "%s(%s): listener exception: %s", 
                       taskName(), _abId.getSignals(), exc.what());
      } catch (...) {
        smsc_log_error(_logger, "%s(%s): listener exception: 'unknown'", 
                       taskName(), _abId.getSignals());
      }
    }
    if (res)
      _lsrList.pop_front();
    else { //rereport is requested, move listener to end of queue
      it->_isAimed = false;
      if (++it != _lsrList.end())
        _lsrList.splice(_lsrList.end(), _lsrList, _lsrList.begin());
    }
  }
  setStage(qryDone);
  _owner->onQueryEvent(_abId);
}

std::string IAPQueryAC::status2Str(void) const
{
  std::string st;

  switch (_qStatus) {
  case IAPQStatus::iqOk:  st += "finished";
    break;
  case IAPQStatus::iqCancelled: st += "cancelled";
    break;
  case IAPQStatus::iqTimeout: {
    st += "timed out";
    if (!_exc.empty()) {
      st += ", "; st += _exc;
    }
  } break;
  default: {
    st += "failed";
    if (!_exc.empty()) {
      st += ", "; st += _exc;
    } else if (_qError) {
      st += ", "; st += URCRegistry::explainHash(_qError);
    }
  }} /* eosw */
  return st;
}

} //iaprvd
} //inman
} //smsc


