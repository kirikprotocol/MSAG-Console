// Class CommandUnregSme
// created by Vitaly on 06.04.05

#ifndef _CommandUnregSme_H
#define	_CommandUnregSme_H

#include "Abstract_CommandSmscInfo.h"
#include "CommandIds.h"

namespace scag {
namespace admin {

class CommandUnregSme : public Abstract_CommandSmscInfo 
{
public:
	CommandUnregSme(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmscInfo((Command::Id)CommandIds::unregSme, document)
  {
  }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

}
}
#endif	/* _CommandUnregSme_H */
