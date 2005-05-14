// 
// File:   CommandTraceRoute.h
// Author: loomox
//
// Created on 22 Mart 2005 Ç., 17:06
//

#ifndef _SCAG_ADMIN_CommandTraceRoute_H
#define	_SCAG_ADMIN_CommandTraceRoute_H

#include <xercesc/dom/DOM.hpp>
#include "SCAGCommand.h"
#include "scag/smsc.hpp"
#include "admin/service/Variant.h"

namespace smsc {
namespace scag {
namespace admin {


class CommandTraceRoute : public smsc::scag::admin::SCAGCommand 
{
public:
  
  CommandTraceRoute(const xercesc::DOMDocument * doc);
  virtual ~CommandTraceRoute();
  virtual Response * CreateResponse(smsc::scag::Smsc * SmscApp);

protected:
  std::string dstAddr;
  std::string srcAddr;
  std::string srcSysId;
};

}
}
}

#endif	/* _SCAG_ADMIN_CommandTraceRoute_H */

