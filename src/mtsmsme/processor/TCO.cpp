static char const ident[] = "$Id$";
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/util.hpp"
#include "mtsmsme/processor/ACRepo.hpp"
#include "util/Exception.hpp"
#include "logger/Logger.h"

namespace smsc{namespace mtsmsme{namespace processor{

using namespace smsc::mtsmsme::processor::util;
using smsc::core::synchronization::MutexGuard;
using smsc::util::Exception;

static int reqnum = 0;
using smsc::logger::Logger;
static Logger *logger = 0;
static SccpSender *sccpsender = 0;

TCO::TCO(int TrLimit):hlr(0),tsms(TrLimit)
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
  smsc_log_warn(logger,"please clean TSM in the TCO::~TCO()");
}
TSM* TCO::TC_BEGIN(AC& appcntx)
{
  TSM* tsm = 0; TrId ltrid;
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
void TCO::NUNITDATA(uint8_t cdlen, uint8_t *cd, /* called party address  */
                    uint8_t cllen, uint8_t *cl, /* calling party address */
                    uint16_t ulen, uint8_t *udp /* user data             */
                   )
{
  {
    smsc_log_debug(logger,
                    "TCO::NUNITDATA Cd(%s) Cl(%s)\ndata[%d]={%s}",
                    getAddressDescription(cdlen,cd).c_str(),
                    getAddressDescription(cllen,cl).c_str(),
                    ulen,dump(ulen,udp).c_str());
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
      SCCPsend(cllen, cl, cdlen, cd, rsp.size(), &rsp[0]);
      return;
    }

    if (msg.isDialoguePortionExist() && !isIncomingContextSupported(appcntx))
    {
      smsc_log_error(logger,"tco receive BEGIN with unsupported app context, send ABORT(application-context-name-not-supported)");
      std::vector<unsigned char> rsp;
      encoder.encodeACNotSupported(rtrid, appcntx, rsp);
      SCCPsend(cllen, cl, cdlen, cd, rsp.size(), &rsp[0]);
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
      SCCPsend(cllen, cl, cdlen, cd, rsp.size(), &rsp[0]);
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
      SCCPsend(cllen, cl, cdlen, cd, rsp.size(), &rsp[0]);
      return;
    }
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
      SCCPsend(cllen, cl, cdlen, cd, rsp.size(), &rsp[0]);
    }
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
  }/* end of END handling section */
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
    smsc_log_debug(logger,"void TCO::SCCPsend: cd=%s, cl=%s",getAddressDescription(cdlen,cd).c_str(),getAddressDescription(cllen,cl).c_str());
    sccpsender->send(cdlen,cd,cllen,cl,ulen,udp);
  }
}

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
