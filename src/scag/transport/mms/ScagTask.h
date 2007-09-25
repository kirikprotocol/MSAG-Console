#ifndef SCAG_TRANSPORT_MMS_SCAG_TASK
#define SCAG_TRANSPORT_MMS_SCAG_TASK

#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"
#include "MmsCommand.h"
#include "MmsContext.h"
#include "MmsProcessor.h"
#include "TaskList.h"

namespace scag { namespace transport { namespace mms {

using smsc::core::synchronization::EventMonitor;
using smsc::logger::Logger;

class MmsManagerImpl;

class ScagTask : public ThreadedTask {
public:
  ScagTask(MmsManagerImpl &m, MmsProcessor &p);
  virtual int Execute();
  virtual void stop();
  virtual const char * taskName();

protected:
  MmsManagerImpl &manager;
  MmsProcessor &processor;
  Logger *logger;
};

}//mms
}//transport
}//scag 

#endif
