#include "system/abonentinfo/AbonentInfo.hpp"
#include "util/smstext.h"
#include "util/Logger.h"

namespace smsc{
namespace system{
namespace abonentinfo{

using smsc::util::getSmsText;
using smsc::smeman;

int AbonentInfoSme::Execute()
{
  SmscCommand cmd,resp,answ;
  char body[MAX_SHORT_MESSAGE_LENGTH+1];
  SMS *sms;
  Profile p;
  SMS s;
  char msc[]="";
  char imsi[]="";
  s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
  time_t t=time(NULL)+60;
  s.setValidTime(t);
  s.setDeliveryReport(0);
  s.setArchivationRequested(false);
  s.setEServiceType(servType.c_str());
  s.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protId);
  log4cpp::Category &log=smsc::util::Logger::getCategory("smsc.system.AbonentInfoSme");

  while(!isStopping)
  {
    if(!hasOutput())
    {
      waitFor();
    }
    if(!hasOutput())continue;
    cmd=getOutgoingCommand();
    if(cmd->cmdid==smsc::smeman::SUBMIT_RESP)continue;
    if(cmd->cmdid!=smsc::smeman::DELIVERY && cmd->cmdid!=smsc::smeman::QUERYABONENTSTATUS_RESP)
    {
      __trace2__("AbonentInfoSme: incorrect command submitted:%d",cmd->cmdid);
      log.warn("Incorrect command received");
      continue;
    }
    if(cmd->cmdid==smsc::smeman::DELIVERY)
    {
      sms=cmd->get_sms();
      resp=SmscCommand::makeDeliverySmResp(
        sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str(),
        cmd->get_dialogId(),
        Status::OK);
      putIncomingCommand(resp);

      getSmsText(sms,body,sizeof(body));
      try{
        Address a(body);
        Address d;
        if(!smsc->AliasToAddress(a,d))
        {
          d=a;
        }
        smsc::router::RouteInfo ri;
        int dest_proxy_index;
        SmeProxy *dest_proxy=NULL;
        bool has_route = smsc->routeSms(sms->getOriginatingAddress(),d,dest_proxy_index,dest_proxy,&ri);

        Address oa=sms->getOriginatingAddress();
        Address da=sms->getDestinationAddress();
        int umr=sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
        cmd=SmscCommand::makeQueryAbonentStatus(d);
        cmd->get_abonentStatus().originalAddr=body;
        cmd->get_abonentStatus().sourceAddr=oa;
        cmd->get_abonentStatus().destAddr=da;
        cmd->get_abonentStatus().userMessageReference=umr;
        cmd->get_abonentStatus().isMobileRequest=(da==hrSrc);

        int status=AbonentStatus::UNKNOWNVALUE;
        if(!has_route || !dest_proxy)
        {
          status=AbonentStatus::OFFLINE;
        }
        else
        {
          if(ri.smeSystemId!="MAP_PROXY")status=AbonentStatus::ONLINE;
        }

        char a1[32];
        da.toString(a1,sizeof(a1));
        char a2[32];
        hrSrc.toString(a2,sizeof(a2));
        __trace2__("AbonentInfo: destaddr=%s, hrAddr=%s",a1,a2);


        if(status!=AbonentStatus::UNKNOWNVALUE)
        {
          __trace__("AbonentInfo: request for sme address.");
          cmd=SmscCommand::makeQueryAbonentStatusResp(cmd->get_abonentStatus(),status,"");
          __trace2__("AbonentInfo: cmdid=%d, QAS_RESP=%d",cmd->cmdid,smsc::smeman::QUERYABONENTSTATUS_RESP);
        }
        else
        {
          putIncomingCommand(cmd);
          __trace2__("AbonentInfo: delivery->QueryAbonentStatus for %s",body);
          continue;
        }
      }
      catch(...)
      {
        __trace__("AbonentInfo: invalid address received. Ignored");
      }
    }
    if(cmd->cmdid==smsc::smeman::QUERYABONENTSTATUS_RESP)
    {
      AbonentStatus &as=cmd->get_abonentStatus();
      Address d=as.addr;
      __trace2__("AbonentInfo: QueryAbonentStatus->response for %s",
        as.originalAddr.c_str());

      p=smsc->getProfiler()->lookup(d);

      char answ[MAX_SHORT_MESSAGE_LENGTH];
      if(as.isMobileRequest)
      {
        sprintf
        (
          answ,
          "Abonent %s is %s. %s",
            as.originalAddr.c_str(),
            as.status==AbonentStatus::ONLINE? "Online":
            as.status==AbonentStatus::OFFLINE?"Offline":
                                              "Unknown",
            as.status==AbonentStatus::UNKNOWNVALUE?"":
            as.msc.length()?("msc +"+as.msc).c_str():
            "msc unknown"
        );
      }else
      {
        sprintf(answ,"%s:%d,%d,%s",
          as.originalAddr.c_str(),
          as.status,
          p.codepage,
          as.msc.length()?("+"+as.msc).c_str():""
          );
      }

      int len=strlen(answ);
      //char buf7[MAX_SHORT_MESSAGE_LENGTH];
      //int len7=ConvertTextTo7Bit(answ,len,buf7,sizeof(buf7),CONV_ENCODING_ANSI);

      s.setOriginatingAddress(as.destAddr);
      s.setDestinationAddress(as.sourceAddr);
      s.setIntProperty(Tag::SMPP_ESM_CLASS,0);
      s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
      s.setBinProperty(Tag::SMPP_SHORT_MESSAGE,answ,len);
      s.setIntProperty(Tag::SMPP_SM_LENGTH,len);
      s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
        as.userMessageReference);


      resp=SmscCommand::makeSumbmitSm(s,getNextSequenceNumber());
      putIncomingCommand(resp);
    }

  }
  return 0;
}

};//abonentinfo
};//system
};//smsc
