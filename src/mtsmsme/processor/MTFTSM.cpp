static char const ident[] = "$Id$";
#include "util.hpp"
#include "MTFTSM.hpp"
#include "TCO.hpp"
//#include "mtsmsme/processor/Message.hpp"
#include "MtForward.hpp"
#include "MTRequest.hpp"
#include "logger/Logger.h"
#include <stdexcept>

namespace smsc{namespace mtsmsme{namespace processor{

using namespace smsc::mtsmsme::processor::util;
using namespace std;

extern void makeSmsToRequest(MtForward* mtf,MTR* req);

MTFTSM::MTFTSM(TrId _ltrid,AC& ac,TCO* _tco):TSM(_ltrid,ac,_tco),req(this)
{
  logger = Logger::getInstance("mt.sme.mttsm");
  smsc_log_debug(logger,"tsm otid=%s create",ltrid.toString().c_str());
}
MTFTSM::~MTFTSM()
{
  smsc_log_debug(logger,"tsm otid=%s delete",ltrid.toString().c_str());
}

/*
 * Если на входе BEGIN без компонента то CONTINUE
 * Если на входе BEGIN с компонентом но без MMS то END
 * Если на входе BEGIN
 */
void MTFTSM::sendResponse(int result,int iid)
{
  if (st==ACTIVE)
  {
    if (req.mms)
    {
      if (result==0)
      {
        ContMsg msg;
        msg.setOTID(ltrid);
        msg.setDTID(rtrid);
        msg.setComponent(result, iid);
        std::vector<unsigned char> rsp;
        tco->encoder.encode_mt_resp(msg,rsp);
        tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);
      }
      else
      {
        EndMsg msg;
        msg.setTrId(rtrid);
        msg.setComponent(result, iid);
        std::vector<unsigned char> rsp;
        tco->encoder.encode_mt_resp(msg,rsp);
        tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);
        tco->TSMStopped(ltrid);
      }
    }
    else
    {
      EndMsg msg;
      msg.setTrId(rtrid);
      msg.setComponent(result, iid);
      std::vector<unsigned char> rsp;
      tco->encoder.encode_mt_resp(msg,rsp);
      tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);
      smsc_log_debug(logger,"tsm otid=%s receive RESULT, END sent",ltrid.toString().c_str());
      tco->TSMStopped(ltrid);
    }
  }
  else
  {
    EndMsg msg;
    msg.setTrId(rtrid);
    msg.setDialog(appcntx);
    msg.setComponent(result, iid);
    std::vector<unsigned char> rsp;
    tco->encoder.encode_mt_resp(msg,rsp);
    tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);
    smsc_log_debug(logger,"tsm otid=%s receive RESULT, END sent",ltrid.toString().c_str());
    tco->TSMStopped(ltrid);
  }
}

void MTFTSM::BEGIN_received(uint8_t _laddrlen, uint8_t *_laddr, uint8_t _raddrlen, uint8_t *_raddr, TrId _rtrid,Message& msg)
{
  // канал TCO->TSM, TCO вызвано из SCCP
  // Должны быть в состоянии IDLE
  // сохранить Calling address and RTID
  // создать DHA
  // вызвать DHA с примитивом TR-BEGIN
  // перейти в состояние IR
  laddrlen = _laddrlen;
  memcpy(&laddr[0],_laddr,_laddrlen);
  raddrlen = _raddrlen;
  memcpy(&raddr[0],_raddr,_raddrlen);
  rtrid = _rtrid;
  if(msg.isComponentPresent())
  {
    std::vector<unsigned char> mtbuf;
    mtbuf = msg.getComponent();
    MtForward mtf;
    tco->decoder.decodemt((void *)&mtbuf[0],mtbuf.size(),mtf);


    //req =  new MTR(this);
    req.invokeId = msg.getInvokeId();
    req.mms = mtf.isMMS();
    try {
      makeSmsToRequest(&mtf,&req);
    } catch (exception& exc) {
        smsc_log_debug(logger,
                       "tsm otid=%s receive BEGIN with MALFORMED(%s) component, mms=%d, END sent, ",
                       ltrid.toString().c_str(),exc.what(),req.mms);

        EndMsg msg;
        msg.setTrId(rtrid);
        msg.setDialog(appcntx);
        msg.setComponent(1025, req.invokeId);// any not OK result, FYI 1025 = 'no route' :)
        std::vector<unsigned char> rsp;
        tco->encoder.encode_mt_resp(msg,rsp);
        tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);
        tco->TSMStopped(ltrid);
        return;
    }
    if (tco->sender)
    {
      smsc_log_debug(logger,"tsm otid=%s receive BEGIN with component, mms=%d, INVOKE sending",ltrid.toString().c_str(),req.mms);
      tco->sender->send(&req);
    }
  }
  else
  {
    ContMsg msg;
    msg.setOTID(ltrid);
    msg.setDTID(rtrid);
    msg.setDialog(appcntx);
    std::vector<unsigned char> rsp;
    tco->encoder.encode_mt_resp(msg,rsp);
    tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);
    st = ACTIVE;
    smsc_log_debug(logger,"tsm otid=%s receive BEGIN with no component, CONTINUE sent",ltrid.toString().c_str());
  }
}
void MTFTSM::CONTINUE_received(uint8_t cdlen,
                               uint8_t *cd, /* called party address */
                               uint8_t cllen,
                               uint8_t *cl, /* calling party address */
                               Message& msg)
{
  if(msg.isComponentPresent())
  {
    std::vector<unsigned char> mtbuf;
    mtbuf = msg.getComponent();
    MtForward mtf;
    tco->decoder.decodemt((void *)&mtbuf[0],mtbuf.size(),mtf);


    //req =  new MTR(this);
    req.invokeId = msg.getInvokeId();
    req.mms = mtf.isMMS();
    try {
      makeSmsToRequest(&mtf,&req);
    } catch (exception& exc) {
        smsc_log_debug(logger,
                       "tsm otid=%s receive CONTINUE with MALFORMED(%s) component, mms=%d, END sent, ",
                       ltrid.toString().c_str(),exc.what(),req.mms);

        EndMsg msg;
        msg.setTrId(rtrid);
        msg.setDialog(appcntx);
        msg.setComponent(1025, req.invokeId);// any not OK result, FYI 1025 = 'no route' :)
        std::vector<unsigned char> rsp;
        tco->encoder.encode_mt_resp(msg,rsp);
        tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);
        tco->TSMStopped(ltrid);
        return;
    }
    if (tco->sender)
    {
      smsc_log_debug(logger,"tsm otid=%s receive CONTINUE with component, mms=%d, INVOKE sent",ltrid.toString().c_str(),req.mms);
      tco->sender->send(&req);
    }
  }
}
void MTFTSM::END_received(Message& msg) { /* there is no action */ }

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/