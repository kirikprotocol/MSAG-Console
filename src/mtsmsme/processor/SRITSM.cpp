static char const ident[] = "$Id$";
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/SRITSM.hpp"
#include "mtsmsme/processor/util.hpp"
#include <string>

namespace smsc{namespace mtsmsme{namespace processor{



SRITSM::SRITSM(TrId _ltrid,AC& ac,TCO* _tco):TSM(_ltrid,ac,_tco)
{
  logger = Logger::getInstance("mt.sme.mofwd");
  smsc_log_debug(logger,"tsm otid=%s create SRI",ltrid.toString().c_str());
}
SRITSM::~SRITSM()
{
  smsc_log_debug(logger,"tsm otid=%s delete SRI",ltrid.toString().c_str());
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
