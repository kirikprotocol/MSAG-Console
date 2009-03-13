static char const ident[] = "$Id$";
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include <sys/time.h>
namespace smsc{namespace mtsmsme{namespace processor{

uint32_t TSM::objcount = 0;
uint32_t TSM::objcreated = 0;
uint32_t TSM::objdeleted = 0;

void TSM::getCounters(TSMSTAT& stat)
{
  stat.objcount = objcount;
  stat.objcreated = objcreated;
  stat.objdeleted = objdeleted;
}
TSM::TSM(TrId _ltrid,AC& ac,TCO* _tco):objnumber(++objcount)
{
  logger = Logger::getInstance("mt.sme.tsm");
  ltrid = _ltrid;
  appcntx = ac;
  tco = _tco;
  st = IDLE;
  listener = 0;
  ++objcreated;
  gettimeofday(&start_ts, NULL);
}
void TSM::setCompletionListener(TsmComletionListener* _listener) { listener = _listener; }

TSM::~TSM()
{
  struct timeval end_ts;
  --objcount;
  ++objdeleted;
//  gettimeofday(&end_ts,NULL);
//  smsc_log_info(logger,"tsm(ltrid=%s,objnumber=%d,lifetime=%ldms)",
//      ltrid.toString().c_str(),objnumber,
//      (end_ts.tv_usec - start_ts.tv_usec)/1000 + 1000*(end_ts.tv_sec - start_ts.tv_sec));
}
void TSM::BEGIN(uint8_t _laddrlen, uint8_t *_laddr,
                uint8_t _raddrlen, uint8_t *_raddr,
                TrId _rtrid, Message& msg)
{
  laddrlen = _laddrlen;
  memcpy(&laddr[0],_laddr,_laddrlen);
  raddrlen = _raddrlen;
  memcpy(&raddr[0],_raddr,_raddrlen);
  rtrid = _rtrid;
  BEGIN(msg);
}
void TSM::BEGIN(Message& msg)
{
  smsc_log_debug(logger,"TSM::BEGIN(Message& msg) is NOT IMPLEMENTED");
}
void TSM::TInvokeReq(int8_t invokeId, uint8_t opcode, CompIF& arg)
{
  arg.encode(internal_arg);
  internal_opcode = opcode;
  internal_invokeId = invokeId;
}
void TSM::TBeginReq(uint8_t cdlen, uint8_t* cd, /* called party address */
                    uint8_t cllen, uint8_t* cl /* calling party address */)
{
  // set SCCP adresses
  raddrlen = cdlen;
  memcpy(raddr,cd,cdlen);
  laddrlen = cllen;
  memcpy(laddr,cl,cllen);
// TO DO check component existing and if no then send empty begin
  BeginMsg begin;
  begin.setOTID(ltrid);
  begin.setDialog(appcntx);
  begin.setInvokeReq(internal_invokeId,internal_opcode,internal_arg);
  vector<unsigned char> data;
  begin.encode(data);
  tco->SCCPsend(raddrlen,raddr,laddrlen,laddr,data.size(),&data[0]);
}
void TSM::CONTINUE_received(uint8_t cdlen,
                                   uint8_t *cd, /* called party address */
                                   uint8_t cllen,
                                   uint8_t *cl, /* calling party address */
                                   Message& msg)
{
  smsc_log_warn(logger,"TSM::CONTINUE_received() is NOT IMPLEMENTED");
}
void TSM::TEndReq()
{
  smsc_log_warn(logger,"TSM::TEndReq() is NOT IMPLEMENTED");
}
void TSM::TResultLReq(uint8_t invokeId, uint8_t opcode, CompIF& arg)
{
  smsc_log_warn(logger,"TSM::TResultLReq() is NOT IMPLEMENTED");
}
void TSM::END_received(Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive END, close dialogue",ltrid.toString().c_str());
  tco->TSMStopped(ltrid);
}

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
