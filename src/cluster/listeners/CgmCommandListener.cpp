#include "CgmCommandListener.h"
#include "closedgroups/ClosedGroupsInterface.hpp"

namespace smsc {
namespace cluster {

void CgmCommandListener::handle(const Command& command)
{
  using smsc::closedgroups::ClosedGroupsInterface;
  ClosedGroupsInterface* cgm=ClosedGroupsInterface::getInstance();
  switch(command.getType())
  {
    case CGM_ADDGRP_CMD:
    {
      const CgmAddGrpCommand& addcmd=dynamic_cast<const CgmAddGrpCommand&>(command);
      cgm->AddGroup(addcmd.id,addcmd.name.c_str());
    }break;
    case CGM_DELGRP_CMD:
    {
      const CgmDelGrpCommand& delcmd=dynamic_cast<const CgmDelGrpCommand&>(command);
      cgm->DeleteGroup(delcmd.id);
    }break;
    case CGM_ADDADDR_CMD:
    {
      const CgmAddAddrCommand& addcmd=dynamic_cast<const CgmAddAddrCommand&>(command);
      cgm->AddAddrToGroup(addcmd.id,addcmd.addr.c_str());
    }break;
    case CGM_DELADDR_CMD:
    {
      const CgmDelAddrCommand& delcmd=dynamic_cast<const CgmDelAddrCommand&>(command);
      cgm->RemoveAddrFromGroup(delcmd.id,delcmd.addr.c_str());
    }break;
    default:
    {
      smsc_log_warn(logger,"Unsupported command with id:%x",command.getType());
    }
  }
}

}
}
