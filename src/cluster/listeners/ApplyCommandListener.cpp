#include "ApplyCommandListener.h"
#include "util/config/alias/aliasconf.h"
#include "system/common/rescheduler.hpp"
#include "util/findConfigFile.h"
#include "resourcemanager/ResourceManager.hpp"
#include "router/route_manager.h"
#include "util/config/route/RouteConfig.h"
#include "util/Exception.hpp"

namespace smsc {
namespace system{
    extern void loadRoutes(smsc::router::RouteManager* rm,const smsc::util::config::route::RouteConfig& rc,bool traceit=false);
}
}

namespace smsc {
namespace cluster {

using smsc::system::Smsc;

using smsc::system::loadRoutes;
using smsc::util::Exception;
using namespace smsc::util;
using namespace smsc::core::buffers;

ApplyCommandListener::ApplyCommandListener(const smsc::system::SmscConfigs *configs_, /*smsc::smeman::SmeManager *smeman_,*/
                                           smsc::system::Smsc* app_)
    : configs(configs_),
    //smeman(smeman_),
    app(app_)
{
}

void ApplyCommandListener::handle(const Command& command)
{
  switch( command.getType() ){
    case APPLYROUTES_CMD:
      applyRoutes();
      break;
    case APPLYALIASES_CMD:
      applyAliases();
      break;
    case APPLYRESCHEDULE_CMD:
      applyReschedule();
      break;
    case APPLYLOCALERESOURCE_CMD:
      applyLocalResource();
      break;
    case APPLYCONFIG_CMD:
      applyConfig();
      break;
    default:break;
  }
}

void ApplyCommandListener::applyConfig()
{
  smsc::util::config::Manager::reinit();
}


void ApplyCommandListener::applyRoutes()
{
    /*configs->routesconfig->reload();
    configs->smemanconfig->reload();

    auto_ptr<RouteManager> router(new RouteManager());
    router->assign(smeman);
    try{
        loadRoutes(router.get(),*(configs->routesconfig));
    }catch(...)
    {
        __warning__("Failed to load routes");
    }
    ResetRouteManager(router.release());*/
    if(!app) throw Exception("Smsc undefined");

    configs->routesconfig->reload();
    configs->smemanconfig->reload();
    app->reloadRoutes();
}

void ApplyCommandListener::applyAliases()
{
    if(!app) throw Exception("Smsc undefined");
    //configs->aliasconfig->reload();
    //app->reloadAliases(*configs);


    /*auto_ptr<AliasManager> aliaser(new AliasManager());
    {
        smsc::util::config::alias::AliasConfig::RecordIterator i =
                                configs->aliasconfig->getRecordIterator();
        while(i.hasRecord())
        {
            smsc::util::config::alias::AliasRecord *rec;
            i.fetchNext(rec);
            __trace2__("adding %20s %20s",rec->addrValue,rec->aliasValue);
            smsc::alias::AliasInfo ai;
            ai.addr = smsc::sms::Address(
                strlen(rec->addrValue),
                rec->addrTni,
                rec->addrNpi,
                rec->addrValue);
            ai.alias = smsc::sms::Address(
                strlen(rec->aliasValue),
                rec->aliasTni,
                rec->aliasNpi,
                rec->aliasValue);
            ai.hide = rec->hide;
            aliaser->addAlias(ai);
        }
        aliaser->commit();
    }

    ResetAliases(aliaser.release());*/
}

void ApplyCommandListener::applyReschedule()
{
    smsc::system::RescheduleCalculator::init(smsc::util::findConfigFile("schedule.xml"));
}

void ApplyCommandListener::applyLocalResource()
{
    if(!configs) Exception("Configs undifend");
    if(!app) throw Exception("Smsc undefined");
    smsc::resourcemanager::ResourceManager::reload(configs->cfgman->getString("core.locales"), configs->cfgman->getString("core.default_locale"));
}

}
}
