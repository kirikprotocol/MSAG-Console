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




class HashFunc{
public:
static int CalcHash(const HashKey& key)
{
  int retval=key.addr.type^key.addr.plan;
  int i;
  for(i=0;i<key.defLength;i++)
  {
    retval=(retval<<1)^key.addr.value[i];
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
  Profile& add(const Address& address,const Profile& profile)
  {
    HashKey k(address);
    Insert(k,profile);
    return Get(k);
  }
};


Profiler::Profiler(const Profile& pr,SmeRegistrar* psmeman,const char* sysId)
{
  state=VALID;
  managerMonitor=NULL;
  profiles=new ProfilesTable(pr,400000);
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
    dbInsert(address,profiles->add(address,profile));
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
  statement->setInt8(1,profile.reportoptions);
  statement->setUint8(2,profile.codepage);
  char addrbuf[30];
  addr.toString(addrbuf,sizeof(addrbuf));
  __trace2__("Profiler: dbUpdate %s=%d,%d,%s,%d,%c,%s,%c,%c",addrbuf,profile.reportoptions,profile.codepage,profile.locale.c_str(),profile.hide,profile.hideModifiable?'Y':'N',profile.divert.c_str(),profile.divertActive?'Y':'N',profile.divertModifiable?'Y':'N');
  statement->setString(3,profile.locale.c_str());
  statement->setInt8(4,profile.hide);
  statement->setString(5,profile.hideModifiable?"Y":"N");
  statement->setString(6,profile.divert.c_str());
  statement->setString(7,profile.divertActive?"Y":"N");
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
  statement->setInt8(2, profile.reportoptions);
  statement->setUint8(3, profile.codepage);
  statement->setString(4,profile.locale.c_str());
  statement->setInt8(5, profile.hide);
  statement->setString(6, profile.hideModifiable?"Y":"N");
  statement->setString(7,profile.divert.c_str());
  statement->setString(8, profile.divertActive?"Y":"N");
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
      }catch(...)
      {
        status=MAKE_COMMAND_STATUS(CMD_ERR_PERM,Status::INVOPTPARAMVAL);
        resp=SmscCommand::makeDeliverySmResp(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str(),
                                             cmd->get_dialogId(),status);
        putIncomingCommand(resp);
        __warning2__("INVALID MESSAGE SENT TO PROFILER FROM",addr.toString().c_str());
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
          if(!strncmp(body+j,"ON",2))
          {
            msg=msgDivertOn;
            internal_update(_update_divert_act,addr,true,0);
          }else
          if(!strncmp(body+j,"OFF",3))
          {
            msg=msgDivertOff;
            internal_update(_update_divert_act,addr,false,0);
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
      fillSms(&ans,msgstr.c_str(),msgstr.length(),CONV_ENCODING_CP1251,ProfileCharsetOptions::UCS2);
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

void Profiler::loadFromDB(smsc::db::DataSource *datasrc)
{
  ds=datasrc;
  using namespace smsc::db;
  using smsc::util::config::Manager;
  using smsc::util::config::ConfigView;
  using smsc::util::config::ConfigException;

  const char* sql = "SELECT MASK, REPORTINFO, CODESET ,LOCALE, HIDDEN, HIDDEN_MOD, DIVERT,DIVERT_ACT,DIVERT_MOD FROM SMS_PROFILE";


  ConnectionGuard connection(ds);

  if(!connection.get())throw Exception("Profiler: Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Profiler: Failed to create statement");
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception("Profiler: Failed to make a query to DB");
  const char* dta;
  Profile p;
  while(rs->fetchNext())
  {
    dta = rs->getString(1);
    Address addr(dta);
    p.reportoptions=rs->getInt8(2);
    p.codepage=rs->getUint8(3);
    const char * l=rs->getString(4);
    p.locale=l?l:"";
    p.hide=rs->getInt8(5);
    const char *hm=rs->getString(6);
    p.hideModifiable=hm?toupper(*hm)=='Y':false;
    const char *d=rs->getString(7);
    debug2(log,"divert.getString=%s",d?d:"NULL");
    p.divert=d?d:"";
    hm=rs->getString(8);
    p.divertActive=hm?toupper(*hm)=='Y':false;
    hm=rs->getString(9);
    p.divertModifiable=hm?toupper(*hm)=='Y':false;
    debug2(log,"init:%s=%s",addr.toString().c_str(),DumpProfile(p).c_str());
    profiles->add(addr,p);
  }
}

}//profiler
}//smsc
