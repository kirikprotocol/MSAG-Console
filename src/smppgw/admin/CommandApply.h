// 
// File:   CommandApply.h
// Author: igork
//
// Created on 27 Май 2004 г., 17:06
//

#ifndef _SMPPGW_ADMIN_CommandApply_H
#define	_SMPPGW_ADMIN_CommandApply_H

#include <xercesc/dom/DOM.hpp>
#include "SmppGwCommand.h"
#include "CommandActions.h"

namespace smsc {
namespace smppgw {
namespace admin {

class CommandApply : public smsc::smppgw::admin::SmppGwCommand 
{
public:
  enum subjects {
    unknown,
    config,
    routes,
    providers,
    smscs
  };
  
  CommandApply(const xercesc::DOMDocument * document);
  virtual ~CommandApply();
  virtual Response * CreateResponse(smsc::smppgw::Smsc * SmscApp);
  virtual smsc::smppgw::admin::Actions::CommandActions GetActions();
  
  //subjects getSubject();

private:
  subjects subj;
};

}
}
}

#endif	/* _SMPPGW_ADMIN_CommandApply_H */

