/*
 * ConfigRegistry.cpp
 *
 *  Created on: Jun 17, 2010
 *      Author: skv
 */

#include "smsc/configregistry/ConfigRegistry.hpp"
#include "util/TimeSource.h"

namespace smsc{
namespace configregistry{

ConfigRegistry* ConfigRegistry::instance=0;

ConfigRegistry::ConfigRegistry()
{
  for(size_t i=0;i<eyeline::clustercontroller::ctConfigsCount;i++)
  {
    lastUpdate[i]=0;
  }
}

void ConfigRegistry::Init()
{
  instance=new ConfigRegistry;
}

void ConfigRegistry::update(eyeline::clustercontroller::ConfigType ct)
{
  lastUpdate[ct]=smsc::util::TimeSourceSetup::AbsSec::getSeconds();
}

void ConfigRegistry::get(std::vector<int64_t>& times)
{
  times.insert(times.begin(),lastUpdate,lastUpdate+eyeline::clustercontroller::ctConfigsCount);
}

}
}
