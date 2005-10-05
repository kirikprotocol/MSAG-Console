// 
// File:   CommandLoadRoutes.cpp
// Author: loomox
//
// Created on 24 march 2005 
//

#include "CommandLoadRoutes.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"
#include "smeman/smeproxy.h"
#include <util/config/route/RouteConfig.h>

#include <router/route_types.h>
#include <sms/sms.h>
#include <sms/sms_const.h>
#include "scag/config/ConfigManager.h"



namespace scag {
namespace admin {

using namespace smsc::util::xml;
using namespace smsc::sms;
using namespace smsc::util::config;
using namespace smsc::util::config::route;
using scag::config::ConfigManager;



CommandLoadRoutes::CommandLoadRoutes(const xercesc::DOMDocument * doc)
  : SCAGCommand((Command::Id)CommandIds::loadRoutes)
{
  smsc_log_debug(logger, "LoadRoutes command");
}


Response * CommandLoadRoutes::CreateResponse(scag::Scag * SmscApp)
{
    smsc_log_info(logger, "CommandLoadRoutes is processing");

    smsc::admin::service::Variant result(smsc::admin::service::StringListType);
    result.appendValueToStringList("Routes configuration successfully loaded");

  try
  {
      ConfigManager& cfg = ConfigManager::Instance();
      cfg.reloadConfig(scag::config::ROUTE_CFG);

      /*RouteConfig cfg;
      if (cfg.load("conf/routes__.xml") == RouteConfig::fail)
          throw AdminException("Load routes config file failed.");

      vector<std::string> traceBuff;

      SmscApp->reloadTestRoutes(cfg);
      SmscApp->getTestRouterInstance()->enableTrace(true);
      SmscApp->getTestRouterInstance()->getTrace(traceBuff);*/

      // 0:   Message (Routes successfully loaded)
      // 1..: Trace (if any)

      smsc::admin::service::Variant result(smsc::admin::service::StringListType);
      result.appendValueToStringList("Routes configuration successfully loaded");

      //for (int i=0; i<traceBuff.size(); i++)
      //    result.appendValueToStringList(traceBuff[i].c_str());
  }
  catch (AdminException& aexc) {
      smsc_log_info(logger, "CommandLoadRoutes exception, %s", aexc.what());
      throw;
  }
  catch (ConfigException& cexc) {
      smsc_log_info(logger, "CommandLoadRoutes exception, %s", cexc.what());
      throw AdminException("Load routes config file failed. Cause: %s", cexc.what());
  }
  catch (std::exception& exc) {
      smsc_log_info(logger, "CommandLoadRoutes exception, %s", exc.what());
      throw AdminException("Load routes failed. Cause: %s.", exc.what());
  }
  catch (...) {
      smsc_log_info(logger, "CommandLoadRoutes exception, unknown exception.");
      throw AdminException("Load routes failed. Cause is unknown.");
  }

  smsc_log_info(logger, "CommandLoadRoutes is processed ok");
  return new Response(Response::Ok, result);

}

CommandLoadRoutes::~CommandLoadRoutes()
{
  id = undefined;
}




}
}

