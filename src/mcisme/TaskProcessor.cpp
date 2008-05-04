//------------------------------------
//  TaskProcessor.cpp
//  Changed by Routman Michael, 2005-2006
//------------------------------------



#include <exception>
#include <algorithm>
#include <iconv.h>
#include <string>

#include <mcisme/TaskProcessor.h>
#include <mcisme/misscall/callproc.hpp>
#include <mcisme/FSStorage.hpp>
#include <mcisme/AbntAddr.hpp>
#include <mcisme/Profiler.h>
#include "ProfilesStorage.hpp"

#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <system/status.h>
#include "system/common/TimeZoneMan.hpp"
#include <time.h>
#include "Templates.h"
#include "scag/util/encodings/Encodings.h"

extern bool isMSISDNAddress(const char* string);
extern "C" void clearSignalMask(void);

namespace smsc {
namespace mcisme {

using namespace scag::util::encodings;
using namespace std;

const uint8_t	STK_PROFILE_ID = 0;

static time_t parseDateTime(const char* str)
{
  int year, month, day, hour, minute, second;
  if (!str || str[0] == '\0' ||
      sscanf(str, "%02d.%02d.%4d %02d:%02d:%02d", 
             &day, &month, &year, &hour, &minute, &second) != 6) return -1;
    
  tm  dt; dt.tm_isdst = -1;
  dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
  dt.tm_hour = hour; dt.tm_min = minute; dt.tm_sec = second;

  return mktime(&dt);
}
static time_t parseDate(const char* str)
{
  int year, month, day;
  if (!str || str[0] == '\0' ||
      sscanf(str, "%02d.%02d.%4d", 
             &day, &month, &year) != 3) return -1;
    
  tm  dt; dt.tm_isdst = -1;
  dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
  dt.tm_hour = 0; dt.tm_min = 0; dt.tm_sec = 0;
    
  return mktime(&dt);
}
static int parseTime(const char* str)
{
  int hour, minute, second;
  if (!str || str[0] == '\0' ||
      sscanf(str, "%02d:%02d:%02d", 
             &hour, &minute, &second) != 3) return -1;
    
  return hour*3600+minute*60+second;
}

static void checkAddress(const char* address)
{
  static const char* ADDRESS_ERROR_NULL_MESSAGE    = "Address is undefined.";
  static const char* ADDRESS_ERROR_INVALID_MESSAGE = "Address '%s' is invalid";
    
  if (!address || address[0] == '\0')
    throw Exception(ADDRESS_ERROR_NULL_MESSAGE);
  if (!isMSISDNAddress(address))
    throw Exception(ADDRESS_ERROR_INVALID_MESSAGE, address);
}

inline static bool checkEventMask(uint8_t userMask, uint8_t eventCause)
{
  return ((userMask & eventCause) == eventCause);
}

TaskProcessor::TaskProcessor(ConfigView* config)
  : Thread(), MissedCallListener(), AdminInterface(), 
    logger(Logger::getInstance("mci.TaskProc")), 
    profileStorage(ProfilesStorage::GetInstance()),
    protocolId(0), daysValid(1), advertising(0), useAdvert(false),
    templateManager(0), mciModule(0), messageSender(0),
    statistics(0), maxInQueueSize(10000), maxOutQueueSize(10000),
    bStarted(false), bInQueueOpen(false), bOutQueueOpen(false), bStopProcessing(false), pStorage(0), pDeliveryQueue(0)
{
  smsc_log_info(logger, "Loading ...");
	
  address = config->getString("Address");
  if ((address.length()==0) || !isMSISDNAddress(address.c_str()))
    throw ConfigException("Address string '%s' is invalid", (address.length()!=0) ? address.c_str():"-");

  try { protocolId = config->getInt("ProtocolId"); }
  catch(ConfigException& exc) { protocolId = 0; };
  try { svcType = config->getString("SvcType"); }
  catch(ConfigException& exc) { svcType = ""; };
  try { daysValid = config->getInt("DaysValid"); }
  catch(ConfigException& exc) { daysValid = 1; };
  try { _groupSmsByCallingAbonent = config->getBool("GroupSmsByCallingAbonent"); }
  catch(ConfigException& exc) { _groupSmsByCallingAbonent = false; }

  std::string callingMask = config->getString("CallingMask");
  std::string calledMask = config->getString("CalledMask");
    
  smsc_log_info(logger, "Loading templates ...");
  std::auto_ptr<ConfigView> templatesCfgGuard(config->getSubConfig("Templates"));
  ConfigView* templatesCfg = templatesCfgGuard.get();
  templateManager = new TemplateManager(templatesCfg);
  smsc_log_info(logger, "Templates loaded.");

  Hash<Circuits> circuitsMap; // loadup all MSC(s) circuits
  std::auto_ptr<ConfigView> circuitsCfgGuard(config->getSubConfig("Circuits"));
  ConfigView* circuitsCfg = circuitsCfgGuard.get();
  std::auto_ptr< std::set<std::string> > circuitsSetGuard(circuitsCfg->getShortSectionNames());
  std::set<std::string>* circuitsSet = circuitsSetGuard.get();
  for (std::set<std::string>::iterator i=circuitsSet->begin();i!=circuitsSet->end();i++)
  {
    const char* circuitsMsc = (const char *)i->c_str();
    if (!circuitsMsc || circuitsMap.Exists(circuitsMsc)) continue;

    std::auto_ptr<ConfigView> mscCfgGuard(circuitsCfg->getSubConfig(circuitsMsc));
    ConfigView* mscCfg = mscCfgGuard.get();

    Circuits circuits;
    circuits.hsn       = mscCfg->getInt   ("hsn");
    circuits.spn       = mscCfg->getInt   ("spn");
    const char* tsmStr = mscCfg->getString("tsm");
    uint32_t tsmLong = 0;
    if (!tsmStr || !tsmStr[0] || sscanf(tsmStr, "%x", &tsmLong) != 1)
      throw ConfigException("Parameter <MCISme.Circuits.%s.tsm> value is empty or invalid."
                            " Expecting hex string, found '%s'.", 
                            circuitsMsc, tsmStr ? tsmStr:"null");
    circuits.ts = tsmLong;
    circuitsMap.Insert(circuitsMsc, circuits);
  }
  if (circuitsMap.GetCount() <= 0) 
    throw ConfigException("No one valid MSC section <MCISme.Circuits.XXX> defined.");
    
  std::vector<Rule> rules; // loadup all redirection rules
  std::auto_ptr<ConfigView> rulesCfgGuard(config->getSubConfig("Rules"));
  ConfigView* rulesCfg = rulesCfgGuard.get();
  if (rulesCfg) 
  {
    std::auto_ptr< std::set<std::string> > rulesSetGuard(rulesCfg->getShortSectionNames());
    std::set<std::string>* rulesSet = rulesSetGuard.get();
    for (std::set<std::string>::iterator i=rulesSet->begin();i!=rulesSet->end();i++)
    {
      const char* ruleName = (const char *)i->c_str();
      if (!ruleName || !ruleName[0]) continue;

      std::auto_ptr<ConfigView> ruleCfgGuard(rulesCfg->getSubConfig(ruleName));
      ConfigView* ruleCfg = ruleCfgGuard.get();

      Rule rule;
      rule.name     = ruleName;
      rule.rx       = ruleCfg->getString("regexp");
      rule.priority = ruleCfg->getInt("priority");
      rule.inform   = ruleCfg->getInt("inform");
      rule.cause    = ruleCfg->getInt("cause");
      rules.push_back(rule);
    }
    std::sort(rules.begin(), rules.end());
  }

  ReleaseSettings releaseSettings;
  std::auto_ptr<ConfigView> releaseSettingsCfgGuard(config->getSubConfig("Reasons"));
  ConfigView* releaseSettingsCfg = releaseSettingsCfgGuard.get();
  try { releaseSettings.detachCause   = releaseSettingsCfg->getInt ("Detach.cause"); } catch (...) {
    smsc_log_warn(logger, "Parameter <MCISme.Reasons.Detach.cause> missed. Using default (20)");
    releaseSettings.detachCause = 20;
  }
  try { releaseSettings.detachInform  = releaseSettingsCfg->getBool("Detach.inform") ? 1:0; } catch (...) {
    smsc_log_warn(logger, "Parameter <MCISme.Reasons.Detach.inform> missed. Using default (0)");
    releaseSettings.detachInform = 0;
  }
  try { releaseSettings.skipUnknownCaller = releaseSettingsCfg->getBool("skipUnknownCaller"); } catch (...) {
    smsc_log_warn(logger, "Parameter <MCISme.Reasons.skipUnknownCaller> missed. Skip is switched off.");
    releaseSettings.skipUnknownCaller = false;
  }
  try { releaseSettings.strategy = releaseSettingsCfg->getInt ("strategy"); } catch (...) {
    smsc_log_warn(logger, "Parameter <MCISme.Reasons.strategy> missed. Using default redirect strategy (MTS defualt)");
    releaseSettings.strategy = REDIRECT_STRATEGY;
  }
  if (releaseSettings.strategy != PREFIXED_STRATEGY && releaseSettings.strategy != REDIREC_RULE_STRATEGY &&
      releaseSettings.strategy != REDIRECT_STRATEGY && releaseSettings.strategy != MIXED_STRATEGY) 
    throw ConfigException("Parameter <MCISme.Reasons.strategy> value '%d' is invalid.", 
                          releaseSettings.strategy);
  releaseCallsStrategy = releaseSettings.strategy;
  releaseSettings.busyCause           = releaseSettingsCfg->getInt ("Busy.cause");
  releaseSettings.busyInform          = releaseSettingsCfg->getBool("Busy.inform") ? 1:0;
  releaseSettings.noReplyCause        = releaseSettingsCfg->getInt ("NoReply.cause");
  releaseSettings.noReplyInform       = releaseSettingsCfg->getBool("NoReply.inform") ? 1:0;
  releaseSettings.unconditionalCause  = releaseSettingsCfg->getInt ("Unconditional.cause");
  releaseSettings.unconditionalInform = releaseSettingsCfg->getBool("Unconditional.inform") ? 1:0;
  releaseSettings.absentCause         = releaseSettingsCfg->getInt ("Absent.cause");
  releaseSettings.absentInform        = releaseSettingsCfg->getBool("Absent.inform") ? 1:0;
  releaseSettings.otherCause          = releaseSettingsCfg->getInt ("Other.cause");
  releaseSettings.otherInform         = releaseSettingsCfg->getBool("Other.inform") ? 1:0;
    
  const char* redirectionAddress = (releaseSettings.strategy == MIXED_STRATEGY) ? 
    config->getString("redirectionAddress") : 0;
  if (redirectionAddress && !isMSISDNAddress(redirectionAddress))
    throw ConfigException("Redirection address string '%s' is invalid", 
                          redirectionAddress ? redirectionAddress:"-");
    
  string countryCode;
  try { countryCode = config->getString("CountryCode"); } catch (...){countryCode = "7";
    smsc_log_warn(logger, "Parameter <MCISme.CountryCode> missed. Default value is '7'(Russia).");}

  smsc::misscall::MissedCallProcessor::instance_type_t instance_type;

  bool hasStackEmulator = false;
  try {
    hasStackEmulator = config->getBool("HasStackEmulator");
    hasStackEmulator = true;
  } catch(ConfigException& exc) {}

  if ( hasStackEmulator) {
    smsc::misscall::MissedCallProcessor::setInstanceType(smsc::misscall::MissedCallProcessor::CALL_PROCESOR_EMULATOR);
    std::string stackEmulatorHost("127.0.0.1");
    try {
      stackEmulatorHost = config->getString("StackEmulatorHost");
    } catch(ConfigException& exc) {}

    int stackEmulatorPort=37000;
    try {
      stackEmulatorPort = config->getInt("StackEmulatorPort");
    } catch(ConfigException& exc) {}
    smsc::misscall::MissedCallProcessorEmulator::setHost(stackEmulatorHost);
    smsc::misscall::MissedCallProcessorEmulator::setPort(stackEmulatorPort);
  } else
    smsc::misscall::MissedCallProcessor::setInstanceType(smsc::misscall::MissedCallProcessor::REAL_CALL_PROCESOR);

  mciModule = new MCIModule(circuitsMap, rules, releaseSettings, redirectionAddress,
                            callingMask.c_str(), calledMask.c_str(), countryCode.c_str());

  smsc_log_info(logger, "MCI Module starting...");
  mciModule->Start();
  smsc_log_info(logger, "MCI Module started.");
  //    responcesTracker.init(this, config);

  maxInQueueSize  = config->getInt("inputQueueSize");
  maxOutQueueSize = config->getInt("outputQueueSize");

  try { forceInform = config->getBool("forceInform"); } catch (...){ forceInform = true;
    smsc_log_warn(logger, "Parameter <MCISme.forceInform> missed. Default value is 'true'.");}
  try { forceNotify = config->getBool("forceNotify"); } catch (...){ forceInform = true;
    smsc_log_warn(logger, "Parameter <MCISme.forceNotify> missed. Default value is 'true'.");}

  try { _isReverseNotifyPolicy = config->getBool("reverseNotifyPolicy"); }
  catch (...){
    _isReverseNotifyPolicy = false;
    smsc_log_warn(logger, "Parameter <MCISme.reverseNotifyPolicy> missed. Default value is 'true'.");
  }

  bool bDefaultInform, bDefaultNotify;
  try { bDefaultInform = config->getBool("defaultInform"); } catch (...) { bDefaultInform = true;
    smsc_log_warn(logger, "Parameter <MCISme.defaultInform> missed. Defaul profile inform flag is on");
  }
  try { bDefaultNotify = config->getBool("defaultNotify"); } catch (...) { bDefaultNotify = false;
    smsc_log_warn(logger, "Parameter <MCISme.defaultNotify> missed. Defaul profile notify flag is off");
  }
  int defaultReasonsMask = smsc::misscall::NONE;
  try { defaultReasonsMask = config->getInt("defaultReasonsMask"); } catch (...) { 
    defaultReasonsMask = smsc::misscall::ALL; // 0xFF;
    smsc_log_warn(logger, "Parameter <MCISme.defaultReasonsMask> missed. All masks on by default");
  }

  int rowsPerMessage = 5;
  try { rowsPerMessage = config->getInt("maxRowsPerMessage"); } catch (...) { rowsPerMessage = 5;
    smsc_log_warn(logger, "Parameter <MCISme.maxRowsPerMessage> missed. Using default %d", rowsPerMessage);
  }
  int maxCallersCount = -1;
  try { maxCallersCount = config->getInt("maxCallersCount"); } catch (...) { maxCallersCount = -1;
    smsc_log_warn(logger, "Parameter <MCISme.maxCallersCount> missed. Callers check disabled");
  }
  int maxMessagesCount = -1;
  try { maxMessagesCount = config->getInt("maxMessagesCount"); } catch (...) { maxMessagesCount = -1;
    smsc_log_warn(logger, "Parameter <MCISme.maxMessagesCount> missed. Messages check disabled");
  }
  if (maxCallersCount > 0 && maxMessagesCount > 0)
    throw ConfigException("MCISme supports only one constraint either by messages or callers. "
                          "Use <MCISme.maxCallersCount> or <MCISme.maxMessagesCount> parameter.");

  string timeZoneFileLocation;
  try { timeZoneFileLocation = config->getString("timeZoneFileLocation"); } catch (...)
  {
    throw ConfigException("MCISme need timeZoneFileLocation parameter."
                          "Use <MCISme.timeZoneFileLocation> parameter in config.xml.");
  }
  string routesFileLocation;
  try { routesFileLocation = config->getString("routesFileLocation"); } catch (...)
  {
    throw ConfigException("MCISme need routesFileLocation parameter."
                          "Use <MCISme.routesFileLocation> parameter in config.xml.");
  }
  try{smsc::system::common::TimeZoneManager::Init(timeZoneFileLocation.c_str(), routesFileLocation.c_str());}catch(Exception e)
  {
    smsc_log_warn(logger, "TimeZoneManager::Init() error. %s", e.what());
    throw ConfigException(e.what());
  }
	
  time_t t = time(NULL); localtime(&t);	// ����� ������ localtime() ������ ��������������������� timezone;

  try { test_number = config->getString("testNumber"); } catch (...)
  {
    test_number="";
  }

  //	Init Statistics
  std::auto_ptr<ConfigView> statCfgGuard(config->getSubConfig("Statistics"));
  ConfigView* statCfg = statCfgGuard.get();

  string statDir;
  try { statDir = statCfg->getString("statDir"); } catch (...){statDir = "stat";
    smsc_log_warn(logger, "Parameter <MCISme.Statistics.location> missed. Default value is 'stat'.");}
	
  statistics = new StatisticsManager(statDir.c_str());
  if (statistics) statistics->Start();
    
  AbonentProfiler::init(0, defaultReasonsMask, bDefaultInform, bDefaultNotify);

  //	Init Delivery Queue
  std::auto_ptr<ConfigView> schedulingCfgGuard(config->getSubConfig("Scheduling"));
  ConfigView* schedulingCfg = schedulingCfgGuard.get();

  string sResendingPeriod;
  try { sResendingPeriod = schedulingCfg->getString("resendingPeriod"); } catch (...){sResendingPeriod = "00:30:00";
    smsc_log_warn(logger, "Parameter <MCISme.ResendingPeriod> missed. Default value is '00:30:00'.");}
  time_t resendingPeriod = parseTime(sResendingPeriod.c_str());

  string sSchedOnBusy;
  try { sSchedOnBusy = schedulingCfg->getString("schedOnBusy"); } catch (...){sSchedOnBusy = "00:05:00";
    smsc_log_warn(logger, "Parameter <MCISme.ResendingPeriod> missed. Default value is '00:05:00'.");}
  time_t schedOnBusy = parseTime(sSchedOnBusy.c_str());

  pDeliveryQueue = new DeliveryQueue(resendingPeriod, schedOnBusy);

  std::auto_ptr<ConfigView> schedCfgGuard(schedulingCfg->getSubConfig("schedule"));
  ConfigView* schedCfg = schedCfgGuard.get();

  std::auto_ptr< std::set<std::string> > schedSetGuard(schedCfg->getStrParamNames());
  std::set<std::string>*	schedSet = schedSetGuard.get();
	
  if(!schedSet->empty())
  {
    for (std::set<std::string>::iterator i=schedSet->begin();i!=schedSet->end();i++)
    {
      string value;
      time_t t;
      int err;
      try { value = schedCfg->getString((*i).c_str()); } catch (...){
        smsc_log_warn(logger, "Parameter <MCISme.%s> missed invalid .", (*i).c_str());}
      t = parseTime(value.c_str());
      if((*i) == "default")
        err = -1;
      else
        err = atoi((*i).c_str());
      pDeliveryQueue->AddScheduleRow(err, t);
      smsc_log_info(logger, "Add row to Schedule Table: %s(%d) = %s (%d)", (*i).c_str(), err, value.c_str(), t);
      //			printf("%s(%d) = %s (%d)\n", (*i).c_str(), err, value.c_str(), t);
    }
  }
  else
    smsc_log_warn(logger, "Schedule Table not specified.");


  //	Init Advertising Client
  std::auto_ptr<ConfigView> advertCfgGuard(config->getSubConfig("Advertising"));
  ConfigView* advertCfg = advertCfgGuard.get();

  try { useAdvert = advertCfg->getBool("useAdvert"); } catch (...) { useAdvert = false;
    smsc_log_warn(logger, "Parameter <MCISme.Advertising.useAdvert> missed. Defaul profile useAdvert is false (off)");
  }

  if(useAdvert)
  {
    string advertServer;
    try { advertServer = advertCfg->getString("server"); } catch (...){advertServer = "0.0.0.0";
      smsc_log_warn(logger, "Parameter <MCISme.Advertising.server> missed. Default value is '0.0.0.0'.");}
    int advertPort;
    try { advertPort = advertCfg->getInt("port"); } catch (...){advertPort = 25000;
      smsc_log_warn(logger, "Parameter <MCISme.Advertising.port> missed. Default value is '25000'.");}
    int advertTimeout;
    try { advertTimeout = advertCfg->getInt("timeout"); } catch (...){advertTimeout = 15;
      smsc_log_warn(logger, "Parameter <MCISme.Advertising.server> missed. Default value is '15'.");}
		
    Advertising::Init(advertServer, advertPort, advertTimeout*1000);
    advertising = &(scag::advert::Advertising::Instance());
  }

  //	Init Storage
  std::auto_ptr<ConfigView> storageCfgGuard(config->getSubConfig("Storage"));
  ConfigView* storageCfg = storageCfgGuard.get();
  pStorage = new FSStorage();
  int ret = pStorage->Init(storageCfg, pDeliveryQueue);
  smsc_log_warn(logger, "ret = %d", ret);

  //	Init Timeout Monitor
  string sResponseWaitTime;
  try { sResponseWaitTime = config->getString("responceWaitTime"); } catch (...){sResponseWaitTime = "00:00:60";
    smsc_log_warn(logger, "Parameter <MCISme.responceWaitTime> missed. Default value is '00:00:30'.");}
  responseWaitTime = parseTime(sResponseWaitTime.c_str());

  //	timeoutMonitor = new TimeoutMonitor(this, responseWaitTime);
  timeoutMonitor = new TimeoutMonitor(this);
  pDeliveryQueue->SetResponseWaitTime(responseWaitTime);
	
  smsc_log_info(logger, "Load success.");
}
TaskProcessor::~TaskProcessor()
{
  this->Stop();
  if (templateManager) delete templateManager;
  if (mciModule) delete mciModule;
  if (statistics) delete statistics;
  if (pStorage) delete pStorage;
  if (pDeliveryQueue) delete pDeliveryQueue;
  if (timeoutMonitor) delete timeoutMonitor;
  smsc::system::common::TimeZoneManager::Shutdown();
}

void TaskProcessor::Start()
{
  MutexGuard guard(startLock);
  if (!bStarted)
  {
    smsc_log_info(logger, "Starting event processing...");

    if (!isMessageSenderAssigned()) {
      smsc_log_error(logger, "Failed to start processing. Message sender is undefined.");
      return;
    }
    timeoutMonitor->Start();
    pDeliveryQueue->OpenQueue();
    openInQueue();
    Thread::Start();
    if (mciModule) mciModule->Attach(this);
        
    bStarted = true;
    smsc_log_info(logger, "Event processing started.");
  }
  bStopProcessing = false;
}
void TaskProcessor::Stop()
{
  MutexGuard  guard(startLock);
  if (bStarted)
  {
    smsc_log_info(logger, "Stopping event processing...");
        
    if (mciModule) mciModule->Detach();
    timeoutMonitor->Stop();
    closeInQueue();
    pDeliveryQueue->CloseQueue();
    bStopProcessing = true;
    exitedEvent.Wait();

    bStarted = false;
    smsc_log_info(logger, "Event processing stoped.");
  }
}
void TaskProcessor::Pause()
{
  MutexGuard  guard(startLock);
  bStopProcessing = true;
}

void TaskProcessor::Run()
{
  static const char* ERROR_MESSAGE = "Failed to process message. Details: %s";
    
  smsc_log_info(logger, "Message processing loop started.");
  time_t	delivery_time;
  time_t	cur_time;
  int		wait_smsc_msg;

  //	while(pDeliveryQueue->isQueueOpened()) sleep(1000); return;
  //	while(bOutQueueOpen)
  //	while(pDeliveryQueue->isQueueOpened())
  while(!bStopProcessing)
  {
    if (mciModule && !mciModule->isRunning()) {
      smsc_log_info(logger, "MCI Module is down. Exiting message processing loop");
      break;
    }
    try
    {
      AbntAddr	abnt;
      if(pDeliveryQueue->Get(abnt))
        ProcessAbntEvents(abnt);
    }
    catch (std::exception& exc){
      smsc_log_error(logger, ERROR_MESSAGE, exc.what());}
    catch (...){
      smsc_log_error(logger, ERROR_MESSAGE, "Cause is unknown");}
  }
	
  smsc_log_info(logger, "Message processing loop exited.");
}

int TaskProcessor::Execute()
{
  clearSignalMask();
    
  MissedCallEvent event;
  MCEvent			outEvent;
	
  smsc_log_debug(logger, "Execute");

  time_t start = time(0);
  time_t end = time(0);
  uint32_t count=0;
	
  //test_sched();
  //	test_advert();
  //	while(1) sleep(1000);return 0;
	
  while (bInQueueOpen)
  {
    try
    {
      if (!getFromInQueue(event)) break;

      //			count++;
      //			end = time(0);
      //			if( (end-start) > 0)
      //			{
      //				MutexGuard guard(inQueueMonitor);
      //				uint32_t size = inQueue.Count();
      //				smsc_log_debug(logger, "speed: Current incoming speed is %d messages per second (current inQueueSize = %d)", count, size);
      //				start = time(0);
      //				count=0;
      //			}

      try{checkAddress(event.to.c_str());}catch(Exception e)
      {
        smsc_log_error(logger, "Bad called number. %s", e.what());
        continue;
      }
      //			if(test_number == event.from) event.from = "";
      try{checkAddress(event.from.c_str());}catch(Exception e)
      {
        smsc_log_error(logger, "Bad calling number. %s", e.what());
        //        continue;
      }
      AbntAddr to(event.to.c_str()), from(event.from.c_str());
      outEvent = from;
      AbonentProfile profile;
      profileStorage->Get(to, profile);
      //			if (releaseCallsStrategy != MIXED_STRATEGY || event.cause != ABSENT)

      if(!forceInform && !profile.inform)
        smsc_log_debug(logger, "Event %s->%s skipped because inform disabled for abonent %s and forceInform is false.", event.from.c_str(), event.to.c_str(), to.toString().c_str());
      else if (checkEventMask(profile.eventMask, event.cause))
      {
        time_t schedTime = pDeliveryQueue->Schedule(to, ((event.cause&0x02)==0x02)); //0x02 - BUSY
        pStorage->addEvent(to, outEvent, schedTime);
        statistics->incMissed();
        smsc_log_debug(logger, "Abonent %s (couse = 0x%02X) was added to Scheduled Delivery Queue", to.toString().c_str(), event.cause);
      }
      else
        smsc_log_debug(logger, "Event: for abonent %s skipped (userMask=%02X, eventCause=%02X)", to.getText().c_str(), profile.eventMask, event.cause);
    }
    catch(...)
    {
      smsc_log_error(logger, "Exception in TaskProcessor::Execute()");
    }
  }
  exitedEvent.Signal();
  return 0;
}

void TaskProcessor::test_stk(void)
{
  Message	msg;
	
  msg.message="79139167634+3+10:45+20/07/06";
  msg.secured_data = true;
  smsc_log_debug(logger, "ProcessAbntEvents: msg = %s", msg.message.c_str());
  //	msg.abonent = "+79163526458";
  msg.abonent = "+79163526460";
  //	msg.abonent = "+79139167634";
	
  int seqNum = messageSender->getSequenceNumber();
  if(!messageSender->send(seqNum, msg))
  {
    smsc_log_debug(logger, "Send DATA_SM for Abonent  failed");
    return;
  }
}

void TaskProcessor::test_sched(void)
{
  AbntAddr abnt1("1111111111");
  AbntAddr abnt2("2222222222");
  AbntAddr abnt3("3333333333");
  AbntAddr abnt4("4444444444");
  AbntAddr abnt5("5555555555");

  pDeliveryQueue->Schedule(abnt1, false);
  sleep(1);
  pDeliveryQueue->Schedule(abnt2, false); 
  sleep(1);
  pDeliveryQueue->Schedule(abnt3, false);
  sleep(1);
  pDeliveryQueue->Schedule(abnt4, false); 
  sleep(1);
  pDeliveryQueue->Schedule(abnt5, false); 
  sleep(5);
  pDeliveryQueue->Reschedule(abnt1, 1179);
  sleep(5);
  pDeliveryQueue->Reschedule(abnt1, 1184);
  sleep(5);
  pDeliveryQueue->Reschedule(abnt1, 1179);
}

void TaskProcessor::test_advert(void)
{
  string banner;
  string text;
  int rc;

  //iconv_t cd = iconv_open("UTF-8", "UTF-16BE");
  //   if( cd == (iconv_t)(-1) )
  //{
  //       smsc_log_debug(logger, "Convertor: iconv_open. Cannot convert. errno=%d", errno);
  //	return;
  //}
  //   iconv(cd, NULL, NULL, NULL, NULL);

  rc = advertising->getBanner(".1.1.78432909209", "mcisme", 1, 1, banner);
  if(rc == 0)
  {
    smsc_log_debug(logger, "rc = %d;", rc);
    smsc_log_debug(logger, "Banner: %s", banner.c_str());
    smsc_log_debug(logger, "len=%d", banner.length());
    //	    if(iconv(cd, &in, &inLen, &outbufptr, &outbytesleft) == (size_t)(-1) && errno != E2BIG)
    //		    throw SCAGException("Convertor: iconv. Cannot convert from '%s' to '%s'. errno=%d. bytesleft=%d", inCharset, outCharset, errno, inLen);

    try{Convertor::convert("UTF-16BE", "UTF-8", banner.c_str(), banner.length(), text);}catch(SCAGException e){
      smsc_log_debug(logger, "Exc: %s" );
    }
    smsc_log_debug(logger, "!!!!!!!!!!!!!!");
    smsc_log_debug(logger, "rc = %d; Banner: %s", rc, text.c_str());
  }
}


void TaskProcessor::ProcessAbntEvents(const AbntAddr& abnt)
{
  static time_t start = time(0);
  time_t end;
  static uint32_t count=0;

  vector<MCEvent>		events;

  if(!GetAbntEvents(abnt, events))
  {
    smsc_log_info(logger, "No more events for Abonent %s ", abnt.toString().c_str());
    pDeliveryQueue->Remove(abnt);
    return;
  }
  smsc_log_debug(logger, "Attempt to send SMS to Abonent %s ", abnt.toString().c_str());
  //	AbonentProfile		profile(AbonentProfiler::getProfile(abnt.getText().c_str()));

  AbonentProfile profile;
  profileStorage->Get(abnt, profile);

  if(!forceInform && !profile.inform)
  {
    smsc_log_debug(logger, "Inform is off for Abonent %s - cancel sending", abnt.toString().c_str());
    pStorage->deleteEvents(abnt, events);
    return;
  }

  MessageFormatter	formatter(templateManager->getInformFormatter(profile.informTemplateId));

  int			seqNum=0;

  int timeOffset = smsc::system::common::TimeZoneManager::getInstance().getTimeZone(abnt.getAddress())+timezone;

  if ( isGroupSmsByCallingAbonent() ) {
    std::vector <MCEventOut> mcEventsOut;
    formatter.formatMessage(abnt, events, 0, mcEventsOut, address, timeOffset);

    for (std::vector <MCEventOut>::iterator iter = mcEventsOut.begin(), end_iter = mcEventsOut.end(); iter !=end_iter; ++iter) {
      Message  msg;

      if(profile.informTemplateId == STK_PROFILE_ID)
        msg.secured_data = true;
      else
        msg.data_sm = true;

      msg.abonent = abnt.getText();
      msg.message = iter->msg;
      msg.caller_abonent = iter->caller;

      smsc_log_debug(logger, "ProcessAbntEvents: prepare message = '%s' for sending to %s from %s", msg.message.c_str(), msg.abonent.c_str(), msg.caller_abonent.c_str());

      if(useAdvert) formatter.addBanner(msg, getBanner(abnt));
      {
        MutexGuard Lock(smsInfoMutex);
        seqNum = messageSender->getSequenceNumber();
        sms_info*		pInfo = new sms_info;

        pInfo->sending_time = time(0);
        pInfo->abnt = abnt;
        pInfo->events = iter->srcEvents;

        int res = smsInfo.Insert(seqNum, pInfo);
        if(!messageSender->send(seqNum, msg))
        {
          smsc_log_debug(logger, "Send DATA_SM for Abonent %s failed", pInfo->abnt.toString().c_str());
          pDeliveryQueue->Reschedule(pInfo->abnt);
          smsInfo.Delete(seqNum);
          delete pInfo;
          return;
        }
      }
    }
  } else {
    Message  msg;
    sms_info*	 pInfo = new sms_info;
    pInfo->abnt = abnt;
        
    if(profile.informTemplateId == STK_PROFILE_ID)
      msg.secured_data = true;
    else
      msg.data_sm = true;

    formatter.formatMessage(msg, abnt, events, 0, pInfo->events, timeOffset);
    smsc_log_debug(logger, "ProcessAbntEvents: msg = %s", msg.message.c_str());
    msg.abonent = abnt.getText();//"777";
    if(useAdvert) formatter.addBanner(msg, getBanner(abnt));
    {
      MutexGuard Lock(smsInfoMutex);
      seqNum = messageSender->getSequenceNumber();
      pInfo->sending_time = time(0);
      int res = smsInfo.Insert(seqNum, pInfo);
      if(!messageSender->send(seqNum, msg))
      {
        smsc_log_debug(logger, "Send DATA_SM for Abonent %s failed", pInfo->abnt.toString().c_str());
        pDeliveryQueue->Reschedule(pInfo->abnt);
        smsInfo.Delete(seqNum);
        delete pInfo;
        return;
      }
//		timeoutMonitor->addSeqNum(seqNum);
    }
  }
}

bool TaskProcessor::invokeProcessDataSmResp(int cmdId, int status, int seqNum)
{
  if(cmdId == smsc::smpp::SmppCommandSet::DATA_SM_RESP)
  {
    //		printf("Recieve a DATA_SM_RESP (cmdId = %d status = %d seqNum = %d )\n", cmdId, status, seqNum);
    //		if((seqNum%5))	return false;

    sms_info* pInfo=0;
    {
      MutexGuard Lock(smsInfoMutex);
      if(!smsInfo.Exist(seqNum))
      {
        smsc_log_debug(logger, "No info for SMS seqNum = %d\n", seqNum);
        return false;
      }
      pInfo = smsInfo.Get(seqNum);
      smsInfo.Delete(seqNum);
      //			timeoutMonitor->removeSeqNum(seqNum);
    }

    smsc_log_debug(logger, "Recieve a DATA_SM_RESP for Abonent %s seq_num = %d, status = %d", pInfo->abnt.toString().c_str(), seqNum, status);

    if(status == smsc::system::Status::OK)
    {	
      SendAbntOnlineNotifications(pInfo);
      statistics->incDelivered(pInfo->events.size());
      pStorage->deleteEvents(pInfo->abnt, pInfo->events);
      time_t schedTime = pDeliveryQueue->Reschedule(pInfo->abnt, smsc::system::Status::OK);
      pStorage->setSchedParams(pInfo->abnt, schedTime, status);
    }
    else if(smsc::system::Status::isErrorPermanent(status))
    {
      statistics->incDelivered(pInfo->events.size());
      StopProcessEvent4Abnt(pInfo->abnt);
    }
    else
    {
      statistics->incFailed(pInfo->events.size());
      time_t schedTime = pDeliveryQueue->Reschedule(pInfo->abnt, status);
      pStorage->setSchedParams(pInfo->abnt, schedTime, status);
    }
    delete pInfo;
  }
  return true;
};

void TaskProcessor::invokeProcessDataSmTimeout(void)
{
  MutexGuard Lock(smsInfoMutex);
  int count_old = smsInfo.Count();
  smsc_log_debug(logger, "Start serching unresponded DATA_SM. Total SMS in Hash is %d", smsInfo.Count());
  smsc::core::buffers::IntHash<sms_info*>::Iterator It(smsInfo.First());
	
  //	It = smsInfo.First();
  sms_info* pInfo=0;
  int seqNum=0;
  int count = 0;
  time_t curTime = time(0);
  while(It.Next(seqNum, pInfo))
  {
    //		smsc_log_debug(logger, "SMS for Abonent %s (seqNum %d) was sent %d sec ago", pInfo->abnt.toString().c_str(), seqNum, curTime - pInfo->sending_time);
    if(curTime > (pInfo->sending_time + responseWaitTime))
    {
      smsc_log_info(logger, "SMS for Abonent %s (seqNum %d) is removed from waiting list by timeout", pInfo->abnt.toString().c_str(), seqNum);

      statistics->incFailed(pInfo->events.size());
      time_t schedTime = pDeliveryQueue->Reschedule(pInfo->abnt);
      pStorage->setSchedParams(pInfo->abnt, schedTime, -1);
      smsInfo.Delete(seqNum);
      delete pInfo;
    }
    count++;
  }
  int count_new = smsInfo.Count();
  smsc_log_debug(logger, "Complete serching unresponded DATA_SM. Total SMS in Hash is %d, removed %d (%d)", count_new, count_old - count_new, count);
}

//void TaskProcessor::invokeProcessDataSmTimeout(int seqNum)
//{
//	smsc_log_debug(logger, "Timeout for SMS seqNum %d", seqNum);
//
//	sms_info* pInfo=0;
//	{
//		MutexGuard Lock(smsInfoMutex);
//		if(!smsInfo.Exist(seqNum))
//		{
//			smsc_log_debug(logger, "No info for SMS seqNum = %d\n", seqNum);
//			return;
//		}
//		pInfo = smsInfo.Get(seqNum);
//		smsInfo.Delete(seqNum);
//	}
//
//	smsc_log_debug(logger, "SMS for Abonent %s (seqNum %d) is removed from waiting list by timeout", pInfo->abnt.toString().c_str(), seqNum);
//
//	statistics->incFailed(pInfo->events.size());
//	time_t schedTime = pDeliveryQueue->Reschedule(pInfo->abnt);
//	pStorage->setSchedParams(pInfo->abnt, schedTime, -1);
//
//	delete pInfo;
//};


bool TaskProcessor::invokeProcessAlertNotification(int cmdId, int status, const AbntAddr& abnt)
{
  smsc_log_debug(logger, "Recieve an ALERT_NOTIFICATION for Abonent %s status = %d", abnt.toString().c_str(), status);
  if(cmdId == smsc::smpp::SmppCommandSet::ALERT_NOTIFICATION)
  {
    time_t schedTime = pDeliveryQueue->RegisterAlert(abnt);
    pStorage->setSchedParams(abnt, schedTime, status);
  }
  else
    return false;
  return true;
}

bool TaskProcessor::GetAbntEvents(const AbntAddr& abnt, vector<MCEvent>&  events)
{
  if(!pStorage)
    return 0;
	
  return pStorage->getEvents(abnt, events);
}

//
//
//����������� �������� ������� �� ���� ������: forceNotify(������������� � config.xml) � profile.notify(��� ����� ��������, ������� ��� ���������� � ��� ���������� �� ��������� ���).
//
//forceNotify  profile.notify
//true             true        -  ������� ����
//false            true        -  ������� ����
//true             false       -  ������� ����
//false            false       -  ������� ���

//� ����������� �������. ������ ��� ����� �������� ��� � ����������� �������, �� ������ ������ ���� ��� ����������� ������ ���������, ������� ���� ��������� � ���������� ���� ����������� �������������� �������. ��������������, ���� �������� ���� ����������� ��������� ��� � ����������� ������� � � ���� � ������� �� ���������� ���� ����������� ������� �� ������������ ���������, �� �������� ������� 


bool
TaskProcessor::noNeedNotify(const AbonentProfile& profile, const sms_info* pInfo) const
{
  if ( _isReverseNotifyPolicy ) {
    if ( profile.notify ) {
      smsc_log_debug(logger, "Notify prohibition is ON for Abonent %s", pInfo->abnt.toString().c_str());
      return true;
    }
  } else {
    if(!forceNotify && !profile.notify)
    {
      smsc_log_debug(logger, "Notify is off for Abonent %s", pInfo->abnt.toString().c_str());
      return true;
    }
  }
  return false;
}

void TaskProcessor::SendAbntOnlineNotifications(const sms_info* pInfo)
{
  //	AbonentProfile	profile(AbonentProfiler::getProfile(pInfo->abnt.getText().c_str()));
  AbonentProfile profile;
  profileStorage->Get(pInfo->abnt, profile);

  smsc_log_debug(logger, "Process Notify message");
  statistics->incNotified();

  if ( noNeedNotify(profile, pInfo) ) return;

  vector<AbntAddr>	callers;				// use Hash
  int events_count = pInfo->events.size();
  int i,j, callers_count=0;
  bool exists;

  for(i = 0; i < events_count; i++)
  {
    AbntAddr caller(&(pInfo->events[i].caller));
    exists = false;
    for(j = 0; j < callers_count; j++)
      if(caller == callers[j])
      { exists = true; break;}
    if(!exists)
    {
      callers.push_back(caller);
      callers_count++;
    }
  }

  NotifyTemplateFormatter* formatter = templateManager->getNotifyFormatter(profile.notifyTemplateId);
  OutputFormatter*  messageFormatter = formatter->getMessageFormatter();
  ContextEnvironment ctx;
  string	abnt = pInfo->abnt.getText().c_str();

  for(i = 0; i < callers_count; i++)
  {
    Message	msg;
    string ph = callers[i].getText().c_str();
    if(ph.length()==0) ph="unknown";

    NotifyGetAdapter	adapter(abnt, ph.c_str());					//"11111");//callers[i].getText().c_str());
    messageFormatter->format(msg.message, adapter, ctx);
    msg.abonent = callers[i].getText();//"444"

    try{checkAddress(msg.abonent.c_str());}catch(Exception e)
    {
      smsc_log_error(logger, "Skip notification message - bad address %s", e.what());
      continue;
    }
    smsc_log_debug(logger, "Notify message = %s", msg.message.c_str());
    msg.caller_abonent = abnt;
    messageSender->send(messageSender->getSequenceNumber(), msg);
  }
}

void TaskProcessor::StopProcessEvent4Abnt(const AbntAddr& abnt)
{
  vector<MCEvent>		events;

  smsc_log_debug(logger, "Recieved a permanent error for abonent %s - stopping processing events.", abnt.toString().c_str());
  if(!GetAbntEvents(abnt, events))
    pStorage->deleteEvents(abnt, events);
  pDeliveryQueue->Remove(abnt);
}

string TaskProcessor::getBanner(const AbntAddr& abnt)
{
  string banner, ret, ret1;
  int rc;
  if(!advertising) return banner;

  rc = advertising->getBanner(abnt.toString(), svcType.c_str(), scag::advert::SMPP_SMS, scag::advert::UTF16BE, ret);
  if(rc == 0)
  {
    try{Convertor::convert("UTF-16BE", "UTF-8", ret.c_str(), ret.length(), ret1);}catch(SCAGException e){
      smsc_log_debug(logger, "Exc: %s", e.what());
      return banner="";
    }
    try{Convertor::convert("UTF-8", "CP1251", ret1.c_str(), ret1.length(), banner);}catch(SCAGException e){
      smsc_log_debug(logger, "Exc: %s", e.what());
      return banner="";
    }
    smsc_log_debug(logger, "rc = %d; Banner: %s (%s)", rc, banner.c_str(), ret.c_str());
  }
  else
    smsc_log_debug(logger, "getBanner Error. Error code = %d", rc);

  return banner;
}

void TaskProcessor::openInQueue() {
  MutexGuard guard(inQueueMonitor);
  bInQueueOpen = true;
  inQueueMonitor.notifyAll();
}
void TaskProcessor::closeInQueue() {
  MutexGuard guard(inQueueMonitor);
  bInQueueOpen = false;
  inQueueMonitor.notifyAll();
}
bool TaskProcessor::putToInQueue(const MissedCallEvent& event, 
                                 bool skip/*=true. Skip event when queue is full*/)
{
  //   smsc_log_debug(logger, "putToQueue: event");

  MutexGuard guard(inQueueMonitor);
  while (!skip && bInQueueOpen && (inQueue.Count() >= maxInQueueSize)) {
    inQueueMonitor.wait();
  }
  if (!bInQueueOpen) return false;
  if (skip && inQueue.Count() >= maxInQueueSize) {
    //        smsc_log_warn(logger, "Input event queue is full (contains %d events, max %d). "
    //                      "Event for abonent '%s' was skipped.", 
    //                      inQueue.Count(), maxInQueueSize, event.to.c_str());
    return false;
  }
  //    smsc_log_debug(logger, "putToQueue: push");
  inQueue.Push(event);
  inQueueMonitor.notifyAll();
  return true;
}
bool TaskProcessor::getFromInQueue(MissedCallEvent& event)
{
  MutexGuard guard(inQueueMonitor);
  do {
    if (inQueue.Count() > 0) {
      inQueue.Pop(event);
      inQueueMonitor.notifyAll();
      return true;
    }
    inQueueMonitor.wait();
  } while (bInQueueOpen);
  smsc_log_info(logger, "Input queue closed");
  return false;
}

// Admin Interface
string TaskProcessor::getSchedItem(const string& Abonent)
{
  smsc_log_info(logger, "Received schedule query for abonent %s", Abonent.c_str());
  string result;

  try{checkAddress(Abonent.c_str());}catch(Exception e)
  {
    smsc_log_info(logger, "Schedule query contains bad address %s", e.what());
    return result;
  }
  AbntAddr			abnt(Abonent.c_str());
  SchedItem			item;
  vector<MCEvent>		events;

  if(!pDeliveryQueue->Get(abnt, item))
  {
    smsc_log_info(logger, "Abonent %s don't exists in Delivery Queue", Abonent.c_str());
    return result;
  }
	
  GetAbntEvents(abnt, events);

  char buff[128];
  if(item.schedTime != 0)
  {
    struct tm* t = localtime(&item.schedTime);
    snprintf(buff, 128, "%.2d.%.2d.%4d %.2d:%.2d:%.2d,%s,%d, %d;", t->tm_mday, t->tm_mon+1, t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec,
             Abonent.c_str(), events.size(), item.lastError);
  }
  else
    snprintf(buff, 128, "In delivery,%s,%d, %d;", Abonent.c_str(), events.size(), item.lastError);

  result = buff;
  return result;
}

string TaskProcessor::getSchedItems(void)
{
  smsc_log_info(logger, "Received schedule query for first 50 abonents ");
  string				result;
  vector<SchedItem>	items;

  if(0 == pDeliveryQueue->Get(items, 50))
  {
    smsc_log_info(logger, "Delivery Queue is empty");
    return result;
  }
	
  for(int i = 0; i < items.size(); i++)
  {
    //		vector<MCEvent>	events;
    char			buff[128];
    time_t			schedTime;
    struct tm*		t;

    //		GetAbntEvents(items[i].abnt, events);
    schedTime = items[i].schedTime;	t = localtime(&schedTime);
    //		snprintf(buff, 128, "%.2d.%.2d.%4d %.2d:%.2d:%.2d,%s,%d,%d;", t->tm_mday, t->tm_mon+1, t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec,
    //														items[i].abnt.getText().c_str(), events.size(), items[i].lastError);
    //		smsc_log_debug(logger, "%.2d.%.2d.%4d %.2d:%.2d:%.2d,%s,%d,%d;", t->tm_mday, t->tm_mon+1, t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec,
    //														items[i].abnt.getText().c_str(), events.size(), items[i].lastError);
    snprintf(buff, 128, "%.2d.%.2d.%4d %.2d:%.2d:%.2d,%d;", t->tm_mday, t->tm_mon+1, t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec,
             items[i].abonentsCount);

    result += buff;
  }
  return result;
}

}
}
