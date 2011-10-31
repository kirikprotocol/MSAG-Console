#ifndef _SCAG_PVSS_SERVER_WRITERTASKMANAGER_H_
#define _SCAG_PVSS_SERVER_WRITERTASKMANAGER_H_

#include "scag/util/RelockMutexGuard.h"
#include "IOTaskManager.h"
#include "IOTask.h"

namespace scag2 {
namespace pvss  {

using scag::util::RelockMutexGuard;

class WriterTaskManager : public IOTaskManager {
public:
  WriterTaskManager(const SyncConfig& cfg) : IOTaskManager(cfg, "writerman") {} 

protected:
    virtual void postRegister( ConnectionContext* ) {}

  IOTask* newTask() {
      return new MTPersWriter(connectionTimeout_, ioTimeout_);
  }
};

}//pvss
}//scag2

#endif
  
 
