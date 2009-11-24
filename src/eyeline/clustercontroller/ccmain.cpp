#include <stdio.h>
#include <stdlib.h>
#include "NetworkProtocol.hpp"
#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "protocol/messages/GetServicesStatus.hpp"

int main()
{
  using namespace eyeline::clustercontroller::protocol::messages;
  smsc::logger::Logger::Init();
  smsc::util::config::Manager::init("config.xml");
  try{
    using namespace eyeline::clustercontroller;
    NetworkProtocol::Init();
    char buf[128];
    printf("Cluster controller console\n");
    while(fgets(buf,(int)sizeof(buf),stdin))
    {
      std::string cmd=buf;
      if(cmd.length() && cmd[cmd.length()-1]<=32)
      {
        cmd.erase(cmd.length()-1);
      }
      std::string args;
      std::string::size_type spPos=cmd.find(' ');
      if(spPos!=std::string::npos)
      {
        std::string::size_type orgSpPos=spPos;
        while(cmd[spPos]==' ')spPos++;
        args=cmd.substr(spPos);
        cmd.erase(orgSpPos);
      }
      if(cmd=="quit" || cmd=="exit")
      {
        break;
      }else
      if(cmd=="getservicesstatus")
      {
        GetServicesStatus gss;
        if(NetworkProtocol::getInstance()->enqueueCommandToType(ctLoadBalancer,gss)==0)
        {
          printf("Load balancer not connected\n");
        }
      }else
      if(cmd=="disconnectservice")
      {
        if(args.length()==0)
        {
          printf("argument required\n");
          continue;
        }
        DisconnectService ds;
        ds.setServiceId(args);
        if(NetworkProtocol::getInstance()->enqueueCommandToType(ctLoadBalancer,ds)==0)
        {
          printf("Load balancer not connected\n");
        }
      }else if(cmd=="applyroutes")
      {
        ApplyRoutes ar;
        NetworkProtocol::getInstance()->enqueueCommandToType(ctSmsc,ar);
      }else
      {
        printf("Unknown command\n");
      }
    }
    NetworkProtocol::Deinit();
  }catch(std::exception& e)
  {
    fprintf(stderr,"Startup exception: '%s'\n",e.what());
  }
}
