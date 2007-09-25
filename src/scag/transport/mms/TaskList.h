#ifndef SCAG_TRANSPORT_MMS_TASK_LIST
#define SCAG_TRANSPORT_MMS_TASK_LIST

#include "core/threads/ThreadedTask.hpp"

namespace scag { namespace transport { namespace mms {

using smsc::core::threads::ThreadedTask;
class IOTaskManager;

class IOTaskParent : public ThreadedTask {
  friend class IOTaskManager;

public:
  IOTaskParent(unsigned int c = 0):socket_count(c), task_index(0) {} ;
  virtual int Execute();
  virtual const char* taskName();
  virtual ~IOTaskParent() {};

protected:
  unsigned int socket_count;

private:
  unsigned int task_index;
};

}//mms
}//transport
}//scag

#endif
