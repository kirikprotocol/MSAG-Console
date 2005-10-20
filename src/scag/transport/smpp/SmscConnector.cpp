#include "SmscConnector.h"

namespace scag{
namespace transport{
namespace smpp{

void SmscConnector::addSmscConnect(const SmscConnectInfo& info)
{
  MutexGuard mg(mtx);
  if(smscConnections.Exists(info.regSysId.c_str()))
    throw SmscConnectAlreadyExistsException(info.regSysId.c_str());
  smscConnections.Insert(info.regSysId.c_str(),info);
  tp.startTask(new SmscConnectTask(this,info));
}

void SmscConnector::deleteSmscConnect(const char* sysId)
{
  MutexGuard mg(mtx);
  if(!smscConnections.Exists(sysId))
  {
    //throw TODO!!!
    return;
  }
  smscConnections.Delete(sysId);
}

void SmscConnector::updateSmscConnect(const SmscConnectInfo& info)
{
  MutexGuard mg(mtx);
  SmscConnectInfo* ptr=smscConnections.GetPtr(info.regSysId.c_str());
  if(!ptr)
  {
    //throw TODO!!!
    return;
  }
  *ptr=info;
}

void SmscConnector::reportSmscDisconnect(const char* sysId)
{
  __trace2__("Disconnect of %s reported",sysId);
  if(!active)return;
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
