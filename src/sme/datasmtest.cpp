#include <stdio.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include <unistd.h>
#include "util/smstext.h"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;

int stopped=0;

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DATA_SM)
    {
      int sz=((PduDataSm*)pdu)->optional.size_messagePayload();
      char *data=new char[sz+1];
      memcpy
      (
        data,
        ((PduDataSm*)pdu)->optional.get_messagePayload(),
        sz
      );
      data[sz]=0;
      printf("\nReceived payload:%s\n",data);
      delete [] data;
      PduDataSmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DATA_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      trans->sendDataSmResp(resp);
    }else
    if(pdu->get_commandId()==SmppCommandSet::SUBMIT_SM_RESP)
    {
      printf("\nReceived async submit sm resp:%d\n",pdu->get_commandStatus());
    }
    disposePdu(pdu);
  }
  void handleError(int errorCode)
  {
    printf("\n\n\nerror!\n\n\n");
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
    printf("usage: %s systemid host[:port] [sourceaddr]\n",argv[0]);
    return -1;
  }
  SmeConfig cfg;
  string host=argc>2?argv[2]:"smsc";
  int pos=host.find(":");
  int port=9001;
  if(pos>0)
  {
    port=atoi(host.substr(pos+1).c_str());
    host.erase(pos);
  }
  string source=argv[1];
  if(argc>3)source=argv[3];
  cfg.host=host;
  cfg.port=port;
  cfg.sid=argv[1];
  cfg.timeOut=10;
  cfg.password=cfg.sid;
  MyListener lst;
  SmppSession ss(cfg,&lst);
  try{
    ss.connect();
    PduDataSm sm;
    SMS s;
//    const char *dst="2";
//47.44.rymhrwDMy4
    Address addr(source.c_str());
    s.setOriginatingAddress(addr);
    char msc[]="123";
    char imsi[]="123";
    s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
    //s.setWaitTime(0);
    //time_t t=time(NULL)+60;
    s.setValidTime(0);
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
    sm.get_header().set_commandId(SmppCommandSet::DATA_SM);
    while(!stopped)
    {
      char message[4096];
      printf("Enter destination:");fflush(stdout);
      fgets((char*)message,sizeof(message),stdin);
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
      if(!*message)continue;
      if(!strcmp((char*)message,"quit"))
      {
        break;
      }
      try{
        Address dst((char*)message);
        s.setDestinationAddress(dst);
      }catch(...)
      {
        printf("Invalid address\n");
        continue;
      }
      printf("Enter message:");fflush(stdout);
      //fgets((char*)message,sizeof(message),stdin);
      for(int i=0;i<sizeof(message);i++)message[i]='!';
      message[sizeof(message)-1]=0;
      /*for(int i=0;message[i];i++)
      {
        if(message[i]<32)message[i]=0;
      }*/
      int len=strlen((char*)message);

      s.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,message,len);
      s.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,0);
      s.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
      fillDataSmFromSms(&sm,&s);
      PduDataSmResp *resp=tr->data(sm);
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
  catch(SmppConnectException& e)
  {
    printf("Bind error:%s\n",e.getTextReason());
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
