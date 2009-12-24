static char const ident[] = "$Id$";
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/SRITSM.hpp"
#include "mtsmsme/comp/SendRoutingInfo.hpp"
#include "mtsmsme/processor/util.hpp"
#include <string>

namespace smsc{namespace mtsmsme{namespace processor{
using smsc::mtsmsme::comp::SendRoutingInfoConf;
using smsc::mtsmsme::comp::SendRoutingInfoConfV2;


SRITSM::SRITSM(TrId _ltrid,AC& ac,TCO* _tco):TSM(_ltrid,ac,_tco)
{
  logger = Logger::getInstance("mt.sme.sri");
  smsc_log_debug(logger,"tsm otid=%s create SRI",ltrid.toString().c_str());
}
SRITSM::~SRITSM()
{
  smsc_log_debug(logger,"tsm otid=%s delete SRI",ltrid.toString().c_str());
}
void SRITSM::END_received(Message& msg)
{
  if (msg.isComponentPresent() && (msg.getOperationCode() == 22))
  {
    smsc_log_debug(logger,"tsm otid=%s receive END with SRI RES, closing",ltrid.toString().c_str());
    std::vector<unsigned char> sriconfbuf;
    sriconfbuf = msg.getComponent();
    if (appcntx == locationInfoRetrievalContext_v2)
    {
      SendRoutingInfoConfV2 sriconf(logger);
      sriconf.decode(sriconfbuf);
    }
    else
    {
      SendRoutingInfoConf sriconf(logger);
      sriconf.decode(sriconfbuf);
    }
  }
  TSM::END_received(msg);
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
