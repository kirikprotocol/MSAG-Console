#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPROCESSORMGRREGISTRY_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPROCESSORMGRREGISTRY_HPP__

# include <map>
# include "core/synchronization/RWLock.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessorMgr.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessorMgrRegistry : public utilx::Singleton<IOProcessorMgrRegistry> {
public:
  void registerIOProcMgr(IOProcessorMgr* io_proc_mgr);
  IOProcessorMgr* unregisterNextIOProcMgr();

  class Iterator;
  Iterator getIterator();

private:
  IOProcessorMgrRegistry()
  : _logger(smsc::logger::Logger::getInstance("io_subsystem"))
  {}
  friend class utilx::Singleton<IOProcessorMgrRegistry>;

  smsc::logger::Logger* _logger;
  typedef std::map<unsigned, IOProcessorMgr*> registered_ioprocmgrs_t;
  registered_ioprocmgrs_t _registeredIoProcMgrs;
  smsc::core::synchronization::RWLock _lock;
public:
  class Iterator {
  public:
    ~Iterator();
    virtual bool hasElement() const;
    virtual void next();

    virtual const IOProcessorMgr* getCurrentElement() const;

    virtual IOProcessorMgr* getCurrentElement();
  private:
    Iterator(registered_ioprocmgrs_t& io_procmgrs,
             smsc::core::synchronization::RWLock& lock);
    registered_ioprocmgrs_t::iterator _iter, _endIter;
    smsc::core::synchronization::RWLock& _lock;
    friend class IOProcessorMgrRegistry;
  };

};

}}}

#endif
