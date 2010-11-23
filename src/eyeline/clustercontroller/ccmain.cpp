#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
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
#include "eyeline/clustercontroller/configregistry/ConfigRegistry.hpp"

static bool stopping=false;

extern "C" void sighandler(int sig)
{
  stopping=true;
}

int main(int argc,char* argv[])
{
  using namespace eyeline::clustercontroller::protocol::messages;
  smsc::logger::Logger::Init();
  sigset(SIGINT,sighandler);
  sigset(SIGTERM,sighandler);
  try{
    smsc::util::config::Manager::init("config.xml");
    smsc::util::config::Manager& cfg=  smsc::util::config::Manager::getInstance();
    using namespace eyeline::clustercontroller;
    configregistry::ConfigRegistry::Init(cfg.getString("configs.registryStoreFile"));
    smsc::closedgroups::ClosedGroupsManager::Init();
    smsc::closedgroups::ClosedGroupsManager::getInstance()->Load(cfg.getString("configs.closedgroups"));
    smsc::closedgroups::ClosedGroupsManager::getInstance()->enableControllerMode();
    profiler::ProfilerConfig::Init(cfg.getString("configs.profiler"));
    configregistry::ConfigRegistry::getInstance()->update(ctProfiles);
    configregistry::ConfigRegistry::getInstance()->update(ctClosedGroups);
    router::RouterConfig::Init(cfg.getString("configs.router"),cfg.getString("configs.smeman"));
    alias::AliasConfig::Init(cfg.getString("configs.aliasStore"));
    configregistry::ConfigRegistry::getInstance()->update(ctAliases);
    acl::AclConfig::Init(cfg.getString("configs.aclStore"));
    configregistry::ConfigRegistry::getInstance()->update(ctAcl);
    NetworkProtocol::Init();
    ConfigLockManager::Init();

    if(argc>1 && std::string(argv[1])=="interactive")
    {
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
        }else if(cmd=="getservicesstatus")
        {
          GetServicesStatus gss;
          if(NetworkProtocol::getInstance()->enqueueCommandToType(ctLoadBalancer,gss)==0)
          {
            printf("Load balancer not connected\n");
          }
        }else if(cmd=="disconnectservice")
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
    }else
    {
      while(!stopping)sleep(1);
    }
    NetworkProtocol::Deinit();
    ConfigLockManager::Shutdown();
  }catch(std::exception& e)
  {
    fprintf(stderr,"Startup exception: '%s'\n",e.what());
  }
  return 0;
}
