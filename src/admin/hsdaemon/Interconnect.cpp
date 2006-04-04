#include "Interconnect.h"
#include <stdexcept>
#include "util/int.h"
#include <string>
#include "core/buffers/TmpBuf.hpp"
#include "DaemonCommandDispatcher.h"
#include "ChildWaiter.h"
#include "InterconnectCommands.h"

namespace smsc{
namespace admin{
namespace hsdaemon{

namespace buf=smsc::core::buffers;

int Interconnect::Execute()
{
  running=true;
  smsc_log_info(log,"Starting interconnect acceptor");
  while(running)
  {
    net::Socket* sck=srvSck.Accept();
    if(!sck)
    {
      smsc_log_warn(log,"accept failed");
      break;
    }
    try{
      ProcessRequest(sck);
    }catch(std::exception& e)
    {
      smsc_log_warn(log,"exception during request processing:%s",e.what());
    }
    delete sck;
  }
  smsc_log_info(log,"Interconnect acceptor execution finished");
  return 0;
}


void Interconnect::ProcessRequest(net::Socket* sck)
{
  using namespace interconnect;
  uint16_t cmdId=readUInt16(sck);
  smsc_log_debug(log,"received cmd=%x",cmdId);
  sync::MutexGuard mg(DaemonCommandDispatcher::servicesListMutex);
  ServicesList& lst=DaemonCommandDispatcher::getServicesList();
  switch(cmdId)
  {
    case cmdSynchronizeServices:
    {
      uint32_t cnt=readUInt32(sck);
      std::string svcName;
      std::vector<std::string> failures;
      for(int i=0;i<cnt;i++)
      {
        svcName=readString(sck);
        if(!lst.isServiceExists(svcName.c_str()))
        {
          failures.push_back(svcName);
        };
      }
      writeUInt16(sck,failures.size());
      for(int i=0;i<failures.size();i++)
      {
        writeString(sck,failures[i]);
      }
    }break;
    case cmdGetServiceStatus:
    {
      std::string svcName=readString(sck);
      uint16_t status=-1;
      try{
        Service* svc=lst.get(svcName.c_str());
        status=svc->getStatus();
      }catch(std::exception& e)
      {
        smsc_log_warn(log,"Failed to get service status:%s",e.what());
      }
      writeUInt16(sck,status);
    }break;
    case cmdStartService:
    {
      std::string svcName=readString(sck);
      smsc_log_info(log,"remote request to start service '%s'",svcName.c_str());
      uint16_t status=0;
      try{
        Service* svc=lst.get(svcName.c_str());
        ChildShutdownWaiter::startService(svcName.c_str());
        status=1;
      }catch(std::exception& e)
      {
        smsc_log_warn(log,"faield to start service %s:%s",svcName.c_str(),e.what());
      }
      writeUInt16(sck,status);
    }break;
    case cmdShutdownService:
    {
      std::string svcName=readString(sck);
      bool switchOver=readUInt16(sck);
      smsc_log_info(log,"remote request to shutdown service '%s'",svcName.c_str());
      uint16_t status=0;
      try{
        Service* svc=lst.get(svcName.c_str());
        svc->setSwitchover(switchOver);
        svc->shutdown();
        status=1;
      }catch(std::exception& e)
      {
        smsc_log_warn(log,"failed to shutdown service %s:%s",svcName.c_str(),e.what());
      }
      writeUInt16(sck,status);
    }break;
    case cmdListServices:
    {
      writeUInt16(sck,lst.Count());
      lst.First();
      char* svcName;
      Service* svc;
      while(lst.Next(svcName,svc))
      {
        writeString(sck,svcName);
      }
    }break;
    case cmdGetServiceProperty:
    {
      std::string svcName=readString(sck);
      int prop=readUInt16(sck);
      try{
        Service* svc=lst.get(svcName.c_str());
        switch(prop)
        {
          case propArgs:
            writeString(sck,svc->getArgs());
            break;
          case propPreferredNode:
            writeString(sck,svc->getInfo().preferredNode.c_str());
            break;
          case propLogicalHostname:
            writeString(sck,svc->getInfo().hostName.c_str());
            break;
          case propServiceType:
            switch(svc->getType())
            {
              case ServiceInfo::standalone:
                writeString(sck,"standalone");
                break;
              case ServiceInfo::failover:
                writeString(sck,"failover");
                break;
            }
            break;
          case propAutostartDelay:
          {
            char buf[32];
            sprintf(buf,"%d",svc->getInfo().autostartDelay);
            writeString(sck,buf);
          }break;
          default:
          {
            writeString(sck,"Unknown property");
          }
        };
      }catch(std::exception& e)
      {
        writeString(sck,e.what());
      }
    }break;
    default:
    {
      smsc_log_warn(log,"Unknown command:%d\n",cmdId);
    }break;
  }
}

void Interconnect::sendSyncCommand()
{
  using namespace interconnect;
  sync::MutexGuard mg(clntMtx);
  if(clntSck.Connect()==-1)throw Exception("Failed to connect to %s:%d",otherHost.c_str(),otherPort);
  writeUInt16(&clntSck,cmdSynchronizeServices);
  std::vector<std::string> svcs;
  sync::MutexGuard mg2(DaemonCommandDispatcher::servicesListMutex);
  ServicesList& lst=DaemonCommandDispatcher::getServicesList();
  lst.First();
  char* name;
  Service* svc;
  while(lst.Next(name,svc))
  {
    svcs.push_back(name);
  }
  writeUInt32(&clntSck,svcs.size());
  for(int i=0;i<svcs.size();i++)writeString(&clntSck,svcs[i]);
  uint16_t failed=readUInt16(&clntSck);
  if(failed>0)smsc_log_warn(log,"%d services failed syncronization",failed);
  std::string svcName;
  for(int i=0;i<failed;i++)
  {
    svcName=readString(&clntSck);
    smsc_log_warn(log,"removing service %s",svcName.c_str());
    lst.remove(svcName.c_str());
  }
  clntSck.Close();
}

Service::run_status Interconnect::remoteGetServiceStatus(const char* svc)
{
  sync::MutexGuard mg(clntMtx);
  if(clntSck.Connect()==-1)throw Exception("Failed to connect to %s:%d",otherHost.c_str(),otherPort);
  return interconnect::remoteGetServiceStatus(clntSck,svc);
}

bool Interconnect::remoteStartService(const char* svc)
{
  sync::MutexGuard mg(clntMtx);
  if(clntSck.Connect()==-1)throw Exception("Failed to connect to %s:%d",otherHost.c_str(),otherPort);
  return interconnect::remoteStartService(clntSck,svc);
}

bool Interconnect::remoteShutdownService(const char* svc,bool switchOver)
{
  sync::MutexGuard mg(clntMtx);
  if(clntSck.Connect()==-1)throw Exception("Failed to connect to %s:%d",otherHost.c_str(),otherPort);
  return interconnect::remoteShutdownService(clntSck,svc,switchOver);
}


Interconnect* icon=0;

}
}
}
