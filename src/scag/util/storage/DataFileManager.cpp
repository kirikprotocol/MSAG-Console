#include "DataFileManager.h"
#include "DataFileCreator.h"

namespace scag    {
namespace util    {
namespace storage {

int DataFileTask::Execute() {
  creator_.createDataFile();
  return 0;
}

const char * DataFileTask::taskName() {
  return creator_.getFileName();
}

void DataFileManager::createDataFile(DataFileCreator& creator) {
  DataFileTask* task = new DataFileTask(creator);
  pool_.startTask(task);
}

}
}
}

