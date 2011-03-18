#include <exception>
#include <algorithm>
#include <iconv.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>

#include "sme/SmppBase.hpp"
#include "system/status.h"
#include "sms/sms.h"
#include "system/common/TimeZoneMan.hpp"
#include "scag/util/encodings/Encodings.h"

#include "util/smstext.h"

#include "TaskProcessor.h"
#include "misscall/callproc.hpp"
#include "Exceptions.hpp"
#include "FSStorage.hpp"
#include "AbntAddr.hpp"
#include "Profiler.h"
#include "ProfilesStorage.hpp"
#include "Templates.h"
#include "MCAEventsStorage.hpp"
#include "OutputMessageProcessor.hpp"
#include "BannerOutputMessageProcessorsDispatcher.hpp"
#include "BannerlessOutputMessageProcessorsDispatcher.hpp"

extern bool isMSISDNAddress(const char* string);
extern "C" void clearSignalMask(void);

namespace smsc {
namespace mcisme {

extern char* cTime(const time_t* clock, char* buff, size_t bufSz);

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
    protocolId(0), daysValid(1), advertising(0),
    templateManager(0), mciModule(0), messageSender(0),
    statistics(0), maxInQueueSize(10000), // maxOutQueueSize(10000),
    bStarted(false), bInQueueOpen(false), bOutQueueOpen(false), bStopProcessing(false),
    pStorage(0), pDeliveryQueue(0), _iasmeProxy(NULL)
{
  smsc_log_info(logger, "Loading ...");

  address = config->getString("Address");
  if ((address.length()==0) || !isMSISDNAddress(address.c_str()))
    throw ConfigException("Address string '%s' is invalid", (address.length()!=0) ? address.c_str():"-");

  try {
    protocolId = config->getInt("ProtocolId");
  } catch(ConfigException& exc) {
    protocolId = 0;
  }
  try {
    svcType = config->getString("SvcType");
  } catch(ConfigException& exc) {
    svcType = "";
  }
  try {
    svcTypeOnLine = config->getString("SvcTypeOnLine");
  } catch(ConfigException& exc) {
    svcTypeOnLine = "";
  }
  try {
    daysValid = config->getInt("DaysValid");
  } catch(ConfigException& exc) {
    daysValid = 1;
  }

  try {
    _qosTimeToLive = config->getInt("QosTimeToLive");
  } catch(ConfigException& exc) {
    _qosTimeToLive = 90; // in minutes
  }

  maxDataSmRegistrySize=50;
  try{
    maxDataSmRegistrySize=config->getInt("maxDataSmRegistrySize");
  }catch(ConfigException& exc)
  {
    smsc_log_warn(logger,"MCISme.maxDataSmRegistrySize not found, using default:%d",maxDataSmRegistrySize);
  }

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
  try {
    releaseSettings.detachCause   = releaseSettingsCfg->getInt ("Detach.cause");
  } catch (...) {
    smsc_log_warn(logger, "Parameter <MCISme.Reasons.Detach.cause> missed. Using default (20)");
    releaseSettings.detachCause = 20;
  }
  try {
    releaseSettings.detachInform  = releaseSettingsCfg->getBool("Detach.inform") ? 1:0;
  } catch (...) {
    smsc_log_warn(logger, "Parameter <MCISme.Reasons.Detach.inform> missed. Using default (0)");
    releaseSettings.detachInform = 0;
  }
  try {
    releaseSettings.skipUnknownCaller = releaseSettingsCfg->getBool("skipUnknownCaller");
  } catch (...) {
    smsc_log_warn(logger, "Parameter <MCISme.Reasons.skipUnknownCaller> missed. Skip is switched off.");
    releaseSettings.skipUnknownCaller = false;
  }
  try {
    releaseSettings.strategy = releaseSettingsCfg->getInt ("strategy");
  } catch (...) {
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

  try {
    _originatingAddressIsMCIAddress = config->getBool("OriginatingAddressIsMCIAddress");
  } catch (...) {
    _originatingAddressIsMCIAddress = false;
  }

  int32_t sendAbntOnlineNotificationPeriodInHours;
  try {
    sendAbntOnlineNotificationPeriodInHours = config->getInt("SendAbntOnlineNotificationPeriod");
  } catch(ConfigException& exc) {
    sendAbntOnlineNotificationPeriodInHours = 12;
  }

  _sendAbntOnlineNotificationPeriod = sendAbntOnlineNotificationPeriodInHours * 3600;

  try {
    _maxMessageSize = config->getInt("maxAbonentMessageLenInOctets");
  } catch(ConfigException& exc) {
    _maxMessageSize = 140;
  }

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

  maxInQueueSize  = config->getInt("inputQueueSize");

  try {
    forceInform = config->getBool("forceInform");
  } catch (...) {
    forceInform = true;
    smsc_log_warn(logger, "Parameter <MCISme.forceInform> missed. Default value is 'true'.");
  }
  try {
    forceNotify = config->getBool("forceNotify");
  } catch (...){
    forceInform = true;
    smsc_log_warn(logger, "Parameter <MCISme.forceNotify> missed. Default value is 'true'.");
  }

  try {
    _isUseWantNotifyPolicy = config->getBool("useWantNotifyPolicy");
  } catch (...) {
    _isUseWantNotifyPolicy = false;
    smsc_log_warn(logger, "Parameter <MCISme.useWantNotifyPolicy> missed. Default value is 'false'.");
  }

  bool bDefaultInform;
  try {
    bDefaultInform = config->getBool("defaultInform");
  } catch (...) {
    bDefaultInform = true;
    smsc_log_warn(logger, "Parameter <MCISme.defaultInform> missed. Default profile inform flag is on");
  }

  bool bDefaultNotify;
  try {
    bDefaultNotify = config->getBool("defaultNotify");
  } catch (...) {
    bDefaultNotify = false;
    smsc_log_warn(logger, "Parameter <MCISme.defaultNotify> missed. Default profile notify flag is off");
  }

  bool bDefaultWantNotifyMe;
  try {
    bDefaultWantNotifyMe = config->getBool("defaultWantNotifyMe");
  } catch (...) {
    bDefaultWantNotifyMe = false;
    smsc_log_warn(logger, "Parameter <MCISme.defaultWantNotifyMe> missed. Default profile wantNotifyMe flag is off");
  }
  int defaultReasonsMask = smsc::misscall::NONE;
  try {
    defaultReasonsMask = config->getInt("defaultReasonsMask");
  } catch (...) {
    defaultReasonsMask = smsc::misscall::ALL; // 0xFF;
    smsc_log_warn(logger, "Parameter <MCISme.defaultReasonsMask> missed. All masks on by default");
  }

  int rowsPerMessage = 5;
  try {
    rowsPerMessage = config->getInt("maxRowsPerMessage");
  } catch (...) {
    rowsPerMessage = 5;
    smsc_log_warn(logger, "Parameter <MCISme.maxRowsPerMessage> missed. Using default %d", rowsPerMessage);
  }
  string timeZoneFileLocation;
  try {
    timeZoneFileLocation = config->getString("timeZoneFileLocation");
  } catch (...) {
    throw ConfigException("MCISme need timeZoneFileLocation parameter."
                          "Use <MCISme.timeZoneFileLocation> parameter in config.xml.");
  }
  string routesFileLocation;
  try {
    routesFileLocation = config->getString("routesFileLocation");
  } catch (...) {
    throw ConfigException("MCISme need routesFileLocation parameter."
                          "Use <MCISme.routesFileLocation> parameter in config.xml.");
  }
  try {
    smsc::system::common::TimeZoneManager::Init(timeZoneFileLocation.c_str(), routesFileLocation.c_str());
  } catch(Exception e) {
    smsc_log_warn(logger, "TimeZoneManager::Init() error. %s", e.what());
    throw ConfigException(e.what());
  }

  try {
    test_number = config->getString("testNumber");
  } catch (...) {
    test_number="";
  }

  // Init Statistics
  std::auto_ptr<ConfigView> statCfgGuard(config->getSubConfig("Statistics"));
  ConfigView* statCfg = statCfgGuard.get();

  string statDir;
  try {
    statDir = statCfg->getString("statDir");
  } catch (...){
    statDir = "stat";
    smsc_log_warn(logger, "Parameter <MCISme.Statistics.location> missed. Default value is 'stat'.");
  }

  statistics = new StatisticsManager(statDir.c_str());
  if (statistics) statistics->Start();

  AbonentProfiler::init(0, defaultReasonsMask, bDefaultInform, bDefaultNotify, bDefaultWantNotifyMe);

  // Init Delivery Queue
  std::auto_ptr<ConfigView> schedulingCfgGuard(config->getSubConfig("Scheduling"));
  ConfigView* schedulingCfg = schedulingCfgGuard.get();

  string sResendingPeriod;
  try {
    sResendingPeriod = schedulingCfg->getString("resendingPeriod");
  } catch (...) {
    sResendingPeriod = "00:30:00";
    smsc_log_warn(logger, "Parameter <MCISme.ResendingPeriod> missed. Default value is '00:30:00'.");
  }
  time_t resendingPeriod = parseTime(sResendingPeriod.c_str());

  string sSchedOnBusy;
  try { sSchedOnBusy = schedulingCfg->getString("schedOnBusy"); } catch (...){sSchedOnBusy = "00:05:00";
    smsc_log_warn(logger, "Parameter <MCISme.ResendingPeriod> missed. Default value is '00:05:00'.");}
  time_t schedOnBusy = parseTime(sSchedOnBusy.c_str());

  pDeliveryQueue = new DeliveryQueue(resendingPeriod, schedOnBusy);

  std::auto_ptr<ConfigView> schedCfgGuard(schedulingCfg->getSubConfig("schedule"));
  ConfigView* schedCfg = schedCfgGuard.get();

  std::auto_ptr< std::set<std::string> > schedSetGuard(schedCfg->getStrParamNames());
  std::set<std::string>* schedSet = schedSetGuard.get();

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
    }
  }
  else
    smsc_log_warn(logger, "Schedule Table not specified.");

  std::auto_ptr<ConfigView> advertCfgGuard(config->getSubConfig("Advertising"));
  ConfigView* advertCfg = advertCfgGuard.get();

  try {
    svcTypeForBe = advertCfg->getString("SvcTypeForBe");
  } catch(ConfigException& exc) {
    svcTypeForBe = "";
  }

  try {
    if (advertCfg->getBool("useAdvert"))
      _outputMessageProcessorsDispatcher = new BannerOutputMessageProcessorsDispatcher(*this, advertCfg);
    else
      _outputMessageProcessorsDispatcher = new BannerlessOutputMessageProcessorsDispatcher(*this);
  } catch (...) {
    smsc_log_warn(logger, "Parameter <MCISme.Advertising.useAdvert> missed. Defaul profile useAdvert is false (off)");
    _outputMessageProcessorsDispatcher = new BannerlessOutputMessageProcessorsDispatcher(*this);
  }

  std::auto_ptr<ConfigView> storageCfgGuard(config->getSubConfig("Storage"));
  pStorage = new FSStorage();
  int ret = pStorage->Init(storageCfgGuard.get(), pDeliveryQueue);
  smsc_log_warn(logger, "ret = %d", ret);

  string sResponseWaitTime;
  try {
    sResponseWaitTime = config->getString("responceWaitTime");
  } catch (...) {
    sResponseWaitTime = "00:00:60";
    smsc_log_warn(logger, "Parameter <MCISme.responceWaitTime> missed. Default value is '00:00:60'.");
  }
  responseWaitTime = parseTime(sResponseWaitTime.c_str());

  timeoutMonitor = new TimeoutMonitor(this);
  pDeliveryQueue->SetResponseWaitTime(responseWaitTime);

  std::auto_ptr<ConfigView> iasmeCfgGuard(config->getSubConfig("IASME"));
  ConfigView* iasmeCfg = iasmeCfgGuard.get();
  if (iasmeCfg) {
    const char* listeningIface = iasmeCfg->getString("listen_iface", NULL, false);
    in_port_t listeningPort = iasmeCfg->getInt("listen_port");
    unsigned maxIASMEQueueSz = 10;
    try {
      maxIASMEQueueSz = iasmeCfg->getInt("max_iasme_events_queue_sz");
    } catch (...) {}
    _iasmeProxy = new IASMEProxy(listeningIface, listeningPort, this, maxIASMEQueueSz);
    _iasmeProxy->Start();
  }
  smsc_log_info(logger, "Load success.");
}

TaskProcessor::~TaskProcessor()
{
  _iasmeProxy->stop();
  try {
    this->Stop();
  } catch (...) {}
  delete templateManager;
  delete mciModule;
  delete statistics;
  delete pStorage;
  delete pDeliveryQueue;
  delete timeoutMonitor;
  try {
    smsc::system::common::TimeZoneManager::Shutdown();
  } catch (...) {}
  delete _outputMessageProcessorsDispatcher;
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
  time_t delivery_time;
  time_t cur_time;
  int    wait_smsc_msg;

  while(!bStopProcessing)
  {
    if (mciModule && !mciModule->isRunning()) {
      smsc_log_info(logger, "MCI Module is down. Exiting message processing loop");
      break;
    }
    if(smsInfo.Count()>=maxDataSmRegistrySize)
    {
      usleep(10000);//microseconds
    }
    try {
      AbntAddr abnt;
      if(pDeliveryQueue->Get(abnt))
      {
        // dispatch message to one of active processing threads
        _outputMessageProcessorsDispatcher->dispatchSendMissedCallNotification(abnt);
      }
    } catch (std::exception& exc) {
      smsc_log_error(logger, ERROR_MESSAGE, exc.what());
    } catch (...) {
      smsc_log_error(logger, ERROR_MESSAGE, "Cause is unknown");
    }
  }
  smsc_log_info(logger, "Message processing loop exited.");
}

int TaskProcessor::Execute()
{
  clearSignalMask();

  MissedCallEvent event;

  smsc_log_debug(logger, "Execute");

  time_t start = time(0);
  time_t end = time(0);
  uint32_t count=0;

  while (bInQueueOpen)
  {
    try
    {
      if (!getFromInQueue(event)) break;

      try {
        checkAddress(event.to.c_str());
      } catch(Exception e) {
        smsc_log_error(logger, "Bad called number. %s", e.what());
        continue;
      }

      try {
        checkAddress(event.from.c_str());
      } catch(Exception e) {
        smsc_log_error(logger, "Bad calling number. %s", e.what());
      }

      AbntAddr to(event.to.c_str()), from(event.from.c_str());

      AbonentProfile profile;
      profileStorage->Get(to, profile);

      if(!forceInform && !profile.inform)
        smsc_log_debug(logger, "Event %s->%s skipped because inform disabled for abonent %s and forceInform is false.", event.from.c_str(), event.to.c_str(), to.toString().c_str());
      else if (checkEventMask(profile.eventMask, event.cause))
      {
        if ( _iasmeProxy && _iasmeProxy->isConnected() &&
            (event.cause&BUSY)==BUSY && !event.gotFromIAMSME ) {
          if ( !forwardEventToIASME(event) )
            processLocally(from, to, event, profile);
        } else
          processLocally(from, to, event, profile);
      }
      else
        smsc_log_debug(logger, "Event: for abonent %s skipped (userMask=%02X, eventCause=%02X)", to.getText().c_str(), profile.eventMask, event.cause);
    } catch (BadAddrException& ex) {
      smsc_log_error(logger, "TaskProcessor::Execute::: caught BadAddrException [%s]", ex.what());
    } catch (...) {
      smsc_log_error(logger, "Exception in TaskProcessor::Execute()");
    }
  }
  exitedEvent.Signal();
  return 0;
}

void
TaskProcessor::processLocally(const AbntAddr& from, const AbntAddr& to,
                              const MissedCallEvent& event,
                              const AbonentProfile& profile)
{
  AbonentProfile callerProfile;
  profileStorage->Get(from, callerProfile);

  MCEvent  outEvent;
  outEvent = from;
  outEvent.dt = time(0);
  outEvent.missCallFlags = event.flags;

  time_t schedTime = pDeliveryQueue->calculateSchedTime((event.cause&BUSY)==BUSY);
  pStorage->addEvent(to, outEvent, schedTime);
  pDeliveryQueue->Schedule(to, ((event.cause&BUSY)==BUSY), schedTime);

  store_A_Event_in_logstore(from, to, profile, callerProfile);
  statistics->incMissed();
  smsc_log_debug(logger, "Abonent %s (cause = 0x%02X) was added to Scheduled Delivery Queue",
                 to.toString().c_str(), event.cause);
}

bool
TaskProcessor::forwardEventToIASME(const MissedCallEvent &event)
{
  return _iasmeProxy->sendRequest(event);
}

void
TaskProcessor::ProcessAbntEvents(const AbntAddr& abnt,
                                 SendMessageEventHandler* bannerEngineProxy)
{
  static time_t start = time(0);
  time_t end;
  static uint32_t count=0;

  vector<MCEvent> events;

  if(!GetAbntEvents(abnt, events))
  {
    smsc_log_info(logger, "No more events for Abonent %s ", abnt.toString().c_str());
    pDeliveryQueue->Remove(abnt);
    return;
  }
  smsc_log_info(logger, "Attempt to send SMS to Abonent %s ", abnt.toString().c_str());

  AbonentProfile profile;
  profileStorage->Get(abnt, profile);

  if(!forceInform && !profile.inform)
  {
    smsc_log_info(logger, "Inform is off for Abonent %s - cancel sending", abnt.toString().c_str());
    pStorage->deleteEvents(abnt, events);
    return;
  }

  MessageFormatter formatter(templateManager->getInformFormatter(profile.informTemplateId));

  int timeOffset = static_cast<int>(smsc::system::common::TimeZoneManager::getInstance().getTimeZone(abnt.getAddress())+timezone);

  MCEventOut mcEventOut("", "");
  if ( !formatter.formatMessage(abnt, events, &mcEventOut, getAddress(), timeOffset, _originatingAddressIsMCIAddress) ) {
    smsc_log_debug(logger, "TaskProcessor::ProcessAbntEvents::: no more events for abonent '%s', formatter.formatMessage retuned false", abnt.toString().c_str());
    return;
  }

  smsc_log_debug(logger, "TaskProcessor::ProcessAbntEvents::: next out event=[%s]", mcEventOut.toString().c_str());
  Message msg;

  if(profile.informTemplateId == STK_PROFILE_ID)
    msg.secured_data = true;
  else
    msg.data_sm = true;

  msg.abonent = abnt.getText();
  msg.message = mcEventOut.msg;
  msg.caller_abonent = _originatingAddressIsMCIAddress ? getAddress() : mcEventOut.caller;

  size_t messageSize = msg.message.size();
  bool needBannerInTranslit = !hasHighBit(msg.message.c_str(), messageSize);
  BannerResponseTrace bannerRespTrace;
  if(bannerEngineProxy)
    addBanner(msg, bannerEngineProxy->getBanner(abnt, &bannerRespTrace, needBannerInTranslit,
                                                static_cast<uint32_t>(_maxMessageSize - messageSize)));

  smsc_log_info(logger, "ProcessAbntEvents: prepared message = '%s' for sending to %s from %s", msg.message.c_str(), msg.abonent.c_str(), msg.caller_abonent.c_str());

  sendMessage(abnt, msg, mcEventOut, bannerRespTrace);
}

bool
TaskProcessor::sendMessage(const AbntAddr& abnt, const Message& msg,
                           const MCEventOut& outEvent, const BannerResponseTrace& bannerRespTrace)
{
  int seqNum;
  try {
    seqNum = getMessageSender()->getSequenceNumber();
  } catch (RetryException& ex) {
    pDeliveryQueue->Reschedule(abnt);
    throw;
  }

  sms_info* pInfo = new sms_info;

  pInfo->sending_time = time(0);
  pInfo->abnt = abnt;
  pInfo->events = outEvent.srcEvents;
  pInfo->bannerRespTrace = bannerRespTrace;
  insertSmsInfo(seqNum, pInfo);

  try {
    if(!getMessageSender()->send(seqNum, msg))
    {
      smsc_log_error(logger, "Send DATA_SM for Abonent %s failed", pInfo->abnt.toString().c_str());
      pDeliveryQueue->Reschedule(pInfo->abnt);
      deleteSmsInfo(seqNum);
      delete pInfo;
      return false;
    }
  } catch (RetryException& ex) {
    pDeliveryQueue->Reschedule(abnt);
    deleteSmsInfo(seqNum);
    delete pInfo;
    throw;
  }
  return true;
}

void
TaskProcessor::store_D_Event_in_logstore(const AbntAddr& abnt,
                                         const vector<MCEvent>& events,
                                         const AbonentProfile& abntProfile )
{
  const std::string& calledAbonent = abnt.getText();
  for(vector<MCEvent>::const_iterator iter = events.begin(), end_iter = events.end();
      iter != end_iter; ++iter) {
    const std::string& callingAbonent = AbntAddr(&iter->caller).getText();
    AbonentProfile callerProfile;
    profileStorage->Get(AbntAddr(&iter->caller), callerProfile);

    smsc_log_debug(logger, "TaskProcessor::store_D_Event_in_logstore::: add event to logstore: calledAbonent=%s,callingAbonent=%s,abntProfile.notify=%d,abntProfile.wantNotifyMe=%d", calledAbonent.c_str(), callingAbonent.c_str(), abntProfile.notify, abntProfile.wantNotifyMe);
    MCAEventsStorageRegister::getMCAEventsStorage().addEvent(Event_MissedCallInfoDelivered(callingAbonent, calledAbonent, abntProfile.notify, callerProfile.wantNotifyMe));
  }
}

void
TaskProcessor::store_F_Event_in_logstore(const AbntAddr& abnt,
                                         const vector<MCEvent>& events)
{
  const std::string& calledAbonent = abnt.getText();
  for(vector<MCEvent>::const_iterator iter = events.begin(), end_iter = events.end();
      iter != end_iter; ++iter) {
    const std::string& callingAbonent = AbntAddr(&iter->caller).getText();

    smsc_log_debug(logger, "TaskProcessor::store_F_Event_in_logstore::: add event to logstore: calledAbonent=%s,callingAbonent=%s", calledAbonent.c_str(), callingAbonent.c_str());
    MCAEventsStorageRegister::getMCAEventsStorage().addEvent(Event_MissedCallInfoDeliveringFailed(callingAbonent, calledAbonent));
  }
}

void
TaskProcessor::store_N_Event_in_logstore(const std::string& calledAbonent,
                                         const std::string& callingAbonent)
{
  smsc_log_debug(logger, "TaskProcessor::store_N_Event_in_logstore::: add event to logstore: calledAbonent=%s,callingAbonent=%s", calledAbonent.c_str(), callingAbonent.c_str());
  MCAEventsStorageRegister::getMCAEventsStorage().addEvent(Event_SendCallerNotification(callingAbonent, calledAbonent));
}

void
TaskProcessor::store_A_Event_in_logstore(const AbntAddr& callingAbonent,
                                         const AbntAddr& calledAbonent,
                                         const AbonentProfile& abntProfile,
                                         const AbonentProfile& callerProfile )
{
  smsc_log_debug(logger, "TaskProcessor::store_A_Event_in_logstore::: add event to logstore: calledAbonent=%s,callingAbonent=%s,abntProfile.notify=%d,abntProfile.wantNotifyMe=%d", calledAbonent.getText().c_str(), callingAbonent.getText().c_str(), abntProfile.notify, abntProfile.wantNotifyMe);
  MCAEventsStorageRegister::getMCAEventsStorage().addEvent(Event_GotMissedCall(callingAbonent.getText(), calledAbonent.getText(), abntProfile.notify, callerProfile.wantNotifyMe));
}

bool TaskProcessor::invokeProcessDataSmResp(int cmdId, int status, int seqNum)
{
  std::auto_ptr<sms_info> pInfo;
  {
    MutexGuard Lock(smsInfoMutex);
    if(!smsInfo.Exist(seqNum))
    {
      smsc_log_debug(logger, "No info for SMS seqNum = %d\n", seqNum);
      return false;
    }
    pInfo.reset(smsInfo.Get(seqNum));
    smsInfo.Delete(seqNum);
    toList.erase(pInfo->timeoutIter);
  }

  smsc_log_info(logger, "Recieve a DATA_SM_RESP for Abonent %s seq_num=%d, status=%d", pInfo->abnt.toString().c_str(), seqNum, status);

  if(status == smsc::system::Status::OK)
  {
    AbonentProfile abntProfile;
    profileStorage->Get(pInfo->abnt, abntProfile);

    _outputMessageProcessorsDispatcher->dispatchSendAbntOnlineNotifications(pInfo.release(), abntProfile);
  } else {
    BannerResponseTrace emptyBannerRespTrace;
    if ( pInfo->bannerRespTrace != emptyBannerRespTrace &&
         pInfo->bannerRespTrace.bannerIdIsNotUsed  == false )
      _outputMessageProcessorsDispatcher->dispatchBERollbackRequest(pInfo->bannerRespTrace);

    statistics->incFailed(static_cast<unsigned>(pInfo->events.size()));
    store_F_Event_in_logstore(pInfo->abnt, pInfo->events);

    if(smsc::system::Status::isErrorPermanent(status)) {
      pStorage->deleteEvents(pInfo->abnt, pInfo->events);
    } else {
      time_t schedTime = pDeliveryQueue->Reschedule(pInfo->abnt, status);
      pStorage->setSchedParams(pInfo->abnt, schedTime, status);
    }
  }

  return true;
}

bool
TaskProcessor::invokeProcessSubmitSmResp(int cmdId, int status, int seqNum)
{
  smsc_log_debug(logger, "Recieve a SUBMIT_SM_RESP seq_num=%d, status=%d", seqNum);

  BannerResponseTrace bannerRespTrace = deleteBannerInfo(seqNum);
  BannerResponseTrace emptyBannerRespTrace;

  if (status != smsc::system::Status::OK &&
      bannerRespTrace != emptyBannerRespTrace &&
      bannerRespTrace.bannerIdIsNotUsed  == false)
  {
    _outputMessageProcessorsDispatcher->dispatchBERollbackRequest(bannerRespTrace);
  }
  return true;
}

void
TaskProcessor::commitMissedCallEvents(const sms_info* pInfo, const AbonentProfile& abntProfile )
{
  statistics->incDelivered(static_cast<unsigned>(pInfo->events.size()));
  pStorage->deleteEvents(pInfo->abnt, pInfo->events);

  store_D_Event_in_logstore(pInfo->abnt, pInfo->events, abntProfile );

  time_t schedTime = pDeliveryQueue->Reschedule(pInfo->abnt, smsc::system::Status::OK);
  pStorage->setSchedParams(pInfo->abnt, schedTime, smsc::system::Status::OK);
}

void TaskProcessor::invokeProcessDataSmTimeout(void)
{
  typedef std::pair<int,sms_info*> sms_info_entry_t;
  std::list<sms_info_entry_t> smsInfoCache;
  sms_info* pInfo = 0;
  int seqNum = 0, count = 0;
  time_t curTime = time(0);
  {
    MutexGuard Lock(smsInfoMutex);
    int count_old = smsInfo.Count();
    if ( !count_old ) return;
    smsc_log_debug(logger, "Start searching unresponded DATA_SM. Total SMS in Hash is %d", count_old);
    TimeoutList::iterator it;
    while((it=toList.begin())->expirationTime<curTime)
    {
      smsInfoCache.push_back(sms_info_entry_t(it->seqNum,it->info));
      smsInfo.Delete(it->seqNum);
      toList.erase(it);
    }
    /*
    smsc::core::buffers::IntHash<sms_info*>::Iterator It(smsInfo.First());

    while (It.Next(seqNum, pInfo)) {
      if(curTime > (pInfo->sending_time + responseWaitTime)) {
        smsc_log_info(logger, "SMS for Abonent %s (seqNum %d) is removed from waiting list by timeout", pInfo->abnt.toString().c_str(), seqNum);

        statistics->incFailed(static_cast<unsigned>(pInfo->events.size()));
        smsInfoCache.push_back(std::make_pair(seqNum, pInfo));
        smsInfo.Delete(seqNum);
        count++;
      }
    }
    */
    int count_new = smsInfo.Count();
    smsc_log_debug(logger, "Complete searching unresponded DATA_SM. Total SMS in Hash is %d, removed %d (%d)", count_new, count_old - count_new, count);

  }

  for(std::list<sms_info_entry_t>::iterator iter = smsInfoCache.begin(), end_iter=smsInfoCache.end();
      iter != end_iter; ++iter)
  {
    seqNum = (*iter).first;
    pInfo = (*iter).second;
    smsc_log_info(logger, "SMS for Abonent %s (seqNum %d) is removed from waiting list by timeout", pInfo->abnt.toString().c_str(), seqNum);
    statistics->incFailed(static_cast<unsigned>(pInfo->events.size()));
    try {
      BannerResponseTrace emptyBannerRespTrace;
      if ( pInfo->bannerRespTrace != emptyBannerRespTrace &&
           pInfo->bannerRespTrace.bannerIdIsNotUsed  == false )
        _outputMessageProcessorsDispatcher->dispatchBERollbackRequest(pInfo->bannerRespTrace);

      time_t schedTime = pDeliveryQueue->Reschedule(pInfo->abnt);
      pStorage->setSchedParams(pInfo->abnt, schedTime, -1);

    } catch (std::exception& ex) {
      smsc_log_error(logger, "TaskProcessor::invokeProcessDataSmTimeout::: catched unexpected exception [%s]", ex.what());
    }
    delete pInfo;
  }
}

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

bool TaskProcessor::GetAbntEvents(const AbntAddr& abnt, vector<MCEvent>& events)
{
  if(!pStorage)
    return 0;

  return pStorage->getEvents(abnt, events);
}

bool
TaskProcessor::needNotify(const AbonentProfile& profile, const sms_info* pInfo) const
{
  if(forceNotify || profile.notify)
    return true;
  else
    smsc_log_info(logger, "Notify is off for Abonent %s", pInfo->abnt.toString().c_str());
  return false;
}

void
TaskProcessor::SendAbntOnlineNotifications(const sms_info* pInfo,
                                           const AbonentProfile& profile,
                                           SendMessageEventHandler* bannerEngineProxy)
{
  if ( !needNotify(profile, pInfo) ) return;

  size_t events_count = pInfo->events.size();

  NotifyTemplateFormatter* formatter = templateManager->getNotifyFormatter(profile.notifyTemplateId);
  OutputFormatter*  messageFormatter = formatter->getMessageFormatter();
  ContextEnvironment ctx;
  string abnt = pInfo->abnt.getText();

  for(int i = 0; i < events_count; i++)
  {
    Message msg;
    AbntAddr caller(&pInfo->events[i].caller);
    if ( time(0) - pInfo->events[i].dt > _sendAbntOnlineNotificationPeriod ) {
      char lastCallingTimeStr[32];
      smsc_log_info(logger, "SendAbntOnlineNotifications: last calling time=[%s] for caller '%s' to '%s', notificationPeriod was expired. Cancel sending online notification to calling abonents",
                    cTime(&pInfo->events[i].dt, lastCallingTimeStr, sizeof(lastCallingTimeStr)),
                    caller.getText().c_str(), abnt.c_str());
      return;
    }

    if ( _isUseWantNotifyPolicy ) {
      AbonentProfile callerProfile;
      profileStorage->Get(caller, callerProfile);
      if ( !callerProfile.wantNotifyMe ) {
        smsc_log_info(logger, "SendAbntOnlineNotifications: useWantNotifyPolicy=true, profile's wantNotifyMe flag=false for abonent=%s, won't send notification to abonent",
                      caller.getText().c_str());
        continue;
      }
    }

    msg.abonent = caller.getText();

    NotifyGetAdapter adapter(abnt, msg.abonent);
    messageFormatter->format(msg.message, adapter, ctx);

    try {
      checkAddress(msg.abonent.c_str());
    } catch(Exception e) {
      smsc_log_error(logger, "Skip notification message - bad address %s", e.what());
      continue;
    }

    msg.caller_abonent = _originatingAddressIsMCIAddress ? getAddress() : abnt;
    msg.notification = true;

    size_t messageSize = msg.message.size();
    bool needBannerInTranslit = !hasHighBit(msg.message.c_str(), messageSize);

    BannerResponseTrace bannerRespTrace;
    if(bannerEngineProxy)
      addBanner(msg, bannerEngineProxy->getBanner(pInfo->abnt, &bannerRespTrace, needBannerInTranslit,
                                                  static_cast<uint32_t>(_maxMessageSize - messageSize)));

    smsc_log_info(logger, "Notify message = %s to %s from %s", msg.message.c_str(), msg.abonent.c_str(), msg.caller_abonent.c_str());

    int seqNum = getMessageSender()->getSequenceNumber();
    insertBannerInfo(seqNum, bannerRespTrace);
    statistics->incNotified();
    getMessageSender()->send(seqNum, msg);
    store_N_Event_in_logstore(abnt, caller.getText());
  }
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
  MutexGuard guard(inQueueMonitor);
  while (!skip && bInQueueOpen && (inQueue.Count() >= maxInQueueSize)) {
    inQueueMonitor.wait();
  }
  if (!bInQueueOpen) return false;
  if (skip && inQueue.Count() >= maxInQueueSize) {
    return false;
  }
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
  AbntAddr        abnt(Abonent.c_str());
  SchedItem       item;
  vector<MCEvent> events;

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
    char			buff[128];
    time_t			schedTime;
    struct tm*		t;

    schedTime = items[i].schedTime;	t = localtime(&schedTime);
    snprintf(buff, 128, "%.2d.%.2d.%4d %.2d:%.2d:%.2d,%d;", t->tm_mday, t->tm_mon+1, t->tm_year+1900, t->tm_hour, t->tm_min, t->tm_sec,
             items[i].abonentsCount);

    result += buff;
  }
  return result;
}

void
TaskProcessor::handle(const mcaia::BusyResponse& msg)
{
  if ( msg.getStatus() == mcaia::Status::OK ) {
    smsc_log_info(logger, "TaskProcessor::handle::: got successful BusyResponse message='%s', stop further processing of corresponding missed call event",
                  msg.toString().c_str());
    return;
  }
  smsc_log_info(logger, "TaskProcessor::handle::: got unsuccessful BusyResponse message='%s', continue further processing of corresponding missed call event",
                msg.toString().c_str());
  misscall::MissedCallEvent eventInResponse;
  eventInResponse.gotFromIAMSME = true;

  eventInResponse.to = msg.getCalled();
  eventInResponse.from = msg.getCaller();
  eventInResponse.cause = msg.getCause();
  eventInResponse.flags = msg.getFlags();
  eventInResponse.time = msg.getDate();
  missed(eventInResponse);
}

}}
