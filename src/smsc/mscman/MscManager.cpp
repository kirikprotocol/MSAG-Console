#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/Array.hpp"
#include "core/threads/Thread.hpp"
#include "core/buffers/FixedRecordFile.hpp"
#include "util/debug.h"
#include "util/config/Manager.h"

#include "MscManager.h"

namespace smsc {
namespace mscman {

MscManager* MscManager::instance = 0;

MscManager::MscManager()
{
  log = Logger::getInstance("smsc.msc");
}

void MscManager::Init()
{
  buf::File f;
  const char* mscStoreFileName=smsc::util::config::Manager::getInstance().getString("MscManager.storeFile");
  f.ROpen(mscStoreFileName);
  std::string line;
  while(f.ReadLine(line))
  {
    mscs.Insert(line.c_str(),true);
  }
}

bool MscManager::check(const std::string& msc)
{
  sync::MutexGuard mg(mtx);
  return mscs.Exists(msc.c_str());
}

void MscManager::add(const std::string& msc)
{
  sync::MutexGuard mg(mtx);
  mscs.Insert(msc.c_str(),true);
}
void MscManager::remove(const std::string& msc)
{
  sync::MutexGuard mg(mtx);
  mscs.Delete(msc.c_str());
}

void MscManager::startup()
{
  if (instance)
  {
    throw smsc::util::Exception("MscManager instance already initialized");
  }
  instance = new MscManager();
  instance->Init();
}
void MscManager::shutdown()
{
  delete instance;
  instance = 0;
}
MscManager& MscManager::getInstance()
{
  return (*instance);
}



}
}
