/*
 * ConfigRegistry.cpp
 *
 *  Created on: Jun 22, 2010
 *      Author: skv
 */

#include <eyeline/clustercontroller/configregistry/ConfigRegistry.hpp>
#include "util/TimeSource.h"

namespace eyeline {
namespace clustercontroller{
namespace configregistry {

ConfigRegistry* ConfigRegistry::instance=0;

void ConfigRegistry::Init(const char* storeFileName)
{
  instance=new ConfigRegistry(storeFileName);
}

void ConfigRegistry::Shutdown()
{
  delete instance;
}


ConfigRegistry::ConfigRegistry(const char* storeFileName)
{
  for(size_t i=0;i<ctConfigsCount;i++)
  {
    configUpdates[i]=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
  }
  using smsc::core::buffers::File;
  store.SetUnbuffered();
  if(File::Exists(storeFileName))
  {
    store.RWOpen(storeFileName);
    for(size_t i=0;i<ctConfigsCount;i++)
    {
      configUpdates[i]=store.ReadNetInt64();
    }
  }else
  {
    store.RWCreate(storeFileName);
    for(size_t i=0;i<ctConfigsCount;i++)
    {
      store.WriteNetInt64(configUpdates[i]);
    }
  }
}

void ConfigRegistry::update(ConfigType ct)
{
  smsc::core::synchronization::MutexGuard mg(mtx);
  configUpdates[ct]=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
  store.SeekCur(ct*8);
  store.WriteNetInt64(configUpdates[ct]);
}

void ConfigRegistry::get(std::vector<time_t>& value)
{
  value.insert(value.begin(),configUpdates,configUpdates+ctConfigsCount);
}

}
}
}
