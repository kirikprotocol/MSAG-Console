#include "util/debug.h"
#include "system/smsc.hpp"
#include "system/state_machine.hpp"
#include <exception>
#include "system/common/rescheduler.hpp"
#include "profiler/profiler.hpp"
#include "util/recoder/recode_dll.h"
#include "core/buffers/Hash.hpp"
#include "util/smstext.h"
#include "logger/Logger.h"
#include "util/regexp/RegExp.hpp"
#include "core/synchronization/Mutex.hpp"
#include "system/status.h"
#include "resourcemanager/ResourceManager.hpp"
#include "util/udh.hpp"

// ������� �� ������, ��� � �������� autodetect :)

namespace smsc{
namespace system{

using namespace smsc::smeman;
using namespace smsc::sms;
using namespace StateTypeValue;
using namespace smsc::smpp;
using namespace util;
using namespace smsc::system;
using namespace smsc::resourcemanager;
using std::exception;
using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;
using smsc::core::synchronization::Mutex;

static const int REPORT_ACK=2;
static const int REPORT_NOACK=255;

//divert flags
static const int DF_UNCOND=1;
static const int DF_ABSENT=2;
static const int DF_BLOCK =4;
static const int DF_BARRED =8;
static const int DF_CAPAC =16;
static const int DF_COND  =DF_ABSENT|DF_BLOCK|DF_BARRED|DF_CAPAC;

static const int DF_UDHCONCAT=128;

static const int DF_DCSHIFT=8;

Hash<std::list<std::string> > StateMachine::directiveAliases;

struct TaskGuard{
  Smsc *smsc;
  uint32_t dialogId;
  uint32_t uniqueId;
  bool active;
  TaskGuard()
  {
    smsc=0;
    dialogId=0;
    uniqueId=0;
    active=false;
  }
  ~TaskGuard()
  {
    if(active)
    {
      Task t;
      if(smsc->tasks.findAndRemoveTask(uniqueId,dialogId,&t))
      {
        __trace2__("TG: killing sms of task %u/%u",dialogId,uniqueId);
        delete t.sms;
      }
    }
  }
};

class ExtractPartFailedException{};


class ReceiptGetAdapter:public GetAdapter{
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

static std::string AltConcat(const char* prefix,Hash<std::list<std::string> >& h,const char* postfix="")
{
  std::string res;
  if(!h.Exists(prefix))
  {
    res+='#';
    res+=prefix;
    res+=postfix;
    res+='#';
    return res;
  }
  res+="#(?:";
  res+=prefix;
  std::list<std::string>& lst=h[prefix];
  std::list<std::string>::const_iterator i=lst.begin();
  while(i!=lst.end())
  {
    res+='|';
    res+=*i;
    i++;
  }
  res+=')';
  res+=postfix;
  res+='#';
  __trace2__("StateMachine:: directive regexp for %s=%s",prefix,res.c_str());
  return res;
}

StateMachine::StateMachine(EventQueue& q,
               smsc::store::MessageStore* st,
               smsc::system::Smsc *app):
               eq(q),
               store(st),
               smsc(app)

{
  using namespace smsc::util::regexp;
  smsLog = smsc::logger::Logger::getInstance("sms.trace");
  dreNoTrans.Compile(AltConcat("notrans",directiveAliases).c_str(),OP_IGNORECASE|OP_OPTIMIZE);
  __throw_if_fail__(dreNoTrans.LastError()==regexp::errNone,RegExpCompilationException);
  dreAck.Compile(AltConcat("ack",directiveAliases).c_str(),OP_IGNORECASE|OP_OPTIMIZE);
  __throw_if_fail__(dreAck.LastError()==regexp::errNone,RegExpCompilationException);
  dreNoAck.Compile(AltConcat("noack",directiveAliases).c_str(),OP_IGNORECASE|OP_OPTIMIZE);
  __throw_if_fail__(dreNoAck.LastError()==regexp::errNone,RegExpCompilationException);
  dreHide.Compile(AltConcat("hide",directiveAliases).c_str(),OP_IGNORECASE|OP_OPTIMIZE);
  __throw_if_fail__(dreHide.LastError()==regexp::errNone,RegExpCompilationException);
  dreUnhide.Compile(AltConcat("unhide",directiveAliases).c_str(),OP_IGNORECASE|OP_OPTIMIZE);
  __throw_if_fail__(dreUnhide.LastError()==regexp::errNone,RegExpCompilationException);
  dreFlash.Compile(AltConcat("flash",directiveAliases).c_str(),OP_IGNORECASE|OP_OPTIMIZE);
  __throw_if_fail__(dreFlash.LastError()==regexp::errNone,RegExpCompilationException);
  dreDef.Compile(AltConcat("def",directiveAliases,"\\s+((?:\\d+|(?=\\.))(\\.\\d+)?)").c_str(),OP_IGNORECASE|OP_OPTIMIZE);
  __throw_if_fail__(dreDef.LastError()==regexp::errNone,RegExpCompilationException);
  dreTemplate.Compile(AltConcat("template",directiveAliases,"=(.*?)").c_str(),OP_IGNORECASE|OP_OPTIMIZE);
  __throw_if_fail__(dreTemplate.LastError()==regexp::errNone,RegExpCompilationException);
  dreTemplateParam.Compile("/\\s*\\{(\\w+)\\}=(\".*?\"|[^{\\s]+)/s");
  __throw_if_fail__(dreTemplateParam.LastError()==regexp::errNone,RegExpCompilationException);
  dreUnknown.Compile("/#.*?#/");
  __throw_if_fail__(dreUnknown.LastError()==regexp::errNone,RegExpCompilationException);
}


void StateMachine::formatDeliver(const FormatData& fd,std::string& out)
{
  ReceiptGetAdapter ga;
  ContextEnvironment ce;
  ce.exportStr("dest",fd.addr);
  ce.exportStr("ddest",fd.ddest);
  ce.exportDat("date",fd.date);
  ce.exportStr("msgId",fd.msgId);
  ce.exportInt("lastResult",fd.lastResult);
  ce.exportInt("lastResultGsm",fd.lastResultGsm);
  ce.exportStr("msc",fd.msc);
  ce.exportDat("discharge",time(NULL));

  string key="receipt.";
  key+=fd.scheme;
  key+=".";
  key+="deliveredFormat";

  __trace2__("RECEIPT: get formatter for key=%s",key.c_str());

  OutputFormatter* ofDelivered=ResourceManager::getInstance()->getFormatter(fd.locale,key);
  if(!ofDelivered)
  {
    out="Unknown formatter "+key+" for locale "+fd.locale;
    return;
  }

  try{
    ofDelivered->format(out,ga,ce);
  }catch(exception& e)
  {
    __trace2__("FORMATTER: %s",e.what());
    out="Invalid formatter "+key+" for locale "+fd.locale;
  }
}
void StateMachine::formatFailed(const FormatData& fd,std::string& out)
{
  ReceiptGetAdapter ga;
  ContextEnvironment ce;
  ce.exportStr("dest",fd.addr);
  ce.exportStr("ddest",fd.ddest);
  char buf[32];
  sprintf(buf,"%d",fd.lastResult);
  string reason=ResourceManager::getInstance()->getString(fd.locale,((string)"reason.")+buf);
  if(reason.length()==0)reason=buf;
  ce.exportStr("reason",reason.c_str());
  ce.exportDat("date",fd.date);
  ce.exportStr("msgId",fd.msgId);
  ce.exportInt("lastResult",fd.lastResult);
  ce.exportInt("lastResultGsm",fd.lastResultGsm);
  ce.exportStr("msc",fd.msc);
  ce.exportDat("discharge",time(NULL));

  string key="receipt.";
  key+=fd.scheme;
  key+=".";
  if(fd.lastResult!=Status::DELETED)
  {
    key+="failedFormat";
  }else
  {
    key+="deletedFormat";
  }

  __trace2__("RECEIPT: get formatter for key=%s",key.c_str());


  OutputFormatter* ofFailed=ResourceManager::getInstance()->getFormatter(fd.locale,key);
  if(!ofFailed)
  {
    out="Unknown formatter "+key+" for locale "+fd.locale;
    return;
  }


  try{
    ofFailed->format(out,ga,ce);
  }catch(exception& e)
  {
    __trace2__("FORMATTER: %s",e.what());
    out="Invalid formatter "+key+" for locale "+fd.locale;
  }
}

void StateMachine::formatNotify(const FormatData& fd,std::string& out)
{
  ReceiptGetAdapter ga;
  ContextEnvironment ce;
  ce.exportStr("dest",fd.addr);
  ce.exportStr("ddest",fd.ddest);
  char buf[32];
  sprintf(buf,"%d",fd.lastResult);
  string reason=ResourceManager::getInstance()->getString(fd.locale,((string)"reason.")+buf);
  if(reason.length()==0)reason=buf;
  ce.exportStr("reason",reason.c_str());
  ce.exportDat("date",fd.date);
  ce.exportStr("msgId",fd.msgId);
  ce.exportInt("lastResult",fd.lastResult);
  ce.exportInt("lastResultGsm",fd.lastResultGsm);
  ce.exportStr("msc",fd.msc);
  ce.exportDat("discharge",time(NULL));

  string key="receipt.";
  key+=fd.scheme;
  key+=".";
  key+="notifyFormat";

  __trace2__("RECEIPT: get formatter for key=%s",key.c_str());


  OutputFormatter* ofNotify=ResourceManager::getInstance()->getFormatter(fd.locale,key);
  if(!ofNotify)
  {
    out="Unknown formatter "+key+" for locale "+fd.locale;
    return;
  }

  try{
    ofNotify->format(out,ga,ce);
  }catch(exception& e)
  {
    __trace2__("FORMATTER: %s",e.what());
    out="Invalid formatter "+key+" for locale "+fd.locale;
  }
}


int StateMachine::Execute()
{
  Tuple t;
  StateType st;
  for(;;)
  {
    eq.selectAndDequeue(t,&isStopping);
    if(isStopping)break;
    try{
      switch(t.command->cmdid)
      {
        case SUBMIT:st=submit(t);break;
        case DELIVERY_RESP:st=deliveryResp(t);break;
        case FORWARD:st=forward(t);break;
        case ALERT:st=alert(t);break;
        case REPLACE:st=replace(t);break;
        case QUERY:st=query(t);break;
        case CANCEL:st=cancel(t);break;
        default:
          __warning2__("UNKNOWN COMMAND:%d",t.command->cmdid);
          st=ERROR_STATE;
          break;
      }
      __trace2__("change state for %lld to %d",t.msgId,st);
      eq.changeState(t.msgId,st);
    }catch(exception& e)
    {
      __warning2__("StateMachine::exception %s",e.what());
    }
    catch(...)
    {
      __warning__("StateMachine::unknown exception");
    }
  }
  __trace__("exit state machine");
  return 0;
}

Hash<RegExp*> reCache;
Mutex reCacheMtx;
static struct CacheKiller{
~CacheKiller()
{
  char* key;
  RegExp* re;
  reCache.First();
  while(reCache.Next(key,re))
  {
    delete re;
  }
  reCache.Empty();
}
}ck;

bool checkSourceAddress(const std::string& pattern,const Address& src)
{
  char buf[32];
  src.toString(buf,sizeof(buf));
  MutexGuard g(reCacheMtx);
  using namespace smsc::util::regexp;
  RegExp **reptr=reCache.GetPtr(pattern.c_str());
  RegExp *re;
  if(!reptr)
  {
    re=new RegExp();
    if(!re->Compile(pattern.c_str(),OP_OPTIMIZE|OP_STRICT))
    {
      smsc_log_error(smsc::logger::Logger::getInstance("smsc.system.StateMachine"), "Failed to compile address range regexp");
    }
    reCache.Insert(pattern.c_str(),re);
  }else
  {
    re=*reptr;
  }
  SMatch m[10];
  int n=10;
  return re->Match(buf,m,n);
}

struct Directive{
  int start;
  int end;
  Directive():start(0),end(0){}
  Directive(int st,int en):start(st),end(en){}
  Directive(const Directive& d):start(d.start),end(d.end){}
};


void StateMachine::processDirectives(SMS& sms,Profile& p,Profile& srcprof)
{
  const char *body="";
  TmpBuf<char,256> tmpBuf(0);
  unsigned int len=0;
  int dc=sms.getIntProperty(Tag::SMPP_DATA_CODING);
  if(dc==DataCoding::BINARY)return;
  bool udhi=(sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)==0x40;
  int udhiLen=0;
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    SMS tmp(sms);
    extractSmsPart(&tmp,0);
    if(tmp.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      const char* tmpBody=tmp.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
      char *dst=tmpBuf.getSize(len);
      body=dst;
      memcpy(dst,tmpBody,len);
    }else
    {
      const char* tmpBody=tmp.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
      char *dst=tmpBuf.getSize(len);
      body=dst;
      memcpy(dst,tmpBody,len);
    }
  }else
  {
    if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      body=sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    }else
    {
      body=sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
    }
  }
  unsigned int olen=len;
  if(udhi)
  {
    udhiLen=(*(const unsigned char*)body)+1;
    len-=udhiLen;
    body+=udhiLen;
  }
  if(len==0)return;
  bool hasDirectives=false;
  switch(sms.getIntProperty(Tag::SMPP_DATA_CODING))
  {
    case DataCoding::SMSC7BIT:
    case DataCoding::LATIN1:
      hasDirectives=*body=='#';
      break;
    case DataCoding::UCS2:
    {
      unsigned short tmp;
      memcpy(&tmp,body,2);
      hasDirectives=(tmp=='#');
      break;
    }
  }
  if(!hasDirectives)return;
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))throw Exception("Directive found in multipart message");
  static const char *escchars="[]{}^~\\|";
  TmpBuf<char,256> bufPtr(len*2+1);
  char *buf=bufPtr.get();

  len=getSmsText(&sms,buf,len*2+1);
  if(((int)len)<0)throw Exception("message too long\n");

  Array<Directive> offsets;
  /*
  RegExp def("/#def\\s+(\\d+)#/i");
  __require__(def.LastError()==regexp::errNone);
  RegExp tmpl("/#template=(.*?)#/i");
  __require__(tmpl.LastError()==regexp::errNone);
  RegExp tmplparam("/\\s*\\{(\\w+)\\}=(\".*?\"|[^{\\s]+)/s");
  __require__(tmplparam.LastError()==regexp::errNone);
  RegExp unkdir("/#.*?#/");
  __require__(unkdir.LastError()==regexp::errNone);
  */
  SMatch m[10];
  int i=0;

  ContextEnvironment ce;
  ReceiptGetAdapter ga;

  string tmplname;

  int tmplstart,tmpllen;

  while(i<len && buf[i]=='#')
  {
    int n=10;
    if(dreAck.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      __trace__("DIRECT: ack found");
      sms.setDeliveryReport(REPORT_ACK);
      offsets.Push(Directive(m[0].start,m[0].end));
      i=m[0].end;
    }else
    if(dreNoAck.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      __trace__("DIRECT: noack found");
      sms.setDeliveryReport(REPORT_NOACK);
      offsets.Push(Directive(m[0].start,m[0].end));
      i=m[0].end;
    }else
    if(dreHide.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      __trace__("DIRECT: hide");
      if(srcprof.hideModifiable)
        sms.setIntProperty(Tag::SMSC_HIDE,1);
      else
      {
        __trace__("DIRECT: error, hide is not modifiable");
      }
      offsets.Push(Directive(m[0].start,m[0].end));
      i=m[0].end;
    }else
    if(dreUnhide.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      __trace__("DIRECT: unhide");
      if(srcprof.hideModifiable)
        sms.setIntProperty(Tag::SMSC_HIDE,0);
      else
      {
        __trace__("DIRECT: error, hide is not modifiable");
      }
      offsets.Push(Directive(m[0].start,m[0].end));
      i=m[0].end;
    }else
    if(dreNoTrans.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      __trace__("DIRECT: notrans");
      sms.setIntProperty(Tag::SMSC_TRANSLIT,0);
      offsets.Push(Directive(m[0].start,m[0].end));
      i=m[0].end;
    }else
    if(dreFlash.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      if(!sms.hasIntProperty(Tag::SMSC_FORCE_DC))
      {
        sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,1);
      }
      offsets.Push(Directive(m[0].start,m[0].end));
      i=m[0].end;
    }else
    if(dreDef.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      int t=atoi(buf+m[1].start);
      int mnts=0;
      if(m[2].start!=-1)mnts=atoi(buf+m[2].start+1);
      if(t>999)t=999;
      t*=60;
      t+=mnts;
      __trace2__("DIRECT: %*s, t=%d",m[0].end-m[0].start,buf+m[0].start,t);
      sms.setNextTime(time(NULL)+t*60);
      offsets.Push(Directive(m[0].start,m[0].end));
      i=m[0].end;
    }else
    if(dreTemplate.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      tmplname.assign(buf+m[1].start,m[1].end-m[1].start);
      __trace2__("DIRECT: template=%s",tmplname.c_str());
      tmplname="templates."+tmplname;
      OutputFormatter *f=ResourceManager::getInstance()->getFormatter(p.locale,tmplname);
      int j=m[0].end;
      n=10;
      string name,value;
      while(dreTemplateParam.MatchEx(buf,buf+j,buf+len,m,n))
      {
        name.assign(buf+m[1].start,m[1].end-m[1].start);
        value.assign
        (
          buf+m[2].start+(buf[m[2].start]=='"'?1:0),
          m[2].end-m[2].start-(buf[m[2].start]=='"'?2:0)
        );
        __trace2__("DIRECT: found template param %s=%s",name.c_str(),value.c_str());
        if(f)
        {
          int et=f->getEntityType(name.c_str());
          switch(et)
          {
            case ET_DATE:
            {
              int day,month,year,hh,mm,ss,n;
              time_t t;
              if(sscanf(value.c_str(),SMSC_DBSME_IO_DEFAULT_PARSE_PATTERN,
                &day,&month,&year,&hh,&mm,&ss,&n)!=6)
              {
                sscanf(value.c_str(),"%d",&t);
              }else
              {
                tm m;
                m.tm_sec=ss;
                m.tm_min=mm;
                m.tm_hour=hh;
                m.tm_mday=day;
                m.tm_mon=month;
                m.tm_year=year-1900;
                t=mktime(&m);
              }
              ce.exportDat(name.c_str(),t);
            }break;
            default:
              ce.exportStr(name.c_str(),value.c_str());
              break;
          }
        }
        j=m[0].end;
      }
      Directive d(i,j);
      tmplstart=i;
      tmpllen=j-i;
      offsets.Push(d);
      i=j;
    }else
    if(dreUnknown.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      i=m[0].end;
    }else
    {
      break;
    }
  }
  if(offsets.Count()==0)return;
  if(tmplname.length() && i<len)
  {
    Directive d(i,len-i);
    offsets.Push(d);
  }
  for(i=offsets.Count()-1;i>=0;i--)
  {
    if(offsets[i-1].end==offsets[i].start)
    {
      offsets[i-1].end=offsets[i].end;
      offsets.Delete(i);
    }
  }
  if(sms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::SMSC7BIT)
  {
    int pos=0;
    int fix=0;
    for(int i=0;i<=len;i++)
    {
      if(offsets[pos].start==i)
      {
        offsets[pos].start+=fix;
      }
      if(offsets[pos].end==i)
      {
        offsets[pos].end+=fix;
        pos++;
        if(pos==offsets.Count())break;
      }
      if(strchr(escchars,buf[i]))fix++;
    }
  }
  if(sms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
  {
    for(i=0;i<offsets.Count();i++)
    {
      offsets[i].start*=2;
      offsets[i].end*=2;
    }
  }
  string newtext;
  if(tmplname.length())
  {
    OutputFormatter *f=ResourceManager::getInstance()->getFormatter(p.locale,tmplname);
    if(!f)
    {
      __warning2__("template %s not found for locale %s",tmplname.c_str(),p.locale.c_str());
      newtext.assign(buf+tmplstart,tmpllen);
    }else
    {
      try{
        f->format(newtext,ga,ce);
      }catch(exception& e)
      {
        __warning2__("failed to format template %s for locale %s: %s",tmplname.c_str(),p.locale.c_str(),e.what());
        newtext.assign(buf+tmplstart,tmpllen);
      }
    }
    udhi=false;
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x40));
  }

  TmpBuf<char,256> newBodyPtr(olen*3+newtext.length()*3);
  char* newBody=newBodyPtr.get();
  char *ptr=newBody;
  if(udhi)
  {
    memcpy(ptr,body-udhiLen,udhiLen);
    ptr+=udhiLen;
  }
  memcpy(ptr,body,offsets[0].start);
  ptr+=offsets[0].start;
  for(i=0;i<offsets.Count()-1;i++)
  {
    memcpy(ptr,body+offsets[i].end,offsets[i+1].start-offsets[i].end);
    ptr+=offsets[i+1].start-offsets[i].end;
  }
  memcpy(ptr,body+offsets[i].end,olen-udhiLen-offsets[i].end);
  int newlen=(ptr-newBody)+olen-udhiLen-offsets[i].end;
  if(!udhi)udhiLen=0;
  if(newtext.length())
  {
    bool hb=hasHighBit(newtext.c_str(),newtext.length());
    if(dc!=DataCoding::UCS2 && hb)
    {
      if(dc==DataCoding::LATIN1)
      {
        auto_ptr<short> b(new short[newlen-udhiLen]);
        ConvertMultibyteToUCS2(newBody+udhiLen,newlen-udhiLen,b.get(),(newlen-udhiLen)*2,CONV_ENCODING_CP1251);
        memcpy(newBody+udhiLen,b.get(),(newlen-udhiLen)*2);
      }else //SMSC7BIT
      {
        TmpBuf<char,256> x(newlen-udhiLen+1);
        int cvtlen=ConvertSMSC7BitToLatin1(newBody+udhiLen,newlen-udhiLen,x.get());
        ConvertMultibyteToUCS2
        (
          x.get(),
          cvtlen,
          (short*)newBody+udhiLen,
          (newlen-udhiLen)*2,
          CONV_ENCODING_CP1251
        );
      }
      sms.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::UCS2);
      dc=DataCoding::UCS2;
      newlen=2*newlen-udhiLen;
    }
    if(hb || dc==DataCoding::UCS2)
    {
      auto_ptr<short> nt(new short[newtext.length()+1]);
      __trace2__("DIRECT: newtext=%s, newtext.length=%d, nt.get()=%p",newtext.c_str(),newtext.length(),nt.get());
      ConvertMultibyteToUCS2
      (
        newtext.c_str(),
        newtext.length(),
        nt.get(),
        newtext.length()*2,
        CONV_ENCODING_CP1251
      );
      memcpy(newBody+newlen,nt.get(),newtext.length()*2);
      newlen+=newtext.length()*2;
    }else
    {
      if(dc==DataCoding::LATIN1)
      {
        memcpy(newBody+newlen,newtext.c_str(),newtext.length());
        newlen+=newtext.length();
      }else
      {
        TmpBuf<char,256> nt(newtext.length()*2);
        int cvtlen=ConvertLatin1ToSMSC7Bit(newtext.c_str(),newtext.length(),nt.get());
        memcpy(newBody+newlen,nt.get(),cvtlen);
        newlen+=cvtlen;
      }
    }
  }
  __require__(newlen>=0 && newlen<=65535);
  __trace2__("DIRECT: newlen=%d",newlen);
  //#def N# #ack# #noack# #template=name# {name}="value"
  if(newlen>255)
  {
    sms.getMessageBody().dropProperty(Tag::SMPP_SHORT_MESSAGE);
    sms.getMessageBody().dropProperty(Tag::SMSC_RAW_SHORTMESSAGE);
    sms.setIntProperty(Tag::SMPP_SM_LENGTH,0);
    sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,newBody,newlen);
  }else
  {
    if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,newBody,newlen);
    }else
    {
      sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,newBody,newlen);
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,newlen);
    }
  }
}


StateType StateMachine::submit(Tuple& t)
{
  __require__(t.state==UNKNOWN_STATE || t.state==ENROUTE_STATE || t.state==ERROR_STATE);

  SmeProxy *src_proxy,*dest_proxy=0;

  src_proxy=t.command.getProxy();

  __require__(src_proxy!=NULL);

  SMS* sms = t.command->get_sms();

  sms->setSourceSmeId(t.command->get_sourceId());

  if(!sms->Invalidate(__FILE__,__LINE__))
  {
    warn2(smsLog, "Invalidate of %lld failed\n",t.msgId);
    submitResp(t,sms,Status::SUBMITFAIL);
    return ERROR_STATE;
  }

  uint32_t dialogId =  t.command->get_dialogId();
  sms->dialogId=dialogId;

  debug2(smsLog, "SBM: Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
    t.msgId,dialogId,
    sms->getOriginatingAddress().toString().c_str(),
    sms->getDestinationAddress().toString().c_str(),
    src_proxy->getSystemId()
  );

  if(t.state==ERROR_STATE)
  {
    warn1(smsLog, "SMS in error state\n");
    submitResp(t,sms,Status::SUBMITFAIL);
    return ERROR_STATE;
  }


  ////
  //
  //  SMS validity checks started
  //

  if(sms->getNextTime()==-1)
  {
    submitResp(t,sms,Status::INVSCHED);
    warn2(smsLog, "SBM: invalid schedule time Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) {
    // force forward mode
    sms->setIntProperty(Tag::SMPP_ESM_CLASS, (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0xFC)|0x02);
  }

  if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) &&
     sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE)   &&
     sms->getIntProperty(Tag::SMPP_SM_LENGTH)!=0)
  {
    submitResp(t,sms,Status::SUBMITFAIL);

    warn2(smsLog, "SBM: both short_message and payload present Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::LATIN1 &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::UCS2 &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::BINARY &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::SMSC7BIT)
  {
    submitResp(t,sms,Status::INVDCS);
    warn2(smsLog, "SBM: invalid datacoding %d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING),
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
  {
    unsigned len;
    const unsigned char* msg;
    if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      msg=(const unsigned char*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    }
    else
    {
      msg=(const unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
    }
    if(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
    {
      len-=*msg+1;
    }
    ////
    // Unicode message with odd length
    if(len&1)
    {
      submitResp(t,sms,Status::INVMSGLEN);
      warn2(smsLog, "SBM: invalid message length for unicode (%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
        len,
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId()
      );
      return ERROR_STATE;
    }
  }


  if(sms->getValidTime()==-1)
  {
    submitResp(t,sms,Status::INVEXPIRY);
    warn2(smsLog, "SBM: invalid valid time Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(src_proxy->getSourceAddressRange().length() &&
     !checkSourceAddress(src_proxy->getSourceAddressRange(),sms->getOriginatingAddress()))
  {
    submitResp(t,sms,Status::INVSRCADR);
    warn2(smsLog, "SBM: invalid source address for range '%s' Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      src_proxy->getSourceAddressRange().c_str(),
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if((
      (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1 ||
      (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2
     ) &&
     sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
  {
    submitResp(t,sms,Status::SUBMITFAIL);
    warn2(smsLog, "SBM: attempt to send concatenated sms in dg or tr mode. Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->originatingDescriptor.mscLength || sms->originatingDescriptor.imsiLength)
  {
    char buf[MAX_ADDRESS_VALUE_LENGTH+MAX_ADDRESS_VALUE_LENGTH+12];
    sprintf(buf,"Org: %s/%s",sms->originatingDescriptor.msc,sms->originatingDescriptor.imsi);
    sms->setStrProperty(Tag::SMSC_DESCRIPTORS,buf);
  }


  time_t now=time(NULL);
  sms->setSubmitTime(now);

  int dest_proxy_index;
  // route sms
  //SmeProxy* dest_proxy = 0;
  Address dst;
  __trace2__("AliasToAddress: %s",sms->getDestinationAddress().toString().c_str());
  if(smsc->AliasToAddress(sms->getDestinationAddress(),dst))
  {
    __trace2__("ALIAS:%s->%s",sms->getDestinationAddress().toString().c_str(),dst.toString().c_str());
  }
  else
  {
    dst=sms->getDestinationAddress();
  }
  sms->setDealiasedDestinationAddress(dst);
  smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms->getOriginatingAddress());
  __trace2__("SUBMIT_SM: lookup %s, result: %d,%d",sms->getOriginatingAddress().toString().c_str(),
    profile.reportoptions,profile.codepage);

  smsc::profiler::Profile orgprofile=profile;

  __trace2__("SUBMIT_SM: profile options=%d",profile.reportoptions);
  if((sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x03)==0x01 ||
     (sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x03)==0x02 ||
     (sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x10)==0x10 ||
     sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST)
    )
  {
    sms->setDeliveryReport(0);
  }else
  {
    sms->setDeliveryReport(profile.reportoptions);
  }

  if(sms->getDeliveryReport()==0 &&
     (sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x03)==0 &&
     (sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x10)==0 &&
     sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
  {
    sms->setDeliveryReport(ProfileReportOptions::ReportFinal);
  }

  sms->setIntProperty(Tag::SMSC_HIDE,profile.hide);
  if(!sms->hasIntProperty(Tag::SMSC_TRANSLIT))
  {
    sms->setIntProperty(Tag::SMSC_TRANSLIT,profile.translit);
    debug2(smsLog,"msgId=%lld, set translit to %d",t.msgId,sms->getIntProperty(Tag::SMSC_TRANSLIT));
  }


  Profile srcprof=profile;

  int profileMatchType;
  std::string profileMatchAddress;
  profile=smsc->getProfiler()->lookupEx(dst,profileMatchType,profileMatchAddress);

  if(profileMatchType==ProfilerMatchType::mtExact)
  {
    debug2(smsLog,"msgId=%lld exact profile match, set translit to 1",t.msgId);
    sms->setIntProperty(Tag::SMSC_TRANSLIT,1);
  }

  if(!smsCanBeTransliterated(sms))
  {
    debug2(smsLog,"msgId=%lld cannot be transliterated, set translit to 0",t.msgId);
    sms->setIntProperty(Tag::SMSC_TRANSLIT,0);
  }


  bool diverted=false;

  int divertFlags=(profile.divertActive        ?DF_UNCOND:0)|
                  (profile.divertActiveAbsent  ?DF_ABSENT:0)|
                  (profile.divertActiveBlocked ?DF_BLOCK :0)|
                  (profile.divertActiveBarred  ?DF_BARRED:0)|
                  (profile.divertActiveCapacity?DF_CAPAC :0);
  if(divertFlags && profile.divert.length()!=0 &&
     !sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) &&
     sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=3
    )
  {
    debug2(smsLog, "divert for %s found",dst.toString().c_str());
    Address divDst;
    try{
      divDst=Address(profile.divert.c_str());
    }catch(...)
    {
      warn2(smsLog, "INVALID DIVERT FOR %s - ADDRESS:%s",dst.toString().c_str(),profile.divert.c_str());
      goto divert_failed;
    }
    Address tmp;
    if(smsc->AliasToAddress(divDst,tmp))
    {
      debug2(smsLog, "Divert address dealiased:%s->%s",divDst.toString().c_str(),tmp.toString().c_str());
      divDst=tmp;
    }
    smsc::router::RouteInfo ri2;
    SmeProxy* prx;
    int idx;
    if(smsc->routeSms(sms->getOriginatingAddress(),divDst,idx,prx,&ri2,src_proxy->getSmeIndex()))
    {
      if(prx && strcmp(prx->getSystemId(),"MAP_PROXY")!=0)
      {
        warn2(smsLog,"attempt to divert to non-map address:%s->%s",
          sms->getOriginatingAddress().toString().c_str(),divDst.toString().c_str());
        goto divert_failed;
      }
    }
    sms->setStrProperty(Tag::SMSC_DIVERTED_TO,divDst.toString().c_str());
    if(divertFlags&DF_UNCOND)
    {
      diverted=true;
      dst=divDst;
    }

    Profile p=smsc->getProfiler()->lookup(divDst);

    divertFlags|=p.udhconcat?DF_UDHCONCAT:0;
    int ddc=p.codepage;
    if(!(profile.codepage&DataCoding::UCS2) && (ddc&DataCoding::UCS2))
    {
      ddc=profile.codepage;
      __trace2__("divert - downgrade dstdc to %d",ddc);
    }

    divertFlags|=(ddc)<<DF_DCSHIFT;

    if(divertFlags&DF_UNCOND)profile=p;

    sms->setIntProperty(Tag::SMSC_DIVERTFLAGS,divertFlags);

  }
  divert_failed:;


  smsc::router::RouteInfo ri;

  ////
  //
  //  Routing here
  //

  bool has_route = smsc->routeSms(sms->getOriginatingAddress(),
                          dst,
                          dest_proxy_index,dest_proxy,&ri,src_proxy->getSmeIndex());

  __trace2__("hide=%s, forceRP=%d",ri.hide?"true":"false",ri.replyPath);

  sms->setRouteId(ri.routeId.c_str());
  if(ri.suppressDeliveryReports)sms->setIntProperty(Tag::SMSC_SUPPRESS_REPORTS,1);
  int prio=sms->getPriority()+ri.priority;
  if(prio>SmeProxyPriorityMax)prio=SmeProxyPriorityMax;
  sms->setPriority(prio);

  //smsc->routeSms(sms,dest_proxy_index,dest_proxy);
  if ( !has_route )
  {
    submitResp(t,sms,Status::NOROUTE);
    char buf1[32];
    char buf2[32];
    sms->getOriginatingAddress().toString(buf1,sizeof(buf2));
    dst.toString(buf2,sizeof(buf2));
    warn2(smsLog, "SBM: no route Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  bool aclCheck=false;
  std::string aclAddr;

  bool fromMap=src_proxy && !strcmp(src_proxy->getSystemId(),"MAP_PROXY");
  bool toMap=dest_proxy && !strcmp(dest_proxy->getSystemId(),"MAP_PROXY");


  if((fromMap || toMap) && !(fromMap && toMap))
  {
    char buf[MAX_ADDRESS_VALUE_LENGTH];
    if(fromMap)
    {
      sms->getOriginatingAddress().getText(buf,sizeof(buf));
    }else
    {
      sms->getDestinationAddress().getText(buf,sizeof(buf));
    }
    aclAddr=buf;
    aclCheck=true;
  }

  if(aclCheck && ri.aclId!=-1 && !smsc->getAclMgr()->isGranted(ri.aclId,aclAddr))
  {
    submitResp(t,sms,Status::NOROUTE);
    char buf1[32];
    char buf2[32];
    sms->getOriginatingAddress().toString(buf1,sizeof(buf1));
    dst.toString(buf2,sizeof(buf2));
    warn2(smsLog, "SBM: acl access denied (aclId=%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      ri.aclId,
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(toMap)
  {
    if(ri.forceDelivery)
    {
      sms->setIntProperty(Tag::SMPP_PRIORITY,3);
    }
  }

  if(fromMap && ( sms->getOriginatingDescriptor().mscLength==0 ||
                  sms->getOriginatingDescriptor().imsiLength==0 ))
  {
    if(!ri.allowBlocked)
    {
      submitResp(t,sms,Status::CALLBARRED);
      char buf1[32];
      char buf2[32];
      sms->getOriginatingAddress().toString(buf1,sizeof(buf1));
      dst.toString(buf2,sizeof(buf2));
      warn2(smsLog, "SBM: call barred Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId()
      );
      return ERROR_STATE;
    }
  }

  SmeInfo dstSmeInfo=smsc->getSmeInfo(dest_proxy_index);

  if(dstSmeInfo.interfaceVersion==0x99)
  {
    sms->setStrProperty(Tag::SMSC_SUPPORTED_LOCALE,orgprofile.locale.c_str());
    sms->setIntProperty(Tag::SMSC_SUPPORTED_CODESET,orgprofile.codepage);
    sms->setStrProperty(Tag::SMSC_IMSI_ADDRESS,sms->getOriginatingDescriptor().imsi);
    sms->setStrProperty(Tag::SMSC_MSC_ADDRESS,sms->getOriginatingDescriptor().msc);
  }

  //__trace2__("SUBMIT_SM: route found, routeId=%s, smeSystemId=%s",ri.routeId.c_str(),ri.smeSystemId.c_str());

  sms->setDestinationSmeId(ri.smeSystemId.c_str());
  sms->setServiceId(ri.serviceId);


  __trace2__("SUBMIT: archivation request for %lld/%d is %s",t.msgId,dialogId,ri.archived?"true":"false");
  sms->setArchivationRequested(ri.archived);
  sms->setBillingRecord(ri.billing);


  sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage);

  if(ri.smeSystemId=="MAP_PROXY" && sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    sms->setIntProperty(Tag::SMSC_TRANSLIT,1);
    if(profile.codepage&smsc::profiler::ProfileCharsetOptions::UssdIn7Bit)
    {
      sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,smsc::profiler::ProfileCharsetOptions::Default);
    }else
    {
      sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,smsc::profiler::ProfileCharsetOptions::Ucs2);
    }
  }else
  {
    sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage&(~smsc::profiler::ProfileCharsetOptions::UssdIn7Bit));
  }

  sms->setIntProperty(Tag::SMSC_UDH_CONCAT,profile.udhconcat);


  int pres=psSingle;

  bool isForwardTo = false;
  if( ri.forwardTo.length() > 0 && ri.smeSystemId=="MAP_PROXY" )
  {
    sms->setStrProperty( Tag::SMSC_FORWARD_MO_TO, ri.forwardTo.c_str());

    // force forward(transaction) mode
    //sms->setIntProperty( Tag::SMPP_ESM_CLASS, sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x02 );
    isForwardTo = true;

    //sms->getMessageBody().dropIntProperty(Tag::SMSC_MERGE_CONCAT);
    //sms->getMessageBody().dropProperty(Tag::SMSC_DC_LIST);
  }

  bool generateDeliver=true; // do not generate in case of merge-concat
  bool allowCreateSms=true;

  bool needToSendResp=true;

  bool noPartitionSms=false; // do not call partitionSms if true!


  if(!extractPortsFromUdh(*sms))
  {
    warn2(smsLog,"extractPortsFromUdh failed. msgId=%lld, from %s to %s",
                          t.msgId,
                          sms->getOriginatingAddress().toString().c_str(),
                          sms->getDestinationAddress().toString().c_str());
    submitResp(t,sms,Status::INVOPTPARAMVAL);
    return ERROR_STATE;
  }

  if(!convertSarToUdh(*sms))
  {
    warn2(smsLog,"convertSarToUdh failed. msgId=%lld, from %s to %s",
                          t.msgId,
                          sms->getOriginatingAddress().toString().c_str(),
                          sms->getDestinationAddress().toString().c_str());
    submitResp(t,sms,Status::INVOPTPARAMVAL);
    return ERROR_STATE;
  };


  ////
  //
  //  Merging
  //

  if(sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
  {
    bool firstPiece=sms->getIntProperty(Tag::SMSC_MERGE_CONCAT)==1;
    unsigned char *body;
    unsigned int len;
    if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      body=(unsigned char*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    }else
    {
      body=(unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
    }
    uint16_t mr;
    uint8_t idx,num;
    bool havemoreudh;
    smsc::util::findConcatInfo(body,mr,idx,num,havemoreudh);

    int dc=sms->getIntProperty(Tag::SMPP_DATA_CODING);
    //bool needrecoding=
    //    (dc==DataCoding::UCS2 && (profile.codepage&ProfileCharsetOptions::Ucs2)!=ProfileCharsetOptions::Ucs2) ||
    //    (dc==DataCoding::LATIN1 && (profile.codepage&ProfileCharsetOptions::Latin1)!=ProfileCharsetOptions::Latin1);

    if(firstPiece) //first piece
    {
      info2(smsLog, "merging sms Id=%lld, first part arrived(%u/%u),mr=%d,dc=%d",t.msgId,idx,num,(int)mr,dc);
      sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)&~0x40);
      string tmp;
      if(!isForwardTo)
      {
        tmp.assign((const char*)body,len);
      }else
      {
        unsigned len;
        const char *body=sms->getBinProperty(Tag::SMSC_MO_PDU,&len);
        tmp.assign(body,len);
        sms->setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::BINARY);
        dc=DataCoding::BINARY;
      }
      sms->setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),tmp.length());
      sms->getMessageBody().dropProperty(Tag::SMPP_SHORT_MESSAGE);
      sms->getMessageBody().dropProperty(Tag::SMSC_RAW_SHORTMESSAGE);
      sms->setIntProperty(Tag::SMPP_SM_LENGTH,0);

      char cibuf[256*2+1];
      ConcatInfo *ci=(ConcatInfo *)cibuf;
      ci->num=1;
      ci->setOff(0,0);
      sms->setBinProperty(Tag::SMSC_CONCATINFO,(char*)ci,1+2*num);
      generateDeliver=false;

      char dc_list[256];
      memset(dc_list,0,num);
      __trace2__("dc_list[%d]=%d",idx-1,dc);
      dc_list[idx-1]=dc;

      sms->setBinProperty(Tag::SMSC_DC_LIST,dc_list,num);


      if(sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
      {
        char buf[256];
        memset(buf,0,sizeof(buf));
        buf[idx-1]=sms->getMessageReference();
        sms->setBinProperty(Tag::SMSC_UMR_LIST,buf,num);
        buf[idx-1]=1;
        sms->setBinProperty(Tag::SMSC_UMR_LIST_MASK,buf,num);
      }

      char buf[64];
      sprintf(buf,"%lld",t.msgId);
      SmscCommand resp = SmscCommand::makeSubmitSmResp
                           (
                             buf,
                             dialogId,
                             Status::OK,
                             sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                           );
      try{
        src_proxy->putCommand(resp);
      }catch(...)
      {
        warn2(smsLog, "SBM: failed to put response command SUBMIT_OK Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
          t.msgId,dialogId,
          sms->getOriginatingAddress().toString().c_str(),
          sms->getDestinationAddress().toString().c_str(),
          src_proxy->getSystemId(),
          ri.smeSystemId.c_str()
        );
      }
      needToSendResp=false;
    }else
    {
      info2(smsLog, "merging sms Id=%lld, next part arrived(%u/%u), mr=%d,dc=%d",t.msgId,idx,num,(int)mr,dc);
      SMS* smsptr=smsc->getTempStore().Extract(t.msgId);
      SMS _sms;
      SMS& newsms=smsptr?*smsptr:_sms;
      bool tempStore=false;
      if(!smsptr)
      {
        try{
          store->retriveSms(t.msgId,newsms);
        }catch(...)
        {
          warn2(smsLog, "sms with id %lld not found or store error",t.msgId);
          submitResp(t,sms,Status::SYSERR);
          return ERROR_STATE;
        }
      }
      if(!newsms.hasIntProperty(Tag::SMSC_MERGE_CONCAT))
      {
        warn2(smsLog, "smsId=%lld:one more part of concatenated message received, but all parts are collected.",t.msgId);
        submitResp(t,sms,Status::SUBMITFAIL);
        if(smsptr)delete smsptr;
        return ERROR_STATE;
      }
      unsigned int newlen;
      unsigned char *newbody;
      unsigned int cilen;
      ConcatInfo *ci=(ConcatInfo*)newsms.getBinProperty(Tag::SMSC_CONCATINFO,&cilen);
      newbody=(unsigned char*)newsms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&newlen);
      if(!isForwardTo)
      {
        __require__(ci!=NULL);
        for(int i=0;i<ci->num;i++)
        {
          uint16_t mr0;
          uint8_t idx0,num0;
          bool havemoreudh0;
          smsc::util::findConcatInfo(newbody+ci->getOff(i),mr0,idx0,num0,havemoreudh0);
          if(idx0==idx || num0!=num || mr0!=mr)
          {
            //submitResp(t,sms,Status::INVOPTPARAMVAL);

            sms->setLastResult(Status::DUPLICATECONCATPART);
            smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
            char buf[64];
            sprintf(buf,"%lld",t.msgId);
            SmscCommand resp = SmscCommand::makeSubmitSmResp
                                 (
                                   buf,
                                   t.command->get_dialogId(),
                                   Status::OK,
                                   sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                                 );
            try{
              t.command.getProxy()->putCommand(resp);
            }catch(...)
            {
              __warning__("SUBMIT_SM: failed to put response command");
            }


            warn2(smsLog, "Duplicate or invalid concatenated message part for id=%lld(idx:%d-%d,num:%d-%d,mr:%d-%d)",t.msgId,idx0,idx,num0,num,mr0,mr);
            if(smsptr)delete smsptr;
            return ENROUTE_STATE;
          }
        }
      }else
      {
        if(newsms.hasBinProperty(Tag::SMSC_DC_LIST))
        {
          unsigned len;
          newsms.getBinProperty(Tag::SMSC_DC_LIST,&len);
          if(len!=num)
          {
            warn2(smsLog, "smsId=%lld: different number of parts detected %d!=%d.",t.msgId,len,num);
            submitResp(t,sms,Status::INVOPTPARAMVAL);
            if(smsptr)delete smsptr;
            return ERROR_STATE;
          }
        }
        body=(unsigned char*)sms->getBinProperty(Tag::SMSC_MO_PDU,&len);
        dc=DataCoding::BINARY;
      }
      /*
      if(!isForwardTo && sms->getIntProperty(Tag::SMPP_DATA_CODING)!=newsms.getIntProperty(Tag::SMPP_DATA_CODING))
      {
        smsc_log_error(smsLog, "different data coding of parts of concatenated message (%d!=%d) id=%lld",
          sms->getIntProperty(Tag::SMPP_DATA_CODING),newsms.getIntProperty(Tag::SMPP_DATA_CODING),t.msgId);
        submitResp(t,sms,Status::INVOPTPARAMVAL);
        if(smsptr)delete smsptr;
        return ERROR_STATE;
      }
      */

      if(newsms.hasBinProperty(Tag::SMSC_UMR_LIST))
      {
        unsigned len;
        unsigned char* umrList=(unsigned char*)newsms.getBinProperty(Tag::SMSC_UMR_LIST,&len);
        if(idx<=len)
        {
          umrList[idx-1]=sms->getMessageReference();
          newsms.setBinProperty(Tag::SMSC_UMR_LIST,(const char*)umrList,len);

          if(newsms.hasBinProperty(Tag::SMSC_UMR_LIST_MASK))
          {
            unsigned mlen;
            char* mask=(char*)newsms.getBinProperty(Tag::SMSC_UMR_LIST_MASK,&mlen);
            if(idx<=len)
            {
              mask[idx-1]=1;
              sms->setBinProperty(Tag::SMSC_UMR_LIST_MASK,mask,mlen);
            }
          }

        }
      }

      if(newsms.hasBinProperty(Tag::SMSC_DC_LIST))
      {
        unsigned len;
        unsigned char* dcList=(unsigned char*)newsms.getBinProperty(Tag::SMSC_DC_LIST,&len);
        if(idx<=len)
        {
          __trace2__("dc_list[%d]=%d",idx-1,dc);
          dcList[idx-1]=dc;
          newsms.setBinProperty(Tag::SMSC_DC_LIST,(const char*)dcList,len);
        }
      }

      string tmp;
      tmp.assign((const char*)newbody,newlen);
      tmp.append((const char*)body,len);
      ci->setOff(ci->num,newlen);
      ci->num++;
      bool allParts=ci->num==num;
      if(allParts) // all parts received
      {
        // now resort parts

        debug2(smsLog,"all parts received, send kill cache item:msgId=%lld;oa=%s;da=%s;mr=%d",t.msgId,sms->getOriginatingAddress().toString().c_str(),sms->getDestinationAddress().toString().c_str(),(int)mr);
        smsc->submitMrKill(sms->getOriginatingAddress(),sms->getDestinationAddress(),mr);

        vector<int> order;
        bool rightOrder=true;
        bool totalMoreUdh=false;
        bool differentDc=false;
        bool haveBinDc=isForwardTo?true:sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::BINARY;

        if(!isForwardTo)
        {
          unsigned char* dcList=0;
          unsigned dcListLen=0;

          if(newsms.hasBinProperty(Tag::SMSC_DC_LIST))
          {
            dcList=(unsigned char*)newsms.getBinProperty(Tag::SMSC_DC_LIST,&dcListLen);
          }

          for(int i=0;i<ci->num;i++)
          {
            uint16_t mr0;
            uint8_t idx0,num0;
            bool havemoreudh0;
            smsc::util::findConcatInfo((unsigned char*)tmp.c_str()+ci->getOff(i),mr0,idx0,num0,havemoreudh0);
            __trace2__("SUBMIT_SM: merge check order %d:%d",i,idx0);
            totalMoreUdh=totalMoreUdh || havemoreudh0;
            order.push_back(idx0);
            rightOrder=rightOrder && idx0==i+1;

            if(dcList)
            {
              if(i>0)
              {
                differentDc=differentDc || dcList[i-1]!=dcList[i];
              }
              haveBinDc=haveBinDc || dcList[i]==DataCoding::BINARY;
            }
          }
          if(!rightOrder)
          {
            __trace__("SUBMIT_SM: not right order - need to reorder");
            //average number of parts is 2-3. so, don't f*ck mind with quick sort and so on.
            //maximum is 255.  65025 comparisons. not very good, but not so bad too.
            string newtmp;
            uint16_t newci[256];

            for(unsigned i=1;i<=num;i++)
            {
              for(unsigned j=0;j<num;j++)
              {
                if(order[j]==i)
                {
                  int partlen=j==num-1?tmp.length()-ci->getOff(j):ci->getOff(j+1)-ci->getOff(j);
                  newci[i-1]=newtmp.length();
                  newtmp.append(tmp.c_str()+ci->getOff(j),partlen);

                }
              }
            }
            //memcpy(ci->off,newci,ci->num*2);
            for(int i=0;i<ci->num;i++)
            {
              ci->setOff(i,newci[i]);
            }
            tmp=newtmp;
          }
        }//isForwardTo
        newsms.setIntProperty(Tag::SMSC_MERGE_CONCAT,3); // final state
        if(!totalMoreUdh && !differentDc && !haveBinDc)//make single text message
        {
          string newtmp;
          for(int i=1;i<=ci->num;i++)
          {
            int partlen=i==num?tmp.length()-ci->getOff(i-1):ci->getOff(i)-ci->getOff(i-1);
            const unsigned char * part=(const unsigned char *)tmp.c_str()+ci->getOff(i-1);
            partlen-=*part+1;
            part+=*part+1;
            newtmp.append((const char*)part,partlen);
          }
          tmp=newtmp;
          newsms.messageBody.dropProperty(Tag::SMSC_CONCATINFO);
          newsms.messageBody.dropIntProperty(Tag::SMSC_MERGE_CONCAT);
          newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),(int)tmp.length());
          try{
            processDirectives(newsms,profile,srcprof);
          }catch(...)
          {
            warn2(smsLog, "Failed to process directives for sms with id=%lld",t.msgId);
            submitResp(t,&newsms,Status::SUBMITFAIL);
            return ERROR_STATE;
          }
          if(ri.smeSystemId=="MAP_PROXY")
          {
            if(!newsms.hasIntProperty(Tag::SMSC_DSTCODEPAGE))
            {
              newsms.setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage);
            }
            if(!newsms.getIntProperty(Tag::SMSC_TRANSLIT) || !smsCanBeTransliterated(&newsms))
            {
              if(newsms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
              {
                newsms.setIntProperty(Tag::SMSC_DSTCODEPAGE,DataCoding::UCS2);
              }
            }
            pres=partitionSms(&newsms);
            if(pres==psMultiple)
            {
              uint8_t msgref=smsc->getNextMR(dst);
              sms->setConcatMsgRef(msgref);
              sms->setConcatSeqNum(0);
            }
            noPartitionSms=true;
          }
        }else
        {
          newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),(int)tmp.length());
          if(!isForwardTo)
          {
            newsms.setIntProperty(Tag::SMPP_ESM_CLASS,newsms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x40);
          }
        }
      }else
      {
        newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),(int)tmp.length());
      }
      if(smsptr)
      {
        smsc->getTempStore().AddPtr(t.msgId,smsptr);
      }else
      {
        try{
          store->replaceSms(t.msgId,newsms);
        }catch(...)
        {
          warn2(smsLog, "Failed to replace sms with id=%lld",t.msgId);
          submitResp(t,&newsms,Status::SUBMITFAIL);
          return ERROR_STATE;
        }
      }
      if(!allParts)
      {
        info2(smsLog, "merging sms %lld, not all parts are here, waiting",t.msgId);
        char buf[64];
        sprintf(buf,"%lld",t.msgId);
        SmscCommand resp = SmscCommand::makeSubmitSmResp
                             (
                               buf,
                               dialogId,
                               Status::OK,
                               sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                             );
        try{
          src_proxy->putCommand(resp);
        }catch(...)
        {
          warn2(smsLog, "SBM: failed to put response command SUBMIT_OK Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
            t.msgId,dialogId,
            sms->getOriginatingAddress().toString().c_str(),
            sms->getDestinationAddress().toString().c_str(),
            src_proxy->getSystemId(),
            ri.smeSystemId.c_str()
          );
        }
        return ENROUTE_STATE;
      }

      *sms=newsms;

      int status=Status::OK;
      /*
      try{
        store->createSms(*sms,t.msgId,smsc::store::CREATE_NEW);
      }catch(...)
      {
        status=Status::SYSERR;
      }
      smsc->getTempStore().Delete(t.msgId);
      */

      char buf[64];
      sprintf(buf,"%lld",t.msgId);

      SmscCommand resp = SmscCommand::makeSubmitSmResp
                           (
                             status==Status::OK?buf:"0",
                             dialogId,
                             status,
                             sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                           );
      try{
        src_proxy->putCommand(resp);
      }catch(...)
      {
        warn2(smsLog, "SBM: failed to put response command SUBMIT_OK Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
          t.msgId,dialogId,
          sms->getOriginatingAddress().toString().c_str(),
          sms->getDestinationAddress().toString().c_str(),
          src_proxy->getSystemId(),
          ri.smeSystemId.c_str()
        );
      }
      needToSendResp=false;

      if(status!=Status::OK)
      {
        sms->lastResult=status;
        smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
        return ERROR_STATE;
      }
      // let deliver begin!
      smsc->registerStatisticalEvent(StatEvents::etSubmitOk,sms);

      allowCreateSms=false;

    }
  }

  //
  //  End of merging
  //
  ////

  bool isDatagram=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1;
  bool isTransaction=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;


  if( !isForwardTo )
  {

    sms->getMessageBody().dropProperty(Tag::SMSC_MO_PDU);

    ////
    //
    // Override delivery mode if specified in config and default mode in sms
    //

    if( ri.deliveryMode != smsc::sms::SMSC_DEFAULT_MSG_MODE )
    {
      int esmcls = sms->getIntProperty( Tag::SMPP_ESM_CLASS );
      if( (esmcls&0x3) == smsc::sms::SMSC_DEFAULT_MSG_MODE )
      {
        // allow override
        sms->setIntProperty( Tag::SMPP_ESM_CLASS, esmcls|(ri.deliveryMode&0x03) );
      }
    }

    ////
    //
    //  Directives
    //

    try{
      if(!sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
      {
        processDirectives(*sms,profile,srcprof);
      }
    }catch(std::exception& e)
    {
      __warning2__("Failed to process directives due to exception:%s",e.what());
      submitResp(t,sms,Status::SUBMITFAIL);
      return ERROR_STATE;
    }

    if(sms->getIntProperty(Tag::SMSC_TRANSLIT)==0)
    {
      if(sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
      {
        debug2(smsLog,"msgId=%lld translit set to 0, patch dstcodepage",t.msgId);
        sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,DataCoding::UCS2);
        if(sms->hasIntProperty(Tag::SMSC_DIVERTFLAGS))
        {
          int df=sms->getIntProperty(Tag::SMSC_DIVERTFLAGS);
          df&=~(0xff<<DF_DCSHIFT);
          df|=(DataCoding::UCS2)<<DF_DCSHIFT;
          sms->setIntProperty(Tag::SMSC_DIVERTFLAGS,df);
        }
      }
    }


    if(sms->getValidTime()==0 || sms->getValidTime()>now+maxValidTime)
    {
      sms->setValidTime(now+maxValidTime);
      __trace2__("maxValidTime=%d",maxValidTime);
    }

    __trace2__("Valid time for sms %lld=%u",t.msgId,(unsigned int)sms->getValidTime());



    if(sms->getNextTime()>now+maxValidTime || sms->getNextTime()>sms->getValidTime())
    {
      submitResp(t,sms,Status::INVSCHED);
      warn2(smsLog, "SBM: invalid schedule time(%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
        sms->getNextTime(),
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId()
      );
      return ERROR_STATE;
    }

    __trace2__("SUBMIT_SM: after processDirectives - delrep=%d, sdt=%d",(int)sms->getDeliveryReport(),sms->getNextTime());

    //__trace2__("SUBMIT_SM: dest_addr_subunit=%d",sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT));

    /*
    // ���� ����� ��������������� ��� ��� ����� ��� �� ������
       ����� �� ��� �������.
    if(ri.smeSystemId=="MAP_PROXY" && sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)==0x03)
    {
      unsigned len=sms->getIntProperty(Tag::SMPP_SM_LENGTH);
      if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
      {
        sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
      }
      __trace2__("SUBMIT_SM: sim specific, len=%d",len);
      if(len>140)
      {
        submitResp(t,sms,Status::INVMSGLEN);
        warn2(smsLog, "SBM: invalid message length(%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
          len,
          t.msgId,dialogId,
          sms->getOriginatingAddress().toString().c_str(),
          sms->getDestinationAddress().toString().c_str(),
          src_proxy->getSystemId(),
          ri.smeSystemId.c_str()
        );
        return ERROR_STATE;
      }
    }
    */
    if(ri.smeSystemId=="MAP_PROXY" && sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)==0x03)
    {
      sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,DataCoding::UCS2|DataCoding::LATIN1);
      sms->setIntProperty(Tag::SMSC_UDH_CONCAT,1);
    }



    if(ri.smeSystemId=="MAP_PROXY" &&
       !sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT)&&
       !sms->hasBinProperty(Tag::SMSC_CONCATINFO) &&
       !noPartitionSms
      )
    {
      pres=partitionSms(sms);
    }
    if(pres==psErrorLength)
    {
      submitResp(t,sms,Status::INVMSGLEN);
      unsigned int len;
      const char *msg=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
      warn2(smsLog, "SBM: invalid message length(%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        len,
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId(),
        ri.smeSystemId.c_str()
      );
      return ERROR_STATE;
    }

    if(pres==psErrorUdhi)
    {
      submitResp(t,sms,Status::SUBMITFAIL);
      warn2(smsLog, "SBM: udhi present in concatenated message!!! Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId(),
        ri.smeSystemId.c_str()
      );
      return ERROR_STATE;
    }

    if(pres==psErrorUssd)
    {
      submitResp(t,sms,Status::USSDMSGTOOLONG);
      warn2(smsLog, "SBM: ussd message too long!!! Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId(),
        ri.smeSystemId.c_str()
      );
      return ERROR_STATE;
    }


    if(pres==psMultiple && !noPartitionSms)
    {
      uint8_t msgref=smsc->getNextMR(dst);
      sms->setConcatMsgRef(msgref);

      sms->setConcatSeqNum(0);
    }

    __trace2__("SUBMIT_SM: Replace if present for message %lld=%d",t.msgId,sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG));
    __trace2__("SUBMIT_SM: SMPP_USSD_SERVICE_OP for %lld=%d",t.msgId,sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP));

    //
    // End of checks. Ready to put sms to database
    //
    ////

    ////
    //
    // Traffic Control
    //

    bool ussdSession=false;
    if(sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
    {
      if(sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP)!=USSD_PSSR_IND &&
         !(
           sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP)==USSD_USSR_REQ &&
           sms->hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)
          )
        )
      {
        ussdSession=true;
      }
    }
    if(!ussdSession && !smsc->allowCommandProcessing(t.command))
    {
      submitResp(t,sms,Status::THROTTLED);
      warn2(smsLog, "SBM: traffic control denied message Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId(),
        ri.smeSystemId.c_str()
      );
      return ERROR_STATE;
    }

    //
    // End of traffic Control
    //
    ////
  }


  time_t stime=sms->getNextTime();

  ////
  //
  // Store sms to database
  //

  __require__(!(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) &&
                sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE)   &&
                sms->getIntProperty(Tag::SMPP_SM_LENGTH)!=0));


  __trace2__("SUBMIT: isDg=%s, isTr=%s",isDatagram?"true":"false",isTransaction?"true":"false");
  if(!isDatagram && !isTransaction && allowCreateSms)
  {
    try{
      if(sms->getNextTime()<now)
      {
        sms->setNextTime(now);
      }
      if(!sms->Invalidate(__FILE__,__LINE__))
      {
         warn2(smsLog, "Invalidate of %lld failed",t.msgId);
         throw "Invalid sms";
      }
      bool rip=sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG)!=0;
      //if(rip || stime>now || )
      //{
        store->createSms(*sms,t.msgId,rip?smsc::store::SMPP_OVERWRITE_IF_PRESENT:smsc::store::CREATE_NEW);
        sms->createdInStore=true;
      //}else
      /*
      {
        smsc->getTempStore().AddSms(t.msgId,*sms);
        sms->createdInStore=false;
      }
      */

    }catch(...)
    {
      __warning2__("failed to create sms with id %lld",t.msgId);
      submitResp(t,sms,Status::SYSERR);
      return ERROR_STATE;
    }
  }

  //
  // stored
  //
  ////



  if(!isDatagram && !isTransaction && sms->createdInStore && needToSendResp) // Store&Forward mode
  {

    if(!sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT) || sms->getIntProperty(Tag::SMSC_MERGE_CONCAT)==3)
    {
      smsc->registerStatisticalEvent(StatEvents::etSubmitOk,sms);
    }

    // sms ��������� � ����, � ������������ Next Time, ����� �������
    // ���� ���� ������ ���-�� ���������, ����� ����� ��� ������� ������� �
    // �� ���� �� ������� sms � ���������, ����� ����� ok.
    char buf[64];
    sprintf(buf,"%lld",t.msgId);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           buf,
                           dialogId,
                           Status::OK,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      warn2(smsLog, "SBM: failed to put response command SUBMIT_OK Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId(),
        ri.smeSystemId.c_str()
      );
    }
  }


  if(!generateDeliver)
  {
    __trace__("leave non merged sms in enroute state");
    return ENROUTE_STATE;
  }

  __trace2__("Sms scheduled to %d, now %d",(int)sms->getNextTime(),(int)now);
  if(!isDatagram && !isTransaction && stime>now)
  {
    smsc->getScheduler()->AddScheduledSms(stime,t.msgId,sms->getDealiasedDestinationAddress(),dest_proxy_index);
    return ENROUTE_STATE;
  }

  // ���� ��� ��������� ������ ��� Datagram � Transction �������.
  // �� ���� �� ������ �� submit ������� submit response, ��������� ��� ���...
  // �� ���� ������� ����� ������� �� ������������ � ������,
  // � sms ���������� setLastError()
  struct ResponseGuard{
    SMS *sms;
    SmeProxy* prx;
    StateMachine* sm;
    SMSId msgId;
    /*ResponseGuard():sms(0),prx(0),sm(0){}
    ResponseGuard(const ResponseGuard& rg)
    {
      sms=rg.sms;
      prx=rg.prx;
      sm=rg.sm;
    }*/
    ResponseGuard(SMS* s,SmeProxy* p,StateMachine *st,SMSId id):sms(s),prx(p),sm(st),msgId(id){}
    ~ResponseGuard()
    {
      if(!sms)return;
      bool sandf=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0 ||
                 (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x3;
      /*
      if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0 ||
         (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x3)return;
      */
      /*
      if(sandf)//store and forward mode
      {
        if(sms->lastResult!=Status::OK)
        {
          bool ok=true;
          if(!sms->createdInStore)
          {
            sm->smsc->getTempStore().Delete(msgId);
            try{
              sms->createdInStore=true;
              sm->store->createSms(*sms,msgId,smsc::store::CREATE_NEW);
            }catch(...)
            {
              __warning2__("SBM: failed to create sms with id=%lld",msgId);
              sms->lastResult=Status::SYSERR;
              ok=false;
            }
            if(ok)
            {
              sm->smsc->registerStatisticalEvent(StatEvents::etSubmitOk,sms);
            }else
            {
              sm->smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
            }
            char buf[64];
            sprintf(buf,"%lld",msgId);
            SmscCommand resp = SmscCommand::makeSubmitSmResp
                                 (
                                   ok?buf:"0",
                                   sms->dialogId,
                                   sms->lastResult,
                                   sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                                 );
            try{
              prx->putCommand(resp);
            }catch(...)
            {
              __trace__("SUBMIT: failed to put response command");
            }
          }
          Descriptor d;
          try{
            sm->changeSmsStateToEnroute(*sms,msgId,d,sms->lastResult,sm->rescheduleSms(*sms));
          }catch(...)
          {
            __warning__("SUBMIT: failed to change state to enroute");
          }
        }
      }else*/
      if(!sandf)
      {
        if(sms->lastResult!=Status::OK)
        {
          sm->smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
        }else
        {
          if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x1)//datagram mode
          {
            sm->smsc->registerStatisticalEvent(StatEvents::etSubmitOk,sms);
          }
        }
        if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x1 ||
           ((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2 && sms->lastResult!=Status::OK))
        {
          char buf[64];
          sprintf(buf,"%lld",sms->lastResult==Status::OK?msgId:0);
          SmscCommand resp = SmscCommand::makeSubmitSmResp
                               (
                                 buf,
                                 sms->dialogId,
                                 sms->lastResult,
                                 sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                               );
          try{
            prx->putCommand(resp);
          }catch(...)
          {
            __trace__("SUBMIT: failed to put response command");
          }
        }
      }
    }
  };
  ResponseGuard respguard(sms,src_proxy,this,t.msgId);

  if ( !dest_proxy )
  {
    sms->setLastResult(Status::SMENOTCONNECTED);
    if(!isTransaction && !isDatagram)
    {
      smsc->registerStatisticalEvent(StatEvents::etDeliverErr,sms);
    }
    info2(smsLog, "SBM: dest sme not connected Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      ri.smeSystemId.c_str()
    );
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("SUBMIT_SM: change state to enroute");

      sendNotifyReport(*sms,t.msgId,"destination unavailable");

      changeSmsStateToEnroute(*sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(*sms));

    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }
  // create task
  uint32_t dialogId2;
  uint32_t uniqueId=dest_proxy->getUniqueId();
  try{
     dialogId2=dest_proxy->getNextSequenceNumber();
  }catch(...)
  {
    sms->setLastResult(Status::SMENOTCONNECTED);
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("SUBMIT_SM: change state to enroute");
      changeSmsStateToEnroute(*sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(*sms));

    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to change state to enroute");
    }
    warn2(smsLog, "SBM: failed to get seq number Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      ri.smeSystemId.c_str()
    );
    sendNotifyReport(*sms,t.msgId,"destination unavailable");
    return ENROUTE_STATE;
  }
  //Task task((uint32_t)dest_proxy_index,dialogId2);

  TaskGuard tg;
  tg.smsc=smsc;
  tg.dialogId=dialogId2;
  tg.uniqueId=uniqueId;

  try{
  Task task(uniqueId,dialogId2,isDatagram || isTransaction?new SMS(*sms):0);
  __trace2__("SUBMIT: task.sms=%p",task.sms);
  task.messageId=t.msgId;
  task.diverted=diverted;
  if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
  {
    sms->setLastResult(Status::SYSERR);
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("SUBMIT_SM: change state to enroute");
      changeSmsStateToEnroute(*sms,t.msgId,d,Status::SYSERR,rescheduleSms(*sms));

    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to change state to enroute");
    }

    warn2(smsLog, "SBMDLV: failed to create task, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      dialogId2,
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      ri.smeSystemId.c_str()
    );

    sendNotifyReport(*sms,t.msgId,"system failure");

    return ENROUTE_STATE;
  }
  }catch(...)
  {
    sms->setLastResult(Status::SYSERR);
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("SUBMIT_SM: change state to enroute");
      changeSmsStateToEnroute(*sms,t.msgId,d,Status::SYSERR,rescheduleSms(*sms));

    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to change state to enroute");
    }
    warn2(smsLog, "SBMDLV: failed to create task, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      dialogId2,
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      ri.smeSystemId.c_str()
    );
    sendNotifyReport(*sms,t.msgId,"system failure");
    return ENROUTE_STATE;
  }

  Address srcOriginal=sms->getOriginatingAddress();
  Address dstOriginal=sms->getDestinationAddress();

  if(isDatagram || isTransaction)
  {
    tg.active=true;
  }

  bool deliveryOk=false;
  int  err=0;
  std::string errstr;

  try{

    if( !isForwardTo )
    {
      // send delivery
      Address src;
      __trace2__("SUBMIT: wantAlias=%s, hide=%s",dstSmeInfo.wantAlias?"true":"false",sms->getIntProperty(Tag::SMSC_HIDE)?"true":"false");
      if(
          dstSmeInfo.wantAlias &&
          sms->getIntProperty(Tag::SMSC_HIDE) &&
          ri.hide &&
          smsc->AddressToAlias(sms->getOriginatingAddress(),src)
        )
      {
        sms->setOriginatingAddress(src);
      }
      sms->setDestinationAddress(dst);

      // profile lookup performed before partitioning
      //profile=smsc->getProfiler()->lookup(dst);
      //
      if(!sms->hasBinProperty(Tag::SMSC_CONCATINFO) && sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3)
      {
        using namespace smsc::profiler::ProfileCharsetOptions;
        if(
           (
             (sms->getIntProperty(Tag::SMSC_DSTCODEPAGE)==Default ||
              sms->getIntProperty(Tag::SMSC_DSTCODEPAGE)==Latin1
             )
             && sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::UCS2
           ) ||
           (
             (sms->getIntProperty(Tag::SMSC_DSTCODEPAGE)&Latin1)!=Latin1 &&
             sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::LATIN1
           )
          )
        {
          try{
            transLiterateSms(sms,sms->getIntProperty(Tag::SMSC_DSTCODEPAGE));
            if(sms->hasIntProperty(Tag::SMSC_ORIGINAL_DC))
            {
              int dc=sms->getIntProperty(Tag::SMSC_ORIGINAL_DC);
              int olddc=dc;
              if((dc&0xc0)==0 || (dc&0xf0)==0xf0) //groups 00xx and 1111
              {
                dc&=0xf3; //11110011 - clear 2-3 bits (set alphabet to default).

              }else if((dc&0xf0)==0xe0)
              {
                dc=0xd0 | (dc&0x0f);
              }
              sms->setIntProperty(Tag::SMSC_ORIGINAL_DC,dc);
              __trace2__("SUBMIT: transliterate olddc(%x)->dc(%x)",olddc,dc);
            }
          }catch(exception& e)
          {
            __warning2__("SUBMIT:Failed to transliterate: %s",e.what());
          }
        }
      }
    }

    if(sms->hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      if(!extractSmsPart(sms,0))
      {
        smsc_log_error(smsLog,"msgId=%lld:failed to extract sms part, aborting.",t.msgId);
        err=Status::SYSERR;
        throw ExtractPartFailedException();
      };
      sms->setIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND,1);
    }

    if(ri.replyPath==smsc::router::ReplyPathForce)
    {
      sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x80);
    }else if(ri.replyPath==smsc::router::ReplyPathSuppress)
    {
      sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x80));
    }

    SmscCommand delivery = SmscCommand::makeDeliverySm(*sms,dialogId2);
    unsigned bodyLen=0;
    delivery->get_sms()->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&bodyLen);
    __trace2__("SUBMIT: delivery.sms.sm_length=%d",bodyLen);
    prio=ri.priority/1000;
    if(prio<0)prio=0;
    if(prio>=32)prio=31;
    delivery->set_priority(prio);
    try{
      dest_proxy->putCommand(delivery);
      deliveryOk=true;
    }catch(InvalidProxyCommandException& e)
    {
      err=Status::INVBNDSTS;
      //sendNotifyReport(*sms,t.msgId,"service rejected");
      errstr="invalid bind state";
    }
  }catch(ExtractPartFailedException& e)
  {
    errstr="Failed to extract sms part";
    err=Status::INVPARLEN;
  }catch(exception& e)
  {
    errstr=e.what();
    //sendNotifyReport(*sms,t.msgId,"system failure");
    err=Status::THROTTLED;
  }catch(...)
  {
    err=Status::THROTTLED;
    errstr="unknown";
    //sendNotifyReport(*sms,t.msgId,"system failure");
  }
  if(!deliveryOk)
  {
    warn2(smsLog, "SBMDLV: failed to put delivery command, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s - %s",
      dialogId2,
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      ri.smeSystemId.c_str(),
      errstr.c_str()
    );
    sms->setOriginatingAddress(srcOriginal);
    sms->setDestinationAddress(dstOriginal);
    sms->setLastResult(err);
    if(Status::isErrorPermanent(err))
      sendFailureReport(*sms,t.msgId,err,"system failure");
    else
      sendNotifyReport(*sms,t.msgId,"system failure");

    if(!isDatagram && !isTransaction)
    {
      try{
        Descriptor d;
        if(Status::isErrorPermanent(err))
        {
          store->changeSmsStateToUndeliverable(t.msgId,d,err);
        }
        else
          changeSmsStateToEnroute(*sms,t.msgId,d,err,rescheduleSms(*sms));

      }catch(...)
      {
        __warning__("SUBMIT: failed to change state to enroute");
      }
    }
    return Status::isErrorPermanent(err)?UNDELIVERABLE_STATE:ENROUTE_STATE;
  }

  if(isDatagram || isTransaction)
  {
    tg.active=false;
  }
  sms->lastResult=Status::OK;
  info2(smsLog, "SBM: submit ok, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
    dialogId2,
    t.msgId,dialogId,
    sms->getOriginatingAddress().toString().c_str(),
    sms->getDestinationAddress().toString().c_str(),
    src_proxy->getSystemId(),
    ri.smeSystemId.c_str()
  );
  return DELIVERING_STATE;
}

StateType StateMachine::forward(Tuple& t)
{
  __trace2__("FWD: id=%lld",t.msgId);
  SMS sms;
  if(smsc->getTempStore().Get(t.msgId,sms))
  {
    __trace2__("FWD: sms %lld found in temp store... need to create it",t.msgId);
    smsc->getTempStore().Delete(t.msgId);
    try{
      store->createSms(sms,t.msgId,smsc::store::CREATE_NEW);
    }catch(...)
    {
      smsc_log_warn(smsLog, "FWD: failed to createSms %lld",t.msgId);
      return UNKNOWN_STATE;
    }
  }else
  {
    try{
      store->retriveSms((SMSId)t.msgId,sms);
    }catch(...)
    {
      smsc_log_warn(smsLog, "FWD: failed to retriveSms %lld",t.msgId);
      smsc->getScheduler()->InvalidSms(t.msgId);
      return UNKNOWN_STATE;
    }
  }


  if(!sms.Invalidate(__FILE__,__LINE__))
  {
    smsc_log_warn(smsLog, "Invalidate of %lld failed",t.msgId);
    smsc->getScheduler()->InvalidSms(t.msgId);
    return ERROR_STATE;
  }

  if(sms.getState()==EXPIRED_STATE)
  {
    smsc_log_warn(smsLog, "FWD: sms in expired state msgId=%lld",t.msgId);
    smsc->getScheduler()->InvalidSms(t.msgId);
    return EXPIRED_STATE;
  }
  time_t now=time(NULL);
  if((sms.getValidTime()<=now && sms.getLastResult()!=0) || //expired or
     RescheduleCalculator::calcNextTryTime(now,sms.getLastResult(),sms.getAttemptsCount())==-1) //max attempts count reached
  {
    sms.setLastResult(Status::EXPIRED);
    smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
    smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
    try{
      store->changeSmsStateToExpired(t.msgId);
      smsc->getScheduler()->InvalidSms(t.msgId);
    }catch(...)
    {
      __warning__("FORWARD: failed to change state to expired");
    }
    info2(smsLog, "FWD: %lld expired (valid:%u - now:%u)",t.msgId,sms.getValidTime(),now);

    sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");

    return EXPIRED_STATE;
  }

  if(sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT) && sms.getIntProperty(Tag::SMSC_MERGE_CONCAT)!=3)
  {
    smsc_log_warn(smsLog, "Attempt to forward incomplete concatenated message %lld",t.msgId);
    try{
      Descriptor d;
      smsc->getScheduler()->InvalidSms(t.msgId);
      store->changeSmsStateToUndeliverable
      (
        t.msgId,
        d,
        Status::SYSERR
      );
    }catch(...)
    {
      __warning__("failed to change sms state to undeliverable");
    }
    return UNDELIVERABLE_STATE;
  }

  if(sms.getState()!=ENROUTE_STATE)
  {
    smsc_log_warn(smsLog, "FWD: sms msgId=%lld is not in enroute (%d)",t.msgId,sms.getState());
    smsc->getScheduler()->InvalidSms(t.msgId);
    return sms.getState();
  }
  if( sms.getNextTime()>now && sms.getAttemptsCount()==0 && (!t.command->is_reschedulingForward() || sms.getLastResult()==0) )
  {
    debug2(smsLog, "FWD: nextTime>now (%d>%d)",sms.getNextTime(),now);
    SmeIndex idx=smsc->getSmeIndex(sms.dstSmeId);
    smsc->getScheduler()->AddScheduledSms(sms.getNextTime(),t.msgId,sms.getDealiasedDestinationAddress(),idx);
    return sms.getState();
  }


  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    Descriptor d;
    try{
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      smsc_log_warn(smsLog,"FORWARD: Failed to change state of USSD request to undeliverable. msgId=%lld",t.msgId);
    }
    return UNDELIVERABLE_STATE;
  }

  ////
  //
  // Traffic Control
  //


  if(!smsc->allowCommandProcessing(t.command))
  {
    sms.setLastResult(Status::THROTTLED);
    Descriptor d;
    //__trace__("FORWARD: traffic control denied forward");
    try{
      changeSmsStateToEnroute(sms,t.msgId,d,Status::THROTTLED,rescheduleSms(sms));
    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
    return ENROUTE_STATE;
  };

  //
  // End of traffic Control
  //
  ////


  if(!t.command->is_reschedulingForward() &&
     sms.getIntProperty(Tag::SMPP_SET_DPF)==1 &&
     sms.getAttemptsCount()==1)
  {
    try{
      SmeProxy  *src_proxy=smsc->getSmeProxy(sms.getSourceSmeId());
      if(src_proxy)
      {
        int dialogId=src_proxy->getNextSequenceNumber();
        SmscCommand cmd=SmscCommand::makeAlertNotificationCommand
        (
          dialogId,
          sms.getDestinationAddress(),
          sms.getOriginatingAddress(),
          0
        );
        src_proxy->putCommand(cmd);
      }
    }catch(exception& e)
    {
      __trace2__("FORWARD: Failed to send AlertNotification:%s",e.what());
    }catch(...)
    {
      __trace__("FORWARD: Failed to send AlertNotification:unknown");
    }
  }

  if(sms.getAttemptsCount()==0 && sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
  {
    smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
  }else
  {
    smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
  }

  SmeProxy *dest_proxy=0;
  int dest_proxy_index;

  Address dst=sms.getDealiasedDestinationAddress();

  bool diverted=false;
  bool doRepartition=false;
  if(t.command->get_forwardAllowDivert() &&
     (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&DF_COND) &&
     !(sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&DF_UNCOND)
    )
  {
    try{
      dst=sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str();
      Address newdst;
      if(smsc->AliasToAddress(dst,newdst))dst=newdst;
      debug2(smsLog,"FWD: cond divert from %s to %s",
        sms.getDealiasedDestinationAddress().toString().c_str(),dst.toString().c_str());
      diverted=true;
      int df=sms.getIntProperty(Tag::SMSC_DIVERTFLAGS);
      sms.setIntProperty(Tag::SMSC_UDH_CONCAT,df&DF_UDHCONCAT);
      int olddc=sms.getIntProperty(Tag::SMSC_DSTCODEPAGE);
      int newdc=(df>>DF_DCSHIFT)&0xFF;
      sms.setIntProperty(Tag::SMSC_DSTCODEPAGE,newdc);
      if(olddc!=newdc && sms.hasBinProperty(Tag::SMSC_CONCATINFO) && !sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT))
      {
        doRepartition=true;
        sms.getMessageBody().dropProperty(Tag::SMSC_CONCATINFO);
      }
    }catch(...)
    {
      smsc_log_warn(smsLog,"FWD: failed to construct address for cond divert %s",sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
    }
  }

  if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&DF_UNCOND))
  {
    dst=sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str();
    Address newdst;
    if(smsc->AliasToAddress(dst,newdst))dst=newdst;
  }

  smsc::router::RouteInfo ri;
  bool has_route = smsc->routeSms
                    (
                      sms.getOriginatingAddress(),
                      dst,
                      dest_proxy_index,
                      dest_proxy,
                      &ri,
                      smsc->getSmeIndex(sms.getSourceSmeId())
                    );
  if ( !has_route )
  {
    char from[32],to[32];
    sms.getOriginatingAddress().toString(from,sizeof(from));
    sms.getDestinationAddress().toString(to,sizeof(to));
    smsc_log_warn(smsLog, "FWD: msgId=%lld, No route (%s->%s)",t.msgId,from,to);
    try{
      sms.setLastResult(Status::NOROUTE);
      sendNotifyReport(sms,t.msgId,"destination unavailable");
    }catch(...)
    {
      __trace__("FORWARD: failed to send intermediate notification");
    }
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      changeSmsStateToEnroute(sms,t.msgId,d,Status::NOROUTE,rescheduleSms(sms));

    }catch(...)
    {
      __trace__("FORWARD: failed to change state to enroute");
    }
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
    return ERROR_STATE;
  }



  if(!dest_proxy)
  {
    char bufsrc[64],bufdst[64];
    sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
    sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
    smsc_log_warn(smsLog, "FWD: msgId=%lld sme is not connected(%s->%s(%s))",t.msgId,bufsrc,bufdst,ri.smeSystemId.c_str());
    sms.setLastResult(Status::SMENOTCONNECTED);
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
    try{
      sendNotifyReport(sms,t.msgId,"destination unavailable");
    }catch(...)
    {
      __trace__("FORWARD: failed to send intermediate notification");
    }
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }
  // create task


  if(doRepartition && ri.smeSystemId=="MAP_PROXY")
  {
    debug2(smsLog,"FWD: sms repartition %lld",t.msgId);
    int pres=partitionSms(&sms);
    if(pres!=psSingle && pres!=psMultiple)
    {
      debug2(smsLog,"FWD: divert failed - cannot concat, msgId=%lld",t.msgId);
      return UNKNOWN_STATE;
    }
    debug2(smsLog,"%lld after repartition: %s",t.msgId,pres==psSingle?"single":"multiple");
  }

  TaskGuard tg;
  tg.smsc=smsc;


  uint32_t dialogId2;
  uint32_t uniqueId=dest_proxy->getUniqueId();
  try{
    dialogId2 = dest_proxy->getNextSequenceNumber();
    tg.dialogId=dialogId2;
    tg.uniqueId=uniqueId;
    debug2(smsLog, "FWDDLV: msgId=%lld, seq number:%d",t.msgId,dialogId2);
    //Task task((uint32_t)dest_proxy_index,dialogId2);
    Task task(uniqueId,dialogId2);
    task.diverted=diverted;
    task.messageId=t.msgId;
    if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
    {
      smsc_log_warn(smsLog, "FWDDLV: failed to create task msgId=%lld, seq number:%d",t.msgId,dialogId2);
      try{
        sms.setLastResult(Status::SYSERR);
        sendNotifyReport(sms,t.msgId,"destination unavailable");
      }catch(...)
      {
        __warning__("FORWARD: failed to send intermediate notification");
      }
      smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
      try{
        Descriptor d;
        __trace__("FORWARD: change state to enroute");
        sms.setLastResult(Status::SYSERR);
        changeSmsStateToEnroute(sms,t.msgId,d,Status::SYSERR,rescheduleSms(sms));

      }catch(...)
      {
        __warning__("FORWARD: failed to change state to enroute");
      }
      return ENROUTE_STATE;
    }
    tg.active=true;
  }catch(...)
  {
    smsc_log_warn(smsLog, "FWDDLV: failed to get seqnum msgId=%lld",t.msgId);
    try{
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      sms.setLastResult(Status::SMENOTCONNECTED);
      smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
      changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }
  Address srcOriginal=sms.getOriginatingAddress();
  Address dstOriginal=sms.getDestinationAddress();
  int errstatus=0;
  const char* errtext;
  try{
    // send delivery
    Address src;
    if(
        smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias &&
        sms.getIntProperty(Tag::SMSC_HIDE) &&
        ri.hide &&
        smsc->AddressToAlias(sms.getOriginatingAddress(),src)
      )
    {
      sms.setOriginatingAddress(src);
    }
    //Address dst=sms.getDealiasedDestinationAddress();
    sms.setDestinationAddress(dst);
    if(!sms.hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      if(sms.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3)
      {
        using namespace smsc::profiler::ProfileCharsetOptions;
        if(
           (
             (sms.getIntProperty(Tag::SMSC_DSTCODEPAGE)==Default ||
              sms.getIntProperty(Tag::SMSC_DSTCODEPAGE)==Latin1
             )
             && sms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::UCS2
           ) ||
           (
             (sms.getIntProperty(Tag::SMSC_DSTCODEPAGE)&Latin1)!=Latin1 &&
             sms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::LATIN1
           )
          )
        {
          try{
            transLiterateSms(&sms,sms.getIntProperty(Tag::SMSC_DSTCODEPAGE));
            if(sms.hasIntProperty(Tag::SMSC_ORIGINAL_DC))
            {
              int dc=sms.getIntProperty(Tag::SMSC_ORIGINAL_DC);
              int olddc=dc;
              if((dc&0xc0)==0 || (dc&0xf0)==0xf0) //groups 00xx and 1111
              {
                dc&=0xf3; //11110011 - clear 2-3 bits (set alphabet to default).

              }else if((dc&0xf0)==0xe0)
              {
                dc=0xd0 | (dc&0x0f);
              }
              sms.setIntProperty(Tag::SMSC_ORIGINAL_DC,dc);
              __trace2__("FORWARD: transliterate olddc(%x)->dc(%x)",olddc,dc);
            }
          }catch(exception& e)
          {
            __warning2__("SUBMIT:Failed to transliterate: %s",e.what());
          }
        }
      }
    }else
    {
      unsigned int len;
      ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
      if(sms.getConcatSeqNum()<ci->num)
      {
        if(!extractSmsPart(&sms,sms.getConcatSeqNum()))
        {
          throw ExtractPartFailedException();
        }
      }else
      {
        __warning__("attempt to forward concatenated message but all parts are delivered!!!");
        try{
          Descriptor d;
          smsc->getScheduler()->InvalidSms(t.msgId);
          store->changeSmsStateToUndeliverable
          (
            t.msgId,
            d,
            Status::SYSERR
          );
        }catch(...)
        {
          __warning2__("failed to change state of sms %lld to final ... again!!!",t.msgId);
        }
        return ERROR_STATE;
      }

    }
    if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      uint32_t len;
      ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
      if(sms.getConcatSeqNum()<ci->num-1)
      {
        sms.setIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND,1);
      }
    }
    if(ri.replyPath==smsc::router::ReplyPathForce)
    {
      sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x80);
    }else if(ri.replyPath==smsc::router::ReplyPathSuppress)
    {
      sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x80));
    }
    SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
    dest_proxy->putCommand(delivery);
    tg.active=false;
  }
  catch(ExtractPartFailedException& e)
  {
    errstatus=Status::INVPARLEN;
    smsc_log_error(smsLog,"FWDDLV: failed to extract sms part for %lld",t.msgId);
    errtext="failed to extract sms part";
  }
  catch(InvalidProxyCommandException& e)
  {
    errstatus=Status::INVBNDSTS;
    errtext="service rejected";
  }
  catch(exception& e)
  {
    smsc_log_warn(smsLog, "FWDDLV: failed create deliver, exception:%s",e.what());
    errstatus=Status::THROTTLED;
    errtext="SME busy";
  }
  catch(...)
  {
    errstatus=Status::THROTTLED;
    errtext="SME busy";
  }
  if(errstatus)
  {
    smsc_log_warn(smsLog, "FWDDLV: failed create deliver(%s) srcSme=%s msgId=%lld, seq number:%d",errtext,sms.getSourceSmeId(),t.msgId,dialogId2);

    sms.setOriginatingAddress(srcOriginal);
    sms.setDestinationAddress(dstOriginal);
    sms.setLastResult(errstatus);
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
//    sendNotifyReport(sms,t.msgId,errtext);
    try{
      if(Status::isErrorPermanent(errstatus))
        sendFailureReport(sms,t.msgId,errstatus,"system failure");
      else
        sendNotifyReport(sms,t.msgId,"system failure");
    }catch(std::exception& e)
    {
      __warning2__("failed to submit receipt:%s",e.what());
    }
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
        if(Status::isErrorPermanent(errstatus))
        {
          smsc->getScheduler()->InvalidSms(t.msgId);
          store->changeSmsStateToUndeliverable(t.msgId,d,errstatus);
        }
        else
          changeSmsStateToEnroute(sms,t.msgId,d,errstatus,rescheduleSms(sms));

    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    Task t;
    smsc->tasks.findAndRemoveTask(uniqueId,dialogId2,&t);
    return Status::isErrorPermanent(errstatus)?UNDELIVERABLE_STATE:ENROUTE_STATE;
  }
  info2(smsLog, "FWDDLV: deliver ok msgId=%lld, seq number:%d",t.msgId,dialogId2);

  return DELIVERING_STATE;
}


StateType StateMachine::DivertProcessing(Tuple& t,SMS& sms)
{
  if(t.command->get_resp()->get_diverted())return UNKNOWN_STATE;
  if(sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&DF_UNCOND)return UNKNOWN_STATE;
  if(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).length()==0)return UNKNOWN_STATE;
  int status=GET_STATUS_CODE(t.command->get_resp()->get_status());

  int divertFlags=sms.getIntProperty(Tag::SMSC_DIVERTFLAGS);

  bool doDivert=
     (
       (divertFlags&DF_ABSENT) &&
       (
         status==Status::UNDEFSUBSCRIBER ||
         status==Status::ABSENTSUBSCRIBERSM ||
         status==Status::ABSENTSUBSCR
       )
     )
     ||
     (
       (divertFlags&DF_BLOCK) &&
       (
         status==Status::TELSVCNOTPROVIS ||
         status==Status::FACILITYNOTSUPP
       )
     )
     ||
     (
       (divertFlags&DF_BARRED) &&
       (
         status==Status::CALLBARRED
       )
     )
     ||
     (
       (divertFlags&DF_CAPAC) &&
       (
         status==Status::SMDELIFERYFAILURE
       )
     );
  if(!doDivert)return UNKNOWN_STATE;
  SmscCommand cmd=SmscCommand::makeForward(t.msgId,false);
  cmd->set_forwardAllowDivert(true);
  Tuple t2;
  t2.command=cmd;
  t2.msgId=t.msgId;
  t2.state=t.state;
  return forward(t2);
}


StateType StateMachine::deliveryResp(Tuple& t)
{
  //__require__(t.state==DELIVERING_STATE);
  if(t.state!=DELIVERING_STATE)
  {
    debug2(smsLog, "DLVRSP: state of SMS isn't DELIVERING!!! msgId=%lld;st=%d",t.msgId,t.command->get_resp()->get_status());
    smsc->getScheduler()->InvalidSms(t.msgId);
    return t.state;
  }
  //smsc::sms::Descriptor d;

  ////
  //
  // register deliveryresp in traffic control
  //

  /*
  bool ussdSession=false;
  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    if(sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)!=USSD_PSSR_IND &&
       !(
         sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)==USSD_USSR_REQ &&
         sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)
        )
      )
    {
      ussdSession=true;
    }
  }
  */

  smsc->allowCommandProcessing(t.command);

  //
  // end of traffic control code
  //
  ////

  SMS sms;
  bool dgortr=t.command->get_resp()->get_sms()!=0; //datagram or transaction
  bool finalized=false;
  if(dgortr)
  {
    sms=*t.command->get_resp()->get_sms();
    sms.destinationDescriptor=t.command->get_resp()->getDescriptor();
  }else if(smsc->getTempStore().Get(t.msgId,sms))
  {
    sms.destinationDescriptor=t.command->get_resp()->getDescriptor();
    smsc->getTempStore().Delete(t.msgId);
    int status=Status::OK;
    if(GET_STATUS_TYPE(t.command->get_resp()->get_status())!=CMD_OK ||
       sms.hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      try{
        store->createSms(sms,t.msgId,smsc::store::CREATE_NEW);
      }catch(...)
      {
        smsc_log_warn(smsLog, "DLVRSP: failed to createSms %lld",t.msgId);
        status=Status::SYSERR;
      }
    }else
    {
      sms.state=DELIVERED;
      try{
        store->createFinalizedSms(t.msgId,sms);
        finalized=true;
      }catch(...)
      {
        smsc_log_warn(smsLog, "DLVRSP: failed to create finalized Sms %lld",t.msgId);
        status=Status::SYSERR;
      }
    }
    if(status==Status::OK)
    {
      smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
    }else
    {
      smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
    }
    SmeProxy *src_proxy=smsc->getSmeProxy(sms.srcSmeId);
    if(src_proxy)
    {
      sms.setLastResult(status);
      char msgId[64];
      sprintf(msgId,"%lld",t.msgId);
      SmscCommand resp = SmscCommand::makeSubmitSmResp
                           (
                             status==Status::OK?msgId:"0",
                             sms.dialogId,
                             status,
                             sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                           );
      try{
        src_proxy->putCommand(resp);
      }catch(...)
      {
        __warning__("SUBMIT_SM: failed to put response command");
      }
    }
    if(status!=Status::OK)return UNKNOWN_STATE;
  }else
  {
    try{
      store->retriveSms((SMSId)t.msgId,sms);
    }catch(exception& e)
    {
      smsc_log_warn(smsLog, "DLVRSP: failed to retrieve sms:%s! msgId=%lld;st=%d",e.what(),t.msgId,t.command->get_resp()->get_status());
      smsc->getScheduler()->InvalidSms(t.msgId);
      return UNKNOWN_STATE;
    }
    sms.destinationDescriptor=t.command->get_resp()->getDescriptor();
  }

  {
    char buf[MAX_ADDRESS_VALUE_LENGTH*4+12];
    if(sms.originatingDescriptor.mscLength && sms.originatingDescriptor.imsiLength &&
       sms.destinationDescriptor.mscLength && sms.destinationDescriptor.imsiLength)
    {
      sprintf(buf,"Org: %s/%s, Dst:%s/%s",sms.originatingDescriptor.msc,sms.originatingDescriptor.imsi,
                         sms.destinationDescriptor.msc,sms.destinationDescriptor.imsi
                        );
    }else if(sms.originatingDescriptor.mscLength && sms.originatingDescriptor.imsiLength)
    {
      sprintf(buf,"Org: %s/%s",sms.originatingDescriptor.msc,sms.originatingDescriptor.imsi);
    }else if(sms.destinationDescriptor.mscLength && sms.destinationDescriptor.imsiLength)
    {
      sprintf(buf,"Dst:%s/%s",sms.destinationDescriptor.msc,sms.destinationDescriptor.imsi);
    }


    sms.setStrProperty(Tag::SMSC_DESCRIPTORS,buf);
  }

  int sttype=GET_STATUS_TYPE(t.command->get_resp()->get_status());
  info2(smsLog, "DLVRSP: msgId=%lld;class=%s;st=%d;od=%s;da=%s",t.msgId,
      sttype==CMD_OK?"OK":
      sttype==CMD_ERR_RESCHEDULENOW?"RESCHEDULEDNOW":
      sttype==CMD_ERR_TEMP?"TEMP ERROR":"PERM ERROR",
      GET_STATUS_CODE(t.command->get_resp()->get_status()),
      sms.getOriginatingAddress().toString().c_str(),
      sms.getDestinationAddress().toString().c_str()
    );


  if(GET_STATUS_TYPE(t.command->get_resp()->get_status())!=CMD_OK)
  {
    switch(GET_STATUS_TYPE(t.command->get_resp()->get_status()))
    {
      case CMD_ERR_RESCHEDULENOW:
      {
        sms.setLastResult(GET_STATUS_CODE(t.command->get_resp()->get_status()));
        try{
          time_t rt=time(NULL)+2;
          if(t.command->get_resp()->get_delay()!=-1)
          {
            rt+=t.command->get_resp()->get_delay()-2;
          }
          __trace2__("DELIVERYRESP: change state to enroute (reschedule now=%d)",rt);
          changeSmsStateToEnroute
          (
            sms,
            t.msgId,
            sms.getDestinationDescriptor(),
            Status::RESCHEDULEDNOW,
            rt,
            true
          );
        }catch(std::exception& e)
        {
          __warning2__("DELIVERYRESP: failed to change state to enroute:%s",e.what());
        }

        smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }
        return UNKNOWN_STATE;
      }break;
      case CMD_ERR_TEMP:
      {
        if(!dgortr)
        {
          StateType st=DivertProcessing(t,sms);
          if(st!=UNKNOWN_STATE)return st;
        }
        sms.setLastResult(GET_STATUS_CODE(t.command->get_resp()->get_status()));
        try{
          __trace__("DELIVERYRESP: change state to enroute");
          time_t rt;
          if(t.command->get_resp()->get_delay()!=-1)
          {
            rt=t.command->get_resp()->get_delay();
          }else
          {
            rt=rescheduleSms(sms);
          }
          changeSmsStateToEnroute
          (
            sms,
            t.msgId,
            sms.getDestinationDescriptor(),
            GET_STATUS_CODE(t.command->get_resp()->get_status()),
            rt
          );
        }catch(std::exception& e)
        {
          __warning2__("DELIVERYRESP: failed to change state to enroute:%s",e.what());
        }

        sendNotifyReport(sms,t.msgId,"subscriber busy");
        smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }
        return UNKNOWN_STATE;
      }break;
      default:
      {
        if(!dgortr)
        {
          try{
            __trace__("DELIVERYRESP: change state to undeliverable");
            store->changeSmsStateToUndeliverable
            (
              t.msgId,
              sms.getDestinationDescriptor(),
              GET_STATUS_CODE(t.command->get_resp()->get_status())
            );
          }catch(std::exception& e)
          {
            __warning2__("DELIVERYRESP: failed to change state to undeliverable:%s",e.what());
          }
        }

        sms.setLastResult(GET_STATUS_CODE(t.command->get_resp()->get_status()));
        sendFailureReport(sms,t.msgId,UNDELIVERABLE_STATE,"permanent error");
        smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);

        smsc->getScheduler()->InvalidSms(t.msgId);

        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
        }
        return UNDELIVERABLE_STATE;
      }
    }
  }

  // concatenated message with conditional divert.
  // first part delivered ok.
  // other parts MUST be delivered to the same address.
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO) && sms.getConcatSeqNum()==0 &&
     (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&(DF_COND|DF_UNCOND)))
  {
    debug2(smsLog,"DLVRESP: msgId=%lld - delivered first part of multipart sms with conditional divert.",t.msgId);
    // first part was delivered to diverted address!
    if(t.command->get_resp()->get_diverted())
    {
      // switch to unconditional divert.
      debug1(smsLog,"deliver to divert address");
      int df=sms.getIntProperty(Tag::SMSC_DIVERTFLAGS);
      sms.setIntProperty(Tag::SMSC_UDH_CONCAT,df&DF_UDHCONCAT);
      int dc=(df>>DF_DCSHIFT)&0xFF;
      sms.setIntProperty(Tag::SMSC_DSTCODEPAGE,dc);
      sms.setIntProperty(Tag::SMSC_DIVERTFLAGS,df|DF_UNCOND);

      if(!sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT))
      {
        sms.getMessageBody().dropProperty(Tag::SMSC_CONCATINFO);
        partitionSms(&sms);
      }

    }else // first part was delivered to original address
    {
      // turn off divert
      debug1(smsLog,"deliver to original address");
      sms.getMessageBody().dropIntProperty(Tag::SMSC_DIVERTFLAGS);
    }
    try{
      //patch sms in store
      store->replaceSms(t.msgId,sms);
    }catch(...)
    {
       //shit happens
       __warning2__("failed to replace sms in store (divert fix) msgId=%lld",t.msgId);
    }
  }

  //bool skipFinalizing=false;

  vector<unsigned char> umrList; //umrs of parts of merged message
  //int umrIndex=-1;//index of current umr
  //bool umrLast=true;//need to generate receipts for the rest of umrs
  //int savedCsn=sms.getConcatSeqNum();

  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    debug2(smsLog, "DLVRSP: sms has concatinfo, csn=%d;msgId=%lld",sms.getConcatSeqNum(),t.msgId);
    unsigned int len;
    ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
    if(sms.getConcatSeqNum()<ci->num-1)
    {
      {
        sms.setConcatSeqNum(sms.getConcatSeqNum()+1);
        if(!dgortr)
        try
        {
          store->changeSmsConcatSequenceNumber(t.msgId);
        }catch(std::exception& e)
        {
          __warning2__("DELIVERYRESP: failed to change sms concat seq num:%lld - %s",t.msgId,e.what());
          try{
            sms.setLastResult(sms.getLastResult());
            changeSmsStateToEnroute
            (
              sms,
              t.msgId,
              sms.getDestinationDescriptor(),
              Status::SYSERR,
              rescheduleSms(sms)
            );
          }catch(...)
          {
             __warning2__("DELIVERYRESP: failed to cahnge sms state to enroute:%lld",t.msgId);
          }
          if(dgortr)
          {
            sms.state=UNDELIVERABLE;
            finalizeSms(t.msgId,sms);
            return UNDELIVERABLE_STATE;
          }
          return UNKNOWN_STATE;
        }
        __trace2__("CONCAT: concatseqnum=%d for msdgId=%lld",sms.getConcatSeqNum(),t.msgId);
      }

      ////
      //
      //  send concatenated
      //

      SmeProxy *dest_proxy=0;
      int dest_proxy_index;

      Address dst=sms.getDealiasedDestinationAddress();
      if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&(DF_COND|DF_UNCOND)))
      {
        dst=sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str();
      }

      smsc::router::RouteInfo ri;
      bool has_route = smsc->routeSms(sms.getOriginatingAddress(),dst,dest_proxy_index,dest_proxy,&ri,smsc->getSmeIndex(sms.getSourceSmeId()));
      if ( !has_route )
      {
        __warning__("CONCAT: No route");
        try{
          sms.setLastResult(Status::NOROUTE);
          sendNotifyReport(sms,t.msgId,"destination unavailable");
        }catch(...)
        {
          __trace__("CONCAT: failed to send intermediate notification");
        }
        try{
          //time_t now=time(NULL);
          Descriptor d;
          __trace__("CONCAT: change state to enroute");
          changeSmsStateToEnroute(sms,t.msgId,d,Status::NOROUTE,rescheduleSms(sms));

        }catch(...)
        {
          __trace__("CONCAT: failed to change state to enroute");
        }
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }
        return ERROR_STATE;
      }
      if(!dest_proxy)
      {
        __trace__("CONCAT: no proxy");
        try{
          sms.setLastResult(Status::SMENOTCONNECTED);
          sendNotifyReport(sms,t.msgId,"destination unavailable");
        }catch(...)
        {
          __trace__("CONCAT: failed to send intermediate notification");
        }
        try{
          //time_t now=time(NULL);
          Descriptor d;
          __trace__("CONCAT: change state to enroute");
          changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

        }catch(...)
        {
          __warning__("CONCAT: failed to change state to enroute");
        }
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }
        return ENROUTE_STATE;
      }
      // create task

      uint32_t dialogId2;
      uint32_t uniqueId=dest_proxy->getUniqueId();

      TaskGuard tg;
      tg.smsc=smsc;
      tg.dialogId=dialogId2;
      tg.uniqueId=uniqueId;

      try{
        dialogId2 = dest_proxy->getNextSequenceNumber();
        __trace2__("CONCAT: seq number:%d",dialogId2);
        //Task task((uint32_t)dest_proxy_index,dialogId2);

        Task task(uniqueId,dialogId2,dgortr?new SMS(sms):0);
        task.messageId=t.msgId;
        if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
        {
          __warning__("CONCAT: can't create task");
          try{
            //time_t now=time(NULL);
            Descriptor d;
            __trace__("CONCAT: change state to enroute");
            changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

          }catch(...)
          {
            __warning__("CONCAT: failed to change state to enroute");
          }
          if(dgortr)
          {
            sms.state=UNDELIVERABLE;
            finalizeSms(t.msgId,sms);
            return UNDELIVERABLE_STATE;
          }
          return ENROUTE_STATE;
        }
        __trace2__("CONCAT: created task for %u/%d",dialogId2,uniqueId);
        if(dgortr)tg.active=true;
      }catch(...)
      {
        try{
          //time_t now=time(NULL);
          Descriptor d;
          __trace__("CONCAT: change state to enroute");
          changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

        }catch(...)
        {
          __warning__("CONCAT: failed to change state to enroute");
        }
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }
        return ENROUTE_STATE;
      }
      Address srcOriginal=sms.getOriginatingAddress();
      Address dstOriginal=sms.getDestinationAddress();
      int errstatus=0;
      const char* errtext;
      try{
        // send delivery
        Address src;
        if(smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias &&
           sms.getIntProperty(Tag::SMSC_HIDE) &&
           smsc->AddressToAlias(sms.getOriginatingAddress(),src))
        {
          sms.setOriginatingAddress(src);
        }
        //Address dst=sms.getDealiasedDestinationAddress();
        sms.setDestinationAddress(dst);

        //
        //

        if(!extractSmsPart(&sms,sms.getConcatSeqNum()))
        {
          throw ExtractPartFailedException();
        }

        if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
        {
          uint32_t len;
          ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
          if(sms.getConcatSeqNum()<ci->num-1)
          {
            sms.setIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND,1);
          }else
          {
            sms.getMessageBody().dropIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND);
          }
        }
        if(ri.replyPath==smsc::router::ReplyPathForce)
        {
          sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x80);
        }else if(ri.replyPath==smsc::router::ReplyPathSuppress)
        {
          sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x80));
        }

        SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
        dest_proxy->putCommand(delivery);
        tg.active=false;
      }
      catch(ExtractPartFailedException& e)
      {
        errstatus=Status::INVPARLEN;
        smsc_log_error(smsLog,"FWDDLV: failed to extract sms part for %lld",t.msgId);
        errtext="failed to extract sms part";
      }
      catch(InvalidProxyCommandException& e)
      {
        errstatus=Status::INVBNDSTS;
        errtext="service rejected";
      }
      catch(...)
      {
        errstatus=Status::THROTTLED;
        errtext="SME busy";
      }
      if(errstatus)
      {
        __warning2__("CONCAT::Err %s",errtext);
        try{
          //time_t now=time(NULL);
          Descriptor d;
          __trace__("CONCAT: change state to enroute");
          //changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));
          sms.setLastResult(errstatus);
          if(Status::isErrorPermanent(errstatus))
          {
            smsc->getScheduler()->InvalidSms(t.msgId);
            sendFailureReport(sms,t.msgId,UNDELIVERABLE_STATE,"permanent error");
            store->changeSmsStateToUndeliverable(t.msgId,d,errstatus);
          }
          else
            changeSmsStateToEnroute(sms,t.msgId,d,errstatus,rescheduleSms(sms));
        }catch(...)
        {
          __warning__("CONCAT: failed to change state to enroute");
        }
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }
        return Status::isErrorPermanent(errstatus)?UNDELIVERABLE_STATE:ENROUTE_STATE;
      }

      //
      //
      //
      ////


      /*
      if(!sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
      {
        return DELIVERING_STATE;
      }else
      {
        skipFinalizing=true;
      }
      umrLast=false;
      */
      return DELIVERING_STATE;
    }
  }

  //if(sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
  //{
  if(sms.hasBinProperty(Tag::SMSC_UMR_LIST))
  {
    unsigned len;
    unsigned char* lst=(unsigned char*)sms.getBinProperty(Tag::SMSC_UMR_LIST,&len);
    if(!sms.hasBinProperty(Tag::SMSC_UMR_LIST_MASK))
    {
      umrList.insert(umrList.end(),lst,lst+len);
      //umrIndex=sms.hasBinProperty(Tag::SMSC_CONCATINFO)?savedCsn:0;
    }else
    {
      unsigned mlen;
      const char* mask=sms.getBinProperty(Tag::SMSC_UMR_LIST_MASK,&mlen);
      if(mlen<len)mlen=len;
      for(int i=0;i<mlen;i++)
      {
        if(mask[i])
        {
          umrList.push_back(lst[i]);
        }
      }
    }
  }
  //}


  //if(!skipFinalizing)
  //{

    if(dgortr)
    {
      sms.state=DELIVERED;
      smsc->registerStatisticalEvent(StatEvents::etDeliveredOk,&sms);
      if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2)
      {
        smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
        SmeProxy *src_proxy=smsc->getSmeProxy(sms.srcSmeId);
        if(src_proxy)
        {
          char msgId[64];
          sprintf(msgId,"%lld",t.msgId);
          SmscCommand resp=SmscCommand::makeSubmitSmResp
                           (
                             msgId,
                             sms.dialogId,
                             sms.lastResult,
                             sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                           );
          try{
            src_proxy->putCommand(resp);
          }catch(...)
          {
            sms.state=UNDELIVERABLE;
            try{
              store->createFinalizedSms(t.msgId,sms);
            }catch(...)
            {
              __warning2__("DELRESP: failed to finalize sms with msgId=%lld",t.msgId);
              return UNDELIVERABLE_STATE;
            }
            return UNDELIVERABLE_STATE;
          }
        }
      }
      try{
        store->createFinalizedSms(t.msgId,sms);
      }catch(...)
      {
        __warning2__("DELRESP: failed to finalize sms with msgId=%lld",t.msgId);
        return UNDELIVERABLE_STATE;
      }
      return DELIVERED_STATE;
    }else if(!finalized)
    {


      try{
        __trace__("change state to delivered");

        store->changeSmsStateToDelivered(t.msgId,t.command->get_resp()->getDescriptor());

        smsc->getScheduler()->DeliveryOk(t.msgId);

        __trace__("change state to delivered: ok");
      }catch(std::exception& e)
      {
        __warning2__("change state to delivered exception:%s",e.what());
        //return UNKNOWN_STATE;
      }
    }
    debug2(smsLog, "DLVRSP: DELIVERED, msgId=%lld",t.msgId);
    __trace__("DELIVERYRESP: registerStatisticalEvent");

    smsc->registerStatisticalEvent(StatEvents::etDeliveredOk,&sms);
  //}

  try{
    //smsc::profiler::Profile p=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
    __trace2__("DELIVERYRESP: suppdelrep=%d, delrep=%d, regdel=%d, srr=%d",
      sms.getIntProperty(Tag::SMSC_SUPPRESS_REPORTS),
      (int)sms.getDeliveryReport(),
      sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY),
      sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST));

    bool regdel=(sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==1 ||
                sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST);

    if(
       (
         sms.getIntProperty(Tag::SMSC_SUPPRESS_REPORTS)
       ) ||
       (
         sms.getDeliveryReport()==REPORT_NOACK
       ) ||
       (
         sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)
       )
      )
    {
      return DELIVERED_STATE;
    }
    if(
        sms.getDeliveryReport() ||
        regdel
      )
    {
      SMS rpt;
      rpt.setOriginatingAddress(scAddress);
      char msc[]="";
      char imsi[]="";
      rpt.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
      rpt.setValidTime(0);
      rpt.setDeliveryReport(0);
      rpt.setArchivationRequested(false);
      rpt.setIntProperty(Tag::SMPP_ESM_CLASS,
        sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST) ||
        (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&3)==1?4:0);
      rpt.setDestinationAddress(sms.getOriginatingAddress());
      rpt.setMessageReference(sms.getMessageReference());
      rpt.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
        sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
      if(umrList.size())
      {
        rpt.setMessageReference(umrList[0]);
        rpt.setIntProperty(Tag::SMSC_RECEIPT_MR,umrList[0]);
      }
      __trace2__("RECEIPT: set mr[0]=%d",rpt.getMessageReference());
      rpt.setIntProperty(Tag::SMPP_MSG_STATE,SmppMessageState::DELIVERED);
      char addr[64];
      sms.getDestinationAddress().getText(addr,sizeof(addr));
      rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
      rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,time(NULL));
      rpt.setIntProperty(Tag::SMSC_RECEIPTED_MSG_SUBMIT_TIME,sms.getSubmitTime());
      char msgid[60];
      sprintf(msgid,"%lld",t.msgId);
      rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
      string out;
      sms.getDestinationAddress().getText(addr,sizeof(addr));
      const Descriptor& d=sms.getDestinationDescriptor();
      __trace2__("RECEIPT: msc=%s, imsi=%s",d.msc,d.imsi);
      char ddest[64];
      Address ddestaddr=sms.getDealiasedDestinationAddress();
      Address tmp;
      if(!smsc->AddressToAlias(ddestaddr,tmp))
      {
        sms.getDealiasedDestinationAddress().getText(ddest,sizeof(ddest));
      }else
      {
        sms.getDestinationAddress().getText(ddest,sizeof(ddest));
      }
      FormatData fd;
      fd.ddest=ddest;
      fd.addr=addr;
      fd.date=time(NULL);
      fd.msgId=msgid;
      fd.err="";
      fd.lastResult=0;
      fd.lastResultGsm=0;
      fd.msc=d.msc;
      smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
      fd.locale=profile.locale.c_str();
      smsc::smeman::SmeInfo si=smsc->getSmeInfo(sms.getSourceSmeId());
      fd.scheme=si.receiptSchemeName.c_str();

      formatDeliver(fd,out);
      rpt.getDestinationAddress().getText(addr,sizeof(addr));
      __trace2__("RECEIPT: sending receipt to %s:%s",addr,out.c_str());
      if(sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
      {
        fillSms(&rpt,"",0,CONV_ENCODING_CP1251,profile.codepage,0);
      }else
      {
        fillSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,0);
      }

      //smsc->submitSms(prpt);

      submitReceipt(rpt,0x4);

      {
        for(int i=1;i<umrList.size();i++)
        {
          rpt.setMessageReference(umrList[i]);
          rpt.setIntProperty(Tag::SMSC_RECEIPT_MR,umrList[i]);
          __trace2__("RECEIPT: set mr[i]=%d",i,rpt.getMessageReference());
          submitReceipt(rpt,0x4);
        }
      }


      /*splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
      for(int i=0;i<arr.Count();i++)
      {
        smsc->submitSms(arr[i]);
      };*/
    }
  }catch(std::exception& e)
  {
    __trace__("DELIVERY_RESP:failed to submit receipt");
  }
  //return skipFinalizing?DELIVERING_STATE:DELIVERED_STATE;
  return DELIVERED_STATE;
}

StateType StateMachine::alert(Tuple& t)
{
  debug2(smsLog, "ALERT: msgId=%lld",t.msgId);
  //time_t now=time(NULL);
  Descriptor d;
  SMS sms;
  if(t.command->get_sms())
  {
    sms=*t.command->get_sms();
  }else if(smsc->getTempStore().Get(t.msgId,sms))
  {
    smsc->getTempStore().Delete(t.msgId);
    smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
    int status=Status::OK;
    try{
      store->createSms(sms,t.msgId,smsc::store::CREATE_NEW);
    }catch(...)
    {
      smsc_log_warn(smsLog, "ALERT: failed to createSms %lld",t.msgId);
      status=Status::SYSERR;
    }
    SmeProxy *src_proxy=smsc->getSmeProxy(sms.srcSmeId);
    if(src_proxy)
    {
      sms.setLastResult(status);
      smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
      char msgId[64];
      sprintf(msgId,"%lld",t.msgId);
      SmscCommand resp = SmscCommand::makeSubmitSmResp
                           (
                             status==Status::OK?msgId:"0",
                             sms.dialogId,
                             status,
                             sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                           );
      try{
        src_proxy->putCommand(resp);
      }catch(...)
      {
        __warning__("ALERT: failed to put response command");
      }
    }
  }else
  {
    try{
      store->retriveSms((SMSId)t.msgId,sms);
    }catch(...)
    {
      smsc_log_warn(smsLog, "ALERT: Failed to retrieve sms:%lld",t.msgId);
      return UNKNOWN_STATE;
    }
  }

  if(!sms.Invalidate(__FILE__,__LINE__))
  {
    smsc_log_warn(smsLog, "Invalidate of %lld failed",t.msgId);
    return ERROR_STATE;
  }

  char bufsrc[64],bufdst[64];
  sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
  sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
  info2(smsLog, "ALERT: delivery timed out(%s->%s), msgId=%lld",bufsrc,bufdst,t.msgId);
  sms.setLastResult(Status::DELIVERYTIMEDOUT);
  smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
  if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1 ||
     (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2)
  {
    sms.state=EXPIRED;
    if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2)
    {
      __trace__("ALERT: Sending submit resp for forward mode sms");
      smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
      SmscCommand resp = SmscCommand::makeSubmitSmResp
                           (
                             "0",
                             sms.dialogId,
                             Status::DELIVERYTIMEDOUT,
                             sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                           );

      SmeProxy *src_proxy=smsc->getSmeProxy(sms.srcSmeId);
      if(src_proxy)
      {
        try{
          src_proxy->putCommand(resp);
        }catch(...)
        {
          __warning__("ALERT: failed to put response command");
        }
      }
    }
    try{
      store->createFinalizedSms(t.msgId,sms);
    }catch(...)
    {
      smsc_log_warn(smsLog, "ALERT: failed to finalize sms:%lld",t.msgId);
    }
    return EXPIRED_STATE;
  }else
  {
    try{
      changeSmsStateToEnroute(sms,t.msgId,d,Status::DELIVERYTIMEDOUT,rescheduleSms(sms));
    }catch(std::exception& e)
    {
      __warning2__("ALERT: failed to change state to enroute:%s",e.what());
    }catch(...)
    {
      __warning__("ALERT: failed to change state to enroute");
    }

    sendNotifyReport(sms,t.msgId,"delivery attempt timed out");
  }
  return UNKNOWN_STATE;
}

StateType StateMachine::replace(Tuple& t)
{
  __trace2__("REPLACE: msgid=%lld",t.msgId);

#define __REPLACE__RESPONSE(status)   \
    try{                              \
      t.command.getProxy()->putCommand\
      (                               \
        SmscCommand::makeReplaceSmResp\
        (                             \
          t.command->get_dialogId(),  \
          Status::status              \
        )                             \
      );                              \
    }catch(...)                       \
    {                                 \
      __trace__("REPLACE: failed to put response command"); \
    }                                 \
    return UNKNOWN_STATE;


  SMS sms;
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    __trace2__("REPLACE: Failed to retrieve sms:%lld",t.msgId);
    __REPLACE__RESPONSE(REPLACEFAIL);
  }
  time_t oldtime=sms.getNextTime();
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    if(sms.getConcatSeqNum()>0)
    {
      __trace__("REPLACE: replace of concatenated message");
      __REPLACE__RESPONSE(REPLACEFAIL);
    }
    sms.getMessageBody().dropProperty(Tag::SMSC_CONCATINFO);
  }
  if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    __trace__("REPLACE: dropping payload");
    sms.getMessageBody().dropProperty(Tag::SMPP_MESSAGE_PAYLOAD);
    sms.getMessageBody().dropProperty(Tag::SMSC_RAW_PAYLOAD);
  }

  try{
    Address addr(t.command->get_replaceSm().sourceAddr.get());
    if(!(sms.getOriginatingAddress()==addr))
    {
      throw Exception("replace failed");
    }
  }
  catch(...)
  {
    __REPLACE__RESPONSE(REPLACEFAIL);
  }


  if(sms.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x03 && (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40))
  {
    if(
        t.command->get_replaceSm().smLength==0 ||
        *((uint8_t*)t.command->get_replaceSm().shortMessage.get())+1 >
        t.command->get_replaceSm().smLength
      )
    {
      __trace__("REPLACE: invalid length of/for UDHI");
      __REPLACE__RESPONSE(REPLACEFAIL);
    }
  }

  sms.setBinProperty
  (
    Tag::SMSC_RAW_SHORTMESSAGE,
    t.command->get_replaceSm().shortMessage.get(),
    t.command->get_replaceSm().smLength
  );
  sms.setIntProperty(Tag::SMPP_SM_LENGTH,t.command->get_replaceSm().smLength);

  /*
  if(sms.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)==0x03 &&
     !strcmp(sms.getDestinationSmeId(),"MAP_PROXY") &&
     t.command->get_replaceSm().smLength>140)
  {
    __trace__("REPLACE: das=3, and smLength>140");
    __REPLACE__RESPONSE(INVMSGLEN);
  }
  */

  if(!strcmp(sms.getDestinationSmeId(),"MAP_PROXY"))
  {
    try{
      int pres=partitionSms(&sms);
      if(pres==psErrorUdhi || pres==psErrorUdhi)
      {
        __trace2__("REPLACE: concatenation failed(%d)",pres);
        __REPLACE__RESPONSE(REPLACEFAIL);
      }
      if(pres==psMultiple)
      {
        Address dst=sms.getDealiasedDestinationAddress();
        uint8_t msgref=smsc->getNextMR(dst);
        sms.setConcatMsgRef(msgref);
        sms.setConcatSeqNum(0);
      }
    }catch(...)
    {
      __warning__("Exception in replace!!!!!!!!!!!!!!");
      __REPLACE__RESPONSE(REPLACEFAIL);
    }
  }


  if(t.command->get_replaceSm().validityPeriod==-1)
  {
    sms.setLastResult(Status::INVEXPIRY);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
    __REPLACE__RESPONSE(INVEXPIRY);
  }
  if(t.command->get_replaceSm().scheduleDeliveryTime==-1)
  {
    sms.setLastResult(Status::INVSCHED);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
    __REPLACE__RESPONSE(INVSCHED);
  }
  time_t newvalid=t.command->get_replaceSm().validityPeriod?
    t.command->get_replaceSm().validityPeriod:sms.getValidTime();
  time_t newsched=t.command->get_replaceSm().scheduleDeliveryTime?
    t.command->get_replaceSm().scheduleDeliveryTime:sms.getNextTime();
  time_t now=time(NULL);
  if(newsched<now)newsched=now;
  if(newvalid>now+maxValidTime)newvalid=now+maxValidTime;

  if(newsched>newvalid)
  {
    sms.setLastResult(Status::INVSCHED);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
    __REPLACE__RESPONSE(INVSCHED);
  }

  sms.setValidTime(newvalid);
  sms.setNextTime(newsched);
  sms.setIntProperty(Tag::SMPP_REGISTRED_DELIVERY,t.command->get_replaceSm().registeredDelivery);

  if(sms.hasIntProperty(Tag::SMPP_DATA_SM))sms.setIntProperty(Tag::SMPP_DATA_SM,0);

  try{
    //Address addr(t.command->get_replaceSm().sourceAddr.get());
    store->replaceSms(t.msgId,sms);
    smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
//  }catch(NoSuchMessageException& e)
//  {
    //
  }catch(...)
  {
    __trace__("REPLACE: replacefailed");
    //
    sms.setLastResult(Status::SYSERR);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
    __REPLACE__RESPONSE(REPLACEFAIL);
  }
  try{
    SmeIndex idx=smsc->getSmeIndex(sms.dstSmeId);
    smsc->getScheduler()->UpdateSmsSchedule(t.msgId,newsched,sms.getDealiasedDestinationAddress());
    t.command.getProxy()->putCommand
    (
      SmscCommand::makeReplaceSmResp
      (
        t.command->get_dialogId(),
        Status::OK
      )
    );
  }catch(...)
  {
    __trace__("REPLACE: failed to put response command");
  }

#undef __REPLACE__RESPONSE
  return ENROUTE;
}

StateType StateMachine::query(Tuple& t)
{
  __trace2__("QUERY: msgId=%lld",t.msgId);
  SMS sms;
  try{
    Address addr(t.command->get_querySm().sourceAddr.get());
    store->retriveSms(t.msgId,sms);
    if(!(sms.getOriginatingAddress()==addr))
    {
      throw 0;
    }
  }catch(...)
  {
    t.command.getProxy()->putCommand
    (
      SmscCommand::makeQuerySmResp
      (
        t.command->get_dialogId(),
        Status::QUERYFAIL,
        t.msgId,
        0,
        0,
        0
      )
    );
    return t.state;
  }
  int state=7;
  switch(sms.getState())
  {
    case ENROUTE:      state=1;break;
    case DELIVERED:    state=2;break;
    case EXPIRED:      state=3;break;
    case DELETED:      state=4;break;
    case UNDELIVERABLE:state=5;break;
  }
  t.command.getProxy()->putCommand
  (
    SmscCommand::makeQuerySmResp
    (
      t.command->get_dialogId(),
      Status::OK,
      t.msgId,
      state==1?0:sms.getLastTime(),
      state,
      0
    )
  );
  return t.state;
}

StateType StateMachine::cancel(Tuple& t)
{
  __trace2__("CANCEL: msgId=%lld",t.msgId);
  SMS sms;
  try{
    Address addr;

    if(!t.command->get_cancelSm().force)
    {
      addr=Address(t.command->get_cancelSm().sourceAddr.get());
    }
    store->retriveSms(t.msgId,sms);

    if(t.command->get_cancelSm().force &&
        (
          !sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT) ||
          sms.getIntProperty(Tag::SMSC_MERGE_CONCAT)==3

        )
      )
    {
      return t.state;
    }


    if(sms.hasBinProperty(Tag::SMSC_CONCATINFO) && sms.getConcatSeqNum()>0)
    {
      throw Exception("CANCEL: attempt to cancel concatenated message already in delivery");
    }

    if(!t.command->get_cancelSm().force) //forced cancel for incomplete concatenated messages
    {
      if(!(sms.getOriginatingAddress()==addr))
      {
        throw Exception("CANCEL: source address doesn't match");
      }
      if(t.command->get_cancelSm().destAddr.get())
      {
        addr=Address(t.command->get_cancelSm().destAddr.get());
        if(!(sms.getDestinationAddress()==addr))
        {
          throw Exception("CANCEL: destination address doesn't match");
        }
      }
    }
  }catch(std::exception& e)
  {
    if(!t.command->get_cancelSm().internall)
    {
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeCancelSmResp
        (
          t.command->get_dialogId(),
          Status::CANCELFAIL
        )
      );
    }
    __warning2__("CANCEL: failed to cancel sms:%s",e.what());
    return t.state;
  }
  try{
    if(!t.command->get_cancelSm().force)
    {
      store->changeSmsStateToDeleted(t.msgId);
      sms.setLastResult(Status::DELETED);
      sendFailureReport(sms,t.msgId,DELETED,"");
    }else
    {
      //cancel timed out merged messages.
      if(sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT) && sms.getIntProperty(Tag::SMSC_MERGE_CONCAT)!=3)
      {
        Descriptor d;
        sms.setLastResult(Status::INCOMPLETECONCATMSG);
        store->changeSmsStateToUndeliverable(t.msgId,d,Status::INCOMPLETECONCATMSG);
        sendFailureReport(sms,t.msgId,UNDELIVERABLE,"");
      }else
      {
        return t.state;
      }
    }
    smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
  }catch(std::exception& e)
  {
    if(!t.command->get_cancelSm().internall)
    {
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeCancelSmResp
        (
          t.command->get_dialogId(),
          Status::CANCELFAIL
        )
      );
    }
    __warning2__("CANCEL: failed to cancel sms:%s",e.what());

    return t.state;
  }
  if(!t.command->get_cancelSm().internall)
  {
    t.command.getProxy()->putCommand
    (
      SmscCommand::makeCancelSmResp
      (
        t.command->get_dialogId(),
        Status::OK
      )
    );
  }
  return t.state;
}


void StateMachine::sendFailureReport(SMS& sms,MsgIdType msgId,int state,const char* reason)
{
  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ||
     sms.getDeliveryReport()==REPORT_NOACK ||
     sms.getIntProperty(Tag::SMSC_SUPPRESS_REPORTS))return;
  bool regdel=(sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==1 ||
              (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==2 ||
              sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST);

  if(!(sms.getDeliveryReport() || regdel))return;
  if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1 ||
     (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2)return;
  SMS rpt;
  rpt.setOriginatingAddress(scAddress);
  char msc[]="";
  char imsi[]="";
  rpt.lastResult=sms.lastResult;
  rpt.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
  rpt.setValidTime(0);
  rpt.setDeliveryReport(0);
  rpt.setArchivationRequested(false);
  rpt.setIntProperty(Tag::SMPP_ESM_CLASS,
    sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST) ||
    (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&3)==1 ||
    (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&3)==2
    ?4:0);
  rpt.setDestinationAddress(sms.getOriginatingAddress());
  rpt.setMessageReference(sms.getMessageReference());
  rpt.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
    sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
  switch(state)
  {
    case ENROUTE:state=SmppMessageState::ENROUTE;break;
    case DELIVERED:state=SmppMessageState::DELIVERED;break;
    case EXPIRED:state=SmppMessageState::EXPIRED;break;
    case UNDELIVERABLE:state=SmppMessageState::UNDELIVERABLE;break;
    case DELETED:state=SmppMessageState::DELETED;break;
  }
  rpt.setIntProperty(Tag::SMPP_MSG_STATE,state);
  char addr[64];
  sms.getDestinationAddress().getText(addr,sizeof(addr));
  rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
  rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,time(NULL));

  char msgid[60];
  sprintf(msgid,"%lld",msgId);
  rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
  //Array<SMS*> arr;
  string out;
  sms.getDestinationAddress().getText(addr,sizeof(addr));
  FormatData fd;
  char ddest[64];
  Address ddestaddr=sms.getDealiasedDestinationAddress();
  Address tmp;
  if(!smsc->AddressToAlias(ddestaddr,tmp))
  {
    sms.getDealiasedDestinationAddress().getText(ddest,sizeof(ddest));
  }else
  {
    sms.getDestinationAddress().getText(ddest,sizeof(ddest));
  }
  fd.ddest=ddest;
  const Descriptor& d=sms.getDestinationDescriptor();
  fd.msc=d.msc;
  fd.addr=addr;
  fd.date=sms.getSubmitTime();
  fd.msgId=msgid;
  fd.err=reason;
  fd.setLastResult(sms.lastResult);
  smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
  fd.locale=profile.locale.c_str();
  smsc::smeman::SmeInfo si=smsc->getSmeInfo(sms.getSourceSmeId());
  fd.scheme=si.receiptSchemeName.c_str();

  formatFailed(fd,out);
  if(sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
  {
    fillSms(&rpt,"",0,CONV_ENCODING_CP1251,profile.codepage,0);
  }else
  {
    fillSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,0);
  }
  //fillSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,140);
  //smsc->submitSms(prpt);
  submitReceipt(rpt,0x4);
  /*splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
  for(int i=0;i<arr.Count();i++)
  {
    smsc->submitSms(arr[i]);
  };*/
}

void StateMachine::sendNotifyReport(SMS& sms,MsgIdType msgId,const char* reason)
{
  try{
    if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ||
       sms.getDeliveryReport()==REPORT_NOACK ||
       sms.getDeliveryReport()==ProfileReportOptions::ReportFinal ||
       sms.getIntProperty(Tag::SMSC_SUPPRESS_REPORTS))return;
    bool regdel=(sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x10)==0x10 ||
                sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST);

    if(!(sms.getDeliveryReport() || regdel))return;
    __trace2__("sendNotifyReport: attemptsCount=%d",sms.getAttemptsCount());
    if(sms.getAttemptsCount()!=0)return;
    if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1 ||
       (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2)return;
    SMS rpt;
    rpt.setOriginatingAddress(scAddress);
    char msc[]="";
    char imsi[]="";
    rpt.lastResult=sms.lastResult;
    rpt.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
    rpt.setValidTime(0);
    rpt.setDeliveryReport(0);
    rpt.setArchivationRequested(false);
    rpt.setIntProperty(Tag::SMPP_ESM_CLASS,0);
    rpt.setDestinationAddress(sms.getOriginatingAddress());
    rpt.setMessageReference(sms.getMessageReference());
    rpt.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
      sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
    rpt.setIntProperty(Tag::SMPP_MSG_STATE,SmppMessageState::ENROUTE);
    char addr[64];
    sms.getDestinationAddress().getText(addr,sizeof(addr));
    rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
    rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,time(NULL));

    char msgid[60];
    sprintf(msgid,"%lld",msgId);
    rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
    //Array<SMS*> arr;
    string out;
    sms.getDestinationAddress().getText(addr,sizeof(addr));
    FormatData fd;
    char ddest[64];
    Address ddestaddr=sms.getDealiasedDestinationAddress();
    Address tmp;
    if(!smsc->AddressToAlias(ddestaddr,tmp))
    {
      sms.getDealiasedDestinationAddress().getText(ddest,sizeof(ddest));
    }else
    {
      sms.getDestinationAddress().getText(ddest,sizeof(ddest));
    }
    fd.ddest=ddest;
    const Descriptor& d=sms.getDestinationDescriptor();
    fd.msc=d.msc;
    fd.date=sms.getSubmitTime();
    fd.addr=addr;
    fd.msgId=msgid;
    fd.err=reason;
    fd.setLastResult(sms.lastResult);
    smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
    fd.locale=profile.locale.c_str();
    smsc::smeman::SmeInfo si=smsc->getSmeInfo(sms.getSourceSmeId());
    fd.scheme=si.receiptSchemeName.c_str();


    formatNotify(fd,out);
    //if(sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
    //{
    //  fillSms(&rpt,"",0,CONV_ENCODING_CP1251,profile.codepage,0);
    //}else
    //{
    fillSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,0);

    //}
    //fillSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,140);
    //smsc->submitSms(prpt);
    submitReceipt(rpt,0x20);
    /*splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
    for(int i=0;i<arr.Count();i++)
    {
      smsc->submitSms(arr[i]);
    };*/
  }catch(...)
  {
    __warning__("notify report failed");
  }
}

time_t StateMachine::rescheduleSms(SMS& sms)
{
  if(sms.getAttemptsCount()>1 && !RescheduleCalculator::isEqualCodes(sms.lastResult,sms.oldResult))
  {
    sms.setAttemptsCount(1);
  }

  time_t basetime=time(NULL);
  time_t nextTryTime=RescheduleCalculator::calcNextTryTime(basetime,sms.getLastResult(),sms.getAttemptsCount());
  if(nextTryTime>sms.getValidTime())nextTryTime=sms.getValidTime();
  if(nextTryTime==-1)nextTryTime=basetime;
  __trace2__("rescheduleSms: bt=%u ntt=%u",basetime,nextTryTime);
  return nextTryTime;
}

void StateMachine::changeSmsStateToEnroute(SMS& sms,SMSId id,const Descriptor& d,
                                           uint32_t failureCause,time_t nextTryTime,
                                           bool skipAttempt)
{
  if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x1 ||
     (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2)return;
  sms.setLastResult(failureCause);
  if(!sms.createdInStore)
  {
    debug2(smsLog, "ENROUTE: msgId=%lld - aborted for non-store message",id);
    return;
  }
  sms.setNextTime(nextTryTime);
  if(failureCause==Status::RESCHEDULEDNOW)
  {
    smsc->getScheduler()->AddScheduledSms(nextTryTime,id,sms.getDealiasedDestinationAddress(),smsc->getSmeIndex(sms.dstSmeId));
  }else
  {
    nextTryTime=smsc->getScheduler()->RescheduleSms(id,sms,smsc->getSmeIndex(sms.dstSmeId));
  }


  debug2(smsLog, "ENROUTE: msgId=%lld;lr=%d;or=%d;ntt=%u;ac=%d",id,sms.getLastResult(),sms.oldResult,nextTryTime,sms.getAttemptsCount());
  store->changeSmsStateToEnroute(id,d,failureCause,nextTryTime,sms.getAttemptsCount()+(skipAttempt?0:1));
}


void StateMachine::submitReceipt(SMS& sms,int type)
{
  SMSId msgId=store->getNextId();
  time_t now=time(NULL);
  sms.setSubmitTime(now);
  Address dst=sms.getDestinationAddress();
  if(!smsc->AliasToAddress(sms.getDestinationAddress(),dst))
  {
    dst=sms.getDestinationAddress();
  }
  sms.setDealiasedDestinationAddress(dst);

  sms.setNextTime(now);
  sms.setValidTime(now+maxValidTime);

  if(!sms.hasIntProperty(Tag::SMPP_ESM_CLASS))
  {
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,0);
  }

  try{
    int dest_proxy_index;
    SmeProxy *dest_proxy;
    smsc::router::RouteInfo ri;
    if(smsc->routeSms(sms.getOriginatingAddress(),sms.getDealiasedDestinationAddress(),dest_proxy_index,dest_proxy,&ri))
    {
      sms.setRouteId(ri.routeId.c_str());
      int prio=sms.getPriority()+ri.priority;
      if(prio>SmeProxyPriorityMax)prio=SmeProxyPriorityMax;
      sms.setPriority(prio);

      sms.setSourceSmeId(smscSmeId.c_str());

      sms.setDestinationSmeId(ri.smeSystemId.c_str());
      sms.setServiceId(ri.serviceId);
      sms.setArchivationRequested(ri.archived);
      sms.setBillingRecord(ri.billing);

      sms.setEServiceType(serviceType.c_str());
      sms.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protocolId);

      Profile profile=smsc->getProfiler()->lookup(dst);
      sms.setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage);
      sms.setIntProperty(Tag::SMSC_UDH_CONCAT,profile.udhconcat);

      if(ri.smeSystemId!="MAP_PROXY")
      {
        sms.setIntProperty(Tag::SMPP_ESM_CLASS,
          (
            sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0xC3 //11000011
          )|type);
      }

      int pres=psSingle;
      if(ri.smeSystemId=="MAP_PROXY")
      {
        pres=partitionSms(&sms);
      }
      if(pres==psMultiple)
      {
        uint8_t msgref=smsc->getNextMR(dst);
        sms.setConcatMsgRef(msgref);
        sms.setConcatSeqNum(0);
      }else if(pres!=psSingle)
      {
        __warning2__("Concatenation error %d for receipt %s->%s",pres,
          sms.getOriginatingAddress().toString().c_str(),
          sms.getDealiasedDestinationAddress().toString().c_str());
        return;
      }

      store->createSms(sms,msgId,smsc::store::CREATE_NEW);
      //smsc->getScheduler()->AddScheduledSms(sms.getNextTime(),msgId,sms.getDealiasedDestinationAddress(),dest_proxy_index);
      smsc->getScheduler()->AddFirstTimeForward(msgId);
    }else
    {
      __warning2__("There is no route for receipt %s->%s",
        sms.getOriginatingAddress().toString().c_str(),
        sms.getDealiasedDestinationAddress().toString().c_str());
    }
  }catch(...)
  {
    __warning__("Failed to create receipt");
  }
}

void StateMachine::submitResp(Tuple& t,SMS* sms,int status)
{
  sms->setLastResult(status);
  smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
  SmscCommand resp = SmscCommand::makeSubmitSmResp
                       (
                         /*messageId*/"0",
                         t.command->get_dialogId(),
                         status,
                         sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                       );
  try{
    t.command.getProxy()->putCommand(resp);
  }catch(...)
  {
    __warning__("SUBMIT_SM: failed to put response command");
  }
}

void StateMachine::finalizeSms(SMSId id,SMS& sms)
{
  if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2)//forward mode (transaction)
  {
    //smsc->registerStatisticalEvent(sms.lastResult==0?StatEvents::etSubmitOk:StatEvents::etSubmitErr,&sms);
    smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
    SmeProxy *src_proxy=smsc->getSmeProxy(sms.srcSmeId);
    if(src_proxy)
    {
      SmscCommand resp=SmscCommand::makeSubmitSmResp
                       (
                         /*messageId*/"0",
                         sms.dialogId,
                         sms.lastResult,
                         sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                       );
      try{
        src_proxy->putCommand(resp);
      }catch(...)
      {
        __warning__("DELIVERYRESP: failed to put transaction response command");
      }
    }
  }
  try{
    store->createFinalizedSms(id,sms);
  }catch(...)
  {
    __warning2__("DELIVERYRESP: failed to finalize sms:%lld",id);
  }
}



}//system
}//smsc
