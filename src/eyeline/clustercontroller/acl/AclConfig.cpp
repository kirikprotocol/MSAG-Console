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

smsc::acls::AclAbstractMgr* AclConfig::instance;

void AclConfig::Init(const char* storePath,int preCreate)
{
  instance=smsc::acls::AclAbstractMgr::Create2();
  instance->LoadUp(storePath,preCreate);
  instance->enableControllerMode();
}


}
}
}
