// 
// File:   CommandUpdateSmeInfo.h
// Author: igork
//
// Created on 20 Август 2004 г., 15:42
//

#ifndef _CommandUpdateSmeInfo_H
#define	_CommandUpdateSmeInfo_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"
#include "smeman/smeinfo.h"

namespace smsc {
namespace smppgw {
namespace admin {

class CommandUpdateSmeInfo : public smsc::admin::protocol::Command
{
public:
	CommandUpdateSmeInfo(const xercesc::DOMDocument * document);
	virtual ~CommandUpdateSmeInfo();
  
  const smsc::smeman::SmeInfo & getSmeInfo() const {return smeInfo;};
  
private:
     smsc::smeman::SmeInfo smeInfo;
};

}
}
}
#endif	/* _CommandUpdateSmeInfo_H */

