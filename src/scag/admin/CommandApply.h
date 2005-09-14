// 
// File:   CommandApply.h
// Author: igork
//
// Created on 27 Май 2004 г., 17:06
//

#ifndef _SCAG_ADMIN_CommandApply_H
#define	_SCAG_ADMIN_CommandApply_H

#include <xercesc/dom/DOM.hpp>
#include "SCAGCommand.h"
#include "CommandActions.h"

namespace scag {
namespace admin {

class CommandApply : public scag::admin::SCAGCommand 
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
  virtual Response * CreateResponse(scag::Scag * ScagApp);
  virtual scag::admin::Actions::CommandActions GetActions();
  
  //subjects getSubject();

private:
  subjects subj;
};

}
}

#endif	/* _SCAG_ADMIN_CommandApply_H */

