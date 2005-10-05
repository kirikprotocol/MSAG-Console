#include "Interconnect.h"
#include <stdexcept>
#include "util/int.h"
#include <string>
#include "core/buffers/TmpBuf.hpp"
#include "DaemonCommandDispatcher.h"
#include "ChildWaiter.h"

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

uint16_t readUInt16(net::Socket* sck)
{
  uint16_t rv;
  if(sck->ReadAll(reinterpret_cast<char*>(&rv),sizeof(rv))==-1)throw Exception("Failed to read uint16_t from socket");
  return ntohs(rv);
}

uint32_t readUInt32(net::Socket* sck)
{
  uint32_t rv;
  if(sck->ReadAll(reinterpret_cast<char*>(&rv),sizeof(rv))==-1)throw Exception("Failed to read uint32_t from socket");
  return ntohl(rv);
}

std::string readString(net::Socket* sck)
{
  uint16_t len=readUInt16(sck);
  TmpBuf<char,64> buf(len);
  if(sck->ReadAll(buf.get(),len)==-1)throw Exception("Failed to read string of length %d from socket",len);
  std::string rv;
  return std::string(buf.get(),len);
}

void writeUInt16(net::Socket* sck,uint16_t val)
{
  val=htons(val);
  if(sck->WriteAll(reinterpret_cast<char*>(&val),sizeof(val))==-1)throw Exception("Failed to write uint16_t to socket");
}

void writeUInt32(net::Socket* sck,uint32_t val)
{
  val=htonl(val);
  if(sck->WriteAll(reinterpret_cast<char*>(&val),sizeof(val))==-1)throw Exception("Failed to write uint32_t to socket");
}

void writeString(net::Socket* sck,const std::string& val)
{
  uint16_t len=static_cast<uint16_t>(val.length());
  writeUInt16(sck,len);
  if(sck->WriteAll(val.c_str(),len)==-1)throw Exception("Failed to write string of length %s to socket",len);
}


void Interconnect::ProcessRequest(net::Socket* sck)
{
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
      Service* svc=lst.get(svcName.c_str());
      uint16_t status=svc->getStatus();
      writeUInt16(sck,status);
    }break;
    case cmdStartService:
    {
      std::string svcName=readString(sck);
      smsc_log_info(log,"remote request to start service '%s'",svcName.c_str());
      Service* svc=lst.get(svcName.c_str());
      uint16_t status=0;
      try{
        ChildShutdownWaiter::startService(svcName.c_str());
        status=1;
      }catch(std::exception& e)
      {
        smsc_log_warn(log,"faield to start service %s:%s",svcName.c_str(),e.what());
      }
      writeUInt16(sck,status);
    }break;
  }
}

void Interconnect::sendSyncCommand()
{
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
  writeUInt16(&clntSck,cmdGetServiceStatus);
  writeString(&clntSck,svc);
  uint16_t rv=readUInt16(&clntSck);
  clntSck.Close();
  return (Service::run_status)rv;
}

bool Interconnect::remoteStartService(const char* svc)
{
  sync::MutexGuard mg(clntMtx);
  if(clntSck.Connect()==-1)throw Exception("Failed to connect to %s:%d",otherHost.c_str(),otherPort);
  writeUInt16(&clntSck,cmdStartService);
  writeString(&clntSck,svc);
  uint16_t rv=readUInt16(&clntSck);
  clntSck.Close();
  return rv!=0;
}

Interconnect* icon=0;

}
}
}
