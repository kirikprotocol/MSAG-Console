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
#include "core/buffers/Hash.hpp"
#include <vector>
#include "logger/Logger.h"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace std;
using namespace smsc::core::buffers;
using smsc::logger::Logger;

int stopped=0;

int temperrProb=0;
int dontrespProb=0;
int permErrProb=0;

int respDelay=0;

int mode=0;

bool unicode=false;
bool binmode=false;
bool dataSm=false;
bool smsc7bit=false;
int ussd=0;

int esmclass=0;

int sar_mr=0;
int sar_num=0;
int sar_seq=0;

int src_port=0;
int dst_port=0;

bool ansi1251=false;

struct Option{
  const char* name;
  char type;
  void* addr;
  int& asInt(){return *(int*)addr;}
  bool& asBool(){return *(bool*)addr;}
};

Option options[]={
{"temperr",'i',&temperrProb},
{"noresp",'i',&dontrespProb},
{"permerr",'i',&permErrProb},
{"respdelay",'i',&respDelay},
{"mode",'m',&mode},
{"unicode",'b',&unicode},
{"datasm",'b',&dataSm},
{"7bit",'b',&smsc7bit},
{"ussd",'i',&ussd},
{"ansi1251",'b',&ansi1251},
{"srcport",'i',&src_port},
{"dstport",'i',&dst_port},
{"binmode",'b',&binmode},
{"esmclass",'i',&esmclass},
};

const int optionsCount=sizeof(options)/sizeof(Option);

string sourceAddress;


bool autoAnswer=false;

static bool splitString(/*in,out*/string& head,/*out*/string& tail)
{
  string::size_type pos=head.find(' ');
  if(pos==string::npos)return false;
  int firstPos=pos;
  while(head[pos]==' ')pos++;
  tail=head.substr(pos);
  head.erase(firstPos);
  return true;
}

typedef void(*CommandProc)(SmppSession&,const string& args);

struct CmdRec{
  const char* cmdname;
  CommandProc cmd;
};

void QueryCmd(SmppSession& ss,const string& args)
{
  PduQuerySm q;
  if(args.length()==0)
  {
    printf("Usage: query msgId\n");
    return;
  }
  q.set_messageId(args.c_str());
  Address src=sourceAddress.c_str();
  q.get_source().set_typeOfNumber(src.type);
  q.get_source().set_numberingPlan(src.plan);
  q.get_source().set_value(src.value);
  SmppTransmitter *tr=ss.getSyncTransmitter();
  PduQuerySmResp *qresp=tr->query(q);
  if(qresp)
  {
      printf("Query result:commandStatus=%#x(%d), messageState=%d, errorCode=%d, finalDate=%s\n",
        qresp->get_header().get_commandStatus(),qresp->get_header().get_commandStatus(),
        qresp->get_messageState(),
        qresp->get_errorCode(),
        qresp->get_finalDate()?qresp->get_finalDate():"NULL"
      );
    disposePdu((SmppHeader*)qresp);
  }
  else
  {
    printf("Query timed out\n");
  }
}

void EnquireLinkCmd(SmppSession& ss,const string& args)
{
  PduEnquireLink eq;
  SmppTransmitter* tr=ss.getSyncTransmitter();
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
}

void ReplaceCmd(SmppSession& ss,const string& args)
{
  PduReplaceSm r;
  string id=args;
  string msg;
  if(!splitString(id,msg))
  {
    printf("Usage: replace msgId newmessage\n");
    return;
  }
  r.set_messageId(id.c_str());
  Address addr(sourceAddress.c_str());
  r.get_source().set_typeOfNumber(addr.type);
  r.get_source().set_numberingPlan(addr.plan);
  r.get_source().set_value(addr.value);
  r.shortMessage.copy(msg.c_str(),msg.length());
  SmppTransmitter* tr=ss.getSyncTransmitter();
  PduReplaceSmResp *replresp=tr->replace(r);
  if(replresp)
  {
    printf("Replace status:%#x(%d)\n",replresp->get_header().get_commandStatus(),replresp->get_header().get_commandStatus());
    disposePdu((SmppHeader*)replresp);
  }
  else
  {
    printf("Replace timed out\n");
  }
}

void CancelCmd(SmppSession& ss,const string& args)
{
  if(args.length()==0)
  {
    printf("Usage: cancel msgId\n");
    return;
  }
  PduCancelSm q;
  q.set_messageId(args.c_str());


  Address addr(sourceAddress.c_str());
  q.get_source().set_typeOfNumber(addr.type);
  q.get_source().set_numberingPlan(addr.plan);
  q.get_source().set_value(addr.value);

  /*
  q.set_serviceType("XXX");
  q.get_dest().set_typeOfNumber(s.getDestinationAddress().type);
  q.get_dest().set_numberingPlan(s.getDestinationAddress().plan);
  q.get_dest().set_value(s.getDestinationAddress().value);
  //printf("Cancelling:%s\n",resp->get_messageId());*/
  SmppTransmitter* tr=ss.getSyncTransmitter();
  PduCancelSmResp *cresp=tr->cancel(q);
  if(cresp)
  {
    printf("Cancel result:%d\n",cresp->get_header().get_commandStatus());
    disposePdu((SmppHeader*)cresp);
  }
  else
  {
    printf("Cancel timedout\n");
  }
}

const char* modes[]=
{
  "default(def)",
  "datagram(dg)",
  "forward(fwd)",
  "store and forward(store)"
};

static void ShowOption(Option& opt)
{
  printf("%s=",opt.name);
  switch(opt.type)
  {
    case 'i':printf("%d\n",opt.asInt());break;
    case 'b':printf("%s\n",opt.asBool()?"yes":"no");break;
    case 'm':printf("%s\n",modes[opt.asInt()]);break;
  }
}

static bool atob(const string& val)
{
  return val=="yes" || val=="true";
}

void SetOption(SmppSession& ss,const string& args)
{
  if(args.length()==0)
  {
    printf("Usage: set optionname=optionvalue\nAvailable options are:\n");
    for(unsigned int i=0;i<sizeof(options)/sizeof(Option);i++)
    {
      ShowOption(options[i]);
    }
    return;
  }
  string::size_type pos=args.find('=');
  if(pos==string::npos)
  {
    for(unsigned int i=0;i<sizeof(options)/sizeof(Option);i++)
    {
      if(args==options[i].name)
      {
        ShowOption(options[i]);
        return;
      }
    }
    printf("Unknown option\n");
    return;
  }
  string opt,val;
  opt=args.substr(0,pos);
  val=args.substr(pos+1);
  for(unsigned int i=0;i<sizeof(options)/sizeof(Option);i++)
  {
    if(opt==options[i].name)
    {
      switch(options[i].type)
      {
        case 'i':options[i].asInt()=atoi(val.c_str());break;
        case 'b':options[i].asBool()=atob(val);break;
        case 'm':
        {
          if(val=="def")options[i].asInt()=0;
          else if(val=="dg")options[i].asInt()=1;
          else if(val=="fwd")options[i].asInt()=2;
          else if(val=="store")options[i].asInt()=4;
        }break;
      }
      return;
    }
  }
  printf("Unknown option\n");
}

void SetSar(SmppSession& ss,const string& args)
{
  string mr=args;
  string num,seq;
  if(args=="off")
  {
    sar_mr=0;
    sar_num=0;
    sar_seq=0;
  }
  if(!splitString(mr,num))
  {
    printf("Usage: sar mr num seq\nor sar off\n to reset sar info");
    return;
  }
  if(!splitString(num,seq))
  {
    printf("Usage: sar mr num seq\nor sar off\n to reset sar info");
    return;
  }
  sar_mr=atoi(mr.c_str());
  sar_num=atoi(num.c_str());
  sar_seq=atoi(seq.c_str());
  printf("sar info set to mr=%d, num=%d, seq=%d\n",sar_mr,sar_num,sar_seq);
}

void ShowHelp(SmppSession& ss,const string& args);

CmdRec commands[]={
{"query",QueryCmd},
{"eq",EnquireLinkCmd},
{"replace",ReplaceCmd},
{"cancel",CancelCmd},
{"set",SetOption},
{"sar",SetSar},
{"help",ShowHelp},
};

const int commandsCount=sizeof(commands)/sizeof(CmdRec);

void ShowHelp(SmppSession& ss,const string& args)
{
  printf("Available commands:\n");
  for( int i=0;i<commandsCount;i++)
  {
    printf("%s\n",commands[i].cmdname);
  }
  printf("/ - send last message\n");
}

char** cmd_completion(const char *text,int start,int end)
{
  string word(rl_line_buffer+start,rl_line_buffer +end);
  vector<string> found;
  string tmp;
  rl_attempted_completion_over=1;
  __trace2__("completion: %s,%d,%d",rl_line_buffer ,start,end);
  if(start>=4 && !strncmp(rl_line_buffer ,"set ",4))
  {
    __trace__("options completion");
    rl_completion_append_character='=';
    for(int i=0;i<optionsCount;i++)
    {
      tmp.assign(options[i].name,word.length());
      if(tmp==word)
      {
        found.push_back(options[i].name);
      }
    }
  }else
  {
    __trace__("commands completion");
    rl_completion_append_character=' ';
    if(strchr(rl_line_buffer,' '))
    {
      /*char **rv=(char**)malloc(2*sizeof(char*));
      rv[0]=(char*)malloc(1);
      rv[0][0]=0;
      rv[1]=0;
      rl_completion_append_character=0;
      return rv;*/
      return 0;
    }
    for(int i=0;i<commandsCount;i++)
    {
      tmp.assign(commands[i].cmdname,word.length());
      if(tmp==word)
      {
        found.push_back(commands[i].cmdname);
      }
    }
  }
  if(found.size()==0)
  {
    /*char **rv=(char**)malloc(2*sizeof(char*));
    rv[0]=(char*)malloc(1);
    rv[0][0]=0;
    rv[1]=0;
    rl_completion_append_character=0;
    return rv;*/
    return 0;
  }
  __trace2__("completion: found %d entries",found.size());
  char **rv;
  rv=(char**)malloc((2+found.size())*sizeof(char*));
  int j=0;
  if(found.size()>1)
  {
    rv[0]=(char*)malloc(1);
    rv[0][0]=0;
    j++;
  }
  for(vector<string>::size_type i=0;i<found.size();i++,j++)
  {
    rv[j]=(char*)malloc(found[i].length()+1);
    memcpy(rv[j],found[i].c_str(),found[i].length()+1);
    __trace2__("match%d=%s",i,rv[j]);
  }
  rv[j]=0;
  return rv;
}


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
      if(s.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE))
      {
        printf("UMR:%d\n",s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
      }
      if(s.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
      {
        printf("MsgState:%d\n",s.getIntProperty(Tag::SMPP_MSG_STATE));
      }

      if(s.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
      {
        unsigned char* body;
        unsigned len;
        if(s.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
        {
          body=(unsigned char*)s.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
        }else
        {
          body=(unsigned char*)s.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
        }
        int udhlen=body[0];
        printf("Udh len: %d\n",udhlen);
        printf("Udh dump:");
        for(int i=1;i<=udhlen;i++)
        {
          printf(" %02X",(unsigned)body[i]);
        }
        printf("\n");
      }
      if(s.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::BINARY)
      {
        printf("Esm class: %02X\n",s.getIntProperty(Tag::SMPP_ESM_CLASS));
        unsigned char* body;
        unsigned len;
        if(s.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
        {
          body=(unsigned char*)s.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
        }else
        {
          body=(unsigned char*)s.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
        }
        if(s.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
        {
          len-=body[0]+1;
          body+=body[0]+1;
        }
        printf("Msg dump:");
        for(int i=0;i<len;i++)
        {
          printf(" %02X",(unsigned)body[i]);
        }
        printf("\n");
      }else
      if(getSmsText(&s,buf,sizeof(buf),ansi1251?CONV_ENCODING_CP1251:CONV_ENCODING_KOI8R)==-1)
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
      printf("\nReceived async submit sm resp:status=%#x, msgId=%s\n",
        pdu->get_commandStatus(),
        ((PduXSmResp*)pdu)->get_messageId());
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
           "\t -s N send sms with ussd service op set to N\n"
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
  Logger::Init();
  SmeConfig cfg;
  string host="smsc";
  int port=9001;


  sourceAddress=argv[1];

  cfg.sid=sourceAddress;
  cfg.password=cfg.sid;
  MyListener lst;

  int bindType=BindType::Transceiver;

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
        sourceAddress=optarg;
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
      case 's':
      {
        ussd=atoi(optarg);
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

  Address srcaddr;
  try{
    srcaddr=Address(sourceAddress.c_str());
  }catch(...)
  {
    fprintf(stderr,"Invalid source address\n");
    return -1;
  }
  try{

    ss.connect(bindType);

    char *addr=NULL;
    string lastAddr;
    char *message=NULL;

    using_history();

    rl_attempted_completion_function=cmd_completion;

    HISTORY_STATE cmdHist,msgHist;

    cmdHist=*history_get_history_state();
    msgHist=*history_get_history_state();


    if(!receiveOnly)
    while(!stopped)
    {
      if(addr)free(addr);
      addr=NULL;
      history_set_history_state(&cmdHist);
      addr=readline("Address or cmd>");
      if(!addr)break;
      if(!*addr)continue;
      add_history(addr);
      cmdHist=*history_get_history_state();
      string cmd=addr;
      string arg;
      splitString(cmd,arg);
      if(cmd=="quit")
      {
        break;
      }
      bool cmdFound=false;
      for(int i=0;i<commandsCount;i++)
      {
        if(cmd==commands[i].cmdname)
        {
          commands[i].cmd(ss,arg);
          cmdFound=true;
          break;
        }
      }
      if(cmdFound)continue;

      SMS s;

      s.setOriginatingAddress(srcaddr);

      __trace2__("org addr=%s",s.getOriginatingAddress().toString().c_str());

      if(cmd!="/")
      {
        try{
          Address dst((char*)addr);
          lastAddr=addr;
          s.setDestinationAddress(dst);
        }catch(...)
        {
          printf("Invalid address\n");
          continue;
        }
        if(message)free(message);
        message=NULL;
        history_set_history_state(&msgHist);
        message=readline("Enter message:");
        if(!message)break;
        add_history(message);
        msgHist=*history_get_history_state();
        rl_reset_line_state();
      }else
      {
        s.setDestinationAddress(lastAddr.c_str());
      }
      if(!message)continue;
      int len=strlen(message);

      PduSubmitSm sm;
      sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
      PduDataSm dsm;
      dsm.get_header().set_commandId(SmppCommandSet::DATA_SM);

      char msc[]="";
      char imsi[]="";
      s.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
      s.setValidTime(0);
      s.setIntProperty(Tag::SMPP_ESM_CLASS,(esmclass&(~0x3))|mode);
      s.setDeliveryReport(0);
      s.setArchivationRequested(false);
      s.setEServiceType("XXX");


      if(ussd)s.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,ussd);
      else s.messageBody.dropIntProperty(Tag::SMPP_USSD_SERVICE_OP);

      if(s.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
      {
        __trace2__("ussd=%d",s.getIntProperty(Tag::SMPP_USSD_SERVICE_OP));
      }else
      {
        __trace__("no ussd");
      }

      if(sar_num!=0)
      {
        s.setIntProperty(Tag::SMPP_SAR_MSG_REF_NUM,sar_mr);
        s.setIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS,sar_num);
        s.setIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM,sar_seq);
      }

      if(src_port!=0 && dst_port!=0)
      {
        s.setIntProperty(Tag::SMPP_SOURCE_PORT,src_port);
        s.setIntProperty(Tag::SMPP_DESTINATION_PORT,dst_port);
      }

      if(binmode)
      {
        string tmp;
        char *ptr=message;
        for(;;)
        {
          int n;
          int c;
          if(sscanf(ptr,"%02X%n",&c,&n)!=1)break;
          tmp.append(1,(char)c);
          ptr+=n;
        }
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::BINARY);
        s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),tmp.length());
        len=tmp.length();
      }else
      if(unicode)
      {
        auto_ptr<short> msg(new short[len+1]);
        ConvertMultibyteToUCS2(message,len,msg.get(),len*2,ansi1251?CONV_ENCODING_CP1251:CONV_ENCODING_KOI8R);
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
        printf("Accepted:%d bytes, msgId=%s\n",len,((PduXSmResp*)resp)->get_messageId());fflush(stdout);
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
  printf("\nUnbinding\n");
  {
    PduUnbind pdu;
    pdu.get_header().set_commandId(SmppCommandSet::UNBIND);
    pdu.get_header().set_sequenceNumber(ss.getNextSeq());
    SmppHeader *resp=tr->sendPdu((SmppHeader*)&pdu);
    if(resp)
    {
      printf("Unbind response:status=%#x\n",resp->get_commandStatus());
      disposePdu(resp);
    }else
    {
      printf("Unbind response timed out\n");
    }
  }
  ss.close();
  printf("Exiting\n");//////
  return 0;
}
