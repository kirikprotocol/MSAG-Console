static char const ident[] = "$Id$";
#include "TCO.hpp"
#include "PRNTSM.hpp"
#include "util.hpp"
#include "mtsmsme/comp/ProvideRoamingNumber.hpp"
#include <string>
#include <asn_application.h>

namespace smsc{namespace mtsmsme{namespace processor{


using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::util::dump;
using smsc::mtsmsme::comp::ProvideRoamingNumberResp;
using smsc::mtsmsme::comp::ProvideRoamingNumberRespV1;
using std::string;

PRNTSM::PRNTSM(TrId _ltrid,AC& ac,TCO* _tco):TSM(_ltrid,ac,_tco)
{
  logger = Logger::getInstance("mt.sme.prn");
  smsc_log_debug(logger,"tsm otid=%s create Provide Roaming Number",ltrid.toString().c_str());
}

PRNTSM::~PRNTSM()
{
  smsc_log_debug(logger,"tsm otid=%s delete Provide Roaming Number",ltrid.toString().c_str());
}
void PRNTSM::BEGIN_received(uint8_t _laddrlen, uint8_t *_laddr, uint8_t _raddrlen, uint8_t *_raddr, TrId _rtrid,Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive BEGIN with component, send END with Missed Calls number as Roaming Number",ltrid.toString().c_str());
  laddrlen = _laddrlen;
  memcpy(&laddr[0],_laddr,_laddrlen);
  raddrlen = _raddrlen;
  memcpy(&raddr[0],_raddr,_raddrlen);
  rtrid = _rtrid;


  int iid = 1;
  if(msg.isComponentPresent()) iid = msg.getInvokeId();

  ProvideRoamingNumberResp resp;
  ProvideRoamingNumberRespV1 respv1;
  resp.setRoamingNumber("79139860932");
  respv1.setRoamingNumber("79139860932");
  if(appcntx == roamingNumberEnquiryContext_v1)
  {
    TResultLReq( iid /* invokeId */, 4 /* ProvideRoamingNumber operation */, respv1);
    smsc_log_debug(logger,"tsm otid=%s receive BEGIN with component MAP V1, send END with Missed Calls number as Roaming Number",ltrid.toString().c_str());
  }
  else
  {
    TResultLReq( iid /* invokeId */, 4 /* ProvideRoamingNumber operation */, resp);
  }
  TEndReq();
  tco->TSMStopped(ltrid);
}
void PRNTSM::TResultLReq(uint8_t invokeId, uint8_t opcode, CompIF& arg)
{
  arg.encode(temp_arg);
  temp_opcode = opcode;
  temp_invokeId = invokeId;
}
void PRNTSM::CONTINUE_received(uint8_t cdlen,
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
                   "tsm.PRNTSM otid=%s receive CONTINUE with component, INVOKE_RES sent",
                   ltrid.toString().c_str());
  }
}
void PRNTSM::END_received(Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive END, close dialogue",ltrid.toString().c_str());
  if(listener) listener->complete(1);
  tco->TSMStopped(ltrid);
}
void PRNTSM::TInvokeReq(int8_t invokeId, uint8_t opcode, CompIF& arg)
{
  arg.encode(temp_arg);
  temp_opcode = opcode;
}
void PRNTSM::TEndReq()
{
  //TO DO insert checking of empty components
  // TO DO insert checking of component type RESULT_L and ERROR
  EndMsg end;
  end.setTrId(rtrid);
  end.setDialog(appcntx);
  end.setReturnResultL(temp_invokeId, temp_opcode, temp_arg);
  vector<unsigned char> data;
  end.encode(data);

  smsc_log_debug(logger,
                   "tsm.prn otid=%s receive BEGIN, END sent",
                   ltrid.toString().c_str());
  smsc_log_error(logger,
                   "CALLED[%d]={%s}",
                   raddrlen,dump(raddrlen,raddr).c_str());
  smsc_log_error(logger,
                   "CALLING[%d]={%s}",
                   laddrlen,dump(laddrlen,laddr).c_str());
  smsc_log_error(logger,
                 "PRN[%d]={%s}",
                 data.size(),dump(data.size(),&data[0]).c_str());
  tco->SCCPsend(raddrlen,raddr,laddrlen,laddr,data.size(),&data[0]);
}
void PRNTSM::TBeginReq(uint8_t  cdlen, uint8_t* cd, uint8_t  cllen, uint8_t* cl)
{
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
