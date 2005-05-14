// Class CommandRegSmsc
// created by Vitaly on 04.04.05

#ifndef _CommandRegSmsc_H
#define	_CommandRegSmsc_H

#include "Abstract_CommandSmscInfo.h"
#include "CommandIds.h"

namespace smsc {
namespace scag {
namespace admin {

class CommandRegSmsc : public Abstract_CommandSmscInfo 
{
public:
	CommandRegSmsc(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmscInfo((Command::Id)CommandIds::regSmsc, document)
  {
  }
    virtual Response * CreateResponse(smsc::scag::Smsc * SmscApp);
};

}
}
}
#endif	/* _CommandRegSmsc_H */
