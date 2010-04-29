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
#include <set>
#include "system/status.h"
#include "util/sleep.h"
#include "core/buffers/File.hpp"

#ifdef linux
typedef timespec timestruc_t;
#endif

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace std;
using namespace smsc::core::buffers;
using smsc::logger::Logger;
using smsc::core::threads::Thread;

FILE* cmdFile=stdout;
FILE* incomFile=stdout;

void CmdOut(const char* fmt,...)
{
  if(!cmdFile)return;
  va_list args;
  va_start (args, fmt);
  vfprintf(cmdFile,fmt,args);
  fflush(cmdFile);
  va_end (args);
}

void IncomOut(const char* fmt,...)
{
  if(!incomFile)return;
  va_list args;
  va_start (args, fmt);
  vfprintf(incomFile,fmt,args);
  fflush(incomFile);
  va_end (args);
}

int stopped=0;
bool connected=false;

bool silent=false;

bool vcmode=false;

int vcmodemaxspeed=0;

int temperrProb=0;
int dontrespProb=0;
int permErrProb=0;
int temperrProbDataSm=0;
int dontrespProbDataSm=0;
int permErrProbDataSm=0;

int respDelayMin;
int respDelayMax;


bool answerMode=false;
int answerMr;
Address answerAddress;

std::set<Address> tempBlock;
int tempBlockMode=2;
std::set<Address> permBlock;


template <class T,int N>
int GetArraySize(const T (&arr)[N])
{
  return N;
}

int tempErrorsDefault[]=
{
  SmppStatusSet::ESME_RMSGQFUL,
  SmppStatusSet::ESME_RTHROTTLED,
  SmppStatusSet::ESME_RX_T_APPN
};
std::vector<int> tempErrors(tempErrorsDefault,tempErrorsDefault+GetArraySize(tempErrorsDefault));
int permErrorsDefault[]=
{
  SmppStatusSet::ESME_RX_P_APPN,
  SmppStatusSet::ESME_RUNKNOWNERR
};
std::vector<int> permErrors(permErrorsDefault,permErrorsDefault+GetArraySize(permErrorsDefault));

std::string vcprefixtemplate="$2i";

int maxHistorySize=100;

struct SmppOptionalTag{
  uint16_t tag;
  std::vector<uint8_t> data;
};

typedef std::map<int,SmppOptionalTag> OptionalTagsMap;
OptionalTagsMap optionalTags;
int lastTagId=0;

template <class PDU>
void AddOptionals(PDU& pdu)
{
  if(optionalTags.empty())return;
  std::vector<uint8_t> dump;
  for(OptionalTagsMap::iterator it=optionalTags.begin();it!=optionalTags.end();it++)
  {
    uint16_t tag=htons(it->second.tag);
    dump.insert(dump.end(),(uint8_t*)&tag,(uint8_t*)&tag+2);
    uint16_t len=htons((uint16_t)it->second.data.size());
    dump.insert(dump.end(),(uint8_t*)&len,(uint8_t*)&len+2);
    dump.insert(dump.end(),it->second.data.begin(),it->second.data.end());
  }
  CmdOut("Dump:");
  for(int i=0;i<dump.size();i++)CmdOut("%02x",dump[i]);
  CmdOut("\n");
  pdu.get_optional().set_unknownFields((char*)&dump[0],(int)dump.size());
}

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
  int vcidx;
  MrKey():addr(),mr(0){}
  MrKey(const char* a,int m,int idx=0):addr(a),mr(m),vcidx(idx){}
  MrKey(const MrKey& k)
  {
    addr=k.addr;
    mr=k.mr;
    vcidx=k.vcidx;
  }
  bool operator==(const MrKey& k)const
  {
    return addr==k.addr && mr==k.mr;
  }
  bool operator<(const MrKey& k)const
  {
    return addr<k.addr?true:
           addr==k.addr?mr<k.mr:false;
  }
};

typedef map<MrKey,int*> MrMap;

Mutex mrMtx;
MrMap mrStore;

struct VClientData{


  int mode;
  bool dataSm;

  int dataCoding;

  bool hexinput;

  int ussd;
  int esmclass;
  int sar_mr;
  int sar_num;
  int sar_seq;
  int src_port;
  int dst_port;

  int umr;
  bool umrPresent;

  int validTime;

  bool replaceIfPresent;

  int respStatus;

  string sourceAddress;

  string eservicetype;

  File *cmdfile;

  bool waitRespMode;
  int wrCount;
  int waitResp;
  bool waitRespNoUMR;
  int lastMr;
  int waitRespTimeout;
  time_t waitStart;

  bool restartOnError;

  bool active;
  int repeatCnt;
  time_t execTime;

  uint32_t sleepTill;
  uint32_t sleepTillMsec;

  bool setDpf;

  VClientData()
  {
    Init();
  }
  void Init()
  {
    mode=0;
    dataSm=false;
    ussd=-1;
    dataCoding=DataCoding::LATIN1;
    esmclass=0;
    restartOnError=true;
    sar_mr=0;
    sar_num=0;
    sar_seq=0;
    src_port=0;
    dst_port=0;
    cmdfile=0;
    waitResp=0;
    wrCount=1;
    lastMr=0;
    waitRespMode=false;
    waitRespNoUMR=false;
    waitRespTimeout=10;
    active=false;
    repeatCnt=0;
    execTime=0;
    sleepTill=0;
    sleepTillMsec=0;
    waitStart=0;
    respStatus=0;
    validTime=0;
    replaceIfPresent=false;
    eservicetype="TEST";
    setDpf=false;
    umr=0;
    umrPresent=false;
    hexinput=false;
  }

}defVC;

VClientData *vcarray=0;

int& mode=defVC.mode;
bool& dataSm=defVC.dataSm;

int& dataCoding=defVC.dataCoding;
bool& hexinput=defVC.hexinput;

int& ussd=defVC.ussd;
int& esmclass=defVC.esmclass;
int& sar_mr=defVC.sar_mr;
int& sar_num=defVC.sar_num;
int& sar_seq=defVC.sar_seq;
int& src_port=defVC.src_port;
int& dst_port=defVC.dst_port;
bool& waitRespMode=defVC.waitRespMode;
bool& waitRespNoUMR=defVC.waitRespNoUMR;
int& waitResp=defVC.waitResp;
int& wrCount=defVC.wrCount;
int& waitRespTimeout=defVC.waitRespTimeout;
int& lastMr=defVC.lastMr;

bool& restartOnError=defVC.restartOnError;

int& validTime=defVC.validTime;

bool& replaceIfPresent=defVC.replaceIfPresent;

int& respStatus=defVC.respStatus;

string& sourceAddress=defVC.sourceAddress;
string& eservicetype=defVC.eservicetype;

int& umr=defVC.umr;
bool& umrPresent=defVC.umrPresent;


bool& setDpf=defVC.setDpf;

File*& cmdfile=defVC.cmdfile;

bool ansi1251=false;
bool cmdecho=false;

bool asyncsend=false;

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
{"temperrdatasm",'i',&temperrProbDataSm},
{"norespdatasm",'i',&dontrespProbDataSm},
{"permerrdatasm",'i',&permErrProbDataSm},
{"respmindelay",'i',&respDelayMin},
{"respmaxdelay",'i',&respDelayMax},
{"mode",'m',&mode},
{"datasm",'b',&dataSm},
{"ussd",'i',&ussd},
{"ansi1251",'b',&ansi1251},
{"srcport",'i',&src_port},
{"dstport",'i',&dst_port},
{"datacoding",'d',&dataCoding},
{"blockmode",'l',&tempBlockMode},
{"esmclass",'i',&esmclass},
{"source",'s',&sourceAddress},
{"echo",'b',&cmdecho},
{"waitresp",'b',&waitRespMode},
{"wrcount",'i',&wrCount},
{"waitrespnoumr",'b',&waitRespNoUMR},
{"wrtimeout",'i',&waitRespTimeout},
{"autoanswer",'b',&autoAnswer},
{"hexinput",'b',&hexinput},
{"respstatus",'i',&respStatus},
{"silent",'b',&silent},
{"validTime",'i',&validTime},
{"replaceIfPresent",'b',&replaceIfPresent},
{"eservicetype",'s',&eservicetype},
{"setDpf",'b',&setDpf},
{"asyncsend",'b',&asyncsend},
{"vcprefixtemplate",'s',&vcprefixtemplate},
{"vcmodemaxspeed",'i',&vcmodemaxspeed},
{"restartonerror",'b',&restartOnError}
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
    CmdOut("Usage: query msgId\n");
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
      CmdOut("Query result:commandStatus=%#x(%d), messageState=%d, errorCode=%d, finalDate=%s\n",
        qresp->get_header().get_commandStatus(),qresp->get_header().get_commandStatus(),
        qresp->get_messageState(),
        qresp->get_errorCode(),
        qresp->get_finalDate()?qresp->get_finalDate():"NULL"
      );
    disposePdu((SmppHeader*)qresp);
  }
  else
  {
    CmdOut("Query timed out\n");
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
    CmdOut("Enquire link ok\n");
    disposePdu(resp);
  }else
  {
    CmdOut("No response\n");
  }
}

void ReplaceCmd(SmppSession& ss,const string& args)
{
  PduReplaceSm r;
  string id=args;
  string msg;
  if(!splitString(id,msg))
  {
    CmdOut("Usage: replace msgId newmessage\n");
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
    CmdOut("Replace status:%#x(%d)\n",replresp->get_header().get_commandStatus(),replresp->get_header().get_commandStatus());
    disposePdu((SmppHeader*)replresp);
  }
  else
  {
    CmdOut("Replace timed out\n");
  }
}

void CancelCmd(SmppSession& ss,const string& args)
{
  if(args.length()==0)
  {
    CmdOut("Usage:\ncancel msgId\nor\ncancel orgaddr dstaddr svctype\n");
    return;
  }
  PduCancelSm q;
  if(args.find(' ')!=string::npos)
  {
    string org,dst,svc;
    org=args;
    splitString(org,dst);
    if(!splitString(dst,svc))
    {
      CmdOut("Invalid arguments count\n");
      return;
    }
    Address addr(org.c_str());
    q.get_source().set_typeOfNumber(addr.type);
    q.get_source().set_numberingPlan(addr.plan);
    q.get_source().set_value(addr.value);
    addr=Address(dst.c_str());
    q.get_dest().set_typeOfNumber(addr.type);
    q.get_dest().set_numberingPlan(addr.plan);
    q.get_dest().set_value(addr.value);
    q.set_serviceType(svc.c_str());
  }else
  {
    q.set_messageId(args.c_str());
    Address addr(sourceAddress.c_str());
    q.get_source().set_typeOfNumber(addr.type);
    q.get_source().set_numberingPlan(addr.plan);
    q.get_source().set_value(addr.value);
  }



  /*
  q.set_serviceType("XXX");
  q.get_dest().set_typeOfNumber(s.getDestinationAddress().type);
  q.get_dest().set_numberingPlan(s.getDestinationAddress().plan);
  q.get_dest().set_value(s.getDestinationAddress().value);
  //printf("Cancelling:%s\n",resp->get_messageId());*/
  SmppTransmitter* tr=ss.getAsyncTransmitter();
  PduCancelSmResp *cresp=tr->cancel(q);
  if(cresp)
  {
    CmdOut("Cancel result:%d\n",cresp->get_header().get_commandStatus());
    disposePdu((SmppHeader*)cresp);
  }
  else
  {
    CmdOut("Cancel timedout\n");
  }
}

void SetUmrCommand(SmppSession& ss,const string& args)
{
  if(args.length()==0)
  {
    CmdOut("Usage: setumr {umr|off}\n");
    return;
  }
  if(args=="no" || args=="off")
  {
    umrPresent=false;
    CmdOut("Umr turned off\n");
    return;
  }
  umr=atoi(args.c_str());
  umrPresent=true;
  CmdOut("umr set to %d\n",umr);
}

void addTempBlock(SmppSession& ss,const string& args)
{
  if(args.length())
  {
    try{
      Address addr(args.c_str());
      tempBlock.insert(addr);
    }catch(...)
    {
      CmdOut("Invalid address:'%s'",args.c_str());
    }
  }
}

void addPermBlock(SmppSession& ss,const string& args)
{
  if(args.length())
  {
    try{
      Address addr(args.c_str());
      permBlock.insert(addr);
    }catch(...)
    {
      CmdOut("Invalid address:'%s'",args.c_str());
    }
  }
}

void delTempBlock(SmppSession& ss,const string& args)
{
  if(args.length())
  {
    try{
      Address addr(args.c_str());
      std::set<Address>::iterator it=tempBlock.find(addr);
      if(it!=tempBlock.end())
      {
        tempBlock.erase(it);
      }else
      {
        CmdOut("Address '%s' wasn't temporary blocked",args.c_str());
      }
    }catch(...)
    {
      CmdOut("Invalid address:'%s'",args.c_str());
    }
  }
}

void delPermBlock(SmppSession& ss,const string& args)
{
  if(args.length())
  {
    try{
      Address addr(args.c_str());
      std::set<Address>::iterator it=permBlock.find(addr);
      if(it!=permBlock.end())
      {
        permBlock.erase(it);
      }else
      {
        CmdOut("Address '%s' wasn't temporary blocked",args.c_str());
      }
    }catch(...)
    {
      CmdOut("Invalid address:'%s'",args.c_str());
    }
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
  CmdOut("%s=",opt.name);
  switch(opt.type)
  {
    case 'i':CmdOut("%d\n",opt.asInt());break;
    case 'b':CmdOut("%s\n",opt.asBool()?"yes":"no");break;
    case 'm':
    {
      CmdOut("%s\n",modes[opt.asInt()]);
      if(singleopt)
      {
        CmdOut("Available modes:\n");
        for(int i=0;i<sizeof(modes)/sizeof(modes[0]);i++)CmdOut("  %s\n",modes[i]);
      }
    }break;
    case 'd':
    {
      int idx=findDcsByValue(opt.asInt());
      if(idx!=-1)
      {
        CmdOut("%s - %s\n",dcs[idx].name.c_str(),dcs[idx].comment);
      }
      if(singleopt)
      {
        CmdOut("Avialable datacoding schemes:\n");
        for(int i=0;i<sizeof(dcs)/sizeof(dcs[0]);i++)
        {
          CmdOut("  %s - %s\n",dcs[i].name.c_str(),dcs[i].comment);
        }
      }
    }break;
    case 'l':
    {
      if(opt.asInt()==0)
      {
        CmdOut("org\n");
      }else if(opt.asInt()==1)
      {
        CmdOut("dst\n");
      }else if(opt.asInt()==2)
      {
        CmdOut("both\n");
      }
      if(singleopt)
      {
        CmdOut("Available temp block modes:\n");
        CmdOut("org\n");
        CmdOut("dst\n");
        CmdOut("both\n");
      }
    }break;
    case 's':CmdOut("%s\n",opt.asString().c_str());break;
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
    CmdOut("Usage: set optionname=optionvalue\nAvailable options are:\n");
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
    CmdOut("Unknown option\n");
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
          else if(val=="store")options[i].asInt()=3;
        }break;
        case 'd':
        {
          int idx=findDcsByName(val.c_str());
          if(idx!=-1)
          {
            dataCoding=dcs[idx].value;
          }else
          {
            CmdOut("Error: unknown dcs %s\n",val.c_str());
          }
        }break;
        case 'l':
        {
          if(val=="org")
          {
            tempBlockMode=0;
          }else if (val=="dst")
          {
            tempBlockMode=1;
          }else if (val=="both")
          {
            tempBlockMode=2;
          }else
          {
            CmdOut("Error: unknown block  mode %s\n",val.c_str());
          }
        }break;
        case 's':options[i].asString()=val;break;
      }
      return;
    }
  }
  CmdOut("Unknown option\n");
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
    CmdOut("Sar off\n");
    return;
  }
  if(!splitString(mr,num))
  {
    CmdOut("Usage: sar mr num seq\nor sar off\n to reset sar info\n");
    return;
  }
  if(!splitString(num,seq))
  {
    CmdOut("Usage: sar mr num seq\nor sar off\n to reset sar info\n");
    return;
  }
  sar_mr=atoi(mr.c_str());
  sar_num=atoi(num.c_str());
  sar_seq=atoi(seq.c_str());
  CmdOut("sar info set to mr=%d, num=%d, seq=%d\n",sar_mr,sar_num,sar_seq);
}

void ShowHelp(SmppSession& ss,const string& args);

#ifdef _WIN32
static void gettimeofday(timeval* tv,void* ptr)
{
  __int64 ft;
  GetSystemTimeAsFileTime((PFILETIME)&ft);
  tv->tv_sec=ft/10000000;
  tv->tv_usec=ft/10;
}

/*
extern "C" int strncasecmp(const char* str,const char* str2,int n)
{
  return _strnicmp(str,str2,n);
}

extern "C" int strcasecmp(const char* str,const char* str2)
{
  return _stricmp(str,str2);
}
*/
#endif


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
#ifndef _WIN32
  sec+=msec/1000;
  msec=msec%1000;
#endif
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
#ifndef _WIN32
  timespec ts;
  ts.tv_sec=sec;
  ts.tv_nsec=msec*1000000;
  nanosleep(&ts,0);
#else
  Sleep(msec);
#endif

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
    CmdOut("Message %d not found in history\n",idx);
    return;
  }
  answerMode=true;
  answerMr=it->second.first;
  answerAddress=it->second.second;
}

void AddSmppOptional(SmppSession& ss,const string& args)
{
  std::string tagstr=args;
  std::string dump;
  if(!splitString(tagstr,dump))
  {
    CmdOut("Usage: addsmppoptional tag hexdump\n");
    return;
  }
  SmppOptionalTag so;
  sscanf(tagstr.c_str(),"%hx",&so.tag);
  for(int i=0;i<dump.length();i+=2)
  {
    int v;
    sscanf(dump.c_str()+i,"%02x",&v);
    so.data.push_back(v);
  }
  optionalTags.insert(OptionalTagsMap::value_type(lastTagId,so));
  CmdOut("Id of added tag:%d\n",lastTagId);
  lastTagId++;
}

void RemoveSmppOptional(SmppSession& ss,const string& args)
{
  if(args.length()==0)
  {
    CmdOut("Usage: removesmppoptional tagid\n");
    CmdOut("tagid printed when you add tag\n");
    return;
  }
  int id=-1;
  if(sscanf(args.c_str(),"%d",&id)!=1 || id<0)
  {
    CmdOut("Invalid id\n");
    return;
  }
  OptionalTagsMap::iterator it=optionalTags.find(id);
  if(it==optionalTags.end())
  {
    CmdOut("Tag with this id not found\n");
    return;
  }
  optionalTags.erase(it);
}

void SetCmdOut(SmppSession& s,const string& args)
{
  if(args.length()==0)
  {
    CmdOut("Usage: setcmdout {filename}|none\n");
    return;
  }
  if(args=="none")
  {
    if(cmdFile && cmdFile!=stdout)
    {
      fclose(cmdFile);
    }
    cmdFile=0;
    return;
  }
  if(args=="stdout")
  {
    if(cmdFile && cmdFile!=stdout)
    {
      fclose(cmdFile);
    }
    cmdFile=stdout;
    return;
  }
  FILE *f=fopen(args.c_str(),"wt");
  if(!f)
  {
    fprintf(stderr,"Failed to open file:'%s' for writing",args.c_str());
    return;
  }
  if(cmdFile && cmdFile!=stdout)fclose(cmdFile);
  cmdFile=f;
}

void SetIncomOut(SmppSession& s,const string& args)
{
  if(args.length()==0)
  {
    CmdOut("Usage: setincomout {filename}|none\n");
    return;
  }
  if(args=="none")
  {
    if(incomFile && incomFile!=stdout)
    {
      fclose(incomFile);
    }
    incomFile=0;
    return;
  }
  if(args=="stdout")
  {
    if(incomFile && incomFile!=stdout)
    {
      fclose(incomFile);
    }
    incomFile=stdout;
    return;
  }
  FILE *f=fopen(args.c_str(),"wt");
  if(!f)
  {
    fprintf(stderr,"Failed to open file:'%s' for writing",args.c_str());
    return;
  }
  if(incomFile && incomFile!=stdout)fclose(incomFile);
  incomFile=f;
}

void LoadTempErrors(SmppSession& s,const std::string& args)
{
  if(args.length()==0)
  {
    CmdOut("Usage: loadtemperrors {filename}\n");
    return;
  }
  FILE *f=fopen(args.c_str(),"rt");
  char buf[128];
  tempErrors.clear();
  while(fgets(buf,sizeof(buf),f))
  {
    if(!buf[0])continue;
    int err=atoi(buf);
    if(smsc::system::Status::isErrorPermanent(err))
    {
      CmdOut("WARNING: permanent error %d loaded as temporal\n",err);
    }
    tempErrors.push_back(err);
  }
  if(tempErrors.size()==0)
  {
    CmdOut("WARNING: file with temporal errors is empty or doesn't contains valid error codes, reseting temporal errors to default");
    tempErrors.insert(tempErrors.begin(),tempErrorsDefault,tempErrorsDefault+GetArraySize(tempErrorsDefault));
  }
}

void LoadPermErrors(SmppSession& s,const std::string& args)
{
  if(args.length()==0)
  {
    CmdOut("Usage: loadpermerrors {filename}\n");
    return;
  }
  FILE *f=fopen(args.c_str(),"rt");
  char buf[128];
  permErrors.clear();
  while(fgets(buf,sizeof(buf),f))
  {
    if(!buf[0])continue;
    int err=atoi(buf);
    if(!smsc::system::Status::isErrorPermanent(err))
    {
      CmdOut("WARNING: temporal error %d loaded as permanent\n",err);
    }
    permErrors.push_back(err);
  }
  if(permErrors.size()==0)
  {
    CmdOut("WARNING: file with permanent errors is empty or doesn't contains valid error codes, reseting permanent errors to default");
    permErrors.insert(permErrors.begin(),permErrorsDefault,permErrorsDefault+GetArraySize(permErrorsDefault));
  }
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
{"addsmppoptional",AddSmppOptional},
{"removesmppoptional",RemoveSmppOptional},
{"setumr",SetUmrCommand},
{"setcmdout",SetCmdOut},
{"setincomout",SetIncomOut},
{"loadtemperrors",LoadTempErrors},
{"loadpermerrors",LoadPermErrors},
{"addtempblock",addTempBlock},
{"deltempblock",delTempBlock},
{"addpermblock",addPermBlock},
{"delpermblock",delPermBlock}
};

const int commandsCount=sizeof(commands)/sizeof(CmdRec);

void ShowHelp(SmppSession& ss,const string& args)
{
  CmdOut("Available commands:\n");
  for( int i=0;i<commandsCount;i++)
  {
    CmdOut("%s\n",commands[i].cmdname);
  }
  CmdOut("/ - send last message\n");
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

std::string vcprefix(int idx,VClientData& vc)
{
  std::string out;
  timeval tv;
  gettimeofday(&tv,0);
  tm tres;
  tm* t=localtime_r(&tv.tv_sec,&tres);
  int msec=tv.tv_usec;
  msec/=1000;
  char buf[128];
  for(int i=0;i<vcprefixtemplate.length();i++)
  {
    if(vcprefixtemplate[i]!='$')
    {
      out+=vcprefixtemplate[i];
      continue;
    }
    i++;if(i>=vcprefixtemplate.length())break;
    int width=0;
    if(isdigit(vcprefixtemplate[i]))
    {
      int skip;
      sscanf(vcprefixtemplate.c_str()+i,"%d%n",&width,&skip);
      i+=skip;
    }
    if(i>=vcprefixtemplate.length())break;
    switch(vcprefixtemplate[i])
    {
      case '$':out+='$';break;
      case 'i':
      {
        snprintf(buf,sizeof(buf),"%0*d",width,idx);
        out+=buf;
      }break;
      case 'y':
      case 'Y':
      {
        int y=t->tm_year+1900;
        if(vcprefixtemplate[i]=='y')y%=100;
        snprintf(buf,sizeof(buf),"%0*d",width,y);
        out+=buf;
      }break;
      case 'M':
      {
        snprintf(buf,sizeof(buf),"%0*d",width,t->tm_mon+1);
        out+=buf;
      }break;
#define TSFIELD(letter,field) \
      case letter: { \
        snprintf(buf,sizeof(buf),"%0*d",width,t->tm_##field); \
        out+=buf; }break;

      TSFIELD('d',mday)
      TSFIELD('h',hour)
      TSFIELD('m',min)
      TSFIELD('s',sec)

#undef TSFIELD

      case 'u':
      {
        snprintf(buf,sizeof(buf),"%0*d",width,msec);
        out+=buf;
      }break;
      case 'a':
      {
        snprintf(buf,sizeof(buf),"%0*s",width,vc.sourceAddress.c_str());
        out+=buf;
      }break;
    }
  }
  return out;
}

void ExtractAddresses(SmppHeader* pdu,Address& org,Address& dst)
{
  if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
  {
    PduDeliverySm* dlvr=(PduDeliverySm*)pdu;
    org=PduAddress2Address(dlvr->get_message().get_source());
    dst=PduAddress2Address(dlvr->get_message().get_dest());
  }else
  {
    PduDataSm* dsm=(PduDataSm*)pdu;
    org=PduAddress2Address(dsm->get_data().get_source());
    dst=PduAddress2Address(dsm->get_data().get_dest());
  }
}

std::string getTimeStamp()
{
  time_t now=time(NULL);
  timeval tv;
  gettimeofday(&tv,0);
  tm tres;
  tm* t=localtime_r(&tv.tv_sec,&tres);
  int msec=tv.tv_usec;
  msec/=1000;
  char buf[128];
  sprintf(buf,"%02d.%02d.%04d %02d:%02d:%02d.%03d:",t->tm_mday,t->tm_mon+1,t->tm_year+1900,t->tm_hour,t->tm_min,t->tm_sec,msec);
  return buf;
}

class ResponseProcessor:public Thread{
  Array<SmppHeader*> queue;
  EventMonitor mon;
  SmppTransmitter* trans;
  SmppTransmitter* atrans;

  int tempErrIdx;
  int permErrIdx;

  struct DelayedResp
  {
    int seq;
    int err;
    timeval delayTill;
    bool isDataSm;
    bool operator<(const DelayedResp& rhs)const
    {
      return delayTill.tv_sec<rhs.delayTill.tv_sec ||
            (delayTill.tv_sec==rhs.delayTill.tv_sec && delayTill.tv_usec<rhs.delayTill.tv_usec);
    }
  };

  std::multiset<DelayedResp> delayedResponses;

  void sendResp(int seq,int err,bool isDataSm,int delay=0)
  {
    if(delay)
    {
      DelayedResp dr;
      dr.seq=seq;
      dr.err=err;
      dr.isDataSm=isDataSm;
      timeval tv;
      gettimeofday(&tv,0);
      tv.tv_usec+=delay*1000;
      tv.tv_sec+=tv.tv_usec/1000000;
      tv.tv_usec%=1000000;
      dr.delayTill=tv;
      delayedResponses.insert(dr);
      return;
    }
    if(isDataSm)
    {
      PduDataSmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DATA_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(seq);
      resp.get_header().set_commandStatus(err);
      atrans->sendDataSmResp(resp);
    }else
    {
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(seq);
      resp.get_header().set_commandStatus(err);
      atrans->sendDeliverySmResp(resp);
    }
  }

  int TimeValDiff(timeval tv1,timeval tv2)
  {
    int diff=(tv1.tv_sec-tv2.tv_sec)*1000;
    diff+=(tv1.tv_usec-tv2.tv_usec)/1000;
    return diff;
  }


public:
  int Execute()
  {
    tempErrIdx=0;
    permErrIdx=0;
    mon.Lock();
    while(!stopped)
    {
      if(queue.Count()==0)
      {
        if(delayedResponses.empty())
        {
          mon.wait();
        }else
        {
          timeval tv;
          gettimeofday(&tv,0);
          int delay=TimeValDiff(delayedResponses.begin()->delayTill,tv);
          if(delay>0)mon.wait(delay);
        }
      }
      if(!delayedResponses.empty())
      {
        timeval tv;
        gettimeofday(&tv,0);
        std::multiset<DelayedResp>::iterator it=delayedResponses.begin();
        if(TimeValDiff(it->delayTill,tv)<=0)
        {
          sendResp(it->seq,it->err,it->isDataSm);
          delayedResponses.erase(it);
        }
      }

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
      bool isDataSm=pdu->get_commandId()==SmppCommandSet::DATA_SM;
      {
        Address org,dst;
        ExtractAddresses(pdu,org,dst);
        if(((tempBlockMode==0 || tempBlockMode==2) && tempBlock.find(org)!=tempBlock.end()) ||
           ((tempBlockMode==1 || tempBlockMode==2) && tempBlock.find(dst)!=tempBlock.end()))
        {
          sendResp(pdu->get_sequenceNumber(),SmppStatusSet::ESME_RX_T_APPN,isDataSm,0);
          if(!silent)
          {
            IncomOut("\nPDU from %s to %s blocked with temporal error\n",org.toString().c_str(),dst.toString().c_str());
          }
          return;
        }
        if(permBlock.find(org)!=permBlock.end() || permBlock.find(dst)!=permBlock.end())
        {
          sendResp(pdu->get_sequenceNumber(),SmppStatusSet::ESME_RX_P_APPN,isDataSm,0);
          if(!silent)
          {
            IncomOut("\nPDU from %s to %s blocked with permanent error\n",org.toString().c_str(),dst.toString().c_str());
          }
          return;
        }
      }

      int rnd=rand()%100;
      if((isDataSm && rnd<temperrProbDataSm) || (!isDataSm && rnd<temperrProb))
      {
        int delay=0;
        if(respDelayMin || respDelayMax)
        {
          double r=1.0*rand()/RAND_MAX;
          delay=respDelayMin+(respDelayMax-respDelayMin)*r;
        }
        sendResp(pdu->get_sequenceNumber(),tempErrors[tempErrIdx++],isDataSm,delay);
        if(tempErrIdx>=tempErrors.size())tempErrIdx=0;
        if(!silent)
        {
          Address org,dst;
          ExtractAddresses(pdu,org,dst);
          if(vcmode)
          {
            IncomOut("%sPDU from %s to %s was replied with temporal error\n",getTimeStamp().c_str(),org.toString().c_str(),dst.toString().c_str());
          }else
          {
            IncomOut("\nPDU from %s to %s was replied with temporal error\n",org.toString().c_str(),dst.toString().c_str());
          }
        }
        return;
      }
      if((isDataSm && rnd<temperrProbDataSm+permErrProbDataSm) || (!isDataSm && rnd<temperrProb+permErrProb))
      {
        int delay=0;
        if(respDelayMin || respDelayMax)
        {
          double r=1.0*rand()/RAND_MAX;
          delay=respDelayMin+(respDelayMax-respDelayMin)*r;
        }
        sendResp(pdu->get_sequenceNumber(),permErrors[permErrIdx++],pdu->get_commandId()==SmppCommandSet::DATA_SM,delay);
        if(permErrIdx>=permErrors.size())permErrIdx=0;
        if(!silent)
        {
          Address org,dst;
          ExtractAddresses(pdu,org,dst);
          if(vcmode)
          {
            IncomOut("%sPDU from %s to %s was replied with permanent error\n",getTimeStamp().c_str(),org.toString().c_str(),dst.toString().c_str());
          }else
          {
            IncomOut("\nPDU from %s to %s was replied with permanent error\n",org.toString().c_str(),dst.toString().c_str());
          }
        }
        return;
      }else if((isDataSm && rnd<temperrProbDataSm+permErrProbDataSm+dontrespProbDataSm) || (!isDataSm && rnd<temperrProb+permErrProb+dontrespProb))
      {
        if(!silent)
        {
          Address org,dst;
          ExtractAddresses(pdu,org,dst);
          if(vcmode)
          {
            IncomOut("%sPDU from %s to %s was not replied\n",getTimeStamp().c_str(),org.toString().c_str(),dst.toString().c_str());
          }else
          {
            IncomOut("\nPDU from %s to %s was not replied\n",org.toString().c_str(),dst.toString().c_str());
          }
        }
        return;
      }

      int delay=0;
      if(respDelayMin || respDelayMax)
      {
        double r=1.0*rand()/RAND_MAX;
        delay=respDelayMin+(respDelayMax-respDelayMin)*r;
      }

      sendResp(pdu->get_sequenceNumber(),respStatus,pdu->get_commandId()==SmppCommandSet::DATA_SM,delay);

      if(silent && !autoAnswer && !waitRespMode)
      {
        return;
      }

      char buf[65535];
      SMS s;
      if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
      {
        fetchSmsFromSmppPdu((PduXSm*)pdu,&s,0);
      }else
      {
        fetchSmsFromDataSmPdu((PduDataSm*)pdu,&s,0);
      }

      if(autoAnswer)
      {
        Address oa=s.getOriginatingAddress();
        s.setOriginatingAddress(s.getDestinationAddress());
        s.setDestinationAddress(oa);
        if(s.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
        {
          s.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,USSD_PSSR_RESP);
        }
        if(dataSm)
        {
          PduDataSm sm;
          sm.get_header().set_commandId(SmppCommandSet::DATA_SM);
          fillDataSmFromSms(&sm,&s,0);
          atrans->data(sm);
          if(!silent)
          {
            IncomOut("Autoanswered: seq=%d\n",sm.get_header().get_sequenceNumber());
          }
        }else
        {
          PduSubmitSm sm;
          sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
          fillSmppPduFromSms(&sm,&s,0);
          atrans->submit(sm);
          if(!silent)
          {
            IncomOut("Autoanswered: seq=%d\n",sm.get_header().get_sequenceNumber());
          }
        }
      }

      int vcidx=-1;

      if(waitRespMode)
      {
        MutexGuard g(mrMtx);
        MrMap::iterator m=mrStore.find
          (
            MrKey
            (
              s.getDestinationAddress().toString().c_str(),
              waitRespNoUMR?0:s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)
            )
          );
        if(m!=mrStore.end())
        {
          (*(m->second))--;
          if(*m->second==0)
          {
            mrStore.erase(m);
          }
          if(vcmode)
          {
            vcidx=m->first.vcidx;
          }
        }
      }


      if(silent)
      {
        return;
      }


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
      IncomOut("\n===== %d =====\n",smsNum);
      if(vcmode && vcidx!=-1)
      {
        IncomOut("VCInfo(%s): %s\n",isDataSm?"dataSm":"deliver",vcprefix(vcidx,vcarray[vcidx]).c_str());
      }else
      {
        IncomOut("Timestamp(%s): %s\n",isDataSm?"dataSm":"deliver",getTimeStamp().c_str());
      }
      IncomOut("From:%s\n",buf);
      s.getDestinationAddress().toString(buf,sizeof(buf));
      IncomOut("To:%s\n",buf);
      IncomOut("DCS:%d\n",s.getIntProperty(Tag::SMPP_DATA_CODING));
      IncomOut("EsmClass:%x\n",s.getIntProperty(Tag::SMPP_ESM_CLASS));
      if(s.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE))
      {
        IncomOut("UMR:%d\n",s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
      }
      if(s.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
      {
        IncomOut("MsgState:%d\n",s.getIntProperty(Tag::SMPP_MSG_STATE));
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
        IncomOut("Udh len: %d\n",udhlen);
        IncomOut("Udh dump:");
        for(int i=1;i<=udhlen;i++)
        {
          IncomOut(" %02X",(unsigned)body[i]);
        }
        IncomOut("\n");
      }
      if(s.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
      {
        IncomOut("Ussd op:%d\n",s.getIntProperty(Tag::SMPP_USSD_SERVICE_OP));
      }
      if(s.hasBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS))
      {
        unsigned len;
        const char* bin=s.getBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS,&len);
        while(len!=0)
        {
          uint16_t tag;
          memcpy(&tag,bin,2);
          tag=ntohs(tag);
          bin+=2;len-=2;
          uint16_t length;
          memcpy(&length,bin,2);
          bin+=2;len-=2;
          std::string dump;
          if(length>len)
          {
            IncomOut("Warning: Unknown tags contain incorrect data!\n");
            length=len;
          }
          for(int i=0;i<length;i++)
          {
            char buf[8];
            sprintf(buf,"%02x",(unsigned int)(unsigned char)(bin[i]));
            dump+=buf;
          }
          bin+=length;
          len-=length;
          IncomOut("Unknown smpp optional: tag=%04x dump: %s\n",tag,dump.c_str());
        }
      }
      if(s.hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM))
      {
        IncomOut("SAR_MR:%d\nSAR_TOTAL:%d\nSAR_SEQNUM:%d\n",s.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM),s.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS),s.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM));
      }
      if(s.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::BINARY)
      {
        IncomOut("Esm class: %02X\n",s.getIntProperty(Tag::SMPP_ESM_CLASS));
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
        IncomOut("Msg dump(%d):",len);
        for(int i=0;i<len;i++)
        {
          IncomOut(" %02X",(unsigned)body[i]);
        }
        IncomOut("\n");
      }else
      if(getSmsText(&s,buf,sizeof(buf),ansi1251?CONV_ENCODING_CP1251:CONV_ENCODING_KOI8R)==-1)
      {
        int sz=65536;
        char *data=new char[sz];
        if(getSmsText(&s,data,sz)!=-1)
        {
          IncomOut("Message(payload):%s\n",data);
        }else
        {
          IncomOut("Error: failed to retrieve message\n");
        }
        delete [] data;
      }else
      {
        IncomOut("Message:%s\n",buf);
      }
      IncomOut("==========\n");
      fflush(stdout);

    }else
    if(pdu->get_commandId()==SmppCommandSet::SUBMIT_SM_RESP)
    {
      if(!silent)
      {
        IncomOut("%sReceived async submit sm resp:status=%#x, seq=%d, msgId=%s\n",
          vcmode?getTimeStamp().c_str():"\n",
          pdu->get_commandStatus(),
          pdu->get_sequenceNumber(),
          ((PduXSmResp*)pdu)->get_messageId()?((PduXSmResp*)pdu)->get_messageId():"NULL");
      }
    }else if(pdu->get_commandId()==SmppCommandSet::UNBIND)
    {
      IncomOut("Received unbind\n");
      connected=false;
    }else if(pdu->get_commandId()==SmppCommandSet::ALERT_NOTIFICATION)
    {
      PduAlertNotification* al=(PduAlertNotification*)pdu;
      IncomOut("\nReceived alert notification:%s->%s:%d\n",
        PduAddress2Address(al->get_esme()).toString().c_str(),
        PduAddress2Address(al->get_source()).toString().c_str(),
        al->get_optional().get_msAvailableStatus());
    }
    if(!cmdfile && !vcmode && !silent && incomFile==stdout)rl_forced_update_display();
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
    IncomOut("Network error!\n");
    fflush(incomFile);
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

void trimend(std::string& line)
{
  int l=line.length()-1;
  while(l>=0 && isspace(line[l]))l--;
  line.erase(l+1);
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
    CmdOut("usage: %s systemid [options] @cmdfile\n",argv[0]);
    CmdOut("Options are:\n");
    CmdOut("\t -h host[:port] (default == smsc:9001)\n"
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
  string host="localhost";
  int port=9001;

  if(getenv("SMPPHOST"))
  {
    host=getenv("SMPPHOST");
    std::string::size_type pos=host.find(":");
    if(pos!=std::string::npos)
    {
      port=atoi(host.c_str()+pos+1);
      host.erase(pos);
    }

  }


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
      CmdOut("Unrecognized command line argument: %s\n",opt);
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

  std::string line;

  SmppSession ss(cfg,&lst);
  SmppTransmitter *tr=ss.getSyncTransmitter();
  SmppTransmitter *atr=ss.getAsyncTransmitter();

  rp.setTrans(tr,atr);

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
      CmdOut("Connect error:%s\n",e.getTextReason());
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
      cmdfile=new File();
      try{
        cmdfile->ROpen(cmdFileName.c_str());
      }catch(std::exception& e)
      {
        CmdOut("Faield to open command file:%s\n",cmdFileName.c_str());
        delete cmdfile;
        cmdfile=0;
      }
    }
    char fileBuf[65536];

    int vccnt=0;
    int vcused=0;
    int vcidx=0;

    vector<PendingVC> pvc;
    VClientData defVCValues;

    if(!receiveOnly)
    while(!stopped)
    {
      if(vcmode)
      {
        int acnt=0;
        for(int x=0;x<vccnt;x++)
        {
          if(!vcarray[x].active)continue;
          if(vcarray[x].waitResp)continue;
          if(vcarray[x].sleepTill)continue;
          acnt++;
        }
        if(acnt==0)
        {
#ifndef _WIN32
          timestruc_t tv={0,10*1000000};
          nanosleep(&tv,0);
#else
          Sleep(0);
#endif
        }
        vcidx++;
        if(vcidx>=vccnt)vcidx=0;
        while(vcused<vccnt && pvc.size())
        {
          PendingVC p=pvc.front();
          pvc.erase(pvc.begin());
          int idx=0;
          while(vcarray[idx].active)idx++;
          CmdOut("%s: new client %s from %s\n",vcprefix(idx,vcarray[idx]).c_str(),p.src.c_str(),p.fn.c_str());
          vcarray[idx].cmdfile=new File;
          try{
            vcarray[idx].cmdfile->ROpen(p.fn.c_str());
          }catch(std::exception& e)
          {
            delete vcarray[idx].cmdfile;
            vcarray[idx].cmdfile=0;
            CmdOut("%s: failed to open file %s, skipping\n",vcprefix(idx,vcarray[idx]).c_str(),p.fn.c_str());
            continue;
          }
          vcarray[idx].sourceAddress=p.src;
          vcarray[idx].repeatCnt=p.repeat;
          vcarray[idx].execTime=time(NULL)+p.timelimit;
          vcarray[idx].active=true;
          vcused++;
        }
        if(vcused==0)break;
        if(!vcarray[vcidx].active)continue;
        defVC=vcarray[vcidx];
        if(waitRespMode)
        {
          if(waitResp && vcarray[vcidx].waitStart+waitRespTimeout>time(NULL))
          {
            continue;
          }
          if(waitResp)
          {
            if(vcmode)CmdOut("%s:",vcprefix(vcidx,vcarray[vcidx]).c_str());
            CmdOut("Response for %s timed out!\n",sourceAddress.c_str());
            waitResp=0;
            MutexGuard g(mrMtx);
            MrMap::iterator i=mrStore.find(MrKey(sourceAddress.c_str(),lastMr));
            if(i!=mrStore.end())mrStore.erase(i);
            if(restartOnError && cmdfile)
            {
              CmdOut("Script restarted\n");
              int repCnt=vcarray[vcidx].repeatCnt;
              vcarray[vcidx]=defVCValues;
              vcarray[vcidx].repeatCnt=repCnt;
              vcarray[vcidx].repeatCnt--;
              if(vcarray[vcidx].repeatCnt==0)
              {
                cmdfile->SeekEnd(0);
              }else
              {
                cmdfile->Seek(0);
              }
              if(vcarray[vcidx].execTime<time(NULL))
              {
                CmdOut("%s:finished\n",vcprefix(vcidx,vcarray[vcidx]).c_str());
                delete cmdfile;
                cmdfile=0;
                vcarray[vcidx].Init();
                vcarray[vcidx]=defVCValues;
                vcused--;
              }
            }
          }
        }
        if(vcarray[vcidx].sleepTill!=0)
        {
          timeval tv;
          gettimeofday(&tv,0);
          if(vcarray[vcidx].sleepTill>tv.tv_sec ||
             (vcarray[vcidx].sleepTill==tv.tv_sec && vcarray[vcidx].sleepTillMsec>tv.tv_usec/1000))continue;
          vcarray[vcidx].sleepTill=0;
          vcarray[vcidx].sleepTillMsec=0;
        }
      }else
      {

        if(waitRespMode)
        {
          time_t waitstart=time(NULL);
          while(waitResp && time(NULL)-waitstart<waitRespTimeout)
          {
#ifndef _WIN32
            timespec ts;
            ts.tv_sec=0;
            ts.tv_nsec=10000000;
            nanosleep(&ts,0);
#else
            Sleep(10);
#endif
          }
          if(waitResp)
          {
            CmdOut("Response timed out!\n");
            waitResp=0;
            MutexGuard g(mrMtx);
            MrMap::iterator i=mrStore.find(MrKey(sourceAddress.c_str(),lastMr));
            if(i!=mrStore.end())mrStore.erase(i);
          }
        }
      }

      if(cmdfile)
      {
        if(!cmdfile->ReadLine(line))
        {
          if(vcmode)
          {
            if(vcarray[vcidx].repeatCnt)
            {
              vcarray[vcidx].repeatCnt--;
              if(vcarray[vcidx].repeatCnt==0)
              {
                CmdOut("%s:finished\n",vcprefix(vcidx,vcarray[vcidx]).c_str());
                delete cmdfile;
                cmdfile=0;
                vcarray[vcidx].Init();
                vcused--;
              }else
              {
                cmdfile->Seek(0);
              }
            }else if(vcarray[vcidx].execTime)
            {
              if(vcarray[vcidx].execTime>time(NULL))
              {
                cmdfile->Seek(0);
              }else
              {
                CmdOut("%s:finished\n",vcprefix(vcidx,vcarray[vcidx]).c_str());
                delete cmdfile;
                cmdfile=0;
                vcarray[vcidx].Init();
                vcarray[vcidx]=defVCValues;
                vcused--;
              }
            }else
            {
              cmdfile->Seek(0);
            }
          }else
          {
            delete cmdfile;
            addr=0;
            message=0;
            cmdfile=0;
          }
          continue;
        }
        //trimend(fileBuf);
        trimend(line);
        addr=const_cast<char*>(line.c_str());
        if(cmdecho)
        {
          if(vcmode)
          {
            CmdOut("%s:>%s\n",vcprefix(vcidx,vcarray[vcidx]).c_str(),line.c_str());
          }else
          {
            CmdOut("Address or cmd>%s\n",line.c_str());
          }
        }
      }else
      {
        if(vcmode)
        {
          CmdOut("something goes wrong... interactive mode not allowed during virtualclients\n");
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

      if(cmd[0]=='#' || cmd[0]==';' || cmd.length()==0)
      {
        //comment
        continue;
      }

      if(cmd=="virtualclients")
      {
        if(vcmode)
        {
          CmdOut("Command virtualclients is not suppoerted in virtual clients mode!!!\n");
          continue;
        }
        int cnt=10;
        if(arg.find(' ')!=string::npos)
        {
          string n;
          splitString(arg,n);
          cnt=atoi(n.c_str());
          if(cnt<=0 || cnt >1024)
          {
            CmdOut("%s is not valid number of active virtual clients\n",n.c_str());
            break;
          }
        }
        FILE *f=fopen(arg.c_str(),"rb");
        if(!f)
        {
          CmdOut("Failed to open virtualclients script:%s\n",arg.c_str());
          continue;
        }

        if(cmdfile)
        {
          delete cmdfile;
          cmdfile=0;
        }
        addr=0;
        message=0;
        cmdfile=0;

        while(fgets(fileBuf,sizeof(fileBuf),f))
        {
          trimend(fileBuf);
          if(fileBuf[0]==0)continue;
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

        vcarray=new VClientData[cnt];
        defVCValues=defVC;
        for(int i=0;i<cnt;i++)vcarray[i]=defVCValues;
        vccnt=cnt;
        vcmode=true;
        continue;
      }

      if(cmd=="connect")
      {
        if(connected)
        {
          CmdOut("Sme already connected, type disconnect first\n");
          continue;
        }
        try{
          ss.close();
          ss.connect(bindType);
          tr=ss.getSyncTransmitter();
          atr=ss.getAsyncTransmitter();
          rp.setTrans(tr,ss.getAsyncTransmitter());
        }catch(SmppConnectException& e)
        {
          CmdOut("Connect error:%s\n",e.getTextReason());
          continue;
        }
        connected=true;
        CmdOut("Connect ok\n");
        continue;
      }

      if(cmd=="disconnect")
      {
        if(!connected)
        {
          CmdOut("Sme already disconnect, connect it first\n");
          continue;
        }
        ss.close();
        CmdOut("Disconnected\n");
        connected=false;
        continue;
      }

      if(cmd[0]=='@')
      {
        string fn=cmd.substr(1);
        if(cmdfile)
        {
          delete cmdfile;
        }

        cmdfile=new File();
        try{
          cmdfile->ROpen(fn.c_str());
        }catch(std::exception& e)
        {
          delete cmdfile;
          CmdOut("Failed to open file:%s(%s)\n",fn.c_str(),strerror(errno));
        }
        if(!cmdfile)
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
        if(vcmode)vcarray[vcidx]=defVC;
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
        if(vcmode)CmdOut("%s:",vcprefix(vcidx,vcarray[vcidx]).c_str());
        CmdOut("Invalid source address:%s\n",sourceAddress.c_str());
        continue;
      }

      s.setOriginatingAddress(srcaddr);

      __trace2__("org addr=%s",s.getOriginatingAddress().toString().c_str());

      if(cmd!="/")
      {
        if(answerMode)
        {
          s.setDestinationAddress(answerAddress);
          lastAddr=answerAddress.toString().c_str();
        }else
        {
          try{
            Address dst((char*)addr);
            lastAddr=addr;
            s.setDestinationAddress(dst);
          }catch(...)
          {
            if(vcmode)CmdOut("%s:",vcprefix(vcidx,vcarray[vcidx]).c_str());
            CmdOut("Invalid address:%s\n",addr);
            continue;
          }
        }
        if(cmdfile)
        {
          //fgets(fileBuf,sizeof(fileBuf),cmdfile);
          line="";
          cmdfile->ReadLine(line);
          trimend(line);
          message=const_cast<char*>(line.c_str());
          if(cmdecho)
          {
            if(vcmode)CmdOut("%s:",vcprefix(vcidx,vcarray[vcidx]).c_str());
            CmdOut("Enter message:%s\n",line.c_str());
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
        CmdOut("Sme not connected, type connect first\n");
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
      s.setValidTime(validTime?time(NULL)+validTime:0);
      if(dataSm && validTime)
      {
        s.setIntProperty(Tag::SMPP_QOS_TIME_TO_LIVE,validTime);
      }
      s.setIntProperty(Tag::SMPP_ESM_CLASS,(esmclass&(~0x3))|mode);
      s.setDeliveryReport(0);
      s.setArchivationRequested(false);
      s.setEServiceType(eservicetype.c_str());

      if(replaceIfPresent)s.setIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG,1);

      if(setDpf)s.setIntProperty(Tag::SMPP_SET_DPF,1);


      if(ussd!=-1)s.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,ussd);
      else s.messageBody.dropIntProperty(Tag::SMPP_USSD_SERVICE_OP);

      if(s.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
      {
        __trace2__("ussd=%d",s.getIntProperty(Tag::SMPP_USSD_SERVICE_OP));
      }else
      {
        __trace__("no ussd");
      }
      if(!answerMode)
      {
        if(umrPresent)
        {
          s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,umr);
        }else
        {
          s.messageBody.dropIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
        }
      }else
      {
        answerMode=false;
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
        if(tmp.length()>140 || dataSm)
        {
          s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),tmp.length());
        }else
        {
          s.setBinProperty(Tag::SMPP_SHORT_MESSAGE,tmp.c_str(),tmp.length());
          s.setIntProperty(Tag::SMPP_SM_LENGTH,tmp.length());
        }
        len=tmp.length();
      }else
      if(dataCoding==DataCoding::UCS2)//UCS2
      {
        auto_ptr<short> msg(new short[len+1]);
        ConvertMultibyteToUCS2(message,len,msg.get(),len*2,ansi1251?CONV_ENCODING_CP1251:CONV_ENCODING_KOI8R);
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::UCS2);
        len*=2;
        if(len>140 || dataSm)
        {
          s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,(char*)msg.get(),len);
        }else
        {
          s.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)msg.get(),len);
          s.setIntProperty(Tag::SMPP_SM_LENGTH,len);
        }
      }else if(dataCoding==DataCoding::SMSC7BIT)
      {
        auto_ptr<char> msg(new char[len*3+1]);
        len=ConvertLatin1ToSMSC7Bit(message,len,msg.get());
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::SMSC7BIT);
        if(len>140 || dataSm)
        {
          s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,(char*)msg.get(),len);
        }else
        {
          s.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)msg.get(),len);
          s.setIntProperty(Tag::SMPP_SM_LENGTH,len);
        }
      }else //latin1
      {
        s.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
        if(len>140 || dataSm)
        {
          s.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,message,len);
        }else
        {
          s.setBinProperty(Tag::SMPP_SHORT_MESSAGE,message,len);
          s.setIntProperty(Tag::SMPP_SM_LENGTH,len);
        }
      }

      /*
      if(hexinput)
      {
        message=hexsave;
      }
      */

      if(waitRespMode)
      {
        int mr=0;
        if(!waitRespNoUMR)
        {
          if(!s.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE))
          {
            mr=getmr(sourceAddress.c_str());
            s.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,mr);
            if(vcmode)CmdOut("%s:",vcprefix(vcidx,vcarray[vcidx]).c_str());
            CmdOut("Set mr to %d\n",mr);
          }else
          {
            mr=s.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
          }
        }
        lastMr=mr;
        MutexGuard g(mrMtx);
        if(vcmode)
        {
          vcarray[vcidx].waitResp=wrCount;
          vcarray[vcidx].waitStart=time(NULL);
          mrStore.insert(make_pair(MrKey(Address(sourceAddress.c_str()).toString().c_str(),mr,vcidx),&vcarray[vcidx].waitResp));
        }else
        {
          waitResp=wrCount;
          mrStore.insert(make_pair(MrKey(Address(sourceAddress.c_str()).toString().c_str(),mr),&waitResp));
        }
      }

      if(vcmode)
      {
        CmdOut("%s:",vcprefix(vcidx,vcarray[vcidx]).c_str());
      }

      SmppHeader *resp=0;
      if(!dataSm)
      {
        fillSmppPduFromSms(&sm,&s,0);
        AddOptionals(sm);
        try{
          if(asyncsend || vcmode)
          {
            atr->submit(sm);
          }else
          {
            resp=(SmppHeader*)tr->submit(sm);
          }
        }catch(SmppInvalidBindState& e)
        {
          CmdOut("Pdu sent in invalid bind state\n");
        }
      }else
      {
        fillDataSmFromSms(&dsm,&s,0);
        AddOptionals(dsm);
        try{
          if(asyncsend || vcmode)
          {
            atr->data(dsm);
          }else
          {
            resp=(SmppHeader*)tr->data(dsm);
          }
        }catch(SmppInvalidBindState& e)
        {
          CmdOut("Pdu sent in invalid bind state\n");
        }
      }


      if(resp && resp->get_commandStatus()==0)
      {
        CmdOut("Accepted:%d bytes, msgId=%s\n",len,((PduXSmResp*)resp)->get_messageId());fflush(stdout);
      }else
      {
        if(resp)
        {
          CmdOut("Wasn't accepted: %d\n",resp->get_commandStatus());
          if(setDpf && resp->get_commandId()==SmppCommandSet::DATA_SM_RESP)
          {
            CmdOut("Dpf result=%d\n",((PduDataSmResp*)resp)->get_optional().get_dpfResult());
          }
        }else
        {
          if(asyncsend || vcmode)
          {
            CmdOut("PDU sent. Seq=%d\n",dataSm?dsm.get_header().get_sequenceNumber():sm.get_header().get_sequenceNumber());
          }else
          {
            CmdOut("Response timed out\n");
          }
        }
        fflush(stdout);
      }

      if(vcmode && vcmodemaxspeed>0)
      {
        int delay=1000/vcmodemaxspeed-1;
        millisleep(delay);
      }

      if(resp)disposePdu(resp);
    }

    if(receiveOnly)
    {
#ifndef _WIN32
      while(!stopped){sleep(5);}
#else
      while(!stopped){Sleep(5000);}
#endif
    }
  }
  catch(std::exception& e)
  {
    CmdOut("Exception: %s\n",e.what());
  }
  catch(...)
  {
    CmdOut("unknown exception\n");
  }
  if(connected)
  {
    CmdOut("\nUnbinding\n");
    try
    {
      PduUnbind pdu;
      pdu.get_header().set_commandId(SmppCommandSet::UNBIND);
      pdu.get_header().set_sequenceNumber(ss.getNextSeq());
      SmppHeader *resp=tr->sendPdu((SmppHeader*)&pdu);
      if(resp)
      {
        CmdOut("Unbind response:status=%#x\n",resp->get_commandStatus());
        disposePdu(resp);
      }else
      {
        CmdOut("Unbind response timed out\n");
      }
    }catch(...)
    {
      CmdOut("Exception during unbind\n");
    }
  }
  fflush(cmdFile);
  fflush(incomFile);
  ss.close();
  stopped=1;
  rp.Notify();
  CmdOut("Exiting\n");//////
  return 0;
}
