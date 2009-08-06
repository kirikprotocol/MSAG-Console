#include "stdio.h"
#include "unistd.h"

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>
#include <db/DataSourceLoader.h>

#include <db/DataSource.h>

#include <mcisme/Profiler.h>
#include <mcisme/ProfilesStorage.hpp>

using namespace smsc::util::config;
using namespace smsc::db;
using namespace smsc::mcisme;
using namespace std;

static smsc::logger::Logger *logger = 0;

DataSource*         ds;
Connection*         dsStatConnection;


const char* GET_ALL_ID  = "GET_ALL_ID";
const char* GET_COUNT_ID  = "GET_COUNT_ID";

const char* GET_ABONENT_PRO_ID  = "GET_ABONENT_PRO_ID";
const char* SET_ABONENT_PRO_ID  = "SET_ABONENT_PRO_ID";
const char* INS_ABONENT_PRO_ID  = "INS_ABONENT_PRO_ID";
const char* DEL_ABONENT_PRO_ID  = "DEL_ABONENT_PRO_ID";

const char* GET_ALL_SQL = "SELECT * FROM MCISME_ABONENTS";
const char* GET_COUNT_SQL = "SELECT COUNT(ABONENT) FROM MCISME_ABONENTS";


const char* GET_ABONENT_PRO_SQL = "SELECT INFORM, NOTIFY, INFORM_ID, NOTIFY_ID, EVENT_MASK FROM MCISME_ABONENTS "
"WHERE ABONENT=:ABONENT";
const char* SET_ABONENT_PRO_SQL = "UPDATE MCISME_ABONENTS SET INFORM=:INFORM, NOTIFY=:NOTIFY, "
"INFORM_ID=:INFORM_ID, NOTIFY_ID=:NOTIFY_ID, EVENT_MASK=:EVENT_MASK "
"WHERE ABONENT=:ABONENT";
const char* INS_ABONENT_PRO_SQL = "INSERT INTO MCISME_ABONENTS (ABONENT, INFORM, INFORM_ID, NOTIFY_ID, EVENT_MASK) "
"VALUES (:ABONENT, :INFORM, :INFORM_ID, :NOTIFY_ID, :EVENT_MASK)";
const char* DEL_ABONENT_PRO_SQL = "DELETE FROM MCISME_ABONENTS WHERE ABONENT=:ABONENT";

void initDataSource(ConfigView* config)
{
  try  {
    std::auto_ptr<char> dsIdentity(config->getString("type"));
    const char* dsIdentityStr = dsIdentity.get();
    ds = DataSourceFactory::getDataSource(dsIdentityStr);
    if (!ds) throw ConfigException("DataSource for '%s' identity wasn't registered !", dsIdentityStr);
    ds->init(config);
  } 
  catch (ConfigException& exc) {
    if (ds) delete ds; ds = 0;
    throw;
  }
}

int main(int argc, char** argv)
{
  printf("Any other programs working with this file MUST BE CLOSED\n");
  
  Logger::Init();
  logger = Logger::getInstance("DBProf2file");
  
  try
  {
    //        smsc_log_info(logger, getStrVersion());
    
    Manager::init("./conf/config.xml");
    Manager& manager = Manager::getInstance();
    
    ConfigView tpConfig(manager, "MCISme");
    
    // Init ProfileStorage
    std::auto_ptr<ConfigView> profStorCfgGuard(tpConfig.getSubConfig("ProfileStorage"));
    ConfigView* profStorCfg = profStorCfgGuard.get();
    string locationProfStor;
    try {locationProfStor = profStorCfg->getString("location"); } catch (...){locationProfStor="./";
      printf("Parameter <MCISme.ProfileStorage.location> missed. Default value is './'\n");}
    printf("Profile Storage location is '%s'\n", locationProfStor.c_str());
    ProfilesStorage::Open(locationProfStor);
    printf("Profile Storage is opened\n");
    
    ConfigView dsConfig(manager, "StartupLoader");
    DataSourceLoader::loadup(&dsConfig);
    
    std::auto_ptr<ConfigView> dsIntCfgGuard(tpConfig.getSubConfig("DataSource"));
    initDataSource(dsIntCfgGuard.get());
    dsStatConnection = ds->getConnection();
    if (!dsStatConnection)
      printf("throw Exception 0");
    
    Statement* getProStmt = dsStatConnection->getStatement(GET_ALL_ID, GET_ALL_SQL);
    if (!getProStmt)
      printf("throw Exception 1");
    
    //        getProStmt->setString(1, abonent);
    std::auto_ptr<ResultSet> rsGuard(getProStmt->executeQuery());
    ResultSet* rs = rsGuard.get();
    if (!rs)
      printf("throw Exception 2");
    AbonentProfile profile;
    AbntAddr       abnt;
    ProfilesStorage* profStorage = ProfilesStorage::GetInstance();
      
    while(rs->fetchNext())
    {
      const char* abntStr = rs->isNull(1) ? 0:rs->getString(1);
      const char* infStr = rs->isNull(2) ? 0:rs->getString(2);
      const char* notStr = rs->isNull(3) ? 0:rs->getString(3);
      int	infId = rs->isNull(4) ? 0:rs->getUint32(4);
      int	notId = rs->isNull(5) ? 0:rs->getUint32(5);
      uint8_t	eventMask = rs->isNull(6) ? 0:rs->getUint8(6);
      //profile.inform   = (infStr && (infStr[0]=='Y' || infStr[0]=='y'));
      //profile.notify   = (notStr && (notStr[0]=='Y' || notStr[0]=='y'));
      //profile.informTemplateId = rs->isNull(3) ?   -1:rs->getUint32(3);
      //profile.notifyTemplateId = rs->isNull(4) ?   -1:rs->getUint32(4);
      //profile.eventMask = rs->isNull(5) ? AbonentProfiler::defaultEventMask : rs->getUint8(5);
      printf("Process Abonent %s inform:%s notify:%s informTemplate:%d notifyTemplate:%d eventMask:%d\n", abntStr, infStr, notStr, infId, notId, eventMask);
      profile.eventMask = eventMask;
      profile.inform = (*infStr=='Y')?true:false;
      profile.notify = (*notStr=='Y')?true:false;
      if( infId == 4 ) profile.informTemplateId = 2;
      else profile.informTemplateId = 1;
      if( notId == 3 ) profile.notifyTemplateId = 2;
      else profile.notifyTemplateId = 1;
      abnt = AbntAddr(abntStr);
      printf("Set Abonent %s inform:%d notify:%d informTemplate:%d notifyTemplate:%d eventMask:%d\n", abnt.getText().c_str(), profile.inform, profile.notify, profile.informTemplateId, profile.notifyTemplateId, profile.eventMask);
      profStorage->Set(abnt,profile);
    } 
    printf("Done\n");
  }
  catch(...)
  {
    printf("Exception\n");
  }

  return 0;
}
