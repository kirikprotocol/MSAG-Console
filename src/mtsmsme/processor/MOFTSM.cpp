static char const ident[] = "$Id$";
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/MOFTSM.hpp"
#include "mtsmsme/processor/util.hpp"
#include <string>

namespace smsc{namespace mtsmsme{namespace processor{



MOFTSM::MOFTSM(TrId _ltrid,AC& ac,TCO* _tco):TSM(_ltrid,ac,_tco)
{
  logger = Logger::getInstance("mt.sme.mofwd");
  smsc_log_debug(logger,"tsm otid=%s create MoForwardSm",ltrid.toString().c_str());
}
MOFTSM::~MOFTSM()
{
  smsc_log_debug(logger,"tsm otid=%s delete MoForwardSm",ltrid.toString().c_str());
}

void MOFTSM::BEGIN(Message& msg)
{
  int iid;
  if(msg.isComponentPresent()) iid = msg.getInvokeId();

  EndMsg end;
  end.setTrId(rtrid);
  end.setDialog(appcntx);
  end.setComponent(0, iid);
  std::vector<unsigned char> rsp;
  tco->encoder.encode_resp(end,rsp);
  tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,(uint16_t)rsp.size(),&rsp[0]);
  smsc_log_debug(logger,"tsm.isd otid=%s receive BEGIN, END sent",ltrid.toString().c_str());
  tco->TSMStopped(ltrid);
}
void MOFTSM::CONTINUE_received(uint8_t cdlen,
                              uint8_t *cd, /* called party address */
                              uint8_t cllen,
                              uint8_t *cl, /* calling party address */
                              Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive CONTINUE, no actions",ltrid.toString().c_str());
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
