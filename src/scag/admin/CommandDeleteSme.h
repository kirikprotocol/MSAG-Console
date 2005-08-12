// 
// File:   CommandDeleteSme.h
// Author: igork
//
// Created on 31 ������ 2004 �., 17:55
//

#ifndef _CommandDeleteSme_H
#define	_CommandDeleteSme_H

#include <xercesc/dom/DOM.hpp>
#include "smeman/smetypes.h"
//#include "admin/protocol/Command.h"
#include "SCAGCommand.h"

using smsc::smeman::SmeSystemId;
  
namespace scag {
namespace admin {

class CommandDeleteSme : public scag::admin::SCAGCommand 
{
public:
  CommandDeleteSme(const xercesc::DOMDocument * const document);

  const SmeSystemId& getSmeSystemId() const {return systemid;}
  virtual Response * CreateResponse(scag::Smsc * SmscApp);

private:
  SmeSystemId systemid;
};

}
}
#endif	/* _CommandDeleteSme_H */

