// 
// File:   CommandTraceRoute.h
// Author: loomox
//
// Created on 22 Mart 2005 Ç., 17:06
//

#ifndef _SMPPGW_ADMIN_CommandTraceRoute_H
#define	_SMPPGW_ADMIN_CommandTraceRoute_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"
#include "smppgw/smsc.hpp"
#include "admin/service/Variant.h"

namespace smsc {
namespace smppgw {
namespace admin {


class CommandTraceRoute : public smsc::admin::protocol::Command
{
public:
  
  CommandTraceRoute(const xercesc::DOMDocument * doc);
  virtual ~CommandTraceRoute();
  smsc::admin::service::Variant GetTraceResult(smsc::smppgw::Smsc * SmscApp);

protected:
  std::string dstAddr;
  std::string srcAddr;
  std::string srcSysId;
};

}
}
}

#endif	/* _SMPPGW_ADMIN_CommandTraceRoute_H */

