// 
// File:   Abstract_CommandSmeInfo.h
// Author: igork
//
// Created on 31 Август 2004 г., 18:07
//

#ifndef _Abstract_CommandSmeInfo_H
#define	_Abstract_CommandSmeInfo_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"
#include "smeman/smeinfo.h"
#include "SmppGwCommand.h"

using namespace xercesc;

namespace smsc {
namespace smppgw {
namespace admin {

class Abstract_CommandSmeInfo : public smsc::smppgw::admin::SmppGwCommand
{
public:
	Abstract_CommandSmeInfo(const Command::Id id, const xercesc::DOMDocument * const document);
	virtual ~Abstract_CommandSmeInfo();
  
  const smsc::smeman::SmeInfo & getSmeInfo() const {return smeInfo;};
  
protected:
  smsc::smeman::SmeInfo smeInfo;
};

}
}
}

#endif	/* _Abstract_CommandSmeInfo_H */

