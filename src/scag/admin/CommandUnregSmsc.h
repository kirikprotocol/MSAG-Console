// Class CommandUnregSme
// created by Vitaly on 06.04.05

#ifndef SCAG_CommandUnregSmsc_H
#define	SCAG_CommandUnregSmsc_H

#include "Abstract_CommandSmscInfo.h"
#include "CommandIds.h"

namespace scag {
namespace admin {

class CommandUnregSmsc : public Abstract_CommandSmscInfo 
{
public:
	CommandUnregSmsc(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmscInfo((Command::Id)CommandIds::unregSmsc, document)
  {
  }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

}
}
#endif	/* _CommandUnregSme_H */
