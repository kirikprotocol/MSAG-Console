// 
// File:   Abstract_CommandSmeInfo.h
// Author: igork
//
// Created on 31 Август 2004 г., 18:07
//

#ifndef _Abstract_CommandSmeInfo_H
#define	_Abstract_CommandSmeInfo_H

#include <xercesc/dom/DOM.hpp>
#include "SCAGCommand.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"

using namespace xercesc;

namespace scag {
namespace admin {

class Abstract_CommandSmeInfo : public scag::admin::SCAGCommand
{
public:
	Abstract_CommandSmeInfo(const Command::Id id, const xercesc::DOMDocument * const document);
	virtual ~Abstract_CommandSmeInfo();
  
  const scag::transport::smpp::SmppEntityInfo & getSmppEntityInfo() const {return smppEntityInfo;};
  
protected:
  scag::transport::smpp::SmppEntityInfo smppEntityInfo;
};

}
}

#endif	/* _Abstract_CommandSmeInfo_H */

