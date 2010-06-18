static char const ident[] = "$Id$";
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/util.hpp"
#include "mtsmsme/processor/ACRepo.hpp"
#include "util/Exception.hpp"
#include "logger/Logger.h"
#include <queue>

namespace smsc{namespace mtsmsme{namespace processor{

using namespace smsc::mtsmsme::processor::util;
using smsc::core::synchronization::MutexGuard;
using smsc::util::Exception;

static int reqnum = 0;
using smsc::logger::Logger;
static Logger *logger = 0;

TCO::TCO(int TrLimit):hlr(0),tsms(TrLimit),sccpsender(0)
{
  logger = Logger::getInstance("mt.sme.tco");
  TrId id; id.size=4; id.buf[0] = 0xBA; id.buf[1] = 0xBE;
  for (unsigned n=1;n<=TrLimit;++n){
    id.buf[2] = (n >> 8) & 0xFF;
    id.buf[3] = n & 0xFF;
    tridpool.push_back(id);
  }
}
TCO::~TCO()
{
  using smsc::core::buffers::XHash;
  typedef XHash<TrId,TSM*,TrIdHash> TsmList;
  TrId k;
  TSM* v;
  for (TsmList::Iterator i(&tsms); i.Next(k, v);)
  {
    smsc_log_debug(logger,"clean TSM(%s)",k.toString().c_str());
    tsms.Delete(k);
    delete(v);
  }
}
TSM* TCO::TC_BEGIN(AC& appcntx)
{
  TSM* tsm = 0;
  // ltrid is not using in TSM constructor, so set fake value for compiler
  TrId ltrid; ltrid.size=4;
  try {
    tsm = createOutgoingTSM(ltrid,appcntx,this);
  } catch (Exception exc ) {
        smsc_log_error(logger,
                       "tco TC-BEGIN and %s, do nothing",exc.what());
        return 0;
  }
  if (tsm)
  {
    MutexGuard g(tridpool_mutex);
    tsms.Insert(tsm->getltrid(),tsm);
  }
  return tsm;
}
void TCO::setAdresses(Address& msc, Address& vlr, Address& _hlr)
{
  msc.getValue(mscnumber);
  vlr.getValue(vlrnumber);
  _hlr.getValue(hlrnumber);
}
void TCO::fixCalledAddress(uint8_t cdlen, uint8_t* cd)
{
  /* fix ssn for configured numbers */
  if (modifyssn(cd, cdlen, mscnumber, 8))
  {
    smsc_log_debug(logger,
                  "Cd equals to %s, set SSN = 8, new Cd(%s)",
                  mscnumber,getAddressDescription(cdlen,cd).c_str());
  }
  else if (modifyssn(cd, cdlen, vlrnumber, 7))
  {
    smsc_log_debug(logger,
                  "Cd GT equals to %s, set SSN = 7, new Cd(%s)",
                  vlrnumber,getAddressDescription(cdlen,cd).c_str());
  }
  else if (modifyssn(cd, cdlen, hlrnumber, 6, true /* force SSN=6 */))
  {
    smsc_log_debug(logger,
                  "Cd GT not equals to VLR or MSC, assuming SSN = 6, new Cd(%s)",
                  getAddressDescription(cdlen,cd).c_str());
  }
}
void TCO::NNOTICE(uint8_t cdlen, uint8_t *cd, /* called party address  */
                  uint8_t cllen, uint8_t *cl, /* calling party address */
                  uint16_t ulen, uint8_t *udp /* user data             */
                  )
{
  smsc_log_debug(logger,
    "TCO::N-NOTICE-IND "
    "Cd[%d]={%s} "
    "Cg[%d]={%s} "
    "Ud[%d]={%s} "
    "Cd(%s) "
    "Cg(%s)",
    cdlen,dump(cdlen,cd).c_str(),
    cllen,dump(cllen,cl).c_str(),
    ulen, dump(ulen,udp).c_str(),
    getAddressDescription(cdlen,cd).c_str(),
    getAddressDescription(cllen,cl).c_str()
  );
}
void TCO::NUNITDATA(uint8_t cdlen, uint8_t *cd, /* called party address  */
                    uint8_t cllen, uint8_t *cl, /* calling party address */
                    uint16_t ulen, uint8_t *udp /* user data             */
                   )
{
  {
    smsc_log_debug(logger,
      "TCO::N-NUNITDATA-IND "
      "Cd[%d]={%s} "
      "Cg[%d]={%s} "
      "Ud[%d]={%s} "
      "Cd(%s) "
      "Cg(%s) "
      "tsms.size=%d",
      cdlen,dump(cdlen,cd).c_str(),
      cllen,dump(cllen,cl).c_str(),
      ulen, dump(ulen,udp).c_str(),
      getAddressDescription(cdlen,cd).c_str(),
      getAddressDescription(cllen,cl).c_str(),
      tsms.Count()
    );
  }
  fixCalledAddress(cdlen, cd);
  Message msg(logger);
  msg.decode(udp, ulen);
  if (msg.isBegin())
  {
    TrId rtrid;
    AC appcntx;
    rtrid = msg.getOTID();
    msg.getAppContext(appcntx);

    //      if (!msg.isDialoguePortionExist() && appcntx != sm_mt_relay_v1)
    if (!msg.isDialoguePortionExist() && !isMapV1ContextSupported(appcntx))
    {
      smsc_log_error(logger,"tco receive BEGIN with no dialogue portion, send ABORT(badlyFormattedTransactionPortion)");
      std::vector<unsigned char> rsp;
      encoder.encodeBadTrPortion(rtrid, rsp);
      SCCPsend(cllen, cl, cdlen, cd, (uint16_t)rsp.size(), &rsp[0]);
      return;
    }

    if (msg.isDialoguePortionExist() && !isIncomingContextSupported(appcntx))
    {
      smsc_log_error(logger,"tco receive BEGIN with unsupported app context, send ABORT(application-context-name-not-supported)");
      std::vector<unsigned char> rsp;
      encoder.encodeACNotSupported(rtrid, appcntx, rsp);
      SCCPsend(cllen, cl, cdlen, cd, (uint16_t)rsp.size(), &rsp[0]);
      return;
    }
    TSM* tsm = 0; TrId ltrid;
    try {
      tsm = createIncomingTSM(ltrid,appcntx,this);
    }
    catch (Exception exc)
    {
      smsc_log_error(logger,
                     "tco BEGIN and , "
                     "send ABORT(resourceLimitation)",exc.what());
      std::vector<unsigned char> rsp;
      encoder.encodeResourceLimitation(rtrid, rsp);
      SCCPsend(cllen, cl, cdlen, cd, (uint16_t)rsp.size(), &rsp[0]);
      return;
    }
    if (tsm)
    {
      {
        MutexGuard g(tridpool_mutex);
        tsms.Insert(tsm->getltrid(), tsm);
      }
      tsm->BEGIN(cdlen, cd, cllen, cl, rtrid, msg);
      return;
    }
    //else
    {
      smsc_log_debug(logger,
          "tco BEGIN and can't create transaction for app context"
          ", send ABORT(resourceLimitation)");
      std::vector<unsigned char> rsp;
      encoder.encodeResourceLimitation(rtrid, rsp);
      SCCPsend(cllen, cl, cdlen, cd, (uint16_t)rsp.size(), &rsp[0]);
      return;
    }
    return;
  } /* end of BEGIN handling section */
  if (msg.isContinue())
  {
    TrId rtrid;
    TrId ltrid;
    ltrid = msg.getDTID();
    rtrid = msg.getOTID();
    TSM* tsm = 0;
    {
      MutexGuard g(tridpool_mutex);
      TSM** ptr = tsms.GetPtr(ltrid);
      if (ptr)
      {
        tsm = *ptr;
      }
    }
    if (tsm)
    {
      tsm->CONTINUE_received(cdlen, cd, cllen, cl, msg);
    } else
    {
      smsc_log_error(logger,"TCO receive CONTINUE but TSM not found, DISCARD, but need to send ABORT for quick transaction release");
      std::vector<unsigned char> rsp;
      encoder.encodeResourceLimitation(rtrid, rsp);
      SCCPsend(cllen, cl, cdlen, cd, (uint16_t)rsp.size(), &rsp[0]);
    }
    return;
  }/* end of CONTINUE handling section */
  if (msg.isEnd())
  {
    TrId rtrid;
    TrId ltrid;
    ltrid = msg.getDTID();
    rtrid = msg.getOTID();
    TSM* tsm = 0;
    {
      MutexGuard g(tridpool_mutex);
      TSM** ptr = tsms.GetPtr(ltrid);
      if(ptr)
      {
        tsm=*ptr;
      }
    }
    if (tsm)
    {
      tsm->END_received(msg);
    } else
    {
      smsc_log_debug(logger,"TCO receive END but TSM not found, DISCARD");
    }
    return;
  }/* end of END handling section */
  if (msg.isAbort())
  {
    TrId rtrid;
    TrId ltrid;
    ltrid = msg.getDTID();
    rtrid = msg.getOTID();
    TSM* tsm = 0;
    {
      MutexGuard g(tridpool_mutex);
      TSM** ptr = tsms.GetPtr(ltrid);
      if(ptr)
      {
        tsm=*ptr;
      }
    }
    if (tsm)
    {
      tsm->ABORT_received(msg);
    } else
    {
      smsc_log_debug(logger,"TCO receive ABORT but TSM not found, DISCARD");
    }
    return;
  }/* end of Abort handling section */
  smsc_log_error(logger,"TCO receive UNSUPPORTED TCAP message, DISCARD");
}
void TCO::TSMStopped(TrId ltrid)
{
  TSM* tsm = 0;
  {
    MutexGuard g(tridpool_mutex);
    TSM** ptr = tsms.GetPtr(ltrid);
    if (ptr)
    {
      tsm = *ptr;
      tsms.Delete(ltrid);
    }
  }
  if (tsm) delete(tsm);
}
void TCO::setHLROAM(HLROAM* _hlr) { hlr = _hlr; }
HLROAM* TCO::getHLROAM() { return hlr; }
void TCO::setRequestSender(RequestSender* _sender) { sender = _sender; }
void TCO::setSccpSender(SccpSender* sndr) { sccpsender = sndr; }
void TCO::SCCPsend(uint8_t cdlen,uint8_t *cd,
                   uint8_t cllen,uint8_t *cl,
                   uint16_t ulen,uint8_t *udp)
{
  if (sccpsender)
  {
    sccpsender->send(cdlen,cd,cllen,cl,ulen,udp);
    smsc_log_debug(logger,
      "TCO::N-NUNITDATA-REQ "
      "Cd[%d]={%s} "
      "Cg[%d]={%s} "
      "Ud[%d]={%s} "
      "Cd(%s) "
      "Cg(%s) "
      "tsms.size=%d",
      cdlen,dump(cdlen,cd).c_str(),
      cllen,dump(cllen,cl).c_str(),
      ulen, dump(ulen,udp).c_str(),
      getAddressDescription(cdlen,cd).c_str(),
      getAddressDescription(cllen,cl).c_str(),
      tsms.Count()
    );
  }
}

class wdtimer {
  public:
    time_t deadline; TrId ltrid;uint32_t secret;
    wdtimer(int delay, TrId _ltrid, uint32_t _secret):
      deadline(time(0)+delay),ltrid(_ltrid),secret(_secret){}
};

class wdtimerComparator { public:
    bool operator()(wdtimer& left, wdtimer& right){
      return ((left.deadline) >= (right.deadline));}};

static std::priority_queue<wdtimer,std::vector<wdtimer>,wdtimerComparator> wdqueue;

void TCO::startwdtimer(int seconds,TrId ltrid, uint32_t secret)
{
  wdqueue.push(wdtimer(seconds,ltrid,secret));
}

void TCO::dlgcleanup()
{
  //check existing watchdogs
  time_t now; time(&now);
  while (! wdqueue.empty())
  {
    wdtimer timer = wdqueue.top();
    if ( now < timer.deadline)
      return;
    wdqueue.pop();
    TSM* tsm = 0;
    {
      MutexGuard g(tridpool_mutex);
      TSM** ptr = tsms.GetPtr(timer.ltrid);
      if (ptr) tsm = *ptr;
    }
    if (tsm)
      tsm->expiredwdtimer(timer.secret);
    else
      smsc_log_error(logger,"timer epires but tsm not found");
  }
}
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
