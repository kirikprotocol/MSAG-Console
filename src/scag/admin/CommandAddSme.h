// 
// File:   CommandAddSme.h
// Author: igork
//
// Created on 31 Август 2004 г., 17:54
//

#ifndef _CommandAddSme_H
#define	_CommandAddSme_H

#include "Abstract_CommandSmeInfo.h"
#include "CommandIds.h"

namespace scag {
namespace admin {

class CommandAddSme : public Abstract_CommandSmeInfo 
{
public:
  CommandAddSme(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmeInfo((Command::Id)CommandIds::addSme, document)
  {
  }
  virtual Response * CreateResponse(scag::Scag * SmscApp);

};

}
}
#endif	/* _CommandAddSme_H */

