#include "SmscConnector2.h"

namespace scag2 {
namespace transport {
namespace smpp {

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
    smsc_log_warn(log, "deleteSmscConnect: SmscConnection with systemId: %s doesn't exist", sysId);
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
    smsc_log_warn(log, "updateSmscConnect: SmscConnection with systemId: %s doesn't exist", info.regSysId.c_str());
    return;
  }
  *ptr=info;
}

void SmscConnector::reportSmscDisconnect(const char* sysId)
{
  smsc_log_info(log, "Disconnect of %s reported", sysId);
  if(!active)return;
  MutexGuard mg(mtx);
  SmscConnectInfo* ptr=smscConnections.GetPtr(sysId);
  if(!ptr)
  {
    smsc_log_warn(log, "reposrtSmscDisconnect: SmscConnection with systemId: %s doesn't exist", sysId);    
    //warning
    return;
  }
  ptr->reportFailure();
  tp.startTask(new SmscConnectTask(this,*ptr));
}

}//smpp
}//transport
}//scag
