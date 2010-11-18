/* ************************************************************************** *
 * Signalling Task Sheduler service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_SCHEDULER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_SCHEDULER_HPP

#include "inman/common/ObjRegistryT.hpp"

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/scheduler/TaskScheduler.hpp"

namespace smsc {
namespace inman {

using smsc::util::POBJRegistry_T;

//Signalling Task Sheduler service.
class ICSTaskScheduler : public ICServiceAC, public TaskSchedulerFactoryITF {
protected:
  typedef POBJRegistry_T<TaskSchedulerITF::SchedulerType,
                                          TaskSchedulerAC> TSchedRegistry;
  mutable Mutex   _sync;
  TSchedRegistry  tschedReg;

  // ---------------------------------
  // -- ICServiceAC interface methods
  // --------------------------------- 
  Mutex & _icsSync(void) const { return _sync; }
  //Initializes service verifying that all dependent services are inited
  RCode _icsInit(void) { return ICServiceAC::icsRcOk; }
  //Starts service verifying that all dependent services are started
  RCode _icsStart(void)  { return ICServiceAC::icsRcOk; }
  //Stops service
  void  _icsStop(bool do_wait = false);

public:
  ICSTaskScheduler(const ICServicesHostITF * svc_host, Logger * use_log)
    : ICServiceAC(ICSIdent::icsIdScheduler, svc_host, use_log)
  {
    _icsState = ICServiceAC::icsStConfig;
  }
  ~ICSTaskScheduler()
  {
    ICSStop(true);
  }

  //Returns TaskSchedulerFactoryITF
  virtual void * Interface(void) const
  {
    return (TaskSchedulerFactoryITF*)this;
  }

  // ------------------------------------------
  // TaskSchedulerFactoryITF interface methods:
  // ------------------------------------------
  //Returns started scheduler
  virtual TaskSchedulerITF *
    getScheduler(TaskSchedulerITF::SchedulerType sched_type);
};

} //inman
} //smsc
#endif /* __INMAN_ICS_SCHEDULER_HPP */

