// 
// File:   SmppGwCommand.h
// Author: loomox
//
// Created on 30 Mart 2005 Ç., 17:06
//

#ifndef _SMPPGW_ADMIN_SmppGwCommand_H
#define	_SMPPGW_ADMIN_SmppGwCommand_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"
#include "smppgw/smsc.hpp"
#include "admin/service/Variant.h"
#include "admin/protocol/ResponseWriter.h"
#include "CommandActions.h"

namespace smsc {
namespace smppgw {
namespace admin {

using namespace smsc::admin::protocol;
using namespace smsc::smppgw;
using namespace smsc::admin::protocol;

class SmppGwCommand : public Command
{
public:
  SmppGwCommand(Command::Id id);
  virtual ~SmppGwCommand();
  virtual Response * CreateResponse(smsc::smppgw::Smsc * SmscApp);
  virtual smsc::smppgw::admin::Actions::CommandActions GetActions();
};

}
}
}

#endif	/* _SMPPGW_ADMIN_SmppGwCommand_H */

