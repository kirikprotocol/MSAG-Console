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
void TSM::setCompletionListener(TsmComletionListener* _listener) { listener = _listener; }

TSM::~TSM()
{
}

void TSM::TBeginReq(uint8_t  cdlen, 
                           uint8_t* cd,        /* called party address */
                           uint8_t  cllen,
                           uint8_t* cl        /* calling party address */)
{
  smsc_log_debug(logger,"TSM::TBeginReq() is NOT IMPLEMENTED");
  return;
}
void TSM::TEndReq()
{
  smsc_log_debug(logger,"TSM::TEndReq() is NOT IMPLEMENTED");
  return;
}
void TSM::TResultLReq(uint8_t invokeId, uint8_t opcode, CompIF& arg)
{
  smsc_log_debug(logger,"TSM::TResultLReq() is NOT IMPLEMENTED");
  return;
}
void TSM::TInvokeReq(uint8_t invokeId, uint8_t opcode,  CompIF& arg) {return;}

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/