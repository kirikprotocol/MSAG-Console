// 
// File:   CommandTraceRoute.h
// Author: loomox
//
// Created on 22 Mart 2005 �., 17:06
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
  
  CommandTraceRoute(const xercesc::DOMDocument * document);
  virtual ~CommandTraceRoute();
  smsc::admin::service::Variant GetTraceResult(smsc::smppgw::Smsc * SmscApp);

protected:
  std::auto_ptr<char> dstAddr;
  std::auto_ptr<char> srcAddr;
  std::auto_ptr<char> srcSysId;
};

}
}
}

#endif	/* _SMPPGW_ADMIN_CommandTraceRoute_H */

