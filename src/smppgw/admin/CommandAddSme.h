// 
// File:   CommandAddSme.h
// Author: igork
//
// Created on 31 ������ 2004 �., 17:54
//

#ifndef _CommandAddSme_H
#define	_CommandAddSme_H

#include "Abstract_CommandSmeInfo.h"
#include "CommandIds.h"

namespace smsc {
namespace smppgw {
namespace admin {

class CommandAddSme : public Abstract_CommandSmeInfo 
{
public:
  CommandAddSme(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmeInfo((Command::Id)CommandIds::addSme, document)
  {
  }
  virtual Response * CreateResponse(smsc::smppgw::Smsc * SmscApp);

};

}
}
}
#endif	/* _CommandAddSme_H */

