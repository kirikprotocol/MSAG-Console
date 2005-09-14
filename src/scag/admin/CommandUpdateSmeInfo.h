// 
// File:   CommandUpdateSmeInfo.h
// Author: igork
//
// Created on 20 Август 2004 г., 15:42
//

#ifndef _CommandUpdateSmeInfo_H
#define	_CommandUpdateSmeInfo_H

#include "Abstract_CommandSmeInfo.h"

namespace scag {
namespace admin {

class CommandUpdateSmeInfo : public Abstract_CommandSmeInfo
{
public:
	CommandUpdateSmeInfo(const xercesc::DOMDocument * const document);
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

}
}
#endif	/* _CommandUpdateSmeInfo_H */

