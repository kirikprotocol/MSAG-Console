#include <vector>
#include <memory>
#include <ctype.h>


#include "profiler/profiler.hpp"

#include "core/buffers/Array.hpp"
#include "core/buffers/XHash.hpp"

#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

#include "db/DataSource.h"
#include "db/DataSourceLoader.h"

#include "util/recoder/recode_dll.h"
#include "util/smstext.h"
#include "util/Logger.h"

namespace smsc{
namespace profiler{

using namespace smsc::core::buffers;
using smsc::util::Exception;
using namespace smsc::util;

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
    defLength=0;
    while(defLength<address.length && address.value[defLength]!='?')defLength++;
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

class ProfilesTable: public smsc::core::buffers::XHash<HashKey,Profile,HashFunc>
{
  Profile defaultProfile;
public:
  ProfilesTable(const Profile& pr,int n):XHash<HashKey,Profile,HashFunc>(n)
  {
    defaultProfile=pr;
  }
  Profile& find(const Address& address,bool& exact)
  {
    HashKey k(address);
    exact=false;
    while(!Exists(k))
    {
      if(!k.defLength)
      {
        return defaultProfile;
      }
      k.defLength--;
    }
    exact=k.defLength==address.length;
    return Get(k);
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

int Profiler::update(const Address& address,const Profile& profile)
{
  MutexGuard g(mtx);
  bool exact;
#ifndef DISABLE_TRACING
  char buf[32];
  address.getText(buf,sizeof(buf));
#endif
  Profile &prof=profiles->find(address,exact);
  __trace2__("Profiler: update %s:%d.%d->%d.%d(%s)",buf,prof.codepage,prof.reportoptions,
    profile.codepage,profile.reportoptions,exact?"exact":"inexact");
  if(prof==profile)return pusUnchanged;
  try{
  if(exact)
  {
    prof.assign(profile);
    dbUpdate(address,prof);
    __trace2__("Profiler: update %s:%d.%d",buf,profile.codepage,profile.reportoptions);
    return pusUpdated;
  }else
  {
    dbInsert(address,profiles->add(address,profile));
    __trace2__("Profiler: insert %s:%d.%d",buf,profile.codepage,profile.reportoptions);
    return pusInserted;
  }
  }catch(...)
  {
    log4cpp::Category &log=smsc::util::Logger::getCategory("smsc.system.Profiler");
    log.error("Database exception during profile update/insert");
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
  const char *sql="UPDATE SMS_PROFILE SET reportinfo=:1, codeset=:2 where mask=:3";
  ConnectionGuard connection(ds);
  if(!connection.get())throw Exception("Profiler: Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Profiler: Failed to create statement");
  statement->setInt8(1,profile.reportoptions);
  statement->setInt8(2,profile.codepage);
  char addrbuf[30];
  sprintf(addrbuf,".%d.%d.%s",addr.type,addr.plan,addr.value);
  __trace2__("profiler: update %s",addrbuf);
  statement->setString(3,addrbuf);
  statement->executeUpdate();
  connection->commit();
}

void Profiler::dbInsert(const Address& addr,const Profile& profile)
{
  using namespace smsc::db;
  using smsc::util::config::Manager;
  using smsc::util::config::ConfigView;
  using smsc::util::config::ConfigException;
  const char* sql = "INSERT INTO SMS_PROFILE (mask, reportinfo, codeset)"
                      " VALUES (:1, :2, :3)";

  ConnectionGuard connection(ds);

  if(!connection.get())throw Exception("Profiler: Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  char addrbuf[30];
  AddressValue val;
  addr.getValue(val);
  sprintf(addrbuf,".%d.%d.%s",addr.type,addr.plan,val);
  __trace2__("profiler: insert %s",addrbuf);
  statement->setString(1, addrbuf);
  statement->setInt8(2, profile.reportoptions);
  statement->setInt8(3, profile.codepage);
  statement->executeUpdate();
  connection->commit();
}

static const int _update_report=1;
static const int _update_charset=2;

void Profiler::internal_update(int flag,const Address& addr,int value)
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
  update(addr,profile);
}


int Profiler::Execute()
{
  SmscCommand cmd,resp;
  SMS *sms;
  Array<SMS*> smsarr;
  int len;
  char body[MAX_SHORT_MESSAGE_LENGTH+1];
//  char buf[MAX_SHORT_MESSAGE_LENGTH+1];
//  int coding;

//  char *str=body;
  int msg;
  while(!isStopping)
  {
    if(!hasOutput())
    {
      waitFor();
    }
    if(!hasOutput())continue;
    cmd=getOutgoingCommand();
    if(cmd->cmdid!=smsc::smeman::DELIVERY)
    {
      __warning2__("Profiler: incorrect command submitted - %d",cmd->cmdid);
      continue;
    }
    sms = cmd->get_sms();
    int status=MAKE_COMMAND_STATUS(CMD_OK,0);
    if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3f)!=3 &&
       (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3f)!=0
      )
    {
      status=MAKE_COMMAND_STATUS(CMD_ERR_PERM,SmppStatusSet::ESME_RINVESMCLASS);
      resp=SmscCommand::makeDeliverySmResp(sms->getStrProperty("SMPP_RECEIPTED_MESSAGE_ID").c_str(),
                                           cmd->get_dialogId(),status);
      putIncomingCommand(resp);
      continue;
    };

    Address& addr=sms->getOriginatingAddress();
    //len = sms->getMessageBody().getData( (uint8_t*)body );
    /*strncpy(buf,sms->getStrProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE).c_str(),sizeof(buf));
    len = sms->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
    coding = sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING);
    __trace2__("Profiler: data coding %d",coding);
    if(coding==DataCoding::DEFAULT)
    {
      Convert7BitToText(buf,len,body,sizeof(body));
    }else if(coding==DataCoding::UCS2)
    {
      ConvertUCS2ToMultibyte((const short*)buf,len,body,sizeof(body),CONV_ENCODING_ANSI);
      body[len/2]=0;
    }else
    {
      memcpy(body,buf,len);
      body[len]=0;
    }
    len=strlen(body);*/
    len=getSmsText(sms,body,sizeof(body));

    __trace2__("Profiler: received %s from .%d.%d.%.20s",body,addr.type,addr.plan,addr.value);

    int i;
    for(i=0;i<len;i++)body[i]=toupper(body[i]);
    i=0;
    while(!isalpha(body[i]) && i<len)i++;
    if(!strncmp(body+i,"REPORT",6))
    {
      i+=7;
      while(i<len && !isalpha(body[i]))i++;
      if(i<len)
      {
        if(!strncmp(body+i,"NONE",4))
        {
          msg=0;
          internal_update(_update_report,addr,ProfileReportOptions::ReportNone);
        }
        else
        if(!strncmp(body+i,"FULL",4))
        {
          msg=1;
          internal_update(_update_report,addr,ProfileReportOptions::ReportFull);
        }
      }
    }else
    if(!strncmp(body+i,"DEFAULT",4))
    {
      msg=2;
      internal_update(_update_charset,addr,ProfileCharsetOptions::Default);
    }else
    if(!strncmp(body+i,"UCS2",4))
    {
      msg=3;
      internal_update(_update_charset,addr,ProfileCharsetOptions::Ucs2);
    }else
    {
      msg=9999;
    }
    resp=SmscCommand::makeDeliverySmResp(sms->getStrProperty("SMPP_RECEIPTED_MESSAGE_ID").c_str(),
                                           cmd->get_dialogId(),status);

    putIncomingCommand(resp);
    SMS ans;
    const char *msgstr;
    switch(msg)
    {
      case 0:
      {
        msgstr=msgRepNone.c_str();
      }break;
      case 1:
      {
        msgstr=msgRepFull.c_str();
      }break;
      case 2:
      {
        msgstr=msgDCDef.c_str();
      }break;
      case 3:
      {
        msgstr=msgDCUCS2.c_str();
      }break;
      default:
      {
        msgstr=msgError.c_str();
      };
    }
    ans.setOriginatingAddress(sms->getDestinationAddress());
    ans.setDestinationAddress(sms->getOriginatingAddress());
    char msc[]="123";
    char imsi[]="123";
    ans.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
    ans.setDeliveryReport(0);
    ans.setArchivationRequested(false);
    ans.setEServiceType(serviceType.c_str());
    ans.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID,protocolId);
    ans.setIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE,
      sms->getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE));

    Profile pr=lookup(addr);
    __trace2__("profiler response:%s!",msgstr);
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
    splitSms(&ans,msgstr,strlen(msgstr),CONV_ENCODING_CP1251,pr.codepage,smsarr);
    for(int i=0;i<smsarr.Count();i++)
    {
      SmscCommand answer=SmscCommand::makeSumbmitSm(*smsarr[i],getNextSequenceNumber());
      putIncomingCommand(answer);
      delete smsarr[i];
    }
  }
  return 0;
}

void Profiler::loadFromDB()
{
  using namespace smsc::db;
  using smsc::util::config::Manager;
  using smsc::util::config::ConfigView;
  using smsc::util::config::ConfigException;

  const char* OCI_DS_FACTORY_IDENTITY = "OCI";

  const char* sql = "SELECT * FROM SMS_PROFILE";

  ConfigView *dsConfig;

  //Manager::init("config.xml");
  Manager& manager = Manager::getInstance();

  dsConfig = new ConfigView(manager, "StartupLoader");
  DataSourceLoader::loadup(dsConfig);

  ds = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
  if (!ds) throw Exception("Failed to get DataSource");
  ConfigView* config =
      new ConfigView(Manager::getInstance(),"DataSource");

  ds->init(config);
  __trace__("Profiler: init ok");

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
    p.codepage=rs->getInt8(3);
    profiles->add(addr,p);
  }
}

};//profiler
};//smsc
