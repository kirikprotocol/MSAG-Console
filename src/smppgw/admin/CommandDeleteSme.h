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
#include "SmppGwCommand.h"

using smsc::smeman::SmeSystemId;
  
namespace smsc {
namespace smppgw {
namespace admin {

class CommandDeleteSme : public smsc::smppgw::admin::SmppGwCommand 
{
public:
  CommandDeleteSme(const xercesc::DOMDocument * const document);

  const SmeSystemId& getSmeSystemId() const {return systemid;}
  virtual Response * CreateResponse(smsc::smppgw::Smsc * SmscApp);

private:
  SmeSystemId systemid;
};

}
}
}
#endif	/* _CommandDeleteSme_H */

