// Class CommandStatusSme
// created by Vitaly on 23.03.05

#ifndef _CommandStatusSme_H
#define	_CommandStatusSme_H

#include "Abstract_CommandSmeInfo.h"
#include "CommandIds.h"

namespace smsc {
namespace smppgw {
namespace admin {

class CommandStatusSme : public Abstract_CommandSmeInfo 
{
public:
	CommandStatusSme(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmeInfo((Command::Id)CommandIds::statusSme, document)
  {
  }
    virtual Response * CreateResponse(smsc::smppgw::Smsc * SmscApp);
};

}
}
}
#endif	/* _CommandStatusSme_H */
