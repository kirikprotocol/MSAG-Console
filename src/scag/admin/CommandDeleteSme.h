// 
// File:   CommandDeleteSme.h
// Author: igork
//
// Created on 31 Август 2004 г., 17:55
//

#ifndef SCAG_CommandDeleteSme_H
#define	SCAG_CommandDeleteSme_H

#include <xercesc/dom/DOM.hpp>
#include "SCAGCommand.h"
#include "Abstract_CommandSmeInfo.h"
  
namespace scag {
namespace admin {

class CommandDeleteSme : public SCAGCommand
{
public:
  CommandDeleteSme(const xercesc::DOMDocument * const document);
  virtual Response * CreateResponse(scag::Scag * SmscApp);
protected:
    std::string systemId;

};

}
}
#endif	/* _CommandDeleteSme_H */

