/*
 * ProfilerConfig.hpp
 *
 *  Created on: Mar 18, 2010
 *      Author: skv
 */

#ifndef __EYELINE_CLUSTERCONTROLLER_PROFILER_PROFILERCONFIG_HPP__
#define __EYELINE_CLUSTERCONTROLLER_PROFILER_PROFILERCONFIG_HPP__

namespace smsc{
namespace profiler{
class Profiler;
}
}

namespace eyeline{
namespace clustercontroller{
namespace profiler{

class ProfilerConfig
{
protected:
  static smsc::profiler::Profiler* profiler;
public:
  static void Init(const char* path);
  static smsc::profiler::Profiler* getProfiler()
  {
    return profiler;
  }
};

}
}
}

#endif /* PROFILERCONFIG_HPP_ */
