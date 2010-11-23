/*
 * AliasConfig.cpp
 *
 *  Created on: May 27, 2010
 *      Author: skv
 */

#include <eyeline/clustercontroller/alias/AliasConfig.hpp>

namespace eyeline{
namespace clustercontroller{
namespace alias{

smsc::alias::AliasManager* AliasConfig::instance=0;


void AliasConfig::Init(const char* path)
{
  instance=new smsc::alias::AliasManImpl(path);
  instance->enableControllerMode();
  instance->Load();
}

}
}
}
