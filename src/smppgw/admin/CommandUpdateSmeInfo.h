// 
// File:   CommandUpdateSmeInfo.h
// Author: igork
//
// Created on 20 ������ 2004 �., 15:42
//

#ifndef _CommandUpdateSmeInfo_H
#define	_CommandUpdateSmeInfo_H

#include "Abstract_CommandSmeInfo.h"

namespace smsc {
namespace smppgw {
namespace admin {

class CommandUpdateSmeInfo : public Abstract_CommandSmeInfo
{
public:
	CommandUpdateSmeInfo(const xercesc::DOMDocument * const document);
    virtual Response * CreateResponse(smsc::smppgw::Smsc * SmscApp);
};

}
}
}
#endif	/* _CommandUpdateSmeInfo_H */

