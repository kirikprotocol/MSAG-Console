#include "system/abonentinfo/AbonentInfo.hpp"

namespace smsc{
namespace system{
namespace abonentinfo{

int AbonentInfoSme::Execute()
{
  SmscCommand cmd,resp;

  while(!isStopping)
  {
    if(!hasOutput())
    {
      waitFor();
    }
    if(!hasOutput())continue;
    cmd=getOutgoingCommand();
    if(cmd->cmdid!=smsc::smeman::DELIVERY)
    {
      __trace2__("Profiler: incorrect command submitted");
      continue;
    }

  }
  return 0;
}

};//abonentinfo
};//system
};//smsc
