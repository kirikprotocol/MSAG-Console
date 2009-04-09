static char const ident[] = "$Id$";
#include "mtsmsme/processor/util.hpp"
#include "MTFTSM.hpp"
#include "TCO.hpp"
//#include "mtsmsme/processor/Message.hpp"
#include "MtForward.hpp"
#include "MTRequest.hpp"
#include "logger/Logger.h"
#include "sms/sms.h"
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
static void DumpSentSms(SMS& sms,Logger* logger)
{
  using namespace smsc::sms;
  const char* prop = 0;
  unsigned int len = 0;
  prop = sms.getBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,&len);
  char buf[20] = {0};
  memcpy(buf,prop,len); buf[len] = 0;
  smsc_log_info(logger,"sent resp %s",buf);
}
/*
 * ���� �� ����� BEGIN ��� ���������� �� CONTINUE
 * ���� �� ����� BEGIN � ����������� �� ��� MMS �� END
 * ���� �� ����� BEGIN
 */
void MTFTSM::sendResponse(int result,int iid)
{
  //simulate MAP_NO_RESPONSE_FROM_PEER=1143
  if ( result == 1143) tco->TSMStopped(ltrid);

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
        DumpSentSms(req.sms,logger);
      }
      else
      {
        EndMsg msg;
        msg.setTrId(rtrid);
        msg.setComponent(result, iid);
        std::vector<unsigned char> rsp;
        tco->encoder.encode_mt_resp(msg,rsp);
        tco->SCCPsend(raddrlen,&raddr[0],laddrlen,laddr,rsp.size(),&rsp[0]);
        DumpSentSms(req.sms,logger);
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
      DumpSentSms(req.sms,logger);
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
    DumpSentSms(req.sms,logger);
    tco->TSMStopped(ltrid);
  }
}

void MTFTSM::BEGIN(Message& msg)
{
  // ����� TCO->TSM, TCO ������� �� SCCP
  // ������ ���� � ��������� IDLE
  // ��������� Calling address and RTID
  // ������� DHA
  // ������� DHA � ���������� TR-BEGIN
  // ������� � ��������� IR
  if(msg.isComponentPresent())
  {
    std::vector<unsigned char> mtbuf;
    mtbuf = msg.getComponent();
    MtForward mtf(logger);
    mtf.decode((void *)&mtbuf[0],mtbuf.size());


    //req =  new MTR(this);
    req.invokeId = msg.getInvokeId();
    req.mms = mtf.isMMS();
    try {
      unpackSccpDigits(req.dstmsc,laddr,laddrlen);
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
    MtForward mtf(logger);
    mtf.decode((void *)&mtbuf[0],mtbuf.size());


    //req =  new MTR(this);
    req.invokeId = msg.getInvokeId();
    req.mms = mtf.isMMS();
    try {
      unpackSccpDigits(req.dstmsc,laddr,laddrlen);
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
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
