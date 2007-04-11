static char const ident[] = "$Id$";
#include "TSM.hpp"
namespace smsc{namespace mtsmsme{namespace processor{

TSM::TSM(TrId _ltrid,AC& ac,TCO* _tco)
{
  logger = Logger::getInstance("mt.sme.tsm");
  ltrid = _ltrid;
  appcntx = ac;
  tco = _tco;
  st = IDLE;
  listener = 0;
}
TSM::~TSM()
{
}

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/