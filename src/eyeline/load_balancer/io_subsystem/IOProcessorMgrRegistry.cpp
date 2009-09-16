#include <utility>

#include "IOProcessorMgrRegistry.hpp"
#include "core/synchronization/RWLockGuard.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

void
IOProcessorMgrRegistry::registerIOProcMgr(IOProcessorMgr* io_proc_mgr)
{
  smsc::core::synchronization::WriteLockGuard synchronize(_lock);
  std::pair<registered_ioprocmgrs_t::iterator, bool> ins_res =
    _registeredIoProcMgrs.insert(std::make_pair(io_proc_mgr->getId(), io_proc_mgr));
  if ( !ins_res.second )
    throw smsc::util::Exception("IOProcessorMgrRegistry::registerIOProcMgr::: try register IOProcessorMgr's object with the same id value=[%d]", io_proc_mgr->getId());
  smsc_log_debug(_logger, "IOProcessorMgr object with id=%d was registered",
                 io_proc_mgr->getId());
}

IOProcessorMgr*
IOProcessorMgrRegistry::unregisterNextIOProcMgr()
{
  smsc::core::synchronization::WriteLockGuard synchronize(_lock);
  if ( _registeredIoProcMgrs.empty() ) {
    smsc_log_debug(_logger, "IOProcessorMgrRegistry::unregisterNextIOProcMgr::: there isn't active IOProcessorMg objects");
    return NULL;
  } else {
    registered_ioprocmgrs_t::iterator iter = _registeredIoProcMgrs.begin();
    IOProcessorMgr* ioProcMgr = iter->second;

    smsc_log_debug(_logger, "IOProcessorMgrRegistry::unregisterNextIOProcMgr::: IOProcessorMgr object with id=%d was unregistered",
                     ioProcMgr->getId());
    _registeredIoProcMgrs.erase(iter);
    return ioProcMgr;
  }
}

IOProcessorMgr*
IOProcessorMgrRegistry::getIOProcessorMgr(unsigned io_proc_mgr_id)
{
  smsc::core::synchronization::WriteLockGuard synchronize(_lock);
  registered_ioprocmgrs_t::iterator iter = _registeredIoProcMgrs.find(io_proc_mgr_id);
  if ( iter == _registeredIoProcMgrs.end() ) {
    smsc_log_debug(_logger, "IOProcessorMgrRegistry::getIOProcessorMgr::: IOProcessorMgr with id=%d is absent",
                   io_proc_mgr_id);
    return NULL;
  }
  return iter->second;
}

IOProcessorMgrRegistry::Iterator
IOProcessorMgrRegistry::getIterator()
{
  return Iterator(_registeredIoProcMgrs, _lock);
}

bool
IOProcessorMgrRegistry::Iterator::hasElement() const
{
  return _iter != _endIter;
}

void
IOProcessorMgrRegistry::Iterator::next()
{
  ++_iter;
}

const IOProcessorMgr*
IOProcessorMgrRegistry::Iterator::getCurrentElement() const
{
  return _iter->second;
}

IOProcessorMgr*
IOProcessorMgrRegistry::Iterator::getCurrentElement()
{
  return _iter->second;
}

IOProcessorMgrRegistry::Iterator::Iterator(IOProcessorMgrRegistry::registered_ioprocmgrs_t& io_procmgrs,
                                           smsc::core::synchronization::RWLock& lock)
  : _iter(io_procmgrs.begin()), _endIter(io_procmgrs.end()),
    _lock(lock)
{
  _lock.rlock();
}

IOProcessorMgrRegistry::Iterator::~Iterator()
{
  _lock.unlock();
}

}}}
