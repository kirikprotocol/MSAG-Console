#include <stdio.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include <unistd.h>
#include "util/recoder/recode_dll.h"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;

int stopped=0;

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      char buf[256];
      const char *msg=((PduXSm*)pdu)->get_message().get_shortMessage();
      int msglen=((PduXSm*)pdu)->get_message().get_smLength();
      int i;
      printf("before:");
      for(i=0;i<msglen;i++)
      {
        printf("%x ",(int)(unsigned char)msg[i]);
      }
      printf("\n");
      int coding=((PduXSm*)pdu)->get_message().get_dataCoding();
      printf("datacoding=%d\n",coding);
      if(coding==DataCoding::DEFAULT)
      {
        msglen=Convert7BitToText(msg,msglen,buf,sizeof(buf));
      }else if(coding==DataCoding::UCS2)
      {
        char bufx[256];
        //len=msglen/2;
        int l7=ConvertUCS2To7Bit((const short*)msg,msglen,bufx,sizeof(bufx));
        msglen=Convert7BitToText(bufx,l7,buf,sizeof(bufx));
      }
      bug[msglen]=0;
      printf("after:");
      for(i=0;i<msglen;i++)
      {
        printf("%x ",(int)(unsigned char)buf[i]);
      }
      printf("\n");
      printf("\nReceived:%s\n",buf);
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      trans->sendDeliverySmResp(resp);
    }else
    if(pdu->get_commandId()==SmppCommandSet::SUBMIT_SM_RESP)
    {
      printf("\nReceived async submit sm resp:%d\n",pdu->get_commandStatus());
    }
  }
  void handleError(int errorCode)
  {
    printf("error!\n");
    stopped=1;
  }

  void setTrans(SmppTransmitter *t)
  {
    trans=t;
  }
protected:
  SmppTransmitter* trans;
};

int main(int argc,char* argv[])
{
  if(argc==1)
  {
    printf("usage: %s systemid\n",argv[0]);
    return -1;
  }
  SmeConfig cfg;
  cfg.host="smsc";
  cfg.port=argc>2?atoi(argv[2]):9001;
  cfg.sid=argv[1];
  cfg.timeOut=10;
  cfg.password="";
  MyListener lst;
  SmppSession ss(cfg,&lst);
  try{
    ss.connect();
    PduSubmitSm sm;
    SMS s;
//    const char *dst="2";
//47.44.rymhrwDMy4
    Address addr(cfg.sid.c_str());
    s.setOriginatingAddress(addr);
    char msc[]="123";
    char imsi[]="123";
    s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
    //s.setWaitTime(0);
    time_t t=time(NULL)+60;
    s.setValidTime(t);
    //s.setSubmitTime(0);
    //s.setPriority(0);
    //s.setProtocolIdentifier(0);
    s.setDeliveryReport(0);
    s.setArchivationRequested(false);
    //unsigned char message[]="SME test message";
    SmppTransmitter *tr=ss.getSyncTransmitter();
    //SmppTransmitter *atr=ss.getAsyncTransmitter();
    lst.setTrans(tr);
    s.setEServiceType("XXX");
    sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
    while(!stopped)
    {
      unsigned char message[512];
      printf("Enter destination:");fflush(stdout);
      fgets((char*)message,sizeof(message),stdin);
      if(!strcmp((char*)message,"quit\n"))
      {
        break;
      }
      int i=0;
      while(message[i])
      {
        if(message[i]<32)
        {
          message[i]=0;
          break;
        }
        i++;
      }
      Address dst((char*)message);
      s.setDestinationAddress(dst);
      printf("Enter message:");fflush(stdout);
      fgets((char*)message,sizeof(message),stdin);
      for(int i=0;message[i];i++)
      {
        if(message[i]<32)message[i]=0;
      }
      int len=strlen((char*)message);
      char buf7[256];
      int len7=ConvertTextTo7Bit((char*)message,len,buf7,sizeof(buf7),CONV_ENCODING_ANSI);

      //s.setMessageBody(len,1,false,message);
      s.setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,buf7,len7);
      s.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,len7);
      s.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);

      fillSmppPduFromSms(&sm,&s);
      PduSubmitSmResp *resp=tr->submit(sm);
//      atr->submit(sm);
      if(resp && resp->get_header().get_commandStatus()==0)
      {
        printf("Accepted:%d bytes\n",len);fflush(stdout);
      }else
      {
        printf("Wasn't accepted\n");fflush(stdout);
      }
      if(resp)disposePdu((SmppHeader*)resp);
    }
  }
  catch(std::exception& e)
  {
    printf("Exception: %s\n",e.what());
  }
  catch(...)
  {
    printf("unknown exception\n");
  }
  ss.close();
  printf("Exiting\n");
  return 0;
}
