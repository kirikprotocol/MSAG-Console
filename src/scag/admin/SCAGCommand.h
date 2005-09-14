// 
// File:   SCAGCommand.h
// Author: loomox
//
// Created on 30 Mart 2005 Ç., 17:06
//

#ifndef _SCAG_ADMIN_SCAGCommand_H
#define	_SCAG_ADMIN_SCAGCommand_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"

// Its during for a test only
//#include "scag/scag.hpp"

#include "admin/service/Variant.h"
#include "admin/protocol/ResponseWriter.h"
#include "CommandActions.h"

#include "util/config/Manager.h"
#include "util/config/route/RouteConfig.h"
#include "alias/aliasman.h"
#include "util/config/alias/aliasconf.h"
#include "util/config/route/RouteConfig.h"
#include "router/route_manager.h"
#include "util/config/smeman/SmeManConfig.h"
#include "scag/scag.h"


// Its during for a test only
/*namespace scag {

using namespace smsc::smeman;
using smsc::alias::AliasManager;
using smsc::router::RouteManager;
using smsc::router::RouteInfo;
using scag::config::RouteConfig;

class Smsc
{
};

struct SmscConfigs{
  smsc::util::config::Manager* cfgman;
  smsc::util::config::smeman::SmeManConfig* smemanconfig;
  smsc::util::config::alias::AliasConfig* aliasconfig;
  smsc::util::config::route::RouteConfig* routesconfig;
  Hash<string> *licconfig;
};

}*/



namespace scag {
namespace admin {

using namespace smsc::admin::protocol;
using namespace scag;

class SCAGCommand : public Command
{
public:
  SCAGCommand(Command::Id id);
  virtual ~SCAGCommand();
  virtual Response * CreateResponse(scag::Scag * SmscApp);
  virtual scag::admin::Actions::CommandActions GetActions();
};

}
}

#endif	/* _SCAG_ADMIN_SCAGCommand_H */

