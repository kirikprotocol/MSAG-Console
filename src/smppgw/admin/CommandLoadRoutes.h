
// 
// File:   CommandLoadRoutes.h
// Author: loomox
//
// Created on 24 Mart 2005 Ç., 17:06
//

#ifndef _SMPPGW_ADMIN_CommandLoadRoutes_H
#define	_SMPPGW_ADMIN_CommandLoadRoutes_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"
#include "smppgw/smsc.hpp"
#include "admin/service/Variant.h"
#include "SmppGwCommand.h"

namespace smsc {
namespace smppgw {
namespace admin {


class CommandLoadRoutes : public smsc::smppgw::admin::SmppGwCommand 
{
public:
  
  CommandLoadRoutes(const xercesc::DOMDocument * document);
  virtual ~CommandLoadRoutes();
  virtual Response * CreateResponse(smsc::smppgw::Smsc * SmscApp);
};

}
}
}

#endif	/* _SMPPGW_ADMIN_CommandLoadRoutes_H */

