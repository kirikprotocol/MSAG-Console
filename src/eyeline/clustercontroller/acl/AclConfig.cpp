/*
 * AclConfig.cpp
 *
 *  Created on: Jun 7, 2010
 *      Author: skv
 */

#include "eyeline/clustercontroller/acl/AclConfig.hpp"

namespace eyeline{
namespace clustercontroller{
namespace acl{


void AclConfig::Init(const char* storePath)
{
  smsc::acl::AclStore::Init();
  smsc::acl::AclStore::getInstance()->enableControllerMode();
  smsc::acl::AclStore::getInstance()->Load(storePath);
}


}
}
}
