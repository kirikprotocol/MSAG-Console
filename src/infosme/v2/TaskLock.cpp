#include "TaskLock.h"
#include "util/Exception.hpp"

namespace smsc{
namespace infosme2 {

TaskLock* TaskLock::instance=0;

void TaskLock::Init(const char* storeFile)
{
  if(instance)
  {
    throw smsc::util::Exception("Double Init of TaskLock!");
  }
  instance=new TaskLock;
  instance->log=smsc::logger::Logger::getInstance("tasklock");
  instance->fileName=storeFile;
  instance->Load();
}

void TaskLock::Shutdown()
{
  delete instance;
  instance=0;
}


}
}

