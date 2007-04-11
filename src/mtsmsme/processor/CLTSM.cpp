static char const ident[] = "$Id$";
#include "TCO.hpp"
#include "CLTSM.hpp"
#include "util.hpp"
#include <string>

namespace smsc{namespace mtsmsme{namespace processor{



CLTSM::CLTSM(TrId _ltrid,AC& ac,TCO* _tco):TSM(_ltrid,ac,_tco)
{
  logger = Logger::getInstance("mt.sme.cltsm");
  smsc_log_debug(logger,"tsm otid=%s create CancelLocation",ltrid.toString().c_str());
}
CLTSM::~CLTSM()
{
  smsc_log_debug(logger,"tsm otid=%s delete CancelLocation",ltrid.toString().c_str());
}

void CLTSM::BEGIN_received(uint8_t _laddrlen, uint8_t *_laddr, uint8_t _raddrlen, uint8_t *_raddr, TrId _rtrid,Message& msg)
{
  laddrlen = _laddrlen;
  memcpy(&laddr[0],_laddr,_laddrlen);
  raddrlen = _raddrlen;
  memcpy(&raddr[0],_raddr,_raddrlen);
  rtrid = _rtrid;

  int iid;
  if(msg.isComponentPresent()) iid = msg.getInvokeId();

  EndMsg end;
  end.setTrId(rtrid);
  end.setDialog(appcntx);
  end.setComponent(0, iid);
  std::vector<unsigned char> rsp;
  tco->encoder.encode_resp(end,rsp);
  tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);
  smsc_log_debug(logger,"tsm.cltsm otid=%s receive BEGIN, END sent",ltrid.toString().c_str());
  tco->TSMStopped(ltrid);
}
void CLTSM::CONTINUE_received(uint8_t cdlen,
                              uint8_t *cd, /* called party address */
                              uint8_t cllen,
                              uint8_t *cl, /* calling party address */
                              Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive CONTINUE, no actions",ltrid.toString().c_str());
}

void CLTSM::BeginTransaction(TsmComletionListener* _listener)
{
  smsc_log_debug(logger,"tsm otid=%s receive BeginTransaction, no actions",ltrid.toString().c_str());
}
void CLTSM::END_received(Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive END, no action",ltrid.toString().c_str());
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/