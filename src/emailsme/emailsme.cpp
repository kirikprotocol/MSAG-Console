#define FILEVER "$Id$"
#include <stdio.h>
#include <string.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include "util/smstext.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "emailsme/statuscodes.h"
#define NAMEDBRACKETS
#include "util/regexp/RegExp.cpp"
#include "core/buffers/Array.hpp"
#include "util/templates/Formatters.h"
#include "util/xml/init.h"
#include "util/BufferSerialization.hpp"
#include "emailsme/util/PipedChild.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "core/buffers/DiskHash.hpp"
#include "core/buffers/PageFile.hpp"
#include "util/crc32.h"
#include <signal.h>
#include <pthread.h>
#include <list>
#include <algorithm>
#include "sms/sms_util.h"
#include "emailsme/AbonentProfile.hpp"
#include "util/Base64.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "util/config/region/RegionsConfig.hpp"
#include "util/config/region/RegionFinder.hpp"
#include "sms/sms_serializer.h"
#include "core/buffers/File.hpp"
#include "system/status.h"
#include "util/sleep.h"
#include "util/udh.hpp"


#include "version.inc"
static const char* cssc_version="@(#)" FILEVER;


using namespace smsc::emailsme;
using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::regexp;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::util::templates;

using namespace smsc::core::buffers;

using namespace smsc::emailsme;

using namespace std;


char hostName[256];

bool isValidAlias(const std::string& value)
{
  bool haveLetters=false;
  for(int i=0;i<value.length();i++)
  {
    if(isalpha(value[i]))haveLetters=true;
    if(!isalnum(value[i]) && !strchr("-._",value[i]))return false;
  }
  return haveLetters;
}

bool ismailchar(char c)
{
  if(isalnum(c))return true;
  return c=='.' || c=='-' || c=='_' || c=='+';
}

bool isValidEmail(const std::string& value)
{
  bool haveAt=false;
  bool haveDot=false;
  for(int i=0;i<value.length();i++)
  {
    if(value[i]=='@')
    {
      if(haveAt)return false;
      haveAt=true;
    }else if(value[i]=='.' && haveAt)
    {
      haveDot=true;
    }else if(!ismailchar(value[i]))return false;
  }
  return haveAt && haveDot;
}

std::string trimSpaces(const char* str)
{
  std::string rv=str;
  size_t i=rv.length()-1;
  while(i>=0 && rv[i]==' ')
  {
    i--;
  }
  rv.erase(i+1);
  return rv;
}

void toLower(std::string& str)
{
  for(int i=0;i<str.length();i++)str[i]=tolower(str[i]);
}


class EmptyGetAdapter:public GetAdapter{
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


template <size_t N>
class StrKey{
protected:
  char str[N+1];
  uint8_t len;
public:
  StrKey()
  {
    memset(str,0,N+1);
    len=0;
  }
  StrKey(const char* s)
  {
    size_t l=strlen(s);
    strncpy(str,s,N);
    str[N]=0;
    len=(uint8_t)(l>N?N:l);
  }
  StrKey(const StrKey& src)
  {
    strcpy(str,src.str);
    len=src.len;
  }
  StrKey& operator=(const StrKey& src)
  {
    strcpy(str,src.str);
    len=src.len;
    return *this;
  }

  bool operator==(const StrKey& cmp)
  {
    return cmp.len==len && !strcmp(cmp.str,str);
  }

  const char* toString(){return str;}
  static uint32_t Size(){return N+1;}
  void Read(File& f)
  {
    f.XRead(len);
    f.Read(str,N);
    str[len]=0;
  }
  void Write(File& f)const
  {
    f.XWrite(len);
    f.Write(str,N);
  }
  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t res=0;
    res=crc32(res,str,len);
    for(;attempt>0;attempt--)res=crc32(res,str,len);
    return res;
  }
};

struct Int64Key{
  uint64_t key;

  Int64Key():key(0){}
  Int64Key(uint64_t key):key(key){}
  Int64Key(const Int64Key& src)
  {
    key=src.key;
  }
  Int64Key& operator=(const Int64Key& src)
  {
    key=src.key;
    return *this;
  }
  uint64_t Get()const{return key;}

  static uint32_t Size(){return 8;}
  void Read(File& f)
  {
    key=f.ReadNetInt64();
  }
  void Write(File& f)const
  {
    f.WriteNetInt64(key);
  }
  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t res=0;
    res=crc32(res,&key,sizeof(key));
    for(;attempt>0;attempt--)res=crc32(res,&key,sizeof(key));
    return res;
  }
  bool operator==(const Int64Key& cmp)
  {
    return key==cmp.key;
  }
};

template <int AbonentProfile::*field>
bool LimitBreak(AbonentProfile& p)
{
  struct tm tdate;
  struct tm tnow;
  localtime_r(&p.limitDate,&tdate);
  time_t now=time(NULL);
  localtime_r(&now,&tnow);
  switch(p.ltype)
  {
    case ltDay:
      if(tdate.tm_mday!=tnow.tm_mday)
      {
        p.limitDate=now;
        p.limitCountEml2Gsm=0;
        p.limitCountGsm2Eml=0;
        return false;
      }
      break;
    case ltWeek:
      if((now-p.limitDate)/(60*60*24)>7)
      {
        p.limitDate=now;
        p.limitCountEml2Gsm=0;
        p.limitCountGsm2Eml=0;
        return false;
      }
      break;
    case ltMonth:
      if(tdate.tm_mon!=tnow.tm_mon)
      {
        p.limitDate=now;
        p.limitCountEml2Gsm=0;
        p.limitCountGsm2Eml=0;
        return false;
      }
      break;
  }
  return p.*field>=p.limitValue;
}

class ProfileStorage{
public:
  ProfileStorage()
  {
  }
  void Open(const char* dir)
  {
    log=smsc::logger::Logger::getInstance("storage");
    std::string baseDir=dir;
    if(baseDir.length()>0 && baseDir[baseDir.length()-1]!='/')baseDir+='/';
    std::string storeFileName=baseDir+"store.bin";
    std::string addressIdxFileName=baseDir+"addr.idx";
    std::string emailIdxFileName=baseDir+"email.idx";
    if(!File::Exists(storeFileName.c_str()))
    {
      store.Create(storeFileName.c_str(),128,100000);
      abonentIndex.Create(addressIdxFileName.c_str(),100000,false);
      emailIndex.Create(emailIdxFileName.c_str(),100000,false);
    }else
    {
      store.Open(storeFileName.c_str());
      abonentIndex.Open(addressIdxFileName.c_str());
      emailIndex.Open(emailIdxFileName.c_str());
    }
  }

  bool CreateProfile(const AbonentProfile& p)
  {
    smsc_log_debug(log,"Create profile for %s/%s",p.addr.toString().c_str(),p.user.c_str());
    File::offset_type off;
    {
      MutexGuard mg(cacheMtx);
      CacheList::iterator* ptr=abonentCache.GetPtr(p.addr.toString().c_str());
      if(ptr)
      {
        return false;
      }
    }
    {
      MutexGuard mg(diskMtx);
      Int64Key value;
      if(abonentIndex.LookUp(p.addr.toString().c_str(),value))
      {
        return false;
      }
      SerializationBuffer sb;
      p.Write(sb);
      off=store.Append(sb.getBuffer(),sb.getPos());
      abonentIndex.Insert(p.addr.value,off);
      emailIndex.Insert(p.user.c_str(),off);
    }
    {
      MutexGuard mg(cacheMtx);

      AbonentProfile* pc=new AbonentProfile;
      *pc=p;
      pc->offset=off;
      cache.push_back(pc);
      CacheList::iterator it=cache.end();
      it--;
      abonentCache.Insert(p.addr.value,it);
      emailCache.Insert(p.user.c_str(),it);
      CheckCacheLimit();
    }
    return true;
  }
  bool getProfileByAddress(const char* addr,AbonentProfile& p)
  {
    {
      MutexGuard mg(cacheMtx);
      CacheList::iterator* ptr=abonentCache.GetPtr(addr);
      if(ptr)
      {
        p=***ptr;
        return true;
      }
    }
    bool rv=false;
    std::vector<unsigned char> buf;
    SerializationBuffer buffer;
    File::offset_type off;
    Int64Key value;
    {
      MutexGuard mg(diskMtx);
      if(abonentIndex.LookUp(addr,value))
      {
        off=value.key;
        store.Read(off,buf);
        rv=true;
      }
    }
    if(rv)
    {
      buffer.setExternalBuffer(&buf[0],(unsigned)buf.size());
      p.Read(buffer);
      p.offset=off;
      MutexGuard mg(cacheMtx);
      cache.push_back(new AbonentProfile(p));
      CacheList::iterator it=cache.end();
      it--;
      abonentCache.Insert(addr,it);
      emailCache.Insert(p.user.c_str(),it);
      CheckCacheLimit();
    }
    return rv;
  }
  bool getProfileByEmail(const char* user,AbonentProfile& p)
  {
    {
      MutexGuard mg(cacheMtx);
      CacheList::iterator* ptr=emailCache.GetPtr(user);
      if(ptr)
      {
        p=***ptr;
        return true;
      }
    }
    bool rv=false;
    std::vector<unsigned char> buf;
    SerializationBuffer buffer;
    File::offset_type off;
    {
      MutexGuard mg(diskMtx);
      Int64Key value;
      if(emailIndex.LookUp(user,value))
      {
        off=value.key;
        store.Read(off,buf);
        rv=true;
      }
    }
    if(rv)
    {
      buffer.setExternalBuffer(&buf[0],(int)buf.size());
      p.Read(buffer);
      p.offset=off;
      MutexGuard mg(cacheMtx);
      cache.push_back(new AbonentProfile(p));
      CacheList::iterator it=cache.end();
      it--;
      abonentCache.Insert(p.addr.value,it);
      emailCache.Insert(user,it);
      CheckCacheLimit();
    }
    return rv;
  }

  void DeleteProfile(const AbonentProfile& p)
  {
    smsc_log_debug(log,"Delete profile %s/%s",p.addr.toString().c_str(),p.user.c_str());
    {
      MutexGuard mg(cacheMtx);
      CacheList::iterator* ptr=emailCache.GetPtr(p.user.c_str());
      if(ptr)
      {
        cache.erase(*ptr);
        abonentCache.Delete(p.addr.value);
        emailCache.Delete(p.user.c_str());
      }
    }
    MutexGuard mg(diskMtx);
    abonentIndex.Delete(p.addr.value);
    emailIndex.Delete(p.user.c_str());
    store.Delete(p.offset);
  }
  void UpdateProfile(AbonentProfile& p)
  {
    smsc_log_debug(log,"Update profile for %s/%s",p.addr.toString().c_str(),p.user.c_str());
    {
      MutexGuard mg(cacheMtx);
      CacheList::iterator* ptr=emailCache.GetPtr(p.user.c_str());
      if(ptr)
      {
        p.offset=(**ptr)->offset;
        ***ptr=p;
      }
    }
    SerializationBuffer buf(256);
    p.Write(buf);
    MutexGuard mg(diskMtx);
    store.Update(p.offset,buf.getBuffer(),buf.getPos());
  }

  bool checkEml2GsmLimit(const char* email)
  {
    MutexGuard lmg(limitMtx);
    AbonentProfile p;
    if(!getProfileByEmail(email,p) && !getProfileByAddress(email,p))return false;
    bool rv=!LimitBreak<&AbonentProfile::limitCountEml2Gsm>(p);
    UpdateProfile(p);
    return rv;
  }

  void incEml2GsmLimit(const char* email)
  {
    MutexGuard lmg(limitMtx);
    AbonentProfile p;
    if(!getProfileByEmail(email,p) && !getProfileByAddress(email,p))throw smsc::util::Exception("Unknown user:%s",email);;
    p.limitCountEml2Gsm++;
    smsc_log_debug(log,"Inc eml2gsm for %s=%d/%d",email,p.limitCountEml2Gsm,p.limitValue);
    UpdateProfile(p);
    /*
    SerializationBuffer buf(256);
    p.Write(buf);
    MutexGuard mg(diskMtx);
    store.Update(p.offset,buf.getBuffer(),buf.getPos());
    */
  }

  bool checkGsm2EmlLimit(const char* addr)
  {
    MutexGuard lmg(limitMtx);
    AbonentProfile p;
    if(!getProfileByAddress(addr,p))return false;
    bool rv=!LimitBreak<&AbonentProfile::limitCountGsm2Eml>(p);
    UpdateProfile(p);
    return rv;
  }

  void incGsm2EmlLimit(const char* addr)
  {
    MutexGuard lmg(limitMtx);
    AbonentProfile p;
    if(!getProfileByAddress(addr,p))throw smsc::util::Exception("Unknown abonent:%s",addr);
    p.limitCountGsm2Eml++;
    smsc_log_debug(log,"Inc gsm2eml for %s=%d/%d",addr,p.limitCountGsm2Eml,p.limitValue);
    UpdateProfile(p);
    /*
    SerializationBuffer buf(256);
    p.Write(buf);
    MutexGuard mg(diskMtx);
    store.Update(p.offset,buf.getBuffer(),buf.getPos());
    */
  }

protected:
  Mutex diskMtx;
  DiskHash<StrKey<24>,Int64Key> abonentIndex;
  DiskHash<StrKey<32>,Int64Key> emailIndex;
  PageFile store;

  Mutex limitMtx;

  Mutex cacheMtx;
  typedef std::list<AbonentProfile*> CacheList;
  Hash<CacheList::iterator> abonentCache,emailCache;
  CacheList cache;
  int cacheLimit;

  smsc::logger::Logger* log;

  void CheckCacheLimit()
  {
    while(abonentCache.GetCount()<cacheLimit)
    {
      abonentCache.Delete(cache.front()->addr.value);
      emailCache.Delete(cache.front()->user.c_str());
      delete cache.front();
      cache.pop_front();
    }
  }
};

ProfileStorage storage;


class AdminCommandsListener:public smsc::core::threads::Thread{
public:
  AdminCommandsListener():running(false)
  {
    log=smsc::logger::Logger::getInstance("emladm");
    readTimeOut=20;
  }
  void Init(const char* host,int port)
  {
    if(sck.InitServer(host,port,0)==-1)throw smsc::util::Exception("Failed to init server socket at %s:%d",host,port);
    if(sck.StartServer()==-1)throw smsc::util::Exception("Failed to start server socket at %s:%d",host,port);
  }
  int Execute()
  {
    smsc_log_debug(log,"Starting AdminCommandsListener");
    running=true;
    SerializationBuffer buf;
    TmpBuf<char,1024> tmp(0);
    while(running)
    {
      std::auto_ptr<smsc::core::network::Socket> clnt(sck.Accept());
      if(!clnt.get())
      {
        int err=errno;
        smsc_log_warn(log,"Accept returned NULL. Errno=%d, Errstr='%s'",err,strerror(err));
        break;
      }
      char szbuf[4]={0,};
      int sz=0;
      while(sz<4)
      {
        if(!clnt->canRead(readTimeOut))break;
        int rd=clnt->Read(szbuf+sz,4-sz);
        if(rd<=0)break;
        sz+=rd;
      }
      if(sz!=4)
      {
        smsc_log_warn(log,"Failed to read packet size from socket");
        continue;
      }
      smsc_log_debug(log,"szbuf:%02x %02x %02x %02x",szbuf[0],szbuf[1],szbuf[2],szbuf[3]);
      uint32_t pktSize=0;
      memcpy(&pktSize,szbuf,4);
      pktSize=ntohl(pktSize);
      if(pktSize>65536)
      {
        smsc_log_warn(log,"Packet size too large:%d",pktSize);
        continue;
      }
      if(pktSize<4)
      {
        smsc_log_warn(log,"Packet size too small:%d",pktSize);
        continue;
      }
      tmp.setSize(pktSize);
      int bufSz=0;
      while(bufSz<pktSize)
      {
        if(!clnt->canRead(readTimeOut))break;
        int rd=clnt->Read(tmp.get()+bufSz,pktSize-bufSz);
        if(rd<=0)break;
        bufSz+=rd;
      }
      if(bufSz<pktSize)
      {
        smsc_log_warn(log,"Failed to read packet from socket:%d/%d",bufSz,pktSize);
        continue;
      }
      std::string dump;
      for(int i=0;i<pktSize;i++)
      {
        char hex[8];
        sprintf(hex," %02x",(int)(unsigned char)tmp.get()[i]);
        dump+=hex;
      }
      smsc_log_debug(log,"Packet dump:%s",dump.c_str());
      buf.setExternalBuffer(tmp.get(),pktSize);
      uint32_t cmdId;
      uint32_t rv=1;
      bool writeRV=true;
      try
      {
        cmdId=buf.ReadNetInt32();
        smsc_log_debug(log,"Received cmdId=%d",cmdId);
        switch(cmdId)
        {
          case cmdUpdateProfile:
          {
            AbonentProfile p,p2;
            p.Read(buf);
            toLower(p.user);
            if(storage.getProfileByEmail(p.user.c_str(),p2))
            {
              if(!(p.addr==p2.addr))
              {
                smsc_log_info(log,"attempt to add duplicate user '%s' for abonent '%s' (exists for '%s')",p.user.c_str(),p.addr.toString().c_str(),p2.addr.toString().c_str());
                rv=2;
                //throw smsc::util::Exception("Duplicate user:%s",p.user.c_str());
              }
            }


            if(rv==1)
            {
              if(storage.getProfileByAddress(p.addr.value,p2))
              {
                p.limitCountEml2Gsm=p2.limitCountEml2Gsm;
                p.limitCountGsm2Eml=p2.limitCountGsm2Eml;
                p.limitDate=p2.limitDate;
                if(p.user!=p2.user)
                {
                  storage.DeleteProfile(p2);
                  storage.CreateProfile(p);
                }else
                {
                  storage.UpdateProfile(p);
                }
              }else
              {
                storage.CreateProfile(p);
              }
            }
          }break;
          case cmdDeleteProfile:
          {
            Address addr;
            ReadAddress(buf,addr);
            AbonentProfile p;
            if(!storage.getProfileByAddress(addr.value,p))
            {
              smsc_log_info(log,"cmdDeleteProfile failed, profile not found for addr:%s",addr.value);
              rv=0;
            }else
            {
              storage.DeleteProfile(p);
            }
          }break;
          case cmdLookupByAddr:
          {
            Address addr;
            ReadAddress(buf,addr);
            AbonentProfile p;
            if(storage.getProfileByAddress(addr.value,p))
            {
              writeRV=false;
              SerializationBuffer outbuf;
              p.Write(outbuf);
              uint32_t wsz=htonl(outbuf.getPos());
              clnt->WriteAll((char*)&wsz,4);
              clnt->WriteAll((char*)outbuf.getBuffer(),outbuf.getPos());
            }else
            {
              rv=0;
            }
          }break;
          case cmdLookupByUser:
          {
            std::string user;
            ReadString(buf,user);
            AbonentProfile p;
            if(storage.getProfileByEmail(user.c_str(),p))
            {
              writeRV=false;
              SerializationBuffer outbuf;
              p.Write(outbuf);
              uint32_t wsz=htonl(outbuf.getPos());
              clnt->WriteAll((char*)&wsz,4);
              clnt->WriteAll((char*)outbuf.getBuffer(),outbuf.getPos());
            }else
            {
              rv=0;
            }
          }break;
          default:
          {
            rv=0;
            smsc_log_warn(log,"unknown cmdid=%d",cmdId);
          }
        }
      }catch(std::exception& e)
      {
        rv=0;
        smsc_log_warn(log,"exception in cmd=%d:'%s'",cmdId,e.what());
      }
      if(writeRV)
      {
        smsc_log_info(log,"Sending answer:%d",rv);
        rv=htonl(rv);
        clnt->Write((char*)&rv,4);
      }

    }
    smsc_log_debug(log,"Finishing AdminCommandsListener");
    return 0;
  }
  void Stop()
  {
    running=false;
    sck.Abort();
  }
protected:
  enum{cmdUpdateProfile=1,cmdDeleteProfile,cmdLookupByAddr,cmdLookupByUser};
  smsc::core::network::Socket sck;
  int readTimeOut;
  bool running;
  smsc::logger::Logger* log;
};

class IAnswerQueue{
public:
  virtual void enqueueAnswer(PduSubmitSm* pdu)=0;
};

class EmailProcessor;
namespace cfg{
  string sourceAddress;
  int protocolId;
  string serviceType;
  SmppTransmitter *tr;
  SmppTransmitter *atr;
  IAnswerQueue* aq;
  int mainId;
  bool stopSme=false;
  string smtpHost;
  int smtpPort=25;
  int retryTime=5;
  bool pauseAfterDisconnect=false;
  //int defaultDailyLimit;
  int annotationSize;
  string maildomain;
  string hdmaildomain="";
  vector<string> validDomains;
  string mailstripper;
  int mailthreadsCount;
  EmailProcessor* mailThreads;

  OutputFormatter *msgFormat;
  Hash<OutputFormatter*> answerFormats;
  string storeDir;
  bool allowGsm2EmlWithoutProfile=false;
  bool allowEml2GsmWithoutProfile=false;
  bool autoCreateGsm2EmlProfile=false;
  LimitType defaultLimitType=ltDay;
  int defaultLimitValue=10;
  bool sendSuccessAnswer=true;

  std::string helpDeskAddress="";
  bool regionsEnabled=false;

  bool useTransformRegexp=false;
  // RegExp reTransform;
  std::string transformResult;
  bool partitionSms=false;
  int maxUdhParts=10;
};

class StatisticsCollector:public smsc::core::threads::Thread{
public:
  StatisticsCollector()
  {
    needToStop=false;
    sms2emlok=0;
    sms2emlerr=0;
    eml2smsok=0;
    eml2smserr=0;
  }
  void Init(const std::string& store,int argFlushPeriod)
  {
    flushPeriod=argFlushPeriod;
    if((3600%flushPeriod)!=0)
    {
      throw Exception("Invalid flush period value(3600 mod period != 0):%d",flushPeriod);
    }
    lastHour=9999999;
    storeLocation=store;
    if(storeLocation.length() && *storeLocation.rbegin()!='/')
    {
      storeLocation+='/';
    }
    if(storeLocation.length() && !File::Exists(storeLocation.c_str()))
    {
      File::MkDir(storeLocation.c_str());
    }
  }
  int Execute()
  {
    MutexGuard mg(mon);
    while(!needToStop)
    {
      time_t now=time(NULL);
      tm t;
      gmtime_r(&now,&t);
      int secs=t.tm_hour*3600+t.tm_min*60+t.tm_sec;
      int toSleep=flushPeriod-(secs%flushPeriod);
      toSleep*=1000;
      mon.wait(toSleep);
      try
      {
        FlushStats();
        smsc::util::millisleep(1000);
      } catch(std::exception& e)
      {
        smsc_log_warn(smsc::logger::Logger::getInstance("stats"),"Exception during FlushStats:'%s'",e.what());
      }
    }
    f.Close();
    return 0;
  }
  void Stop()
  {
    MutexGuard mg(mon);
    needToStop=true;
    mon.notify();
  }
  void FlushStats()
  {
    time_t now=time(NULL);
    tm t;
    gmtime_r(&now,&t);
    if(t.tm_hour!=lastHour)
    {
      f.Close();
      char buf[32];
      sprintf(buf,"%04d%02d%02d",t.tm_year+1900,t.tm_mon+1,t.tm_mday);
      std::string fullPath=storeLocation;
      fullPath+=buf;
      if(!File::Exists(fullPath.c_str()))
      {
        File::MkDir(fullPath.c_str());
      }
      sprintf(buf,"%02d.txt",t.tm_hour);
      fullPath+='/';
      fullPath+=buf;
      f.Append(fullPath.c_str());
      lastHour=t.tm_hour;
    }
    char buf[64];
    int len=sprintf(buf,"%04d-%02d-%02d %02d:%02d\n",t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min);
    f.Write(buf,len);
    len=sprintf(buf,"sms_received_ok,%d\nsms_transmitted_ok,%d\n",sms2emlok,eml2smsok);
    f.Write(buf,len);
    len=sprintf(buf,"sms_received_fail,%d\nsms_transmitted_fail,%d\n",sms2emlerr,eml2smserr);
    f.Write(buf,len);
    f.Flush();
    sms2emlok=0;
    sms2emlerr=0;
    eml2smsok=0;
    eml2smserr=0;
  }
  void IncSms2Eml(bool ok=true)
  {
    MutexGuard mg(mon);
    if(ok)
    {
      sms2emlok++;
    }else
    {
      sms2emlerr++;
    }
  }
  void IncEml2Sms(bool ok=true)
  {
    MutexGuard mg(mon);
    if(ok)
    {
      eml2smsok++;
    }else
    {
      eml2smserr++;
    }
  }
protected:
  bool needToStop;
  EventMonitor mon;
  int flushPeriod;

  File f;
  std::string storeLocation;
  int lastHour;
  int sms2emlok;
  int sms2emlerr;
  int eml2smsok;
  int eml2smserr;
}statCollector;

int stopped=0;

RegExp reParseSms;

bool getField(const char* text,Hash<SMatch>& h,const char* key,string& out)
{
  if(!h.Exists(key))return false;
  SMatch& m=h[key];
  if(m.start==-1 || m.end==-1)return false;
  out.assign(text,m.start,m.end-m.start);
  return true;
}

namespace ProcessSmsCodes{
const int INVALIDSMS   =0;
const int NETERROR     =1;
const int UNABLETOSEND =2;
const int NOPROFILE    =3;
const int OUTOFLIMIT   =4;
const int OK           =256;
};

void CheckCode(Socket& s,int code)
{
  char buf[1024];
  if(s.Gets(buf,(int)sizeof(buf))==-1)throw Exception("Failed to read response from smtp server");
  int retcode;
  if(sscanf(buf,"%d",&retcode)!=1)throw Exception("failed to scan response code from '%s'",buf);
  if(code<10)retcode/=100;
  if(retcode!=code)
  {
    __warning2__("code %d, expected %d",retcode,code);
    throw Exception("code %d, expected %d",retcode,code);
  }
}

string makeFromAddress(const char* fromaddress,bool hdMode)
{
  return (string)fromaddress+"@"+(hdMode?cfg::hdmaildomain:cfg::maildomain);
}

string ExtractEmail(const string& value,size_t startPos=0)
{
  std::string::size_type pos=value.find('@',startPos);
  if(pos==string::npos)return "";
  size_t start=pos,end=pos;
  while(start>0 && ismailchar(value[start-1]))start--;
  while(end<value.length() && ismailchar(value[end+1]))end++;
  string res(value,start,end-start+1);
  __trace2__("extracting email from '%s':'%s'",value.c_str(),res.c_str());
  return res;
}

string MapEmailToAddress(const string& username,string& fwd)
{
  AbonentProfile p;
  if(!storage.getProfileByEmail(username.c_str(),p))
  {
    if(!storage.getProfileByAddress(username.c_str(),p) || !p.numberMap)
    {
      throw Exception("Unknown username:%s",username.c_str());
    }
  }
  fwd=p.forwardEmail;
  return p.addr.toString();
  /*
  using namespace smsc::db;
  const char* sql="select address,forward,daily_limit,(select count(*) from EMLSME_HISTORY where MSG_DATE<=:1 and EMLSME_HISTORY.address=EMLSME_PROFILES.address) from EMLSME_PROFILES where username=:2";
  ConnectionGuard connection(cfg::dataSource);
  if(!connection.get())throw Exception("Failed to get db connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Failed to prepare statement");
  __trace__("MapEmailToAddress statement created");
  time_t now=time(NULL);
  statement->setDateTime(1,now+24*60*60);
  statement->setString(2,username.c_str());
  auto_ptr<ResultSet> rs(statement->executeQuery());
  __trace__("Execute ok");
  if(!rs.get())throw Exception("Failed to make a query to DB");
  if(!rs->fetchNext())throw Exception("Mapping username->address for %s not found",username.c_str());
  int dailyLimit=rs->isNull(3)?cfg::defaultDailyLimit:rs->getInt32(3);
  int currentCount=rs->getInt32(4);
  __trace2__("%s:currentCount=%d, dailyLimit=%d",username.c_str(),currentCount,dailyLimit);
  if(currentCount>=dailyLimit)
  {
    throw Exception("daily limit reached for %s",username.c_str());
  }
  fwd=rs->isNull(2)?"":rs->getString(2);
  __trace2__("Map %s->%s",username.c_str(),rs->getString(1));
  return rs->getString(1);
  */
}

string MapAddressToEmail(const string& address)
{
  AbonentProfile p;
  if(!storage.getProfileByAddress(address.c_str(),p))
  {
    throw Exception("Profile not found for address=%s",address.c_str());
  }
  return p.user;
  /*
  using namespace smsc::db;
  const char* sql="select username from EMLSME_PROFILES where address=:1";
  ConnectionGuard connection(cfg::dataSource);
  if(!connection.get())throw Exception("Failed to get db connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Failed to prepare statement");
  statement->setString(1,address.c_str());
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception("Failed to make a query to DB");
  if(!rs->fetchNext())throw Exception("Mapping address->email for %s not found",address.c_str());
  __trace2__("Map %s->%s",address.c_str(),rs->getString(1));
  return rs->getString(1);
  */
}


void ReplaceString(string& s,const char* what,const char* to)
{
  size_t i=0;
  size_t l=strlen(what);
  size_t l2=strlen(to);
  while((i=s.find(what,i))!=string::npos)
  {
    s.replace(i,l,to);
    i+=l2;
  }
}

int SendEMail(const string& from,const Array<string>& to,const string& subj,const string& body,bool rawMsg=false)
{
  __trace2__("sending email from %s to %d recepients(first=%s), subj=%s",from.c_str(),to.Count(),to[0].c_str(),subj.c_str());
  Socket s;
  if(s.Init(cfg::smtpHost.c_str(),cfg::smtpPort,0)==-1)
  {
    __warning__("smtp host resolve failed");
    return ProcessSmsCodes::NETERROR;
  }
  if(s.Connect()==-1)
  {
    __warning__("smtp connect failed");
    return ProcessSmsCodes::NETERROR;
  }
  CheckCode(s,220);
  s.Printf("HELO %s\r\n",hostName);
  CheckCode(s,250);
  s.Printf("MAIL FROM: %s\r\n",ExtractEmail(from).c_str());
  CheckCode(s,250);
  for(int i=0;i<to.Count();i++)
  {
    s.Printf("RCPT TO: %s\r\n",ExtractEmail(to[i]).c_str());
    CheckCode(s,2);
  }
  s.Puts("DATA\r\n");
  CheckCode(s,3);
  if(!rawMsg)
  {
    s.Printf("From: %s\r\n",from.c_str());
    string addr;
    for(int i=0;i<to.Count();i++)
    {
      addr+=to[i];
      if(i!=to.Count()-1)addr+=',';
    }
    s.Printf("To: %s\r\n",addr.c_str());
    s.Printf("Content-Type: text/plain; charset=windows-1251\n");
    if(subj.length())s.Printf("Subject: =?windows-1251?B?%s?=\r\n",encode64(subj).c_str());
    s.Printf("\r\n");
  }
  if(s.WriteAll(body.c_str(),(int)body.length())==-1)throw Exception("Failed to write body");
  s.Puts("\r\n.\r\n");
  CheckCode(s,250);
  s.Puts("QUIT\r\n");
  return ProcessSmsCodes::OK;
}



void sendAnswer(const Address& orgAddr,bool hdMode,const char* msgName,const char* paramName=0,const char* paramValue=0)
{
  try{
    SMS sms;
    sms.setOriginatingAddress(hdMode?cfg::helpDeskAddress.c_str():cfg::sourceAddress.c_str());
    ContextEnvironment ce;
    EmptyGetAdapter ga;
    if(paramName)
    {
      ce.exportStr(paramName,paramValue?paramValue:"");
    }
    std::string text;
    std::string msgNameStr=hdMode?"helpdesk.":"";
    msgNameStr+=msgName;
    cfg::answerFormats[msgNameStr.c_str()]->format(text,ga,ce);
    sms.setDestinationAddress(orgAddr);
    sms.destinationAddress.type=1;
    sms.destinationAddress.plan=1;
    char msc[]="";
    char imsi[]="";
    sms.setOriginatingDescriptor((uint8_t)strlen(msc),msc,(uint8_t)strlen(imsi),imsi,1);
    sms.setDeliveryReport(0);
    sms.setArchivationRequested(false);
    sms.setEServiceType(cfg::serviceType.c_str());
    sms.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,cfg::protocolId);
    sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS,0);
    __trace2__("sending answer:%s",text.c_str());
    if(hasHighBit(text.c_str(),text.length()))
    {
      TmpBuf<short,1024> ucs2(text.length()+1);
      ConvertMultibyteToUCS2(text.c_str(),text.length(),ucs2.get(),(text.length()+1)*2,CONV_ENCODING_CP1251);
      sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::UCS2);
      sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,(char*)ucs2.get(),(unsigned)text.length()*2);
    }else
    {
      sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
      sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,text.c_str(),(unsigned)text.length());
    }
    PduSubmitSm* sm=new PduSubmitSm;
    sm->get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
    fillSmppPduFromSms(sm,&sms);
    cfg::aq->enqueueAnswer(sm);
  }catch(std::exception& e)
  {
    __warning2__("Failed to send answer sms:%s",e.what());
  }
}

int processSms(const char* text,const char* fromaddress,const char* toaddress)
{
  __trace2__("sms from %s to %s. hdaddr=%s",fromaddress,toaddress,cfg::helpDeskAddress.c_str());
  bool hdMode=false;
  try{
    string addr,subj,body,from,fromdecor;
    if(cfg::helpDeskAddress.length() && cfg::regionsEnabled && cfg::helpDeskAddress==toaddress)
    {
      using namespace smsc::util::config::region;
      const Region* reg=RegionFinder::getInstance().findRegionByAddress(fromaddress);
      addr=reg->getEmail();
      subj="EmailSMS from abonent ";
      subj+=fromaddress;
      body=text;
      hdMode=true;
    }else
    {
      Hash<SMatch> h;
      SMatch m[10];
      int n=10;
      if(!reParseSms.Match(text,m,n,&h))
      {
        __trace2__("RegExp match failed:%d",reParseSms.LastError());
        sendAnswer(fromaddress,hdMode,"unknowncommand");
        return ProcessSmsCodes::INVALIDSMS;
      }
      //getField(text,h,"flag",cf);
      getField(text,h,"address",addr);
      //getField(text,h,"realname",rn);
      getField(text,h,"subj",subj);
      getField(text,h,"body",body);

      ReplaceString(addr,"*","@");
      ReplaceString(addr,"$","_");
    }
    //Socket s;


    ReplaceString(body,"\n.\n","\n..\n");

    AbonentProfile p;
    bool haveprofile=true;
    if(!storage.getProfileByAddress(fromaddress,p))
    {
      __trace2__("no profile for abonent %s",fromaddress);
      if(!cfg::allowGsm2EmlWithoutProfile)
      {
        sendAnswer(fromaddress,hdMode,"messagefailednoprofile");
        return ProcessSmsCodes::NOPROFILE;
      }
      haveprofile=false;
      p.user=fromaddress;
    }

    while(*text==' ')text++;
    const char* sp=strchr(text,' ');
    std::string cmd;
    if(sp)
    {
      cmd.assign(text,0,sp-text);
      toLower(cmd);
      while(*sp==' ')sp++;
    }else
    {
      cmd=text;
      toLower(cmd);
    }
    while(cmd.length() && cmd[cmd.length()-1]==' ')cmd.erase(cmd.length()-1);

    __trace2__("cmd=%s",cmd.c_str());

    if(haveprofile)
    {
      try{
        if(cmd=="alias")
        {
          std::string value=trimSpaces(sp);
          toLower(value);
          AbonentProfile p2;
          if(storage.getProfileByEmail(value.c_str(),p2))
          {
            sendAnswer(fromaddress,hdMode,"aliasbusy","alias",value.c_str());
          }else
          if(value.length() && value!=p.user && isValidAlias(value))
          {
            storage.DeleteProfile(p);
            p.user=value;
            p.numberMap=false;
            storage.CreateProfile(p);
            sendAnswer(fromaddress,hdMode,"alias","alias",value.c_str());
          }else
          {
            sendAnswer(fromaddress,hdMode,"aliasfailed","alias",value.c_str());
          }
          return ProcessSmsCodes::OK;
        }else
        if(cmd=="noalias")
        {
          if(p.user!=p.addr.value)
          {
            storage.DeleteProfile(p);
            p.user=p.addr.value;
            storage.CreateProfile(p);
          }
          sendAnswer(fromaddress,hdMode,"noalias");
          return ProcessSmsCodes::OK;
        }else
        if(cmd=="forward")
        {
          std::string eml=trimSpaces(sp);
          if(isValidEmail(eml.c_str()))
          {
            p.forwardEmail=eml;
            storage.UpdateProfile(p);
            sendAnswer(fromaddress,hdMode,"forward","email",eml.c_str());
          }else
          {
            sendAnswer(fromaddress,hdMode,"forwardfailed","email",eml.c_str());
          }
          return ProcessSmsCodes::OK;
        }else
        if(cmd=="forwardoff" || cmd=="noforward")
        {
          p.forwardEmail="";
          storage.UpdateProfile(p);
          sendAnswer(fromaddress,hdMode,"forwardoff");
          return ProcessSmsCodes::OK;
        }else
        if(cmd=="realname")
        {
          p.realName=trimSpaces(sp);
          storage.UpdateProfile(p);
          sendAnswer(fromaddress,hdMode,"realname","realname",p.realName.c_str());
          return ProcessSmsCodes::OK;
        }else
        if(cmd=="norealname")
        {
          std::string oldRn=p.realName;
          p.realName="";
          storage.UpdateProfile(p);
          sendAnswer(fromaddress,hdMode,"norealname","realname",oldRn.c_str());
          return ProcessSmsCodes::OK;
        }else
        if(cmd=="number")
        {
          std::string val=trimSpaces(sp);
          for(int i=0;i<val.length();i++)val[i]=tolower(val[i]);
          if(val=="on")
          {
            p.numberMap=true;
            sendAnswer(fromaddress,hdMode,"numberon");
            storage.UpdateProfile(p);
          }
          else if(val=="off")
          {
            p.numberMap=false;
            storage.UpdateProfile(p);
            sendAnswer(fromaddress,hdMode,"numberoff");
          }
          else
          {
            sendAnswer(fromaddress,hdMode,"numberfailed");
          }
          return ProcessSmsCodes::OK;
        }
      }
      catch(std::exception& e)
      {
        __trace2__("Exception in command processing:%s",e.what());
        sendAnswer(fromaddress,hdMode,"systemerror");
        return ProcessSmsCodes::OK;
      }
      catch(...)
      {
        __trace__("Exception in command processing:unknown");
        sendAnswer(fromaddress,hdMode,"systemerror");
        return ProcessSmsCodes::OK;
      }
    }

    if(haveprofile)
    {
      if(!storage.checkGsm2EmlLimit(fromaddress))
      {
        sendAnswer(fromaddress,hdMode,"messagefailedlimit");
        return ProcessSmsCodes::OUTOFLIMIT;
      }
    }

    if(addr.length()==0)
    {
      __trace__("to addr is empty");
      sendAnswer(fromaddress,hdMode,"unknowncommand");
      return ProcessSmsCodes::INVALIDSMS;
    }

    try{
      if(haveprofile)
      {
        from=makeFromAddress(MapAddressToEmail(fromaddress).c_str(),hdMode);
      }else
      {
        from=makeFromAddress(fromaddress,hdMode);
      }
    }catch(exception& e)
    {
      __warning2__("failed to map address to mail:%s",e.what());
      sendAnswer(fromaddress,hdMode,"systemerror");
      return ProcessSmsCodes::NOPROFILE;
    }
    fromdecor=from;
    if(p.realName.length()>0)
    {
      fromdecor='"'+p.realName+"\" <"+from+">";
    }

    Array<string> to;
    size_t startpos=0;
    size_t pos=addr.find(',');
    if(pos==string::npos)
    {
      to.Push(addr);
    }else
    {
      while(pos!=string::npos)
      {
        to.Push(addr.substr(startpos,pos-startpos));
        startpos=pos+1;
        pos=addr.find(',',pos+1);
      }
      to.Push(addr.substr(startpos));
    }

    try{
      int rv=SendEMail(fromdecor,to,subj,body);
      if(!haveprofile && cfg::autoCreateGsm2EmlProfile)
      {
        __trace2__("Creating implicit profile for address %s",fromaddress);
        AbonentProfile prof;
        prof.addr=fromaddress;
        if(prof.addr.value[0]=='7')
        {
          prof.addr.type=1;
          prof.addr.plan=1;
        }
        prof.user=fromaddress;
        //prof.forwardEmail;
        //prof.realName;
        prof.ltype=cfg::defaultLimitType;
        prof.limitDate=time(NULL);
        prof.numberMap=false;
        prof.limitValue=cfg::defaultLimitValue;
        prof.limitCountGsm2Eml=1;
        prof.limitCountEml2Gsm=0;
        if(!storage.CreateProfile(prof))
        {
          haveprofile=true;
        }
      }
      if(haveprofile)
      {
        storage.incGsm2EmlLimit(fromaddress);
      }

      if(rv!=ProcessSmsCodes::OK)
      {
        statCollector.IncSms2Eml(false);
        sendAnswer(fromaddress,hdMode,"messagefailedsendmail");
        return rv;
      }else
      {
        statCollector.IncSms2Eml();
      }
      if(cfg::sendSuccessAnswer)
      {
        sendAnswer(fromaddress,hdMode,"messagesent","to",to[0].c_str());
      }
    }catch(std::exception& e)
    {
      __warning2__("SMTP session aborted:%s",e.what());
      sendAnswer(fromaddress,hdMode,"messagefailedsendmail");
      return ProcessSmsCodes::UNABLETOSEND;
    }
    return ProcessSmsCodes::OK;
  }catch(std::exception& e)
  {
    __warning2__("Exception in processSms:%s",e.what());
    sendAnswer(fromaddress,hdMode,"messagefailedsystem");
  }
  return ProcessSmsCodes::NETERROR;
}

class ConcatManager:public smsc::core::threads::Thread{
public:
  ConcatManager()
  {
    lastIdx=0;
    needToStop=false;
    concatTimeout=120;
  }
  void Init(const std::string& argLocation,int argConcatTimeout)
  {
    concatTimeout=argConcatTimeout;
    log=smsc::logger::Logger::getInstance("concat");
    storeLocation=argLocation;
    if(storeLocation.length() && *storeLocation.rbegin()!='/')
    {
      storeLocation+='/';
    }
    std::vector<std::string> dir;
    File::ReadDir(storeLocation.c_str(),dir,File::rdfFilesOnly);
    for(std::vector<std::string>::iterator it=dir.begin();it!=dir.end();it++)
    {
      char ext[8];
      int idx;
      if(sscanf(it->c_str(),"sms%d.%3s",&idx,ext)==2)
      {
        if(strcmp(ext,"bin")!=0)
        {
          smsc_log_info(log,"Skipped unrecognized file:'%s'",it->c_str());
          continue;
        }
        ConcatRecord rec;
        rec.fileIdx=idx;
        if(idx>lastIdx)
        {
          lastIdx=idx;
        }
        ReadRecord(rec);
        RecordKey key(rec.mr,rec.oa);
        rec.tit=timeMap.insert(TimeMap::value_type(rec.lastUpdate,key));
        records.insert(RecordsMap::value_type(key,rec));
      }
    }

  }
  int Execute()
  {
    smsc_log_info(log,"Starting ConcatManager");
    MutexGuard mg(mon);
    while(!needToStop)
    {
      try
      {
        if(timeMap.empty())
        {
          mon.wait();
        }else
        {
          int toSleep=(int)(timeMap.begin()->first-time(NULL))*1000;
          if(toSleep<=0)
          {
            toSleep=1;
          }
          mon.wait(toSleep);
        }
        if(needToStop)
        {
          break;
        }
        if(timeMap.empty())
        {
          continue;
        }
        TimeMap::iterator it=timeMap.begin();
        if(it->first>time(NULL))
        {
          continue;
        }

        RecordKey key(it->second);
        RecordsMap::iterator rit=records.find(key);
        if(rit==records.end())
        {
          smsc_log_warn(log,"Inconsistency: timerecord without record for %d/%s",key.mr,key.oa.c_str());
          timeMap.erase(it);
          continue;
        }
        ConcatRecord& rec=rit->second;
        processSms(rec.combineTxt().c_str(),Address(rec.oa.c_str()).value,Address(rec.da.c_str()).value);
        smsc_log_info(log,"Concat timed out:%d/%s",rec.mr,rec.oa.c_str());
        timeMap.erase(it);
        records.erase(key);
        DelRecord(rec);
      } catch(std::exception& e)
      {
        smsc_log_warn(log,"Exception in ConcatManager::Execute: '%s'",e.what());
      }
    }
    smsc_log_info(log,"Stopped ConcatManager");
    needToStop=false;
    return 0;
  }
  void Stop()
  {
    MutexGuard mg(mon);
    needToStop=true;
    mon.notify();
  }
  bool Enqueue(PduXSm* pdu)
  {
    SMS sms;
    fetchSmsFromSmppPdu(pdu,&sms);
    uint16_t mr;
    uint8_t idx,num;
    if(sms.hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM))
    {
      mr=sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM);
      idx=sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
      num=sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
    }else
    {
      if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)==0)
      {
        return false;
      }
      bool havemoreudh;
      if(!smsc::util::findConcatInfo(sms,mr,idx,num,havemoreudh))
      {
        return false;
      }
    }
    MutexGuard mg(mon);
    RecordKey key(mr,sms.getOriginatingAddress().toString());
    RecordsMap::iterator it=records.find(key);
    if(it==records.end())
    {
      smsc_log_info(log,"Starting concat for %d/%s[num=%d]",mr,sms.getOriginatingAddress().toString().c_str(),num);
      ConcatRecord rec;
      rec.fileIdx=lastIdx++;
      rec.oa=sms.getOriginatingAddress().toString().c_str();
      rec.da=sms.getDestinationAddress().toString().c_str();
      rec.num=num;
      rec.mr=mr;
      ConcatRecord::Part part(idx,"");
      getSmsText(&sms,part.txt);
      rec.lastUpdate=time(NULL);
      rec.parts.insert(ConcatRecord::PartsMap::value_type(idx,part));
      rec.tit=timeMap.insert(TimeMap::value_type(rec.lastUpdate+concatTimeout,key));
      records.insert(RecordsMap::value_type(key,rec)).second;
      WriteRecord(rec);
      mon.notify();
      return true;
    }
    ConcatRecord& rec=it->second;
    if(rec.parts.find(idx)!=rec.parts.end())
    {
      smsc_log_info(log,"Duplicate part %d for %d/%s",idx,rec.mr,rec.oa.c_str());
      return true;
    }

    ConcatRecord::Part part(idx,"");
    getSmsText(&sms,part.txt);
    rec.parts.insert(ConcatRecord::PartsMap::value_type(idx,part));
    if(rec.parts.size()==rec.num)
    {
      smsc_log_info(log,"Concat complete for %d/%s",rec.mr,rec.oa.c_str());
      std::string txt=rec.combineTxt();
      processSms(txt.c_str(),Address(rec.oa.c_str()).value,Address(rec.da.c_str()).value);
      timeMap.erase(rec.tit);
      records.erase(it);
      DelRecord(rec);
      return true;
    }
    smsc_log_info(log,"Received %d/%d parts for %d/%s",rec.parts.size(),num,rec.mr,rec.oa.c_str());
    rec.lastUpdate=time(NULL);
    timeMap.erase(rec.tit);
    rec.tit=timeMap.insert(TimeMap::value_type(rec.lastUpdate+concatTimeout,key));
    WriteRecord(rec);
    mon.notify();
    return true;
  }
protected:

  struct RecordKey{
    int mr;
    std::string oa;
    RecordKey(int argMr,const std::string& argOa):mr(argMr),oa(argOa)
    {
    }
    bool operator<(const RecordKey& key)const
    {
      return mr<key.mr || (mr==key.mr && oa<key.oa);
    }
  };

  typedef std::multimap<time_t,RecordKey> TimeMap;

  struct ConcatRecord{
    struct Part{
      Part()
      {
      }
      Part(int argSeq,const std::string& argTxt):seq(argSeq),txt(argTxt)
      {
      }
      int seq;
      std::string txt;
    };
    typedef std::map<int,Part> PartsMap;
    PartsMap parts;
    int num;
    int mr;
    std::string oa;
    std::string da;
    time_t lastUpdate;
    TimeMap::iterator tit;
    int fileIdx;

    void Write(File& f)const
    {
      f.WriteNetInt32(num);
      f.WriteNetInt32(mr);
      WriteString(f,oa);
      WriteString(f,da);
      f.WriteNetInt64(lastUpdate);
      f.WriteNetInt16((uint16_t)parts.size());
      for(PartsMap::const_iterator it=parts.begin();it!=parts.end();it++)
      {
        f.WriteNetInt32(it->second.seq);
        WriteString(f,it->second.txt);
      }
    }

    void Read(File& f)
    {
      num=f.ReadNetInt32();
      mr=f.ReadNetInt32();
      ReadString(f,oa);
      ReadString(f,da);
      lastUpdate=f.ReadNetInt64();
      int count=f.ReadNetInt16();
      for(int i=0;i<count;i++)
      {
        Part p;
        p.seq=f.ReadNetInt32();
        ReadString(f,p.txt);
        parts.insert(PartsMap::value_type(p.seq,p));
      }
    }

    std::string combineTxt()
    {
      std::string rv;
      for(PartsMap::iterator it=parts.begin();it!=parts.end();it++)
      {
        rv+=it->second.txt;
      }
      return rv;
    }
  };

  std::string mkFileName(const ConcatRecord& rec)
  {
    char buf[32];
    sprintf(buf,"sms%08d.bin",rec.fileIdx);
    return storeLocation+buf;
  }

  void ReadRecord(ConcatRecord& rec)
  {
    std::string fileName=mkFileName(rec);
    File f;
    f.ROpen(fileName.c_str());
    rec.Read(f);
    f.Flush();
  }
  void WriteRecord(const ConcatRecord& rec)
  {
    std::string fileName=mkFileName(rec);
    File f;
    f.RWCreate(fileName.c_str());
    rec.Write(f);
    f.Flush();
  }
  void DelRecord(const ConcatRecord& rec)
  {
    std::string fileName=mkFileName(rec);
    File::Unlink(fileName.c_str());
  }

  typedef std::map<RecordKey,ConcatRecord> RecordsMap;
  RecordsMap records;
  TimeMap timeMap;
  EventMonitor mon;

  bool needToStop;
  int lastIdx;
  int concatTimeout;
  std::string storeLocation;
  smsc::logger::Logger* log;
}concatManager;

class MyListener:public SmppPduEventListener,public IAnswerQueue{
public:
  MyListener()
  {
    trans=0;
    log=smsc::logger::Logger::getInstance("smpp.lst");
  }

  void enqueueAnswer(PduSubmitSm* pdu)
  {
    MutexGuard mg(mtx);
    answers.push_back(pdu);
  }


  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      char buf[65536];
      PduXSm* xsm=(PduXSm*)pdu;

      int code=ProcessSmsCodes::INVALIDSMS;
      bool concatRv=false;
      try{
        if(cfg::partitionSms)
        {
          concatRv=concatManager.Enqueue(xsm);
        }
      }catch(std::exception& e)
      {
        smsc_log_warn(log,"Exception in concatManager.Enqueue:'%s'",e.what());
      }
      if(!concatRv)
      {
        try{
          getPduText(xsm,buf,sizeof(buf));
          /*Address addr(
           ((PduXSm*)pdu)->get_message().get_source().value.size(),
           ((PduXSm*)pdu)->get_message().get_source().get_typeOfNumber(),
           ((PduXSm*)pdu)->get_message().get_source().get_numberingPlan(),
           ((PduXSm*)pdu)->get_message().get_source().get_value());*/
          code=processSms(buf,xsm->get_message().get_source().get_value(),xsm->get_message().get_dest().get_value());
        }catch(std::exception& e)
        {
          __warning2__("Exception in processSms:%s",e.what());
        }
        __trace2__("processSms: code=%d",code);
        switch(code)
        {
          case ProcessSmsCodes::OUTOFLIMIT:
          case ProcessSmsCodes::NOPROFILE:
          case ProcessSmsCodes::UNABLETOSEND:
          case ProcessSmsCodes::INVALIDSMS:code=SmppStatusSet::ESME_ROK;break;//ESME_RX_P_APPN
          case ProcessSmsCodes::NETERROR:code=SmppStatusSet::ESME_RX_T_APPN;break;
          case ProcessSmsCodes::OK:code=SmppStatusSet::ESME_ROK;break;
        }
      }else
      {
        code=SmppStatusSet::ESME_ROK;
      }
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.get_header().set_commandStatus(code);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      trans->sendDeliverySmResp(resp);
      sendAnswers();
    }else if(pdu->get_commandId()==SmppCommandSet::SUBMIT_SM_RESP)
    {
      if(pdu->get_commandStatus()==0)
      {
        __trace__("Received ok resp on answer submit");
      }else
      {
        __warning2__("Answer submit failed with error:%d",pdu->get_commandStatus());
      }
    }
    disposePdu(pdu);
  }
  void handleError(int errorCode)
  {
    __warning2__("SMPP::Error:%d",errorCode);
    pthread_kill(cfg::mainId,16);
  }

  void sendAnswers()
  {
    MutexGuard mg(mtx);
    while(!answers.empty())
    {
      cfg::atr->submit(*answers.back());
      delete answers.back();
      std::vector<PduSubmitSm*>::iterator it=answers.end();
      it--;
      answers.erase(it);
    }
  }

  bool handleIdle()
  {
    return false;
  }

  void setTrans(SmppTransmitter *t)
  {
    trans=t;
  }
protected:
  SmppTransmitter* trans;
  Mutex mtx;
  std::vector<PduSubmitSm*> answers;
  smsc::logger::Logger* log;
};

bool GetNextLine(const char* text,size_t maxlen,size_t& pos,string& line)
{
  size_t start=pos;
  if(pos>=maxlen)return false;
  do{
    while(pos<maxlen && text[pos]!=0x0d && text[pos]!=0x0a)pos++;
    line.assign(text+start,pos-start);
    if(text[pos]==0x0d && pos<maxlen)pos++;
    if(text[pos]==0x0a && pos<maxlen)pos++;
  }while(pos<maxlen && (text[pos]==' ' || text[pos]=='\t'));
  return true;
}

/*
void IncUsageCounter(const string& address)
{

  using namespace smsc::db;
  __trace2__("inc counter for %s",address.c_str());
  const char* sql="insert into EMLSME_HISTORY (address,MSG_DATE) values (:1,:2)";
  ConnectionGuard connection(cfg::dataSource);
  if(!connection.get())throw Exception("Failed to get db connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Failed to prepare statement");
  statement->setString(1,address.c_str());
  statement->setDateTime(2,time(NULL));
  statement->executeUpdate();
  connection->commit();
}
*/

std::string RxSubst(const std::string& src,const std::string& pat,SMatch* m,int n)
{
  std::string result;
  for(int i=0;i<pat.length();i++)
  {
    if(pat[i]!='$')
    {
      result+=pat[i];
    }
    else
    {
      if(i<pat.length()+1)
      {
        if(!isdigit(pat[i+1]))
        {
          result+=pat[i+1];
          i++;
        }
        else
        {
          int idx,sz;
          if(sscanf(pat.c_str()+i+1,"%d%n",&idx,&sz)==1)
          {
            if(idx<n)
            {
              result+=src.substr(m[idx].start,m[idx].end-m[idx].start);
            }
            i+=sz;
          }
        }
      }
    }
  }
  return result;
}

bool CheckDomain(const std::string eml)
{
  for(int i=0;i<cfg::validDomains.size();i++)
  {
    if(eml.substr(eml.find('@')+1)==cfg::validDomains[i])
    {
      return true;
    }
  }
  return false;
}

class MultipartSendQueue:public smsc::core::threads::Thread{
public:
  MultipartSendQueue()
  {
    needToStop=false;
    nextFileIdx=0;
    sendDelay=30000;
  }
  void Init(const std::string& storeDir,int argSendDelay)
  {
    log=smsc::logger::Logger::getInstance("msqueue");
    sendDelay=argSendDelay;
    storeLocation=storeDir;
    if(storeLocation.length() && *storeLocation.rbegin()!='/')
    {
      storeLocation+='/';
    }
    std::vector<std::string> dirContent;
    File::ReadDir(storeLocation.c_str(),dirContent,File::rdfFilesOnly);
    for(std::vector<std::string>::iterator it=dirContent.begin();it!=dirContent.end();it++)
    {
      char ext[8];
      int idx;
      if(sscanf(it->c_str(),"sms%d.%3s",&idx,ext)==2)
      {
        if(strcmp(ext,"bin")!=0)
        {
          smsc_log_info(log,"Skipped unrecognized file:'%s'",it->c_str());
          continue;
        }
        SmsInfo info;
        info.fileIdx=idx;
        if(idx>nextFileIdx)
        {
          nextFileIdx=idx+1;
        }
        File f;
        std::string fileName=mkFileName(info);
        if(!File::Exists(fileName.c_str()))
        {
          smsc_log_info(log,"Skipped file with wrong filename format:'%s'",it->c_str());
          continue;
        }
        f.ROpen(fileName.c_str());
        File::offset_type sz=f.Size();
        BufOps::SmsBuffer buf(sz);
        f.Read(buf.get(),sz);
        info.sms=new SMS;
        Deserialize(buf,*info.sms,0x10001);
        smsQueue.push_back(info);
        smsc_log_info(log,"Loaded file:'%s'",fileName.c_str());
      }else
      {
        smsc_log_info(log,"Skipped unrecognized file:'%s'",it->c_str());
      }
    }
  }

  int Execute()
  {
    MutexGuard mg(mon);
    smsc_log_info(log,"Starting MultipartSendQueue");
    while(!needToStop)
    {
      while(!needToStop && smsQueue.empty())
      {
        mon.wait();
      }
      if(needToStop)
      {
        break;
      }
      if(smsQueue.empty())
      {
        continue;
      }
      SmsQueue::iterator it=smsQueue.begin();
      SMS& sms=*it->sms;
      ConcatInfo* ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,0);
      PduSubmitSm sm;
      sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
      PduSubmitSmResp *resp=0;
      while(!needToStop)
      {
        mon.wait(sendDelay);
        if(needToStop)
        {
          break;
        }
        SMS psms=sms;
        if(ci->num<=cfg::maxUdhParts)
        {
          psms.setIntProperty(Tag::SMSC_UDH_CONCAT,1);
        }
        extractSmsPart(&psms,psms.concatSeqNum);
        fillSmppPduFromSms(&sm,&psms);
        try
        {
          mon.Unlock();
          resp=0;
          smsc_log_info(log,"Send part %s->%s %d/%d",sms.getOriginatingAddress().toString().c_str(),
                        sms.getDestinationAddress().toString().c_str(),sms.concatSeqNum,ci->num);
          resp=cfg::tr->submit(sm);
          mon.Lock();
        } catch(...)
        {
          mon.Lock();
        }
        if(!resp || resp->get_header().get_commandStatus()!=SmppStatusSet::ESME_ROK)
        {
          if(resp)
          {
            int code=resp->get_header().get_commandStatus();
            if(smsc::system::Status::isErrorPermanent(code))
            {
              smsc_log_warn(log,"SMS delivery failed with permanent error:%s->%s:%d",sms.getOriginatingAddress().toString().c_str(),
                            sms.getDestinationAddress().toString().c_str(),resp->get_header().get_commandStatus());
              smsQueue.erase(it);
              disposePdu((SmppHeader*)resp);
              break;
            }
            if(code==SmppStatusSet::ESME_RMSGQFUL)
            {
              disposePdu((SmppHeader*)resp);
              smsQueue.push_back(*it);
              smsQueue.erase(it);
              break;
            }
          }
          if(resp)disposePdu((SmppHeader*)resp);
          continue;
        }
        disposePdu((SmppHeader*)resp);
        sms.concatSeqNum++;
        if(sms.concatSeqNum>=ci->num)
        {
          smsc_log_info(log,"SMS delivery successfully completed:%s->%s",sms.getOriginatingAddress().toString().c_str(),
                        sms.getDestinationAddress().toString().c_str());
          DeleteSms(*it);
          smsQueue.erase(it);
          break;
        }
        SaveSms(*it);
      }
    }
    smsc_log_info(log,"Exiting MultipartSendQueue");
    needToStop=false;
    return 0;
  }

  void Stop()
  {
    MutexGuard mg(mon);
    needToStop=true;
    mon.notify();
  }

  void enqueue(const SMS& sms)
  {
    MutexGuard mg(mon);
    bool wasEmpty=smsQueue.empty();
    SmsInfo info;
    info.fileIdx=nextFileIdx++;
    info.sms=new SMS(sms);
    smsQueue.push_back(info);
    SaveSms(info);
    if(wasEmpty)
    {
      mon.notify();
    }
  }

protected:
  struct SmsInfo{
    SMS* sms;
    int fileIdx;
  };
  typedef std::list<SmsInfo> SmsQueue;
  SmsQueue smsQueue;
  smsc::core::synchronization::EventMonitor mon;
  bool needToStop;
  int nextFileIdx;
  std::string storeLocation;
  int sendDelay;
  smsc::logger::Logger* log;

  std::string mkFileName(const SmsInfo& info)
  {
    return storeLocation+smsc::util::format("sms%08d.bin",info.fileIdx);
  }
  void SaveSms(const SmsInfo& info)
  {
    smsc::sms::BufOps::SmsBuffer buf(0);
    smsc::sms::Serialize(*info.sms,buf);
    std::string fileName=mkFileName(info);
    smsc::core::buffers::File f;
    f.RWCreate(fileName.c_str());
    f.Write(buf.get(),buf.GetPos());
    f.Flush();
  }
  void DeleteSms(const SmsInfo& info)
  {
    smsc::core::buffers::File::Unlink(mkFileName(info).c_str());
    delete info.sms;
  }
}multiPartSendQueue;


struct XBuffer{
  char* buffer;
  int size;
  int offset;

  XBuffer(){buffer=0;size=0;offset=0;}
  ~XBuffer(){if(buffer)delete [] buffer;}

  void setSize(int newsize)
  {
    if(newsize<size)return;
    char *newbuf=new char[newsize];
    if(offset)memcpy(newbuf,buffer,offset);
    if(buffer)delete [] buffer;
    buffer=newbuf;
    size=newsize;
  }
  void append(char *mem,int count)
  {
    if(offset+count>size)setSize((offset+count)+(offset+count)/2);
    memcpy(buffer+offset,mem,count);
    offset+=count;
  }
  char* current(){return buffer+offset;}
  int freeSpace(){return size-offset;}
};


class EmailProcessor:public smsc::core::threads::Thread{
public:
  EmailProcessor()
  {
    busy=false;
    needToStop=false;
    log=smsc::logger::Logger::getInstance("eml.proc");
    clnt=0;
    emlOut=0;
    emlIn=0;
    needRestartChild=false;
    mr=0;
  }
  int Execute()
  {
    idx=util::InitPipeThread();
    XBuffer buf;
    while(!needToStop)
    {
      {
        MutexGuard mg(mon);
        if(busy)
        {
          busyCount--;
          busy=false;
          mon.notifyAll();
        }
        while(!clnt && !needToStop)
        {
          mon.wait();
        }
        if(needToStop)break;
        if(!clnt)continue;
      }
      int sz;
      clnt->setTimeOut(10);
      smsc_log_debug(log,"Got connection");
      if(clnt->ReadAll((char*)&sz,4)==-1)continue;
      sz=ntohl(sz);
      smsc_log_debug(log,"Message size:%d",sz);
      buf.setSize(sz+1);
      if(clnt->ReadAll(buf.buffer,sz)==-1)continue;
      smsc_log_debug(log,"Processing message");
      int retCode;
      try{
        buf.buffer[sz]=0;
        static smsc::logger::Logger* dumplog=smsc::logger::Logger::getInstance("msgdmp");
        smsc_log_debug(dumplog,"msgdump:\n=== begin ===\n%s\n=== end ===",buf.buffer);
        retCode=ProcessMessage(buf.buffer,sz);
      }catch(exception& e)
      {
        smsc_log_warn(log,"process message failed:%s",e.what());
        retCode=StatusCodes::STATUS_CODE_UNKNOWNERROR;
      }
      smsc_log_debug(log,"Processing finished, code=%d",retCode);
      retCode=htonl(retCode);
      clnt->WriteAll(&retCode,4);
      delete clnt;
      clnt=0;
    }
    if(emlIn)
    {
      fclose(emlIn);
      emlIn=0;
    }
    if(emlOut)
    {
      fclose(emlOut);
      emlOut=0;
    }
    return 0;
  }
  int sendSms(std::string from,const std::string to,const char* msg,int msglen);
  int ProcessMessage(const char *msg,int msglen);
  static void startEmailProcessing(Socket* s);
  void assignTransformRegexp(const char* reSrc)
  {
    reTransform.Compile(reSrc,OP_OPTIMIZE);
  }
  bool isBusy()const
  {
    return busy;
  }
  void processSocket(Socket* s)
  {
    clnt=s;
    busy=true;
    busyCount++;
  }
  void stop()
  {
    needToStop=true;
    MutexGuard mg(mon);
    mon.notifyAll();
  }
protected:
  int idx;
  bool needToStop;
  RegExp reTransform;
  FILE* emlIn;
  FILE* emlOut;
  Socket* clnt;
  bool busy;
  bool needRestartChild;
  int mr;
  static smsc::logger::Logger* log;
  static int busyCount;
  static EventMonitor mon;
};

int EmailProcessor::busyCount=0;
EventMonitor EmailProcessor::mon;
smsc::logger::Logger* EmailProcessor::log=0;

void EmailProcessor::startEmailProcessing(Socket* s)
{
  MutexGuard mg(mon);
  while(busyCount==cfg::mailthreadsCount)
  {
    smsc_log_debug(log,"all mail processing threads are busy. waiting");
    mon.wait();
  }
  for(int i=0;i<cfg::mailthreadsCount;i++)
  {
    if(!cfg::mailThreads[i].isBusy())
    {
      smsc_log_debug(log,"starting mail processing on thread %d",i);
      cfg::mailThreads[i].processSocket(s);
      mon.notifyAll();
      break;
    }
  }
}

int EmailProcessor::sendSms(std::string from,const std::string to,const char* msg,int msglen)
{
  AbonentProfile p;
  bool noProfile=true;


  string dstUser=to.substr(0,to.find('@'));
  string toDomain;
  size_t atPos=to.find('@');
  if(atPos!=string::npos)
  {
    toDomain=to.substr(atPos+1);
  }

  toLower(dstUser);

  if(cfg::useTransformRegexp)
  {
    SMatch m[10];
    int n=10;
    if(reTransform.Match(dstUser.c_str(),m,n))
    {
      smsc_log_debug(log,"performing transformation for username %s",dstUser.c_str());
      dstUser=RxSubst(dstUser,cfg::transformResult,m,n);
      smsc_log_debug(log,"transformation result:%s",dstUser.c_str());
    }
  }

  if(!storage.getProfileByEmail(dstUser.c_str(),p))
  {
    smsc_log_debug(log,"no profile for user:%s",dstUser.c_str());
    std::string addr=dstUser;
    if(storage.getProfileByAddress(addr.c_str(),p))
    {
      if(!p.numberMap)
      {
        smsc_log_debug(log,"number map turned off for address:%s",addr.c_str());
        return StatusCodes::STATUS_CODE_NOUSER;
      }
      noProfile=false;
    }else
    {
      smsc_log_debug(log,"no profile for address:%s",dstUser.c_str());
      //return StatusCodes::STATUS_CODE_NOUSER;
    }
  }else
  {
    noProfile=false;
  }

  if(!cfg::allowEml2GsmWithoutProfile && noProfile)
  {
    return StatusCodes::STATUS_CODE_NOUSER;
  }


  if(!noProfile)
  {
    if(!storage.checkEml2GsmLimit(dstUser.c_str()))
    {
      smsc_log_debug(log,"limit exceeded for user:%s",dstUser.c_str());
      return StatusCodes::STATUS_CODE_LIMIEXCEEDED;
    }
  }

  if(!util::isChildRunning(idx) || needRestartChild)
  {
    smsc_log_debug(log,"forking mailstripper[%d] child:%s",idx,cfg::mailstripper.c_str());
    if(emlIn){fclose(emlIn);emlIn=0;}
    if(emlOut){fclose(emlOut);emlOut=0;}
    if(util::ForkPipedCmd(idx,cfg::mailstripper.c_str(),emlIn,emlOut)<=0)
    {
      smsc_log_debug(log,"failed to fork mailstripper child:%s",strerror(errno));
      return StatusCodes::STATUS_CODE_TEMPORARYERROR;
    }
    needRestartChild=false;
  }
  smsc_log_debug(log,"write msg size:%d",msglen);
  fprintf(emlOut,"%d\n",msglen);fflush(emlOut);
  smsc_log_debug(log,"write msg");
  size_t sz=0;
  while(sz<msglen)
  {
    size_t wr=fwrite(msg+sz,1,msglen-sz,emlOut);fflush(emlOut);
    if(wr==0)
    {
      smsc_log_warn(log,"failed to write data for mailstripper");
      needRestartChild=true;
      return StatusCodes::STATUS_CODE_TEMPORARYERROR;
    }
    sz+=wr;
  }
  smsc_log_debug(log,"read resp len");
  char buf[16];
  if(!fgets(buf,(int)sizeof(buf),emlIn))
  {
    smsc_log_warn(log,"failed to read data from mailstripper");
    needRestartChild=true;
    return StatusCodes::STATUS_CODE_TEMPORARYERROR;
  }
  int len=atoi(buf);
  smsc_log_debug(log,"resp len=%d",len);
  auto_ptr<char> newmsg(new char[len+1]);

  sz=0;
  while(sz<len)
  {
    size_t rv=fread(newmsg.get(),1,len-sz,emlIn);
    if(rv==0)
    {
      smsc_log_debug(log,"failed to read data from mailstripper");
      return StatusCodes::STATUS_CODE_TEMPORARYERROR;
    }
    sz+=rv;
  }
  newmsg.get()[len]=0;

  smsc_log_debug(log,"newmsg:%s",newmsg.get());

  //cfg::annotationSize

  smsc_log_debug(log,"toDomain=%s, hdmaildomain=%s",toDomain.c_str(),cfg::hdmaildomain.c_str());

  SMS sms;
  sms.setOriginatingAddress(toDomain==cfg::hdmaildomain?cfg::helpDeskAddress.c_str():cfg::sourceAddress.c_str());

  if(noProfile)
  {
    bool numberOk=true;
    for(int i=0;i<dstUser.length();i++)
    {
      if(!isdigit(dstUser[i]))
      {
        numberOk=false;
        break;
      }
    }
    if(!numberOk)return StatusCodes::STATUS_CODE_NOUSER;
  }

  string fwd;
  string dst=noProfile?'+'+dstUser:MapEmailToAddress(dstUser,fwd);
  if(fwd.length())
  {
    smsc_log_debug(log,"forwarding email to %s",fwd.c_str());
    smsc_log_debug(log,"fwd body(%d):%s",msglen,msg);
    try{
      Array<string> to2;
      to2.Push(fwd);
      string body(msg,msglen);
      SendEMail(from,to2,"",body,true);
    }catch(exception& e)
    {
      smsc_log_warn(log,"Failed to forward msg to %s:%s",fwd.c_str(),e.what());
    }
  }

  size_t pos=0;
  string subj;
  GetNextLine(newmsg.get(),len,pos,from);
  GetNextLine(newmsg.get(),len,pos,subj);
  string text=newmsg.get()+pos;

  ContextEnvironment ce;
  EmptyGetAdapter ga;

  size_t fldlen=subj.length()+from.length()+to.length();
  if(text.length()>cfg::annotationSize-fldlen)
  {
    text=text.substr(0,cfg::annotationSize-fldlen);
  }

  pos=0;
  while((pos=text.find("\"",pos))!=string::npos)
  {
    text.at(pos)='\'';
  }
  pos=0;
  while((pos=subj.find("\"",pos))!=string::npos)
  {
    subj.at(pos)='\'';
  }

  ce.exportStr("from",ExtractEmail(from).c_str());
  ce.exportStr("to",to.c_str());
  ce.exportStr("subj",subj.c_str());
  ce.exportStr("body",text.c_str());

  text="";
  cfg::msgFormat->format(text,ga,ce);

  smsc_log_debug(log,"result:%s",text.c_str());



  sms.setDestinationAddress(dst.c_str());
  char msc[]="";
  char imsi[]="";
  sms.setOriginatingDescriptor((uint8_t)strlen(msc),msc,(uint8_t)strlen(imsi),imsi,1);
  sms.setDeliveryReport(0);
  sms.setArchivationRequested(false);
  sms.setEServiceType(cfg::serviceType.c_str());
  sms.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,cfg::protocolId);

  sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS,0);
  if(hasHighBit(text.c_str(),text.length()))
  {
    auto_ptr<short> ucs2(new short[text.length()+1]);
    ConvertMultibyteToUCS2(text.c_str(),text.length(),ucs2.get(),(text.length()+1)*2,CONV_ENCODING_CP1251);
    sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::UCS2);
    sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,(char*)ucs2.get(),(unsigned)text.length()*2);
  }else
  {
    sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
    sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,text.c_str(),(unsigned)text.length());
  }
  //sms.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,text.length());
  PduSubmitSm sm;
  sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
  PduSubmitSmResp *resp=0;

  if(cfg::partitionSms)
  {
    sms.setIntProperty(Tag::SMSC_DSTCODEPAGE,DataCoding::UCS2);
    int pres=partitionSms(&sms);
    if(pres==psSingle)
    {
      fillSmppPduFromSms(&sm,&sms);
      resp=cfg::tr->submit(sm);
    }else if(pres==psMultiple)
    {
      SMS psms=sms;
      ConcatInfo* ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,0);
      if(ci->num<=cfg::maxUdhParts)
      {
        psms.setIntProperty(Tag::SMSC_UDH_CONCAT,1);
      }
      int newMr=mr++;
      psms.setConcatMsgRef(newMr);
      sms.setConcatMsgRef(newMr);
      extractSmsPart(&psms,0);
      fillSmppPduFromSms(&sm,&psms);
      resp=cfg::tr->submit(sm);
      if(resp && resp->get_header().get_commandStatus()==SmppStatusSet::ESME_ROK)
      {
        sms.concatSeqNum=1;
        multiPartSendQueue.enqueue(sms);
      }
    }else
    {
      smsc_log_warn(log,"Failed to partition sms %s->%s",sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str());
      return StatusCodes::STATUS_CODE_INVALIDMSG;
    }

  }
  else
  {
    fillSmppPduFromSms(&sm,&sms);
    resp=cfg::tr->submit(sm);
  }
  int rc=StatusCodes::STATUS_CODE_TEMPORARYERROR;
  if(resp)
  {
    smsc_log_info(log,"Received resp with statusCode=%d",resp->get_header().get_commandStatus());
    switch(resp->get_header().get_commandStatus())
    {
      case SmppStatusSet::ESME_ROK:rc=StatusCodes::STATUS_CODE_OK;break;
      case SmppStatusSet::ESME_RINVDSTADR:rc=StatusCodes::STATUS_CODE_NOUSER;break;
      default:rc=StatusCodes::STATUS_CODE_UNKNOWNERROR;break;
    }
    if(rc==StatusCodes::STATUS_CODE_OK)
    {
      if(noProfile)
      {
        smsc_log_info(log,"Creating implicit profile creation for address %s",dst.c_str());
        AbonentProfile prof;
        prof.addr=dst.c_str();
        if(prof.addr.value[0]=='7')
        {
          prof.addr.type=1;
          prof.addr.plan=1;
        }
        prof.user=dstUser.c_str();
        //prof.forwardEmail;
        //prof.realName;
        prof.ltype=cfg::defaultLimitType;
        prof.limitDate=time(NULL);
        prof.numberMap=false;
        prof.limitValue=cfg::defaultLimitValue;
        prof.limitCountGsm2Eml=0;
        prof.limitCountEml2Gsm=0;
        storage.CreateProfile(prof);
      }
      try{
        storage.incEml2GsmLimit(dstUser.c_str());
      }catch(exception& e)
      {
        smsc_log_warn(log,"failed to inc counter:%s",e.what());
      }
    }
  }else
  {
    smsc_log_info(log,"submit sms resp timed out");
  }
  if(rc==StatusCodes::STATUS_CODE_OK)
  {
    statCollector.IncEml2Sms();
  }else
  {
    statCollector.IncEml2Sms(false);
  }
  if(resp)
  {
    disposePdu((SmppHeader*)resp);
  }
  return rc;
}

int EmailProcessor::ProcessMessage(const char *msg,int msglen)
{
  string line,name,value,from,to;
  std::vector<std::string> toarr;
  bool inheader=true;
  size_t pos=0;
  for(;;)
  {
    if(!GetNextLine(msg,msglen,pos,line))break;
    if(inheader)
    {
      if(line.length()==0)
      {
        inheader=false;
        continue;
      }
      size_t pos2=line.find(':');
      if(pos2==string::npos)continue;
      name=line.substr(0,pos2);
      toLower(name);
      pos2++;
      while(pos2<line.length() && line[pos2]==' ')pos2++;
      value=line.substr(pos2);

      if(name=="from")
      {
        from=ExtractEmail(value);
        if(from.length()==0)return StatusCodes::STATUS_CODE_INVALIDMSG;
        continue;
      }
      if(name=="x-to")
      {
        to=value;
        if(to.find('@')==std::string::npos)
        {
          to+='@';
          to+=cfg::maildomain;
        }
        toarr.push_back(to);
        continue;
      }
      if(name=="to" || name=="cc")
      {
        size_t emlpos=0;
        do{
          to=ExtractEmail(value,emlpos);
          if(to.length()>0)
          {
            if(CheckDomain(to))
            {
              toarr.push_back(to);
            }else
            {
              __warning2__("email '%s' doesn't pass domain check",to.c_str());
            }
          }
          emlpos=value.find(',',emlpos+1);
        }while(emlpos!=std::string::npos);
        continue;
      }
      continue;
    }
    break;
  }

  if(from.length()==0 || toarr.size()==0)
  {
    return StatusCodes::STATUS_CODE_INVALIDMSG;
  }

  {
    std::set<std::string> toset;
    for(int i=0;i<toarr.size();i++)
    {
      toset.insert(toarr[i]);
    }
    toarr.clear();
    for(std::set<std::string>::iterator it=toset.begin();it!=toset.end();it++)
    {
      toarr.push_back(*it);
    }
  }

  int rc=StatusCodes::STATUS_CODE_INVALIDMSG;

  for(int i=0;i<toarr.size();i++)
  {
    int localrc=sendSms(from,toarr[i],msg,msglen);
    __trace2__("rc=%d from eml=%s",localrc,toarr[i].c_str());
    if(localrc==StatusCodes::STATUS_CODE_OK)
    {
      rc=StatusCodes::STATUS_CODE_OK;
    }
    if(rc==StatusCodes::STATUS_CODE_INVALIDMSG)
    {
      rc=localrc;
    }
  }
  return rc;
}


bool reconnectFlag=false;

extern "C"  void disp(int sig)
{
  reconnectFlag=true;
}

extern "C"  void ctrlc(int sig)
{
  cfg::stopSme=1;
  if(pthread_self()!=cfg::mainId)
  {
    pthread_kill(cfg::mainId,16);
  }
}


extern "C" void atExitHandler(void)
{
  smsc::util::xml::TerminateXerces();
  smsc::logger::Logger::Shutdown();
}

void initRegions(const char* regions_xml_file,const char* route_xml_file)
{
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.infosme.InfoSme");
  using namespace smsc::util::config::region;

  RegionsConfig* regionsConfig=new RegionsConfig(regions_xml_file);

  RegionsConfig::status st = regionsConfig->load();
  if ( st == RegionsConfig::success )
  {
      smsc_log_info(log, "config file %s has been loaded successful", regions_xml_file);
  }
  else
  {
      throw smsc::util::config::ConfigException("can't load config file %s", regions_xml_file);
  }

  smsc::util::config::route::RouteConfig routeConfig;
  if ( routeConfig.load(route_xml_file) == smsc::util::config::route::RouteConfig::success )
  {
    smsc_log_info(log, "config file %s has been loaded successful", route_xml_file);
  }
  else
  {
    throw smsc::util::config::ConfigException("can't load config file %s", route_xml_file);
  }

  Region* region;
  RegionsConfig::RegionsIterator regsIter = regionsConfig->getIterator();

  RegionFinder& rf=RegionFinder::getInstance();
  while (regsIter.fetchNext(region) == RegionsConfig::success)
  {
    region->expandSubjectRefs(routeConfig);
    Region::MasksIterator maskIter = region->getMasksIterator();
    std::string addressMask;
    while(maskIter.fetchNext(addressMask))
    {
      Address addr(addressMask.c_str());
      rf.registerAddressMask(addr.value, region);
    }
  }

  rf.registerDefaultRegion(&(regionsConfig->getDefaultRegion()));
}



int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  atexit(atExitHandler);

  sigset(16,disp);
  sigset(SIGINT,ctrlc);
  sigset(SIGTERM,ctrlc);
  smsc::logger::Logger *log=smsc::logger::Logger::getInstance("emlsme");
  smsc_log_info(log,"\n====================\n%s\n====================\n",getStrVersion());

  if(gethostname(hostName,(int)sizeof(hostName))!=0)
  {
    smsc_log_warn(log,"gethostname failed:%d",errno);
    strcpy(hostName,"localhost");
  }

  cfg::mainId=pthread_self();

  RegExp::InitLocale();
  /*
  reParseSms.Compile(
"/(#(?{flag}\\w)#)?"                         // optional control flag
"(?{address}(?:.*?[@*][^#\\(\\s,]+,?)+)"     // mandatory address(es)
"(#(?{realname}[^#(]+))?"                    // optional real name
"(?:##(?{subj}.*?)#|\\((?{subj}.*?)\\)|\\s)" // optinal subject
"(?{body}.+)$/xs"                            // body
);
*/
  if(!reParseSms.Compile(
"/^\\s*(?:"
"alias\\s+[\\w.\\-]*|"
"noalias|"
"forward\\s+[\\w\\-+\\.]+@[\\w\\-\\]+\\.[\\w\\-\\.]+|"
"forwardoff|"
"noforward|"
"number\\s+on|"
"number\\s+off|"
"realname\\s+.*|"
"norealname|"
"(?{address}[\\w\\-\\.]+@[\\w\\-\\.]+)(?:\\s(?:subj=)?['\"](?{subj}.*?)['\"])?\\s*(?{body}.*)"
")\\s*$/isx"
))
  {
    fprintf(stderr,"Failed to compile parseregexp\n");
    return -1;
  }


  try{

    using namespace smsc::util;
    //using namespace smsc::db;
    config::Manager::init("conf/config.xml");
    config::Manager& cfgman= config::Manager::getInstance();

    config::ConfigView *dsConfig = new config::ConfigView(cfgman, "StartupLoader");


    cfg::smtpHost=cfgman.getString("smtp.host");
    try{
      cfg::smtpPort=cfgman.getInt("smtp.port");
    }catch(...)
    {
      __warning__("smpp.port not found, using default");
    }
    try{
      cfg::retryTime=cfgman.getInt("smpp.retryTime");
    }catch(...)
    {
      __warning__("smpp.retryTime not found, using default");
    }

    try
    {
      cfg::pauseAfterDisconnect=cfgman.getBool("smpp.pauseAfterDisconnect");

    } catch(...)
    {
      __warning__("smpp.pauseAfterDisconnect not found, disabled");
    }

    try{
      cfg::allowGsm2EmlWithoutProfile=cfgman.getBool("admin.allowGsm2EmlWithoutProfile");
    }catch(...)
    {
      __warning__("admin.allowGsm2EmlWithoutProfile not found, disabled by default");
    }

    try{
      cfg::allowEml2GsmWithoutProfile=cfgman.getBool("admin.allowEml2GsmWithoutProfile");
    }catch(...)
    {
      __warning__("admin.allowEml2GsmWithoutProfile not found, disabled by default");
    }

    try{
      cfg::autoCreateGsm2EmlProfile=cfgman.getBool("admin.autoCreateGsm2EmlProfile");
    }catch(...)
    {
      __warning__("admin.autoCreateGsm2EmlProfile not found disabled by default");
    }

    try
    {
      initRegions(cfgman.getString("admin.regionsconfig"),cfgman.getString("admin.routesconfig"));
      cfg::regionsEnabled=true;
    } catch(std::exception& e)
    {
      __warning2__("regions support disabled, exception: %s", e.what());
    }

    try
    {
      cfg::helpDeskAddress=Address(cfgman.getString("admin.helpdeskAddress")).value;
    } catch(std::exception& e)
    {
      __warning__("helpdesk support disabled");
    }

    bool haveStats=false;
    try
    {
      statCollector.Init(cfgman.getString("stat.storeLocation"),cfgman.getInt("stat.flushPeriodInSec"));
      haveStats=true;
      statCollector.Start();

    } catch(std::exception&e)
    {
      __warning2__("stats disabled:%s",e.what());
    }

    try{
      const char* lmt=cfgman.getString("admin.defaultLimit");
      int val;
      char type;
      if(sscanf(lmt,"%d%c",&val,&type)!=2)
      {
        throw smsc::util::Exception("Invalid limit format:%s",lmt);
      }
      cfg::defaultLimitValue=val;
      switch(type)
      {
        case 'd':cfg::defaultLimitType=ltDay;break;
        case 'w':cfg::defaultLimitType=ltWeek;break;
        case 'm':cfg::defaultLimitType=ltMonth;break;
        default:throw smsc::util::Exception("Invalid limit format:%s",lmt);
      }
    }catch(std::exception& e)
    {
      __warning2__("parameter admin.defaultLimit parsing exception:%s, using default",e.what());
    }
    catch(...)
    {
      __warning__("parameter admin.defaultLimit not found, using default value");
    }

    cfg::storeDir=cfgman.getString("store.dir");



    storage.Open(cfg::storeDir.c_str());


    SmeConfig cfg;
    cfg.host=cfgman.getString("smpp.host");
    cfg.port=cfgman.getInt("smpp.port");
    cfg.sid=cfgman.getString("smpp.systemId");
    cfg.timeOut=cfgman.getInt("smpp.timeout");
    cfg.password=cfgman.getString("smpp.password");
    cfg.origAddr=cfgman.getString("smpp.sourceAddress");
    try
    {
      cfg.systemType=cfgman.getString("smpp.systemType");
    } catch(...)
    {
    }

    try
    {
      cfg::partitionSms=cfgman.getBool("smpp.partitionSms");
      cfg::maxUdhParts=cfgman.getInt("smpp.maxUdhParts");
    } catch(...)
    {
    }

    if(cfg::partitionSms)
    {
      int sendSpeed=cfgman.getInt("smpp.partsSendSpeedPerHour");
      int sendDelay=3600*1000/sendSpeed;
      multiPartSendQueue.Init(cfgman.getString("store.queueDir"),sendDelay);

      std::string concatStore=cfg::storeDir;
      if(concatStore.length() && *concatStore.rbegin()!='/')
      {
        concatStore+='/';
      }
      concatStore+="concat";
      if(!File::Exists(concatStore.c_str()))
      {
        File::MkDir(concatStore.c_str());
      }
      concatManager.Init(concatStore,cfgman.getInt("smpp.concatTimeout"));
    }

    cfg::serviceType=cfgman.getString("smpp.serviceType");
    cfg::protocolId=cfgman.getInt("smpp.protocolId");

    cfg::maildomain=cfgman.getString("mail.domain");
    try{
      cfg::hdmaildomain=cfgman.getString("mail.helpdeskDomain");
    }catch(...)
    {
      __warning__("helpdesk mail domain not found");
    }

    {
      std::string validToDomains=cfgman.getString("mail.validToDomains");
      size_t pos=0;
      size_t comma=0;
      do{
        pos=comma;
        while(pos<validToDomains.length() && isspace(validToDomains[pos]))
        {
          pos++;
        }
        comma=validToDomains.find(',',comma);
        cfg::validDomains.push_back(trimSpaces(validToDomains.substr(pos,comma==std::string::npos?std::string::npos:comma-pos).c_str()));
        if(comma!=std::string::npos)
        {
          comma++;
        }
      }while(comma!=std::string::npos);
    }

    std::string reSrc;
    try{
      reSrc=cfgman.getString("mail.userNameTransformRegexp");
      cfg::transformResult=cfgman.getString("mail.userNameTransformResult");
      if(reSrc.length())
      {
        RegExp re;
        if(!re.Compile(reSrc.c_str(),OP_OPTIMIZE))
        {
          __warning2__("Username Transformation Regexp compilation error(%d). Transformation disabled!",re.LastError());
        }else
        {
          cfg::useTransformRegexp=true;
        }
      }
    }catch(std::exception& e)
    {
      __warning2__("Missing optional parameter:%s",e.what());
    }

    AdminCommandsListener acl;

    acl.Init(cfgman.getString("admin.host"),cfgman.getInt("admin.port"));
    acl.Start();

    Socket srv;

    if(srv.InitServer(cfgman.getString("listener.host"),cfgman.getInt("listener.port"),0,0)==-1)
    {
      __warning2__("emailsme: Failed to init listener at %s:%d",cfgman.getString("listener.host"),cfgman.getInt("listener.port"));
      return -1;
    };
    if(srv.StartServer()==-1)
    {
      __warning__("Failed to start listener");
      return -1;
    };


    cfg::mailstripper=cfgman.getString("mail.stripper");
    try{
      cfg::mailthreadsCount=cfgman.getInt("mail.threadsCount");
    }catch(...)
    {
      __warning__("mail.threadsCount not found. using default=1");
    }
    if(cfg::mailthreadsCount<1)
    {
      __warning__("mail.threadsCount cannot be less than 1");
      cfg::mailthreadsCount=1;
    }
    util::PipesInit(cfg::mailthreadsCount);
    cfg::mailThreads=new EmailProcessor[cfg::mailthreadsCount];
    for(int i=0;i<cfg::mailthreadsCount;i++)
    {
      cfg::mailThreads[i].assignTransformRegexp(reSrc.c_str());
      cfg::mailThreads[i].Start();
    }

    /*if(util::ForkPipedCmd(cfg::mailstripper.c_str(),emlIn,emlOut)<=0)
     {
     __warning2__("Failed to execute mail stripper:%s",strerror(errno));
     fprintf(stderr,"Failed to execute mail stripper:%s",strerror(errno));
     return -1;
     }
     */

    cfg::sendSuccessAnswer=cfgman.getBool("answers.sendSuccessAnswer");

    cfg::msgFormat=new OutputFormatter(cfgman.getString("mail.format"));

    {
      const char* answers[]=
      {
        "alias",
        "aliasfailed",
        "aliasbusy",
        "noalias",
        "forward",
        "forwardfailed",
        "forwardoff",
        "realname",
        "numberon",
        "numberoff",
        "numberfailed",
        "systemerror",
        "unknowncommand",
        "messagesent",
        "messagefailedlimit",
        "messagefailednoprofile",
        "messagefailedsendmail",
        "messagefailedsystem",
        "norealname"
      };
      for(int i=0;i<sizeof(answers)/sizeof(answers[0]);i++)
      {
        std::string fullCfgString="answers.";
        fullCfgString+=answers[i];
        cfg::answerFormats.Insert(answers[i],new OutputFormatter(cfgman.getString(fullCfgString.c_str())));
      }
      for(int i=0;i<sizeof(answers)/sizeof(answers[0]);i++)
      {
        std::string fullCfgString="answers.helpdesk.";
        fullCfgString+=answers[i];
        std::string msgName="helpdesk.";
        msgName+=answers[i];
        cfg::answerFormats.Insert(msgName.c_str(),new OutputFormatter(cfgman.getString(fullCfgString.c_str())));
      }
    }

    //cfg::defaultDailyLimit=cfgman.getInt("defaults.dailyLimit");
    cfg::annotationSize=cfgman.getInt("defaults.annotationSize");

    __trace2__("defaults.annotationSize:%d",cfg::annotationSize);

    MyListener lst;
    cfg::aq=&lst;

    SmppSession ss(cfg,&lst);
    cfg::tr=ss.getSyncTransmitter();
    cfg::atr=ss.getAsyncTransmitter();
    cfg::sourceAddress=cfgman.getString("smpp.sourceAddress");
    lst.setTrans(cfg::tr);

    while(!cfg::stopSme)
    {
      for(;;)
      {
        if(cfg::stopSme)break;
        try{
          ss.connect();
        }catch(std::exception& e)
        {
          __warning2__("exception in session::connect:'%s'",e.what());
          sleep(cfg::retryTime);
          continue;
        }
        break;
      }
      if(cfg::stopSme)break;
      if(cfg::partitionSms)
      {
        multiPartSendQueue.Start();
        concatManager.Start();
      }

      reconnectFlag=false;
      while(!cfg::stopSme)
      {
        if(reconnectFlag)
        {
          __trace__("reconnecting");
          break;
        }
        {
          if(srv.canRead(2)<=0)continue;
          Socket* clnt=srv.Accept();
          if(!clnt || reconnectFlag)
          {
            __trace__("reconnecting");
            break;
          }
          EmailProcessor::startEmailProcessing(clnt);
          /*
          auto_ptr<Socket> clnt(srv.Accept());
          if(!clnt.get() || reconnectFlag)
          {
            __trace__("reconnecting");
            break;
          }
          int sz;
          clnt->setTimeOut(10);
          __trace__("Got connection");
          if(clnt->ReadAll((char*)&sz,4)==-1)continue;
          sz=ntohl(sz);
          __trace2__("Message size:%d",sz);
          buf.setSize(sz+1);
          if(clnt->ReadAll(buf.buffer,sz)==-1)continue;
          __trace__("Processing message");
          int retcode;
          try{
            buf.buffer[sz]=0;
            static smsc::logger::Logger* log=smsc::logger::Logger::getInstance("msgdmp");
            smsc_log_debug(log,"msgdump:\n=== begin ===\n%s\n=== end ===",buf.buffer);
            retcode=ProcessMessage(buf.buffer,sz);
          }catch(exception& e)
          {
            __warning2__("process message failed:%s",e.what());
            retcode=StatusCodes::STATUS_CODE_UNKNOWNERROR;
          }
          __trace2__("Processing finished, code=%d",retcode);
          retcode=htonl(retcode);
          clnt->WriteAll(&retcode,4);
           */
        }
      }
      __trace__("exiting loop, closing session");
      if(cfg::partitionSms)
      {
        multiPartSendQueue.Stop();
        multiPartSendQueue.WaitFor();
        concatManager.Stop();
        concatManager.WaitFor();
      }
      ss.close();
      if(!cfg::stopSme && cfg::pauseAfterDisconnect)
      {
        sleep(cfg::retryTime);
      }
    }
    if(haveStats)
    {
      statCollector.Stop();
      statCollector.WaitFor();
    }
    __trace__("exiting");
    //srv.Close();
    srv.Abort();
    for(int i=0;i<cfg::mailthreadsCount;i++)
    {
      cfg::mailThreads[i].stop();
    }
    delete [] cfg::mailThreads;
  }catch(exception& e)
  {
    __warning2__("Top level exception:%s",e.what());
  }
  catch(...)
  {
    __warning__("Top level exception:unknown");
  }
  statCollector.Stop();
  smsc_log_info(log,"\n====================\nE-Mail sme shutdown complete\n====================\n");
  return 0;
}
