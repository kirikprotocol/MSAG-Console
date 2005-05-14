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
#include "smeman/smeinfo.h"

using namespace xercesc;

namespace smsc {
namespace scag {
namespace admin {

class Abstract_CommandSmeInfo : public smsc::scag::admin::SCAGCommand
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

