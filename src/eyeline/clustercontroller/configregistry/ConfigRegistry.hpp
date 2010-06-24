/*
 * ConfigRegistry.hpp
 *
 *  Created on: Jun 22, 2010
 *      Author: skv
 */

#ifndef __EYELINE_CONFIGREGISTRY_CONFIGREGISTRY_HPP__
#define __EYELINE_CONFIGREGISTRY_CONFIGREGISTRY_HPP__

#include "eyeline/clustercontroller/ConfigLocks.hpp"
#include <sys/types.h>
#include "core/buffers/File.hpp"
#include "core/synchronization/Mutex.hpp"
#include <vector>

namespace eyeline {
namespace clustercontroller {
namespace configregistry {

class ConfigRegistry
{
protected:
  time_t configUpdates[ctConfigsCount];
  smsc::core::buffers::File store;
  smsc::core::synchronization::Mutex mtx;

  static ConfigRegistry* instance;
public:
  ConfigRegistry(const char* storeFileName);
  static void Init(const char* storeFileName);
  static void Shutdown();
  static ConfigRegistry* getInstance()
  {
    return instance;
  }
  void update(ConfigType ct);
  void get(std::vector<time_t>& value);
};

}
}
}

#endif /* __EYELINE_CONFIGREGISTRY_CONFIGREGISTRY_HPP__ */
