/*
 * ProfilerConfig.cpp
 *
 *  Created on: Mar 18, 2010
 *      Author: skv
 */

#include "eyeline/clustercontroller/profiler/ProfilerConfig.hpp"
#define CCMODE
#include "smsc/profiler/profiler.cpp"

namespace eyeline{
namespace clustercontroller{
namespace profiler{

smsc::profiler::Profiler* ProfilerConfig::profiler=0;

void ProfilerConfig::Init(const char* path)
{
  profiler=new smsc::profiler::Profiler(0,"");
  profiler->load(path);
  profiler->setControllerMode();
}

}
}
}
