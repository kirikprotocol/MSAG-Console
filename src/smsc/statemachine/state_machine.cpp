#include <exception>

#include "logger/Logger.h"
#include "util/debug.h"
#include "util/udh.hpp"
#include "util/recoder/recode_dll.h"
#include "util/smstext.h"
#include "util/regexp/RegExp.hpp"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/FixedLengthString.hpp"
#include "inman/storage/cdrutil.hpp"
#include "smsc/closedgroups/ClosedGroupsInterface.hpp"
#include "smsc/status.h"
#include "smsc/resourcemanager/ResourceManager.hpp"
#include "smsc/smsc.hpp"
#include "smsc/common/rescheduler.hpp"
#include "smsc/profiler/profiler.hpp"
#include "smsc/common/TimeZoneMan.hpp"
#ifdef SMSEXTRA
#include "smsc/extra/Extra.hpp"
#include "smsc/extra/ExtraBits.hpp"
#endif

#include "constants.hpp"
#include "util.hpp"
#include "state_machine.hpp"


#ident "@(#)$Id$"



namespace smsc{
namespace statemachine{

using namespace smsc::smeman;
using namespace smsc::sms;
using namespace StateTypeValue;
using namespace smsc::smpp;
using namespace util;
using namespace smsc::common;
using namespace smsc::resourcemanager;
using namespace smsc::profiler;
using std::exception;
using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;
using smsc::core::synchronization::Mutex;

Hash<std::list<std::string> > StateMachine::directiveAliases;

#ifdef SNMP
void StateMachine::incSnmpCounterForError(int code,const char* sme)
{
  using namespace smsc::snmp;
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

TaskGuard::~TaskGuard()
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
               smsc::Smsc *app):
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
#ifdef SMSEXTRA
  createCopyOnNickUsage=false;
#endif
  smeAckRx.Compile("/id:\\w+\\s+sub:\\d+\\s+dlvrd:\\d+\\s+submit date:(\\d{10}) done date:(\\d{10})\\s+stat:(\\w+)\\s+err:(\\d+)\\s+Text:(.*)/");
  __throw_if_fail__(smeAckRx.LastError()==regexp::errNone,RegExpCompilationException);
}


void StateMachine::formatDeliver(const FormatData& fd,std::string& out)
{
  ReceiptGetAdapter ga;
  ContextEnvironment ce;
  ce.exportStr("dest",fd.addr);
  ce.exportStr("ddest",fd.ddest);
  ce.exportDat("date",fd.date);
  ce.exportDat("submitDate",fd.submitDate);
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
  ce.exportDat("submitDate",fd.submitDate);
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
  ce.exportDat("submitDate",fd.submitDate);
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


FixedLengthString<64> StateMachine::AddrPair(const char* s1,const Address& a1,const char* s2,const Address& a2)
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

bool StateMachine::checkSourceAddress(const std::string& pattern,const Address& src)
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
  static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("sms.dir");
  const char *body="";
  TmpBuf<char,256> tmpBuf(0);
  unsigned int len=0;
  int dc=sms.getIntProperty(Tag::SMPP_DATA_CODING);
  if(dc==DataCoding::BINARY)
  {
    smsc_log_info(log,"DIRECT: disabled for binary datacoding");
    return;
  }
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
  {

    unsigned char tmp[4]={
      0,
    };
    tmp[0]=len>0?body[0]:0;
    tmp[1]=len>1?body[1]:0;
    tmp[2]=len>2?body[2]:0;
    tmp[3]=len>3?body[3]:0;
    smsc_log_info(log,"DIRECT: oa=%s, da=%s, Body: %02x %02x %02x %02x, len=%d, dc=%d",
                  sms.getOriginatingAddress().toString().c_str(),
                  sms.getDestinationAddress().toString().c_str(),
                  tmp[0],tmp[1],tmp[2],tmp[3],len,sms.getIntProperty(Tag::SMPP_DATA_CODING));
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
      uint16_t tmp;
      memcpy(&tmp,body,2);
      hasDirectives=(tmp=='#');
      break;
    }
  }
  if(!hasDirectives)
  {
    smsc_log_info(log,"Directive not found");
    return;
  }
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
      smsc_log_info(log,"DIRECT: ack found");
      sms.setDeliveryReport(REPORT_ACK);
      lastDirectiveSymbol=m[0].end;
      i=m[0].end;
    }else
    if(dreNoAck.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      smsc_log_info(log,"DIRECT: noack found");
      sms.setDeliveryReport(REPORT_NOACK);
      lastDirectiveSymbol=m[0].end;
      i=m[0].end;
    }else
    if(dreHide.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      smsc_log_info(log,"DIRECT: hide");
      if(srcprof.hideModifiable)
        sms.setIntProperty(Tag::SMSC_HIDE,1);
      else
      {
        smsc_log_info(log,"DIRECT: error, hide is not modifiable");
      }
      lastDirectiveSymbol=m[0].end;
      i=m[0].end;
    }else
    if(dreUnhide.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      smsc_log_info(log,"DIRECT: unhide");
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
      smsc_log_info(log,"DIRECT: notrans");
      sms.setIntProperty(Tag::SMSC_TRANSLIT,0);
      lastDirectiveSymbol=m[0].end;
      i=m[0].end;
    }else
    if(dreFlash.MatchEx(buf,buf+i,buf+len,m,n=10))
    {
      smsc_log_info(log,"DIRECT: flash");
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
      smsc_log_info(log,"DIRECT: template=%s",tmplname.c_str());
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
        smsc_log_info(log,"DIRECT: found template param %s=%s",name.c_str(),value.c_str());
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
  if(lastDirectiveSymbol==0)
  {
    smsc_log_info(log,"DIRECT: lastDirectiveSymbol==0");
    return;
  }
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
  size_t newlen=tailLen+udhLen;
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
  smsc_log_info(log,"DIRECT: newlen=%ld",newlen);
  //#def N# #ack# #noack# #template=name# {name}="value"
  if(newlen>255)
  {
    sms.getMessageBody().dropProperty(Tag::SMPP_SHORT_MESSAGE);
    sms.getMessageBody().dropProperty(Tag::SMSC_RAW_SHORTMESSAGE);
    sms.setIntProperty(Tag::SMPP_SM_LENGTH,0);
    sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,newBody,(unsigned)newlen);
  }else
  {
    if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,newBody,(unsigned)newlen);
    }else
    {
      sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,newBody,(unsigned)newlen);
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,(unsigned)newlen);
    }
  }
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

  bool dgortr=(sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1 || (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;

  time_t now=time(NULL);
  if( !dgortr && ((sms.getValidTime()<=now) || //expired or
     RescheduleCalculator::calcNextTryTime(now,sms.getLastResult(),sms.getAttemptsCount())==-1)) //max attempts count reached
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
       dgortr,
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

  if(dgortr)
  {
#ifdef SNMP
    smsc::snmp::SnmpCounter::getInstance().incCounter(smsc::snmp::SnmpCounter::cntRejected,sms.getDestinationSmeId());
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
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
  #ifdef SNMP
    incSnmpCounterForError(Status::DELIVERYTIMEDOUT,sms.getDestinationSmeId());
  #endif
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
      throw Exception("addr not match");
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
  rpt.setOriginatingDescriptor((uint8_t)strlen(msc),msc,(uint8_t)strlen(imsi),imsi,1);
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
  rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,(unsigned)time(NULL));

  SmeInfo si=smsc->getSmeInfo(sms.getSourceSmeId());
  if(si.hasFlag(sfForceReceiptToSme))
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
  fd.submitDate=fd.date;
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
    rpt.setOriginatingDescriptor((uint8_t)strlen(msc),msc,(uint8_t)strlen(imsi),imsi,1);
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
    rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,(unsigned)time(NULL));

    SmeInfo si=smsc->getSmeInfo(sms.getSourceSmeId());
    if(si.hasFlag(sfForceReceiptToSme))
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
    fd.submitDate=fd.date;
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
  if(nextTryTime==-1)nextTryTime=1;
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
    if(sms.lastResult==0)
    {
      smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
    }else
    {
      if(sms.hasIntProperty(Tag::SMPP_SET_DPF))
      {
        smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
      }else
      {
        smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
      }
    }
    //smsc->registerStatisticalEvent(sms.lastResult==0?StatEvents::etSubmitOk:StatEvents::etSubmitErr,&sms);
    //smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
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
      if(sms.hasIntProperty(Tag::SMPP_SET_DPF))
      {
        resp->get_resp()->haveDpf=true;
        resp->get_resp()->dpfResult=sms.getIntProperty(Tag::SMPP_SET_DPF);
      }
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

#ifdef SMSEXTRA
bool StateMachine::ExtraProcessing(SbmContext& c)
{
  bool toSmsx=c.dest_proxy && !strcmp(c.dest_proxy->getSystemId(),"smsx");
  bool isMultipart=c.sms->hasBinProperty(Tag::SMSC_CONCATINFO) || c.sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT);

  if((((c.fromMap || c.fromDistrList) && c.toMap) || (c.fromMap && toSmsx))&& !isMultipart)
  {
    ExtraInfo::ServiceInfo xsi;
    int extrabit=ExtraInfo::getInstance().checkExtraService(*c.sms,xsi);
    if(extrabit)
    {
      info2(smsLog,"EXTRA: service with bit=%x detected for abonent %s",xsi.serviceBit,c.sms->getOriginatingAddress().toString().c_str());
    }
    if((/*(srcprof.subscription&EXTRA_NICK) ||*/ extrabit && xsi.serviceBit==EXTRA_NICK))
    {
      if(!c.srcprof.nick.length())
      {
        submitResp(c.t,c.sms,Status::INVOPTPARAMVAL);
        warn2(smsLog,"EXTRA: abonent '%s' sent nick command without nick set",c.sms->getOriginatingAddress().toString().c_str());
        return false;
      }
      c.sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,c.sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_NICK);
      //c.sms->setIntProperty(Tag::SMSC_HIDE,HideOption::hoEnabled);
      info2(smsLog,"EXTRA: smsnick for abonent %s",c.sms->getOriginatingAddress().toString().c_str());
      unsigned txtlen;
      if(c.sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
      {
        c.sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&txtlen);
      }else
      {
        txtlen=c.sms->getIntProperty(Tag::SMPP_SM_LENGTH);
      }
      TmpBuf<char,2048> txt(txtlen*3);
      txtlen=getSmsText(c.sms,txt.get(),txtlen*3);
      int i=0;
      while(i<txtlen && !isspace(txt[i]))i++;
      while(i<txtlen && isspace(txt[i]))i++;
      int j=i;
      while(i<txtlen && !isspace(txt[i]))i++;
      std::string addrstr;
      addrstr.assign(txt.get()+j,i-j);
      Address daaddr;
      try{
        daaddr=addrstr.c_str();
      }catch(...)
      {
        submitResp(c.t,c.sms,Status::INVDSTADR);
        warn2(smsLog,"EXTRA: abonent '%s' sent nick command with invalid address '%s'",c.sms->getOriginatingAddress().toString().c_str(),addrstr.c_str());
        return false;
      }
      Address ddadst;
      if(smsc->AliasToAddress(daaddr,ddadst))
      {
        daaddr=ddadst;
      }
      c.sms->setDealiasedDestinationAddress(daaddr);
      c.dst=daaddr;
      c.sms->setBillingRecord(BILLING_NORMAL);
    }


    if((c.srcprof.subscription&EXTRA_FLASH) || (extrabit && xsi.serviceBit==EXTRA_FLASH))
    {
      c.sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,c.sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_FLASH);
      c.sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,1);
      info2(smsLog,"EXTRA: smsflash for abonent %s",c.sms->getOriginatingAddress().toString().c_str());
    }
    if(extrabit && xsi.diverted)
    {
      c.sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,xsi.serviceBit|(c.sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)&(EXTRA_FLASH|EXTRA_NICK)));
      //c.sms->setIntProperty(Tag::SMSC_HIDE,HideOption::hoDisabled);
      //c.sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,0);
      c.sms->setIntProperty(Tag::SMPP_ESM_CLASS,(c.sms->getIntProperty(Tag::SMPP_ESM_CLASS)&~3)|2);
      c.dst=xsi.divertAddr;
      c.sms->setDestinationAddress(c.sms->getDealiasedDestinationAddress());
      info2(smsLog,"EXTRA: divert for abonent %s to %s",c.sms->getOriginatingAddress().toString().c_str(),xsi.divertAddr.toString().c_str());
      c.noDestChange=true;
      c.createSms=scsDoNotCreate;
      c.noPartitionSms=true;
      try{
        c.has_route=smsc->routeSms(c.sms->getOriginatingAddress(),
                                c.dst,
                                c.dest_proxy_index,c.dest_proxy,&c.ri,c.src_proxy->getSmeIndex());
      }catch(std::exception& e)
      {
        warn2(smsLog,"Routing %s->%s failed:%s",c.sms->getOriginatingAddress().toString().c_str(),
          c.dst.toString().c_str(),e.what());
      }
      if(!c.has_route)
      {
        submitResp(c.t,c.sms,Status::NOROUTE);
        warn2(smsLog, "SBM: no route(extra divert) Id=%lld;oa=%s;%s;srcprx=%s",
          c.t.msgId,
          c.sms->getOriginatingAddress().toString().c_str(),
          AddrPair("da",c.sms->getDestinationAddress(),"dda",c.dst).c_str(),
          c.src_proxy->getSystemId()
        );
        return false;
      }
    }
  }
  return true;
}
#endif

bool StateMachine::processMerge(SbmContext& c)
{
  bool firstPiece=c.sms->getIntProperty(Tag::SMSC_MERGE_CONCAT)==1;
  unsigned char *body;
  unsigned int len;
  if(c.sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    body=(unsigned char*)c.sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }else
  {
    body=(unsigned char*)c.sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  }
  uint16_t mr;
  uint8_t idx,num;
  bool havemoreudh;
  smsc::util::findConcatInfo(body,mr,idx,num,havemoreudh);

  int dc=c.sms->getIntProperty(Tag::SMPP_DATA_CODING);
  //bool needrecoding=
  //    (dc==DataCoding::UCS2 && (profile.codepage&ProfileCharsetOptions::Ucs2)!=ProfileCharsetOptions::Ucs2) ||
  //    (dc==DataCoding::LATIN1 && (profile.codepage&ProfileCharsetOptions::Latin1)!=ProfileCharsetOptions::Latin1);

  if(firstPiece) //first piece
  {
    info2(smsLog, "merging sms Id=%lld, first part arrived(%u/%u),mr=%d,dc=%d",c.t.msgId,idx,num,(int)mr,dc);
    c.sms->setIntProperty(Tag::SMPP_ESM_CLASS,c.sms->getIntProperty(Tag::SMPP_ESM_CLASS)&~0x40);
    string tmp;
    if(!c.isForwardTo)
    {
      tmp.assign((const char*)body,len);
    }else
    {
      unsigned lenMo;
      const char *bodyMo=c.sms->getBinProperty(Tag::SMSC_MO_PDU,&lenMo);
      if(lenMo>512)
      {
        warn2(smsLog,"MO LEN=%d",lenMo);
      }
      tmp.assign(bodyMo,lenMo);
      c.sms->setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::BINARY);
      dc=DataCoding::BINARY;
    }
    c.sms->setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),(unsigned)tmp.length());
    c.sms->getMessageBody().dropProperty(Tag::SMPP_SHORT_MESSAGE);
    c.sms->getMessageBody().dropProperty(Tag::SMSC_RAW_SHORTMESSAGE);
    c.sms->setIntProperty(Tag::SMPP_SM_LENGTH,0);
    c.sms->setIntProperty(Tag::SMSC_ORIGINALPARTSNUM,num);

    if(c.sms->getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM))
    {
      c.sms->getMessageBody().dropIntProperty(Tag::SMPP_SAR_MSG_REF_NUM);
      c.sms->getMessageBody().dropIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
      c.sms->getMessageBody().dropIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
    }

    char cibuf[256*2+1];
    ConcatInfo *ci=(ConcatInfo *)cibuf;
    ci->num=1;
    ci->setOff(0,0);
    c.sms->setBinProperty(Tag::SMSC_CONCATINFO,(char*)ci,1+2*num);
    c.generateDeliver=false;

    char dc_list[256];
    memset(dc_list,0,num);
    __trace2__("dc_list[%d]=%d",idx-1,dc);
    dc_list[idx-1]=dc;

    c.sms->setBinProperty(Tag::SMSC_DC_LIST,dc_list,num);


    if(c.sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
    {
      char buf[256];
      memset(buf,0,sizeof(buf));
      buf[idx-1]=c.sms->getMessageReference();
      c.sms->setBinProperty(Tag::SMSC_UMR_LIST,buf,num);
      buf[idx-1]=1;
      c.sms->setBinProperty(Tag::SMSC_UMR_LIST_MASK,buf,num);
    }

    char buf[64];
    sprintf(buf,"%lld",c.t.msgId);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           buf,
                           c.dialogId,
                           Status::OK,
                           c.sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      c.src_proxy->putCommand(resp);
    }catch(...)
    {
      warn2(smsLog, "SBM: failed to put response command SUBMIT_OK Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        c.t.msgId,c.dialogId,
        c.sms->getOriginatingAddress().toString().c_str(),
        c.sms->getDestinationAddress().toString().c_str(),
        c.src_proxy->getSystemId(),
        c.ri.smeSystemId.c_str()
      );
    }
    c.needToSendResp=false;
    c.createSms=scsCreate;
  }else
  {
    info2(smsLog, "merging sms Id=%lld, next part arrived(%u/%u), mr=%d,dc=%d",c.t.msgId,idx,num,(int)mr,dc);
    SMS newsms;
    try{
      store->retriveSms(c.t.msgId,newsms);
    }catch(...)
    {
      warn2(smsLog, "sms with id %lld not found or store error",c.t.msgId);
      submitResp(c.t,c.sms,Status::SYSERR);
      c.rvstate=ERROR_STATE;
      return false;
    }
    if(!newsms.hasIntProperty(Tag::SMSC_MERGE_CONCAT))
    {
      warn2(smsLog, "smsId=%lld:one more part of concatenated message received, but all parts are collected.",c.t.msgId);
      submitResp(c.t,c.sms,Status::SUBMITFAIL);
      c.rvstate=ERROR_STATE;
      return false;
    }
    if(c.sms->hasIntProperty(Tag::SMPP_REGISTRED_DELIVERY) && !newsms.hasIntProperty(Tag::SMPP_REGISTRED_DELIVERY))
    {
      newsms.setIntProperty(Tag::SMPP_REGISTRED_DELIVERY,c.sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY));
    }
    unsigned int newlen;
    unsigned char *newbody;
    unsigned int cilen;
    ConcatInfo *ci=(ConcatInfo*)newsms.getBinProperty(Tag::SMSC_CONCATINFO,&cilen);
    newbody=(unsigned char*)newsms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&newlen);
    if(!c.isForwardTo)
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

          c.sms->setLastResult(Status::DUPLICATECONCATPART);
          smsc->registerStatisticalEvent(StatEvents::etSubmitErr,c.sms);
          char buf[64];
          sprintf(buf,"%lld",c.t.msgId);
          SmscCommand resp = SmscCommand::makeSubmitSmResp
                               (
                                 buf,
                                 c.t.command->get_dialogId(),
                                 Status::OK,
                                 c.sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                               );
          try{
            c.t.command.getProxy()->putCommand(resp);
          }catch(...)
          {
            __warning__("SUBMIT_SM: failed to put response command");
          }


          warn2(smsLog, "Duplicate or invalid concatenated message part for id=%lld(idx:%d-%d,num:%d-%d,mr:%d-%d)",c.t.msgId,idx0,idx,num0,num,mr0,mr);
          c.rvstate=ERROR_STATE;
          return false;
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
          warn2(smsLog, "smsId=%lld: different number of parts detected %d!=%d.",c.t.msgId,dclen,num);
          submitResp(c.t,c.sms,Status::INVOPTPARAMVAL);
          c.rvstate=ERROR_STATE;
          return false;
        }
      }
      body=(unsigned char*)c.sms->getBinProperty(Tag::SMSC_MO_PDU,&len);
      dc=DataCoding::BINARY;
    }

    if(newsms.hasBinProperty(Tag::SMSC_UMR_LIST))
    {
      unsigned ulen;
      unsigned char* umrList=(unsigned char*)newsms.getBinProperty(Tag::SMSC_UMR_LIST,&ulen);
      if(idx<=ulen)
      {
        umrList[idx-1]=c.sms->getMessageReference();
        newsms.setBinProperty(Tag::SMSC_UMR_LIST,(const char*)umrList,ulen);

        if(newsms.hasBinProperty(Tag::SMSC_UMR_LIST_MASK))
        {
          unsigned mlen;
          char* mask=(char*)newsms.getBinProperty(Tag::SMSC_UMR_LIST_MASK,&mlen);
          if(idx<=mlen)
          {
            mask[idx-1]=1;
            c.sms->setBinProperty(Tag::SMSC_UMR_LIST_MASK,mask,mlen);
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

      debug2(smsLog,"all parts received, send kill cache item:msgId=%lld;oa=%s;da=%s;mr=%d",c.t.msgId,c.sms->getOriginatingAddress().toString().c_str(),c.sms->getDestinationAddress().toString().c_str(),(int)mr);
      smsc->submitMrKill(c.sms->getOriginatingAddress(),c.sms->getDestinationAddress(),mr);

      vector<int> order;
      bool rightOrder=true;
      bool totalMoreUdh=false;
      bool differentDc=false;
      bool haveBinDc=c.isForwardTo?true:c.sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::BINARY;

      if(!c.isForwardTo)
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
                int partlen=j==num-1?(int)tmp.length()-ci->getOff(j):ci->getOff(j+1)-ci->getOff(j);
                newci[i-1]=(uint16_t)newtmp.length();
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
          int partlen=i==num?(int)tmp.length()-ci->getOff(i-1):ci->getOff(i)-ci->getOff(i-1);
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
          processDirectives(newsms,c.profile,c.srcprof);
        }catch(...)
        {
          warn2(smsLog, "Failed to process directives for sms with id=%lld",c.t.msgId);
          submitResp(c.t,&newsms,Status::SUBMITFAIL);
          c.rvstate=ERROR_STATE;
          return false;
        }
        /*
        if(c.ri.smeSystemId=="MAP_PROXY")
        {
          if(!newsms.hasIntProperty(Tag::SMSC_DSTCODEPAGE))
          {
            newsms.setIntProperty(Tag::SMSC_DSTCODEPAGE,c.profile.codepage);
          }
          if(!newsms.getIntProperty(Tag::SMSC_TRANSLIT) || !smsCanBeTransliterated(&newsms))
          {
            if(newsms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
            {
              newsms.setIntProperty(Tag::SMSC_DSTCODEPAGE,DataCoding::UCS2);
            }
          }
          int pres=partitionSms(&newsms);
          if(pres==psMultiple)
          {
            uint8_t msgref=smsc->getNextMR(c.dst);
            c.sms->setConcatMsgRef(msgref);
            c.sms->setConcatSeqNum(0);
          }
          c.noPartitionSms=true;
        }
        */
      }else
      {
        newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),(int)tmp.length());
        if(!c.isForwardTo)
        {
          newsms.setIntProperty(Tag::SMPP_ESM_CLASS,newsms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x40);
        }
      }
    }else
    {
      newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.c_str(),(int)tmp.length());
    }
    try{
      store->replaceSms(c.t.msgId,newsms);
    }catch(...)
    {
      warn2(smsLog, "Failed to replace sms with id=%lld",c.t.msgId);
      submitResp(c.t,&newsms,Status::SUBMITFAIL);
      c.rvstate=ERROR_STATE;
      return false;
    }
    if(!allParts)
    {
      info2(smsLog, "merging sms %lld, not all parts are here, waiting",c.t.msgId);
      char buf[64];
      sprintf(buf,"%lld",c.t.msgId);
      SmscCommand resp = SmscCommand::makeSubmitSmResp
                           (
                             buf,
                             c.dialogId,
                             Status::OK,
                             c.sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                           );
      try{
        c.src_proxy->putCommand(resp);
      }catch(...)
      {
        warn2(smsLog, "SBM: failed to put response command SUBMIT_OK Id=%lld;seq=%d;oa=%s;%s;srcprx=%s;dstprx=%s",
          c.t.msgId,c.dialogId,
          c.sms->getOriginatingAddress().toString().c_str(),
          AddrPair("da",c.sms->getDestinationAddress(),"dda",c.dst).c_str(),
          c.src_proxy->getSystemId(),
          c.ri.smeSystemId.c_str()
        );
      }
      c.rvstate=ENROUTE_STATE;
      return false;
    }

    *c.sms=newsms;

    c.createSms=scsReplace;
#ifdef SMSEXTRA
    if(!ExtraProcessing(c))
    {
      try{
        store->changeSmsStateToDeleted(c.t.msgId);
      }catch(...)
      {
        __warning2__("Failed to delete sms with msgId=%lld denied by EXTRA from store",c.t.msgId);
      }
      c.rvstate=ERROR_STATE;
      return false;
    }
#endif

  }
  return true;
}


}//system
}//smsc
