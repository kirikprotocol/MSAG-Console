#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include "util/smstext.h"
#include "readline/readline.h"
#include "readline/history.h"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;

int stopped=0;

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      char buf[256];
      if(!getPduText((PduXSm*)pdu,buf,sizeof(buf)))
      {
        int sz=((PduXSm*)pdu)->optional.size_messagePayload();
        char *data=new char[sz+1];
        memcpy
        (
          data,
          ((PduXSm*)pdu)->optional.get_messagePayload(),
          sz
        );
        data[sz]=0;
        printf("\nReceived payload:%s\n",data);
        delete [] data;
      }else
      {
        printf("\nReceived:%s\n",buf);
      }
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
    rl_forced_update_display();
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
  using_history ();

  if(argc==1)
  {
    printf("usage: %s systemid [options]\n",argv[0]);
    printf("Options are:\n");
    printf("\t -h host[:port] (default == smsc:9001)\n"
           "\t -a addr (defauld == sid)\n"
           "\t -p password (default == sid)\n"
           "\t -t bind type (tx,rx,trx)\n");
    return -1;
  }
  Logger::Init("log4cpp.t");
  SmeConfig cfg;
  string host="smsc";
  int port=9001;

  string source=argv[1];

  cfg.sid=source;
  cfg.password=cfg.sid;
  MyListener lst;

  int bindType=BindType::Transceiver;

  for(int i=2;i<argc;i+=2)
  {
    char *opt=argv[i];
    if(i+1==argc)
    {
      fprintf(stderr,"Option argument missing:%s\n",opt);
      return -1;
    }
    char *optarg=argv[i+1];
    if(opt[0]!='-' || strlen(opt)!=2)
    {
      fprintf(stderr,"Unrecognized option:%s\n",opt);
      return -1;
    }
    switch(opt[1])
    {
      case 'h':
      {
        host=optarg;
        int pos=host.find(":");
        if(pos>0)
        {
          port=atoi(host.substr(pos+1).c_str());
          host.erase(pos);
        }
      }break;
      case 'a':
      {
        source=optarg;
      }break;
      case 'p':
      {
        cfg.password=optarg;
      }break;
      case 't':
      {
        string t=optarg;
        if(t=="rx")bindType=BindType::Receiver;
        else if(t=="tx")bindType=BindType::Transmitter;
        else if(t=="trx")bindType=BindType::Transceiver;
        else
        {
          fprintf(stderr,"Unknown bind type:%s\n",optarg);
          return -1;
        }
      }break;
    }
  }

  cfg.host=host;
  cfg.port=port;
  cfg.timeOut=10;

  SmppSession ss(cfg,&lst);
  try{
    ss.connect(bindType);
    PduSubmitSm sm;
    SMS s;

    {
      try{
        Address addr(source.c_str());
        s.setOriginatingAddress(addr);
      }catch(...)
      {
        fprintf(stderr,"Invalid source address\n");
        return -1;
      }
    }
    char msc[]="";
    char imsi[]="";
    s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);


    s.setValidTime(0);


    s.setIntProperty(Tag::SMPP_ESM_CLASS,0);
    s.setDeliveryReport(0);
    s.setArchivationRequested(false);

    SmppTransmitter *tr=ss.getSyncTransmitter();

    lst.setTrans(tr);
    s.setEServiceType("XXX");
    sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
    char *addr=NULL;
    char *message=NULL;
    while(!stopped)
    {
      if(addr)free(addr);
      addr=NULL;
      addr=readline("Enter destination:");
      if(!addr)break;
      if(!*addr)continue;
      if(!strcmp((char*)addr,"quit"))
      {
        break;
      }
      try{
        Address dst((char*)addr);
        s.setDestinationAddress(dst);
      }catch(...)
      {
        printf("Invalid address\n");
        continue;
      }
      if(message)free(message);
      message=NULL;
      message=readline("Enter message:");
      int len=strlen(message);

      s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
      s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,message,len);

      fillSmppPduFromSms(&sm,&s);
      PduSubmitSmResp *resp;
      try{
        resp=tr->submit(sm);
      }catch(SmppInvalidBindState& e)
      {
        resp=NULL;
        printf("Pdu sent  in invalid bind state\n");
      }

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
