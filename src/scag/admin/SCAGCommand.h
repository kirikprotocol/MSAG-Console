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
#include "scag/smsc.hpp"
#include "admin/service/Variant.h"
#include "admin/protocol/ResponseWriter.h"
#include "CommandActions.h"

namespace scag {
namespace admin {

using namespace smsc::admin::protocol;
using namespace scag;
using namespace smsc::admin::protocol;

class SCAGCommand : public Command
{
public:
  SCAGCommand(Command::Id id);
  virtual ~SCAGCommand();
  virtual Response * CreateResponse(scag::Smsc * SmscApp);
  virtual scag::admin::Actions::CommandActions GetActions();
};

}
}

#endif	/* _SCAG_ADMIN_SCAGCommand_H */

