#include <vector>
#include <memory>
#include <ctype.h>


#include "profiler/profiler.hpp"

#include "core/buffers/Array.hpp"
#include "core/buffers/XHash.hpp"

#include "util/config/Manager.h"

#include "db/DataSource.h"
#include "db/DataSourceLoader.h"

#include "util/recoder/recode_dll.h"
#include "util/smstext.h"
#include "logger/Logger.h"
#include "system/status.h"

#include "resourcemanager/ResourceManager.hpp"

#include <exception>

namespace smsc{
namespace profiler{

using namespace smsc::core::buffers;
using smsc::util::Exception;
using std::exception;
using namespace smsc::util;

using namespace smsc::system;

using namespace smsc::resourcemanager;

class AccessDeniedException{};

struct HashKey{
  Address addr;
  uint8_t defLength;

  HashKey(const HashKey& key)
  {
    addr=key.addr;
    defLength=key.defLength;
  }

  HashKey(const Address& address)
  {
    addr=address;
    defLength=address.length;
    while(defLength>0 && address.value[defLength-1]=='?')defLength--;
  }

  bool operator==(const HashKey& key)
  {
    return key.addr.length==addr.length &&
           defLength==key.defLength &&
           key.addr.plan==addr.plan &&
           key.addr.type==addr.type &&
           !strncmp(key.addr.value,addr.value,defLength);
  }
};


uint32_t rnd[]={0x14bcdd6e,0xf553733f,0x9998729e,0xb95fd4c0,0x0b520739,0x3547ae78,0xa2bcb36d,0x8ad3cec8,0x25002a6a,0x66d05d7d,0x2be546db,0x4fa60819,0x77aec701,0x138246bb,0x7298216c,0x512eb6a7,0xcd037352,0x287cac72,0x3b2b9f86,0x065bd3c9,0x4d8d781a,0x9287f0db,0x73e7208c,0xb9d8c960,0x15d974a8,0xde362c45,0xd127a9fd,0x222910f1,0xda98f9f8,0xd809db34,0x7379819b,0x0bcec9c9,0x8cbc2fd6,0x30877084,0x80bcd783,0xee61641a,0xf29b6ffb,0x1263fd88,0xc8455c53,0x96b73d1e,0x4b0eec2f,0xcf9cd228,0x63f8e744,0xbefd3d35,0xf18f4869,0x769a1703,0x516d1153,0xb1de7706,0xf35a3eed,0x764f758f,0x1b0edb55,0xeb9ece9c,0xbc903a6f,0x4059ae38,0x74394823,0xbb3b908b,0xac53ff31,0xe51f4480,0xd45cffb1,0xdb8e160e,0xbee74224,0xb6f41221,0x1f1aef33,0x1b6b6825,0x26d35006,0xe633f12b,0x3f66e83c,0xf8bfd8b6,0xed01a786,0x26655823,0xc8a840dd,0x42b3a7af,0x99dd9b5e,0x4a5bf926,0x97d970c3,0xb6c9a224,0xc575f67a,0xe4516107,0x6fa4b95e,0xa500c06f,0xc69a9313,0xea0e9c70,0x9d89bdfa,0x90efc852,0x490105cf,0x4f03d201,0x98f723e1,0xc6e9eb12,0xf55f30e6,0xa76ce770,0x9d72397c,0x081a67a0,0x068df13b,0xca6f1bac,0x53b18e74,0x2aab28ae,0xf6a5b682,0x6c3dacf6,0x69ba98ce,0xacdc64d7,0x1126215d,0xc431730b,0x360a510f,0x612b40be,0x220fac7a,0x29f08539,0x5badd65b,0x0d6e6c29,0x060443b9,0xb28bd789,0x60630c94,0x03f7c528,0x5868e646,0xd79cd7d7,0x55bf3879,0xc7b2f42f,0x09844cbe,0x116910f9,0x7545a7c9,0xae450f39,0x02a17d4a,0x7902cad8,0xc28d4862,0x7e3138e9,0xc72d7ac2,0x6760ab96,0xa560515f,0x0bf13ea7,0x10d7d3ce,0x168853ab,0x93dbdb3b,0xdc1d3dc1,0x73af5204,0x41aa0004,0xa78d84ef,0xc8833c8e,0xfa4a9206,0xc13f2c26,0x44970d11,0x954fd088,0xcadfa4a9,0x332b2e4d,0xb7ccfdf7,0x9d27b973,0xbf66ac0c,0x93ded88a,0xd6d441d6,0x654a8677,0x10bc84bf,0xdd6f1be5,0x9e47c6e0,0x48b22f44,0xeabe5ae0,0xadc4a17c,0xd4cf2367,0x0f36bb2f,0x126d503c,0xe2aa75a5,0xa84c2efb,0x93a3de55,0x890b1e6e,0x38f99b0c,0x51eea689,0x5ee595b9,0x2440aeff,0xf0b6b5d3,0xad56cd7f,0x4b1fce64,0x3531c288,0x662da1b2,0xe6bb08e7,0x25d10086,0x23ab8cce,0xbc171918,0x0a05828c,0x47f4d195,0x0f3a57e7,0x71b7544b,0xb5edcb15,0x3f1bb26f,0x724e2056,0x04f9a788,0xa720762a,0x68968076,0xbe59392e,0x98962020,0x6a5fbb90,0x918d26b5,0xfeefe82e,0x8d302e9d,0xf09a184a,0x95243bfc,0x74e708e6,0x267339df,0x3616edbb,0x0499a202,0x3b7ca9d3,0x2537463b,0xde8a20c3,0xbd642b84,0xe869ae82,0x6d9d979f,0xd43ebbe3,0x8e573266,0x260c71ad,0xa0314cae,0xed308e56,0xd7c241d2,0x5b875d5c,0xc40bffe0,0x852dcb3b,0x288aac5a,0x43de9608,0xdceb68a7,0x24f6a9ab,0xe2543615,0x941690b6,0x8e5dfb88,0x165f07e0,0xd69cacc0,0x9d58b61b,0xbfd78941,0x126c0351,0xe6d88ae0,0xe50e07ba,0x34dadfe7,0xd677a3d5,0xa49c97e0,0xcb08b604,0x35126501,0xcc4b71c1,0xf3b48632,0x2694cc79,0x2873c4b3,0xa2441b07,0xa546996c,0xe7a6c5c9,0x345e7cd2,0xf272155a,0x1ff53c6e,0xb7f132ef,0xe2be8d0c,0xdbb9344a,0xf4fba87d,0x960f5858,0xb9281305,0xa9ec5566,0x8a508463,0x7f99d2fd,0xe3fce678,0x6cf3f85a,0xb4c48193,0x054b2d87,0xca833b56,0xa0e8e213,0x5a24fe45};

class HashFunc{
public:
static uint32_t CalcHash(const HashKey& key)
{
  uint32_t retval=key.addr.type^key.addr.plan;
  int i;
  for(i=0;i<key.defLength;i++)
  {
    //retval=(retval<<1)^rnd[(unsigned char)key.addr.value[i]]^rnd[retval&0xff];
    retval=retval*10+(key.addr.value[i]-'0');
  }
  return retval;
}
};

static string DumpProfile(const Profile& profile)
{
  string rv;
  char buf[64];
  sprintf(buf,"r=%d;",profile.reportoptions);
  rv+=buf;
  sprintf(buf,"dc=%d;",profile.codepage);
  rv+=buf;
  sprintf(buf,"l=%s;",profile.locale.c_str());
  rv+=buf;
  sprintf(buf,"h=%d;",profile.hide);
  rv+=buf;
  sprintf(buf,"hm=%c;",profile.hideModifiable?'Y':'N');
  rv+=buf;
  sprintf(buf,"d=%s;",profile.divert.length()?"(NULL)":profile.divert.c_str());
  rv+=buf;
  sprintf(buf,"da=%c;",profile.divertActive?'Y':'N');
  rv+=buf;
  sprintf(buf,"da=%c;",profile.divertModifiable?'Y':'N');
  rv+=buf;
  return rv;
}

class ProfilesTable: public smsc::core::buffers::XHash<HashKey,Profile,HashFunc>
{
  Profile defaultProfile;
  smsc::logger::Logger* log;
public:
  ProfilesTable(const Profile& pr,int n):XHash<HashKey,Profile,HashFunc>(n)
  {
    defaultProfile=pr;
    log=smsc::logger::Logger::getInstance("smsc.prof");
  }
  Profile& find(const Address& address,bool& exact)
  {
    HashKey k(address);
    exact=false;
    Profile *p;
    while((p=GetPtr(k))==NULL)
    {
      if(!k.defLength)
      {
        debug2(log,"lookup %s(default profile):%s",address.toString().c_str(),DumpProfile(defaultProfile).c_str());
        return defaultProfile;
      }
      k.defLength--;
    }
    exact=k.defLength==address.length;
    debug2(log,"lookup %s(%s):%s",exact?"exact":"mask",address.toString().c_str(),DumpProfile(defaultProfile).c_str());
    return *p;
  }
  Profile& findEx(const Address& address,int& matchType,string& matchAddr)
  {
    HashKey k(address);
    matchType=ProfilerMatchType::mtDefault;
    Profile *p;
    while((p=GetPtr(k))==NULL)
    {
      if(!k.defLength)
      {
        return defaultProfile;
      }
      k.defLength--;
    }
    matchType=k.defLength==address.length?ProfilerMatchType::mtExact:ProfilerMatchType::mtMask;
    char buf[32];
    address.toString(buf,sizeof(buf));
    matchAddr=buf;
    for(int i=k.defLength;i<address.length;i++)
    {
      matchAddr.at(matchAddr.length()-address.length+i)='?';
    }
    return *p;
  }
  void add(const Address& address,const Profile& profile)
  {
    HashKey k(address);
    Insert(k,profile);
  }
};


Profiler::Profiler(const Profile& pr,SmeRegistrar* psmeman,const char* sysId)
{
  state=VALID;
  managerMonitor=NULL;
  profiles=new ProfilesTable(pr,1000000);
  systemId=sysId;
  smeman=psmeman;
  seq=1;
  prio=SmeProxyPriorityDefault;
  log=smsc::logger::Logger::getInstance("smsc.prof");
}

Profiler::~Profiler()
{
  delete profiles;
  try{
    smeman->unregisterSmeProxy(systemId);
  }catch(...)
  {
  }
}

Profile& Profiler::lookup(const Address& address)
{
  MutexGuard g(mtx);
  bool exact;
  return profiles->find(address,exact);
}

Profile& Profiler::lookupEx(const Address& address,int& matchType,std::string& matchAddr)
{
  MutexGuard g(mtx);
  return profiles->findEx(address,matchType,matchAddr);
}

void Profiler::remove(const Address& address)
{
  MutexGuard g(mtx);
  profiles->Delete(address);
}


int Profiler::update(const Address& address,const Profile& profile)
{
  MutexGuard g(mtx);
  bool exact;
  Profile &prof=profiles->find(address,exact);
  debug2(log,"update %s:%s->%s",
     address.toString().c_str(),
      DumpProfile(prof).c_str(),
      DumpProfile(profile).c_str(),
      exact?"exact":"inexact");
  if(prof==profile)return pusUnchanged;
  try{
  if(exact)
  {
    prof.assign(profile);
    dbUpdate(address,prof);
    debug2(log,"update %s",address.toString().c_str());
    return pusUpdated;
  }else
  {
    profiles->add(address,profile);
    dbInsert(address,profile);
    debug2(log,"insert %s",address.toString().c_str());
    return pusInserted;
  }
  }catch(...)
  {
    smsc_log_error(log, "Database exception during profile update/insert");
    return pusError;
  }
}

int Profiler::updatemask(const Address& address,const Profile& profile)
{
  MutexGuard g(mtx);
  HashKey k(address);
  bool exists=profiles->Exists(k);
  profiles->add(address,profile);
  try{
    if(exists)
    {
      dbUpdate(address,profile);
      return pusUpdated;
    }else
    {
      dbInsert(address,profile);
      return pusInserted;
    }
  }catch(...)
  {
    smsc_log_error(log, "Database exception during mask profile update");
    return pusError;
  }
}


void Profiler::add(const Address& address,const Profile& profile)
{
  MutexGuard g(mtx);
  profiles->add(address,profile);
}


class ConnectionGuard{
  smsc::db::DataSource *ds;
  smsc::db::Connection *conn;
public:
  ConnectionGuard(smsc::db::DataSource *_ds):ds(_ds)
  {
    conn=ds->getConnection();
  }
  ~ConnectionGuard()
  {
    ds->freeConnection(conn);
  }
  smsc::db::Connection* operator->()
  {
    return conn;
  }
  smsc::db::Connection* get()
  {
    return conn;
  }
};

void Profiler::dbUpdate(const Address& addr,const Profile& profile)
{
  using namespace smsc::db;
  using smsc::util::config::Manager;
  using smsc::util::config::ConfigView;
  using smsc::util::config::ConfigException;
  const char *sql="UPDATE SMS_PROFILE SET reportinfo=:1, codeset=:2, locale=:3, hidden=:4, hidden_mod=:5,divert=:6,divert_act=:7,divert_mod=:8 where mask=:9";
  ConnectionGuard connection(ds);
  if(!connection.get())throw Exception("Profiler: Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Profiler: Failed to create statement");
  char buf[32];
  sprintf(buf,"%d",profile.reportoptions);
  statement->setString(1,buf);
  sprintf(buf,"%d",profile.codepage);
  statement->setString(2,buf);
  char addrbuf[30];
  addr.toString(addrbuf,sizeof(addrbuf));
  __trace2__("Profiler: dbUpdate %s=%d,%d,%s,%d,%c,%s,%c,%c",addrbuf,profile.reportoptions,profile.codepage,profile.locale.c_str(),profile.hide,profile.hideModifiable?'Y':'N',profile.divert.c_str(),profile.divertActive?'Y':'N',profile.divertModifiable?'Y':'N');
  statement->setString(3,profile.locale.c_str());
  statement->setString(4,profile.hide?"Y":"N");
  statement->setString(5,profile.hideModifiable?"Y":"N");
  statement->setString(6,profile.divert.c_str());
  char div[16];
  div[0]=profile.divertActive?'Y':'N';
  div[1]=profile.divertActiveAbsent?'Y':'N';
  div[2]=profile.divertActiveBlocked?'Y':'N';
  div[3]=profile.divertActiveBared?'Y':'N';
  div[4]=profile.divertActiveCapacity?'Y':'N';
  div[5]=0;
  statement->setString(7, div);
  statement->setString(8,profile.divertModifiable?"Y":"N");
  statement->setString(9,addrbuf);
  statement->executeUpdate();
  connection->commit();
}

void Profiler::dbInsert(const Address& addr,const Profile& profile)
{
  using namespace smsc::db;
  using smsc::util::config::Manager;
  using smsc::util::config::ConfigView;
  using smsc::util::config::ConfigException;
  const char* sql = "INSERT INTO SMS_PROFILE (mask, reportinfo, codeset, locale,hidden,hidden_mod,divert,divert_act,divert_mod)"
                      " VALUES (:1, :2, :3, :4, :5,:6,:7,:8,:9)";

  ConnectionGuard connection(ds);

  if(!connection.get())throw Exception("Profiler: Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  char addrbuf[30];
  addr.toString(addrbuf,sizeof(addrbuf));
  __trace2__("Profiler: dbInsert %s=%d,%d,%s,%d,%c,%s,%c,%c",addrbuf,profile.reportoptions,profile.codepage,profile.locale.c_str(),profile.hide,profile.hideModifiable?'Y':'N',profile.divert.c_str(),profile.divertActive?'Y':'N',profile.divertModifiable?'Y':'N');
  statement->setString(1, addrbuf);
  char buf[32];
  sprintf(buf,"%d",profile.reportoptions);
  statement->setString(2, buf);
  sprintf(buf,"%d",profile.codepage);
  statement->setString(3, buf);
  statement->setString(4,profile.locale.c_str());
  statement->setString(5, profile.hide?"Y":"N");
  statement->setString(6, profile.hideModifiable?"Y":"N");
  statement->setString(7,profile.divert.c_str());
  char div[16];
  div[0]=profile.divertActive?'Y':'N';
  div[1]=profile.divertActiveAbsent?'Y':'N';
  div[2]=profile.divertActiveBlocked?'Y':'N';
  div[3]=profile.divertActiveBared?'Y':'N';
  div[4]=profile.divertActiveCapacity?'Y':'N';
  div[5]=0;
  statement->setString(8, div);
  statement->setString(9, profile.divertModifiable?"Y":"N");
  statement->executeUpdate();
  connection->commit();
}

static const int _update_report=1;
static const int _update_charset=2;
static const int _update_locale=3;
static const int _update_hide=4;
static const int _update_divert_act=5;
static const int _update_divert=6;
static const int _update_charset_ussd=7;
static const int _update_divert_cond=8;

static const int update_div_cond_Absent=1;
static const int update_div_cond_Blocked=2;
static const int update_div_cond_Bared=3;
static const int update_div_cond_Capacity=4;

static const int update_div_cond_OnBit=0x8000;



void Profiler::internal_update(int flag,const Address& addr,int value,const char* svalue)
{
  Profile profile;
  profile.assign(lookup(addr));

  if(flag==_update_report)
  {
    profile.reportoptions=value;
  }
  if(flag==_update_charset)
  {
    profile.codepage=value;
  }
  if(flag==_update_locale)
  {
    profile.locale=svalue;
  }
  if(flag==_update_hide)
  {
    if(!profile.hideModifiable)throw AccessDeniedException();
    profile.hide=value;
  }
  if(flag==_update_divert_act)
  {
    if(!profile.divertModifiable)throw AccessDeniedException();
    profile.divertActive=value;
  }
  if(flag==_update_divert)
  {
    if(!profile.divertModifiable)throw AccessDeniedException();
    profile.divert=svalue;
  }
  if(flag==_update_divert_cond)
  {
    if(!profile.divertModifiable)throw AccessDeniedException();
    int div=value&(~update_div_cond_OnBit);
    bool on=(value&update_div_cond_OnBit)==update_div_cond_OnBit;
    switch(div)
    {
      case update_div_cond_Absent:profile.divertActiveAbsent=on;break;
      case update_div_cond_Blocked:profile.divertActiveBlocked=on;break;
      case update_div_cond_Bared:profile.divertActiveBared=on;break;
      case update_div_cond_Capacity:profile.divertActiveCapacity=on;break;
    }
  }
  if(flag==_update_charset_ussd)
  {
    if(value)
    {
      profile.codepage|=ProfileCharsetOptions::UssdIn7Bit;
    }else
    {
      profile.codepage&=~ProfileCharsetOptions::UssdIn7Bit;
    }
  }
  update(addr,profile);
}

enum{
  msgReportNone,
  msgReportFull,
  msgDefault,
  msgUCS2,
  msgLatin1,
  msgUCS2AndLat,
  msgLocaleChanged,
  msgLocaleUnknown,
  msgHide,
  msgUnhide,
  msgReportFinal,
  msgAccessDenied,
  msgDivertOn,
  msgDivertOff,
  msgDivertChanged,
  msgInvalidParam,
  msg7BitUssdOn,
  msg7BitUssdOff
};

int Profiler::Execute()
{
  SmscCommand cmd,resp;
  SMS *sms;
  //Array<SMS*> smsarr;
  int len;
  char body[MAX_SHORT_MESSAGE_LENGTH+1];
//  char buf[MAX_SHORT_MESSAGE_LENGTH+1];
//  int coding;

//  char *str=body;
  int msg=-1;
  while(!isStopping)
  {
    if(!hasOutput())
    {
      waitFor();
    }
    if(!hasOutput())continue;
    try{
      cmd=getOutgoingCommand();
      if(cmd->cmdid==smsc::smeman::SUBMIT_RESP)continue;
      if(cmd->cmdid!=smsc::smeman::DELIVERY)
      {
        __warning2__("Profiler: incorrect command submitted - %d",cmd->cmdid);
        continue;
      }
      sms = cmd->get_sms();
      int status=MAKE_COMMAND_STATUS(CMD_OK,0);
      if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3c)!=0)
      {
        status=MAKE_COMMAND_STATUS(CMD_ERR_PERM,Status::INVESMCLASS);
        resp=SmscCommand::makeDeliverySmResp(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str(),
                                             cmd->get_dialogId(),status);
        putIncomingCommand(resp);
        continue;
      };

      if(sms->hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
      {
        __warning__("Profiler: received receipt!!!");
        status=Status::OK;
        resp=SmscCommand::makeDeliverySmResp("",cmd->get_dialogId(),status);
        putIncomingCommand(resp);
        continue;
      }

      if(sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) && sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP)!=1)
      {
        __warning__("Profiler: ussd service op != 1");
        status=MAKE_COMMAND_STATUS(CMD_ERR_PERM,Status::INVOPTPARAMVAL);
        resp=SmscCommand::makeDeliverySmResp(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str(),
                                             cmd->get_dialogId(),status);
        putIncomingCommand(resp);
        continue;
      }

      Address& addr=sms->getOriginatingAddress();

      try{
        len=getSmsText(sms,body,sizeof(body));
        if(len<=0)throw Exception("sms for profiler too large");
      }catch(...)
      {
        status=MAKE_COMMAND_STATUS(CMD_ERR_PERM,Status::INVOPTPARAMVAL);
        resp=SmscCommand::makeDeliverySmResp(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str(),
                                             cmd->get_dialogId(),status);
        putIncomingCommand(resp);
        __warning2__("INVALID MESSAGE SENT TO PROFILER FROM %s",addr.toString().c_str());
        continue;
      }

      __trace2__("Profiler: received %s from .%d.%d.%.20s",body,addr.type,addr.plan,addr.value);

      int i;
      for(i=0;i<len;i++)body[i]=toupper(body[i]);
      i=0;
      while(!isalnum(body[i]) && body[i]!='*' && body[i]!='#' && i<len)i++;
      msg=-1;
      try{
        if(sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP))
        {
          __trace2__("ussd service op=%d",sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP));
          char *str=body;
          char *ptr=str;
          while(ptr=strchr(ptr,'#'))*ptr='*';

          int code=-1;
          int pos;
          if(sscanf(str,"%d%n",&code,&pos)==1 && (str[pos]==0 || str[pos]=='*'))
          {
            __trace2__("Profiler: ussd op=%s(%d)",str,code);
            if(ussdCmdMap.Exist(code))
            {
              string scmd=ussdCmdMap.Get(code);
              if(scmd=="DIVERT TO")
              {
                scmd+=' ';
                if(str[pos]=='*')
                {
                  scmd.append(str+pos+1);
                }
              }
              strcpy(body,scmd.c_str());
              __trace2__("Profiler: command mapped to %s",body);
              i=0;
              len=strlen(body);
            }
          }
        }
        if(!strncmp(body+i,"REPORT",6))
        {
          i+=7;
          while(i<len && !isalpha(body[i]))i++;
          if(i<len)
          {
            if(!strncmp(body+i,"NONE",4))
            {
              msg=msgReportNone;
              internal_update(_update_report,addr,ProfileReportOptions::ReportNone);
            }
            else
            if(!strncmp(body+i,"FULL",4))
            {
              msg=msgReportFull;
              internal_update(_update_report,addr,ProfileReportOptions::ReportFull);
            }else
            if(!strncmp(body+i,"FINAL",5))
            {
              msg=msgReportFinal;
              internal_update(_update_report,addr,ProfileReportOptions::ReportFinal);
            }
          }
        }else
        if(!strncmp(body+i,"LOCALE",6))
        {
          i+=6;
          while(body[i] && isspace(body[i]))i++;
          int j=i;
          while(body[i] && !isspace(body[i]))i++;
          if(i==j)
          {
            msg=-1;
          }else
          {
            string loc;
            loc.assign(body+j,i-j);
            for(int x=0;x<loc.length();x++)loc.at(x)=tolower(loc.at(x));
            __trace2__("Profiler: new locale %s",loc.c_str());
            if(ResourceManager::getInstance()->isValidLocale(loc))
            {
              internal_update(_update_locale,addr,0,loc.c_str());
              msg=msgLocaleChanged;
            }else
            {
              msg=msgLocaleUnknown;
            }
          }
        }else
        if(!strncmp(body+i,"DEFAULT",7))
        {
          msg=msgDefault;
          internal_update(_update_charset,addr,ProfileCharsetOptions::Default);
        }else
        if(!strncmp(body+i,"LATIN1",6))
        {
          msg=msgLatin1;
          internal_update(_update_charset,addr,ProfileCharsetOptions::Latin1);
        }else
        if(!strncmp(body+i,"UCS2ANDLAT",10))
        {
          msg=msgUCS2AndLat;
          internal_update(_update_charset,addr,ProfileCharsetOptions::Ucs2AndLat);
        }else
        if(!strncmp(body+i,"UCS2",4))
        {
          msg=msgUCS2;
          internal_update(_update_charset,addr,ProfileCharsetOptions::Ucs2);
        }
        if(!strncmp(body+i,"USSD7BIT",8))
        {
          i+=8;
          while(body[i]==' ')i++;
          if(!strcmp(body+i,"ON"))
          {
            internal_update(_update_charset_ussd,addr,1);
            msg=msg7BitUssdOn;
          }else if(!strcmp(body+i,"OFF"))
          {
            internal_update(_update_charset_ussd,addr,0);
            msg=msg7BitUssdOff;
          }
        }
        else
        if(!strncmp(body+i,"HIDE",4))
        {
          msg=msgHide;
          internal_update(_update_hide,addr,1);
        }else
        if(!strncmp(body+i,"UNHIDE",6))
        {
          msg=msgUnhide;
          internal_update(_update_hide,addr,0);
        }else
        if(!strncmp(body+i,"DIVERT",6))
        {
          i+=6;
          while(body[i] && isspace(body[i]))i++;
          int j=i;
          if(!strncmp(body+j,"TO",2))
          {
            while(body[i] && !isspace(body[i]))i++;
            while(body[i] && isspace(body[i]))i++;
            j=len-1;
            while(j>i && isspace(body[j]))j--;
            string div(body+i,body+j+1);
            __trace2__("divert address=%s",div.c_str());
            try{
              Address addrCheck(div.c_str());
            }catch(...)
            {
              msg=msgInvalidParam;
            }
            if(msg==-1 && div.length()<21)
            {
              msg=msgDivertChanged;
              internal_update(_update_divert,addr,0,div.c_str());
            }
          }else
          {
            //(abs|absent)|(blk|blocked)|(bar|bared)|(cap|capacity)
            string arg=body+j,val;
            string::size_type pos=arg.find(' ');
            if(pos!=string::npos)
            {
              string::size_type pos0=pos;
              while(pos<arg.length() && arg[pos]==' ')pos++;
              val=arg.substr(pos);
              arg.erase(pos0);
            }
            if(val.length()==0)
            {
              if(arg=="ON")
              {
                msg=msgDivertOn;
                internal_update(_update_divert_act,addr,true,0);
              }else
              if(arg=="OFF")
              {
                msg=msgDivertOff;
                internal_update(_update_divert_act,addr,false,0);
              }
            }else
            {
              int onbit=val=="ON"?update_div_cond_OnBit:0;
              if(onbit || val=="OFF")
              {
                if(arg=="ABS" || arg=="ABSENT")
                {
                  msg=onbit?msgDivertOn:msgDivertOff;
                  internal_update(_update_divert_cond,addr,update_div_cond_Absent|onbit,0);
                }else
                if(arg=="BLK" || arg=="BLOCKED")
                {
                  msg=onbit?msgDivertOn:msgDivertOff;
                  internal_update(_update_divert_cond,addr,update_div_cond_Blocked|onbit,0);
                }else
                if(arg=="BAR" || arg=="BARED")
                {
                  msg=onbit?msgDivertOn:msgDivertOff;
                  internal_update(_update_divert_cond,addr,update_div_cond_Bared|onbit,0);
                }else
                if(arg=="CAP" || arg=="CAPACITY")
                {
                  msg=onbit?msgDivertOn:msgDivertOff;
                  internal_update(_update_divert_cond,addr,update_div_cond_Capacity|onbit,0);
                }
              }
            }
          }
        }

      }catch(AccessDeniedException& e)
      {
        msg=msgAccessDenied;
      }

      resp=SmscCommand::makeDeliverySmResp(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str(),
                                             cmd->get_dialogId(),status);

      putIncomingCommand(resp);
      SMS ans;
      string msgstr;
      Profile p=lookup(addr);
      switch(msg)
      {
        case msgReportNone:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgReportNone");
        }break;
        case msgReportFull:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgReportFull");
        }break;
        case msgReportFinal:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgReportFinal");
        }break;
        case msgDefault:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgDefault");
        }break;
        case msgUCS2:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgUCS2");
        }break;
        case msgLatin1:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgLatin1");
        }break;
        case msgUCS2AndLat:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgUCS2AndLat");
        }break;
        case msgLocaleChanged:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgLocaleChanged");
        }break;
        case msgLocaleUnknown:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgLocaleUnknown");
        }break;
        case msgHide:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgHide");
        }break;
        case msgUnhide:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgUnhide");
        }break;
        case msgAccessDenied:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgModificationDenied");
        }break;
        case msgDivertOn:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgDivertOn");
        }break;
        case msgDivertOff:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgDivertOff");
        }break;
        case msgDivertChanged:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgDivertChanged");
        }break;
        case msgInvalidParam:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgInvalidParam");
        }break;
        case msg7BitUssdOn:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msg7BitUssdOn");
        }break;
        case msg7BitUssdOff:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msg7BitUssdOff");
        }break;
        default:
        {
          msgstr=ResourceManager::getInstance()->getString(p.locale,"profiler.msgError");
        };
      }
      __trace2__("Profiler: msgstr=%s",msgstr.c_str());
      ans.setOriginatingAddress(sms->getDestinationAddress());
      ans.setDestinationAddress(sms->getOriginatingAddress());
      char msc[]="";
      char imsi[]="";
      ans.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
      ans.setDeliveryReport(0);
      ans.setArchivationRequested(false);
      ans.setEServiceType(serviceType.c_str());
      ans.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS,0);
      ans.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protocolId);
      ans.setIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE,
        sms->getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE));
      if(sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP)==1)
      {
        ans.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,17);
        // clear 0,1 bits and set them to datagram mode
        ans.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS,
            (ans.getIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS)&~0x03)|0x01);
      }

      Profile pr=lookup(addr);
      __trace2__("profiler response:%s!",msgstr.c_str());
      /*
      if(pr.codepage==ProfileCharsetOptions::Default)
      {
        //len=ConvertTextTo7Bit(msgstr,strlen(msgstr),body,sizeof(body),CONV_ENCODING_CP1251);
        len=strlen(msgstr);
        memcpy(body,msgstr,len);
        ans.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
      }else
      {
        ConvertMultibyteToUCS2(msgstr,strlen(msgstr),(short*)body,sizeof(body),CONV_ENCODING_CP1251);
        ans.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::UCS2);
        len=strlen(msgstr)*2;
      }
      ans.setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,body,len);
      ans.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,len);
      SmscCommand answer=SmscCommand::makeSumbmitSm(ans,getNextSequenceNumber());
      putIncomingCommand(answer);
      */
      /*splitSms(&ans,msgstr,strlen(msgstr),CONV_ENCODING_CP1251,pr.codepage,smsarr);
      for(int i=0;i<smsarr.Count();i++)
      {
        SmscCommand answer=SmscCommand::makeSumbmitSm(*smsarr[i],getNextSequenceNumber());
        putIncomingCommand(answer);
        delete smsarr[i];
      }*/
      fillSms(&ans,msgstr.c_str(),msgstr.length(),CONV_ENCODING_CP1251,ProfileCharsetOptions::Ucs2);
      SmscCommand answer=SmscCommand::makeSumbmitSm(ans,getNextSequenceNumber());
      putIncomingCommand(answer);
    }catch(exception& e)
    {
      __warning2__("EXCEPTION IN PROFILER: %s",e.what());
    }catch(...)
    {
      __warning__("EXCEPTION IN PROFILER: unknown");
    }
  }
  return 0;
}

static int RsAsInt(smsc::db::ResultSet* rs,int idx)
{
  const char *r=rs->getString(idx);
  if(!r)return 0;
  return atoi(r);
}

static bool RsAsBool(smsc::db::ResultSet* rs,int idx)
{
  const char *r=rs->getString(idx);
  if(!r)return false;
  return toupper(*r)=='Y';
}

void Profiler::loadFromDB(smsc::db::DataSource *datasrc)
{
  ds=datasrc;
  using namespace smsc::db;
  using smsc::util::config::Manager;
  using smsc::util::config::ConfigView;
  using smsc::util::config::ConfigException;

  const char* sql = "SELECT MASK, REPORTINFO, CODESET ,LOCALE, HIDDEN, HIDDEN_MOD, DIVERT,DIVERT_ACT,DIVERT_MOD FROM SMS_PROFILE";
  //const char* sql = "SELECT MASK FROM SMS_PROFILE";


  ConnectionGuard connection(ds);

  if(!connection.get())throw Exception("Profiler: Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Profiler: Failed to create statement");
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception("Profiler: Failed to make a query to DB");
  const char* dta;

  hrtime_t st=gethrtime();

  while(rs->fetchNext())
  {

    Profile p;

    dta = rs->getString(1);
    if(!dta)
    {
      smsc_log_error(log,"EMPTY MASK IN PROFILE!");
      continue;
    }
    Address addr(dta);

    p.reportoptions=RsAsInt(rs.get(),2);
    p.codepage=RsAsInt(rs.get(),3);
    const char * l=rs->getString(4);
    p.locale=l?l:"";
    p.hide=RsAsBool(rs.get(),5);
    p.hideModifiable=RsAsBool(rs.get(),6);
    const char *d=rs->getString(7);
    p.divert=d?d:"";
    const char* dopt=rs->getString(8);
    if(dopt)
    {
      int len=strlen(dopt);
      if(len>0)p.divertActive        =toupper(dopt[0])=='Y';

      if(len>1)p.divertActiveAbsent  =toupper(dopt[1])=='Y';
      if(len>2)p.divertActiveBlocked =toupper(dopt[2])=='Y';
      if(len>3)p.divertActiveBared   =toupper(dopt[3])=='Y';
      if(len>4)p.divertActiveCapacity=toupper(dopt[4])=='Y';
    }

    p.divertModifiable=RsAsBool(rs.get(),9);


    //debug2(log,"init:%s=%s",addr.toString().c_str(),DumpProfile(p).c_str());

    profiles->add(addr,p);
  }


  /*
  for(int i=0;i<500000;i++)
  {
    char buf[32];
    sprintf(buf,".1.1.913%07d",i);
    Address addr(buf);
    Profile p;
    profiles->add(addr,p);
  }
  */
  st=gethrtime()-st;
  smsc_log_info(log,"load time=%lld",st/1000000l);
  int mc,bu,bn;
  profiles->GetStats(mc,bu,bn);
  smsc_log_info(log,"hash stats max=%d,bucks used=%d,count=%d,bucks num=%d\n",mc,bu,profiles->Count(),bn);
}

}//profiler
}//smsc
