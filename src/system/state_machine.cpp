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
#include "inman/storage/cdrutil.hpp"
#ifdef SMSEXTRA
#include "Extra.hpp"
#include "ExtraBits.hpp"
#include "router/route_types.h"
#endif


#ident "@(#)$Id$"


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
  smsLog=smsc::logger::Logger::getInstance("sms.trace");
  perfLog=smsc::logger::Logger::getInstance("sm.perf");
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
  ce.exportDat("submitTime",fd.submitDate);
  ce.exportStr("msgId",fd.msgId);
  ce.exportInt("lastResult",fd.lastResult);
  ce.exportInt("lastResultGsm",fd.lastResultGsm);
  ce.exportStr("msc",fd.msc);
  ce.exportDat("discharge",time(NULL));

  string key="receipt.";
  key+=fd.scheme;
  key+=".";
  key+="deliveredFormat";

  smsc_log_debug(smsLog,"RECEIPT: get formatter for key=%s",key.c_str());

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
    smsc_log_warn(smsLog,"FORMATTER: %s",e.what());
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
  ce.exportDat("submitTime",fd.submitDate);
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

  smsc_log_debug(smsLog,"RECEIPT: get formatter for key=%s",key.c_str());


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
    smsc_log_warn(smsLog,"FORMATTER: %s",e.what());
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
  ce.exportDat("submitTime",fd.submitDate);
  ce.exportStr("msgId",fd.msgId);
  ce.exportInt("lastResult",fd.lastResult);
  ce.exportInt("lastResultGsm",fd.lastResultGsm);
  ce.exportStr("msc",fd.msc);
  ce.exportDat("discharge",time(NULL));

  string key="receipt.";
  key+=fd.scheme;
  key+=".";
  key+="notifyFormat";

  smsc_log_debug(smsLog,"RECEIPT: get formatter for key=%s",key.c_str());


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
    smsc_log_warn(smsLog,"FORMATTER: %s",e.what());
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
      smsc_log_debug(smsLog,"id=%lld, cmd=%d",t.msgId,t.command->cmdid);
      const char* op="unknown";
      hrtime_t opStart;
      if(perfLog->isInfoEnabled())
      {
        opStart=gethrtime();
      }
      switch(t.command->cmdid)
      {
        case SUBMIT:st=submit(t);op="submit";break;
        case INSMSCHARGERESPONSE:st=submitChargeResp(t);op="insmschargeresp";break;
        case DELIVERY_RESP:st=deliveryResp(t);op="deliveryresp";break;
        case FORWARD:st=forward(t);op="forward";break;
        case INFWDSMSCHARGERESPONSE:st=forwardChargeResp(t);op="infwdchargereso";break;
        case ALERT:st=alert(t);op="alert";break;
        case REPLACE:st=replace(t);op="replace";break;
        case QUERY:st=query(t);op="query";break;
        case CANCEL:st=cancel(t);op="cancel";break;
        default:
          smsc_log_warn(smsLog,"UNKNOWN COMMAND:%d",t.command->cmdid);
          st=ERROR_STATE;
          break;
      }
      if(perfLog->isInfoEnabled())
      {
        opStart=(gethrtime()-opStart)/1000000;
        if(opStart>50)
        {
          smsc_log_info(perfLog,"command %s processing takes %lldms",op,opStart);
        }
      }
      smsc_log_debug(smsLog,"change state for Id=%lld to %d",t.msgId,st);
      eq.changeState(t.msgId,st);
    }catch(exception& e)
    {
      smsc_log_warn(smsLog,"StateMachine::exception %s",e.what());
    }
    catch(...)
    {
      smsc_log_warn(smsLog,"StateMachine::unknown exception");
    }
  }
  smsc_log_info(smsLog,"exit state machine");
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
        smsc_log_info(smsLog,"DIRECT: error, hide is not modifiable");
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
      smsc_log_debug(smsLog,"DIRECT: %*s, t=%d",m[0].end-m[0].start,buf+m[0].start,t);
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
        smsc_log_warn(smsLog,"tail of template '%s' arguments wasn't parsed:'%s'",tmplname.c_str(),buf+j);
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
      smsc_log_warn(smsLog,"template %s not found for locale %s",tmplname.c_str(),p.locale.c_str());
      newtext.assign(buf+tmplstart,tmpllen);
    }else
    {
      try{
        f->format(newtext,ga,ce);
      }catch(exception& e)
      {
        smsc_log_warn(smsLog,"failed to format template %s for locale %s: %s",tmplname.c_str(),p.locale.c_str(),e.what());
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
      smsc_log_debug(smsLog,"DIRECT: newtext=%s, newtext.length=%d, nt.get()=%p",newtext.c_str(),newtext.length(),nt.get());
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

static time_t parseReceiptTime(const std::string& txt,SMatch* m,int idx)
{
  int YY,MM,DD,hh,mm;
  sscanf(txt.c_str()+m[idx].start,"%02d%02d%02d%02d%02d",&YY,&MM,&DD,&hh,&mm);
  YY+=2000;
  tm t={0,};
  t.tm_year=YY-1900;
  t.tm_mon=MM-1;
  t.tm_mday=DD;
  t.tm_hour=hh;
  t.tm_min=mm;
  return mktime(&t);
}


StateType StateMachine::submit(Tuple& t)
{
  __require__(t.state==UNKNOWN_STATE || t.state==ENROUTE_STATE || t.state==ERROR_STATE);

  SbmContext c(t);

  c.src_proxy=t.command.getProxy();

  __require__(c.src_proxy!=NULL);

  SMS* sms = t.command->get_sms();
  c.sms=sms;

  sms->setSourceSmeId(t.command->get_sourceId());

  if(!sms->Invalidate(__FILE__,__LINE__))
  {
    warn2(smsLog, "Invalidate of %lld failed\n",t.msgId);
    submitResp(t,sms,Status::SUBMITFAIL);
    return ERROR_STATE;
  }

  c.dialogId =  t.command->get_dialogId();
  sms->dialogId=c.dialogId;

  debug2(smsLog, "SBM: Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
    t.msgId,c.dialogId,
    sms->getOriginatingAddress().toString().c_str(),
    sms->getDestinationAddress().toString().c_str(),
    c.src_proxy->getSystemId()
  );

  if(t.state==ERROR_STATE)
  {
    warn1(smsLog, "SMS in error state\n");
    submitResp(t,sms,Status::SUBMITFAIL);
    return ERROR_STATE;
  }

  // check scheduler limit
  if(smsc->checkSchedulerHardLimit())
  {
    submitResp(t,sms,Status::SCHEDULERLIMIT);
    info2(smsLog,"SBM: sms denied by hard scheduler limit oa=%s;da=%s;srcprx=%s",
          sms->getOriginatingAddress().toString().c_str(),
          sms->getDestinationAddress().toString().c_str(),
          c.src_proxy->getSystemId());
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
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
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
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
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
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG) && sms->hasBinProperty(Tag::SMSC_MERGE_CONCAT))
  {
    submitResp(t,sms,Status::INVOPTPARAMVAL);
    smsc->submitMrKill(sms->getOriginatingAddress(),sms->getDestinationAddress(),sms->getConcatMsgRef());
    warn2(smsLog, "SBM: 'replace if present' flag not supported in concatenated message Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM) &&
     (
       sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)<1 ||
       sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)>sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS)
     )
    )
  {
    submitResp(t,sms,Status::INVOPTPARAMVAL);
    warn2(smsLog, "SBM: invalid SMPP_SAR_SEGMENT_SEQNUM %d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM),
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
  {
    unsigned len;
    const unsigned char* msg;
    if(sms->hasBinProperty(Tag::SMSC_RAW_PAYLOAD))
    {
      msg=(const unsigned char*)sms->getBinProperty(Tag::SMSC_RAW_PAYLOAD,&len);
    }
    else
    {
      msg=(const unsigned char*)sms->getBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,&len);
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
        t.msgId,c.dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        c.src_proxy->getSystemId()
      );
      return ERROR_STATE;
    }
  }


  if(sms->getValidTime()==-1)
  {
    submitResp(t,sms,Status::INVEXPIRY);
    warn2(smsLog, "SBM: invalid valid time Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(c.src_proxy->getSourceAddressRange().length() &&
     !checkSourceAddress(c.src_proxy->getSourceAddressRange(),sms->getOriginatingAddress()))
  {
    submitResp(t,sms,Status::INVSRCADR);
    warn2(smsLog, "SBM: invalid source address for range '%s' Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      c.src_proxy->getSourceAddressRange().c_str(),
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
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
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
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

  // route sms
  //SmeProxy* c.dest_proxy = 0;
  smsc_log_debug(smsLog,"AliasToAddress: %s",sms->getDestinationAddress().toString().c_str());
  if(smsc->AliasToAddress(sms->getDestinationAddress(),c.dst))
  {
    smsc_log_debug(smsLog,"ALIAS:%s->%s",sms->getDestinationAddress().toString().c_str(),c.dst.toString().c_str());
  }
  else
  {
    c.dst=sms->getDestinationAddress();
  }
  sms->setDealiasedDestinationAddress(c.dst);
  c.profile=smsc->getProfiler()->lookup(sms->getOriginatingAddress());
  Profile& profile=c.profile;

  if(profile.closedGroupId!=0 && !smsc::closedgroups::ClosedGroupsInterface::getInstance()->Check(profile.closedGroupId,sms->getDealiasedDestinationAddress()))
  {
    info2(smsLog,"SBM: Id=%lld, denied by closed group(%d:'%s') check",
      t.msgId,profile.closedGroupId,
      smsc::closedgroups::ClosedGroupsInterface::getInstance()->GetClosedGroupName(profile.closedGroupId)
    );
    submitResp(t,sms,Status::DENIEDBYCLOSEDGROUP);
    return ERROR_STATE;
  }

  smsc::profiler::Profile orgprofile=profile;

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
      info2(smsLog,"Id=%lld: oa subst: %s->%s",t.msgId,sms->getOriginatingAddress().toString().c_str(),addr.toString().c_str());
      sms->setOriginatingAddress(addr);
    }
  }

  if(!sms->hasIntProperty(Tag::SMSC_TRANSLIT))
  {
    sms->setIntProperty(Tag::SMSC_TRANSLIT,profile.translit);
    debug2(smsLog,"Id=%lld, set translit to %d",t.msgId,sms->getIntProperty(Tag::SMSC_TRANSLIT));
  }

  if(sms->getIntProperty(Tag::SMPP_SET_DPF) && (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)!=2)
  {
    sms->setIntProperty(Tag::SMPP_SET_DPF,0);
  };


  c.srcprof=profile;

  int profileMatchType;
  std::string profileMatchAddress;
  profile=smsc->getProfiler()->lookupEx(c.dst,profileMatchType,profileMatchAddress);

  if(profileMatchType==ProfilerMatchType::mtExact)
  {
    debug2(smsLog,"Id=%lld exact profile match, set translit to 1",t.msgId);
    sms->setIntProperty(Tag::SMSC_TRANSLIT,1);
  }

  if(!smsCanBeTransliterated(sms))
  {
    debug2(smsLog,"Id=%lld cannot be transliterated, set translit to 0",t.msgId);
    sms->setIntProperty(Tag::SMSC_TRANSLIT,0);
  }

  if(smsc->isNoDivert(sms->getSourceSmeId()))
  {
    profile.divert="";
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
    debug2(smsLog, "divert for %s found",c.dst.toString().c_str());
    Address divDst;
    try{
      divDst=Address(profile.divert.c_str());
    }catch(...)
    {
      warn2(smsLog, "INVALID DIVERT FOR %s - ADDRESS:%s",c.dst.toString().c_str(),profile.divert.c_str());
      goto divert_failed;
    }
    Address tmp;
    if(smsc->AliasToAddress(divDst,tmp))
    {
      debug2(smsLog, "Divert address dealiased:%s->%s",divDst.toString().c_str(),tmp.toString().c_str());
      divDst=tmp;
    }
    smsc::router::RouteResult rr2;
    try{
      if(smsc->routeSms(c.src_proxy->getSmeIndex(),sms->getOriginatingAddress(),divDst,rr2))
      {
        if(rr2.info.smeSystemId!="MAP_PROXY")
        {
          warn2(smsLog,"attempt to divert to non-map address(sysId=%s):%s->%s",
            rr2.info.smeSystemId.c_str(),
            sms->getOriginatingAddress().toString().c_str(),divDst.toString().c_str());
          goto divert_failed;
        }
      }
    }catch(std::exception& e)
    {
      warn2(smsLog,"routing failed during divert check:%s",e.what());
      goto divert_failed;
    }
    if(rr2.info.trafMode==smsc::router::tmUssdOnly)
    {
      smsc_log_info(smsLog,"attempt to divert sms to ussd only route(%s->%s)",
          sms->getOriginatingAddress().toString().c_str(),divDst.toString().c_str());
      goto divert_failed;
    }
    sms->setStrProperty(Tag::SMSC_DIVERTED_TO,divDst.toString().c_str());
    if(divertFlags&DF_UNCOND)
    {
      diverted=true;
      c.dst=divDst;
    }

    Profile p=smsc->getProfiler()->lookup(divDst);

    divertFlags|=p.udhconcat?DF_UDHCONCAT:0;
    int ddc=p.codepage;
    if(!(profile.codepage&DataCoding::UCS2) && (ddc&DataCoding::UCS2))
    {
      ddc=profile.codepage;
      smsc_log_debug(smsLog,"divert - downgrade dstdc to %d",ddc);
    }

    divertFlags|=(ddc)<<DF_DCSHIFT;

    if(divertFlags&DF_UNCOND)profile=p;

    sms->setIntProperty(Tag::SMSC_DIVERTFLAGS,divertFlags);

  }
  divert_failed:;

  sms->setIntProperty(Tag::SMSC_ORIGINALPARTSNUM,1);



  ////
  //
  //  Routing here
  //

  c.has_route = false;

  try{
    c.has_route=smsc->routeSms(c.src_proxy->getSmeIndex(),sms->getOriginatingAddress(),c.dst,c.rr);
  }catch(std::exception& e)
  {
    warn2(smsLog,"Routing %s->%s failed:%s",sms->getOriginatingAddress().toString().c_str(),
      c.dst.toString().c_str(),e.what());
  }

  if ( !c.has_route )
  {
    submitResp(t,sms,Status::NOROUTE);
    warn2(smsLog, "SBM: no route Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",c.dst).c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(c.rr.info.trafMode==smsc::router::tmSmsOnly && sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    submitResp(t,sms,Status::PROHIBITED);
    warn2(smsLog, "SBM: sms only route Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",c.dst).c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(c.rr.info.trafMode==smsc::router::tmUssdOnly && !sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    submitResp(t,sms,Status::PROHIBITED);
    warn2(smsLog, "SBM: ussd only route Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",c.dst).c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }


  if(!c.rr.info.backupSme.empty())
  {
    sms->setStrProperty(Tag::SMSC_BACKUP_SME,c.rr.info.backupSme.c_str());
  }

  c.generateDeliver=true; // do not generate in case of merge-concat
  c.createSms=scsCreate;

  c.needToSendResp=true;

  c.noPartitionSms=false; // do not call partitionSms if true!

  SmeInfo dstSmeInfo=smsc->getSmeInfo(c.rr.destSmeIdx);

  c.fromMap=c.src_proxy && !strcmp(c.src_proxy->getSystemId(),"MAP_PROXY");
  c.toMap=dstSmeInfo.systemId=="MAP_PROXY";
  c.fromDistrList=c.src_proxy && !strcmp(c.src_proxy->getSystemId(),"DSTRLST");


  if(c.toMap && ((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3c)==0x08 ||
      (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3c)==0x10))
  {
    std::string txt;
    getSmsText(sms,txt,CONV_ENCODING_CP1251);
    SMatch m[10];
    int n=10;
    if(smeAckRx.Match(txt.c_str(),txt.c_str()+txt.length(),m,n))
    {
      time_t sbmTime=parseReceiptTime(txt,m,1);
      time_t dlvTime=parseReceiptTime(txt,m,2);
      std::string st=txt.substr(m[3].start,m[3].end-m[3].start);
      int err;
      sscanf(txt.c_str()+m[4].start,"%d",&err);
      FormatData fd;
      char addr[32];
      sms->getOriginatingAddress().getText(addr,sizeof(addr));
      fd.addr=addr;
      fd.ddest=addr;
      fd.submitDate=sbmTime;
      fd.date=dlvTime;
      fd.setLastResult(err);
      fd.locale=profile.locale.c_str();
      fd.msc="";
      fd.msgId="";
      fd.scheme=dstSmeInfo.receiptSchemeName.c_str();
      std::string out;
      fd.err="";
      smsc_log_debug(smsLog,"esme delivery ack detected in Id=%lld. sbmTime=%lu dlvTime=%lu, st=%s",t.msgId,sbmTime,dlvTime,st.c_str());
      if(st=="DELIVRD")
      {
        formatDeliver(fd,out);
      }else if(st=="ACCEPTD")
      {
        formatNotify(fd,out);
      }else
      {
        fd.err=txt.c_str()+m[5].start;
        formatFailed(fd,out);
      }
      fillSms(sms,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,0);
      if(sms->hasIntProperty(Tag::SMSC_ORIGINAL_DC))
      {
        sms->getMessageBody().dropIntProperty(Tag::SMSC_ORIGINAL_DC);
        sms->getMessageBody().dropIntProperty(Tag::SMSC_FORCE_DC);
      }
    }
  }

  sms->setArchivationRequested(c.rr.info.archived);

#ifdef SMSEXTRA
  /*
  if(sms->getIntProperty(Tag::SMSC_MERGE_CONCAT))
  {
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
    if(idx!=1)
    {
      firstPart=false;
    }
  }
  */
  if(!ExtraProcessing(c))
  {
    return ERROR_STATE;
  }
#endif

  bool dropMergeConcat=false;

  if(c.rr.info.transit)
  {
    dropMergeConcat=true;
  }

  c.isForwardTo = false;
  if( c.rr.info.forwardTo.length() > 0 && c.toMap )
  {
    sms->setStrProperty( Tag::SMSC_FORWARD_MO_TO, c.rr.info.forwardTo.c_str());

    // force forward(transaction) mode
    sms->setIntProperty( Tag::SMPP_ESM_CLASS, sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x02 );
    c.isForwardTo = true;

    dropMergeConcat=true;
    sms->getMessageBody().dropProperty(Tag::SMSC_ORGPARTS_INFO);
  }

  if(dropMergeConcat)
  {
    if(sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
    {
      dropMergeConcat=true;
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

      smsc->submitMrKill(sms->getOriginatingAddress(),sms->getDestinationAddress(),mr);
      sms->getMessageBody().dropIntProperty(Tag::SMSC_MERGE_CONCAT);
    }
  }


  if(!c.rr.info.transit)
  {
    if(!extactConcatInfoToSar(*sms))
    {
      warn2(smsLog,"extactConcatInfoToSar failed. Id=%lld;oa=%s;da=%s",
                            t.msgId,
                            sms->getOriginatingAddress().toString().c_str(),
                            sms->getDestinationAddress().toString().c_str());
      submitResp(t,sms,Status::INVOPTPARAMVAL);
      return ERROR_STATE;
    }


    if(!extractPortsFromUdh(*sms))
    {
      warn2(smsLog,"extractPortsFromUdh failed. Id=%lld;oa=%s;da=%s",
                            t.msgId,
                            sms->getOriginatingAddress().toString().c_str(),
                            sms->getDestinationAddress().toString().c_str());
      submitResp(t,sms,Status::INVOPTPARAMVAL);
      return ERROR_STATE;
    }


    if(!convertSarToUdh(*sms))
    {
      warn2(smsLog,"convertSarToUdh failed. Id=%lld;oa=%s;da=%s",
                            t.msgId,
                            sms->getOriginatingAddress().toString().c_str(),
                            sms->getDestinationAddress().toString().c_str());
      submitResp(t,sms,Status::INVOPTPARAMVAL);
      return ERROR_STATE;
    };
  }

  sms->setRouteId(c.rr.info.routeId.c_str());
  if(c.rr.info.suppressDeliveryReports)sms->setIntProperty(Tag::SMSC_SUPPRESS_REPORTS,1);
  int prio=sms->getPriority()+c.rr.info.priority;
  if(prio>SmeProxyPriorityMax)prio=SmeProxyPriorityMax;
  sms->setPriority(prio);

  debug2(smsLog,"SBM: route %s->%s found:%s",
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",c.dst).c_str(),
      c.rr.info.routeId.c_str());


  sms->setIntProperty(Tag::SMSC_PROVIDERID,c.rr.info.providerId);
  sms->setIntProperty(Tag::SMSC_CATEGORYID,c.rr.info.categoryId);

  bool aclCheck=false;
  smsc::core::buffers::FixedLengthString<32> aclAddr;
  //std::string aclAddr;

  bool& fromMap=c.fromMap;
  bool& toMap=c.toMap;

  if(c.toMap)
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
    aclAddr=(const char*)buf;
    aclCheck=true;
  }

  if(c.rr.info.billing==smsc::sms::BILLING_ONSUBMIT || c.rr.info.billing==smsc::sms::BILLING_CDR)
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,Smsc::chargeOnSubmit);
  }else
  if((fromMap || c.fromDistrList) && toMap)//peer2peer
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,smsc->p2pChargePolicy);
  }else
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,smsc->otherChargePolicy);
  }

#ifdef SMSEXTRA
  if(c.rr.info.billing==smsc::sms::BILLING_MT)
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,Smsc::chargeOnSubmit);
    sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_INCHARGE);
  }
#endif
  if(c.rr.info.deliveryMode==SMSC_TRANSACTION_MSG_MODE || (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2)
  {
    // set charge on delivery to avoid charging of sms that could be undelivered
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,Smsc::chargeOnDelivery);
  }

  if(aclCheck && c.rr.info.aclId!=-1 && !smsc->getAclMgr()->isGranted(c.rr.info.aclId,aclAddr.c_str()))
  {
    submitResp(t,sms,Status::NOROUTE);
    char buf1[32];
    char buf2[32];
    sms->getOriginatingAddress().toString(buf1,sizeof(buf1));
    c.dst.toString(buf2,sizeof(buf2));
    warn2(smsLog, "SBM: acl access denied (aclId=%d) Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
      c.rr.info.aclId,
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",c.dst).c_str(),
      c.src_proxy->getSystemId()
    );
    if(c.createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }

  if(toMap)
  {
    if(c.rr.info.forceDelivery)
    {
      sms->setIntProperty(Tag::SMPP_PRIORITY,3);
    }
  }

  if(fromMap && ( sms->getOriginatingDescriptor().mscLength==0 ||
                  sms->getOriginatingDescriptor().imsiLength==0 ))
  {
    if(!c.rr.info.allowBlocked)
    {
      submitResp(t,sms,Status::CALLBARRED);
      char buf1[32];
      char buf2[32];
      sms->getOriginatingAddress().toString(buf1,sizeof(buf1));
      c.dst.toString(buf2,sizeof(buf2));
      warn2(smsLog, "SBM: call barred Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
        t.msgId,c.dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        AddrPair("da",sms->getDestinationAddress(),"dda",c.dst).c_str(),
        c.src_proxy->getSystemId()
      );
      if(c.createSms==scsReplace)
      {
        try{
          store->changeSmsStateToDeleted(t.msgId);
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
        }
      }
      return ERROR_STATE;
    }
  }

  smsc_log_debug(smsLog,"dst sme '%s', flags=%x",dstSmeInfo.systemId.c_str(),dstSmeInfo.flags);

  SmeInfo srcSmeInfo=smsc->getSmeInfo(c.src_proxy->getSystemId());

  if(srcSmeInfo.hasFlag(smsc::smeman::sfDefaultDcLatin1) &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)==smsc::smpp::DataCoding::SMSC7BIT)
  {
    sms->setIntProperty(Tag::SMPP_DATA_CODING,smsc::smpp::DataCoding::LATIN1);
  }


#ifdef SMSEXTRA

  if(srcSmeInfo.SME_N==EXTRA_GROUPS)
  {
    smsc_log_debug(smsLog,"added EXTRA_GROUPS to SMSC_EXTRAFLAGS from smeN");
    sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_GROUPS);
  }

  /*
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
  */
  /*
  NO LONGER NEEDED!
  Sponsored uses another scheme.

  if(fromMap && toMap && c.srcprof.sponsored>0)
  {
    info2(smsLog,"EXTRA: sponsored sms for abonent %s(cnt=%d)",sms->getOriginatingAddress().toString().c_str(),c.srcprof.sponsored);
    sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_SPONSORED);
    smsc->getProfiler()->decrementSponsoredCount(sms->getOriginatingAddress());
  }
  */
#endif

  if((dstSmeInfo.accessMask&c.srcprof.accessMaskOut)==0)
  {
    info2(smsLog,"SBM: Id=%lld, denied by access out mask (%s=%x,%s=%x",t.msgId,dstSmeInfo.systemId.c_str(),dstSmeInfo.accessMask,sms->getOriginatingAddress().toString().c_str(),c.srcprof.accessMaskOut);
    submitResp(t,sms,Status::DENIEDBYACCESSMASK);
    if(c.createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }

  if((c.src_proxy->getAccessMask()&profile.accessMaskIn)==0)
  {
    info2(smsLog,"SBM: Id=%lld, denied by access in mask(%s=%x,%s=%x",t.msgId,c.src_proxy->getSystemId(),c.src_proxy->getAccessMask(),sms->getDestinationAddress().toString().c_str(),profile.accessMaskIn);
    submitResp(t,sms,Status::DENIEDBYACCESSMASK);
    if(c.createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }

  if((c.srcprof.accessMaskOut&profile.accessMaskIn)==0)
  {
    info2(smsLog,"SBM: Id=%lld, denied by access masks(%s=%x,%s=%x",t.msgId,sms->getOriginatingAddress().toString().c_str(),c.srcprof.accessMaskOut,sms->getDestinationAddress().toString().c_str(),profile.accessMaskIn);
    submitResp(t,sms,Status::DENIEDBYACCESSMASK);
    if(c.createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }


  ////
  //
  //  Merging
  //

  if(sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
  {
    if(!processMerge(c))
    {
      if(c.rvstate!=ENROUTE_STATE || sms->getIntProperty(Tag::SMSC_CHARGINGPOLICY)!=Smsc::chargeOnSubmit)
      {
        if(c.rvstate==ENROUTE_STATE)
        {
          onSubmitOk(t.msgId,*sms);
        }
        return c.rvstate;
      }
      c.generateDeliver=false;
      c.createSms=scsDoNotCreate;
    }
  }

  //
  //  End of merging
  //
  ////


  if(dstSmeInfo.hasFlag(sfCarryOrgAbonentInfo))
  {
    sms->setStrProperty(Tag::SMSC_SUPPORTED_LOCALE,orgprofile.locale.c_str());
    sms->setIntProperty(Tag::SMSC_SUPPORTED_CODESET,orgprofile.codepage);
  }
  if(dstSmeInfo.hasFlag(sfCarryOrgDescriptor))
  {
    if(sms->getOriginatingDescriptor().imsiLength)
    {
      sms->setStrProperty(Tag::SMSC_IMSI_ADDRESS,sms->getOriginatingDescriptor().imsi);
    }
    if(sms->getOriginatingDescriptor().mscLength)
    {
      sms->setStrProperty(Tag::SMSC_MSC_ADDRESS,sms->getOriginatingDescriptor().msc);
    }
  }

#ifdef SMSEXTRA
  {
    if(c.toMap && srcSmeInfo.hasFlag(sfFillExtraDescriptor) && sms->hasStrProperty(Tag::SMSC_MSC_ADDRESS))
    {
      smsc_log_debug(smsLog,"Filling descriptor from smpp fields:%s/%s",sms->getStrProperty(Tag::SMSC_IMSI_ADDRESS).c_str(),sms->getStrProperty(Tag::SMSC_MSC_ADDRESS).c_str());
      Descriptor d;
      const SmsPropertyBuf& imsi(sms->getStrProperty(Tag::SMSC_IMSI_ADDRESS));
      d.setImsi((uint8_t)imsi.length(),imsi.c_str());
      const SmsPropertyBuf& msc(sms->getStrProperty(Tag::SMSC_MSC_ADDRESS));
      d.setMsc((uint8_t)msc.length(),msc.c_str());
      sms->setOriginatingDescriptor(d);
    }
  }
#endif


  sms->setDestinationSmeId(c.rr.info.smeSystemId.c_str());
  sms->setServiceId(c.rr.info.serviceId);


  smsc_log_debug(smsLog,"SUBMIT: archivation request for Id=%lld;seq=%d is %s",t.msgId,c.dialogId,c.rr.info.archived?"true":"false");

#ifdef SMSEXTRA
  if(sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)&EXTRA_FAKE)
  {
    sms->setBillingRecord(BILLING_NONE);
  }
  if(!(sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)&EXTRA_NICK))
  {
    sms->setBillingRecord(c.rr.info.billing);
  }
#else
  sms->setBillingRecord(c.rr.info.billing);
#endif


  sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage);

  if(c.rr.info.smeSystemId=="MAP_PROXY" && sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
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


  if((sms->getValidTime()==0 || sms->getValidTime()>now+maxValidTime) && !sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) &&
     (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)!=0x2)
  {
    sms->setValidTime(now+maxValidTime);
    debug2(smsLog,"maxValidTime=%d",maxValidTime);
  }

  debug2(smsLog,"Valid time for sms Id=%lld:%u",t.msgId,(unsigned int)sms->getValidTime());



  if(sms->getNextTime()>now+maxValidTime || sms->getNextTime()>sms->getValidTime())
  {
    submitResp(t,sms,Status::INVSCHED);
    warn2(smsLog, "SBM: invalid schedule time(%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      sms->getNextTime(),
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    if(c.createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }


  bool isDatagram=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1;
  bool isTransaction=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;


  if( !c.isForwardTo && !c.rr.info.transit)
  {

    sms->getMessageBody().dropProperty(Tag::SMSC_MO_PDU);

    ////
    //
    // Override delivery mode if specified in config and default mode in sms
    //

    if( c.rr.info.deliveryMode != smsc::sms::SMSC_DEFAULT_MSG_MODE)
    {
      if(sms->hasBinProperty(Tag::SMSC_CONCATINFO) && c.rr.info.deliveryMode!=smsc::sms::SMSC_STOREANDFORWARD_MSG_MODE)
      {
        smsc_log_warn(smsLog,"Attempt to send multipart message in forward mode with route '%s'",c.rr.info.routeId.c_str());
      }else
      {
        int esmcls = sms->getIntProperty( Tag::SMPP_ESM_CLASS );
        // following if removed at 25.09.2006 by request of customers
        //if( (esmcls&0x3) == smsc::sms::SMSC_DEFAULT_MSG_MODE )
        {
          // allow override
          sms->setIntProperty( Tag::SMPP_ESM_CLASS, (esmcls&~0x03)|(c.rr.info.deliveryMode&0x03) );
          isDatagram=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1;
          isTransaction=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;
        }
      }
    }

    if((isDatagram || isTransaction) && sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
    {
      try
      {
        smsc_log_debug(smsLog,"SBM:Delete fwd/dgm mode merged Id=%lld from store",t.msgId);
        c.createSms=scsDoNotCreate;
        store->changeSmsStateToDeleted(t.msgId);
      } catch(std::exception& e)
      {
        smsc_log_warn(smsLog,"SBM: merged miltipart->forward: failed to change Id=%lld to deleted",t.msgId);
      }
    }

    ////
    //
    //  Directives
    //

    try{
      if(!sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
      {
        processDirectives(*sms,profile,c.srcprof);
      }
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"Failed to process directives due to exception:%s",e.what());
      submitResp(t,sms,Status::SUBMITFAIL);
      if(c.createSms==scsReplace)
      {
        try{
          store->changeSmsStateToDeleted(t.msgId);
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
        }
      }
      return ERROR_STATE;
    }

    if(sms->getIntProperty(Tag::SMSC_TRANSLIT)==0)
    {
      if(sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
      {
        debug2(smsLog,"Id=%lld translit set to 0, patch dstcodepage",t.msgId);
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


    smsc_log_debug(smsLog,"SUBMIT_SM: after processDirectives - delrep=%d, sdt=%d",(int)sms->getDeliveryReport(),sms->getNextTime());

    if(c.rr.info.smeSystemId=="MAP_PROXY" && sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)==0x03)
    {
      sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,DataCoding::UCS2|DataCoding::LATIN1);
      sms->setIntProperty(Tag::SMSC_UDH_CONCAT,1);
    }



    if(c.rr.info.smeSystemId=="MAP_PROXY" &&
       !sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT)&&
       !sms->hasBinProperty(Tag::SMSC_CONCATINFO) &&
       !c.noPartitionSms
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
        t.msgId,c.dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        c.src_proxy->getSystemId(),
        c.rr.info.smeSystemId.c_str()
      );
      if(c.createSms==scsReplace)
      {
        try{
          store->changeSmsStateToDeleted(t.msgId);
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
        }
      }
      return ERROR_STATE;
    }

    if(pres==psErrorUdhi)
    {
      submitResp(t,sms,Status::SUBMITFAIL);
      warn2(smsLog, "SBM: udhi present in concatenated message!!! Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,c.dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        c.src_proxy->getSystemId(),
        c.rr.info.smeSystemId.c_str()
      );
      if(c.createSms==scsReplace)
      {
        try{
          store->changeSmsStateToDeleted(t.msgId);
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
        }
      }
      return ERROR_STATE;
    }

    if(pres==psErrorUssd)
    {
/*      submitResp(t,sms,Status::USSDMSGTOOLONG);
      warn2(smsLog, "SBM: ussd message too long!!! Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        c.src_proxy->getSystemId(),
        ri.smeSystemId.c_str()
      );
      return ERROR_STATE;*/
      pres = psSingle;
    }


    if(pres==psMultiple && !c.noPartitionSms)
    {
      uint8_t msgref=smsc->getNextMR(c.dst);
      sms->setConcatMsgRef(msgref);

      sms->setConcatSeqNum(0);
    }

    smsc_log_debug(smsLog,"SUBMIT_SM: SMPP_USSD_SERVICE_OP for Id=%lld:%d",t.msgId,sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP));

    //
    // End of checks. Ready to put sms to database
    //
    ////

  }

  INSmsChargeResponse::SubmitContext ctx;
  ctx.srcProxy=c.src_proxy;
  ctx.dstProxy=c.rr.destProxy;
  ctx.createSms=c.createSms;
  ctx.needToSendResp=c.needToSendResp;
  ctx.dialogId=c.dialogId;
  ctx.dest_proxy_index=c.rr.destSmeIdx;
  ctx.isForwardTo=c.isForwardTo;
  ctx.diverted=diverted;
  ctx.routeHide=c.rr.info.hide;
  ctx.dst=c.dst;
  ctx.transit=c.rr.info.transit;
  ctx.replyPath=c.rr.info.replyPath;
  ctx.priority=c.rr.info.priority;
  ctx.generateDeliver=c.generateDeliver;
#ifdef SMSEXTRA
  ctx.noDestChange=c.noDestChange;
#endif
  if(sms->billingRequired() &&
      (c.generateDeliver || sms->getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnSubmit))
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

  SmsCreationState createSms=(SmsCreationState)resp->cntx.createSms;
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
  sms->dialogId=dialogId;

  if(!resp->result)
  {
    submitResp(t,sms,Status::DENIEDBYINMAN);
    warn2(smsLog, "SBM: denied by inman Id=%lld;seq=%d;oa=%s;%s;srcprx=%s: '%s'",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",dst).c_str(),
      src_proxy->getSystemId(),
      resp->inmanError.c_str()
    );
    if(createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }

#ifdef SMSEXTRA
  if(sms->billingRecord==BILLING_MT && resp->contractType!=smsc::inman::cdr::CDRRecord::abtPrepaid)
  {
    sms->setIntProperty(Tag::SMPP_ESM_CLASS,(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x3))|0x2);
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,Smsc::chargeOnDelivery);
  }
#endif

  ////
  //
  // Store sms to database
  //

  __require__(!(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) &&
                sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE)   &&
                sms->getIntProperty(Tag::SMPP_SM_LENGTH)!=0));

  bool isDatagram=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1;
  bool isTransaction=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;


  smsc_log_debug(smsLog,"SUBMIT: msgId=%lld, isDg=%s, isTr=%s",t.msgId,isDatagram?"true":"false",isTransaction?"true":"false");
  if(!isDatagram && !isTransaction && createSms==scsCreate)
  {
    try{
      if(sms->getNextTime()<now)
      {
        sms->setNextTime(now);
      }
      if(!sms->Invalidate(__FILE__,__LINE__))
      {
         warn2(smsLog, "Invalidate of %lld failed",t.msgId);
         throw Exception("Invalid sms");
      }
      bool rip=sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG)!=0;

      SMSId replaceId=store->createSms(*sms,t.msgId,rip?smsc::store::SMPP_OVERWRITE_IF_PRESENT:smsc::store::CREATE_NEW);
      /*
      if(rip && replaceId!=t.msgId)
      {
        smsc->getScheduler()->CancelSms(replaceId,sms->getDealiasedDestinationAddress());
      }
      */

    }catch(...)
    {
      smsc_log_warn(smsLog,"failed to create sms with Id=%lld,oa=%s,da=%s",t.msgId,sms->getOriginatingAddress().toString().c_str(),sms->getDestinationAddress().toString().c_str());
      submitResp(t,sms,Status::SYSERR);
      smsc->ReportDelivery(resp->cntx.inDlgId,*sms,true,Smsc::chargeAlways);
      return ERROR_STATE;
    }
  }else if(createSms==scsReplace)
  {
    try
    {
      store->replaceSms(t.msgId,*sms);
    } catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"failed to create/replace sms with Id=%lld:%s",t.msgId,e.what());
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

  onSubmitOk(t.msgId,*sms);


  if(!isDatagram && !isTransaction && needToSendResp) // Store&Forward mode
  {
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

  if(!resp->cntx.generateDeliver)
  {
    smsc_log_debug(smsLog,"leave non merged Id=%lld in enroute state",t.msgId);
    return ENROUTE_STATE;
  }

  smsc_log_debug(smsLog,"Sms scheduled to %d, now %d",(int)sms->getNextTime(),(int)now);
  if(!isDatagram && !isTransaction && stime>now)
  {
    smsc->getScheduler()->AddScheduledSms(t.msgId,*sms,dest_proxy_index);
    sms->setLastResult(Status::DEFERREDDELIVERY);
    smsc->ReportDelivery(resp->cntx.inDlgId,*sms,false,Smsc::chargeOnDelivery);
    return ENROUTE_STATE;
  }

  //      Datagram  Transction .
  //      submit  submit response,   ...
  //         ,
  //  sms  setLastError()
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
      if(sms)
      {

        if(sms->lastResult!=Status::OK)
        {
          sm->onDeliveryFail(msgId,*sms);
        }

        bool sandf=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0 ||
            (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x3;
        bool isDg=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x1;

        if((!sandf && sms->lastResult!=Status::OK) || isDg)
        {
          char msgIdBuf[64];
          if(sms->lastResult!=Status::OK)
          {
            sprintf(msgIdBuf,"0");
          }else
          {
            sprintf(msgIdBuf,"%lld",msgId);
          }
          SmscCommand resp = SmscCommand::makeSubmitSmResp
              (
                  msgIdBuf,
                  sms->dialogId,
                  sms->lastResult,
                  sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
              );
          try{
            prx->putCommand(resp);
          }catch(...)
          {
            warn1(sm->smsLog,"SUBMIT: failed to put response command");
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
    info2(smsLog, "SBM: dest sme not connected Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      sms->getDestinationSmeId()
    );
    Descriptor d;
    sendNotifyReport(*sms,t.msgId,"destination unavailable");
    changeSmsStateToEnroute(*sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(*sms));
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
    Descriptor d;
    changeSmsStateToEnroute(*sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(*sms));
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
  //Task task((uint32_t)c.dest_proxy_index,dialogId2);

  TaskGuard tg;
  tg.smsc=smsc;
  tg.dialogId=dialogId2;
  tg.uniqueId=uniqueId;

  bool taskCreated=false;
  try{
    Task task(uniqueId,dialogId2,isDatagram || isTransaction?new SMS(*sms):0);
    task.messageId=t.msgId;
    task.diverted=diverted;
    task.inDlgId=resp->cntx.inDlgId;
    if ( smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
    {
      taskCreated=true;
      tg.active=true;
    }
  }catch(...)
  {
  }

  if(!taskCreated)
  {
    sms->setLastResult(Status::SYSERR);
    Descriptor d;
    changeSmsStateToEnroute(*sms,t.msgId,d,Status::SYSERR,rescheduleSms(*sms));

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

  bool deliveryOk=false;
  int  err=0;
  smsc::core::buffers::FixedLengthString<64> errstr;

  try{

    if( !isForwardTo )
    {
      // send delivery
      Address src;
      smsc_log_debug(smsLog,"SUBMIT: Id=%lld wantAlias=%s, hide=%s",t.msgId,dstSmeInfo.wantAlias?"true":"false",HideOptionToText(sms->getIntProperty(Tag::SMSC_HIDE)));
#ifdef SMSEXTRA
      if(sms->getIntProperty(Tag::SMPP_PRIVACYINDICATOR)==2)
      {
        Profile srcprof=smsc->getProfiler()->lookup(sms->getOriginatingAddress());
        if(srcprof.nick.length())
        {
          try{
            Address nick((uint8_t)srcprof.nick.length(),5,0,srcprof.nick.c_str());
            sms->setOriginatingAddress(nick);
          }catch(...)
          {
            warn2(smsLog,"Failed to construct nick from '%s' for abonent '%s'",srcprof.nick.c_str(),sms->getOriginatingAddress().toString().c_str());
          }
        }
      }else
#endif
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
        prepareSmsDc(*sms,dstSmeInfo.hasFlag(sfDefaultDcLatin1));
      }
    }

    if(sms->hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      if(!extractSmsPart(sms,0))
      {
        smsc_log_error(smsLog,"Id=%lld:failed to extract sms part, aborting.",t.msgId);
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

    smsc_log_debug(smsLog,"SBM: Id=%lld, esm_class=%x",t.msgId,sms->getIntProperty(Tag::SMPP_ESM_CLASS));

    SmscCommand delivery = SmscCommand::makeDeliverySm(*sms,dialogId2);
    unsigned bodyLen=0;
    delivery->get_sms()->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&bodyLen);
    int prio=priority/1000;
    if(prio<0)prio=0;
    if(prio>=32)prio=31;
    delivery->set_priority(prio);
    try{
      hrtime_t putCommandStart;

      if(perfLog->isInfoEnabled())
      {
        putCommandStart=gethrtime();
      }
      dest_proxy->putCommand(delivery);

      if (perfLog->isInfoEnabled())
      {
        hrtime_t putCommandTime=(gethrtime()-putCommandStart)/1000000;
        if(putCommandTime>20)
        {
          smsc_log_info(perfLog,"put command time=%lld",putCommandTime);
        }
      }
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
        smsc_log_warn(smsLog,"SUBMIT: failed to change state of Id=%lld to enroute/undeliverable",t.msgId);
      }
    }
    return Status::isErrorPermanent(err)?UNDELIVERABLE_STATE:ENROUTE_STATE;
  }

  tg.active=false;
  repGuard.active=false;

  sms->lastResult=Status::OK;
  char ussdOpBuf[32]={0,};
  if(sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    sprintf(ussdOpBuf,";ussdOp=%d",sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP));
  }
  char esmBuf[32]={0,};
  if(sms->hasIntProperty(Tag::SMPP_ESM_CLASS) && sms->getIntProperty(Tag::SMPP_ESM_CLASS)!=0)
  {
    sprintf(esmBuf,";esm=%x",sms->getIntProperty(Tag::SMPP_ESM_CLASS));
  }
  info2(smsLog,"SBM: submit ok, seqnum=%d Id=%lld;seq=%d;%s;%s;srcprx=%s;dstprx=%s;valid=%lu%s%s",
    dialogId2,
    t.msgId,dialogId,
    AddrPair("oa",sms->getOriginatingAddress(),"ooa",srcOriginal).c_str(),
    AddrPair("da",dstOriginal,"dda",sms->getDestinationAddress()).c_str(),
    src_proxy->getSystemId(),
    sms->getDestinationSmeId(),
    sms->getValidTime(),
    ussdOpBuf,
    esmBuf
  );
  return DELIVERING_STATE;
}

void StateMachine::prepareSmsDc(SMS& sms,bool defaultDcLatin1)
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
        smsc_log_debug(smsLog,"SUBMIT: transliterate olddc(%x)->dc(%x)",olddc,dc);
      }
    }catch(exception& e)
    {
      smsc_log_warn(smsLog,"SUBMIT:Failed to transliterate: %s",e.what());
    }
  }
  if(defaultDcLatin1 && sms.getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::SMSC7BIT)
  {
    const char* body;
    unsigned len;
    bool payload=false;
    if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      body=sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
      payload=true;
    }else
    {
      body=sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
    }
    TmpBuf<char,2048> buf(len+1);
    unsigned msgstart=0;
    if(sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
    {
      msgstart=1+*((unsigned char*)body);
      memcpy(buf.get(),body,msgstart);
    }
    unsigned newlen=ConvertSMSC7BitToLatin1(body+msgstart,len-msgstart,buf.get()+msgstart)+msgstart;
    if(payload)
    {
      sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,buf.get(),newlen);
    }else
    {
      sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,buf.get(),newlen);
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,newlen);
    }
  }

}


StateType StateMachine::forward(Tuple& t)
{
  SMS sms;
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    smsc_log_warn(smsLog, "FWD: failed to retriveSms Id=%lld",t.msgId);
    smsc->getScheduler()->InvalidSms(t.msgId);
    return UNKNOWN_STATE;
  }

  bool firstPart=true;
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    if(sms.getConcatSeqNum()!=0)
    {
      firstPart=false;
    }
  }

  info2(smsLog,"FWD: Id=%lld;oa=%s;da=%s;srcSme=%s",t.msgId,sms.originatingAddress.toString().c_str(),sms.destinationAddress.toString().c_str(),sms.getSourceSmeId());

  if(sms.getAttemptsCount()==0 && sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
  {
    onSubmitOk(t.msgId,sms);
  }else
  {
    onForwardOk(t.msgId,sms);
  }

  smsc_log_debug(smsLog,"orgMSC=%s, orgIMSI=%s",sms.originatingDescriptor.msc,sms.originatingDescriptor.imsi);
  INFwdSmsChargeResponse::ForwardContext ctx;
  ctx.allowDivert=t.command->get_forwardAllowDivert();
  ctx.reschedulingForward=t.command->is_reschedulingForward();
  if(sms.billingRequired() && !sms.hasIntProperty(Tag::SMSC_CHARGINGPOLICY))
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
      warn2(smsLog,"Failed to replace sms with Id=%lld in store:%s",t.msgId,e.what());
    }
  }

  if(sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnDelivery  && sms.billingRequired() && firstPart)
  {
    try{
      smsc->ChargeSms(t.msgId,sms,ctx);
    }catch(std::exception& e)
    {
      warn2(smsLog,"FWD: ChargeSms for Id=%lld failed:%s",t.msgId,e.what());
      sms.setLastResult(Status::NOCONNECTIONTOINMAN);
      onDeliveryFail(t.msgId,sms);
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
    smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    onDeliveryFail(t.msgId,sms);
    return ERROR_STATE;
  }

  if(sms.getState()==EXPIRED_STATE)
  {
    smsc_log_warn(smsLog, "FWD: sms in expired state Id=%lld",t.msgId);
    sms.setLastResult(Status::EXPIRED);
    smsc->getScheduler()->InvalidSms(t.msgId);
    onDeliveryFail(t.msgId,sms);
    return EXPIRED_STATE;
  }


  if(sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT) && sms.getIntProperty(Tag::SMSC_MERGE_CONCAT)!=3)
  {
    smsc_log_warn(smsLog, "Attempt to forward incomplete concatenated message Id=%lld",t.msgId);
    try{
      Descriptor d;
      sms.setLastResult(Status::SYSERR);
      smsc->getScheduler()->InvalidSms(t.msgId);
      store->changeSmsStateToUndeliverable
      (
        t.msgId,
        d,
        Status::SYSERR
      );
    }catch(...)
    {
      smsc_log_warn(smsLog,"failed to change sms state to undeliverable");
    }
    smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    onDeliveryFail(t.msgId,sms);
    return UNDELIVERABLE_STATE;
  }

  if(sms.getState()!=ENROUTE_STATE)
  {
    smsc_log_warn(smsLog, "FWD: sms Id=%lld is not in enroute (%d)",t.msgId,sms.getState());
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::SYSERR);
    smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    onDeliveryFail(t.msgId,sms);
    return sms.getState();
  }
  time_t now=time(NULL);
  if( sms.getNextTime()>now && sms.getAttemptsCount()==0 && (!isReschedulingForward || sms.getLastResult()==0) )
  {
    debug2(smsLog, "FWD: nextTime>now (%d>%d)",sms.getNextTime(),now);
    SmeIndex idx=smsc->getSmeIndex(sms.dstSmeId);
    smsc->getScheduler()->AddScheduledSms(t.msgId,sms,idx);
    sms.setLastResult(Status::SYSERR);
    smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    onDeliveryFail(t.msgId,sms);
    return sms.getState();
  }


  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    try{
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      smsc_log_warn(smsLog,"FORWARD: Failed to change state of USSD request to undeliverable. Id=%lld",t.msgId);
    }
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::EXPIRED);
    smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    onUndeliverable(t.msgId,sms);
    return UNDELIVERABLE_STATE;
  }


  if(sms.getLastTime()>sms.getValidTime() || sms.getNextTime()==1)
  {
    sms.setLastResult(Status::EXPIRED);
    try{
      smsc->getScheduler()->InvalidSms(t.msgId);
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      smsc_log_warn(smsLog,"FWD: failed to change state to expired");
    }
    info2(smsLog, "FWD: Id=%lld expired lastTry(%u)>valid(%u) or max attempts reached(%d:%d)",
          t.msgId,sms.getLastTime(),sms.getValidTime(),
          sms.oldResult,sms.getAttemptsCount());
    sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");
    smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    onUndeliverable(t.msgId,sms);
    return EXPIRED_STATE;
  }

  if(!t.command->get_fwdChargeSmsResp()->result)
  {
    char bufsrc[64],bufdst[64];
    sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
    sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
    smsc_log_warn(smsLog, "FWD: denied by inman Id=%lld;oa=%s;da=%s;err='%s'",t.msgId,bufsrc,bufdst,t.command->get_fwdChargeSmsResp()->inmanError.c_str());
    sms.setLastResult(Status::DENIEDBYINMAN);
    onDeliveryFail(t.msgId,sms);
    sendNotifyReport(sms,t.msgId,"destination unavailable");
    Descriptor d;
    changeSmsStateToEnroute(sms,t.msgId,d,Status::DENIEDBYINMAN,rescheduleSms(sms));
    return ENROUTE_STATE;
  }

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
      diverted=true;
      if(smsc->AliasToAddress(dst,newdst))
      {
        dst=newdst;
      }
    }
  }

  smsc::router::RouteResult rr;
  bool has_route = false;
  try{
    has_route=smsc->routeSms(smsc->getSmeIndex(sms.getSourceSmeId()),sms.getOriginatingAddress(),dst,rr);
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
    smsc_log_warn(smsLog, "FWD: No route Id=%lld;oa=%s;da=%s",t.msgId,from,to);
    sms.setLastResult(Status::NOROUTE);
    sendNotifyReport(sms,t.msgId,"destination unavailable");
    time_t ntt=rescheduleSms(sms);
    if(ntt==sms.getValidTime())
    {
      try{
        store->changeSmsStateToExpired(t.msgId);
      }catch(...)
      {
        smsc_log_warn(smsLog,"FORWARD: Failed to change state of USSD request to undeliverable. Id=%lld",t.msgId);
      }
      smsc->getScheduler()->InvalidSms(t.msgId);
      sms.setLastResult(Status::EXPIRED);
      onUndeliverable(t.msgId,sms);
    }else
    {
      Descriptor d;
      changeSmsStateToEnroute(sms,t.msgId,d,Status::NOROUTE,ntt);
      onDeliveryFail(t.msgId,sms);
    }
    smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    return ERROR_STATE;
  }

  smsc_log_debug(smsLog,"FWD: last result=%d",sms.getLastResult());
  if(sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID) &&
     sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
  {
    smsc_log_debug(smsLog,"FWD: diverted receipt for %s",sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
    rr.destSmeIdx=smsc->getSmeIndex(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
    rr.destProxy=smsc->getSmeProxy(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
  }else if(sms.getLastResult()==Status::BACKUPSMERESCHEDULE && sms.hasStrProperty(Tag::SMSC_BACKUP_SME))
  {
    smsc_log_debug(smsLog,"FWD: reroute to backup sme %s",sms.getStrProperty(Tag::SMSC_BACKUP_SME).c_str());
    rr.destSmeIdx=smsc->getSmeIndex(sms.getStrProperty(Tag::SMSC_BACKUP_SME).c_str());
    rr.destProxy=smsc->getSmeProxy(sms.getStrProperty(Tag::SMSC_BACKUP_SME).c_str());
  }


  if(!rr.destProxy)
  {
    char bufsrc[64],bufdst[64];
    sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
    sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
    smsc_log_warn(smsLog, "FWD: sme is not connected Id=%lld;oa=%s;da=%s;dstSme=%s",t.msgId,bufsrc,bufdst,rr.info.smeSystemId.c_str());
    sms.setLastResult(Status::SMENOTCONNECTED);
    onDeliveryFail(t.msgId,sms);
    sendNotifyReport(sms,t.msgId,"destination unavailable");
    Descriptor d;
    changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));
    smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    return ENROUTE_STATE;
  }
  // create task


  if(doRepartition && rr.info.smeSystemId=="MAP_PROXY")
  {
    debug2(smsLog,"FWD: sms repartition Id=%lld",t.msgId);
    int pres=partitionSms(&sms);
    if(pres!=psSingle && pres!=psMultiple)
    {
      debug2(smsLog,"FWD: divert failed - cannot concat, Id=%lld",t.msgId);
      sms.setLastResult(Status::SYSERR);
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
      Descriptor d;
      changeSmsStateToEnroute(sms,t.msgId,d,Status::SYSERR,rescheduleSms(sms));
      return UNKNOWN_STATE;
    }
    debug2(smsLog,"Id=%lld after repartition: %s",t.msgId,pres==psSingle?"single":"multiple");
  }

  SmeInfo dstSmeInfo=smsc->getSmeInfo(rr.destSmeIdx);

  if(dstSmeInfo.systemId!=sms.getDestinationSmeId() || rr.info.routeId!=sms.getRouteId())
  {
    sms.setDestinationSmeId(dstSmeInfo.systemId.c_str());
    sms.setRouteId(rr.info.routeId.c_str());
    try{
      store->replaceSms(t.msgId,sms);
    }catch(...)
    {
      smsc_log_warn(smsLog,"Failed to replace Id=%lld",t.msgId);
    }
  }

  if(dstSmeInfo.hasFlag(sfForceReceiptToSme) && sms.hasStrProperty(Tag::SMSC_RECIPIENTADDRESS))
  {
    sms.setOriginatingAddress(sms.getStrProperty(Tag::SMSC_RECIPIENTADDRESS).c_str());
  }

  TaskGuard tg;
  tg.smsc=smsc;


  uint32_t dialogId2;
  uint32_t uniqueId=rr.destProxy->getUniqueId();
  bool taskCreated=false;
  try{
    dialogId2 = rr.destProxy->getNextSequenceNumber();
    tg.dialogId=dialogId2;
    tg.uniqueId=uniqueId;
    debug2(smsLog, "FWDDLV: Id=%lld;seq=%d",t.msgId,dialogId2);
    //Task task((uint32_t)dest_proxy_index,dialogId2);
    Task task(uniqueId,dialogId2);
    task.diverted=diverted;
    task.messageId=t.msgId;
    task.inDlgId=inDlgId;
    if ( smsc->tasks.createTask(task,rr.destProxy->getPreferredTimeout()) )
    {
      taskCreated=true;
      tg.active=true;
    }
  }catch(...)
  {
  }
  if(!taskCreated)
  {
    smsc_log_info(smsLog,"Failed to create task for Id=%lld",t.msgId);
    Descriptor d;
    sms.setLastResult(Status::SMENOTCONNECTED);
    changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));
    onDeliveryFail(t.msgId,sms);
    smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
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
        smsc->getSmeInfo(rr.destProxy->getIndex()).wantAlias &&
        sms.getIntProperty(Tag::SMSC_HIDE)==HideOption::hoEnabled &&
        rr.info.hide &&
        smsc->AddressToAlias(sms.getOriginatingAddress(),src)
      )
    {
      sms.setOriginatingAddress(src);
    }
    //Address dst=sms.getDealiasedDestinationAddress();
    sms.setDestinationAddress(dst);
    if(!sms.hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      if(sms.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3 && !rr.info.transit)
      {
        prepareSmsDc(sms,dstSmeInfo.hasFlag(sfDefaultDcLatin1));
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
        smsc_log_warn(smsLog,"attempt to forward concatenated message but all parts are delivered!!!");
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
          smsc_log_warn(smsLog,"failed to change state of sms %lld to final ... again!!!",t.msgId);
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
    if(rr.info.replyPath==smsc::router::ReplyPathForce)
    {
      sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x80);
    }else if(rr.info.replyPath==smsc::router::ReplyPathSuppress)
    {
      sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x80));
    }
    smsc_log_debug(smsLog,"FWD: Id=%lld, esm_class=%x",t.msgId,sms.getIntProperty(Tag::SMPP_ESM_CLASS));
    SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
    rr.destProxy->putCommand(delivery);
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
    smsc_log_warn(smsLog, "FWDDLV: failed create deliver(%s) srcSme=%s;Id=%lld;seqNum=%d;oa=%s;da=%s",errtext,
        sms.getSourceSmeId(),t.msgId,dialogId2,
        sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str());

    sms.setOriginatingAddress(srcOriginal);
    sms.setDestinationAddress(dstOriginal);
    sms.setLastResult(errstatus);
    onDeliveryFail(t.msgId,sms);
    if(Status::isErrorPermanent(errstatus))
      sendFailureReport(sms,t.msgId,errstatus,"system failure");
    else
      sendNotifyReport(sms,t.msgId,"system failure");
    try{
      Descriptor d;
      if(Status::isErrorPermanent(errstatus))
      {
        smsc->getScheduler()->InvalidSms(t.msgId);
        store->changeSmsStateToUndeliverable(t.msgId,d,errstatus);
      }
      else
        changeSmsStateToEnroute(sms,t.msgId,d,errstatus,rescheduleSms(sms));
    }catch(...)
    {
      smsc_log_warn(smsLog,"FORWARD: failed to change state to enroute/undeliverable");
    }
    smsc->ReportDelivery(inDlgId,sms,Status::isErrorPermanent(errstatus),Smsc::chargeOnDelivery);
    return Status::isErrorPermanent(errstatus)?UNDELIVERABLE_STATE:ENROUTE_STATE;
  }
  info2(smsLog, "FWDDLV: deliver ok Id=%lld;seqNum=%d;oa=%s;da=%s;srcSme=%s;dstSme=%s;routeId=%s",t.msgId,dialogId2,
      sms.getOriginatingAddress().toString().c_str(),
      sms.getDealiasedDestinationAddress().toString().c_str(),
      sms.getSourceSmeId(),
      sms.getDestinationSmeId(),
      rr.info.routeId.c_str());

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
         status==Status::MSMEMCAPACITYFULL
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
    warn2(smsLog, "DLVRSP: state of SMS isn't DELIVERING!!! Id=%lld;st=%d",t.msgId,t.command->get_resp()->get_status());
    smsc->getScheduler()->InvalidSms(t.msgId);
    return t.state;
  }

  {
    Descriptor d=t.command->get_resp()->getDescriptor();
    smsc_log_debug(smsLog,"resp dest descriptor:%s(%d)/%s(%d), Id=%lld",d.imsi,d.imsiLength,d.msc,d.mscLength,t.msgId);
  }
  int statusType=GET_STATUS_TYPE(t.command->get_resp()->get_status());
  int statusCode=GET_STATUS_CODE(t.command->get_resp()->get_status());

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
      smsc_log_warn(smsLog, "DLVRSP: failed to retrieve sms:%s! Id=%lld;st=%d",e.what(),t.msgId,t.command->get_resp()->get_status());
      smsc->getScheduler()->InvalidSms(t.msgId);
      onDeliveryFail(t.msgId,sms);
      return UNKNOWN_STATE;
    }
    sms.destinationDescriptor=t.command->get_resp()->getDescriptor();
  }

  {
    Descriptor d=sms.destinationDescriptor;
    smsc_log_debug(smsLog,"sms dest descriptor:%s(%d)/%s(%d), Id=%lld",d.imsi,d.imsiLength,d.msc,d.mscLength,t.msgId);
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

  time_t now=time(NULL);
  bool firstPart=false;
  bool multiPart=sms.hasBinProperty(Tag::SMSC_CONCATINFO);

  if(statusType!=CMD_OK && !dgortr)
  {
    bool softLimit=smsc->checkSchedulerSoftLimit();
    if((sms.getValidTime()<=now) || //expired or
       RescheduleCalculator::calcNextTryTime(now,sms.getLastResult(),sms.getAttemptsCount())==-1 || //max attempts count reached or
       softLimit  //soft limit reached
       )
    {
      sms.setLastResult(softLimit?Status::SCHEDULERLIMIT:Status::EXPIRED);
      onUndeliverable(t.msgId,sms);
      try{
        smsc->getScheduler()->InvalidSms(t.msgId);
        store->changeSmsStateToExpired(t.msgId);
      }catch(...)
      {
        smsc_log_warn(smsLog,"DLVRSP: failed to change state to expired");
      }
      info2(smsLog, "DLVRSP: Id=%lld;oa=%s;da=%s %s (valid:%u - now:%u), attempts=%d",t.msgId,
          sms.getOriginatingAddress().toString().c_str(),
          sms.getDestinationAddress().toString().c_str(),
          softLimit?"denied by soft sched limit":"expired",sms.getValidTime(),now,sms.getAttemptsCount());
      sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");
      smsc->ReportDelivery(t.command->get_resp()->get_inDlgId(),sms,true,Smsc::chargeOnDelivery);
      return EXPIRED_STATE;
    }
  }

  if(sms.billingRequired() && sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnDelivery)
  {
    bool final=
      statusType==CMD_OK ||
      statusType==CMD_ERR_PERM ||
      (
       statusType==CMD_ERR_TEMP &&
       (
        sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)||
        (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2
       )
      );
    if(multiPart)
    {
      unsigned len;
      if(sms.getConcatSeqNum()==0)
      {
        firstPart=true;
      }
    }
    smsc_log_debug(smsLog,"multiPart=%s, firstPart=%s, final=%s",multiPart?"true":"false",firstPart?"true":"false",final?"true":"false");
    if(!multiPart || firstPart)
    {
      int savedLastResult=sms.getLastResult();
      try{
        sms.setLastResult(statusCode);
        if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && !t.command->get_resp()->get_diverted())
        {
          std::string savedDivert=sms.getStrProperty(Tag::SMSC_DIVERTED_TO);
          sms.getMessageBody().dropProperty(Tag::SMSC_DIVERTED_TO);
          smsc->ReportDelivery(t.command->get_resp()->get_inDlgId(),sms,final,Smsc::chargeOnDelivery);
          sms.setStrProperty(Tag::SMSC_DIVERTED_TO,savedDivert.c_str());
        }else
        {
          smsc->ReportDelivery(t.command->get_resp()->get_inDlgId(),sms,final,Smsc::chargeOnDelivery);
        }
      }catch(std::exception& e)
      {
        smsc_log_warn(smsLog,"ReportDelivery for Id=%lld failed:'%s'",t.msgId,e.what());
      }
      sms.setLastResult(savedLastResult);
      /*
      if(multiPart && statusType==CMD_OK && (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x02) && firstPart)
      {
        smsc_log_info(smsLog,"Remove billing flag for multipart sms msgId=%lld",t.msgId);
        sms.setBillingRecord(0);
      }
      */
    }
  }

  {
    char partBuf[32];
    if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      ConcatInfo* ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,0);
      sprintf(partBuf,";part=%d/%d",sms.getConcatSeqNum()+1,ci->num);
    }else
    {
      partBuf[0]=0;
    }

    info2(smsLog, "DLVRSP: Id=%lld;class=%s;st=%d;oa=%s;%s;srcprx=%s;dstprx=%s;route=%s%s%s%s",t.msgId,
        statusType==CMD_OK?"OK":
        statusType==CMD_ERR_RESCHEDULENOW?"RESCHEDULEDNOW":
        statusType==CMD_ERR_TEMP?"TEMP ERROR":"PERM ERROR",
        statusCode,
        sms.getOriginatingAddress().toString().c_str(),
        AddrPair("da",sms.getDestinationAddress(),"dda",sms.getDealiasedDestinationAddress()).c_str(),
        sms.getSourceSmeId(),
        sms.getDestinationSmeId(),
        sms.getRouteId(),
        t.command->get_resp()->get_diverted()?";diverted_to=":"",
        t.command->get_resp()->get_diverted()?sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str():"",
        partBuf
    );
  }


  if(statusType==CMD_OK)
  {
    sms.setLastResult(Status::OK);
    onDeliveryOk(t.msgId,sms);
  }else
  {
    sms.setLastResult(statusCode);
    if(dgortr)
    {
      onUndeliverable(t.msgId,sms);
    }else
    {
      if(statusType==CMD_ERR_RESCHEDULENOW || statusType==CMD_ERR_TEMP)
      {
        onDeliveryFail(t.msgId,sms);
      }else
      {
        onUndeliverable(t.msgId,sms);
      }
    }
  }

  if((statusCode==Status::MAP_NO_RESPONSE_FROM_PEER ||
     statusCode==Status::BLOCKEDMSC) &&
     sms.hasStrProperty(Tag::SMSC_BACKUP_SME)
  )
  {
    statusCode=Status::BACKUPSMERESCHEDULE;
    statusType=CMD_ERR_TEMP;
    t.command->get_resp()->set_status(MAKE_COMMAND_STATUS(statusType,statusCode));
  }

  if(statusType!=CMD_OK)
  {
    if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2 &&
       sms.getIntProperty(Tag::SMPP_SET_DPF))//forward/transaction mode
    {
      if(statusCode==1179 || statusCode==1044)
      {
        try{
          if(!smsc->getScheduler()->registerSetDpf(
              sms.getDealiasedDestinationAddress(),
              sms.getOriginatingAddress(),
              statusCode,
              sms.getValidTime(),
              sms.getSourceSmeId()))
          {
            sms.setIntProperty(Tag::SMPP_SET_DPF,0);
          }
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to create dpf sms:%s",e.what());
          sms.setIntProperty(Tag::SMPP_SET_DPF,0);
        }
      }else
      {
        sms.setIntProperty(Tag::SMPP_SET_DPF,0);
      }
    }

    switch(statusType)
    {
      case CMD_ERR_RESCHEDULENOW:
      {
        time_t rt=now+2;
        if(t.command->get_resp()->get_delay()>2)
        {
          rt+=t.command->get_resp()->get_delay()-2;
        }
        smsc_log_debug(smsLog,"DELIVERYRESP: change state to enroute (reschedule now=%d)",rt);
        changeSmsStateToEnroute
        (
            sms,
            t.msgId,
            sms.getDestinationDescriptor(),
            Status::RESCHEDULEDNOW,
            rt,
            true
        );

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
        time_t rt;
        if(t.command->get_resp()->get_delay()>2)
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
            statusCode,
            rt
        );

        sendNotifyReport(sms,t.msgId,"subscriber busy");
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
            smsc_log_debug(smsLog,"DELIVERYRESP: change state to undeliverable");

            if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && !t.command->get_resp()->get_diverted())
            {
              sms.getMessageBody().dropProperty(Tag::SMSC_DIVERTED_TO);
              store->replaceSms(t.msgId,sms);
            }

            store->changeSmsStateToUndeliverable
            (
              t.msgId,
              sms.getDestinationDescriptor(),
              statusCode
            );
          }catch(std::exception& e)
          {
            smsc_log_warn(smsLog,"DELIVERYRESP: failed to change state to undeliverable:%s",e.what());
          }
        }

        sendFailureReport(sms,t.msgId,UNDELIVERABLE_STATE,"permanent error");

        if(!dgortr)
        {
          smsc->getScheduler()->InvalidSms(t.msgId);
        }

        fullReport(t.msgId,sms);
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
        }
        return UNDELIVERABLE_STATE;
      }
    }
  }


#ifdef SMSEXTRA
  if(!sms.hasBinProperty(Tag::SMSC_CONCATINFO) ||
     (sms.hasBinProperty(Tag::SMSC_CONCATINFO) && sms.getConcatSeqNum()==0))
  {
    if(createCopyOnNickUsage && sms.getIntProperty(Tag::SMSC_EXTRAFLAGS)&EXTRA_NICK)
    {
      SMS newsms=sms;
      newsms.setIntProperty(Tag::SMSC_EXTRAFLAGS,EXTRA_FAKE);
      try{
        SMSId msgId=store->getNextId();
        newsms.setSourceSmeId(smscSmeId.c_str());
        store->createSms(newsms,msgId,smsc::store::CREATE_NEW);
        smsc->getScheduler()->AddFirstTimeForward(msgId,newsms);
      }catch(std::exception& e)
      {
        smsc_log_warn(smsLog,"Failed to create fake sms for Id=%lld;oa=%s;da=%s;err='%s'",t.msgId,sms.getOriginatingAddress().toString().c_str(),
                      sms.getDealiasedDestinationAddress().toString().c_str(),e.what());
      }
    }
  }
#endif

  // concatenated message with conditional divert.
  // first part delivered ok.
  // other parts MUST be delivered to the same address.
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO) && sms.getConcatSeqNum()==0 &&
     (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&(DF_COND|DF_UNCOND)))
  {
    debug2(smsLog,"DLVRESP: Id=%lld - delivered first part of multipart sms with conditional divert.",t.msgId);
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
      smsc_log_warn(smsLog,"failed to replace sms in store (divert fix) Id=%lld",t.msgId);
    }
  }

  //bool skipFinalizing=false;

  unsigned char umrList[256]; //umrs of parts of merged message
  umrList[0]=sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
  uint64_t stimeList[256]; // submit time of merged messages
  stimeList[0]=sms.getSubmitTime();
  int umrListSize=0;
  //int umrIndex=-1;//index of current umr
  //bool umrLast=true;//need to generate receipts for the rest of umrs
  //int savedCsn=sms.getConcatSeqNum();

  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    if(smsc->getSmartMultipartForward() && (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&3)==2)
    {
      info2(smsLog,"enabling smartMultipartForward  for Id=%lld",t.msgId);
      try{
        sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&~3);
        store->createSms(sms,t.msgId,smsc::store::CREATE_NEW_NO_CLEAR);
      }catch(std::exception& e)
      {
        smsc_log_warn(smsLog,"DELIVERYRESP: failed to create sms for SmartMultipartForward:'%s'",e.what());
        finalizeSms(t.msgId,sms);
        onUndeliverable(t.msgId,sms);
        return UNDELIVERABLE_STATE;
      }
      dgortr=false;
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
          smsc_log_warn(smsLog,"DELIVERYRESP: failed to put transaction response command");
        }
      }
    }

    unsigned int len;
    ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
    info2(smsLog, "DLVRSP: sms has concatinfo, csn=%d/%d;Id=%lld",sms.getConcatSeqNum(),ci->num,t.msgId);
    if(sms.getConcatSeqNum()<ci->num-1)
    {
      if(sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnSubmit)
      {
        fullReport(t.msgId,sms);
      }
      {
        sms.setConcatSeqNum(sms.getConcatSeqNum()+1);
        if(!dgortr)
        try
        {
          store->changeSmsConcatSequenceNumber(t.msgId);
        }catch(std::exception& e)
        {
          smsc_log_warn(smsLog,"DELIVERYRESP: failed to change sms concat seq num Id=%lld;err='%s'",t.msgId,e.what());
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
            smsc_log_warn(smsLog,"DELIVERYRESP: failed to cahnge sms state to enroute:Id=%lld",t.msgId);
          }
          if(dgortr)
          {
            sms.state=UNDELIVERABLE;
            finalizeSms(t.msgId,sms);
            onUndeliverable(t.msgId,sms);
            return UNDELIVERABLE_STATE;
          }else
          {
            onDeliveryFail(t.msgId,sms);
          }
          return UNKNOWN_STATE;
        }
        smsc_log_debug(smsLog,"CONCAT: concatseqnum=%d for Id=%lld",sms.getConcatSeqNum(),t.msgId);
      }

      ////
      //
      //  send concatenated
      //

      Address dst=sms.getDealiasedDestinationAddress();

      // for interfaceVersion==0x50
      if(!sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
      {
        if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&(DF_COND|DF_UNCOND)))
        {
          dst=sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str();
        }
      }

      smsc::router::RouteResult rr;
      bool has_route = false;
      try{
        has_route=smsc->routeSms(smsc->getSmeIndex(sms.getSourceSmeId()),sms.getOriginatingAddress(),dst,rr);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Routing %s->%s failed:%s",sms.getOriginatingAddress().toString().c_str(),
          dst.toString().c_str(),e.what());
      }
      if ( !has_route )
      {
        smsc_log_warn(smsLog,"CONCAT: no route Id=%lld;oa=%s;da=%s",t.msgId,sms.getOriginatingAddress().toString().c_str(),dst.toString().c_str());
        sms.setLastResult(Status::NOROUTE);
        sendNotifyReport(sms,t.msgId,"destination unavailable");
        //time_t now=time(NULL);
        Descriptor d;
        changeSmsStateToEnroute(sms,t.msgId,d,Status::NOROUTE,rescheduleSms(sms));

        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          onUndeliverable(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }else
        {
          onDeliveryFail(t.msgId,sms);
        }
        return ERROR_STATE;
      }

      if(sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID) &&
         sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
      {
        smsc_log_debug(smsLog,"CONCAT: diverted receipt for %s",sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
        rr.destSmeIdx=smsc->getSmeIndex(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
        rr.destProxy=smsc->getSmeProxy(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
      }

      if(!rr.destProxy)
      {
        smsc_log_info(smsLog,"CONCAT: dest sme %s not connected msgId=%lld;oa=%s;da=%s",sms.getDestinationSmeId(),t.msgId,
            sms.getOriginatingAddress().toString().c_str(),
            sms.getDestinationAddress().toString().c_str());
        sms.setLastResult(Status::SMENOTCONNECTED);
        sendNotifyReport(sms,t.msgId,"destination unavailable");
        Descriptor d;
        changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          return UNDELIVERABLE_STATE;
          onUndeliverable(t.msgId,sms);
        }else
        {
          onDeliveryFail(t.msgId,sms);
        }
        return ENROUTE_STATE;
      }
      // create task

      uint32_t dialogId2;
      uint32_t uniqueId=rr.destProxy->getUniqueId();

      TaskGuard tg;
      tg.smsc=smsc;
      tg.uniqueId=uniqueId;

      try{
        dialogId2 = rr.destProxy->getNextSequenceNumber();
        tg.dialogId=dialogId2;

        Task task(uniqueId,dialogId2,dgortr?new SMS(sms):0);
        task.messageId=t.msgId;
        task.inDlgId=t.command->get_resp()->get_inDlgId();
        task.diverted=t.command->get_resp()->get_diverted();
        if ( smsc->tasks.createTask(task,rr.destProxy->getPreferredTimeout()) )
        {
          tg.active=true;
        }
      }catch(...)
      {
        Descriptor d;
        changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          onUndeliverable(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }else
        {
          onDeliveryFail(t.msgId,sms);
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
        if(smsc->getSmeInfo(rr.destProxy->getIndex()).wantAlias &&
           sms.getIntProperty(Tag::SMSC_HIDE)==HideOption::hoEnabled &&
           rr.info.hide &&
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
        if(rr.info.replyPath==smsc::router::ReplyPathForce)
        {
          sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x80);
        }else if(rr.info.replyPath==smsc::router::ReplyPathSuppress)
        {
          sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x80));
        }

        smsc_log_debug(smsLog,"CONCAT: Id=%lld;esm_class=%x",t.msgId,sms.getIntProperty(Tag::SMPP_ESM_CLASS));
        SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
        rr.destProxy->putCommand(delivery);
        tg.active=false;
      }
      catch(ExtractPartFailedException& e)
      {
        errstatus=Status::INVPARLEN;
        smsc_log_error(smsLog,"CONCAT: failed to extract sms part for Id=%lld",t.msgId);
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
        smsc_log_warn(smsLog,"CONCAT::Err %s",errtext);
        try{
          Descriptor d;
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
          smsc_log_warn(smsLog,"CONCAT: failed to change state to enroute");
        }
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          onUndeliverable(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }
        if(Status::isErrorPermanent(errstatus))
        {
          onUndeliverable(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }else
        {
          onDeliveryFail(t.msgId,sms);
          return ENROUTE_STATE;
        }
      }

      return DELIVERING_STATE;
    }
  }

  if(sms.hasBinProperty(Tag::SMSC_ORGPARTS_INFO))
  {
    int cnt=getSMSPartsCount(sms);
    for(int i=0;i<cnt;i++)
    {
      SMSPartInfo spi=getSMSPartInfo(sms,i);
      if(spi.fl&SMSPartInfo::flHasSRR)
      {
        stimeList[umrListSize]=spi.stime?spi.stime:sms.getSubmitTime();
        umrList[umrListSize++]=spi.mr;
      }
    }
  }else
  if(sms.hasBinProperty(Tag::SMSC_UMR_LIST))
  {
    unsigned len;
    unsigned char* lst=(unsigned char*)sms.getBinProperty(Tag::SMSC_UMR_LIST,&len);
    if(!sms.hasBinProperty(Tag::SMSC_UMR_LIST_MASK))
    {
      for(unsigned i=0;i<len;i++)
      {
        stimeList[i]=sms.getSubmitTime();
      }
      memcpy(umrList,lst,len);
      umrListSize=len;
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
          stimeList[umrListSize]=sms.getSubmitTime();
          umrList[umrListSize++]=lst[i];
        }
      }
    }
  }

  if(dgortr)
  {
    sms.state=DELIVERED;
    if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2)
    {
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

        }
      }
    }
    fullReport(t.msgId,sms);
    try{
      store->createFinalizedSms(t.msgId,sms);
    }catch(...)
    {
      smsc_log_warn(smsLog,"DELRESP: failed to finalize sms with Id=%lld",t.msgId);
    }
    return DELIVERED_STATE;
  }else if(!finalized)
  {
    try{

      if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && !t.command->get_resp()->get_diverted())
      {
        sms.getMessageBody().dropProperty(Tag::SMSC_DIVERTED_TO);
        store->replaceSms(t.msgId,sms);
      }

      store->changeSmsStateToDelivered(t.msgId,t.command->get_resp()->getDescriptor());

      smsc->getScheduler()->DeliveryOk(t.msgId);

    }catch(std::exception& e)
    {
      warn2(smsLog,"change state to delivered exception:%s",e.what());
      //return UNKNOWN_STATE;
    }
  }
  debug2(smsLog, "DLVRSP: DELIVERED, Id=%lld",t.msgId);

  fullReport(t.msgId,sms);



  try{
    smsc_log_debug(smsLog,"DELIVERYRESP: suppdelrep=%d, delrep=%d, regdel=%d, srr=%d",
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
      rpt.setOriginatingDescriptor((uint8_t)strlen(msc),msc,(uint8_t)strlen(imsi),imsi,1);
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
      if(umrListSize)
      {
        rpt.setMessageReference(umrList[0]);
        rpt.setIntProperty(Tag::SMSC_RECEIPT_MR,umrList[0]);
      }
      smsc_log_debug(smsLog,"RECEIPT: set mr[0]=%d",rpt.getMessageReference());
      rpt.setIntProperty(Tag::SMPP_MSG_STATE,SmppMessageState::DELIVERED);
      char addr[64];
      sms.getDestinationAddress().getText(addr,sizeof(addr));
      rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
      rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,(unsigned)time(NULL));
      rpt.setIntProperty(Tag::SMSC_RECEIPTED_MSG_SUBMIT_TIME,(unsigned)stimeList[0]);
      SmeInfo si=smsc->getSmeInfo(sms.getSourceSmeId());
      if(si.hasFlag(sfForceReceiptToSme))
      {
        rpt.setStrProperty(Tag::SMSC_DIVERTED_TO,sms.getSourceSmeId());
      }
      char msgid[60];
      sprintf(msgid,"%lld",t.msgId);
      rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
      string out;
      sms.getDestinationAddress().getText(addr,sizeof(addr));
      const Descriptor& d=sms.getDestinationDescriptor();
      smsc_log_debug(smsLog,"RECEIPT: msc=%s, imsi=%s",d.msc,d.imsi);
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
      time_t tz=common::TimeZoneManager::getInstance().getTimeZone(rpt.getDestinationAddress())+timezone;
      fd.submitDate=stimeList[0]+tz;
      fd.date=time(NULL)+tz;
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
      smsc_log_debug(smsLog,"RECEIPT: sending receipt to %s:%s",addr,out.c_str());
      if(sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
      {
        fillSms(&rpt,"",0,CONV_ENCODING_CP1251,profile.codepage,0);
      }else
      {
        fillSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,0);
      }

      //smsc->submitSms(prpt);

      submitReceipt(rpt,0x4);

      for(int i=1;i<umrListSize;i++)
      {
        rpt.setIntProperty(Tag::SMSC_RECEIPTED_MSG_SUBMIT_TIME,(unsigned)stimeList[i]);
        rpt.setMessageReference(umrList[i]);
        rpt.setIntProperty(Tag::SMSC_RECEIPT_MR,umrList[i]);
        smsc_log_debug(smsLog,"RECEIPT: set mr[i]=%d",i,rpt.getMessageReference());
        submitReceipt(rpt,0x4);
      }
    }
  }catch(std::exception& e)
  {
    smsc_log_warn(smsLog,"DELIVERY_RESP:failed to submit receipt");
  }
  //return skipFinalizing?DELIVERING_STATE:DELIVERED_STATE;
  return DELIVERED_STATE;
}

void StateMachine::fullReport(SMSId msgId,SMS& sms)
{
  if(sms.billingRecord==BILLING_FINALREP)
  {
    smsc->FullReportDelivery(msgId,sms);
  }else if(sms.billingRequired() && sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnSubmit)
  {
    int reportsToSend=1;
    if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      ConcatInfo* ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,0);
      if(sms.getConcatSeqNum()>=sms.getIntProperty(Tag::SMSC_ORIGINALPARTSNUM))
      {
        reportsToSend=0;
      }else
      if(sms.getConcatSeqNum()==ci->num-1)
      {
        if(sms.getIntProperty(Tag::SMSC_ORIGINALPARTSNUM)>ci->num)
        {
          reportsToSend=sms.getIntProperty(Tag::SMSC_ORIGINALPARTSNUM)-ci->num+1;
        }
      }
      smsc_log_debug(smsLog,"sending fullreport for msgId=%lld, concatSeq=%d, ci->num=%d, orgparts=%d, repstosend=%d",
          msgId,sms.getConcatSeqNum(),(int)ci->num,sms.getIntProperty(Tag::SMSC_ORIGINALPARTSNUM),reportsToSend);
    }
    time_t stime=sms.getSubmitTime();
    for(int i=0;i<reportsToSend;i++)
    {
      if(sms.hasBinProperty(Tag::SMSC_ORGPARTS_INFO) && getSMSPartsCount(sms)>sms.getConcatSeqNum()+i)
      {
        SMSPartInfo spi=getSMSPartInfo(sms,sms.getConcatSeqNum()+i);
        if(spi.stime)
        {
          sms.setSubmitTime(spi.stime);
        }
      }
      smsc->FullReportDelivery(msgId,sms);
    }
    sms.setSubmitTime(stime);
  }
}


StateType StateMachine::alert(Tuple& t)
{
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
      smsc_log_warn(smsLog, "ALERT: Failed to retrieve sms:Id=%lld",t.msgId);
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
    try{
      smsc->getScheduler()->InvalidSms(t.msgId);
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      smsc_log_warn(smsLog,"ALERT: failed to change state to expired");
    }
    info2(smsLog, "ALERT: Id=%lld;oa=%s;da=%s expired (valid:%u - now:%u), attempts=%d",t.msgId,
        sms.getOriginatingAddress().toString().c_str(),
        sms.getDestinationAddress().toString().c_str(),
        sms.getValidTime(),now,sms.getAttemptsCount());
    sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");
    smsc->ReportDelivery(ad.inDlgId,sms,true,Smsc::chargeOnDelivery);
    onUndeliverable(t.msgId,sms);
    return EXPIRED_STATE;
  }

  sms.setLastResult(Status::DELIVERYTIMEDOUT);

  if(sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnDelivery)
  {
    smsc->ReportDelivery(ad.inDlgId,sms,
        dgortr,
        Smsc::chargeOnDelivery
    );
  }

  if(!sms.Invalidate(__FILE__,__LINE__))
  {
    smsc_log_warn(smsLog, "Invalidate of %lld failed",t.msgId);
    smsc->getScheduler()->InvalidSms(t.msgId);
    return ERROR_STATE;
  }

  smsc_log_info(smsLog, "ALERT: Id=%lld;oa=%s;da=%s;dstSme=%s",t.msgId,
      sms.getOriginatingAddress().toString().c_str(),
      sms.getDestinationAddress().toString().c_str(),
      sms.getDestinationSmeId());

  if(dgortr)
  {
    sms.state=EXPIRED;
    if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2)
    {
      smsc_log_debug(smsLog,"ALERT: Sending submit resp for forward mode sms");
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
          smsc_log_warn(smsLog,"ALERT: failed to put response command");
        }
      }
    }
    try{
      store->createFinalizedSms(t.msgId,sms);
    }catch(...)
    {
      smsc_log_warn(smsLog, "ALERT: failed to finalize sms:Id=%lld",t.msgId);
    }
    onUndeliverable(t.msgId,sms);
    return EXPIRED_STATE;
  }else
  {
    onDeliveryFail(t.msgId,sms);
    changeSmsStateToEnroute(sms,t.msgId,d,Status::DELIVERYTIMEDOUT,rescheduleSms(sms));
    sendNotifyReport(sms,t.msgId,"delivery attempt timed out");
  }
  return UNKNOWN_STATE;
}

StateType StateMachine::replace(Tuple& t)
{

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
      smsc_log_warn(smsLog,"REPLACE: failed to put response command"); \
    }                                 \
    return UNKNOWN_STATE;


  SMS sms;
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    smsc_log_warn(smsLog,"REPLACE: Failed to retrieve sms:%lld",t.msgId);
    __REPLACE__RESPONSE(REPLACEFAIL);
  }
  smsc_log_info(smsLog,"REPLACE: Id=%lld;oa=%s;da=%s",t.msgId,
      sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str());
  time_t oldtime=sms.getNextTime();
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    if(sms.getConcatSeqNum()>0)
    {
      smsc_log_warn(smsLog,"REPLACE: replace of concatenated message");
      __REPLACE__RESPONSE(REPLACEFAIL);
    }
    sms.getMessageBody().dropProperty(Tag::SMSC_CONCATINFO);
  }
  if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    smsc_log_debug(smsLog,"REPLACE: dropping payload");
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
      smsc_log_warn(smsLog,"REPLACE: invalid length of/for UDHI");
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

  if(!strcmp(sms.getDestinationSmeId(),"MAP_PROXY"))
  {
    try{
      int pres=partitionSms(&sms);
      if(pres==psErrorUdhi || pres==psErrorUdhi)
      {
        smsc_log_warn(smsLog,"REPLACE: concatenation failed(%d)",pres);
        __REPLACE__RESPONSE(REPLACEFAIL);
      }
      if(pres==psMultiple)
      {
        Address dst=sms.getDealiasedDestinationAddress();
        uint8_t msgref=smsc->getNextMR(dst);
        sms.setConcatMsgRef(msgref);
        sms.setConcatSeqNum(0);
      }
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"Exception in replace:%s",e.what());
      __REPLACE__RESPONSE(REPLACEFAIL);
    }
  }


  if(t.command->get_replaceSm().validityPeriod==-1)
  {
    sms.setLastResult(Status::INVEXPIRY);
    onSubmitFail(t.msgId,sms);
    __REPLACE__RESPONSE(INVEXPIRY);
  }
  if(t.command->get_replaceSm().scheduleDeliveryTime==-1)
  {
    sms.setLastResult(Status::INVSCHED);
    onSubmitFail(t.msgId,sms);
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
    onSubmitFail(t.msgId,sms);
    __REPLACE__RESPONSE(INVSCHED);
  }

  sms.setValidTime(newvalid);
  sms.setNextTime(newsched);
  sms.setIntProperty(Tag::SMPP_REGISTRED_DELIVERY,t.command->get_replaceSm().registeredDelivery);

  if(sms.hasIntProperty(Tag::SMPP_DATA_SM))sms.setIntProperty(Tag::SMPP_DATA_SM,0);

  try{
    store->replaceSms(t.msgId,sms);
    onSubmitOk(t.msgId,sms);
  }catch(...)
  {
    smsc_log_warn(smsLog,"REPLACE: replacefailed");
    //
    sms.setLastResult(Status::SYSERR);
    onSubmitFail(t.msgId,sms);
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
    smsc_log_warn(smsLog,"REPLACE: failed to put response command");
  }

#undef __REPLACE__RESPONSE
  return ENROUTE;
}

StateType StateMachine::query(Tuple& t)
{
  smsc_log_info(smsLog,"QUERY: Id=%lld",t.msgId);
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
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"QUERY: failed to send query response:%s",e.what());
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
  }catch(std::exception& e)
  {
    smsc_log_warn(smsLog,"QUERY: failed to send query response:%s",e.what());
  }
  return t.state;
}

StateType StateMachine::cancel(Tuple& t)
{
  SMS sms;
  try{
    Address addr;

    if(!t.command->get_cancelSm().force)
    {
      addr=Address(t.command->get_cancelSm().sourceAddr.get());
    }
    store->retriveSms(t.msgId,sms);
    smsc_log_info(smsLog,"CANCEL: Id=%lld;oa=%s;da=%s;srcSme=%s",t.msgId,
        sms.getOriginatingAddress().toString().c_str(),
        sms.getDestinationAddress().toString().c_str(),
        sms.getSourceSmeId()
    );

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
        smsc_log_warn(smsLog,"CANCEL: failed to send cancel response");
      }
    }
    smsc_log_warn(smsLog,"CANCEL: failed to cancel sms with Id=%lld:%s",t.msgId,e.what());
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
        if(sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnSubmit)
        {
          smsc_log_info(smsLog,"CANCEL: msgId=%lld, incomplete multipart sms enqueued in scheduler",t.msgId);
          sms.setIntProperty(Tag::SMSC_MERGE_CONCAT,3);
          bool ok=false;
          try{
            store->replaceSms(t.msgId,sms);
            ok=true;
          }catch(...)
          {
            Descriptor d;
            sms.setLastResult(Status::INCOMPLETECONCATMSG);
            store->changeSmsStateToUndeliverable(t.msgId,d,Status::INCOMPLETECONCATMSG);
            sendFailureReport(sms,t.msgId,UNDELIVERABLE,"");
          }
          if(ok)
          {
            sms.setNextTime(time(0));
            smsc->getScheduler()->RescheduleSms(t.msgId,sms,smsc->getSmeIndex(sms.getSourceSmeId()));
            return ENROUTE_STATE;
          }
        }else
        {
          Descriptor d;
          sms.setLastResult(Status::INCOMPLETECONCATMSG);
          store->changeSmsStateToUndeliverable(t.msgId,d,Status::INCOMPLETECONCATMSG);
          sendFailureReport(sms,t.msgId,UNDELIVERABLE,"");
        }
      }else
      {
        return t.state;
      }
    }
    onUndeliverable(t.msgId,sms);
  }catch(std::exception& e)
  {
    code=Status::CANCELFAIL;
    smsc_log_warn(smsLog,"CANCEL: failed to cancel sms with Id=%lld:'%s'",t.msgId,e.what());
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
      smsc_log_warn(smsLog,"CANCEL: failed to send cancel response: %s",e.what());
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

  unsigned char umrList[256];
  umrList[0]=sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
  time_t stimeList[256];
  stimeList[0]=sms.getSubmitTime();
  int umrListSize=0;
  if(sms.hasBinProperty(Tag::SMSC_ORGPARTS_INFO))
  {
    int cnt=getSMSPartsCount(sms);
    for(int i=0;i<cnt;i++)
    {
      SMSPartInfo spi=getSMSPartInfo(sms,i);
      if(spi.fl&SMSPartInfo::flHasSRR)
      {
        stimeList[umrListSize]=spi.stime?spi.stime:sms.getSubmitTime();
        umrList[umrListSize++]=spi.mr;
      }
    }
  }
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
  if(umrListSize)
  {
    rpt.setMessageReference(umrList[0]);
  }
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

  rpt.setIntProperty(Tag::SMSC_RECEIPTED_MSG_SUBMIT_TIME,(unsigned)stimeList[0]);

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
  time_t tz=common::TimeZoneManager::getInstance().getTimeZone(rpt.getDestinationAddress())+timezone;
  fd.submitDate=sms.getSubmitTime()+tz;
  fd.date=fd.submitDate;
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
  for(int i=1;i<umrListSize;i++)
  {
    rpt.setMessageReference(umrList[i]);
    rpt.setIntProperty(Tag::SMSC_RECEIPTED_MSG_SUBMIT_TIME,(unsigned)stimeList[i]);
    rpt.setIntProperty(Tag::SMSC_RECEIPT_MR,umrList[i]);
    submitReceipt(rpt,0x4);
  }
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
    smsc_log_debug(smsLog,"sendNotifyReport: Id=%lld, attemptsCount=%d",msgId,sms.getAttemptsCount());
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
    rpt.setIntProperty(Tag::SMSC_RECEIPTED_MSG_SUBMIT_TIME,(unsigned)sms.getSubmitTime());

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
    time_t tz=common::TimeZoneManager::getInstance().getTimeZone(rpt.getDestinationAddress())+timezone;
    fd.submitDate=sms.getSubmitTime()+tz;
    fd.date=fd.submitDate;
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
  }catch(std::exception& e)
  {
    smsc_log_warn(smsLog,"notify report failed:%s",e.what());
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
  smsc_log_debug(smsLog,"rescheduleSms: bt=%u ntt=%u",basetime,nextTryTime);
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
  try{
    if(failureCause==Status::RESCHEDULEDNOW)
    {
      smsc->getScheduler()->AddScheduledSms(id,sms,smsc->getSmeIndex(sms.dstSmeId));
    }else
    {
      time_t ntt=smsc->getScheduler()->RescheduleSms(id,sms,smsc->getSmeIndex(sms.dstSmeId));
      if(ntt)nextTryTime=ntt;
    }


    debug2(smsLog, "ENROUTE: Id=%lld;oa=%s;da=%s;lr=%d;or=%d;ntt=%u;ac=%d",id,
        sms.getOriginatingAddress().toString().c_str(),
        sms.getDestinationAddress().toString().c_str(),
        sms.getLastResult(),sms.oldResult,nextTryTime,sms.getAttemptsCount());
    store->changeSmsStateToEnroute(id,d,failureCause,nextTryTime,sms.getAttemptsCount()+(skipAttempt?0:1));
  }catch(std::exception& e)
  {
    warn2(smsLog,"Failed to change state to enroute:%s",e.what());
  }
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
    smsc::router::RouteResult rr;
    bool has_route=false;
    sms.setSourceSmeId(smscSmeId.c_str());
    try{
      has_route=smsc->routeSms(smsc->getSmeIndex(smscSmeId),sms.getOriginatingAddress(),sms.getDealiasedDestinationAddress(),rr);
    }catch(std::exception& e)
    {
      warn2(smsLog,"Routing %s->%s failed:%s",sms.getOriginatingAddress().toString().c_str(),
        sms.getDealiasedDestinationAddress().toString().c_str(),
        e.what());
    }


    if(has_route)
    {
      sms.setRouteId(rr.info.routeId.c_str());
      int prio=sms.getPriority()+rr.info.priority;
      if(prio>SmeProxyPriorityMax)prio=SmeProxyPriorityMax;
      sms.setPriority(prio);

      sms.setDestinationSmeId(rr.info.smeSystemId.c_str());
      sms.setServiceId(rr.info.serviceId);
      sms.setArchivationRequested(rr.info.archived);
      sms.setBillingRecord(rr.info.billing);

      sms.setEServiceType(serviceType.c_str());
      sms.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protocolId);
      sms.setIntProperty(Tag::SMSC_ORIGINALPARTSNUM,1);

      Profile profile=smsc->getProfiler()->lookup(dst);
      sms.setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage);
      sms.setIntProperty(Tag::SMSC_UDH_CONCAT,profile.udhconcat);

      if(rr.info.smeSystemId!="MAP_PROXY")
      {
        sms.setIntProperty(Tag::SMPP_ESM_CLASS,
          (
            sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0xC3 //11000011
          )|type);
      }

      int pres=psSingle;
      if(rr.info.smeSystemId=="MAP_PROXY")
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
        smsc_log_warn(smsLog,"Concatenation error %d for receipt %s->%s",pres,
          sms.getOriginatingAddress().toString().c_str(),
          sms.getDealiasedDestinationAddress().toString().c_str());
        return;
      }

      store->createSms(sms,msgId,smsc::store::CREATE_NEW);
      //smsc->getScheduler()->AddScheduledSms(sms.getNextTime(),msgId,sms.getDealiasedDestinationAddress(),dest_proxy_index);
      smsc->getScheduler()->AddFirstTimeForward(msgId,sms);
    }else
    {
      smsc_log_warn(smsLog,"There is no route for receipt %s->%s",
        sms.getOriginatingAddress().toString().c_str(),
        sms.getDealiasedDestinationAddress().toString().c_str());
    }
  }catch(std::exception& e)
  {
    smsc_log_warn(smsLog,"Failed to create receipt:%s",e.what());
  }
}

void StateMachine::submitResp(Tuple& t,SMS* sms,int status)
{
  sms->setLastResult(status);
  onSubmitFail(t.msgId,*sms);
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
    smsc_log_warn(smsLog,"SUBMIT_SM: failed to put response command");
  }
}

void StateMachine::finalizeSms(SMSId id,SMS& sms)
{
  if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2)//forward mode (transaction)
  {
    if(sms.lastResult!=0)
    {
      if(sms.hasIntProperty(Tag::SMPP_SET_DPF))
      {
        smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
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
        smsc_log_warn(smsLog,"DELIVERYRESP: failed to put transaction response command");
      }
    }
  }
  try{
    store->createFinalizedSms(id,sms);
  }catch(...)
  {
    smsc_log_warn(smsLog,"DELIVERYRESP: failed to finalize sms:Id=%lld",id);
  }
}

#ifdef SMSEXTRA
bool StateMachine::ExtraProcessing(SbmContext& c)
{
  bool toSmsx=c.rr.destProxy && !strcmp(c.rr.destProxy->getSystemId(),"smsx");
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
        c.has_route=smsc->routeSms(c.src_proxy->getSmeIndex(),c.sms->getOriginatingAddress(),c.dst,c.rr);
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
  c.rvstate=ENROUTE_STATE;
  smsc::util::findConcatInfo(body,mr,idx,num,havemoreudh);

  int dc=c.sms->getIntProperty(Tag::SMPP_DATA_CODING);
  //bool needrecoding=
  //    (dc==DataCoding::UCS2 && (profile.codepage&ProfileCharsetOptions::Ucs2)!=ProfileCharsetOptions::Ucs2) ||
  //    (dc==DataCoding::LATIN1 && (profile.codepage&ProfileCharsetOptions::Latin1)!=ProfileCharsetOptions::Latin1);

  if(firstPiece) //first piece
  {
    info2(smsLog, "merging sms Id=%lld;oa=%s;da=%s, first part (%u/%u),mr=%d,dc=%d,srr=%d",c.t.msgId,
        c.sms->getOriginatingAddress().toString().c_str(),
        c.sms->getDestinationAddress().toString().c_str(),
        idx,num,(int)mr,dc,c.sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST));
    c.sms->setIntProperty(Tag::SMPP_ESM_CLASS,c.sms->getIntProperty(Tag::SMPP_ESM_CLASS)&~0x40);
    TmpBuf<char,2048> tmp(0);
    if(!c.isForwardTo)
    {
      tmp.Append((const char*)body,len);
    }else
    {
      unsigned lenMo;
      const char *bodyMo=c.sms->getBinProperty(Tag::SMSC_MO_PDU,&lenMo);
      if(lenMo>512)
      {
        warn2(smsLog,"MO LEN=%d",lenMo);
      }
      tmp.Append(bodyMo,lenMo);
      c.sms->setIntProperty(Tag::SMPP_DATA_CODING,DataCoding::BINARY);
      dc=DataCoding::BINARY;
    }
    c.sms->setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.get(),(unsigned)tmp.GetPos());
    c.sms->getMessageBody().dropProperty(Tag::SMPP_SHORT_MESSAGE);
    c.sms->getMessageBody().dropProperty(Tag::SMSC_RAW_SHORTMESSAGE);
    c.sms->setIntProperty(Tag::SMPP_SM_LENGTH,0);
    c.sms->setIntProperty(Tag::SMSC_ORIGINALPARTSNUM,num);

    if(c.sms->hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM))
    {
      smsc_log_debug(smsLog,"droping sar fields");
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

    SMSPartInfo spi;
    spi.fl=SMSPartInfo::flPartPresent;
    if(c.sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
    {
      spi.fl|=SMSPartInfo::flHasSRR;
      spi.mr=c.sms->getMessageReference();
    }
    spi.dc=dc;
    spi.stime=(uint32_t)c.sms->getSubmitTime();
    fillSMSPartInfo(*c.sms,num,idx-1,spi);
    /*
    char dc_list[256];
    memset(dc_list,0,num);
    smsc_log_debug(smsLog,"dc_list[%d]=%d",idx-1,dc);
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
    */

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
        c.rr.info.smeSystemId.c_str()
      );
    }
    c.needToSendResp=false;
    c.createSms=scsCreate;
  }else
  {
    info2(smsLog, "merging sms Id=%lld;oa=%s;da=%s next part (%u/%u), mr=%d,dc=%d,srr=%d",c.t.msgId,
        c.sms->getOriginatingAddress().toString().c_str(),
        c.sms->getDestinationAddress().toString().c_str(),
        idx,num,(int)mr,dc,c.sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST));
    SMS newsms;
    try{
      store->retriveSms(c.t.msgId,newsms);
    }catch(...)
    {
      warn2(smsLog, "sms with Id=%lld not found or store error",c.t.msgId);
      submitResp(c.t,c.sms,Status::SYSERR);
      c.rvstate=ERROR_STATE;
      return false;
    }
    if(!newsms.hasIntProperty(Tag::SMSC_MERGE_CONCAT))
    {
      warn2(smsLog, "Id=%lld:one more part of concatenated message received, but all parts are collected.",c.t.msgId);
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
            smsc_log_warn(smsLog,"SUBMIT_SM: failed to put response command");
          }


          warn2(smsLog, "Duplicate or invalid concatenated message part for Id=%lld(idx:%d-%d,num:%d-%d,mr:%d-%d)",c.t.msgId,idx0,idx,num0,num,mr0,mr);
          c.rvstate=ERROR_STATE;
          return false;
        }
      }
    }else
    {
      if(newsms.hasBinProperty(Tag::SMSC_ORGPARTS_INFO))
      {
        int oldNum=getSMSPartsCount(newsms);
        if(oldNum!=num)
        {
          warn2(smsLog, "Id=%lld: different number of parts detected %d!=%d.",c.t.msgId,oldNum,num);
          submitResp(c.t,c.sms,Status::INVOPTPARAMVAL);
          c.rvstate=ERROR_STATE;
          return false;
        }
      }
      /*
      if(newsms.hasBinProperty(Tag::SMSC_DC_LIST))
      {
        unsigned dclen;
        newsms.getBinProperty(Tag::SMSC_DC_LIST,&dclen);
        if(dclen!=num)
        {
          warn2(smsLog, "Id=%lld: different number of parts detected %d!=%d.",c.t.msgId,dclen,num);
          submitResp(c.t,c.sms,Status::INVOPTPARAMVAL);
          c.rvstate=ERROR_STATE;
          return false;
        }
      }
      */
      body=(unsigned char*)c.sms->getBinProperty(Tag::SMSC_MO_PDU,&len);
      dc=DataCoding::BINARY;
    }

    {
      SMSPartInfo spi;
      spi.dc=dc;
      spi.stime=c.sms->getSubmitTime();
      if(c.sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
      {
        spi.fl|=SMSPartInfo::flHasSRR;
        spi.mr=c.sms->getMessageReference();
      }
      fillSMSPartInfo(newsms,num,idx-1,spi);
    }

    /*
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
        smsc_log_debug(smsLog,"dc_list[%d]=%d",idx-1,dc);
        dcList[idx-1]=dc;
        newsms.setBinProperty(Tag::SMSC_DC_LIST,(const char*)dcList,dclen);
      }
    }
    */

    if(c.sms->hasIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST) && c.sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST)!=0)
    {
      newsms.setIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST,1);
    }

    TmpBuf<char,2048> tmp(newlen+len);
    tmp.Append((const char*)newbody,newlen);
    tmp.Append((const char*)body,len);
    ci->setOff(ci->num,newlen);
    ci->num++;
    bool allParts=ci->num==num;
    if(allParts) // all parts received
    {
      // now resort parts

      debug2(smsLog,"all parts received, send kill cache item:Id=%lld;oa=%s;da=%s;mr=%d",c.t.msgId,c.sms->getOriginatingAddress().toString().c_str(),c.sms->getDestinationAddress().toString().c_str(),(int)mr);
      smsc->submitMrKill(c.sms->getOriginatingAddress(),c.sms->getDestinationAddress(),mr);

      vector<int> order;
      bool rightOrder=true;
      bool totalMoreUdh=false;
      bool differentDc=false;
      bool haveBinDc=c.isForwardTo?true:c.sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::BINARY;

      if(!c.isForwardTo)
      {
        /*
        unsigned char* dcList=0;
        unsigned dcListLen=0;

        if(newsms.hasBinProperty(Tag::SMSC_DC_LIST))
        {
          dcList=(unsigned char*)newsms.getBinProperty(Tag::SMSC_DC_LIST,&dcListLen);
        }
        */

        for(int i=0;i<ci->num;i++)
        {
          uint16_t mr0;
          uint8_t idx0,num0;
          bool havemoreudh0;
          smsc::util::findConcatInfo((unsigned char*)tmp.get()+ci->getOff(i),mr0,idx0,num0,havemoreudh0);
          smsc_log_debug(smsLog,"SUBMIT_SM: merge check order %d:%d",i,idx0);
          totalMoreUdh=totalMoreUdh || havemoreudh0;
          order.push_back(idx0);
          rightOrder=rightOrder && idx0==i+1;

          SMSPartInfo spi=getSMSPartInfo(newsms,i);
          if(i>0)
          {
            SMSPartInfo spi0=getSMSPartInfo(newsms,i-1);
            if(spi0.dc!=spi.dc)
            {
              differentDc=true;
            }
          }
          if(spi.dc==DataCoding::BINARY)
          {
            haveBinDc=true;
          }
          /*
          if(dcList)
          {
            if(i>0)
            {
              differentDc=differentDc || dcList[i-1]!=dcList[i];
            }
            haveBinDc=haveBinDc || dcList[i]==DataCoding::BINARY;
          }
          */
        }
        if(!rightOrder)
        {
          smsc_log_info(smsLog,"SUBMIT_SM: Id=%lld, not right order - need to reorder",c.t.msgId);
          //average number of parts is 2-3. so, don't f*ck mind with quick sort and so on.
          //maximum is 255.  65025 comparisons. not very good, but not so bad too.
          TmpBuf<char,2048> newtmp(0);
          uint16_t newci[256];
          SMSPartInfo spi[256];

          for(unsigned i=1;i<=num;i++)
          {
            for(unsigned j=0;j<num;j++)
            {
              if(order[j]==i)
              {
                spi[i-1]=getSMSPartInfo(newsms,i-1);
                int partlen=j==num-1?(int)tmp.GetPos()-ci->getOff(j):ci->getOff(j+1)-ci->getOff(j);
                newci[i-1]=(uint16_t)newtmp.GetPos();
                newtmp.Append(tmp.get()+ci->getOff(j),partlen);

              }
            }
          }
          //memcpy(ci->off,newci,ci->num*2);
          for(int i=0;i<ci->num;i++)
          {
            ci->setOff(i,newci[i]);
            fillSMSPartInfo(newsms,num,i,spi[i]);
          }
          tmp.SetPos(0);
          tmp.Append(newtmp.get(),newtmp.GetPos());
        }
      }//isForwardTo
      newsms.setIntProperty(Tag::SMSC_MERGE_CONCAT,3); // final state
      if(!totalMoreUdh && !differentDc && !haveBinDc)//make single text message
      {
        TmpBuf<char,2048> newtmp(0);
        for(int i=1;i<=ci->num;i++)
        {
          int partlen=i==num?(int)tmp.GetPos()-ci->getOff(i-1):ci->getOff(i)-ci->getOff(i-1);
          const unsigned char * part=(const unsigned char *)tmp.get()+ci->getOff(i-1);
          partlen-=*part+1;
          part+=*part+1;
          newtmp.Append((const char*)part,partlen);
        }
        tmp.SetPos(0);
        tmp.Append(newtmp.get(),newtmp.GetPos());
        newsms.messageBody.dropProperty(Tag::SMSC_CONCATINFO);
        newsms.messageBody.dropIntProperty(Tag::SMSC_MERGE_CONCAT);
        newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.get(),(int)tmp.GetPos());
        try{
          processDirectives(newsms,c.profile,c.srcprof);
        }catch(...)
        {
          warn2(smsLog, "Failed to process directives for sms with Id=%lld",c.t.msgId);
          submitResp(c.t,&newsms,Status::SUBMITFAIL);
          c.rvstate=ERROR_STATE;
          return false;
        }
        /*
        if(c.rr.info.smeSystemId=="MAP_PROXY")
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
        newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.get(),(int)tmp.GetPos());
        if(!c.isForwardTo)
        {
          newsms.setIntProperty(Tag::SMPP_ESM_CLASS,newsms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x40);
        }
      }
    }else
    {
      newsms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,tmp.get(),(int)tmp.GetPos());
    }
    try{
      store->replaceSms(c.t.msgId,newsms);
    }catch(...)
    {
      warn2(smsLog, "Failed to replace sms with Id=%lld",c.t.msgId);
      submitResp(c.t,&newsms,Status::SUBMITFAIL);
      c.rvstate=ERROR_STATE;
      return false;
    }
    if(!allParts)
    {
      info2(smsLog, "merging sms Id=%lld;oa=%s;da=%s not all parts are here, waiting",
          c.t.msgId,
          c.sms->getOriginatingAddress().toString().c_str(),
          c.sms->getDestinationAddress().toString().c_str());
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
          c.rr.info.smeSystemId.c_str()
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
        smsc_log_warn(smsLog,"Failed to delete sms with msgId=%lld denied by EXTRA from store",c.t.msgId);
      }
      c.rvstate=ERROR_STATE;
      return false;
    }
#endif

  }
  return true;
}

void StateMachine::onSubmitOk(SMSId id,SMS& sms)
{
  if(!sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT) || sms.getIntProperty(Tag::SMSC_MERGE_CONCAT)==3)
  {
    smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
  }else
  {
    smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms,true);
  }
#ifdef SNMP
  SnmpCounter::getInstance().incCounter(SnmpCounter::cntAccepted,sms.getSourceSmeId());
#endif
}

void StateMachine::onSubmitFail(SMSId id,SMS& sms)
{
  smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
#ifdef SNMP
  SnmpCounter::getInstance().incCounter(SnmpCounter::cntRejected,sms.getSourceSmeId());
#endif
}

void StateMachine::onForwardOk(SMSId id,SMS& sms)
{
  smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
#ifdef SNMP
  SnmpCounter::getInstance().incCounter(SnmpCounter::cntRetried,sms.getDestinationSmeId());
#endif
}

void StateMachine::onDeliveryOk(SMSId id,SMS& sms)
{
  bool msuOnly=false;
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    ConcatInfo* ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,0);
    if(sms.getConcatSeqNum()<ci->num-1)
    {
      msuOnly=true;
    }
  }
  smsc->registerStatisticalEvent(StatEvents::etDeliveredOk,&sms,msuOnly);
#ifdef SNMP
  SnmpCounter::getInstance().incCounter(SnmpCounter::cntDelivered,sms.getDestinationSmeId());
#endif
}

void StateMachine::onDeliveryFail(SMSId id,SMS& sms)
{
  smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
#ifdef SNMP
  if(Status::isErrorPermanent(sms.getLastResult()))
  {
    SnmpCounter::getInstance().incCounter(SnmpCounter::cntFailed,sms.getDestinationSmeId());
  }else
  {
    incSnmpCounterForError(sms.getLastResult(),sms.getDestinationSmeId());
  }
#endif
}

void StateMachine::onUndeliverable(SMSId id,SMS& sms)
{
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    unsigned len;
    ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
    int i=sms.getConcatSeqNum();
    int mx=ci->num;
    for(;i<mx;i++)
    {
      smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms,i!=mx-1);
#ifdef SNMP
      SnmpCounter::getInstance().incCounter(SnmpCounter::cntFailed,sms.getDestinationSmeId());
#endif
    }
  }else
  {
    smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
#ifdef SNMP
    SnmpCounter::getInstance().incCounter(SnmpCounter::cntFailed,sms.getDestinationSmeId());
#endif
  }
}

}//system
}//smsc
