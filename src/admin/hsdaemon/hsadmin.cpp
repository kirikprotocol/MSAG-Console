#include <stdio.h>
#include "core/network/Socket.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#include "InterconnectCommands.h"

namespace net=smsc::core::network;

using namespace smsc::admin::hsdaemon;

struct HostPort{
  HostPort():port(0){}
  HostPort(const std::string& argHost,int argPort):host(argHost),port(argPort){}
  std::string host;
  int port;
};

std::vector<HostPort> hosts;

int getServiceStatus(const char* serviceId,int& hostIdx)
{
  bool connectOk=false;
  for(hostIdx=0;hostIdx<hosts.size();hostIdx++)
  {
    std::string host=hosts[hostIdx].host;
    int port=hosts[hostIdx].port;
    net::Socket clntSck;
    if(clntSck.Init(host.c_str(),port,0)==-1)continue;
    if(clntSck.Connect()==-1)continue;
    connectOk=true;
    int rv=interconnect::remoteGetServiceStatus(clntSck,serviceId);
    if(rv==Service::stopped)continue;
    return rv;
  }
  if(connectOk)return Service::stopped;
  return -1;
}

void connectSocket(net::Socket& sck,int idx=0)
{
  for(int i=idx;i<hosts.size();i++)
  {
    if(sck.Init(hosts[i].host.c_str(),hosts[i].port,0)==-1)continue;
    if(sck.Connect()==-1)continue;
    return;
  }
  throw std::runtime_error("Failed to connect to either node\n");
}

int main(int argc,char* argv[])
{
  if(argc<3)
  {
    printf("Usage: %s host:port[,host:port] command [args]\n");
    printf("Supported commands:\n");
    printf("status [serviceId]\n");
    printf("start serviceId\n");
    printf("stop serviceId\n");
    printf("switchover serviceId\n");
    printf("getproperty serviceId propertyName\n");
    return -1;
  }
  const char* hostList=argv[1];
  for(;;)
  {
    char host[64];
    int port;
    int pos;
    if(sscanf(hostList,"%64[^:]:%d%n",host,&port,&pos)!=2)
    {
      break;
    }
    hosts.push_back(HostPort(host,port));
    if(hostList[pos]!=',')break;
    hostList+=pos+1;
  }
  if(hosts.size()==0)
  {
    printf("Failed to parse hosts list\n");
    return -1;
  }
  std::string command=argv[2];
  try{
    if(command=="status")
    {
      std::list<std::string> lst;
      if(argc==4)
      {
        lst.push_back(argv[3]);
      }else
      {
        net::Socket sck;
        connectSocket(sck);
        interconnect::remoteListServices(sck,lst);
      }
      for(std::list<std::string>::iterator it=lst.begin();it!=lst.end();it++)
      {
        int idx=0;
        int st=getServiceStatus(it->c_str(),idx);
        if(st<0)
        {
          printf("Failed to connect to either node!\n");
          return -1;
        }
        printf("Status of service %s:",it->c_str());
        switch(st)
        {
          case Service::stopped:printf("stopped");break;
          case Service::starting:printf("starting");break;
          case Service::running:printf("running");break;
          case Service::stopping:printf("stopping");break;
        };
        if(idx<hosts.size())
        {
          printf(" at %s:%d",hosts[idx].host.c_str(),hosts[idx].port);
        }
        printf("\n");
      }
    }else if(command=="start")
    {
      if(argc<4)
      {
        printf("Not enough arguments for start command\n");
        printf("Usage: start serviceId\n");
        return -1;
      }
      std::string svcId=argv[3];
      net::Socket sck;
      connectSocket(sck);
      std::string prefNode=interconnect::remoteGetServiceProperty(sck,svcId.c_str(),interconnect::propPreferredNode);
      int idx=0;
      if(prefNode.length()>0)
      {
        for(;idx<hosts.size();idx++)
        {
          if(hosts[idx].host==prefNode)break;
        }
        if(idx>=hosts.size())
        {
          printf("Failed to detect service's preferred node:%s\n",prefNode.c_str());
        }
      }

      connectSocket(sck,idx);
      if(interconnect::remoteStartService(sck,svcId.c_str()))
      {
        printf("Command successful\n");
      }else
      {
        printf("Command failed\n");
      }
    }else if(command=="stop" || command=="switchover")
    {
      if(argc<4)
      {
        printf("Not enough arguments for start command\n");
        printf("Usage: stop serviceId\n");
        return -1;
      }
      int idx=0;
      int st=getServiceStatus(argv[3],idx);
      if(st<0)
      {
        printf("Failed to connect to either node!\n");
        return -1;
      }
      if(idx>=hosts.size())
      {
        printf("Service is not running on any node\n");
        return -1;
      }

      net::Socket sck;
      connectSocket(sck,idx);
      if(interconnect::remoteShutdownService(sck,argv[3],command=="switchover"))
      {
        printf("Command successful\n");
      }else
      {
        printf("Command failed\n");
      }
    }else if(command=="getproperty")
    {
      if(argc<5)
      {
        printf("Not enough arguments for getproperty command\n");
        printf("Usage: getproperty serviceId propertyName\n");
        printf("Valid properties: \n"
          "Args, PreferredNode, LogicalHostname, ServiceType, AutostartDelay\n");
        return -1;
      }
      std::string svcId=argv[3];
      std::string prop=argv[4];
      for(int i=0;i<prop.length();i++)prop[i]=tolower(prop[i]);
      interconnect::ServiceProperty propVal=(interconnect::ServiceProperty)-1;
      if(prop=="args")propVal=interconnect::propArgs;
      else if(prop=="preferrednode")propVal=interconnect::propPreferredNode;
      else if(prop=="logicalhostname")propVal=interconnect::propLogicalHostname;
      else if(prop=="servicetype")propVal=interconnect::propServiceType;
      else if(prop=="autostartdelay")propVal=interconnect::propAutostartDelay;
      else
      {
        printf("Unknown property:%s\n",prop.c_str());
        return -1;
      }
      net::Socket sck;
      connectSocket(sck);
      printf("%s\n",interconnect::remoteGetServiceProperty(sck,svcId.c_str(),propVal).c_str());
    }
  }catch(std::exception& e)
  {
    printf("Exception: %s\n",e.what());
  }
  return 0;
}
