//------------------------------------
//  TaskProcessor.cpp
//  Changed by Routman Michael, 2005-2006
//------------------------------------



#include <exception>
#include <algorithm>

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

extern bool isMSISDNAddress(const char* string);
extern "C" void clearSignalMask(void);

namespace smsc { namespace mcisme 
{

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
    static const char* ADDRESS_ERROR_NULL_MESSAGE    = "Destination address is undefined.";
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
        logger(Logger::getInstance("smsc.mcisme.TaskProcessor")), 
		profileStorage(ProfilesStorage::GetInstance()),
        protocolId(0), daysValid(1), svcType(0), address(0), 
        templateManager(0), mciModule(0), messageSender(0),
        statistics(0), maxInQueueSize(10000), maxOutQueueSize(10000),
        bStarted(false), bInQueueOpen(false), bOutQueueOpen(false), pStorage(0), pDeliveryQueue(0)
{
    smsc_log_info(logger, "Loading ...");

    address = config->getString("Address");
    if (!address || !isMSISDNAddress(address))
        throw ConfigException("Address string '%s' is invalid", address ? address:"-");

    try { protocolId = config->getInt("ProtocolId"); }
    catch(ConfigException& exc) { protocolId = 0; };
    try { svcType = config->getString("SvcType"); }
    catch(ConfigException& exc) { svcType = 0; };
    try { daysValid = config->getInt("DaysValid"); }
    catch(ConfigException& exc) { daysValid = 1; };

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
        if (!tsmStr || !tsmStr[0] || sscanf(tsmStr, "%lx", &tsmLong) != 1)
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

    bool bDefaultInform, bDefaultNotify;
    try { bDefaultInform = config->getBool("defaultInform"); } catch (...) { bDefaultInform = true;
        smsc_log_warn(logger, "Parameter <MCISme.defaultInform> missed. Defaul profile inform flag is on");
    }
    try { bDefaultNotify = config->getBool("defaultNotify"); } catch (...) { bDefaultNotify = false;
        smsc_log_warn(logger, "Parameter <MCISme.defaultNotify> missed. Defaul profile notify flag is off");
    }
    int defaultReasonsMask = smsc::misscall::NONE;
    try { defaultReasonsMask = config->getBool("defaultReasonsMask"); } catch (...) { 
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

//    std::auto_ptr<ConfigView> dsIntCfgGuard(config->getSubConfig("DataSource"));
//    initDataSource(dsIntCfgGuard.get());
//    dsStatConnection = ds->getConnection();
    
	std::auto_ptr<ConfigView> statCfgGuard(config->getSubConfig("Statistics"));
    ConfigView* statCfg = statCfgGuard.get();

	string statDir;
	try { statDir = statCfg->getString("statDir"); } catch (...){statDir = "stat";
		smsc_log_warn(logger, "Parameter <MCISme.Statistics.location> missed. Default value is 'stat'.");}
	
	statistics = new StatisticsManager(statDir.c_str());
    if (statistics) statistics->Start();
    AbonentProfiler::init(0, defaultReasonsMask, bDefaultInform, bDefaultNotify);

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


	std::auto_ptr<ConfigView> storageCfgGuard(config->getSubConfig("Storage"));
    ConfigView* storageCfg = storageCfgGuard.get();
	pStorage = new FSStorage();
	int ret = pStorage->Init(storageCfg, pDeliveryQueue);
	
	smsc_log_warn(logger, "ret = %d", ret);
	
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
	smsc::system::common::TimeZoneManager::Shutdown();
//    if (dsStatConnection) ds->freeConnection(dsStatConnection);
//    if (ds) delete ds;
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
        
		pDeliveryQueue->OpenQueue();
        openInQueue();
        Thread::Start();
        if (mciModule) mciModule->Attach(this);
        
        bStarted = true;
        smsc_log_info(logger, "Event processing started.");
    }
}
void TaskProcessor::Stop()
{
    MutexGuard  guard(startLock);
    if (bStarted)
    {
        smsc_log_info(logger, "Stopping event processing...");
        
        if (mciModule) mciModule->Detach();
        
        closeInQueue();
		pDeliveryQueue->CloseQueue();
        exitedEvent.Wait();

		bStarted = false;
        smsc_log_info(logger, "Event processing stoped.");
    }
}

void TaskProcessor::Run()
{
    static const char* ERROR_MESSAGE = "Failed to process message. Details: %s";
    
    smsc_log_info(logger, "Message processing loop started.");
	time_t	delivery_time;
	time_t	cur_time;
	int		wait_smsc_msg;

//	while(bOutQueueOpen)
	while(pDeliveryQueue->isQueueOpened())
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

	while (bInQueueOpen)//++i<=12)//bInQueueOpen
	{
		try
		{
			if (!getFromInQueue(event)) break;

			count++;
			end = time(0);
			if( (end-start) > 0)
			{
				MutexGuard guard(inQueueMonitor);
				uint32_t size = inQueue.Count();
				smsc_log_debug(logger, "speed: Current incoming speed is %d messages per second (current inQueueSize = %d)", count, size);
				start = time(0);
				count=0;
			}

			try{checkAddress(event.to.c_str());}catch(Exception e)
			{
				smsc_log_error(logger, "Bad called number. %s", e.what());
				continue;
			}
			if(test_number == event.from) event.from = "";
			try{checkAddress(event.from.c_str());}catch(Exception e)
			{
				smsc_log_error(logger, "Bad calling number. %s", e.what());
			}
			AbntAddr to(event.to.c_str()), from(event.from.c_str());
			outEvent = from;
			AbonentProfile profile;
			profileStorage->Get(to, profile);
//			if (releaseCallsStrategy != MIXED_STRATEGY || event.cause != ABSENT)
			if (checkEventMask(profile.eventMask, event.cause))
			{
				pStorage->addEvent(to, outEvent);
				pDeliveryQueue->Schedule(to, ((event.cause&0x02)==0x02)); //0x02 - BUSY
				statistics->incMissed();
				smsc_log_debug(logger, "Abonent %s (couse = 0x%02X) was added to Scheduled Delivery Queue", to.toString().c_str(), event.cause);
			}
			else
				smsc_log_debug(logger, "Event: for abonent %s skipped (userMask=%02X, eventCause=%02X)", to.getText().c_str(), profile.eventMask, event.cause);
		}
		catch(...)
		{
			smsc_log_error(logger, "Exeption in TaskProcessor::Execute()");
		}
	}
	exitedEvent.Signal();
    return 0;
}

void TaskProcessor::ProcessAbntEvents(const AbntAddr& abnt)
{
	static time_t start = time(0);
	time_t end;
	static uint32_t count=0;

	vector<MCEvent>		events;

	smsc_log_debug(logger, "Attempt to send SMS to Abonent %s ", abnt.toString().c_str());
	if(!GetAbntEvents(abnt, events))
	{
		smsc_log_debug(logger, "No more events for Abonent %s ", abnt.toString().c_str());
		pDeliveryQueue->Remove(abnt);
		return;
	}
//	AbonentProfile		profile(AbonentProfiler::getProfile(abnt.getText().c_str()));

	AbonentProfile profile;
	profileStorage->Get(abnt, profile);
	
	if(!forceInform && !profile.inform)
	{
		smsc_log_debug(logger, "Inform is off for Abonent %s - cancel sending", abnt.toString().c_str());
		pStorage->deleteEvents(abnt, events);
		return;
	}
	Message				msg;
	MessageFormatter	formatter(templateManager->getInformFormatter(profile.informTemplateId));
	sms_info*			pInfo = new sms_info;
	
	pInfo->abnt = abnt;
	int timeOffset = smsc::system::common::TimeZoneManager::getInstance().getTimeZone(abnt.getAddress())+timezone;
	formatter.formatMessage(msg, abnt, events, 0, pInfo->events, timeOffset);
	msg.data_sm = true;
//	smsc_log_debug(logger, "ProcessAbntEvents: msg = %s", msg.message.c_str());
	msg.abonent = abnt.getText();//"777";
	
	MutexGuard Lock(smsInfoMutex);
	int seqNum = messageSender->getSequenceNumber();
	int res = smsInfo.Insert(seqNum, pInfo);
	if(!messageSender->send(seqNum, msg))
	{
		smsc_log_debug(logger, "Send DATA_SM for Abonent %s failed", pInfo->abnt.toString().c_str());
		pDeliveryQueue->Reschedule(pInfo->abnt);
		smsInfo.Delete(seqNum);
		delete pInfo;
		return;
	}

	count+=pInfo->events.size();
	end = time(0);
	if( (end-start) > 0)
	{
		smsc_log_debug(logger, "speed: Current Delivery speed is %d messages per second ", count);
		start = time(0);
		count=0;
	}
}

bool TaskProcessor::invokeProcessDataSmResp(int cmdId, int status, int seqNum)
{
	if(cmdId == smsc::smpp::SmppCommandSet::DATA_SM_RESP)
	{
//		printf("Recieve a DATA_SM_RESP (cmdId = %d status = %d seqNum = %d )\n", cmdId, status, seqNum);

		MutexGuard Lock(smsInfoMutex);
		if(!smsInfo.Exist(seqNum))
		{
			smsc_log_debug(logger, "No info about SMS seqNum = %d\n", seqNum);
			return false;
		}
		sms_info* pInfo = smsInfo.Get(seqNum);
		smsInfo.Delete(seqNum);

		smsc_log_debug(logger, "Recieve a DATA_SM_RESP for Abonent %s seq_num = %d, status = %d", pInfo->abnt.toString().c_str(), seqNum, status);

		if(status == smsc::system::Status::OK)
		{	
			SendAbntOnlineNotifications(pInfo);
			statistics->incDelivered(pInfo->events.size());
			pStorage->deleteEvents(pInfo->abnt, pInfo->events);
			pDeliveryQueue->Reschedule(pInfo->abnt);
		}
		else if(smsc::system::Status::isErrorPermanent(status))
		{
			statistics->incDelivered(pInfo->events.size());
			StopProcessEvent4Abnt(pInfo->abnt);
		}
		else
		{
			statistics->incFailed(pInfo->events.size());
			pDeliveryQueue->Reschedule(pInfo->abnt, status);
		}
		delete pInfo;
	}
	return true;
};

bool TaskProcessor::invokeProcessAlertNotification(int cmdId, int status, const AbntAddr& abnt)
{
	smsc_log_debug(logger, "Recieve an ALERT_NOTIFICATION for Abonent %s status = %d", abnt.toString().c_str(), status);
	if(cmdId == smsc::smpp::SmppCommandSet::ALERT_NOTIFICATION)
		pDeliveryQueue->RegisterAlert(abnt);
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

void TaskProcessor::SendAbntOnlineNotifications(const sms_info* pInfo)
{
//	AbonentProfile	profile(AbonentProfiler::getProfile(pInfo->abnt.getText().c_str()));
	AbonentProfile profile;
	profileStorage->Get(pInfo->abnt, profile);

	smsc_log_debug(logger, "Process Notify message");
	statistics->incNotified();
	if(!forceNotify && !profile.notify)
	{
		smsc_log_debug(logger, "Notify is off for Abonent %s - cancel sending", pInfo->abnt.toString().c_str());
		return;
	}

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
		NotifyGetAdapter	adapter(abnt, callers[i].getText().c_str());
		messageFormatter->format(msg.message, adapter, ctx);
		msg.abonent = msg.abonent = callers[i].getText();//"444"
		try{checkAddress(msg.abonent.c_str());}catch(Exception e)
		{
			smsc_log_error(logger, "Skip notification message - bad address %s", e.what());
			continue;
		}
		smsc_log_debug(logger, "Notify message = %s", msg.message.c_str());
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
}
}