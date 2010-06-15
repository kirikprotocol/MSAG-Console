/*
 * RouterConfig.cpp
 *
 *  Created on: May 11, 2010
 *      Author: skv
 */

#include "RouterConfig.hpp"
#include "smsc/config/route/RouteConfig.h"
#include "smsc/config/smeman/SmeManConfig.h"
#include "smsc/router/route_manager.h"

namespace smsc{
namespace common{
extern void loadRoutes(smsc::router::RouteManager* rm,const smsc::config::route::RouteConfig& rc,bool traceit=false);
}
}

namespace eyeline {
namespace clustercontroller {
namespace router {

RouterConfig::RouterRef RouterConfig::instance;
RouterConfig::SmeTableImpl* RouterConfig::smeTable=0;
std::string RouterConfig::routerConfig;
std::string RouterConfig::smeConfig;

void RouterConfig::Init(const char* routeConfigPath,const char* smeConfigPath)
{
  routerConfig=routeConfigPath;
  smeConfig=smeConfigPath;
  instance=new smsc::router::RouteManager();
  smeTable=new RouterConfig::SmeTableImpl();
  smeTable->Load(smeConfigPath);
  smsc::config::route::RouteConfig rc;
  rc.load(routeConfigPath);
  //instance->assign()
  smsc::common::loadRoutes(instance.Get(),rc,true);
}

void RouterConfig::Reload()
{
  smsc::router::RouteManager* newInstance=new smsc::router::RouteManager();
  smeTable->Load(smeConfig.c_str());
  smsc::config::route::RouteConfig rc;
  rc.load(routerConfig.c_str());
  smsc::common::loadRoutes(newInstance,rc,true);
  instance=newInstance;
}

void RouterConfig::SmeTableImpl::Load(const char* path)
{
  smsc::config::smeman::SmeManConfig cfg;
  cfg.load(path);
  smsc::config::smeman::SmeManConfig::RecordIterator it=cfg.getRecordIterator();
  smsc::config::smeman::SmeRecord* rec;
  table.clear();
  while(it.fetchNext(rec))
  {
    smsc::smeman::SmeInfo info;
    info.systemId=rec->smeUid;
    info.disabled=rec->recdata.smppSme.disabled;
    table.push_back(info);
  }
}

}
}
}
