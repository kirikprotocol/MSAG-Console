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



namespace scag {
namespace admin {

using namespace smsc::util::xml;
using namespace smsc::sms;
using namespace smsc::util::config;
using namespace smsc::util::config::route;



CommandLoadRoutes::CommandLoadRoutes(const xercesc::DOMDocument * doc)
  : SCAGCommand((Command::Id)CommandIds::loadRoutes)
{
  smsc_log_debug(logger, "LoadRoutes command");
}


Response * CommandLoadRoutes::CreateResponse(scag::Smsc * SmscApp)
{
  try
  {
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

      return new Response(Response::Ok, result);
  }
  catch (AdminException& aexc) {
      throw;
  }
  catch (ConfigException& cexc) {
      throw AdminException("Load routes config file failed. Cause: %s", cexc.what());
  }
  catch (std::exception& exc) {
      throw AdminException("Load routes failed. Cause: %s.", exc.what());
  }
  catch (...) {
      throw AdminException("Load routes failed. Cause is unknown.");
  }

}

CommandLoadRoutes::~CommandLoadRoutes()
{
  id = undefined;
}




}
}

