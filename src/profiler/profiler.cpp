#include "profiler/profiler.hpp"
#include <vector>
#include "core/buffers/Array.hpp"
#include "core/buffers/XHash.hpp"

#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

#include "db/DataSource.h"
#include "db/DataSourceLoader.h"

#include "memory"

namespace smsc{
namespace profiler{

using namespace smsc::core::buffers;
using smsc::util::Exception;

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
    while(defLength<address.lenght && address.value[defLength]!='?')defLength++;
  }

  bool operator==(const HashKey& key)
  {
    return key.addr.lenght==addr.lenght &&
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
    while(!Exists(k))
    {
      if(!k.defLength)
      {
        return defaultProfile;
      }
      k.defLength--;
    }
    exact=k.defLength==address.lenght;
    return Get(k);
  }
  Profile& add(const Address& address,const Profile& profile)
  {
    HashKey k(address);
    Insert(k,profile);
    return Get(k);
  }
};


Profiler::Profiler(const Profile& pr)
{
  state=VALID;
  managerMonitor=NULL;
  profiles=new ProfilesTable(pr,400000);
}

Profiler::~Profiler()
{
  delete profiles;
}

Profile& Profiler::lookup(const Address& address)
{
  bool exact;
  return profiles->find(address,exact);
}

void Profiler::update(const Address& address,const Profile& profile)
{
  bool exact;
  Profile &prof=profiles->find(address,exact);
  if(prof==profile)return;
  if(exact)
  {
    prof.assign(profile);
    dbUpdate(address,prof);
  }else
  {
    dbInsert(address,profiles->add(address,profile));
  }
}

void Profiler::add(const Address& address,const Profile& profile)
{
  profiles->add(address,profile);
}

void Profiler::dbUpdate(const Address& addr,const Profile& profile)
{
  using namespace smsc::db;
  using smsc::util::config::Manager;
  using smsc::util::config::ConfigView;
  using smsc::util::config::ConfigException;
  const char *sql="UPDATE SMS_PROFILE SET reportinfo=:1, codeset=:2 where mask=:3";
  std::auto_ptr<Connection> connection(ds->getConnection());
  if(!connection.get())throw Exception("Profiler: Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Profiler: Failed to create statement");
  statement->setInt8(1,profile.reportoptions);
  statement->setInt8(2,profile.codepage);
  char addrbuf[30];
  sprintf(addrbuf,"%d.%d.%s",addr.type,addr.plan,addr.value);
  statement->setString(3,addrbuf);
  statement->executeUpdate();
}

void Profiler::dbInsert(const Address& addr,const Profile& profile)
{
  using namespace smsc::db;
  using smsc::util::config::Manager;
  using smsc::util::config::ConfigView;
  using smsc::util::config::ConfigException;
  const char* sql = "INSERT INTO SMS_PROFILE (mask, reportinfo, codeset)"
                      " VALUES (:1, :2, :3)";
  std::auto_ptr<Connection> connection(ds->getConnection());
  if(!connection.get())throw Exception("Profiler: Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  char addrbuf[30];
  sprintf(addrbuf,"%d.%d.%s",addr.type,addr.plan,addr.value);
  statement->setString(1, addrbuf);
  statement->setInt8(2, profile.reportoptions);
  statement->setInt8(3, profile.codepage);
  statement->executeUpdate();
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
  SmscCommand cmd;
  SMS *sms;
  int len;
  char body[MAX_SHORT_MESSAGE_LENGTH];
  while(!isStopping)
  {
    waitFor();
    if(!hasOutput())continue;
    cmd=getOutgoingCommand();
    if(cmd->cmdid!=smsc::smeman::SUBMIT)
    {
      __trace2__("Profiler: incorrect command submitted");
      continue;
    }
    sms = cmd->get_sms();
    Address& addr=sms->getOriginatingAddress();
    len = sms->getMessageBody().getData( (uint8_t*)body );
    if(!strncmp(body,"REPORT",6))
    {
      int i=7;
      while(i<len && !isalpha(body[i]))i++;
      if(i<len)
      {
        if(!strncmp(body+i,"NONE",4))
        {
          internal_update(_update_report,addr,ProfileReportOptions::ReportNone);
        }
        else
        if(!strncmp(body+i,"FULL",4))
        {
          internal_update(_update_report,addr,ProfileReportOptions::ReportFull);
        }
      }
    }else
    if(!strncmp(body,"UCS2",4))
    {
      internal_update(_update_charset,addr,ProfileCharsetOptions::Ucs2);

    }else
    if(!strncmp(body,"DEFAULT",4))
    {
      internal_update(_update_charset,addr,ProfileCharsetOptions::Default);
    }
  }
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

  Manager::init("config.xml");
  Manager& manager = Manager::getInstance();

  dsConfig = new ConfigView(manager, "StartupLoader");
  DataSourceLoader::loadup(dsConfig);

  ds = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
  if (!ds) throw Exception("Failed to get DataSource");
  ConfigView* config =
      new ConfigView(Manager::getInstance(),"DataSource");

  ds->init(config);
  __trace__("Profiler: init ok");
  std::auto_ptr<Connection> connection(ds->getConnection());
  if(!connection.get())throw Exception("Profiler: Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Profiler: Failed to create statement");
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception("Profiler: Failed to make a query to DB");
  Address addr;
  const char* dta;
  int np;
  int ton;
  Profile p;
  while(rs->fetchNext())
  {
    dta = rs->getString(1);
    int scaned = sscanf(dta,".%d.%d.%20s",
         &ton,
         &np,
         addr.value);
    if ( scaned == 3 )
    {
      addr.type=ton;
      addr.plan=np;
    }else
    {
      scaned = sscanf(dta,"+%[0123456789]20s",addr.value);
      if ( scaned )
      {
        addr.plan = 1;//ISDN
        addr.type = 1;//INTERNATIONAL
      }
      else
      {
        scaned = sscanf(dta,"%[0123456789]20s",addr.value);
        if ( !scaned )
        {
          continue;
        }
        else
        {
          addr.plan = 1;//ISDN
          addr.type = 2;//NATIONAL
        }
      }
    }
    addr.lenght=strlen((char*)addr.value);
    p.reportoptions=rs->getInt8(2);
    p.codepage=rs->getInt8(3);
    profiles->add(addr,p);
  }
}

};//profiler
};//smsc
