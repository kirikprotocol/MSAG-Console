#include "util/debug.h"
#include "system/smsc.hpp"
#include "system/state_machine.hpp"
#include <exception>
#include "system/rescheduler.hpp"
#include "profiler/profiler.hpp"
#include "util/recoder/recode_dll.h"
#include "core/buffers/Hash.hpp"
#include "util/smstext.h"
#include "logger/Logger.h"
#include "util/regexp/RegExp.hpp"
#include "core/synchronization/Mutex.hpp"
#include "system/status.h"
#include "resourcemanager/ResourceManager.hpp"

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

void StateMachine::formatDeliver(const FormatData& fd,std::string& out)
{
  ReceiptGetAdapter ga;
  ContextEnvironment ce;
  ce.exportStr("dest",fd.addr);
  ce.exportDat("date",fd.date);
  ce.exportStr("msgId",fd.msgId);
  ce.exportInt("lastResult",fd.lastResult);
  ce.exportInt("lastResultGsm",fd.lastResultGsm);
  ce.exportStr("msc",fd.msc);

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
      __warning2__("StateMachine::unknown exception");
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
      smsc::util::Logger::getCategory("smsc.system.StateMachine").
        error("Failed to compile address range regexp");
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
  Directive(int st,int len):start(st),end(st+len){}
  Directive(const Directive& d):start(d.start),end(d.end){}
};


void StateMachine::processDirectives(SMS& sms,Profile& p,Profile& srcprof)
{
  const char *body="";
  unsigned int len=0;
  int dc=sms.getIntProperty(Tag::SMPP_DATA_CODING);
  bool udhi=(sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)==0x40;
  int udhiLen=0;
  if(sms.getIntProperty(Tag::SMPP_SM_LENGTH))
  {
    body=sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  }else
  {
    if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      body=sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
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
  static const char *escchars="[]{}^~\\|";
  auto_ptr<char> bufptr(new char[len+1]);
  char *buf=bufptr.get();

  len=getSmsText(&sms,buf,len+1);

  Array<Directive> offsets;
  int i=0;
  RegExp def("/#def\\s+(\\d+)#/i");
  __require__(def.LastError()==regexp::errNone);
  RegExp tmpl("/#template=(.*?)#/i");
  __require__(tmpl.LastError()==regexp::errNone);
  RegExp tmplparam("/\\s*\\{(\\w+)\\}=(\".*?\"|[^{\\s]+)/");
  __require__(tmplparam.LastError()==regexp::errNone);
  RegExp unkdir("/#.*?#/");
  __require__(unkdir.LastError()==regexp::errNone);
  SMatch m[10];

  ContextEnvironment ce;
  ReceiptGetAdapter ga;

  string tmplname;

  while(i<len && buf[i]=='#')
  {
    int n=10;
    if(!strncasecmp(buf+i,"#ack#",5))
    {
      __trace__("DIRECT: ack found");
      sms.setDeliveryReport(REPORT_ACK);
      Directive d(i,5);
      offsets.Push(d);
      i+=5;
    }else
    if(!strncasecmp(buf+i,"#noack#",7))
    {
      __trace__("DIRECT: noack found");
      sms.setDeliveryReport(REPORT_NOACK);
      Directive d(i,7);
      offsets.Push(d);
      i+=7;
    }else
    if(!strncasecmp(buf+i,"#hide#",6))
    {
      __trace__("DIRECT: hide");
      if(srcprof.hideModifiable)
        sms.setIntProperty(Tag::SMSC_HIDE,1);
      else
      {
        __trace__("DIRECT: error, hide is not modifiable");
      }
      Directive d(i,6);
      offsets.Push(d);
      i+=6;
    }else
    if(!strncasecmp(buf+i,"#unhide#",8))
    {
      __trace__("DIRECT: unhide");
      if(srcprof.hideModifiable)
        sms.setIntProperty(Tag::SMSC_HIDE,0);
      else
      {
        __trace__("DIRECT: error, hide is not modifiable");
      }
      Directive d(i,8);
      offsets.Push(d);
      i+=8;
    }else
    if(!strncasecmp(buf+i,"#flash#",7))
    {
      if(!sms.hasIntProperty(Tag::SMSC_FORCE_DC))
      {
        sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,1);
      }
      Directive d(i,7);
      offsets.Push(d);
      i+=7;
    }else
    if(def.MatchEx(buf,buf+i,buf+len,m,n))
    {
      int t=atoi(buf+m[1].start);
      if(t>999)t=999;
      __trace2__("DIRECT: %*s, t=%d",m[0].end-m[0].start,buf+m[0].start,t);
      sms.setNextTime(time(NULL)+t*60*60);
      Directive d(i,m[0].end-m[0].start);
      offsets.Push(d);
      i+=m[0].end-m[0].start;
    }else
    if(tmpl.MatchEx(buf,buf+i,buf+len,m,n))
    {
      tmplname.assign(buf+m[1].start,m[1].end-m[1].start);
      __trace2__("DIRECT: template=%s",tmplname.c_str());
      tmplname="templates."+tmplname;
      OutputFormatter *f=ResourceManager::getInstance()->getFormatter(p.locale,tmplname);
      int j=m[0].end;
      n=10;
      string name,value;
      while(tmplparam.MatchEx(buf,buf+j,buf+len,m,n))
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
      Directive d(i,j-i);
      offsets.Push(d);
      i=j;
    }else
    if(unkdir.MatchEx(buf,buf+i,buf+len,m,n))
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
      throw Exception("Unknown template name");
    }
    f->format(newtext,ga,ce);
    udhi=false;
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x40));
  }

  auto_ptr<char> newBodyPtr(new char[olen*3+newtext.length()*3]);
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
        auto_ptr<char> x(new char[newlen-udhiLen+1]);
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
        auto_ptr<char> nt(new char[newtext.length()*2]);
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
  __require__(t.state==UNKNOWN_STATE);

  SmeProxy *src_proxy,*dest_proxy=0;

  src_proxy=t.command.getProxy();

  __require__(src_proxy!=NULL);

  SMS* sms = t.command->get_sms();
  uint32_t dialogId =  t.command->get_dialogId();

  smsLog->debug("SBM: Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
    t.msgId,dialogId,
    sms->getOriginatingAddress().toString().c_str(),
    sms->getDestinationAddress().toString().c_str(),
    src_proxy->getSystemId()
  );


  ////
  //
  //  SMS validity checks started
  //

  if(sms->getNextTime()==-1)
  {
    sms->setLastResult(Status::INVSCHED);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVSCHED,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to put response command");
    }
    smsLog->warn("SBM: invalid schedule time Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) &&
     sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE)   &&
     sms->getIntProperty(Tag::SMPP_SM_LENGTH)!=0)
  {
    sms->setLastResult(Status::SUBMITFAIL);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::SUBMITFAIL,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to put response command");
    }
    smsLog->warn("SBM: both short_message and payload present Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
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
    sms->setLastResult(Status::INVDCS);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                       (
                         /*messageId*/"0",
                         dialogId,
                         Status::INVDCS,
                         sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                       );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to put response command");
    }
    smsLog->warn("SBM: invalid datacoding %d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING),
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3 &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
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
      sms->setLastResult(Status::INVMSGLEN);
      smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
      SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVMSGLEN,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
      try{
        src_proxy->putCommand(resp);
      }catch(...)
      {
      }
      smsLog->warn("SBM: invalid message length for unicode (%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
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
    sms->setLastResult(Status::INVEXPIRY);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVEXPIRY,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    smsLog->warn("SBM: invalid valid time Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
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
    sms->setLastResult(Status::INVSRCADR);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVSRCADR,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to put response command");
    }
    smsLog->warn("SBM: invalid source address for range '%s' Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      src_proxy->getSourceAddressRange().c_str(),
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
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
  smsc::router::RouteInfo ri;

  ////
  //
  //  Routing here
  //

  bool has_route = smsc->routeSms(sms->getOriginatingAddress(),
                          dst,
                          dest_proxy_index,dest_proxy,&ri,src_proxy->getSmeIndex());

  sms->setRouteId(ri.routeId.c_str());
  if(ri.suppressDeliveryReports)sms->setIntProperty(Tag::SMSC_SUPPRESS_REPORTS,1);
  int prio=sms->getPriority()+ri.priority;
  if(prio>SmeProxyPriorityMax)prio=SmeProxyPriorityMax;
  sms->setPriority(prio);

  //smsc->routeSms(sms,dest_proxy_index,dest_proxy);
  if ( !has_route )
  {
    sms->setLastResult(Status::NOROUTE);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    //send_no_route;
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::NOROUTE,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT: failed to put response command");
    }
    char buf1[32];
    char buf2[32];
    sms->getOriginatingAddress().toString(buf1,sizeof(buf2));
    dst.toString(buf2,sizeof(buf2));
    smsLog->warn("SBM: no route Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  //__trace2__("SUBMIT_SM: route found, routeId=%s, smeSystemId=%s",ri.routeId.c_str(),ri.smeSystemId.c_str());

  sms->setSourceSmeId(t.command->get_sourceId());

  sms->setDestinationSmeId(ri.smeSystemId.c_str());
  sms->setServiceId(ri.serviceId);


  __trace2__("SUBMIT: archivation request for %lld/%d is %s",t.msgId,dialogId,ri.archived?"true":"false");
  sms->setArchivationRequested(ri.archived);
  sms->setBillingRecord(ri.billing);


  sms->setIntProperty(Tag::SMSC_HIDE,profile.hide);
  Profile srcprof=profile;
  profile=smsc->getProfiler()->lookup(dst);
  sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage);
  int pres=psSingle;


  ////
  //
  //  Directives
  //

  try{
    processDirectives(*sms,profile,srcprof);
  }catch(std::exception& e)
  {
    __warning2__("Failed to process directives due to exception:%s",e.what());
    sms->setLastResult(Status::SUBMITFAIL);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::SUBMITFAIL,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT: failed to put response command");
    }
    return ERROR_STATE;
  }

  if(sms->getValidTime()==0 || sms->getValidTime()>now+maxValidTime)
  {
    sms->setValidTime(now+maxValidTime);
    __trace2__("maxValidTime=%d",maxValidTime);
  }

  __trace2__("Valid time for sms %lld=%u",t.msgId,(unsigned int)sms->getValidTime());



  if(sms->getNextTime()>now+maxValidTime || sms->getNextTime()>sms->getValidTime())
  {
    sms->setLastResult(Status::INVSCHED);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVSCHED,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT: failed to put response command");
    }
    smsLog->warn("SBM: invalid schedule time(%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      sms->getNextTime(),
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  __trace2__("SUBMIT_SM: after processDirectives - delrep=%d, sdt=%d",(int)sms->getDeliveryReport(),sms->getNextTime());

  __trace2__("SUBMIT_SM: dest_addr_subunit=%d",sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT));

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
      sms->setLastResult(Status::INVMSGLEN);
      smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
      SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVMSGLEN,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
      try{
        src_proxy->putCommand(resp);
      }catch(...)
      {
      }
      smsLog->warn("SBM: invalid message length(%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
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

  if(ri.smeSystemId=="MAP_PROXY")
  {
    pres=partitionSms(sms,profile.codepage);
  }
  if(pres==psErrorLength)
  {
    sms->setLastResult(Status::INVMSGLEN);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                       (
                         /*messageId*/"0",
                         dialogId,
                         Status::INVMSGLEN,
                         sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                       );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT:Failed to put command");
    }
    unsigned int len;
    const char *msg=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    smsLog->warn("SBM: invalid message length(%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
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
    sms->setLastResult(Status::SUBMITFAIL);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::SUBMITFAIL,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to put response command");
    }
    smsLog->warn("SBM: udhi present in concatenated message!!! Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
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
    sms->setLastResult(Status::USSDMSGTOOLONG);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::USSDMSGTOOLONG,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to put response command");
    }
    smsLog->warn("SBM: ussd message too long!!! Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      ri.smeSystemId.c_str()
    );
    return ERROR_STATE;
  }


  if(pres==psMultiple)
  {
    uint8_t msgref=smsc->getNextMR(dst);
    sms->setConcatMsgRef(msgref);
    sms->setConcatSeqNum(0);
  }

  __trace2__("SUBMIT_SM: Replace if present for message %lld=%d",t.msgId,sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG));
  __trace2__("SUBMIT_SM: SMPP_USSD_SERVICE_OP for %lld=%d",t.msgId,sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP));

  time_t stime=sms->getNextTime();

  //
  // End of checks. Ready to put sms to database
  //
  ////

  ////
  //
  // Traffic Control
  //

  if(!smsc->allowCommandProcessing(t.command))
  {
    sms->setLastResult(Status::THROTTLED);
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::THROTTLED,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to put response command");
    }
    smsLog->warn("SBM: traffic control denied message Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
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
  if(!isDatagram && !isTransaction)
  {
    try{
      if(sms->getNextTime()<now)
      {
        sms->setNextTime(now);
      }
      store->createSms(*sms,t.msgId,
        sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG)?smsc::store::SMPP_OVERWRITE_IF_PRESENT:smsc::store::CREATE_NEW);
    }catch(...)
    {
      __warning2__("failed to create sms with id %lld",t.msgId);
      sms->setLastResult(Status::SYSERR);
      smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
      SmscCommand resp = SmscCommand::makeSubmitSmResp
                           (
                             /*messageId*/"0",
                             dialogId,
                             Status::SYSERR,
                             sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                           );
      try{
        src_proxy->putCommand(resp);
      }catch(...)
      {
        __trace__("SUBMIT: failed to put response command");
      }
      return ERROR_STATE;
    }
  }

  //
  // stored
  //
  ////

  smsLog->debug("SBM: sms created Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
    t.msgId,dialogId,
    sms->getOriginatingAddress().toString().c_str(),
    sms->getDestinationAddress().toString().c_str(),
    src_proxy->getSystemId(),
    ri.smeSystemId.c_str()
  );


  if(!isDatagram && !isTransaction) // Store&Forward mode
  {

    smsc->registerStatisticalEvent(StatEvents::etSubmitOk,sms);

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
      smsLog->warn("SBM: failed to put response command SUBMIT_OK Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId(),
        ri.smeSystemId.c_str()
      );
    }
  }

  __trace2__("Sms scheduled to %d, now %d",(int)sms->getNextTime(),(int)now);
  if(!isDatagram && !isTransaction && stime>now)
  {
    smsc->ChangeSmsSchedule(t.msgId,stime,dest_proxy_index);
    smsc->notifyScheduler();
    return ENROUTE_STATE;
  }

  sms->dialogId=dialogId;


  // ���� ��� ��������� ������ ��� Datagram � Transction �������.
  // �� ���� �� ������ �� submit ������� submit response, ��������� ��� ���...
  // �� ���� ������� ����� ������� �� ������������ � ������,
  // � sms ���������� setLastError()
  struct ResponseGuard{
    SMS *sms;
    SmeProxy* prx;
    StateMachine* sm;
    /*ResponseGuard():sms(0),prx(0),sm(0){}
    ResponseGuard(const ResponseGuard& rg)
    {
      sms=rg.sms;
      prx=rg.prx;
      sm=rg.sm;
    }*/
    ResponseGuard(SMS* s,SmeProxy* p,StateMachine *st):sms(s),prx(p),sm(st){}
    ~ResponseGuard()
    {
      if(!sms)return;
      if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0 ||
         (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x3)return;
      if(sms->lastResult!=Status::OK)
      {
        sm->smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
      }else
      {
        sm->smsc->registerStatisticalEvent(StatEvents::etSubmitOk,sms);
      }
      if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x1 ||
         ((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2 && sms->lastResult!=Status::OK))
      {
        SmscCommand resp = SmscCommand::makeSubmitSmResp
                             (
                               /*messageId*/"0",
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
  };
  ResponseGuard respguard(sms,src_proxy,this);

  if ( !dest_proxy )
  {
    sms->setLastResult(Status::SMENOTCONNECTED);
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,sms);
    smsLog->debug("SBM: dest sme not connected Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
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
      smsc->notifyScheduler();
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }
  // create task
  uint32_t dialogId2;
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
      smsc->notifyScheduler();
    }catch(...)
    {
      __trace__("SUBMIT_SM: failed to change state to enroute");
    }
    smsLog->warn("SBM: failed to get seq number Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      ri.smeSystemId.c_str()
    );
    sendNotifyReport(*sms,t.msgId,"destination unavailable");
    return ENROUTE_STATE;
  }
  smsLog->debug("SBM: seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
    dialogId2,
    t.msgId,dialogId,
    sms->getOriginatingAddress().toString().c_str(),
    sms->getDestinationAddress().toString().c_str(),
    src_proxy->getSystemId(),
    ri.smeSystemId.c_str()
  );
  //Task task((uint32_t)dest_proxy_index,dialogId2);
  try{
  Task task(dest_proxy->getUniqueId(),dialogId2,isDatagram || isTransaction?sms:0);
  __trace2__("SUBMIT: task.sms=%p",task.sms);
  task.messageId=t.msgId;
  if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
  {
    sms->setLastResult(Status::SYSERR);
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("SUBMIT_SM: change state to enroute");
      changeSmsStateToEnroute(*sms,t.msgId,d,Status::SYSERR,rescheduleSms(*sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to change state to enroute");
    }

    smsLog->warn("SBMDLV: failed to create task, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
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
  if(isDatagram || isTransaction)
  {
    t.command->get_sms_and_forget();
  }
  }catch(...)
  {
    sms->setLastResult(Status::SYSERR);
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("SUBMIT_SM: change state to enroute");
      changeSmsStateToEnroute(*sms,t.msgId,d,Status::SYSERR,rescheduleSms(*sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to change state to enroute");
    }
    smsLog->warn("SBMDLV: failed to create task, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
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
  try{
    // send delivery
    Address src;
    __trace2__("SUBMIT: wantAlias=%s, hide=%s",smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias?"true":"false",sms->getIntProperty(Tag::SMSC_HIDE)?"true":"false");
    if(
        smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias &&
        sms->getIntProperty(Tag::SMSC_HIDE) &&
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
    }else
    {
      extractSmsPart(sms,0);
    }

    SmscCommand delivery = SmscCommand::makeDeliverySm(*sms,dialogId2);
    unsigned bodyLen=0;
    delivery->get_sms()->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&bodyLen);
    __trace2__("SUBMIT: delivery.sms.sm_length=%d",bodyLen);
    int prio=ri.priority/1000;
    if(prio<0)prio=0;
    if(prio>=32)prio=31;
    delivery->set_priority(prio);
    try{
      dest_proxy->putCommand(delivery);
    }catch(InvalidProxyCommandException& e)
    {
      sms->setOriginatingAddress(srcOriginal);
      sms->setDestinationAddress(dstOriginal);
      sms->setLastResult(Status::INVBNDSTS);
      sendNotifyReport(*sms,t.msgId,"service rejected");
      if(!isDatagram && !isTransaction)
      {
        try{
          Descriptor d;
          changeSmsStateToEnroute(*sms,t.msgId,d,Status::INVBNDSTS,rescheduleSms(*sms));
          smsc->notifyScheduler();
        }catch(...)
        {
          __warning__("SUBMIT: failed to change state to enroute");
        }
      }
      smsLog->warn("SBMDLV: Attempt to putCommand for sme in invalid bind state, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        dialogId2,
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId(),
        ri.smeSystemId.c_str()
      );
      Task t;
      smsc->tasks.findAndRemoveTask(dest_proxy->getUniqueId(),dialogId2,&t);

      return ENROUTE_STATE;
    }
  }catch(exception& e)
  {
    smsLog->warn("SBMDLV: failed to put delivery command, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      dialogId2,
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      ri.smeSystemId.c_str()
    );
    sms->setOriginatingAddress(srcOriginal);
    sms->setDestinationAddress(dstOriginal);
    sms->setLastResult(Status::THROTTLED);
    sendNotifyReport(*sms,t.msgId,"system failure");
    if(!isDatagram && !isTransaction)
    {
      try{
        Descriptor d;
        changeSmsStateToEnroute(*sms,t.msgId,d,Status::THROTTLED,rescheduleSms(*sms));
        smsc->notifyScheduler();
      }catch(...)
      {
        __warning__("SUBMIT: failed to change state to enroute");
      }
    }
    Task t;
    smsc->tasks.findAndRemoveTask(dest_proxy->getUniqueId(),dialogId2,&t);
    return ENROUTE_STATE;
  }catch(...)
  {
    smsLog->warn("SBMDLV: failed to put delivery command, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      dialogId2,
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      ri.smeSystemId.c_str()
    );
    sms->setOriginatingAddress(srcOriginal);
    sms->setDestinationAddress(dstOriginal);
    sms->setLastResult(Status::THROTTLED);
    sendNotifyReport(*sms,t.msgId,"system failure");
    if(!isDatagram && !isTransaction)
    {
      try{
        Descriptor d;
        changeSmsStateToEnroute(*sms,t.msgId,d,Status::THROTTLED,rescheduleSms(*sms));
        smsc->notifyScheduler();
      }catch(...)
      {
        __warning__("SUBMIT: failed to change state to enroute");
      }
    }
    Task t;
    smsc->tasks.findAndRemoveTask(dest_proxy->getUniqueId(),dialogId2,&t);
    return ENROUTE_STATE;
  }
  if(isDatagram || isTransaction)
  {
    t.command->get_sms_and_forget();
  }
  sms->lastResult=Status::OK;
  smsLog->debug("SBM: submit ok, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
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
  SMS sms;
  smsLog->debug("FWD: msgId=%lld",t.msgId);
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    smsLog->warn("FWD: failed to retriveSms %lld",t.msgId);
    return UNKNOWN_STATE;
  }
  if(sms.getState()==EXPIRED_STATE)
  {
    smsLog->warn("FWD: sms in expired state msgId=%lld",t.msgId);
    return EXPIRED_STATE;
  }
  time_t now=time(NULL);
  if(sms.getValidTime()<=now && sms.getAttemptsCount()!=0)
  {
    sms.setLastResult(Status::EXPIRED);
    smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
    smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
    try{
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      __warning__("FORWARD: failed to change state to expired");
    }
    smsLog->debug("FWD: %lld expired (valid:%u - now:%u)",t.msgId,sms.getValidTime(),now);

    sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");

    return EXPIRED_STATE;
  }

  if(sms.getState()!=ENROUTE_STATE)
  {
    smsLog->warn("FWD: sms msgId=%lld is not in enroute (%d)",t.msgId,sms.getState());
    return sms.getState();
  }
  if(!t.command->is_reschedulingForward() && sms.getNextTime()>now)
  {
    smsLog->debug("FWD: nextTime>now (%d>%d)",sms.getNextTime(),now);
    SmeIndex idx=smsc->getSmeIndex(sms.dstSmeId);
    smsc->ChangeSmsSchedule(t.msgId,sms.getNextTime(),idx);
    return sms.getState();
  }


  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    Descriptor d;
    try{
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      __trace__("FORWARD: Failed to change state of USSD request to undeliverable");
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
      __trace2__("FORWARD: Failed to send AlertNotification:unknown");
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

  smsc::router::RouteInfo ri;
  bool has_route = smsc->routeSms
                    (
                      sms.getOriginatingAddress(),
                      sms.getDealiasedDestinationAddress(),
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
    smsLog->warn("FWD: msgId=%lld, No route (%s->%s)",t.msgId,from,to);
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
      smsc->notifyScheduler();
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
    smsLog->warn("FWD: msgId=%lld sme is not connected(%s->%s(%s))",t.msgId,bufsrc,bufdst,ri.smeSystemId.c_str());
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
      smsc->notifyScheduler();
    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }
  // create task

  uint32_t dialogId2;
  try{
    dialogId2 = dest_proxy->getNextSequenceNumber();
    smsLog->debug("FWDDLV: msgId=%lld, seq number:%d",t.msgId,dialogId2);
    //Task task((uint32_t)dest_proxy_index,dialogId2);
    Task task(dest_proxy->getUniqueId(),dialogId2);
    task.messageId=t.msgId;
    if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
    {
      smsLog->warn("FWDDLV: failed to create task msgId=%lld, seq number:%d",t.msgId,dialogId2);
      try{
        sms.setLastResult(Status::SYSERR);
        sendNotifyReport(sms,t.msgId,"destination unavailable");
      }catch(...)
      {
        __warning__("FORWARD: failed to send intermediate notification");
      }
      try{
        Descriptor d;
        __trace__("FORWARD: change state to enroute");
        sms.setLastResult(Status::SYSERR);
        changeSmsStateToEnroute(sms,t.msgId,d,Status::SYSERR,rescheduleSms(sms));
        smsc->notifyScheduler();
      }catch(...)
      {
        __warning__("FORWARD: failed to change state to enroute");
      }
      return ENROUTE_STATE;
    }
  }catch(...)
  {
    smsLog->warn("FWDDLV: failed to get seqnum msgId=%lld",t.msgId,dialogId2);
    try{
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      sms.setLastResult(Status::SMENOTCONNECTED);
      changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));
      smsc->notifyScheduler();
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
        smsc->AddressToAlias(sms.getOriginatingAddress(),src)
      )
    {
      sms.setOriginatingAddress(src);
    }
    Address dst=sms.getDealiasedDestinationAddress();
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
        }
      }
    }else
    {
      unsigned int len;
      ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
      if(sms.getConcatSeqNum()<ci->num)
      {
        extractSmsPart(&sms,sms.getConcatSeqNum());
      }else
      {
        __warning__("attempt to forward concatenated message but all parts are delivered!!!");
        try{
          Descriptor d;
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
    SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
    dest_proxy->putCommand(delivery);
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
    smsLog->warn("FWDDLV: failed create deliver(%s) msgId=%lld, seq number:%d",errtext,t.msgId,dialogId2);

    sms.setOriginatingAddress(srcOriginal);
    sms.setDestinationAddress(dstOriginal);
    sms.setLastResult(errstatus);
    sendNotifyReport(sms,t.msgId,errtext);
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      changeSmsStateToEnroute(sms,t.msgId,d,errstatus,rescheduleSms(sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    Task t;
    smsc->tasks.findAndRemoveTask(dest_proxy->getUniqueId(),dialogId2,&t);
    return ENROUTE_STATE;
  }
  smsLog->debug("FWDDLV: deliver ok msgId=%lld, seq number:%d",t.msgId,dialogId2);

  return DELIVERING_STATE;
}

StateType StateMachine::deliveryResp(Tuple& t)
{
  smsLog->debug("DLVRSP: msgId=%lld;st=%d",t.msgId,t.command->get_resp()->get_status());
  //__require__(t.state==DELIVERING_STATE);
  if(t.state!=DELIVERING_STATE)
  {
    smsLog->debug("DLVRSP: state of SMS isn't DELIVERING!!! msgId=%lld;st=%d",t.msgId,t.command->get_resp()->get_status());
    return t.state;
  }
  //smsc::sms::Descriptor d;

  ////
  //
  // register deliveryresp in traffic control
  //

  smsc->allowCommandProcessing(t.command);

  //
  // end of traffic control code
  //
  ////

  SMS sms;
  bool dgortr=t.command->get_resp()->get_sms()!=0; //datagram or transaction
  if(dgortr)
  {
    sms=*t.command->get_resp()->get_sms();
  }else
  {
    try{
      store->retriveSms((SMSId)t.msgId,sms);
    }catch(exception& e)
    {
      smsLog->warn("DLVRSP: failed to retrieve sms:%s! msgId=%lld;st=%d",e.what(),t.msgId,t.command->get_resp()->get_status());
      return UNKNOWN_STATE;
    }
  }
  sms.destinationDescriptor=t.command->get_resp()->getDescriptor();
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
        smsc->notifyScheduler();
        smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2)
          {
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
                __warning2__("DELIVERYRESP: failed to put transaction response command");
              }
            }
          }
          try{
            store->createFinalizedSms(t.msgId,sms);
          }catch(...)
          {
            __warning2__("DELIVERYRESP: failed to finalize sms:%lld",t.msgId);
          }
          return UNDELIVERABLE_STATE;
        }
        return UNKNOWN_STATE;
      }break;
      case CMD_ERR_TEMP:
      {
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
        smsc->notifyScheduler();
        sendNotifyReport(sms,t.msgId,"subscriber busy");
        smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2)
          {
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
                __warning2__("DELIVERYRESP: failed to put transaction response command");
              }
            }
          }
          try{
            store->createFinalizedSms(t.msgId,sms);
          }catch(...)
          {
            __warning2__("DELIVERYRESP: failed to finalize sms:%lld",t.msgId);
          }
          return UNDELIVERABLE_STATE;
        }
        return UNKNOWN_STATE;
      }break;
      default:
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
          __warning2__("DELIVERYRESP: failed to change state to enroute:%s",e.what());
        }

        sms.setLastResult(GET_STATUS_CODE(t.command->get_resp()->get_status()));
        sendFailureReport(sms,t.msgId,UNDELIVERABLE_STATE,"permanent error");
        smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2)
          {
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
                __warning2__("DELIVERYRESP: failed to put transaction response command");
              }
            }
          }
          try{
            store->createFinalizedSms(t.msgId,sms);
          }catch(...)
          {
            __warning2__("DELIVERYRESP: failed to finalize sms:%lld",t.msgId);
          }
        }
        return UNDELIVERABLE_STATE;
      }
    }
  }

  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    smsLog->debug("DLVRSP: sms has concatinfo, csn=%d;msgId=%lld",sms.getConcatSeqNum(),t.msgId);
    unsigned int len;
    ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
    if(sms.getConcatSeqNum()<ci->num-1)
    {
      {
        sms.setConcatSeqNum(sms.getConcatSeqNum()+1);
        try
        {
          store->changeSmsConcatSequenceNumber(t.msgId);
        }catch(...)
        {
          __warning2__("DELIVERYRESP: failed to change sms concat seq num:%lld",t.msgId);
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

      bool has_route = smsc->routeSms(sms.getOriginatingAddress(),sms.getDealiasedDestinationAddress(),dest_proxy_index,dest_proxy,NULL);
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
          smsc->notifyScheduler();
        }catch(...)
        {
          __trace__("CONCAT: failed to change state to enroute");
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
          smsc->notifyScheduler();
        }catch(...)
        {
          __trace__("CONCAT: failed to change state to enroute");
        }
        return ENROUTE_STATE;
      }
      // create task

      uint32_t dialogId2;
      try{
        dialogId2 = dest_proxy->getNextSequenceNumber();
        __trace2__("CONCAT: seq number:%d",dialogId2);
        //Task task((uint32_t)dest_proxy_index,dialogId2);
        Task task(dest_proxy->getUniqueId(),dialogId2);
        task.messageId=t.msgId;
        if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
        {
          __warning__("CONCAT: can't create task");
          return ENROUTE_STATE;
        }
      }catch(...)
      {
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
        Address dst=sms.getDealiasedDestinationAddress();
        sms.setDestinationAddress(dst);

        //
        //
        extractSmsPart(&sms,sms.getConcatSeqNum());

        SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
        dest_proxy->putCommand(delivery);
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
        __trace2__("CONCAT::Err %s",errtext);
      }

      //
      //
      //
      ////


      return DELIVERING_STATE;
    }
  }

  if(dgortr)
  {
    sms.state=DELIVERED;
    smsc->registerStatisticalEvent(StatEvents::etDeliveredOk,&sms);
    if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2)
    {
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
  }else
  {

    try{
      __trace__("change state to delivered");

      store->changeSmsStateToDelivered(t.msgId,t.command->get_resp()->getDescriptor());

      __trace__("change state to delivered: ok");
    }catch(std::exception& e)
    {
      __warning2__("change state to delivered exception:%s",e.what());
      return UNKNOWN_STATE;
    }
  }
  smsLog->debug("DLVRSP: DELIVERED, msgId=%lld",t.msgId);
  __trace__("DELIVERYRESP: registerStatisticalEvent");
  smsc->registerStatisticalEvent(StatEvents::etDeliveredOk,&sms);
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
      rpt.setIntProperty(Tag::SMPP_MSG_STATE,DELIVERED_STATE);
      char addr[64];
      sms.getDestinationAddress().getText(addr,sizeof(addr));
      rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
      rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,time(NULL));
      char msgid[60];
      sprintf(msgid,"%lld",t.msgId);
      rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
      string out;
      sms.getDestinationAddress().getText(addr,sizeof(addr));
      const Descriptor& d=sms.getDestinationDescriptor();
      __trace2__("RECEIPT: msc=%s, imsi=%s",d.msc,d.imsi);
      FormatData fd;
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
      fillSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,140);

      //smsc->submitSms(prpt);

      submitReceipt(rpt);


      /*splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
      for(int i=0;i<arr.Count();i++)
      {
        smsc->submitSms(arr[i]);
      };*/
    }
  }catch(std::exception& e)
  {
    __trace2__("DELIVERY_RESP:failed to submit receipt");
  }
  return DELIVERED_STATE;
}

StateType StateMachine::alert(Tuple& t)
{
  smsLog->debug("ALERT: msgId=%lld",t.msgId);
  //time_t now=time(NULL);
  Descriptor d;
  SMS sms;
  if(t.command->get_sms())
  {
    sms=*t.command->get_sms();
  }else
  {
    try{
      store->retriveSms((SMSId)t.msgId,sms);
    }catch(...)
    {
      smsLog->warn("ALERT: Failed to retrieve sms:%lld",t.msgId);
      return UNKNOWN_STATE;
    }
  }
  char bufsrc[64],bufdst[64];
  sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
  sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
  smsLog->warn("ALERT: delivery timed out(%s->%s)",bufsrc,bufdst);
  sms.setLastResult(Status::DELIVERYTIMEDOUT);
  if(sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3==1 ||
     sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3==2)
  {
    sms.state=EXPIRED;
    try{
      store->createFinalizedSms(t.msgId,sms);
    }catch(...)
    {
      smsLog->warn("ALERT: failed to finalize sms:%lld",t.msgId);
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
      __warning2__("ALERT: failed to change state to enroute");
    }
    smsc->notifyScheduler();
    sendNotifyReport(sms,t.msgId,"delivery attempt timed out");
  }
  smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
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
  }catch(...)
  {
    __REPLACE__RESPONSE(REPLACEFAIL);
  }


  if(sms.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x03 && sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
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

  if(sms.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)==0x03 &&
     !strcmp(sms.getDestinationSmeId(),"MAP_PROXY") &&
     t.command->get_replaceSm().smLength>140)
  {
    __trace__("REPLACE: das=3, and smLength>140");
    __REPLACE__RESPONSE(INVMSGLEN);
  }

  if(!strcmp(sms.getDestinationSmeId(),"MAP_PROXY"))
  {
    try{
      int pres=partitionSms(&sms,sms.getIntProperty(Tag::SMSC_DSTCODEPAGE));
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
    smsc->UpdateSmsSchedule(oldtime,t.msgId,newsched,idx);
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
  smsc->notifyScheduler();
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
    Address addr(t.command->get_cancelSm().sourceAddr.get());
    store->retriveSms(t.msgId,sms);

    if(sms.hasBinProperty(Tag::SMSC_CONCATINFO) && sms.getConcatSeqNum()>0)
    {
      throw Exception("CANCEL: attempt to cancel concatenated message already in delivery");
    }

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
    store->changeSmsStateToDeleted(t.msgId);
    sms.setLastResult(Status::DELETED);
    sendFailureReport(sms,t.msgId,DELETED,"");
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
  fillSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,140);
  //smsc->submitSms(prpt);
  submitReceipt(rpt);
  /*splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
  for(int i=0;i<arr.Count();i++)
  {
    smsc->submitSms(arr[i]);
  };*/
}

void StateMachine::sendNotifyReport(SMS& sms,MsgIdType msgId,const char* reason)
{
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
  rpt.setIntProperty(Tag::SMPP_ESM_CLASS,regdel?0x20:0);
  rpt.setDestinationAddress(sms.getOriginatingAddress());
  rpt.setMessageReference(sms.getMessageReference());
  rpt.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
    sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
  rpt.setIntProperty(Tag::SMPP_MSG_STATE,ENROUTE);
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
  fillSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,140);
  //smsc->submitSms(prpt);
  submitReceipt(rpt);
  /*splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
  for(int i=0;i<arr.Count();i++)
  {
    smsc->submitSms(arr[i]);
  };*/
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
  return nextTryTime;
}

void StateMachine::changeSmsStateToEnroute(SMS& sms,SMSId id,const Descriptor& d,
                                           uint32_t failureCause,time_t nextTryTime,
                                           bool skipAttempt)
{
  smsLog->debug("ENROUTE: msgId=%lld;lr=%d;or=%d;ntt=%u;ac=%d",id,sms.getLastResult(),sms.oldResult,nextTryTime,sms.getAttemptsCount());
  store->changeSmsStateToEnroute(id,d,failureCause,nextTryTime,sms.getAttemptsCount()+(skipAttempt?0:1));
  sms.setNextTime(nextTryTime);
  SmeIndex idx=smsc->getSmeIndex(sms.dstSmeId);
  smsc->ChangeSmsSchedule(id,sms.getNextTime(),idx);
}


void StateMachine::submitReceipt(SMS& sms)
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

  sms.setNextTime(time(NULL));

  try{
    int dest_proxy_index;
    SmeProxy *dest_proxy;
    smsc::router::RouteInfo ri;
    if(smsc->routeSms(sms.getOriginatingAddress(),sms.getDealiasedDestinationAddress(),dest_proxy_index,dest_proxy,NULL))
    {
      sms.setRouteId(ri.routeId.c_str());
      int prio=sms.getPriority()+ri.priority;
      if(prio>SmeProxyPriorityMax)prio=SmeProxyPriorityMax;
      sms.setPriority(prio);

      sms.setSourceSmeId("smscsme");

      sms.setDestinationSmeId(ri.smeSystemId.c_str());
      sms.setServiceId(ri.serviceId);
      sms.setArchivationRequested(ri.archived);
      sms.setBillingRecord(ri.billing);

      Profile profile=smsc->getProfiler()->lookup(dst);
      sms.setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage);


      store->createSms(sms,msgId,smsc::store::CREATE_NEW);
      smsc->ChangeSmsSchedule(msgId,sms.getNextTime(),dest_proxy_index);
    }else
    {
      __warning2__("There is no route for receipt %s->%s",
        sms.getOriginatingAddress().toString().c_str(),
        sms.getDealiasedDestinationAddress().toString().c_str());
    }
  }catch(...)
  {
    __warning2__("Faield to create receipt");
  }
}


};//system
};//smsc
