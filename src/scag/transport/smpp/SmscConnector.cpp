#include "SmscConnector.h"

namespace scag{
namespace transport{
namespace smpp{

void SmscConnector::init()
{
//
}

void SmscConnector::reportSmscDisconnect(const char* sysId)
{
  sync::MutexGuard mg(mtx);
  SmscConnectInfo* ptr=smscConnections.GetPtr(sysId);
  if(!ptr)
  {
    //warning
    return;
  }
  ptr->reportFailure();
  tp.startTask(new SmscConnectTask(this,*ptr));
}

}//smpp
}//transport
}//scag
