
// 
// File:   CommandLoadRoutes.h
// Author: loomox
//
// Created on 24 Mart 2005 �., 17:06
//

#ifndef _SCAG_ADMIN_CommandLoadRoutes_H
#define	_SCAG_ADMIN_CommandLoadRoutes_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"
#include "scag/smsc.hpp"
#include "admin/service/Variant.h"
#include "SCAGCommand.h"

namespace scag {
namespace admin {


class CommandLoadRoutes : public scag::admin::SCAGCommand 
{
public:
  
  CommandLoadRoutes(const xercesc::DOMDocument * document);
  virtual ~CommandLoadRoutes();
  virtual Response * CreateResponse(scag::Smsc * SmscApp);
};

}
}

#endif	/* _SCAG_ADMIN_CommandLoadRoutes_H */

