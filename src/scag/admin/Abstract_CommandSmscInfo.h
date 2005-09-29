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
#include "scag/admin/SCAGCommand.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"
#include <string>

using namespace xercesc;

namespace scag {
namespace admin {

class Abstract_CommandSmscInfo : public scag::admin::SCAGCommand
{
public:
	Abstract_CommandSmscInfo(const Command::Id id, const xercesc::DOMDocument * const document);
	virtual ~Abstract_CommandSmscInfo();
  
  const scag::transport::smpp::SmppEntityInfo & getSmppEntityInfo() const {return smppEntityInfo;};
  
protected:
  scag::transport::smpp::SmppEntityInfo smppEntityInfo;
};

}
}

#endif	/* _Abstract_CommandSmscInfo_H */

