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

// Its during for a test only
//#include "scag/scag.h"

#include "admin/service/Variant.h"

namespace scag {
namespace admin {


class CommandTraceRoute : public scag::admin::SCAGCommand 
{
public:
  
  CommandTraceRoute(const xercesc::DOMDocument * doc);
  virtual ~CommandTraceRoute();
  virtual Response * CreateResponse(scag::Smsc * SmscApp);

protected:
  std::string dstAddr;
  std::string srcAddr;
  std::string srcSysId;
};

}
}

#endif	/* _SCAG_ADMIN_CommandTraceRoute_H */

