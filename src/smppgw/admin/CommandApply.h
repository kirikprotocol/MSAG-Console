// 
// File:   CommandApply.h
// Author: igork
//
// Created on 27 ��� 2004 �., 17:06
//

#ifndef _SMPPGW_ADMIN_CommandApply_H
#define	_SMPPGW_ADMIN_CommandApply_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"

namespace smsc {
namespace smppgw {
namespace admin {

class CommandApply : public smsc::admin::protocol::Command
{
public:
  enum subjects {
    unknown,
    config,
    routes
  };
  
  CommandApply(const xercesc::DOMDocument * document);
  virtual ~CommandApply();
  
  subjects getSubject();

private:
  subjects subj;
};

}
}
}

#endif	/* _SMPPGW_ADMIN_CommandApply_H */

