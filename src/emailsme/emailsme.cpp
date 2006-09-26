#include <stdio.h>
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
#include "sms/sms_util.h"
#include "emailsme/AbonentProfile.hpp"
#include "util/Base64.hpp"

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

FILE *emlIn;
FILE *emlOut;

char hostName[256];

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


template <int N>
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
    int l=strlen(s);
    strncpy(str,s,N);
    str[N]=0;
    len=l>N?N:l;
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
        p.*field=0;
        return false;
      }
      break;
    case ltWeek:
      if((now-p.limitDate)/(60*60*24)>7)
      {
        p.limitDate=now;
        p.*field=0;
        return false;
      }
      break;
    case ltMonth:
      if(tdate.tm_mon!=tnow.tm_mon)
      {
        p.limitDate=now;
        p.*field=0;
        return false;
      }
      break;
  }
  return p.*field>p.limitValue;
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

  void CreateProfile(const AbonentProfile& p)
  {
    smsc_log_debug(log,"Create profile for %s/%s",p.addr.toString().c_str(),p.user.c_str());
    File::offset_type off;
    {
      MutexGuard mg(diskMtx);
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
      buffer.setExternalBuffer(&buf[0],buf.size());
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
      buffer.setExternalBuffer(&buf[0],buf.size());
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
      if(!clnt.get())break;
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
      uint32_t cmdId=buf.ReadNetInt32();
      uint32_t rv=1;
      bool writeRV=true;
      try
      {
        smsc_log_debug(log,"Received cmdId=%d",cmdId);
        switch(cmdId)
        {
          case cmdUpdateProfile:
          {
            AbonentProfile p,p2;
            p.Read(buf);
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
              uint32_t sz=htonl(outbuf.getPos());
              clnt->WriteAll((char*)&sz,4);
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
              uint32_t sz=htonl(outbuf.getPos());
              clnt->WriteAll((char*)&sz,4);
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

namespace cfg{
 string sourceAddress;
 int protocolId;
 string serviceType;
 SmppTransmitter *tr;
 int mainId;
 bool stopSme=false;
 string smtpHost;
 int smtpPort=25;
 int retryTime=5;
 int defaultDailyLimit;
 int annotationSize;
 string maildomain;
 string mailstripper;
 OutputFormatter *msgFormat;
 string storeDir;
 bool allowGsm2EmlWithoutProfile=false;
 bool allowEml2GsmWithoutProfile=false;
 LimitType defaultLimitType=ltDay;
 int defaultLimitValue=10;
};

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
  if(s.Gets(buf,sizeof(buf))==-1)throw Exception("");
  int retcode;
  if(sscanf(buf,"%d",&retcode)!=1)throw Exception("");;
  if(code<10)retcode/=100;
  if(retcode!=code)
  {
    __warning2__("code %d, expected %d",retcode,code);
    throw Exception("code %d, expected %d",retcode,code);
  }
}

string makeFromAddress(const char* fromaddress)
{
  return (string)fromaddress+"@"+cfg::maildomain;
}

bool ismailchar(char c)
{
  if(isalnum(c))return true;
  return c=='.' || c=='-' || c=='_';
}

string ExtractEmail(const string& value)
{
  unsigned int pos=value.find('@');
  if(pos==string::npos)return "";
  int start=pos,end=pos;
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
  int i=0;
  int l=strlen(what);
  int l2=strlen(to);
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
  if(s.WriteAll(body.c_str(),body.length())==-1)throw Exception("Failed to write body");
  s.Puts("\r\n.\r\n");
  CheckCode(s,250);
  s.Puts("QUIT\r\n");
  return ProcessSmsCodes::OK;
}

int processSms(const char* text,const char* fromaddress)
{
  try{
    Hash<SMatch> h;
    SMatch m[10];
    int n=10;
    if(!reParseSms.Match(text,m,n,&h))
    {
      __trace2__("RegExp match failed:%d",reParseSms.LastError());
      return ProcessSmsCodes::INVALIDSMS;
    }
    string addr,subj,body,from,fromdecor;
    //getField(text,h,"flag",cf);
    getField(text,h,"address",addr);
    //getField(text,h,"realname",rn);
    getField(text,h,"subj",subj);
    getField(text,h,"body",body);
    Socket s;

    ReplaceString(addr,"*","@");
    ReplaceString(addr,"$","_");

    ReplaceString(body,"\n.\n","\n..\n");

    AbonentProfile p;
    bool haveprofile=true;
    if(!storage.getProfileByAddress(fromaddress,p))
    {
      __trace2__("no profile for abonent %s",fromaddress);
      if(!cfg::allowGsm2EmlWithoutProfile)
      {
        return ProcessSmsCodes::NOPROFILE;
      }
      haveprofile=false;
      p.user=fromaddress;
    }

    if(haveprofile)
    {
      if(!storage.checkGsm2EmlLimit(fromaddress))
      {
        return ProcessSmsCodes::OUTOFLIMIT;
      }
    }


    const char* sp=strchr(text,' ');
    std::string cmd;
    if(sp)
    {
      cmd.assign(text,0,sp-text);
      for(int i=0;i<cmd.length();i++)cmd[i]=tolower(cmd[i]);
      while(*sp==' ')sp++;
    }else
    {
      cmd=text;
    }

    __trace2__("cmd=%s",cmd.c_str());

    if(haveprofile)
    {
      if(cmd=="alias")
      {
        std::string value=sp;
        if(value.length() && value!=p.user)
        {
          storage.DeleteProfile(p);
          p.user=value;
          storage.CreateProfile(p);
        }
        return ProcessSmsCodes::OK;
      }
      if(cmd=="aliasoff")
      {
        if(p.user!=p.addr.value)
        {
          p.user=p.addr.value;
          storage.DeleteProfile(p);
          storage.CreateProfile(p);
        }
        return ProcessSmsCodes::OK;
      }
      if(cmd=="forward")
      {
        p.forwardEmail=sp;
        storage.UpdateProfile(p);
        return ProcessSmsCodes::OK;
      }
      if(cmd=="forwardoff")
      {
        p.forwardEmail="";
        storage.UpdateProfile(p);
        return ProcessSmsCodes::OK;
      }

      if(cmd=="realname")
      {
        p.realName=sp;
        storage.UpdateProfile(p);
        return ProcessSmsCodes::OK;
      }
      if(cmd=="number")
      {
        std::string val=sp;
        for(int i=0;i<val.length();i++)val[i]=tolower(val[i]);
        if(val=="on")p.numberMap=true;
        else if(val=="off")p.numberMap=false;
        else return ProcessSmsCodes::INVALIDSMS;
        storage.UpdateProfile(p);
        return ProcessSmsCodes::OK;
      }
    }

    try{
      if(haveprofile)
      {
        from=makeFromAddress(MapAddressToEmail(fromaddress).c_str());
      }else
      {
        from=makeFromAddress(fromaddress);
      }
    }catch(exception& e)
    {
      __warning2__("failed to map address to mail:%s",e.what());
      return ProcessSmsCodes::NOPROFILE;
    }
    fromdecor=from;
    if(p.realName.length()>0)
    {
      fromdecor='"'+p.realName+"\" <"+from+">";
    }

    Array<string> to;
    int startpos=0;
    int pos=addr.find(',');
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
      if(haveprofile)storage.incGsm2EmlLimit(fromaddress);
      if(rv!=ProcessSmsCodes::OK)return rv;
    }catch(...)
    {
      __warning__("SMTP session aborted");
      return ProcessSmsCodes::UNABLETOSEND;
    }
    return ProcessSmsCodes::OK;
  }catch(std::exception& e)
  {
    __warning2__("Exception in processSms:%s",e.what());
  }
  return ProcessSmsCodes::NETERROR;
}

class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu)
  {
    if(pdu->get_commandId()==SmppCommandSet::DELIVERY_SM)
    {
      char buf[65536];
      getPduText((PduXSm*)pdu,buf,sizeof(buf));
      Address addr(
      ((PduXSm*)pdu)->get_message().get_source().value.size(),
      ((PduXSm*)pdu)->get_message().get_source().get_typeOfNumber(),
      ((PduXSm*)pdu)->get_message().get_source().get_numberingPlan(),
      ((PduXSm*)pdu)->get_message().get_source().get_value());
      int code=processSms(buf,addr.value);
      __trace2__("processSms: code=%d",code);
      switch(code)
      {
        case ProcessSmsCodes::OUTOFLIMIT:
        case ProcessSmsCodes::NOPROFILE:
        case ProcessSmsCodes::INVALIDSMS:code=SmppStatusSet::ESME_RX_P_APPN;break;
        case ProcessSmsCodes::UNABLETOSEND:
        case ProcessSmsCodes::NETERROR:code=SmppStatusSet::ESME_RX_T_APPN;break;
        case ProcessSmsCodes::OK:code=SmppStatusSet::ESME_ROK;break;
      }
      PduDeliverySmResp resp;
      resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      resp.get_header().set_commandStatus(code);
      resp.set_messageId("");
      resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      trans->sendDeliverySmResp(resp);
    }
  }
  void handleError(int errorCode)
  {
    pthread_kill(cfg::mainId,16);
  }

  void setTrans(SmppTransmitter *t)
  {
    trans=t;
  }
protected:
  SmppTransmitter* trans;
};

bool GetNextLine(const char* text,int maxlen,int& pos,string& line)
{
  int start=pos;
  if(pos>=maxlen)return false;
  do{
    while(pos<maxlen && text[pos]!=0x0d && text[pos]!=0x0a)pos++;
    line.assign(text+start,pos-start);
    if(text[pos]==0x0d && pos<maxlen)pos++;
    if(text[pos]==0x0a && pos<maxlen)pos++;
  }while(pos<maxlen && text[pos]==' ');
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

int ProcessMessage(const char *msg,int len)
{
  string line,name,value,from,to;
  bool inheader=true;
  int pos=0;
  for(;;)
  {
    if(!GetNextLine(msg,len,pos,line))break;
    if(inheader)
    {
      if(line.length()==0)
      {
        inheader=false;
        continue;
      }
      int pos2=line.find(':');
      if(pos2==string::npos)continue;
      name=line.substr(0,pos2);
      pos2++;
      while(pos2<line.length() && line[pos2]==' ')pos2++;
      value=line.substr(pos2);

      if(name=="From")
      {
        from=ExtractEmail(value);
        if(from.length()==0)return StatusCodes::STATUS_CODE_INVALIDMSG;
        continue;
      }
      if(name=="To")
      {
        to=ExtractEmail(value);
        if(to.length()==0)return StatusCodes::STATUS_CODE_INVALIDMSG;
        continue;
      }
      continue;
    }
    break;
  }

  AbonentProfile p;
  bool noProfile=true;

  string dstUser=to.substr(0,to.find('@'));

  if(!storage.getProfileByEmail(dstUser.c_str(),p))
  {
    __trace2__("no profile for user:%s",dstUser.c_str());
    std::string addr="+"+dstUser;
    if(storage.getProfileByAddress(addr.c_str(),p))
    {
      if(!p.numberMap)
      {
        __trace2__("number map turned off for address:%s",addr.c_str());
        return StatusCodes::STATUS_CODE_NOUSER;
      }
    }else
    {
      __trace2__("no profile for address:%s",dstUser.c_str());
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
      __trace2__("limit exceeded for user:%s",dstUser.c_str());
      return StatusCodes::STATUS_CODE_UNKNOWNERROR;
    }
  }

  if(!util::childRunning)
  {
    if(emlIn)fclose(emlIn);
    if(emlOut)fclose(emlOut);
    if(util::ForkPipedCmd(cfg::mailstripper.c_str(),emlIn,emlOut)<=0)
    {
      __trace2__("failed to fork mailstripper child:%s",strerror(errno));
      return StatusCodes::STATUS_CODE_UNKNOWNERROR;
    }
  };
  __trace2__("write msg size:%d",len);
  fprintf(emlOut,"%d\n",len);fflush(emlOut);
  __trace__("write msg");
  int sz=0;
  while(sz<len)
  {
    int wr=fwrite(msg,1,len,emlOut);fflush(emlOut);
    if(wr<=0)
    {
      __trace__("failed to write data for mailstripper");
      return StatusCodes::STATUS_CODE_UNKNOWNERROR;
    }
    sz+=wr;
  }
  __trace__("read resp len");
  char buf[16];
  if(!fgets(buf,sizeof(buf),emlIn))
  {
    __trace__("failed to read data from mailstripper");
    return StatusCodes::STATUS_CODE_UNKNOWNERROR;
  }
  __trace2__("resp len=%d",len);
  len=atoi(buf);
  auto_ptr<char> newmsg(new char[len+1]);

  sz=0;
  while(sz<len)
  {
    int rv=fread(newmsg.get(),1,len-sz,emlIn);
    if(rv==0 || rv==-1)
    {
      __trace__("failed to read data from mailstripper");
      return StatusCodes::STATUS_CODE_UNKNOWNERROR;
    }
    sz+=rv;
  }
  newmsg.get()[len]=0;

  __trace2__("newmsg:%s",newmsg.get());

  //cfg::annotationSize


  SMS sms;
  sms.setOriginatingAddress(cfg::sourceAddress.c_str());

  string fwd;
  string dst=noProfile?'+'+dstUser:MapEmailToAddress(dstUser,fwd);
  if(fwd.length())
  {
    try{
      Array<string> to2;
      to2.Push(fwd);
      string body(msg,len);
      SendEMail(from,to2,"",body,true);
    }catch(exception& e)
    {
      __warning2__("Failed to forward msg to %s:%s",fwd.c_str(),e.what());
    }
  }

  pos=0;
  string subj;
  GetNextLine(newmsg.get(),len,pos,from);
  GetNextLine(newmsg.get(),len,pos,subj);
  string text=newmsg.get()+pos;

  ContextEnvironment ce;
  EmptyGetAdapter ga;

  int fldlen=subj.length()+from.length()+to.length();
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

  __trace2__("result:%s",text.c_str());



  sms.setDestinationAddress(dst.c_str());
  char msc[]="";
  char imsi[]="";
  sms.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
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
    sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,(char*)ucs2.get(),text.length()*2);
  }else
  {
    sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::LATIN1);
    sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD,text.c_str(),text.length());
  }
  //sms.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,text.length());

  PduSubmitSm sm;
  sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
  fillSmppPduFromSms(&sm,&sms);
  PduSubmitSmResp *resp=cfg::tr->submit(sm);
  int rc=StatusCodes::STATUS_CODE_TEMPORARYERROR;
  if(resp)
  {
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
        __trace2__("Creating implicit profile creation for address %s",dst.c_str());
        AbonentProfile prof;
        prof.addr=dst.c_str();
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
        __warning2__("failed to inc counter:%s",e.what());
      }
    }
  }
  if(resp)disposePdu((SmppHeader*)resp);
  return rc;
}

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

extern "C"  void disp(int sig)
{
}

extern "C"  void ctrlc(int sig)
{
  cfg::stopSme=1;
  pthread_kill(cfg::mainId,16);
}


extern "C" void atExitHandler(void)
{
  smsc::util::xml::TerminateXerces();
  smsc::logger::Logger::Shutdown();
}

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  atexit(atExitHandler);

  sigset(16,disp);
  sigset(SIGINT,ctrlc);
  sigset(SIGTERM,ctrlc);

  if(gethostname(hostName,sizeof(hostName))!=0)
  {
    smsc_log_warn(smsc::logger::Logger::getInstance("emlsme"),"gethostname failed:%d",errno);
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
"/^(?:"
"alias\\s+\\w*|"
"aliasoff|"
"forward\\s+[\\w\\-\\.]+@[\\w\\-\\.]+|"
"forwardoff|"
"number\\s+on|"
"number\\s+off|"
"realname\\s+.*|"
"(?{address}[\\w\\-\\.]+@[\\w\\-\\.]+)(?:\\ssubj=\"(?{subj}.*?)\")?\\s*(?{body}.*)"
")$/isx"
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

  cfg::serviceType=cfgman.getString("smpp.serviceType");
  cfg::protocolId=cfgman.getInt("smpp.protocolId");

  cfg::maildomain=cfgman.getString("mail.domain");

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
  if(util::ForkPipedCmd(cfg::mailstripper.c_str(),emlIn,emlOut)<=0)
  {
    __warning2__("Failed to execute mail stripper:%s",strerror(errno));
    fprintf(stderr,"Failed to execute mail stripper:%s",strerror(errno));
    return -1;
  }

  cfg::msgFormat=new OutputFormatter(cfgman.getString("mail.format"));

  cfg::defaultDailyLimit=cfgman.getInt("defaults.dailyLimit");
  cfg::annotationSize=cfgman.getInt("defaults.annotationSize");

  __trace2__("defaults.annotationSize:%d",cfg::annotationSize);

  MyListener lst;
  SmppSession ss(cfg,&lst);
  cfg::tr=ss.getSyncTransmitter();
  cfg::sourceAddress=cfgman.getString("smpp.sourceAddress");
  lst.setTrans(cfg::tr);
  XBuffer buf;

  while(!cfg::stopSme)
  {
    for(;;)
    {
      if(cfg::stopSme)break;
      try{
        ss.connect();
      }catch(...)
      {
        sleep(cfg::retryTime);
        continue;
      }
      break;
    }
    if(cfg::stopSme)break;
    for(;;)
    {
      __trace__("Waiting for connection");
      {
        auto_ptr<Socket> clnt(srv.Accept());
        if(!clnt.get())break;
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
      }
    }
    ss.close();
  }
  __trace__("exiting");
  //srv.Close();
  srv.Abort();
  }catch(exception& e)
  {
    __warning2__("Top level exception:%s",e.what());
  }
  catch(...)
  {
    __warning__("Top level exception:unknown");
  }
  if(emlIn)fclose(emlIn);
  if(emlOut)fclose(emlOut);
  return 0;
}
