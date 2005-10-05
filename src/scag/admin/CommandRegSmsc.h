// Class CommandRegSmsc
// created by Vitaly on 04.04.05

#ifndef SCAG_CommandRegSmsc_H
#define	SCAG_CommandRegSmsc_H

#include "Abstract_CommandSmscInfo.h"
#include "CommandIds.h"
#include "scag/scag.h"

namespace scag {
namespace admin {

class CommandRegSmsc : public Abstract_CommandSmscInfo 
{
public:
	CommandRegSmsc(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmscInfo((Command::Id)CommandIds::regSmsc, document)
  {
  }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

}
}
#endif	/* _CommandRegSmsc_H */
