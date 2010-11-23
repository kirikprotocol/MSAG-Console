/*
 * RouterConfig.cpp
 *
 *  Created on: May 11, 2010
 *      Author: skv
 */

#include "RouterConfig.hpp"
#include "smsc/config/route/RouteConfig.h"
#include "smsc/config/smeman/SmeManConfig.h"
#include "smsc/router/Router.hpp"


namespace eyeline {
namespace clustercontroller {
namespace router {

RouterConfig::SmeTableImpl* RouterConfig::smeTable=0;
std::string RouterConfig::routerConfig;
std::string RouterConfig::smeConfig;

smsc::logger::Logger* log;

void RouterConfig::Init(const char* routeConfigPath,const char* smeConfigPath)
{
  log=smsc::logger::Logger::getInstance("router.cfg");
  routerConfig=routeConfigPath;
  smeConfig=smeConfigPath;
  smeTable=new RouterConfig::SmeTableImpl();
  smeTable->Load(smeConfigPath);
  smsc::config::route::RouteConfig rc;
  rc.load(routeConfigPath);
  smsc::router::Router::Init();
  smsc::router::Router::getInstance()->Load(smeTable,rc);
  //instance->assign()
}

void RouterConfig::Reload()
{
  smeTable->Load(smeConfig.c_str());
  smsc::config::route::RouteConfig rc;
  rc.load(routerConfig.c_str());
  smsc::router::Router::getInstance()->Load(smeTable,rc);
}

void RouterConfig::SmeTableImpl::Load(const char* path)
{
  smsc_log_debug(log,"Loading sme table");
  smsc::config::smeman::SmeManConfig cfg;
  cfg.load(path);
  smsc::config::smeman::SmeManConfig::RecordIterator it=cfg.getRecordIterator();
  smsc::config::smeman::SmeRecord* rec;
  table.clear();
  while(it.fetchNext(rec)==smsc::config::smeman::SmeManConfig::success)
  {
    smsc_log_debug(log,"load sme %s:%s",rec->smeUid,rec->smppSme.disabled?"disabled":"enabled");
    smsc::smeman::SmeInfo info;
    info.systemId=rec->smeUid;
    info.disabled=rec->smppSme.disabled;
    table.push_back(info);
  }
}

}
}
}
