// 
// File:   Abstract_CommandSmscInfo.h
// Author: Vitaly
//
// Created on 04.04.05
//

#ifndef _Abstract_CommandSmscInfo_H
#define	_Abstract_CommandSmscInfo_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"
#include "sme/SmppBase.hpp"
#include "smppgw/admin/SmppGwCommand.h"
#include <string>

using namespace xercesc;

namespace smsc {
namespace smppgw {
namespace admin {

class Abstract_CommandSmscInfo : public smsc::smppgw::admin::SmppGwCommand
{
public:
	Abstract_CommandSmscInfo(const Command::Id id, const xercesc::DOMDocument * const document);
	virtual ~Abstract_CommandSmscInfo();
  
  //const smsc::sme::SmeConfig & getSmscInfo() const {return smscConfig;};
  
protected:
  smsc::sme::SmeConfig smscConfig;
  std::string altHost;
  std::string systemId;
  uint8_t altPort;
  uint8_t uid;
};

}
}
}

#endif	/* _Abstract_CommandSmscInfo_H */

