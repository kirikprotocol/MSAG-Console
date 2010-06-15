/*
 * AclConfig.hpp
 *
 *  Created on: Jun 7, 2010
 *      Author: skv
 */

#ifndef __EYELINE_CLUSTERCONTROLLER_ACL_ACLCONFIG_HPP__
#define __EYELINE_CLUSTERCONTROLLER_ACL_ACLCONFIG_HPP__

#include "smsc/acls/interfaces.h"

namespace eyeline{
namespace clustercontroller{
namespace acl{

class AclConfig
{
protected:
  static smsc::acls::AclAbstractMgr* instance;
public:

  static void Init(const char* storePath,int preCreate);

  static smsc::acls::AclAbstractMgr* getInstance()
  {
    return instance;
  }
};

}
}
}

#endif /* ACLCONFIG_HPP_ */
