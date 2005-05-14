
// 
// File:   CommandLoadRoutes.h
// Author: loomox
//
// Created on 24 Mart 2005 Ç., 17:06
//

#ifndef _SCAG_ADMIN_CommandLoadRoutes_H
#define	_SCAG_ADMIN_CommandLoadRoutes_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"
#include "scag/smsc.hpp"
#include "admin/service/Variant.h"
#include "SCAGCommand.h"

namespace smsc {
namespace scag {
namespace admin {


class CommandLoadRoutes : public smsc::scag::admin::SCAGCommand 
{
public:
  
  CommandLoadRoutes(const xercesc::DOMDocument * document);
  virtual ~CommandLoadRoutes();
  virtual Response * CreateResponse(smsc::scag::Smsc * SmscApp);
};

}
}
}

#endif	/* _SCAG_ADMIN_CommandLoadRoutes_H */

