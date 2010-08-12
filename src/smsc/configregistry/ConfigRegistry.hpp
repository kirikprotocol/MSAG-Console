/*
 * ConfigRegistry.hpp
 *
 *  Created on: Jun 17, 2010
 *      Author: skv
 */

#ifndef __SMSC_CONFIGREGISTRY_CONFIGREGISTRY_HPP__
#define __SMSC_CONFIGREGISTRY_CONFIGREGISTRY_HPP__

#include <time.h>
#include "eyeline/clustercontroller/ConfigLocks.hpp"
#include <vector>

namespace smsc{
namespace configregistry{

class ConfigRegistry
{
protected:
  time_t lastUpdate[eyeline::clustercontroller::ctConfigsCount];
  static ConfigRegistry* instance;
public:
  ConfigRegistry();
  static void Init();
  static ConfigRegistry* getInstance()
  {
    return instance;
  }
  void update(eyeline::clustercontroller::ConfigType ct);
  void get(std::vector<time_t>& times);
};

}
}

#endif /* __SMSC_CONFIGREGISTRY_CONFIGREGISTRY_HPP__ */
