#ifndef __SCAG_TRANSPORT_MMS_SCAG_TASK_H__
#define __SCAG_TRANSPORT_MMS_SCAG_TASK_H__

#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "MmsProcessor.h"

namespace scag { namespace transport { namespace mms {

using smsc::core::threads::ThreadedTask;
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
