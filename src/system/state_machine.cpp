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
#include "core/buffers/FixedLengthString.hpp"
#include "closedgroups/ClosedGroupsInterface.hpp"
#include "system/common/TimeZoneMan.hpp"
#ifdef SMSEXTRA
#include "Extra.hpp"
#include "ExtraBits.hpp"
#endif

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

#ifdef SNMP
static void incSnmpCounterForError(int code,const char* sme)
{
  switch(code)
  {
    case 0x14:
      SnmpCounter::getInstance().incCounter(SnmpCounter::cntErr0x14,sme);
      break;
    case 0x58:
      SnmpCounter::getInstance().incCounter(SnmpCounter::cntErr0x58,sme);
      break;
    case 0x440:
    case 0x442:
    case 0x443:
    case 0x444:
    case 0x445:
    case 0x446:
    case 0x447:
      SnmpCounter::getInstance().incCounter(SnmpCounter::cntErrSDP,sme);
      break;
    default:
      SnmpCounter::getInstance().incCounter(SnmpCounter::cntErrOther,sme);
      break;
  }
  SnmpCounter::getInstance().incCounter(SnmpCounter::cntTempError,sme);
}
#endif

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
  dreTemplateParam.Compile("/\\s*\\{(\\w+)\\}=(\".*?\"|[^\"{\\s]*)/s");
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


FixedLengthString<64> AddrPair(const char* s1,const Address& a1,const char* s2,const Address& a2)
{
  char buf[64];
  char buf1[32];
  char buf2[32];
  if(a1==a2)
  {
    a1.toString(buf1,sizeof(buf1));
    sprintf(buf,"%s=%s",s1,buf1);
  }else
  {
    a1.toString(buf1,sizeof(buf1));
    a2.toString(buf2,sizeof(buf2));
    sprintf(buf,"%s=%s;%s=%s",s1,buf1,s2,buf2);
  }
  return buf;
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
        case INSMSCHARGERESPONSE:st=submitChargeResp(t);break;
        case DELIVERY_RESP:st=deliveryResp(t);break;
        case FORWARD:st=forward(t);break;
        case INFWDSMSCHARGERESPONSE:st=forwardChargeResp(t);break;
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

void StateMachine::processDirectives(SMS& sms,Profile& p,Profile& srcprof)
{
  const char *body="";
  TmpBuf<char,256> tmpBuf(0);
  unsigned int len=0;
  int dc=sms.getIntProperty(Tag::SMPP_DATA_CODING);
  if(dc==DataCoding::BINARY)return;
  bool udhi=(sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)==0x40;
  int udhLen=0;
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    SMS tmp(sms);
    extractSmsPart(&tmp,0);
    if(tmp.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      const char* tmpBody=tmp.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
      char *dst=tmpBuf.setSize(len);
      body=dst;
      memcpy(dst,tmpBody,len);
    }else
    {
      const char* tmpBody=tmp.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
      char *dst=tmpBuf.setSize(len);
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
    udhLen=(*(const unsigned char*)body)+1;
    len-=udhLen;
    body+=udhLen;
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

  int lastDirectiveSymbol=0;
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
      lastDirectiveSymbol=m[0].end;
      i=m[0].end;
    }else
    if(dreNoAck.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      __trace__("DIRECT: noack found");
      sms.setDeliveryReport(REPORT_NOACK);
      lastDirectiveSymbol=m[0].end;
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
      lastDirectiveSymbol=m[0].end;
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
      lastDirectiveSymbol=m[0].end;
      i=m[0].end;
    }else
    if(dreNoTrans.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      __trace__("DIRECT: notrans");
      sms.setIntProperty(Tag::SMSC_TRANSLIT,0);
      lastDirectiveSymbol=m[0].end;
      i=m[0].end;
    }else
    if(dreFlash.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      if(!sms.hasIntProperty(Tag::SMSC_FORCE_DC))
      {
        sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,1);
      }
      lastDirectiveSymbol=m[0].end;
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
      lastDirectiveSymbol=m[0].end;
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
        if(m[2].end!=m[2].start)
        {
          value.assign
          (
            buf+m[2].start+(buf[m[2].start]=='"'?1:0),
            m[2].end-m[2].start-(buf[m[2].start]=='"'?2:0)
          );
        }else
        {
          value="";
        }
        __trace2__("DIRECT: found template param %s=%s",name.c_str(),value.c_str());
        if(f)
        {
          int et=f->getEntityType(name.c_str());
          switch(et)
          {
            case ET_DATE:
            {
              int day,month,year,hh,mm,ss,nn;
              time_t t;
              if(sscanf(value.c_str(),SMSC_DBSME_IO_DEFAULT_PARSE_PATTERN,
                &day,&month,&year,&hh,&mm,&ss,&nn)!=6)
              {
                sscanf(value.c_str(),"%d",&t);
              }else
              {
                tm tmptm;
                tmptm.tm_sec=ss;
                tmptm.tm_min=mm;
                tmptm.tm_hour=hh;
                tmptm.tm_mday=day;
                tmptm.tm_mon=month;
                tmptm.tm_year=year-1900;
                t=mktime(&tmptm);
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
      if(j!=len)
      {
        __warning2__("tail of template '%s' arguments wasn't parsed:'%s'",tmplname.c_str(),buf+j);
      }
      lastDirectiveSymbol=j;
      tmplstart=i;
      tmpllen=j-i;
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
  if(lastDirectiveSymbol==0)return;
  if(tmplname.length())
  {
    lastDirectiveSymbol=len;
  }
  if(sms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::SMSC7BIT)
  {
    int pos=0;
    int fix=0;
    for(int j=0;j<len;j++)
    {
      if(strchr(escchars,buf[j]))lastDirectiveSymbol++;
    }
  }
  if(sms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
  {
    lastDirectiveSymbol*=2;
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
    memcpy(ptr,body-udhLen,udhLen);
    ptr+=udhLen;
  }

  int tailLen=olen-udhLen-lastDirectiveSymbol;
  if(tailLen)
  {
    memcpy(ptr,body+lastDirectiveSymbol,tailLen);
    ptr+=tailLen;
  }
  int newlen=tailLen+udhLen;
  if(newtext.length())
  {
    bool hb=hasHighBit(newtext.c_str(),newtext.length());
    if(dc!=DataCoding::UCS2 && hb)
    {
      if(dc==DataCoding::LATIN1)
      {
        auto_ptr<short> b(new short[newlen-udhLen]);
        ConvertMultibyteToUCS2(newBody+udhLen,newlen-udhLen,b.get(),(newlen-udhLen)*2,CONV_ENCODING_CP1251);
        memcpy(newBody+udhLen,b.get(),(newlen-udhLen)*2);
      }else //SMSC7BIT
      {
        TmpBuf<char,256> x(newlen-udhLen+1);
        int cvtlen=ConvertSMSC7BitToLatin1(newBody+udhLen,newlen-udhLen,x.get());
        ConvertMultibyteToUCS2
        (
          x.get(),
          cvtlen,
          (short*)newBody+udhLen,
          (newlen-udhLen)*2,
          CONV_ENCODING_CP1251
        );
      }
      sms.setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::UCS2);
      dc=DataCoding::UCS2;
      newlen=2*newlen-udhLen;
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

  if(profile.closedGroupId!=0 && !smsc::closedgroups::ClosedGroupsInterface::getInstance()->Check(profile.closedGroupId,sms->getDealiasedDestinationAddress()))
  {
    info2(smsLog,"SBM: msgId=%lld, denied by closed group(%d:'%s') check",
      t.msgId,profile.closedGroupId,
      smsc::closedgroups::ClosedGroupsInterface::getInstance()->GetClosedGroupName(profile.closedGroupId)
    );
    submitResp(t,sms,Status::DENIEDBYCLOSEDGROUP);
    return ERROR_STATE;
  }

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
  if(profile.hide==HideOption::hoSubstitute)
  {
    Address addr;
    if(smsc->AddressToAlias(sms->getOriginatingAddress(),addr))
    {
      info2(smsLog,"msgId=%lld: oa subst: %s->%s",t.msgId,sms->getOriginatingAddress().toString().c_str(),addr.toString().c_str());
      sms->setOriginatingAddress(addr);
    }
  }

  if(!sms->hasIntProperty(Tag::SMSC_TRANSLIT))
  {
    sms->setIntProperty(Tag::SMSC_TRANSLIT,profile.translit);
    debug2(smsLog,"msgId=%lld, set translit to %d",t.msgId,sms->getIntProperty(Tag::SMSC_TRANSLIT));
  }

  if(sms->getIntProperty(Tag::SMPP_SET_DPF) && (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)!=2)
  {
    sms->setIntProperty(Tag::SMPP_SET_DPF,0);
  };


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
    try{
      if(smsc->routeSms(sms->getOriginatingAddress(),divDst,idx,prx,&ri2,src_proxy->getSmeIndex()))
      {
        if(ri2.smeSystemId!="MAP_PROXY")
        {
          warn2(smsLog,"attempt to divert to non-map address(sysId=%s):%s->%s",
            ri2.smeSystemId.c_str(),
            sms->getOriginatingAddress().toString().c_str(),divDst.toString().c_str());
          goto divert_failed;
        }
      }
    }catch(std::exception& e)
    {
      warn2(smsLog,"routing failed during divert check:%s",e.what());
      goto divert_failed;
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

  sms->setIntProperty(Tag::SMSC_ORIGINALPARTSNUM,1);



  smsc::router::RouteInfo ri;

  ////
  //
  //  Routing here
  //

  bool has_route = false;

  try{
    has_route=smsc->routeSms(sms->getOriginatingAddress(),
                            dst,
                            dest_proxy_index,dest_proxy,&ri,src_proxy->getSmeIndex());
  }catch(std::exception& e)
  {
    warn2(smsLog,"Routing %s->%s failed:%s",sms->getOriginatingAddress().toString().c_str(),
      dst.toString().c_str(),e.what());
  }

  if ( !has_route )
  {
    submitResp(t,sms,Status::NOROUTE);
    warn2(smsLog, "SBM: no route Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",dst).c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  bool fromDistrList=src_proxy && !strcmp(src_proxy->getSystemId(),"DSTRLST");
  bool fromMap=src_proxy && !strcmp(src_proxy->getSystemId(),"MAP_PROXY");
  bool toMap=dest_proxy && !strcmp(dest_proxy->getSystemId(),"MAP_PROXY");

#ifdef SMSEXTRA
  bool noDestChange=false;
  if((fromMap || fromDistrList) && toMap)
  {
    ExtraInfo::ServiceInfo xsi;
    int extrabit=ExtraInfo::getInstance().checkExtraService(*sms,xsi);
    if(extrabit)
    {
      info2(smsLog,"EXTRA: service with bit=%x detected for abonent %s",xsi.serviceBit,sms->getOriginatingAddress().toString().c_str());
    }
    if((srcprof.subscription&EXTRA_NICK) && (srcprof.hide==HideOption::hoEnabled || xsi.serviceBit==EXTRA_NICK))
    {
      sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_NICK);
      sms->setIntProperty(Tag::SMSC_HIDE,HideOption::hoEnabled);
      info2(smsLog,"EXTRA: smsnick for abonent %s",sms->getOriginatingAddress().toString().c_str());
    }
    if((srcprof.subscription&EXTRA_FLASH) || xsi.serviceBit==EXTRA_FLASH)
    {
      sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_FLASH);
      sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,1);
      info2(smsLog,"EXTRA: smsflash for abonent %s",sms->getOriginatingAddress().toString().c_str());
    }
    if(extrabit && xsi.diverted)
    {
      sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,xsi.serviceBit|(sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)&EXTRA_FLASH));
      sms->setIntProperty(Tag::SMSC_HIDE,HideOption::hoDisabled);
      //sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,0);
      sms->setIntProperty(Tag::SMPP_ESM_CLASS,(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&~3)|2);
      dst=xsi.divertAddr;
      sms->setDestinationAddress(sms->getDealiasedDestinationAddress());
      info2(smsLog,"EXTRA: divert for abonent %s to %s",sms->getOriginatingAddress().toString().c_str(),xsi.divertAddr.toString().c_str());
      noDestChange=true;
      try{
        has_route=smsc->routeSms(sms->getOriginatingAddress(),
                                dst,
                                dest_proxy_index,dest_proxy,&ri,src_proxy->getSmeIndex());
      }catch(std::exception& e)
      {
        warn2(smsLog,"Routing %s->%s failed:%s",sms->getOriginatingAddress().toString().c_str(),
          dst.toString().c_str(),e.what());
      }
      if(!has_route)
      {
        submitResp(t,sms,Status::NOROUTE);
        warn2(smsLog, "SBM: no route(extra divert) Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
          t.msgId,dialogId,
          sms->getOriginatingAddress().toString().c_str(),
          AddrPair("da",sms->getDestinationAddress(),"dda",dst).c_str(),
          src_proxy->getSystemId()
        );
        return ERROR_STATE;
      }
    }
  }
#endif


  __trace2__("hide=%s, forceRP=%d",ri.hide?"true":"false",ri.replyPath);

  sms->setRouteId(ri.routeId.c_str());
  if(ri.suppressDeliveryReports)sms->setIntProperty(Tag::SMSC_SUPPRESS_REPORTS,1);
  int prio=sms->getPriority()+ri.priority;
  if(prio>SmeProxyPriorityMax)prio=SmeProxyPriorityMax;
  sms->setPriority(prio);

  debug2(smsLog,"SBM: route %s->%s found:%s",
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",dst).c_str(),
      ri.routeId.c_str());

  if(ri.transit)
  {
    if(sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
    {
      sms->getMessageBody().dropIntProperty(Tag::SMSC_MERGE_CONCAT);
    }
  }

  sms->setIntProperty(Tag::SMSC_PROVIDERID,ri.providerId);
  sms->setIntProperty(Tag::SMSC_CATEGORYID,ri.categoryId);

  bool aclCheck=false;
  std::string aclAddr;

  if(toMap)
  {
    if(sms->hasIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND))
    {
      debug1(smsLog,"drop SMPP_MORE_MESSAGES_TO_SEND");
      sms->getMessageBody().dropIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND);
    }
  }

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

  if((fromMap || !strcmp(src_proxy->getSystemId(),"DSTRLST")) && toMap)//peer2peer
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,smsc->p2pChargePolicy);
  }else
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,smsc->otherChargePolicy);
  }

#ifdef SMSEXTRA
  if(ri.billing==2)
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,Smsc::chargeOnSubmit);
    sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_INCHARGE);
  }
#endif


  if(aclCheck && ri.aclId!=-1 && !smsc->getAclMgr()->isGranted(ri.aclId,aclAddr))
  {
    submitResp(t,sms,Status::NOROUTE);
    char buf1[32];
    char buf2[32];
    sms->getOriginatingAddress().toString(buf1,sizeof(buf1));
    dst.toString(buf2,sizeof(buf2));
    warn2(smsLog, "SBM: acl access denied (aclId=%d) Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
      ri.aclId,
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",dst).c_str(),
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
      warn2(smsLog, "SBM: call barred Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        AddrPair("da",sms->getDestinationAddress(),"dda",dst).c_str(),
        src_proxy->getSystemId()
      );
      return ERROR_STATE;
    }
  }

  SmeInfo dstSmeInfo=smsc->getSmeInfo(dest_proxy_index);

#ifdef SMSEXTRA
  if((sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)&EXTRA_NICK) &&
     (
       !dstSmeInfo.wantAlias ||
       !ri.hide
     )
    )
  {
    info2(smsLog,"EXTRA: smsnick not allowed for route %s",ri.routeId.c_str());
    sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)&~EXTRA_NICK);
  }

  if(fromMap && toMap && srcprof.sponsored>0)
  {
    info2(smsLog,"EXTRA: sponsored sms for abonent %s(cnt=%d)",sms->getOriginatingAddress().toString().c_str(),srcprof.sponsored);
    sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_SPONSORED);
    smsc->getProfiler()->decrementSponsoredCount(sms->getOriginatingAddress());
  }
#endif

  if((dstSmeInfo.accessMask&srcprof.accessMaskOut)==0)
  {
    info2(smsLog,"SBM: msgId=%lld, denied by access out mask (%s=%x,%s=%x",t.msgId,dstSmeInfo.systemId.c_str(),dstSmeInfo.accessMask,sms->getOriginatingAddress().toString().c_str(),srcprof.accessMaskOut);
    submitResp(t,sms,Status::DENIEDBYACCESSMASK);
    return ERROR_STATE;
  }

  if((src_proxy->getAccessMask()&profile.accessMaskIn)==0)
  {
    info2(smsLog,"SBM: msgId=%lld, denied by access in mask(%s=%x,%s=%x",t.msgId,src_proxy->getSystemId(),src_proxy->getAccessMask(),sms->getDestinationAddress().toString().c_str(),profile.accessMaskIn);
    submitResp(t,sms,Status::DENIEDBYACCESSMASK);
    return ERROR_STATE;
  }

  if((srcprof.accessMaskOut&profile.accessMaskIn)==0)
  {
    info2(smsLog,"SBM: msgId=%lld, denied by access masks(%s=%x,%s=%x",t.msgId,sms->getOriginatingAddress().toString().c_str(),srcprof.accessMaskOut,sms->getDestinationAddress().toString().c_str(),profile.accessMaskIn);
    submitResp(t,sms,Status::DENIEDBYACCESSMASK);
    return ERROR_STATE;
  }

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
    sms->setIntProperty( Tag::SMPP_ESM_CLASS, sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x02 );
    isForwardTo = true;

    sms->getMessageBody().dropIntProperty(Tag::SMSC_MERGE_CONCAT);
    sms->getMessageBody().dropProperty(Tag::SMSC_DC_LIST);
  }

  bool generateDeliver=true; // do not generate in case of merge-concat
  bool allowCreateSms=true;

  bool needToSendResp=true;

  bool noPartitionSms=false; // do not call partitionSms if true!

  if(!ri.transit)
  {
    if(!extactConcatInfoToSar(*sms))
    {
      warn2(smsLog,"extactConcatInfoToSar failed. msgId=%lld, from %s to %s",
                            t.msgId,
                            sms->getOriginatingAddress().toString().c_str(),
                            sms->getDestinationAddress().toString().c_str());
      submitResp(t,sms,Status::INVOPTPARAMVAL);
      return ERROR_STATE;
    }


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
  }


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
        unsigned lenMo;
        const char *bodyMo=sms->getBinProperty(Tag::SMSC_MO_PDU,&lenMo);
        if(lenMo>512)
        {
          warn2(smsLog,"MO LEN=%d",lenMo);
        }
        tmp.assign(bodyMo,lenMo);
        sms->setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::BINARY);
        dc=DataCoding::BINARY;
      }
      sms->setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),tmp.length());
      sms->getMessageBody().dropProperty(Tag::SMPP_SHORT_MESSAGE);
      sms->getMessageBody().dropProperty(Tag::SMSC_RAW_SHORTMESSAGE);
      sms->setIntProperty(Tag::SMPP_SM_LENGTH,0);
      sms->setIntProperty(Tag::SMSC_ORIGINALPARTSNUM,num);

      if(sms->getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM))
      {
        sms->getMessageBody().dropIntProperty(Tag::SMPP_SAR_MSG_REF_NUM);
        sms->getMessageBody().dropIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        sms->getMessageBody().dropIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
      }

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
      SMS newsms;
      try{
        store->retriveSms(t.msgId,newsms);
      }catch(...)
      {
        warn2(smsLog, "sms with id %lld not found or store error",t.msgId);
        submitResp(t,sms,Status::SYSERR);
        return ERROR_STATE;
      }
      if(!newsms.hasIntProperty(Tag::SMSC_MERGE_CONCAT))
      {
        warn2(smsLog, "smsId=%lld:one more part of concatenated message received, but all parts are collected.",t.msgId);
        submitResp(t,sms,Status::SUBMITFAIL);
        return ERROR_STATE;
      }
      if(sms->hasIntProperty(Tag::SMPP_REGISTRED_DELIVERY) && !newsms.hasIntProperty(Tag::SMPP_REGISTRED_DELIVERY))
      {
        newsms.setIntProperty(Tag::SMPP_REGISTRED_DELIVERY,sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY));
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
            return ENROUTE_STATE;
          }
        }
      }else
      {
        if(newsms.hasBinProperty(Tag::SMSC_DC_LIST))
        {
          unsigned dclen;
          newsms.getBinProperty(Tag::SMSC_DC_LIST,&dclen);
          if(dclen!=num)
          {
            warn2(smsLog, "smsId=%lld: different number of parts detected %d!=%d.",t.msgId,dclen,num);
            submitResp(t,sms,Status::INVOPTPARAMVAL);
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
        unsigned ulen;
        unsigned char* umrList=(unsigned char*)newsms.getBinProperty(Tag::SMSC_UMR_LIST,&ulen);
        if(idx<=ulen)
        {
          umrList[idx-1]=sms->getMessageReference();
          newsms.setBinProperty(Tag::SMSC_UMR_LIST,(const char*)umrList,ulen);

          if(newsms.hasBinProperty(Tag::SMSC_UMR_LIST_MASK))
          {
            unsigned mlen;
            char* mask=(char*)newsms.getBinProperty(Tag::SMSC_UMR_LIST_MASK,&mlen);
            if(idx<=mlen)
            {
              mask[idx-1]=1;
              sms->setBinProperty(Tag::SMSC_UMR_LIST_MASK,mask,mlen);
            }
          }
        }
      }

      if(newsms.hasBinProperty(Tag::SMSC_DC_LIST))
      {
        unsigned dclen;
        unsigned char* dcList=(unsigned char*)newsms.getBinProperty(Tag::SMSC_DC_LIST,&dclen);
        if(idx<=dclen)
        {
          __trace2__("dc_list[%d]=%d",idx-1,dc);
          dcList[idx-1]=dc;
          newsms.setBinProperty(Tag::SMSC_DC_LIST,(const char*)dcList,dclen);
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
      try{
        store->replaceSms(t.msgId,newsms);
      }catch(...)
      {
        warn2(smsLog, "Failed to replace sms with id=%lld",t.msgId);
        submitResp(t,&newsms,Status::SUBMITFAIL);
        return ERROR_STATE;
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
          warn2(smsLog, "SBM: failed to put response command SUBMIT_OK Id=%lld;seq=%d;oa=%s;%s;srcprx=%s;dstprx=%s",
            t.msgId,dialogId,
            sms->getOriginatingAddress().toString().c_str(),
            AddrPair("da",sms->getDestinationAddress(),"dda",dst).c_str(),
            src_proxy->getSystemId(),
            ri.smeSystemId.c_str()
          );
        }
        return ENROUTE_STATE;
      }

      *sms=newsms;

      /*
      int status=Status::OK;

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
      */
      //smsc->registerStatisticalEvent(StatEvents::etSubmitOk,sms);

      allowCreateSms=false;

    }
  }

  //
  //  End of merging
  //
  ////


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


  bool isDatagram=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1;
  bool isTransaction=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;


  if( !isForwardTo && !ri.transit)
  {

    sms->getMessageBody().dropProperty(Tag::SMSC_MO_PDU);

    ////
    //
    // Override delivery mode if specified in config and default mode in sms
    //

    if( ri.deliveryMode != smsc::sms::SMSC_DEFAULT_MSG_MODE)
    {
      if(sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
      {
        smsc_log_warn(smsLog,"Attempt to send multipart message in forward mode with route '%s'",ri.routeId.c_str());
      }else
      {
        int esmcls = sms->getIntProperty( Tag::SMPP_ESM_CLASS );
        // following if removed at 25.09.2006 by request of customers
        //if( (esmcls&0x3) == smsc::sms::SMSC_DEFAULT_MSG_MODE )
        {
          // allow override
          sms->setIntProperty( Tag::SMPP_ESM_CLASS, (esmcls&~0x03)|(ri.deliveryMode&0x03) );
          isDatagram=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1;
          isTransaction=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;
        }
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

  }

  if(!generateDeliver)
  {
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

        SMSId replaceId=store->createSms(*sms,t.msgId,rip?smsc::store::SMPP_OVERWRITE_IF_PRESENT:smsc::store::CREATE_NEW);
        if(rip && replaceId!=t.msgId)
        {
          smsc->getScheduler()->CancelSms(replaceId,sms->getDealiasedDestinationAddress());
        }

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



    if(!isDatagram && !isTransaction && needToSendResp) // Store&Forward mode
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
          sms->getDestinationSmeId()
        );
      }
    }


    __trace__("leave non merged sms in enroute state");
    return ENROUTE_STATE;
  }

  INSmsChargeResponse::SubmitContext ctx;
  ctx.srcProxy=src_proxy;
  ctx.dstProxy=dest_proxy;
  ctx.allowCreateSms=allowCreateSms;
  ctx.needToSendResp=needToSendResp;
  ctx.dialogId=dialogId;
  ctx.dest_proxy_index=dest_proxy_index;
  ctx.isForwardTo=isForwardTo;
  ctx.diverted=diverted;
  ctx.routeHide=ri.hide;
  ctx.dst=dst;
  ctx.transit=ri.transit;
  ctx.replyPath=ri.replyPath;
  ctx.priority=ri.priority;
#ifdef SMSEXTRA
  ctx.noDestChange=noDestChange;
#endif
  if(ri.billing)
  {
    try{
      smsc->ChargeSms(t.msgId,*sms,ctx);
    }catch(std::exception& e)
    {
      submitResp(t,sms,Status::NOCONNECTIONTOINMAN);
      warn2(smsLog,"SBM:ChargeSms failed:%s",e.what());
      return ERROR_STATE;
    }
    return CHARGING_STATE;
  }else
  {
    Tuple t2;
    t2.msgId=t.msgId;
    t2.state=UNKNOWN_STATE;
    t2.command=SmscCommand::makeINSmsChargeResponse(t.msgId,*sms,ctx,1);
    return submitChargeResp(t2);
  }
}

StateType StateMachine::submitChargeResp(Tuple& t)
{
  INSmsChargeResponse* resp=t.command->get_chargeSmsResp();
  SMS* sms=&resp->sms;
  time_t stime=sms->getNextTime();
  time_t now=time(NULL);

  bool allowCreateSms=resp->cntx.allowCreateSms;
  bool needToSendResp=resp->cntx.needToSendResp;
  SmeProxy* src_proxy=resp->cntx.srcProxy;
  SmeProxy* dest_proxy=resp->cntx.dstProxy;
  int dialogId=resp->cntx.dialogId;
  int dest_proxy_index=resp->cntx.dest_proxy_index;
  bool isForwardTo=resp->cntx.isForwardTo;
  bool diverted=resp->cntx.diverted;
  bool routeHide=resp->cntx.routeHide;
  Address dst=resp->cntx.dst;
  bool transit=resp->cntx.transit;
  smsc::router::ReplyPath replyPath=resp->cntx.replyPath;
  int priority=resp->cntx.priority;
#ifdef SMSEXTRA
  bool noDestChange=resp->cntx.noDestChange;
#endif

  SmeInfo dstSmeInfo=smsc->getSmeInfo(dest_proxy_index);

  t.command.setProxy(src_proxy);
  t.command->set_dialogId(dialogId);

  if(!resp->result)
  {
    submitResp(t,sms,Status::DENIEDBYINMAN);
    warn2(smsLog, "SBM: denied by inman Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",dst).c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }


  ////
  //
  // Store sms to database
  //

  __require__(!(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) &&
                sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE)   &&
                sms->getIntProperty(Tag::SMPP_SM_LENGTH)!=0));

  bool isDatagram=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1;
  bool isTransaction=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;


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

      SMSId replaceId=store->createSms(*sms,t.msgId,rip?smsc::store::SMPP_OVERWRITE_IF_PRESENT:smsc::store::CREATE_NEW);
      if(rip && replaceId!=t.msgId)
      {
        smsc->getScheduler()->CancelSms(replaceId,sms->getDealiasedDestinationAddress());
      }

    }catch(...)
    {
      __warning2__("failed to create sms with id %lld",t.msgId);
      submitResp(t,sms,Status::SYSERR);
      smsc->ReportDelivery(resp->cntx.inDlgId,*sms,true,Smsc::chargeAlways);
      return ERROR_STATE;
    }
  }


  smsc->ReportDelivery(resp->cntx.inDlgId,*sms,true,Smsc::chargeOnSubmit);

  //
  // stored
  //
  ////



  if(!isDatagram && !isTransaction && needToSendResp) // Store&Forward mode
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
    SmscCommand response = SmscCommand::makeSubmitSmResp
                         (
                           buf,
                           dialogId,
                           Status::OK,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(response);
    }catch(...)
    {
      warn2(smsLog, "SBM: failed to put response command SUBMIT_OK Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId(),
        sms->getDestinationSmeId()
      );
    }
  }


  __trace2__("Sms scheduled to %d, now %d",(int)sms->getNextTime(),(int)now);
  if(!isDatagram && !isTransaction && stime>now)
  {
    smsc->getScheduler()->AddScheduledSms(t.msgId,*sms,dest_proxy_index);
    sms->setLastResult(Status::DEFERREDDELIVERY);
    smsc->ReportDelivery(resp->cntx.inDlgId,*sms,false,Smsc::chargeOnDelivery);
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

      if(!sandf)
      {
        if(sms->lastResult!=Status::OK)
        {
          sm->smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
#ifdef SNMP
          SnmpCounter::getInstance().incCounter(SnmpCounter::cntRejected,sms->getSourceSmeId());
#endif
        }else
        {
          if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x1)//datagram mode
          {
            sm->smsc->registerStatisticalEvent(StatEvents::etSubmitOk,sms);
#ifdef SNMP
            SnmpCounter::getInstance().incCounter(SnmpCounter::cntAccepted,sms->getSourceSmeId());
#endif
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
            __warning__("SUBMIT: failed to put response command");
          }
        }
      }
    }
  };

  ResponseGuard respguard(sms,src_proxy,this,t.msgId);

  struct DeliveryReportGuard{
    Smsc* smsc;
    SMS* sms;
    INSmsChargeResponse::SubmitContext* cntx;
    bool final;
    bool active;
    DeliveryReportGuard():active(true){}
    ~DeliveryReportGuard()
    {
      if(active)
      {
        smsc->ReportDelivery(cntx->inDlgId,*sms,final,Smsc::chargeOnDelivery);
      }
    }
  };
  DeliveryReportGuard repGuard;
  repGuard.smsc=smsc;
  repGuard.sms=sms;
  repGuard.cntx=&resp->cntx;
  repGuard.final=isDatagram || isTransaction;

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
      sms->getDestinationSmeId()
    );
#ifdef SNMP
    incSnmpCounterForError(Status::SMENOTCONNECTED,sms->getDestinationSmeId());
#endif
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
#ifdef SNMP
    incSnmpCounterForError(Status::SMENOTCONNECTED,sms->getSourceSmeId());
#endif
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
      sms->getDestinationSmeId()
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
  task.inDlgId=resp->cntx.inDlgId;
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
      sms->getDestinationSmeId()
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
      sms->getDestinationSmeId()
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
      __trace2__("SUBMIT: wantAlias=%s, hide=%s",dstSmeInfo.wantAlias?"true":"false",HideOptionToText(sms->getIntProperty(Tag::SMSC_HIDE)));
      if(
          dstSmeInfo.wantAlias &&
          sms->getIntProperty(Tag::SMSC_HIDE)==HideOption::hoEnabled &&
          routeHide &&
          smsc->AddressToAlias(sms->getOriginatingAddress(),src)
        )
      {
        sms->setOriginatingAddress(src);
      }
#ifdef SMSEXTRA
      if(!noDestChange)
      {
#endif
      sms->setDestinationAddress(dst);
#ifdef SMSEXTRA
      }
#endif

      // profile lookup performed before partitioning
      //profile=smsc->getProfiler()->lookup(dst);
      //
      if(!sms->hasBinProperty(Tag::SMSC_CONCATINFO) && sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3 && !transit)
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

    if(replyPath==smsc::router::ReplyPathForce)
    {
      sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x80);
    }else if(replyPath==smsc::router::ReplyPathSuppress)
    {
      sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x80));
    }

    smsc_log_debug(smsLog,"SBM: msgId=%lld, esm_class=%x",t.msgId,sms->getIntProperty(Tag::SMPP_ESM_CLASS));

    SmscCommand delivery = SmscCommand::makeDeliverySm(*sms,dialogId2);
    unsigned bodyLen=0;
    delivery->get_sms()->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&bodyLen);
    __trace2__("SUBMIT: delivery.sms.sm_length=%d",bodyLen);
    int prio=priority/1000;
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
      sms->getDestinationSmeId(),
      errstr.c_str()
    );
    sms->setOriginatingAddress(srcOriginal);
    sms->setDestinationAddress(dstOriginal);
    sms->setLastResult(err);
#ifdef SNMP
    if(Status::isErrorPermanent(err))
    {
      SnmpCounter::getInstance().incCounter(SnmpCounter::cntFailed,src_proxy->getSystemId());
    }else
    {
      incSnmpCounterForError(err,src_proxy->getSystemId());
    }
#endif
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
          repGuard.final=true;
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
  repGuard.active=false;

  sms->lastResult=Status::OK;
  info2(smsLog, "SBM: submit ok, seqnum=%d Id=%lld;seq=%d;%s;%s;srcprx=%s;dstprx=%s",
    dialogId2,
    t.msgId,dialogId,
    AddrPair("oa",sms->getOriginatingAddress(),"ooa",srcOriginal).c_str(),
    AddrPair("da",dstOriginal,"dda",sms->getDestinationAddress()).c_str(),
    src_proxy->getSystemId(),
    sms->getDestinationSmeId()
  );
  return DELIVERING_STATE;
}

StateType StateMachine::forward(Tuple& t)
{
  debug2(smsLog,"FWD: id=%lld",t.msgId);
  SMS sms;
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    smsc_log_warn(smsLog, "FWD: failed to retriveSms %lld",t.msgId);
    smsc->getScheduler()->InvalidSms(t.msgId);
    return UNKNOWN_STATE;
  }
#ifdef SNMP
  SnmpCounter::getInstance().incCounter(SnmpCounter::cntRetried,sms.getDestinationSmeId());
#endif

  smsc_log_debug(smsLog,"orgMSC=%s, orgIMSI=%s",sms.originatingDescriptor.msc,sms.originatingDescriptor.imsi);
  INFwdSmsChargeResponse::ForwardContext ctx;
  ctx.allowDivert=t.command->get_forwardAllowDivert();
  ctx.reschedulingForward=t.command->is_reschedulingForward();
  if(sms.billingRecord && !sms.hasIntProperty(Tag::SMSC_CHARGINGPOLICY))
  {
    if(strcmp(sms.getSourceSmeId(),"MAP_PROXY")==0 && strcmp(sms.getDestinationSmeId(),"MAP_PROXY")==0)
    {
      sms.setIntProperty(Tag::SMSC_CHARGINGPOLICY,smsc->p2pChargePolicy);
    }else
    {
      sms.setIntProperty(Tag::SMSC_CHARGINGPOLICY,smsc->otherChargePolicy);
    }
    try{
      store->replaceSms(t.msgId,sms);
    }catch(std::exception& e)
    {
      warn2(smsLog,"Failed to replace sms with msgId=%lld in store:%s",t.msgId,e.what());
    }
  }

  if(sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnDelivery  && sms.billingRecord)
  {
    try{
      smsc->ChargeSms(t.msgId,sms,ctx);
    }catch(std::exception& e)
    {
      warn2(smsLog,"FWD: ChargeSms for id=%lld failed:%s",t.msgId,e.what());
      sms.setLastResult(Status::NOCONNECTIONTOINMAN);
      smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
      smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
      Descriptor d;
      changeSmsStateToEnroute(sms,t.msgId,d,Status::NOCONNECTIONTOINMAN,rescheduleSms(sms));
      return ENROUTE_STATE;
    }
    return CHARGINGFWD_STATE;
  }else
  {
    t.command=SmscCommand::makeINFwdSmsChargeResponse(t.msgId,sms,ctx,1);
    return forwardChargeResp(t);
  }
}


StateType StateMachine::forwardChargeResp(Tuple& t)
{
  SMS& sms=t.command->get_fwdChargeSmsResp()->sms;
  int  inDlgId=t.command->get_fwdChargeSmsResp()->cntx.inDlgId;
  bool allowDivert=t.command->get_fwdChargeSmsResp()->cntx.allowDivert;
  bool isReschedulingForward=t.command->get_fwdChargeSmsResp()->cntx.reschedulingForward;

  if(!sms.Invalidate(__FILE__,__LINE__))
  {
    smsc_log_warn(smsLog, "Invalidate of %lld failed",t.msgId);
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::INVOPTPARAMVAL);
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return ERROR_STATE;
  }

  if(sms.getState()==EXPIRED_STATE)
  {
    smsc_log_warn(smsLog, "FWD: sms in expired state msgId=%lld",t.msgId);
    sms.setLastResult(Status::EXPIRED);
    smsc->getScheduler()->InvalidSms(t.msgId);
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
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return UNDELIVERABLE_STATE;
  }

  if(sms.getState()!=ENROUTE_STATE)
  {
    smsc_log_warn(smsLog, "FWD: sms msgId=%lld is not in enroute (%d)",t.msgId,sms.getState());
    smsc->getScheduler()->InvalidSms(t.msgId);
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return sms.getState();
  }
  time_t now=time(NULL);
  if( sms.getNextTime()>now && sms.getAttemptsCount()==0 && (!isReschedulingForward || sms.getLastResult()==0) )
  {
    debug2(smsLog, "FWD: nextTime>now (%d>%d)",sms.getNextTime(),now);
    SmeIndex idx=smsc->getSmeIndex(sms.dstSmeId);
    smsc->getScheduler()->AddScheduledSms(t.msgId,sms,idx);
    sms.setLastResult(Status::SYSERR);
    try{
      smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return sms.getState();
  }


  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    try{
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      smsc_log_warn(smsLog,"FORWARD: Failed to change state of USSD request to undeliverable. msgId=%lld",t.msgId);
    }
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::EXPIRED);
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return UNDELIVERABLE_STATE;
  }

  //
  //sms in forward mode forwarded. this mean, that sms with set_dpf was sent,
  //but request timed out. need to send alert notification with status unavialable.
  //

  if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2)
  {
    SmeProxy* proxy=smsc->getSmeProxy(sms.srcSmeId);
    debug2(smsLog,"Sending AlertNotification to '%s'",sms.srcSmeId);
    if(proxy!=0)
    {
      try{
        proxy->putCommand(
          SmscCommand::makeAlertNotificationCommand
          (
            proxy->getNextSequenceNumber(),
            sms.getOriginatingAddress(),
            sms.getDestinationAddress(),
            2
          )
        );
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to put Alert Notification Command:%s",e.what());
      }
    }else
    {
      warn2(smsLog,"Sme %s requested dpf, but not connected at the moment",sms.srcSmeId);
    }
    try{
      store->changeSmsStateToDeleted(t.msgId);
    }catch(...)
    {
      smsc_log_warn(smsLog,"FORWARD: Failed to change state of fwd/dgm sms to undeliverable. msgId=%lld",t.msgId);
    }
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::EXPIRED);
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return UNDELIVERABLE_STATE;
  }

  if(sms.getLastTime()>sms.getValidTime())
  {
    sms.setLastResult(Status::EXPIRED);
    smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
    smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
    try{
      smsc->getScheduler()->InvalidSms(t.msgId);
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      __warning__("FWD: failed to change state to expired");
    }
    info2(smsLog, "FWD: %lld expired lastTry(%u)>valid(%u)",t.msgId,sms.getLastTime(),sms.getValidTime());
    sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return EXPIRED_STATE;
  }

  if(!t.command->get_fwdChargeSmsResp()->result)
  {
    char bufsrc[64],bufdst[64];
    sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
    sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
    smsc_log_warn(smsLog, "FWD: msgId=%lld denied by inman(%s->%s)",t.msgId,bufsrc,bufdst);
    sms.setLastResult(Status::DENIEDBYINMAN);
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
    try{
      sendNotifyReport(sms,t.msgId,"destination unavailable");
    }catch(...)
    {
      __warning__("FORWARD: failed to send intermediate notification");
    }
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      changeSmsStateToEnroute(sms,t.msgId,d,Status::DENIEDBYINMAN,rescheduleSms(sms));

    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }

  ////
  //
  // Traffic Control
  //


  /*
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
  */

  //
  // End of traffic Control
  //
  ////


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
  if(allowDivert &&
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
      sms.setIntProperty(Tag::SMSC_DSTCODEPAGE,newdc&(~smsc::profiler::ProfileCharsetOptions::UssdIn7Bit));
      debug2(smsLog,"FWD: set dstdc to %x for diverted msg (was %x)",newdc,olddc);
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

  //for interfaceVersion==0x50
  if(!sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
  {
    if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&DF_UNCOND))
    {
      dst=sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str();
      Address newdst;
      if(smsc->AliasToAddress(dst,newdst))dst=newdst;
    }
  }

  smsc::router::RouteInfo ri;
  bool has_route = false;
  try{
    has_route=smsc->routeSms
                    (
                      sms.getOriginatingAddress(),
                      dst,
                      dest_proxy_index,
                      dest_proxy,
                      &ri,
                      smsc->getSmeIndex(sms.getSourceSmeId())
                    );
  }catch(std::exception& e)
  {
    warn2(smsLog,"Routing %s->%s failed:%s",sms.getOriginatingAddress().toString().c_str(),
     dst.toString().c_str(),e.what());
  }
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
      __warning__("FORWARD: failed to send intermediate notification");
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
    try{
      smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return ERROR_STATE;
  }

  if(sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID) &&
     sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
  {
    smsc_log_debug(smsLog,"FWD: diverted receipt for %s",sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
    dest_proxy_index=smsc->getSmeIndex(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
    dest_proxy=smsc->getSmeProxy(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
  }

  if(!dest_proxy)
  {
    char bufsrc[64],bufdst[64];
    sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
    sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
    smsc_log_warn(smsLog, "FWD: msgId=%lld sme is not connected(%s->%s(%s))",t.msgId,bufsrc,bufdst,ri.smeSystemId.c_str());
    sms.setLastResult(Status::SMENOTCONNECTED);
#ifdef SNMP
    incSnmpCounterForError(Status::SMENOTCONNECTED,ri.smeSystemId.c_str());
#endif
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
    try{
      smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
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
    try{
      sms.setLastResult(Status::SYSERR);
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
      return UNKNOWN_STATE;
    }
    debug2(smsLog,"%lld after repartition: %s",t.msgId,pres==psSingle?"single":"multiple");
  }

  SmeInfo dstSmeInfo=smsc->getSmeInfo(dest_proxy_index);

  if(dstSmeInfo.interfaceVersion==0x50 && sms.hasStrProperty(Tag::SMSC_RECIPIENTADDRESS))
  {
    sms.setOriginatingAddress(sms.getStrProperty(Tag::SMSC_RECIPIENTADDRESS).c_str());
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
    task.inDlgId=inDlgId;
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
    try{
      smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
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
#ifdef SNMP
    incSnmpCounterForError(Status::SMENOTCONNECTED,ri.smeSystemId.c_str());
#endif
      smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
      changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    try{
      smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
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
        sms.getIntProperty(Tag::SMSC_HIDE)==HideOption::hoEnabled &&
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
      if(sms.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3 && !ri.transit)
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
    smsc_log_debug(smsLog,"FWD: msgId=%lld, esm_class=%x",t.msgId,sms.getIntProperty(Tag::SMPP_ESM_CLASS));
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
#ifdef SNMP
    if(Status::isErrorPermanent(errstatus))
    {
      SnmpCounter::getInstance().incCounter(SnmpCounter::cntFailed,sms.getDestinationSmeId());
    }else
    {
      incSnmpCounterForError(errstatus,sms.getDestinationSmeId());
    }
#endif
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
    Task tsk;
    smsc->tasks.findAndRemoveTask(uniqueId,dialogId2,&tsk);
    try{
      smsc->ReportDelivery(inDlgId,sms,Status::isErrorPermanent(errstatus),Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
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

  //smsc->allowCommandProcessing(t.command);

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
  }
  else
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
    char buf[MAX_ADDRESS_VALUE_LENGTH*4+12]="";
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

  bool wasBillReport=false;

  if(sms.billingRecord && sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnDelivery)
  {
    bool final=
      GET_STATUS_TYPE(t.command->get_resp()->get_status())==CMD_OK ||
      GET_STATUS_TYPE(t.command->get_resp()->get_status())==CMD_ERR_PERM;
    bool lastPart=false;
    bool multiPart=sms.hasBinProperty(Tag::SMSC_CONCATINFO);
    if(final && GET_STATUS_TYPE(t.command->get_resp()->get_status())!=CMD_ERR_PERM)
    {
      if(multiPart)
      {
        unsigned len;
        ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
        if(sms.getConcatSeqNum()==ci->num-1)
        {
          lastPart=true;
        }
      }
    }
    smsc_log_debug(smsLog,"multiPart=%s, lastPart=%s, final=%s",multiPart?"true":"false",lastPart?"true":"false",final?"true":"false");
    if(!multiPart || lastPart || !final)
    {
      int savedLastResult=sms.getLastResult();
      try{
        sms.setLastResult(GET_STATUS_CODE(t.command->get_resp()->get_status()));
        if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && !t.command->get_resp()->get_diverted())
        {
          std::string savedDivert=sms.getStrProperty(Tag::SMSC_DIVERTED_TO);
          sms.getMessageBody().dropProperty(Tag::SMSC_DIVERTED_TO);
          smsc->ReportDelivery(t.command->get_resp()->get_inDlgId(),sms,final,Smsc::chargeOnDelivery);
          sms.setStrProperty(Tag::SMSC_DIVERTED_TO,savedDivert.c_str());
        }else
        {
          smsc->ReportDelivery(t.command->get_resp()->get_inDlgId(),sms,final,Smsc::chargeOnDelivery);
          wasBillReport=true;
        }
      }catch(std::exception& e)
      {
        smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
      }
      sms.setLastResult(savedLastResult);
    }
  }

  int sttype=GET_STATUS_TYPE(t.command->get_resp()->get_status());
  info2(smsLog, "DLVRSP: msgId=%lld;class=%s;st=%d;oa=%s;%s;srcprx=%s;dstprx=%s;route=%s;%s%s",t.msgId,
      sttype==CMD_OK?"OK":
      sttype==CMD_ERR_RESCHEDULENOW?"RESCHEDULEDNOW":
      sttype==CMD_ERR_TEMP?"TEMP ERROR":"PERM ERROR",
      GET_STATUS_CODE(t.command->get_resp()->get_status()),
      sms.getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms.getDestinationAddress(),"dda",sms.getDealiasedDestinationAddress()).c_str(),
      sms.getSourceSmeId(),
      sms.getDestinationSmeId(),
      sms.getRouteId(),
      t.command->get_resp()->get_diverted()?";diverted_to=":"",
      t.command->get_resp()->get_diverted()?sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str():""
    );

#ifdef SNMP
  if(GET_STATUS_TYPE(t.command->get_resp()->get_status())==CMD_OK)
  {
    SnmpCounter::getInstance().incCounter(SnmpCounter::cntDelivered,sms.getDestinationSmeId());
  }else
  {
    switch(GET_STATUS_TYPE(t.command->get_resp()->get_status()))
    {
      case CMD_ERR_RESCHEDULENOW:
      case CMD_ERR_TEMP:
      {
        incSnmpCounterForError(GET_STATUS_CODE(t.command->get_resp()->get_status()),sms.getDestinationSmeId());
      }break;
      default:
      {
        SnmpCounter::getInstance().incCounter(SnmpCounter::cntFailed,sms.getDestinationSmeId());
      }break;
    }
  }
#endif



  if(GET_STATUS_TYPE(t.command->get_resp()->get_status())!=CMD_OK)
  {
    time_t now=time(NULL);
    if((sms.getValidTime()<=now && sms.getLastResult()!=0) || //expired or
       RescheduleCalculator::calcNextTryTime(now,sms.getLastResult(),sms.getAttemptsCount())==-1) //max attempts count reached
    {
      sms.setLastResult(Status::EXPIRED);
      smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
      smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
      try{
        smsc->getScheduler()->InvalidSms(t.msgId);
        store->changeSmsStateToExpired(t.msgId);
      }catch(...)
      {
        __warning__("DLVRSP: failed to change state to expired");
      }
      info2(smsLog, "DLVRSP: %lld expired (valid:%u - now:%u), attempts=%d",t.msgId,sms.getValidTime(),now,sms.getAttemptsCount());
      sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");
      if(!wasBillReport)
      {
        try{
          smsc->ReportDelivery(t.command->get_resp()->get_inDlgId(),sms,true,Smsc::chargeOnDelivery);
        }catch(std::exception& e)
        {
          smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
        }
      }
      return EXPIRED_STATE;
    }

    sms.setLastResult(GET_STATUS_CODE(t.command->get_resp()->get_status()));
    if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2 &&
       sms.getIntProperty(Tag::SMPP_SET_DPF))//forward/transaction mode
    {
      if(GET_STATUS_CODE(t.command->get_resp()->get_status())==1179 || GET_STATUS_CODE(t.command->get_resp()->get_status())==1044)
      {
        try{
          sms.lastTime=time(NULL);
          sms.setNextTime(rescheduleSms(sms));
          bool saveNeedArchivate=sms.needArchivate;
          sms.needArchivate=false;
          sms.billingRecord=0;
          store->createSms(sms,t.msgId,smsc::store::CREATE_NEW_NO_CLEAR);
          int dest_proxy_index=smsc->getSmeIndex(sms.getDestinationSmeId());
          try{
            smsc->getScheduler()->AddScheduledSms(t.msgId,sms,dest_proxy_index);
          }catch(std::exception& e)
          {
            store->changeSmsStateToDeleted(t.msgId);
          }
          sms.needArchivate=saveNeedArchivate;
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to create dpf sms:%s",e.what());
        }
      }else
      {
        sms.setIntProperty(Tag::SMPP_SET_DPF,0);
      }
    }

    switch(GET_STATUS_TYPE(t.command->get_resp()->get_status()))
    {
      case CMD_ERR_RESCHEDULENOW:
      {
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
        try{
          __trace__("DELIVERYRESP: change state to enroute");
          time_t rt;
          if(t.command->get_resp()->get_delay()!=-1)
          {
            rt=time(NULL)+t.command->get_resp()->get_delay();
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

            if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && !t.command->get_resp()->get_diverted())
            {
              sms.getMessageBody().dropProperty(Tag::SMSC_DIVERTED_TO);
              store->replaceSms(t.msgId,sms);
            }

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

        sendFailureReport(sms,t.msgId,UNDELIVERABLE_STATE,"permanent error");
        smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);

        smsc->getScheduler()->InvalidSms(t.msgId);

#ifdef SMSEXTRA
        if(sms.billingRecord && sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnSubmit)
        {
          smsc->FullReportDelivery(t.msgId,sms);
        }
#endif

        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
        }
        return UNDELIVERABLE_STATE;
      }
    }
  }

  /*
  if(sms.getIntProperty(Tag::SMPP_SET_DPF)==1 && sms.getAttemptsCount()>0)
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
      __warning2__("DLVRESP: Failed to send AlertNotification:%s",e.what());
    }
  }
  */

  sms.setLastResult(Status::OK);

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
    if(smsc->getSmartMultipartForward() && (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&3)==2)
    {
      info2(smsLog,"enabling smartMultipartForward  for msgId=%lld",t.msgId);
      try{
        sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&~3);
        store->createSms(sms,t.msgId,smsc::store::CREATE_NEW_NO_CLEAR);
      }catch(std::exception& e)
      {
        __warning2__("DELIVERYRESP: failed to create sms for SmartMultipartForward:'%s'",e.what());
        finalizeSms(t.msgId,sms);
        return UNDELIVERABLE_STATE;
      }
      dgortr=false;
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
          __warning__("DELIVERYRESP: failed to put transaction response command");
        }
      }
    }

    unsigned int len;
    ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
    info2(smsLog, "DLVRSP: sms has concatinfo, csn=%d/%d;msgId=%lld",sms.getConcatSeqNum(),ci->num,t.msgId);
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

      // for interfaceVersion==0x50
      if(!sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
      {
        if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&(DF_COND|DF_UNCOND)))
        {
          dst=sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str();
        }
      }

      smsc::router::RouteInfo ri;
      bool has_route = false;
      try{
        has_route=smsc->routeSms(sms.getOriginatingAddress(),dst,dest_proxy_index,dest_proxy,&ri,smsc->getSmeIndex(sms.getSourceSmeId()));
      }catch(std::exception& e)
      {
        warn2(smsLog,"Routing %s->%s failed:%s",sms.getOriginatingAddress().toString().c_str(),
          dst.toString().c_str(),e.what());
      }
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

      if(sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID) &&
         sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
      {
        smsc_log_debug(smsLog,"CONCAT: diverted receipt for %s",sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
        dest_proxy_index=smsc->getSmeIndex(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
        dest_proxy=smsc->getSmeProxy(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
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
#ifdef SNMP
        incSnmpCounterForError(Status::SMENOTCONNECTED,ri.smeSystemId.c_str());
#endif
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
      tg.uniqueId=uniqueId;

      try{
        dialogId2 = dest_proxy->getNextSequenceNumber();
        tg.dialogId=dialogId2;
        __trace2__("CONCAT: seq number:%d",dialogId2);
        //Task task((uint32_t)dest_proxy_index,dialogId2);

        Task task(uniqueId,dialogId2,dgortr?new SMS(sms):0);
        task.messageId=t.msgId;
        task.inDlgId=t.command->get_resp()->get_inDlgId();
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
           sms.getIntProperty(Tag::SMSC_HIDE)==HideOption::hoEnabled &&
           ri.hide &&
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
          uint32_t clen;
          ConcatInfo *nci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&clen);
          if(sms.getConcatSeqNum()<nci->num-1)
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

        smsc_log_debug(smsLog,"CONCAT: msgId=%lld, esm_class=%x",t.msgId,sms.getIntProperty(Tag::SMPP_ESM_CLASS));
        SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
        dest_proxy->putCommand(delivery);
        tg.active=false;
      }
      catch(ExtractPartFailedException& e)
      {
        errstatus=Status::INVPARLEN;
        smsc_log_error(smsLog,"CONCAT: failed to extract sms part for %lld",t.msgId);
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

        if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && !t.command->get_resp()->get_diverted())
        {
          sms.getMessageBody().dropProperty(Tag::SMSC_DIVERTED_TO);
          store->replaceSms(t.msgId,sms);
        }

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

#ifdef SMSEXTRA
    if(sms.billingRecord && sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnSubmit)
    {
      smsc->FullReportDelivery(t.msgId,sms);
    }
#endif


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
      SmeInfo si=smsc->getSmeInfo(sms.getSourceSmeId());
      if(si.interfaceVersion==0x50)
      {
        rpt.setStrProperty(Tag::SMSC_DIVERTED_TO,sms.getSourceSmeId());
      }
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
      fd.date=time(NULL)+common::TimeZoneManager::getInstance().getTimeZone(rpt.getDestinationAddress())+timezone;
      fd.msgId=msgid;
      fd.err="";
      fd.lastResult=0;
      fd.lastResultGsm=0;
      fd.msc=d.msc;
      smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
      fd.locale=profile.locale.c_str();
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
  const AlertData& ad=t.command->get_alertData();
  SMS sms;
  if(ad.sms)
  {
    sms=*ad.sms;
  }
  else
  {
    try{
      store->retriveSms((SMSId)t.msgId,sms);
    }catch(...)
    {
      smsc_log_warn(smsLog, "ALERT: Failed to retrieve sms:%lld",t.msgId);
      smsc->getScheduler()->InvalidSms(t.msgId);
      return UNKNOWN_STATE;
    }
  }

  time_t now=time(NULL);
  if((sms.getValidTime()<=now && sms.getLastResult()!=0) || //expired or
     RescheduleCalculator::calcNextTryTime(now,sms.getLastResult(),sms.getAttemptsCount())==-1) //max attempts count reached
  {
    sms.setLastResult(Status::EXPIRED);
    smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
    smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
    try{
      smsc->getScheduler()->InvalidSms(t.msgId);
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      __warning__("ALERT: failed to change state to expired");
    }
    info2(smsLog, "ALERT: %lld expired (valid:%u - now:%u), attempts=%d",t.msgId,sms.getValidTime(),now,sms.getAttemptsCount());
    sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");
    try{
      smsc->ReportDelivery(ad.inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return EXPIRED_STATE;
  }

  sms.setLastResult(Status::DELIVERYTIMEDOUT);

  if(sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnDelivery)
  {
    try{
      smsc->ReportDelivery(ad.inDlgId,sms,
       (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1 ||
       (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2,
       Smsc::chargeOnDelivery
       );
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery failed:'%s'",e.what());
    }
  }

  if(!sms.Invalidate(__FILE__,__LINE__))
  {
    smsc_log_warn(smsLog, "Invalidate of %lld failed",t.msgId);
    smsc->getScheduler()->InvalidSms(t.msgId);
    return ERROR_STATE;
  }

  char bufsrc[64],bufdst[64];
  sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
  sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
  info2(smsLog, "ALERT: delivery timed out(%s->%s), msgId=%lld",bufsrc,bufdst,t.msgId);
  smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
#ifdef SNMP
  incSnmpCounterForError(Status::DELIVERYTIMEDOUT,sms.getDestinationSmeId());
#endif

  if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1 ||
     (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2)
  {
#ifdef SNMP
    SnmpCounter::getInstance().incCounter(SnmpCounter::cntRejected,sms.getDestinationSmeId());
#endif
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
  bool updateSmsSchedule=t.command->get_replaceSm().scheduleDeliveryTime!=0;
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
    if(updateSmsSchedule)
    {
      smsc->getScheduler()->UpdateSmsSchedule(t.msgId,sms);
    }
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
    try{
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
    }catch(...)
    {
      __warning__("QUERY: failed to send query response");
    }
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
  try{
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
  }catch(...)
  {
    __warning__("QUERY: failed to send query response");
  }
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
      try{
        t.command.getProxy()->putCommand
        (
          SmscCommand::makeCancelSmResp
          (
            t.command->get_dialogId(),
            Status::CANCELFAIL
          )
        );
      }catch(...)
      {
        __warning__("CANCEL: failed to send cancel response");
      }
    }
    __warning2__("CANCEL: failed to cancel sms with msgId=%lld:%s",t.msgId,e.what());
    return t.state;
  }
  int code=Status::OK;
  try{
    smsc->getScheduler()->CancelSms(t.msgId,sms.getDealiasedDestinationAddress());

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
    smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
    smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
  }catch(std::exception& e)
  {
    code=Status::CANCELFAIL;
    __warning2__("CANCEL: failed to cancel sms with msgId=%lld:%s",t.msgId,e.what());
    return t.state;
  }
  if(!t.command->get_cancelSm().internall)
  {
    try{
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeCancelSmResp
        (
          t.command->get_dialogId(),
          code
        )
      );
    }catch(std::exception& e)
    {
      __warning2__("CANCEL: failed to send cancel response: %s",e.what());
    }
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

  SmeInfo si=smsc->getSmeInfo(sms.getSourceSmeId());
  if(si.interfaceVersion==0x50)
  {
    rpt.setStrProperty(Tag::SMSC_DIVERTED_TO,sms.getSourceSmeId());
  }

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
  fd.date=sms.getSubmitTime()+common::TimeZoneManager::getInstance().getTimeZone(rpt.getDestinationAddress())+timezone;
  fd.msgId=msgid;
  fd.err=reason;
  fd.setLastResult(sms.lastResult);
  smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
  fd.locale=profile.locale.c_str();
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

    SmeInfo si=smsc->getSmeInfo(sms.getSourceSmeId());
    if(si.interfaceVersion==0x50)
    {
      rpt.setStrProperty(Tag::SMSC_DIVERTED_TO,sms.getSourceSmeId());
    }

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
    fd.date=sms.getSubmitTime()+common::TimeZoneManager::getInstance().getTimeZone(rpt.getDestinationAddress())+timezone;
    fd.addr=addr;
    fd.msgId=msgid;
    fd.err=reason;
    fd.setLastResult(sms.lastResult);
    smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
    fd.locale=profile.locale.c_str();
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
  sms.setNextTime(nextTryTime);
  if(failureCause==Status::RESCHEDULEDNOW)
  {
    smsc->getScheduler()->AddScheduledSms(id,sms,smsc->getSmeIndex(sms.dstSmeId));
  }else
  {
    time_t ntt=smsc->getScheduler()->RescheduleSms(id,sms,smsc->getSmeIndex(sms.dstSmeId));
    if(ntt)nextTryTime=ntt;
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
    bool has_route=false;
    sms.setSourceSmeId(smscSmeId.c_str());
    try{
      has_route=smsc->routeSms(sms.getOriginatingAddress(),sms.getDealiasedDestinationAddress(),dest_proxy_index,dest_proxy,&ri,smsc->getSmeIndex(smscSmeId));
    }catch(std::exception& e)
    {
      warn2(smsLog,"Routing %s->%s failed:%s",sms.getOriginatingAddress().toString().c_str(),
        sms.getDealiasedDestinationAddress().toString().c_str(),
        e.what());
    }


    if(has_route)
    {
      sms.setRouteId(ri.routeId.c_str());
      int prio=sms.getPriority()+ri.priority;
      if(prio>SmeProxyPriorityMax)prio=SmeProxyPriorityMax;
      sms.setPriority(prio);

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
      smsc->getScheduler()->AddFirstTimeForward(msgId,sms);
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
  if(sms->hasIntProperty(Tag::SMPP_SET_DPF))
  {
    resp->get_resp()->haveDpf=true;
    resp->get_resp()->dpfResult=sms->getIntProperty(Tag::SMPP_SET_DPF);
  }
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
