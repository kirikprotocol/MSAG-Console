#include <stdio.h>
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "core/buffers/Array.hpp"
#include <signal.h>
#include "db/DataSource.h"
#include "db/DataSourceLoader.h"
#include "util/templates/Formatters.h"
#include "util/xml/init.h"

#include "acls/interfaces.h"

//using namespace smsc::sms;
using namespace smsc::util;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using smsc::core::buffers::Hash;
using smsc::core::buffers::Array;
using namespace std;
using namespace smsc::util;
using namespace smsc::db;

using namespace smsc::acls;

extern "C"  void disp(int sig);
extern "C"  void ctrlc(int sig);
extern "C" void atExitHandler(void);

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  atexit(atExitHandler);

  try
  {
    
    DataSource *dataSource;
    const char* OCI_DS_FACTORY_IDENTITY = "OCI";

    config::Manager::init("config.xml");
    config::Manager& cfgman= config::Manager::getInstance();
    config::ConfigView *dsConfig = new config::ConfigView(cfgman, "StartupLoader");

    DataSourceLoader::loadup(dsConfig);

    dataSource = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
    if (!dataSource) throw Exception("Failed to get DataSource");
    auto_ptr<ConfigView> config(new ConfigView(cfgman,"DataSource"));

    dataSource->init(config.get());

    auto_ptr<AclAbstractMgr> acl ( AclAbstractMgr::Create2() );
    acl->LoadUp(dataSource);
    acl->remove(1);

    {
      vector<AclPhoneNumber> phones;
      for ( int i = 0; i < 10; ++i ) {
        char buf[128];
        sprintf(buf,"+0%d",i);
        phones.push_back(buf);
      }
      acl->create(1,"acl0","acl0_desc",phones,ACT_FULLCACHE);
      //acl->create(1,"acl0","acl0_desc",phones,ACT_DBSDIRECT);
    }

    {
      AclInfo aclInfo = acl->getInfo(1);
      printf("Getted info: #%d, name:\"%s\", description:\"%s\"\n", aclInfo.ident, aclInfo.name.c_str(), aclInfo.desctiption.c_str());
    }

    vector<AclNamedIdent> result;
    acl->enumerate(result);

    for ( int i = 0; i < result.size(); ++i ) {
      printf("%d,%s\n",result[i].first,result[i].second.c_str());
    }

    for ( int i = 0; i < 10; ++i ) {
      char buf[128];
      sprintf(buf,"+1%d",i);
      acl->addPhone(1,buf);
    }

    {
      vector<AclPhoneNumber> result1;
      acl->lookupByPrefix(1,"0",result1);
      for ( int i = 0; i < result1.size(); ++i ) {
        printf("#,%s\n",result1[i].c_str());
      }
    }

    {
      vector<AclPhoneNumber> result1;
      acl->lookupByPrefix(1,"1",result1);
      for ( int i = 0; i < result1.size(); ++i ) {
        printf("#,%s\n",result1[i].c_str());
      }
    }

    for ( int i = 0; i < 10; ++i ) {
      char buf[128];
      sprintf(buf,"+1%d",i);
      printf("%s graned = %d\n", buf, acl->isGranted(1,buf));
    }

    for ( int i = 0; i < 2; ++i ) {
      char buf[128];
      sprintf(buf,"+4%d",i);
      printf("%s graned = %d\n", buf, acl->isGranted(1,buf));
    }

    acl->remove(1);
    printf("erase\n");

    {
      vector<AclPhoneNumber> result1;
      acl->lookupByPrefix(1,0,result1);
      for ( int i = 0; i < result1.size(); ++i ) {
        printf("#,%s\n",result1[i].c_str());
      }
    }

    printf("success\n");
  }
  catch(exception& e)
  {
    printf("Top level exception:%s\n",e.what());
  }
  catch(...)
  {
    printf("Top level exception:unknown\n");
  }
  return 0;
}

extern "C" void atExitHandler(void)
{
  smsc::util::xml::TerminateXerces();
  smsc::logger::Logger::Shutdown();
}

