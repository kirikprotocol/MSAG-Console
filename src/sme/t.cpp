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

int respDelay=0;


bool autoAnswer=false;

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
        atrans->sendDeliverySmResp(resp);
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
        atrans->sendDeliverySmResp(resp);
        disposePdu(pdu);
        return;
      }else if(rnd<temperrProb+permErrProb+dontrespProb)
      {
        disposePdu(pdu);
        return;
      }
      char buf[65535];
      SMS s;
      fetchSmsFromSmppPdu((PduXSm*)pdu,&s);

      unsigned msgsmlen,msgpllen;
      s.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&msgsmlen);
      s.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&msgpllen);
      __trace2__("received msglen=%u/%u",msgsmlen,msgpllen);
      s.getOriginatingAddress().toString(buf,sizeof(buf));
      printf("\n==========\nFrom:%s\n",buf);
      s.getDestinationAddress().toString(buf,sizeof(buf));
      printf("To:%s\n",buf);
      printf("DCS:%d\n",s.getIntProperty(Tag::SMPP_DATA_CODING));
      printf("UMR:%d\n",s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
      if(s.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
      {
        printf("MsgState:%d\n",s.getIntProperty(Tag::SMPP_MSG_STATE));
      }
      if(getSmsText(&s,buf,sizeof(buf))==-1)
      {
        int sz=65536;
        char *data=new char[sz];
        if(getSmsText(&s,data,sz)!=-1)
        {
          printf("Message(payload):%s\n",data);
        }else
        {
          printf("Error: faield to retrieve message\n");
        }
        delete [] data;
      }else
      {
        printf("Message:%s\n",buf);
      }
      printf("==========\n");
      fflush(stdout);
      if(respDelay)
      {
        int sec=respDelay/1000;
        int msec=respDelay%1000;
        timestruc_t tv={sec,msec*1000000};
        nanosleep(&tv,0);
      }

      if(autoAnswer)
      {
        printf("Autoanswered\n");
        Address oa=s.getOriginatingAddress();
        s.setOriginatingAddress(s.getDestinationAddress());
        s.setDestinationAddress(oa);
        PduSubmitSm sm;
        sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
        fillSmppPduFromSms(&sm,&s);
        atrans->submit(sm);
      }


      if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
      {
        PduDeliverySmResp resp;
        resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
        resp.set_messageId("");
        resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        atrans->sendDeliverySmResp(resp);
      }else
      {
        PduDataSmResp resp;
        resp.get_header().set_commandId(SmppCommandSet::DATA_SM_RESP);
        resp.set_messageId("");
        resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        atrans->sendDataSmResp(resp);
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

  void setTrans(SmppTransmitter *t,SmppTransmitter *at)
  {
    trans=t;
    atrans=at;
  }
protected:
  SmppTransmitter* trans;
  SmppTransmitter* atrans;
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
           "\t -7 send messages in 7 in 8 bit\n"
           "\t -d send messages as DATA_SM\n"
           "\t -s send sms with ussd service op\n"
           "\t -w autoanswer(ping pong) mode\n"
           "\t -c receive only mode\n"
           "\t -m {D|T} send messages in datagram or transaction mode\n"
           "\t -e N probability of answer with temp error\n"
           "\t -r N probability of answer with perm error\n"
           "\t -n N probability of not answering at all\n"
           "\t -l N delay between deliver and response\n"
           );
    return -1;
  }
  Logger::Init("log4cpp.t");
  SmeConfig cfg;
  string host="smsc";
  int port=9001;

  int mode=0;

  bool unicode=false;
  bool dataSm=false;

  string source=argv[1];

  cfg.sid=source;
  cfg.password=cfg.sid;
  MyListener lst;

  int bindType=BindType::Transceiver;

  bool smsc7bit=false;
  bool ussd=false;
  bool receiveOnly=false;

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
      case '7':
      {
        smsc7bit=true;
        i--;
      }continue;
      case 'd':
      {
        dataSm=true;
        i--;
      }continue;
      case 's':
      {
        ussd=true;
        i--;
      }continue;
      case 'c':
      {
        receiveOnly=true;
        i--;
      }continue;
      case 'w':
      {
        autoAnswer=true;
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
      case 'l':
      {
        respDelay=atoi(optarg);
      }break;
      case 'm':
      {
        if(optarg[0]=='D')mode=1;
        else if(optarg[0]=='T')mode=2;
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

  lst.setTrans(tr,ss.getAsyncTransmitter());
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


    s.setIntProperty(Tag::SMPP_ESM_CLASS,mode);
    s.setDeliveryReport(0);
    s.setArchivationRequested(false);

    s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,1234);

    if(ussd)s.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,1);

    s.setEServiceType("XXX");
    char *addr=NULL;
    char *message=NULL;
    if(!receiveOnly)
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
      if(!strcmp((char*)addr,"eq"))
      {
        PduEnquireLink eq;
        eq.get_header().set_commandId(SmppCommandSet::ENQUIRE_LINK);
        eq.get_header().set_sequenceNumber(ss.getNextSeq());
        SmppHeader *resp=tr->sendPdu((SmppHeader*)&eq);
        if(resp)
        {
          printf("Enquire link ok\n");
          disposePdu(resp);
        }else
        {
          printf("No response\n");
        }
        continue;
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
      if(!message)continue;
      int len=strlen(message);


      if(unicode)
      {
        auto_ptr<short> msg(new short[len+1]);
        ConvertMultibyteToUCS2(message,len,msg.get(),len*2,CONV_ENCODING_KOI8R);
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::UCS2);
        len*=2;
        s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,(char*)msg.get(),len);
      }else if(smsc7bit)
      {
        auto_ptr<char> msg(new char[len*3+1]);
        len=ConvertLatin1ToSMSC7Bit(message,len,msg.get());
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::SMSC7BIT);
        s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,msg.get(),len);
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
        if(resp)
        {
          printf("Wasn't accepted: %08X\n",resp->get_commandStatus());
        }else
        {
          printf("Timed out\n");
        }
        fflush(stdout);
      }
      if(resp)disposePdu(resp);
    }

    if(receiveOnly)
    {
      while(!stopped){sleep(5);}
    }
  }
  catch(SmppConnectException& e)
  {
    printf("Bind error:%s\n",e.getTextReason());
    return -1;
  }
  catch(std::exception& e)
  {
    printf("Exception: %s\n",e.what());
  }
  catch(...)
  {
    printf("unknown exception\n");
  }
  {
    PduUnbind pdu;
    pdu.get_header().set_commandId(SmppCommandSet::UNBIND);
    pdu.get_header().set_sequenceNumber(ss.getNextSeq());
    SmppHeader *resp=tr->sendPdu((SmppHeader*)&pdu);
    if(resp)
    {
      printf("unbind resp:%x-%x\n",resp->get_commandId(),resp->get_commandStatus());
      disposePdu(resp);
    }else
    {
      printf("unbind resp timed out\n");
    }
  }
  ss.close();
  printf("Exiting\n");//////
  return 0;
}
