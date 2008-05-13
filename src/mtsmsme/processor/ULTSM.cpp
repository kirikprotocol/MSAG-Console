static char const ident[] = "$Id$";
#include "TCO.hpp"
#include "ULTSM.hpp"
#include "util.hpp"
#include "mtsmsme/comp/UpdateLocation.hpp"
#include <string>

namespace smsc{namespace mtsmsme{namespace processor{


using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::util::dump;
using smsc::mtsmsme::comp::UpdateLocationMessage;
using std::string;

ULTSM::ULTSM(TrId _ltrid,AC& ac,TCO* _tco):TSM(_ltrid,ac,_tco)
{
  logger = Logger::getInstance("mt.sme.ultsm");
  smsc_log_debug(logger,"tsm otid=%s create UpdateLocation",ltrid.toString().c_str());
}

ULTSM::~ULTSM()
{
  smsc_log_debug(logger,"tsm otid=%s delete UpdateLocation",ltrid.toString().c_str());
}

void ULTSM::BEGIN_received(uint8_t _laddrlen, uint8_t *_laddr, uint8_t _raddrlen, uint8_t *_raddr, TrId _rtrid,Message& msg)
{
}
void ULTSM::CONTINUE_received(uint8_t cdlen,
                              uint8_t *cd, /* called party address */
                              uint8_t cllen,
                              uint8_t *cl, /* calling party address */
                              Message& msg)
{
  //first response may contain address specified by remote side, copy
  raddrlen = cllen;
  memcpy(&raddr[0],cl,cllen);
  rtrid = msg.getOTID();

  if(msg.isComponentPresent())
  {
    ContMsg cont;
    int iid = msg.getInvokeId();
    cont.setOTID(ltrid);
    cont.setDTID(rtrid);
    cont.setComponent(0,iid);
    std::vector<unsigned char> rsp;
    tco->encoder.encode_resp(cont,rsp);
    tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);

    smsc_log_debug(logger,
                   "tsm.ultsm otid=%s receive CONTINUE with component, INVOKE_RES sent",
                   ltrid.toString().c_str());
  }
}

void ULTSM::END_received(Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive END, close dialogue",ltrid.toString().c_str());
  if(listener) listener->complete(1);
  tco->TSMStopped(ltrid);
}
void ULTSM::TInvokeReq(uint8_t opcode, CompIF& arg)
{
  /*
   * MSC E.164 = 791398699812
   * VLR E.164 = 791398699813
   * MS  E.164 = 79134632021
   * MS  E.212 = 250013903784021
   * MS  E.214 = 791603903784021
   */
  UpdateLocationMessage& msg = static_cast<UpdateLocationMessage&>(arg);
  msg.setOTID(ltrid);
  msg.encode(ulmsg);
}
void ULTSM::TBeginReq(uint8_t  cdlen, uint8_t* cd, uint8_t  cllen, uint8_t* cl)
{
  // set SCCP adresses

  raddrlen = cdlen;
  memcpy(raddr,cd,cdlen);
  laddrlen = cllen;
  memcpy(laddr,cl,cllen);

  smsc_log_error(logger,
                 "CALLED[%d]={%s}",raddrlen,dump(raddrlen,raddr).c_str());
  smsc_log_error(logger,
                 "CALLING[%d]={%s}",laddrlen,dump(laddrlen,laddr).c_str());
  smsc_log_error(logger,
                 "UpdateLocation[%d]={%s}",ulmsg.size(),dump(ulmsg.size(),&ulmsg[0]).c_str());
  tco->SCCPsend(raddrlen,raddr,laddrlen,laddr,ulmsg.size(),&ulmsg[0]);
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/