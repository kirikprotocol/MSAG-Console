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
#include "core/buffers/Array.hpp"
#include <vector>
#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include <map>

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace std;
using namespace smsc::core::buffers;
using smsc::logger::Logger;
using smsc::core::threads::Thread;

int stopped=0;
bool connected=false;

bool hexinput=false;

bool vcmode=false;

bool answerMode=false;
int answerMr;
Address answerAddress;

int maxHistorySize=100;

Hash<int> mrcache;

int getmr(const char* addr)
{
  int rv=0;
  if(mrcache.Exists(addr))
  {
    rv=++mrcache[addr];
  }else
  {
    mrcache.Insert(addr,0);
  }
  return rv;
}

struct MrKey{
  string addr;
  int mr;
  MrKey():addr(),mr(0){}
  MrKey(const char* a,int m):addr(a),mr(m){}
  MrKey(const MrKey& k)
  {
    addr=k.addr;
    mr=k.mr;
  }
  bool operator=(const MrKey& k)const
  {
    return addr==k.addr && mr==k.mr;
  }
  bool operator<(const MrKey& k)const
  {
    return addr<k.addr?true:
           addr==k.addr?mr<k.mr:false;
  }
};

typedef map<MrKey,bool*> MrMap;

Mutex mrMtx;
MrMap mrStore;

struct VClientData{


  int temperrProb;
  int dontrespProb;
  int permErrProb;
  int respDelayMin;
  int respDelayMax;
  int mode;
  bool dataSm;

  int dataCoding;

  int ussd;
  int esmclass;
  int sar_mr;
  int sar_num;
  int sar_seq;
  int src_port;
  int dst_port;

  int respStatus;

  string sourceAddress;

  FILE *cmdfile;

  bool waitRespMode;
  bool waitResp;
  int lastMr;
  int waitRespTimeout;
  time_t waitStart;

  bool active;
  int repeatCnt;
  time_t execTime;

  uint32_t sleepTill;
  uint32_t sleepTillMsec;

  VClientData()
  {
    Init();
  }
  void Init()
  {
    temperrProb=0;
    dontrespProb=0;
    permErrProb=0;
    respDelayMin=0;
    respDelayMax=0;
    mode=0;
    dataSm=false;
    ussd=0;
    dataCoding=DataCoding::LATIN1;
    esmclass=0;
    sar_mr=0;
    sar_num=0;
    sar_seq=0;
    src_port=0;
    dst_port=0;
    cmdfile=0;
    waitResp=false;
    lastMr=0;
    waitRespMode=false;
    waitRespTimeout=10;
    active=false;
    repeatCnt=0;
    execTime=0;
    sleepTill=0;
    sleepTillMsec=0;
    waitStart=0;
    respStatus=0;
  }

}defVC;

int& temperrProb=defVC.temperrProb;
int& dontrespProb=defVC.dontrespProb;
int& permErrProb=defVC.permErrProb;
int& respDelayMin=defVC.respDelayMin;
int& respDelayMax=defVC.respDelayMax;
int& mode=defVC.mode;
bool& dataSm=defVC.dataSm;

int& dataCoding=defVC.dataCoding;

int& ussd=defVC.ussd;
int& esmclass=defVC.esmclass;
int& sar_mr=defVC.sar_mr;
int& sar_num=defVC.sar_num;
int& sar_seq=defVC.sar_seq;
int& src_port=defVC.src_port;
int& dst_port=defVC.dst_port;
bool& waitRespMode=defVC.waitRespMode;
bool& waitResp=defVC.waitResp;
int& waitRespTimeout=defVC.waitRespTimeout;
int& lastMr=defVC.lastMr;

int& respStatus=defVC.respStatus;

string& sourceAddress=defVC.sourceAddress;

FILE*& cmdfile=defVC.cmdfile;

bool ansi1251=false;
bool cmdecho=false;

struct Option{
  const char* name;
  char type;
  void* addr;
  int& asInt(){return *(int*)addr;}
  bool& asBool(){return *(bool*)addr;}
  string& asString(){return *(string*)addr;}
};


bool autoAnswer=false;

Option options[]={
{"temperr",'i',&temperrProb},
{"noresp",'i',&dontrespProb},
{"permerr",'i',&permErrProb},
{"respmindelay",'i',&respDelayMin},
{"respmaxdelay",'i',&respDelayMax},
{"mode",'m',&mode},
{"datasm",'b',&dataSm},
{"ussd",'i',&ussd},
{"ansi1251",'b',&ansi1251},
{"srcport",'i',&src_port},
{"dstport",'i',&dst_port},
{"datacoding",'d',&dataCoding},
{"esmclass",'i',&esmclass},
{"source",'s',&sourceAddress},
{"echo",'b',&cmdecho},
{"waitresp",'b',&waitRespMode},
{"wrtimeot",'i',&waitRespTimeout},
{"autoanswer",'b',&autoAnswer},
{"hexinput",'b',&hexinput},
{"respstatus",'i',&respStatus},
};

const int optionsCount=sizeof(options)/sizeof(Option);




/*
static bool splitString(string& head,string& tail)
{
  string::size_type pos=head.find(' ');
  if(pos==string::npos)return false;
  int firstPos=pos;
  while(head[pos]==' ')pos++;
  tail=head.substr(pos);
  head.erase(firstPos);
  return true;
}
*/

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


struct DataCodingSchema{
  string name;
  int    value;
  const char* comment;
};

DataCodingSchema dcs[]=
{
  {"default",DataCoding::SMSC7BIT,"GSM 7 meaningful bits in 8 bit chars, 8th bit is zero"},
  {"ucs2",DataCoding::UCS2,"UCS2, by default converted from KOI8-R, see ansi1251"},
  {"bin",DataCoding::BINARY,"Binary data. Binary dump expected as input."},
  {"latin1",DataCoding::LATIN1,"Latin1 - ASCII data coding. 8th bit ignored"},
};

int findDcsByName(const char* name)
{
  for(int i=0;i<sizeof(dcs)/sizeof(dcs[0]);i++)
  {
    if(dcs[i].name==name)return i;
  }
  return -1;
}

int findDcsByValue(int val)
{
  for(int i=0;i<sizeof(dcs)/sizeof(dcs[0]);i++)
  {
    if(dcs[i].value==val)return i;
  }
  return -1;
}

static void ShowOption(Option& opt,bool singleopt=false)
{
  printf("%s=",opt.name);
  switch(opt.type)
  {
    case 'i':printf("%d\n",opt.asInt());break;
    case 'b':printf("%s\n",opt.asBool()?"yes":"no");break;
    case 'm':
    {
      printf("%s\n",modes[opt.asInt()]);
      if(singleopt)
      {
        printf("Available modes:\n");
        for(int i=0;i<sizeof(modes)/sizeof(modes[0]);i++)printf("  %s\n",modes[i]);
      }
    }break;
    case 'd':
    {
      int idx=findDcsByValue(opt.asInt());
      if(idx!=-1)
      {
        printf("%s - %s\n",dcs[idx].name.c_str(),dcs[idx].comment);
      }
      if(singleopt)
      {
        printf("Avialable datacoding schemes:\n");
        for(int i=0;i<sizeof(dcs)/sizeof(dcs[0]);i++)
        {
          printf("  %s - %s\n",dcs[i].name.c_str(),dcs[i].comment);
        }
      }
    }break;
    case 's':printf("%s\n",opt.asString().c_str());break;
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
        ShowOption(options[i],true);
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
        case 'd':
        {
          int idx=findDcsByName(val.c_str());
          if(idx!=-1)
          {
            dataCoding=dcs[idx].value;
          }else
          {
            printf("Error: unknown dcs %s\n",val.c_str());
          }
        }break;
        case 's':options[i].asString()=val;break;
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
    printf("Sar off\n");
    return;
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

void SleepCmd(SmppSession& ss,const string& args)
{
  int sec=1,msec=0;
  if(args.length())
  {
    if(sscanf(args.c_str(),"%d.%d",&sec,&msec)==1)
    {
      msec=sec;
      sec=0;
    };
  }
  sec+=msec/1000;
  msec=msec%1000;
  if(vcmode)
  {
    timeval tv;
    gettimeofday(&tv,0);
    defVC.sleepTill=tv.tv_sec+sec;
    defVC.sleepTillMsec=tv.tv_usec/1000+msec;
    if(defVC.sleepTillMsec>1000)
    {
      defVC.sleepTillMsec-=1000;
      defVC.sleepTill++;
    }
    return;
  }
  timespec ts;
  ts.tv_sec=sec;
  ts.tv_nsec=msec*1000000;
  nanosleep(&ts,0);
}

Mutex hstMtx;
typedef map<int,pair<int,Address> > HistoryMap;
HistoryMap historyMap;
int historyCnt=0;

void Answer(SmppSession& ss,const string& args)
{
  MutexGuard g(hstMtx);
  if(historyMap.size()==0)return;
  int idx=args.length()>0?atoi(args.c_str()):(--historyMap.end())->first;
  HistoryMap::iterator it=historyMap.find(idx);
  if(it==historyMap.end())
  {
    printf("Message %d not found in history\n",idx);
    return;
  }
  answerMode=true;
  answerMr=it->second.first;
  answerAddress=it->second.second;
}

CmdRec commands[]={
{"query",QueryCmd},
{"eq",EnquireLinkCmd},
{"replace",ReplaceCmd},
{"cancel",CancelCmd},
{"set",SetOption},
{"sar",SetSar},
{"help",ShowHelp},
{"connect",0},
{"disconnect",0},
{"quit",0},
{"sleep",SleepCmd},
{"virtualclients",0},
{"answer",Answer},
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

extern "C" char** cmd_completion(const char *text,int start,int end)
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

class ResponseProcessor:public Thread{
  Array<SmppHeader*> queue;
  EventMonitor mon;
  SmppTransmitter* trans;
  SmppTransmitter* atrans;

public:
  int Execute()
  {
    mon.Lock();
    while(!stopped)
    {
      if(queue.Count()==0)mon.wait();
      if(!queue.Count())continue;
      SmppHeader* pdu;
      queue.Shift(pdu);
      mon.Unlock();
      Process(pdu);
      mon.Lock();
      disposePdu(pdu);
    }
    mon.Unlock();
    return 0;
  }
  void Process(SmppHeader* pdu)
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
        return;
      }else if(rnd<temperrProb+permErrProb+dontrespProb)
      {
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
      int smsNum;
      {
        MutexGuard g(hstMtx);
        while(historyMap.size()>maxHistorySize)historyMap.erase(historyMap.begin());
        smsNum=historyCnt++;
        int mr=s.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)?s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE):-1;
        historyMap.insert(pair<int,pair<int,Address> > (smsNum,pair<int,Address>(mr,s.getOriginatingAddress())));
      }
      printf("\n===== %d =====\nFrom:%s\n",smsNum,buf);
      s.getDestinationAddress().toString(buf,sizeof(buf));
      printf("To:%s\n",buf);
      printf("DCS:%d\n",s.getIntProperty(Tag::SMPP_DATA_CODING));
      if(s.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE))
      {
        printf("UMR:%d\n",s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
        MutexGuard g(mrMtx);
        MrMap::iterator m=mrStore.find
          (
            MrKey
            (
              s.getDestinationAddress().toString().c_str(),
              s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)
            )
          );
        if(m!=mrStore.end())
        {
          *(m->second)=false;
        }
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
        printf("Msg dump(%d):",len);
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
      if(respDelayMin || respDelayMax)
      {
        double r=rand()/RAND_MAX;
        int delay=respDelayMin+(respDelayMax-respDelayMin)*r;
        int sec=delay/1000;
        int msec=delay%1000;
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
        resp.get_header().set_commandStatus(respStatus);
        resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
        resp.set_messageId("");
        resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        atrans->sendDeliverySmResp(resp);
      }else
      {
        PduDataSmResp resp;
        resp.get_header().set_commandStatus(respStatus);
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
        ((PduXSmResp*)pdu)->get_messageId()?((PduXSmResp*)pdu)->get_messageId():"NULL");
    }
    if(!cmdfile && !vcmode)rl_forced_update_display();
  }
  void enqueue(SmppHeader* pdu)
  {
    mon.Lock();
    queue.Push(pdu);
    mon.notify();
    mon.Unlock();
  }
  void setTrans(SmppTransmitter *t,SmppTransmitter *at)
  {
    trans=t;
    atrans=at;
  }
  void Notify()
  {
    mon.Lock();
    mon.notify();
    mon.Unlock();
  }
};

class MyListener:public SmppPduEventListener{
public:
  MyListener(ResponseProcessor& rp):respProcessor(rp)
  {
  }
  void handleEvent(SmppHeader *pdu)
  {
    respProcessor.enqueue(pdu);
  }
  void handleError(int errorCode)
  {
    printf("Network error!\n");
    fflush(stdout);
    connected=false;
  }

protected:
  ResponseProcessor& respProcessor;
};


void trimend(char* buf)
{
  int l=strlen(buf)-1;
  while(l>=0 && isspace(buf[l]))l--;
  buf[l+1]=0;
}

struct PendingVC{
  string src;
  string fn;
  int repeat;
  time_t timelimit;
};

int main(int argc,char* argv[])
{
  using_history ();

  if(argc==1)
  {
    printf("usage: %s systemid [options] @cmdfile\n",argv[0]);
    printf("Options are:\n");
    printf("\t -h host[:port] (default == smsc:9001)\n"
           "\t -a addr (defauld == sid)\n"
           "\t -p password (default == sid)\n"
           "\t -t bind type (tx,rx,trx)\n"
           "\t -d send messages as DATA_SM\n"
           "\t -s N send sms with ussd service op set to N\n"
           "\t -w autoanswer(ping pong) mode\n"
           "\t -c receive only mode\n"
           "\t -m {D|T} send messages in datagram or transaction mode\n"
           "\t -e N probability of answer with temp error\n"
           "\t -r N probability of answer with perm error\n"
           "\t -n N probability of not answering at all\n"
           "\t -l Min[:Max] delay between deliver and response (random between min and max)\n"
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
  ResponseProcessor rp;
  MyListener lst(rp);
  rp.Start();

  int bindType=BindType::Transceiver;

  bool receiveOnly=false;

  string cmdFileName;

  for(int i=2;i<argc;i+=2)
  {
    char *opt=argv[i];
    if(opt[0]=='@')
    {
      cmdFileName=opt+1;
      continue;
    }
    if(opt[0]!='-' && opt[0]!='/')
    {
      printf("Unrecognized command line argument: %s\n",opt);
      return -1;
    }
    switch(opt[1])
    {
      case 'u':
      {
        dataCoding=DataCoding::UCS2;
        i--;
      }continue;
      case '7':
      {
        dataCoding=DataCoding::SMSC7BIT;
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
        if(strchr(optarg,':'))
        {
          sscanf(optarg,"%d:%d",&respDelayMin,&respDelayMax);
        }else
        {
          respDelayMin=respDelayMax=atoi(optarg);
        }
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

  rp.setTrans(tr,ss.getAsyncTransmitter());

  Address srcaddr;
  try{
    srcaddr=Address(sourceAddress.c_str());
  }catch(...)
  {
    fprintf(stderr,"Invalid source address\n");
    return -1;
  }

  try{

    try{
      ss.connect(bindType);
      connected=true;
    }catch(SmppConnectException& e)
    {
      printf("Connect error:%s\n",e.getTextReason());
    }

    char *addr=NULL;
    string lastAddr;
    char *message=NULL;

    using_history();

    rl_attempted_completion_function=cmd_completion;

    HISTORY_STATE cmdHist,msgHist;

    cmdHist=*history_get_history_state();
    msgHist=*history_get_history_state();

    cmdfile=0;
    if(cmdFileName.length()>0)
    {
      cmdfile=fopen(cmdFileName.c_str(),"rt");
      if(!cmdfile)
      {
        printf("Faield to open command file:%s\n",cmdFileName.c_str());
      }
    }
    char fileBuf[65536];

    VClientData *vc=0;
    int vccnt=0;
    int vcused=0;
    int vcidx=0;

    vector<PendingVC> pvc;

    if(!receiveOnly)
    while(!stopped)
    {
      if(vcmode)
      {
        int acnt=0;
        for(int x=0;x<vccnt;x++)
        {
          if(!vc[x].active)continue;
          if(vc[x].waitResp)continue;
          if(vc[x].sleepTill)continue;
          acnt++;
        }
        if(acnt==0)
        {
          timestruc_t tv={0,10*1000000};
          nanosleep(&tv,0);
        }
        vcidx++;
        if(vcidx>=vccnt)vcidx=0;
        while(vcused<vccnt && pvc.size())
        {
          PendingVC p=pvc.front();
          pvc.erase(pvc.begin());
          int idx=0;
          while(vc[idx].active)idx++;
          printf("%02d: new client %s from %s\n",idx,p.src.c_str(),p.fn.c_str());
          vc[idx].cmdfile=fopen(p.fn.c_str(),"rb");
          if(!vc[idx].cmdfile)
          {
            printf("%02d: failed to open file %s, skipping\n",p.fn.c_str());
            continue;
          }
          vc[idx].sourceAddress=p.src;
          vc[idx].repeatCnt=p.repeat;
          vc[idx].execTime=time(NULL)+p.timelimit;
          vc[idx].active=true;
          vcused++;
        }
        if(vcused==0)break;
        if(!vc[vcidx].active)continue;
        defVC=vc[vcidx];
        if(waitRespMode)
        {
          if(waitResp && vc[vcidx].waitStart+waitRespTimeout<time(NULL))
          {
            continue;
          }
          if(waitResp)
          {
            printf("Response for %s timed out!\n",sourceAddress.c_str());
            waitResp=false;
            MutexGuard g(mrMtx);
            MrMap::iterator i=mrStore.find(MrKey(sourceAddress.c_str(),lastMr));
            if(i!=mrStore.end())mrStore.erase(i);
          }
        }
        if(vc[vcidx].sleepTill!=0)
        {
          timeval tv;
          gettimeofday(&tv,0);
          if(vc[vcidx].sleepTill>tv.tv_sec ||
             (vc[vcidx].sleepTill==tv.tv_sec && vc[vcidx].sleepTillMsec>tv.tv_usec/1000))continue;
          vc[vcidx].sleepTill=0;
          vc[vcidx].sleepTillMsec=0;
        }
      }else
      {

        if(waitRespMode)
        {
          time_t waitstart=time(NULL);
          while(waitResp && time(NULL)-waitstart<waitRespTimeout)
          {
            timespec ts;
            ts.tv_sec=0;
            ts.tv_nsec=10000000;
            nanosleep(&ts,0);
          }
          if(waitResp)
          {
            printf("Response timed out!\n");
            waitResp=false;
            MutexGuard g(mrMtx);
            MrMap::iterator i=mrStore.find(MrKey(sourceAddress.c_str(),lastMr));
            if(i!=mrStore.end())mrStore.erase(i);
          }
        }
      }

      if(cmdfile)
      {
        if(!fgets(fileBuf,sizeof(fileBuf),cmdfile))
        {
          if(vcmode)
          {
            if(vc[vcidx].repeatCnt)
            {
              vc[vcidx].repeatCnt--;
              if(vc[vcidx].repeatCnt==0)
              {
                printf("%02d:finished\n",vcidx);
                fclose(cmdfile);
                vc[vcidx].Init();
                vcused--;
              }else
              {
                fseek(cmdfile,0,SEEK_SET);
              }
            }else if(vc[vcidx].execTime)
            {
              if(vc[vcidx].execTime<=time(NULL))
              {
                fseek(cmdfile,0,SEEK_SET);
              }else
              {
                printf("%02d:finished\n",vcidx);
                fclose(cmdfile);
                vc[vcidx].Init();
                vcused--;
              }
            }else
            {
              fseek(cmdfile,0,SEEK_SET);
            }
          }else
          {
            fclose(cmdfile);
            addr=0;
            message=0;
            cmdfile=0;
          }
          continue;
        }
        trimend(fileBuf);
        addr=fileBuf;
        if(cmdecho)
        {
          if(vcmode)
          {
            printf("%02d:Address or cmd>%s\n",vcidx,fileBuf);
          }else
          {
            printf("Address or cmd>%s\n",fileBuf);
          }
        }
      }else
      {
        if(vcmode)
        {
          printf("something goes wrong... interactive mode not allowed during virtualclients\n");
          break;
        }
        if(addr)free(addr);
        addr=NULL;
        history_set_history_state(&cmdHist);
          addr=readline("Address or cmd>");
        if(!addr)break;
        if(!*addr)continue;
        add_history(addr);
        cmdHist=*history_get_history_state();
      }
      string cmd=addr;
      string arg;
      splitString(cmd,arg);
      if(cmd=="quit")
      {
        break;
      }

      if(cmd=="virtualclients")
      {
        if(vcmode)
        {
          printf("Command virtualclients is not suppoerted in virtual clients mode!!!\n");
          continue;
        }
        int cnt=10;
        if(arg.find(' ')!=string::npos)
        {
          string n;
          splitString(arg,n);
          cnt=atoi(n.c_str());
          if(cnt<=0 || cnt >256)
          {
            printf("%s is not valid number of active virtual clients\n",n.c_str());
            break;
          }
        }
        FILE *f=fopen(arg.c_str(),"rb");
        if(!f)
        {
          printf("Failed to open virtualclients script:%s\n",arg.c_str());
          continue;
        }

        if(cmdfile){fclose(cmdfile);cmdfile=0;}
        addr=0;
        message=0;
        cmdfile=0;

        while(fgets(fileBuf,sizeof(fileBuf),f))
        {
          trimend(fileBuf);
          string src=fileBuf;
          string file,rep;
          splitString(src,file);
          splitString(file,rep);
          int rc=0;
          time_t rt=0;
          if(rep.length() && rep[0]=='*')
          {
            rt=atoi(rep.c_str()+1);
          }else if(rep.length())
          {
            rc=atoi(rep.c_str());
          }

          PendingVC p={src,file,rc,rt};
          pvc.push_back(p);
        }

        vc=new VClientData[cnt];
        vccnt=cnt;
        vcmode=true;
        continue;
      }

      if(cmd=="connect")
      {
        if(connected)
        {
          printf("Sme already connected, type disconnect first\n");
          continue;
        }
        try{
          ss.close();
          ss.connect(bindType);
          tr=ss.getSyncTransmitter();
          rp.setTrans(tr,ss.getAsyncTransmitter());
        }catch(SmppConnectException& e)
        {
          printf("Connect error:%s\n",e.getTextReason());
          continue;
        }
        connected=true;
        printf("Connect ok\n");
        continue;
      }

      if(cmd=="disconnect")
      {
        if(!connected)
        {
          printf("Sme already disconnect, connect it first\n");
          continue;
        }
        ss.close();
        printf("Disconnected\n");
        connected=false;
        continue;
      }

      if(cmd[0]=='@')
      {
        string fn=cmd.substr(1);
        if(cmdfile)fclose(cmdfile);
        cmdfile=fopen(fn.c_str(),"rt");
        if(!cmdfile)
        {
          printf("Failed to open file:%s(%s)\n",fn.c_str(),strerror(errno));
        }else
        {
          if(addr)free(addr);
          if(message)free(message);
          addr=0;
          message=0;
        }
        continue;
      }

      bool cmdFound=false;
      for(int i=0;i<commandsCount;i++)
      {
        if(cmd==commands[i].cmdname)
        {
          if(commands[i].cmd)commands[i].cmd(ss,arg);
          cmdFound=true;
          break;
        }
      }
      if(cmdFound && !answerMode)
      {
        if(vcmode)vc[vcidx]=defVC;
        continue;
      }

      SMS s;

      if(answerMode)
      {
        if(answerMr!=-1)
          s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,answerMr);
      }

      try{
        srcaddr=Address(sourceAddress.c_str());
      }catch(...)
      {
        if(vcmode)printf("%02d:",vcidx);
        printf("Invalid source address:%s\n",sourceAddress.c_str());
        continue;
      }

      s.setOriginatingAddress(srcaddr);

      __trace2__("org addr=%s",s.getOriginatingAddress().toString().c_str());

      if(cmd!="/")
      {
        if(answerMode)
        {
          s.setDestinationAddress(answerAddress);
          lastAddr=answerAddress.toString();
          answerMode=false;
        }else
        {
          try{
            Address dst((char*)addr);
            lastAddr=addr;
            s.setDestinationAddress(dst);
          }catch(...)
          {
            if(vcmode)printf("%02d:",vcidx);
            printf("Invalid address:%s\n",addr);
            continue;
          }
        }
        if(cmdfile)
        {
          fgets(fileBuf,sizeof(fileBuf),cmdfile);
          trimend(fileBuf);
          message=fileBuf;
          if(cmdecho)
          {
            if(vcmode)printf("%02d:",vcidx);
            printf("Enter message:%s\n",fileBuf);
          }
        }else
        {
          if(message)free(message);
          message=NULL;
          history_set_history_state(&msgHist);
          message=readline("Enter message:");
          if(!message)break;
          add_history(message);
          msgHist=*history_get_history_state();
          rl_reset_line_state();
        }
      }else
      {
        s.setDestinationAddress(lastAddr.c_str());
      }
      if(!message)continue;

      if(!connected)
      {
        printf("Sme not connected, type connect first\n");
        continue;
      }

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
      s.setEServiceType("TEST");


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

      /*
      string hextmp;
      char *hexsave=0;
      if(hexinput && dataCoding!=DataCoding::BINARY)
      {
        char *ptr=message;
        for(;;)
        {
          int n;
          int c;
          if(sscanf(ptr,"%02X%n",&c,&n)!=1)break;
          hextmp.append(1,(char)c);
          ptr+=n;
        }
        hexsave=message;
        message=(char*)hextmp.c_str();
        len=hextmp.length();
      }
      */

      if(dataCoding==DataCoding::BINARY || hexinput)//binary
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
        s.setIntProperty(Tag::SMPP_DATA_CODING,dataCoding);
        s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),tmp.length());
        len=tmp.length();
      }else
      if(dataCoding==DataCoding::UCS2)//UCS2
      {
        auto_ptr<short> msg(new short[len+1]);
        ConvertMultibyteToUCS2(message,len,msg.get(),len*2,ansi1251?CONV_ENCODING_CP1251:CONV_ENCODING_KOI8R);
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::UCS2);
        len*=2;
        s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,(char*)msg.get(),len);
      }else if(dataCoding==DataCoding::SMSC7BIT)
      {
        auto_ptr<char> msg(new char[len*3+1]);
        len=ConvertLatin1ToSMSC7Bit(message,len,msg.get());
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::SMSC7BIT);
        s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,msg.get(),len);
      }else //latin1
      {
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
        s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,message,len);
      }

      /*
      if(hexinput)
      {
        message=hexsave;
      }
      */

      if(waitRespMode)
      {
        int mr;
        if(!s.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE))
        {
          mr=getmr(sourceAddress.c_str());
          s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,mr);
          printf("Set mr to %d\n",mr);
        }else
        {
          mr=s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
        }
        lastMr=mr;
        MutexGuard g(mrMtx);
        if(vcmode)
        {
          vc[vcidx].waitResp=true;
          vc[vcidx].waitStart=time(NULL);
          mrStore.insert(make_pair(MrKey(Address(sourceAddress.c_str()).toString().c_str(),mr),&vc[vcidx].waitResp));
        }else
        {
          waitResp=true;
          mrStore.insert(make_pair(MrKey(Address(sourceAddress.c_str()).toString().c_str(),mr),&waitResp));
        }
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
        if(vcmode)
        {
          printf("%02d:",vcidx);
        }
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
  catch(std::exception& e)
  {
    printf("Exception: %s\n",e.what());
  }
  catch(...)
  {
    printf("unknown exception\n");
  }
  if(connected)
  {
    printf("\nUnbinding\n");
    try
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
    }catch(...)
    {
      printf("Exception during unbind\n");
    }
  }
  ss.close();
  stopped=1;
  rp.Notify();
  printf("Exiting\n");//////
  return 0;
}
