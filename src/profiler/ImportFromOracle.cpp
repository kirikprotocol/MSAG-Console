#include <stdio.h>
#include "profiler/profiler-types.hpp"
#include "util/config/Manager.h"
#include "util/findConfigFile.h"
#include "db/DataSourceLoader.h"
#include "logger/Logger.h"
#include "core/buffers/File.hpp"

using namespace smsc::profiler;
using namespace smsc::util;
using namespace smsc::store;
using namespace smsc::sms;
using namespace smsc::db;
using namespace smsc::core::buffers;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;

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

static int RsAsHide(smsc::db::ResultSet* rs,int idx)
{
  const char *r=rs->getString(idx);
  if(!r)return 0;
  return toupper(*r)=='Y'?HideOption::hoEnabled:toupper(*r)=='S'?HideOption::hoSubstitute:HideOption::hoDisabled;
}

static void ReadAddress(File& f,Address& addr)
{
  addr.type=f.ReadByte();
  addr.plan=f.ReadByte();
  f.Read(addr.value,sizeof(addr.value));
  addr.value[sizeof(addr.value)-1]=0;
  addr.length=strlen(addr.value);
}

static void WriteAddress(File& f,const Address& addr)
{
  f.WriteByte(addr.type);
  f.WriteByte(addr.plan);
  AddressValue buf={0,};
  strcpy(buf,addr.value);
  f.Write(buf,sizeof(buf));
}

static uint32_t AddressSize()
{
  return 1+1+sizeof(AddressValue);
}

static char profileMagic[]="SmScPrOf";


int main(int argc,char* argv[])
{
  Logger::Init();
  Logger* log=Logger::getInstance("ora2file");
  smsc::logger::Logger *logger = Logger::getInstance("smscmain");
  smsc::util::config::Manager::init(findConfigFile("config.xml"));
  using namespace smsc::db;
  using smsc::util::config::ConfigView;
  const char* OCI_DS_FACTORY_IDENTITY = "OCI";

  config::Manager& cfgman=config::Manager::getInstance();

  std::auto_ptr<ConfigView> dsConfig(new config::ConfigView(config::Manager::getInstance(), "StartupLoader"));
  DataSourceLoader::loadup(dsConfig.get());

  DataSource* dataSource = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
  std::auto_ptr<ConfigView> config(new ConfigView(cfgman,"DataSource"));

  dataSource->init(config.get());

  const char* sql = "SELECT MASK, REPORTINFO, CODESET ,LOCALE, HIDDEN, HIDDEN_MOD, DIVERT,DIVERT_ACT,DIVERT_MOD,UDHCONCAT,TRANSLIT FROM SMS_PROFILE";
  //const char* sql = "SELECT MASK FROM SMS_PROFILE";


  ConnectionGuard connection(dataSource);

  if(!connection.get())throw Exception("Profiler: Failed to get connection");
  auto_ptr<Statement> statement(connection->createStatement(sql));
  if(!statement.get())throw Exception("Profiler: Failed to create statement");
  auto_ptr<ResultSet> rs(statement->executeQuery());
  if(!rs.get())throw Exception("Profiler: Failed to make a query to DB");
  const char* dta;

  const char sig[]="SMSCPROF";
  const uint32_t ver=0x00010000;

  File storeFile;

  const char* filename=cfgman.getString("profiler.storeFile");

  if(!File::Exists(filename))
  {
    __warning2__("Profiler store file not found:%s",filename);
    storeFile.RWCreate(filename);
    storeFile.Write(sig,8);
    storeFile.WriteNetInt32(ver);
    storeFile.Flush();
  }else
  {
    storeFile.RWOpen(filename);
  }

  int count=0;

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
    p.hide=RsAsHide(rs.get(),5);
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
      if(len>3)p.divertActiveBarred  =toupper(dopt[3])=='Y';
      if(len>4)p.divertActiveCapacity=toupper(dopt[4])=='Y';
    }

    p.divertModifiable=RsAsBool(rs.get(),9);

    p.udhconcat=RsAsBool(rs.get(),10);
    p.translit=RsAsBool(rs.get(),11);

    storeFile.WriteByte(1);
    storeFile.Write(profileMagic,8);
    WriteAddress(storeFile,addr);
    p.offset=storeFile.Pos();
    p.Write(storeFile);
    storeFile.Flush();
    count++;
  }
  printf("%d profiles imported ok\n",count);


  return 0;
}
