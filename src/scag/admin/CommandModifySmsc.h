// Class CommandModifySmsc
// created by Vitaly on 19.04.05

#ifndef _CommandModifySmsc_H
#define	_CommandModifySmsc_H

#include "Abstract_CommandSmscInfo.h"
#include "CommandIds.h"

namespace smsc {
namespace scag {
namespace admin {

class CommandModifySmsc : public Abstract_CommandSmscInfo 
{
public:
	CommandModifySmsc(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmscInfo((Command::Id)CommandIds::modifySmsc, document)
  {
  }
    virtual Response * CreateResponse(smsc::scag::Smsc * SmscApp);
};

}
}
}
#endif	/* _CommandModifySmsc_H */
