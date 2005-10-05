// Class CommandModifySmsc
// created by Vitaly on 19.04.05

#ifndef SCAG_CommandModifySmsc_H
#define	SCAG_CommandModifySmsc_H

#include "Abstract_CommandSmscInfo.h"
#include "CommandIds.h"
#include "scag/scag.h"

namespace scag {
namespace admin {

class CommandModifySmsc : public Abstract_CommandSmscInfo 
{
public:
	CommandModifySmsc(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmscInfo((Command::Id)CommandIds::modifySmsc, document)
  {
  }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

}
}
#endif	/* _CommandModifySmsc_H */
