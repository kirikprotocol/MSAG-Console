/*
 * AclConfig.hpp
 *
 *  Created on: Jun 7, 2010
 *      Author: skv
 */

#ifndef __EYELINE_CLUSTERCONTROLLER_ACL_ACLCONFIG_HPP__
#define __EYELINE_CLUSTERCONTROLLER_ACL_ACLCONFIG_HPP__

#include "smsc/acls/AclManager.hpp"

namespace eyeline{
namespace clustercontroller{
namespace acl{

class AclConfig
{
public:

  static void Init(const char* storePath);

  static smsc::acl::AclStore* getInstance()
  {
    return smsc::acl::AclStore::getInstance();
  }
};

}
}
}

#endif /* ACLCONFIG_HPP_ */
