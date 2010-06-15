#include <stdio.h>
#include <stdlib.h>
#include "NetworkProtocol.hpp"
#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "protocol/messages/GetServicesStatus.hpp"
#include "eyeline/clustercontroller/ConfigLockManager.hpp"
#include "eyeline/clustercontroller/profiler/ProfilerConfig.hpp"
#include "smsc/closedgroups/ClosedGroupsManager.hpp"
#include "router/RouterConfig.hpp"
#include "alias/AliasConfig.hpp"
#include "eyeline/clustercontroller/acl/AclConfig.hpp"

int main()
{
  using namespace eyeline::clustercontroller::protocol::messages;
  smsc::logger::Logger::Init();
  try{
    smsc::util::config::Manager::init("config.xml");
    smsc::util::config::Manager& cfg=  smsc::util::config::Manager::getInstance();
    using namespace eyeline::clustercontroller;
    profiler::ProfilerConfig::Init(cfg.getString("configs.profiler"));
    smsc::closedgroups::ClosedGroupsManager::Init();
    smsc::closedgroups::ClosedGroupsManager::getInstance()->Load(cfg.getString("configs.closedgroups"));
    smsc::closedgroups::ClosedGroupsManager::getInstance()->enableControllerMode();
    router::RouterConfig::Init(cfg.getString("configs.router"),cfg.getString("configs.smeman"));
    alias::AliasConfig::Init(cfg.getString("configs.aliasStore"));
    acl::AclConfig::Init(cfg.getString("configs.aclStore"),cfg.getInt("configs.aclPreCreate"));
    NetworkProtocol::Init();
    ConfigLockManager::Init();
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
    ConfigLockManager::Shutdown();
  }catch(std::exception& e)
  {
    fprintf(stderr,"Startup exception: '%s'\n",e.what());
  }
  return 0;
}
