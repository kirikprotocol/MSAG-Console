/*
 * AliasConfig.hpp
 *
 *  Created on: May 27, 2010
 *      Author: skv
 */

#ifndef __EYELINE_CLUSTERCONTROLLER_ALIAS_ALIASCONFIG_HPP__
#define __EYELINE_CLUSTERCONTROLLER_ALIAS_ALIASCONFIG_HPP__

#include "smsc/alias/AliasManImpl.hpp"

namespace eyeline{
namespace clustercontroller{
namespace alias{

class AliasConfig
{
protected:
  static smsc::alias::AliasManager* instance;
public:
  static void Init(const char* path);
  static smsc::alias::AliasManager* getInstance()
  {
    return instance;
  }
};

}
}
}

#endif /* ALIASCONFIG_HPP_ */
