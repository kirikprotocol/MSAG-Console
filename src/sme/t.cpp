#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include "util/smstext.h"
#include "readline/readline.h"
#include "readline/history.h"
#include <locale.h>

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;

int stopped=0;

int temperrProb=0;
int dontrespProb=0;
int permErrProb=0;

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM  ||
       pdu->get_commandId()==SmppCommandSet::DATA_SM)
    {
      int rnd=rand()%100;
      if(rnd<temperrProb)
      {
        PduDeliverySmResp resp;
        resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
        resp.set_messageId("");
        resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        resp.get_header().set_commandStatus(SmppStatusSet::ESME_RMSGQFUL);
        trans->sendDeliverySmResp(resp);
        disposePdu(pdu);
        return;
      }
      if(rnd<temperrProb+permErrProb)
      {
        PduDeliverySmResp resp;
        resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
        resp.set_messageId("");
        resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        resp.get_header().set_commandStatus(SmppStatusSet::ESME_RX_P_APPN);
        trans->sendDeliverySmResp(resp);
        disposePdu(pdu);
        return;
      }else if(rnd<temperrProb+permErrProb+dontrespProb)
      {
        disposePdu(pdu);
        return;
      }
      char buf[256];
      SMS s;
      fetchSmsFromSmppPdu((PduXSm*)pdu,&s);
      s.getOriginatingAddress().toString(buf,sizeof(buf));
      printf("\n==========\nFrom:%s\n",buf);
      s.getDestinationAddress().toString(buf,sizeof(buf));
      printf("To:%s\n",buf);
      printf("DCS:%d\n",s.getIntProperty(Tag::SMPP_DATA_CODING));
      printf("UMR:%d\n",s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
      if(getPduText((PduXSm*)pdu,buf,sizeof(buf))==-1)
      {
        int sz=((PduXSm*)pdu)->optional.size_messagePayload();
        char *data=new char[sz+1];
        if(getPduText((PduXSm*)pdu,buf,sizeof(buf))==-1)
        {
          printf("Message(payload):%s\n",data);
        }else
        {
          printf("Error: faield to retrieve message");
        }
        delete [] data;
      }else
      {
        printf("Message:%s\n",buf);
      }
      printf("==========\n");
      fflush(stdout);
      if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
      {
        PduDeliverySmResp resp;
        resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
        resp.set_messageId("");
        resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        trans->sendDeliverySmResp(resp);
      }else
      {
        PduDataSmResp resp;
        resp.get_header().set_commandId(SmppCommandSet::DATA_SM_RESP);
        resp.set_messageId("");
        resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        trans->sendDataSmResp(resp);
      }
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
           "\t -t bind type (tx,rx,trx)\n"
           "\t -u send messages in unicode\n"
           "\t -d send messages as DATA_SM\n"
           "\t -e N probability of answer with temp error\n"
           "\t -r N probability of answer with perm error\n"
           "\t -n N probability of not answering at all\n"
           );
    return -1;
  }
  Logger::Init("log4cpp.t");
  SmeConfig cfg;
  string host="smsc";
  int port=9001;

  bool unicode=false;
  bool dataSm=false;

  string source=argv[1];

  cfg.sid=source;
  cfg.password=cfg.sid;
  MyListener lst;

  int bindType=BindType::Transceiver;

  for(int i=2;i<argc;i+=2)
  {
    char *opt=argv[i];
    switch(opt[1])
    {
      case 'u':
      {
        unicode=true;
        i--;
      }continue;
      case 'd':
      {
        dataSm=true;
        i--;
      }continue;
    }
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
      case 'e':
      {
        temperrProb=atoi(optarg);
      }break;
      case 'r':
      {
        permErrProb=atoi(optarg);
      }break;
      case 'n':
      {
        dontrespProb=atoi(optarg);
      }break;
      default:
      {
        fprintf(stderr,"Unknown option:%s\n",opt);
        return -1;
      };
    }
  }

  cfg.host=host;
  cfg.port=port;
  cfg.timeOut=10;

  SmppSession ss(cfg,&lst);
  SmppTransmitter *tr=ss.getSyncTransmitter();

  lst.setTrans(tr);
  try{
    ss.connect(bindType);
    PduSubmitSm sm;
    sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
    PduDataSm dsm;
    dsm.get_header().set_commandId(SmppCommandSet::DATA_SM);


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

    s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,1234);

    s.setEServiceType("XXX");
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
      if(strcmp(addr,"/"))
      {
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
        if(!message)break;
        rl_reset_line_state();
      }
      int len=strlen(message);


      if(unicode)
      {
        short *msg=new short[len+1];
        ConvertMultibyteToUCS2(message,len,msg,len*2,CONV_ENCODING_KOI8R);
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::UCS2);
        len*=2;
        s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,(char*)msg,len);
      }else
      {
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
        s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,message,len);
      }

      SmppHeader *resp;
      if(!dataSm)
      {
        fillSmppPduFromSms(&sm,&s);
        try{
          resp=(SmppHeader*)tr->submit(sm);
        }catch(SmppInvalidBindState& e)
        {
          resp=NULL;
          printf("Pdu sent in invalid bind state\n");
        }
      }else
      {
        fillDataSmFromSms(&dsm,&s);
        try{
          resp=(SmppHeader*)tr->data(dsm);
        }catch(SmppInvalidBindState& e)
        {
          resp=NULL;
          printf("Pdu sent in invalid bind state\n");
        }
      }

      if(resp && resp->get_commandStatus()==0)
      {
        printf("Accepted:%d bytes\n",len);fflush(stdout);
      }else
      {
        printf("Wasn't accepted: %08X\n",resp?resp->get_commandStatus():-1);fflush(stdout);
      }
      if(resp)disposePdu(resp);
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
  printf("Exiting\n");//////
  return 0;
}
