static char const ident[] = "$Id$";
#include "util.hpp"
#include "MTFTSM.hpp"
#include "TCO.hpp"
#include "Message.hpp"
#include "MtForward.hpp"
#include "MTRequest.hpp"
#include "logger/Logger.h"

namespace smsc{namespace mtsmsme{namespace processor{

using namespace smsc::mtsmsme::processor::util;

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
 * ���� �� ����� BEGIN ��� ���������� �� CONTINUE
 * ���� �� ����� BEGIN � ����������� �� ��� MMS �� END
 * ���� �� ����� BEGIN
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
  // ����� TCO->TSM, TCO ������� �� SCCP
  // ������ ���� � ��������� IDLE
  // ��������� Calling address and RTID
  // ������� DHA
  // ������� DHA � ���������� TR-BEGIN
  // ������� � ��������� IR
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
    makeSmsToRequest(&mtf,&req);
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
    makeSmsToRequest(&mtf,&req);
    if (tco->sender) tco->sender->send(&req);
    smsc_log_debug(logger,"tsm otid=%s receive CONTINUE with component, mms=%d, INVOKE sent",ltrid.toString().c_str(),req.mms);
  }
}
void MTFTSM::BeginTransaction(TsmComletionListener* _listener) { /* there is no action */ }
void MTFTSM::END_received(Message& msg) { /* there is no action */ }

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/