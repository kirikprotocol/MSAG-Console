#include "system/abonentinfo/AbonentInfo.hpp"
#include "util/smstext.h"
#include "logger/Logger.h"
#include "resourcemanager/ResourceManager.hpp"
#include <exception>

namespace smsc{
namespace system{
namespace abonentinfo{

using smsc::util::getSmsText;
using smsc::util::fillSms;
using namespace smsc::smeman;
using namespace smsc::resourcemanager;

class FakeGetAdapter:public GetAdapter{
public:

  virtual bool isNull(const char* key)
      throw(AdapterException)
  {
    return false;
  }

  virtual const char* getString(const char* key)
      throw(AdapterException)
  {
    return "";
  }

  virtual int8_t getInt8(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual int16_t getInt16(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual int32_t getInt32(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual int64_t getInt64(const char* key)
      throw(AdapterException)
  {
    return 0;
  }


  virtual uint8_t getUint8(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual uint16_t getUint16(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual uint32_t getUint32(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual uint64_t getUint64(const char* key)
      throw(AdapterException)
  {
    return 0;
  }


  virtual float getFloat(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual double getDouble(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual long double getLongDouble(const char* key)
      throw(AdapterException)
  {
    return 0;
  }


  virtual time_t getDateTime(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

};


int AbonentInfoSme::Execute()
{
  SmscCommand cmd,resp;
  char body[MAX_SHORT_MESSAGE_LENGTH+1];
  SMS *sms;
  Profile p;
  SMS s;
  char msc[]="";
  char imsi[]="";
  s.setOriginatingDescriptor((uint8_t)strlen(msc),msc,(uint8_t)strlen(imsi),imsi,1);
  s.setDeliveryReport(0);
  s.setArchivationRequested(false);
  s.setEServiceType(servType.c_str());
  s.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protId);
  smsc::logger::Logger *log=smsc::logger::Logger::getInstance("smsc.system.AbonentInfoSme");

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
      smsc_log_warn(log, "Incorrect command received");
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

      getSmsText(sms,body,(unsigned)sizeof(body));
      try{
        char* comma=strchr(body,',');
        AbonentStatus::StatusRequestMode srm=AbonentStatus::srmDefault;
        bool isError=false;
        if(comma)
        {
          *comma=0;
          comma++;
          if(strcmp(comma,"ATI")==0)
          {
            srm=AbonentStatus::srmATI;
          }else if(strcmp(comma,"SRI4SM")==0)
          {
            srm=AbonentStatus::srmSRI4SM;
          }else
          {
            isError=true;
          }
        }
        Address a(body);
        Address d;
        if(!smsc->AliasToAddress(a,d))
        {
          d=a;
        }
        smsc::router::RouteResult rr;
        int src_proxy_index=-1;
        if(sms->getSourceSmeId()[0])
        {
          src_proxy_index=smsc->getSmeIndex(sms->getSourceSmeId());
        }
        bool has_route = smsc->routeSms(src_proxy_index,sms->getOriginatingAddress(),d,rr);

        Address oa=sms->getOriginatingAddress();
        Address da=sms->getDestinationAddress();
        int umr=sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
        cmd=SmscCommand::makeQueryAbonentStatus(d);
        cmd->get_abonentStatus().originalAddr=body;
        cmd->get_abonentStatus().sourceAddr=oa;
        cmd->get_abonentStatus().destAddr=da;
        cmd->get_abonentStatus().userMessageReference=umr;
        cmd->get_abonentStatus().isMobileRequest=(da==hrSrc);
        cmd->get_abonentStatus().srm=srm;

        int status=AbonentStatus::UNKNOWNVALUE;
        if(!has_route || !rr.destProxy)
        {
          status=AbonentStatus::OFFLINE;
        }
        else
        {
          if(rr.info.smeSystemId!="MAP_PROXY")status=AbonentStatus::ONLINE;
        }

        char a1[32];
        da.toString(a1,sizeof(a1));
        char a2[32];
        hrSrc.toString(a2,sizeof(a2));
        __trace2__("AbonentInfo: destaddr=%s, hrAddr=%s",a1,a2);


        if(status!=AbonentStatus::UNKNOWNVALUE)
        {
          __trace__("AbonentInfo: request for sme address.");
          cmd=SmscCommand::makeQueryAbonentStatusResp(cmd->get_abonentStatus(),status,0,"","");
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

      std::string answ;

      try{
        if(as.isMobileRequest)
        {
          FakeGetAdapter ga;
          ContextEnvironment ce;
          ce.exportStr("abonent",as.originalAddr.c_str());
          ce.exportStr("status",as.status==AbonentStatus::ONLINE? "Online":
                                as.status==AbonentStatus::OFFLINE?"Offline":
                                                                  "Unknown");
          ce.exportStr("msc",as.msc.length()?as.msc.c_str():"unknown");
          Profile pr=smsc->getProfiler()->lookup(as.sourceAddr);
          OutputFormatter* f=ResourceManager::getInstance()->getFormatter(pr.locale,"abonentinfo.mobileFormat");
          if(!f)
          {
            answ="Unknown formatter abonentinfo.mobileFormat for locale ";
            answ+=pr.locale.c_str();
          }else
          {
            f->format(answ,ga,ce);
          }
        }else
        {
          FakeGetAdapter ga;
          ContextEnvironment ce;
          ce.exportStr("abonent",as.originalAddr.c_str());
          ce.exportInt("status",as.status);
          ce.exportInt("code",as.code);
          ce.exportInt("encoding",p.codepage);
          ce.exportStr("msc",as.msc.length()?("+"+as.msc).c_str():"");
          ce.exportStr("imsi",as.imsi.length()?as.imsi.c_str():"");
          Profile pr=smsc->getProfiler()->lookup(as.sourceAddr);
          OutputFormatter* f=ResourceManager::getInstance()->getFormatter(pr.locale,"abonentinfo.smeFormat");
          __trace2__("AbonentInfo: formatter=%p",f);
          if(!f)
          {
            answ="Unknown formatter abonentinfo.smeFormat for locale ";
            answ+=pr.locale.c_str();
          }else
          {
            f->format(answ,ga,ce);
          }
        }
      }catch(std::exception& e)
      {
        __trace2__("AbonentInfo: Formatter exception %s",e.what());
      }catch(...)
      {
        __trace__("AbonentInfo: Formatter exception unknown");
      }

      int len=(int)answ.length();
      //char buf7[MAX_SHORT_MESSAGE_LENGTH];
      //int len7=ConvertTextTo7Bit(answ,len,buf7,sizeof(buf7),CONV_ENCODING_ANSI);

      time_t t=time(NULL)+60;
      s.setValidTime(t);
      s.setOriginatingAddress(as.destAddr);
      s.setDestinationAddress(as.sourceAddr);
      s.setIntProperty(Tag::SMPP_ESM_CLASS,0);
      fillSms(&s,answ.c_str(),len,CONV_ENCODING_CP1251,DataCoding::UCS2);
      /*
      s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
      s.setBinProperty(Tag::SMPP_SHORT_MESSAGE,answ.c_str(),len);
      s.setIntProperty(Tag::SMPP_SM_LENGTH,len);

      */
      s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
        as.userMessageReference);

      resp=SmscCommand::makeSumbmitSm(s,getNextSequenceNumber());
      putIncomingCommand(resp);
    }

  }
  return 0;
}

}//abonentinfo
}//system
}//smsc
