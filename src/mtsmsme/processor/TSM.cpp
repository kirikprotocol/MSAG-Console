static char const ident[] = "$Id$";
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "util/Exception.hpp"
#include <sys/time.h>
namespace smsc{namespace mtsmsme{namespace processor{

uint32_t TSM::objcount = 0;
uint32_t TSM::objcreated = 0;
uint32_t TSM::objdeleted = 0;
static uint32_t secretcount = 1971;

void TSM::getCounters(TSMSTAT& stat)
{
  stat.objcount = objcount;
  stat.objcreated = objcreated;
  stat.objdeleted = objdeleted;
}
TSM::TSM(TrId _ltrid,AC& ac,TCO* _tco):tco(_tco),appcntx(ac),listener(0),st(IDLE)
{
  if ( tco->tridpool.empty() ) throw smsc::util::Exception("transaction pool is empty");
  ltrid = tco->tridpool.front();
  tco->tridpool.pop_front();

  logger = Logger::getInstance("mt.sme.tsm");
  objnumber = ++objcount;
  ++objcreated;
  secret = ++secretcount;
  gettimeofday(&start_ts, NULL);
}
TrId TSM::getltrid() { return ltrid; }
void TSM::setCompletionListener(TsmComletionListener* _listener) { listener = _listener; }
TSM::~TSM()
{
  struct timeval end_ts;
  --objcount;
  ++objdeleted;
  tco->tridpool.push_back(ltrid);
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
  BeginMsg begin;
  begin.setOTID(ltrid);
  begin.setDialog(appcntx);
  //TODO implement list of components
  //send empty begin if no components
  if ( !internal_arg.empty())
    begin.setInvokeReq(internal_invokeId,internal_opcode,internal_arg);
  vector<unsigned char> data;
  begin.encode(data);
  tco->SCCPsend(raddrlen,raddr,laddrlen,laddr,(uint16_t)data.size(),&data[0]);
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
void TSM::TContReq()
{
  // TODO make sure SCCP adresses were set
  ContMsg cont;
  cont.setOTID(ltrid);
  cont.setDTID(rtrid);
    if ( !internal_arg.empty())
      cont.setInvokeReq(internal_invokeId,internal_opcode,internal_arg);
    vector<unsigned char> data;
    cont.encode(data);
    tco->SCCPsend(raddrlen,raddr,laddrlen,laddr,(uint16_t)data.size(),&data[0]);
}
void TSM::TResultLReq(uint8_t invokeId, uint8_t opcode, CompIF& arg)
{
  smsc_log_warn(logger,"TSM::TResultLReq() is NOT IMPLEMENTED");
}
void TSM::startwdtimer(int seconds) {tco->startwdtimer(seconds,ltrid,secret);}
void TSM::expiredwdtimer(uint32_t _secret)
{
  if (_secret == secret)
  {
    smsc_log_debug(logger,"tsm otid=%s expire watch dog timer, closing...",ltrid.toString().c_str());
    tco->TSMStopped(ltrid);
  }
  else
  {
    smsc_log_error(logger,"tsm otid=%s expire UNKNOWN timer, skipping...",ltrid.toString().c_str());
  }
}
void TSM::END_received(Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive END, close dialogue",ltrid.toString().c_str());
  tco->TSMStopped(ltrid);
}
void TSM::ABORT_received(Message& msg)
{
  smsc_log_debug(logger,"tsm otid=%s receive ABORT, close dialogue",ltrid.toString().c_str());
  tco->TSMStopped(ltrid);
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
