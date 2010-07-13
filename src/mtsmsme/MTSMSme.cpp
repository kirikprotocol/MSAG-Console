static char const ident[] = "$Id$";
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/recoder/recode_dll.h>
#include <util/smstext.h>
#include "util/regexp/RegExp.hpp"

#include <system/smscsignalhandlers.h>
#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <util/xml/init.h>

#include <system/smscsignalhandlers.h>
#include <system/status.h>

#include "processor/Processor.h"
#include "version.inc"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;
using namespace smsc::mtsmsme::processor;
using namespace smsc::system::Status;
static smsc::logger::Logger *logger = 0;

static Event mtsmSmeWaitEvent, mtsmSmeReady;
static Mutex needStopLock, needReconnectLock;

static int   mtsmSmeReadyTimeout  = 60000;
static bool  bMTSMSmeIsStopped    = false;
static bool  bMTSMSmeIsConnected  = false;
static bool  bMTSMSmeIsConnecting = false;

static RequestProcessor* requestProcessor = 0;

// TODO: Check Stop & Reconnect logic !!!

static void setNeedStop(bool stop=true) {
    MutexGuard gauard(needStopLock);
    bMTSMSmeIsStopped = stop;
    if (stop && requestProcessor) requestProcessor->Stop();
}
static bool isNeedStop() {
    MutexGuard gauard(needStopLock);
    return bMTSMSmeIsStopped;
}

static void setNeedReconnect(bool reconnect=true) {
    MutexGuard gauard(needReconnectLock);
    bMTSMSmeIsConnected = !reconnect;
    if (reconnect && requestProcessor) requestProcessor->Stop();
}
static bool isNeedReconnect() {
    MutexGuard gauard(needReconnectLock);
    return !bMTSMSmeIsConnected;
}

extern bool isMSISDNAddress(const char* string)
{
    try { Address converted(string); } catch (...) { return false; }
    return true;
}
extern bool convertMSISDNStringToAddress(const char* string, Address& address)
{
    try { Address converted(string); address = converted; } catch (...) { return false; }
    return true;
}

class FilterManager
{
  private:
    Logger* logger;
    bool filteringMode;
    smsc::util::regexp::RegExp allowSenderMaskRx;
    smsc::util::regexp::RegExp denySenderMaskRx;
  public:
    bool setCalledMask(const char* rx) {
      smsc::util::regexp::RegExp::InitLocale();
      return calledMaskRx.Compile(rx)!=0;
    }
    FilterManager(Logger* _logger):logger(_logger),filteringMode(false){}
    void init(Manager& manager)
    {
      char* sectionName = "MTSMSme.Filters";
      if (!manager.findSection(sectionName))
      {
        smsc_log_warn(logger,"\'%s\' section is missed, sender filtering is turned OFF",sectionName);
        return;
      }

      ConfigView filterConfig(manager, sectionName);

      char* allowSenderStr = 0;
      try {
        allowSenderStr = filterConfig.getString("allowSender");
      } catch (ConfigException& exc) {
        smsc_log_warn(logger,"\'allowSender\' is unknown or missing");
        return;
      }

      char* denySenderStr = 0;
      try {
        denySenderStr = filterConfig.getString("denySender");
      } catch (ConfigException& exc) {
        smsc_log_warn(logger,"\'denySender\' is unknown or missing");
        return;
      }
      smsc::util::regexp::RegExp::InitLocale();
      if (allowSenderMaskRx.Compile(allowSenderStr)==0)
      {
         smsc_log_warn(logger,"Failed to compile \'allowSender\' mask '%s'.",
             allowSenderStr ? allowSenderStr:"");
         return;
      }
      if (denySenderMaskRx.Compile(denySenderStr)==0)
      {
         smsc_log_warn(logger,"Failed to compile \'denySenderStr\' mask '%s'.",
             denySenderStr ? denySenderStr:"");
         return;
      }
      filteringMode = true;
    }
    bool isSenderAllowed(const Address& _sender)
    {
      if (filteringMode)
      {
        char sender[32] = {0};
        _sender.toString(sender,sizeof(sender));

        using smsc::util::regexp::SMatch;

        std::vector<SMatch> ma(allowSenderMaskRx.GetBracketsCount());
        int na=static_cast<int>(ma.size());
        if(!allowSenderMaskRx.Match(sender,&ma[0],na))
          return false;

        std::vector<SMatch> md(denySenderMaskRx.GetBracketsCount());
        int nd=static_cast<int>(md.size());
        if(denySenderMaskRx.Match(sender,&md[0],nd))
          return false;
      }
      return true;
    }
};

class AliasManager
{
private:

    Mutex           aliasLock;
    Hash<Address>   aliasByAddress;

public:

    AliasManager(ConfigView* config) throw(ConfigException)
    {
        MutexGuard guard(aliasLock); // init alias mapping

        smsc_log_info(logger, "Address to alias mapping loading ...");
        std::auto_ptr< std::set<std::string> > setGuard(config->getShortSectionNames());
        std::set<std::string>* set = setGuard.get();
        for (std::set<std::string>::iterator i=set->begin(); i!=set->end(); i++)
        {
            const char* section = (const char *)i->c_str();
            if (!section || !section[0]) continue;

            std::auto_ptr<ConfigView> addressConfigGuard(config->getSubConfig(section));
            ConfigView* addressConfig = addressConfigGuard.get();

            const char* address = addressConfig->getString("address");
            if (!address || !address[0])
                throw ConfigException("Address is empty or wasn't specified");
            const char* alias = addressConfig->getString("alias");
            if (!alias || !alias[0])
                throw ConfigException("Alias for address '%s' is empty or wasn't specified", address);

            Address addressAddress;
            if (!convertMSISDNStringToAddress(address, addressAddress)) {
                smsc_log_warn(logger, "Address '%s' has invalid format, skipped", address);
                continue;
            }
            address = addressAddress.toString().c_str();
            if (!aliasByAddress.Exists(address)) {
                Address aliasAddress;
                if (convertMSISDNStringToAddress(alias, aliasAddress)) aliasByAddress.Insert(address, aliasAddress);
                else smsc_log_warn(logger, "Alias '%s' for address '%s' has invalid format, skipped", alias, address);
            }
            else smsc_log_warn(logger, "Alias for address '%s' already defined", address);
        }
        smsc_log_info(logger, "Address to alias mapping loaded");
    }

    bool getAliasForAddress(const Address& address, Address& alias)
    {
        try
        {
            const char* addressStr = address.toString().c_str();
            if (!addressStr || !addressStr[0]) return false;

            MutexGuard guard(aliasLock);
            Address* aliasPtr = aliasByAddress.GetPtr(addressStr);
            if (!aliasPtr) return false;
            alias = *aliasPtr;
            return true;
        }
        catch (Exception& exc) {
            smsc_log_error(logger, "Alias mapping failure");
            return false;
        }
    }
};

class RequestController
{
private:

    Mutex             requestLock;
    IntHash<Request*> requestBySeqNum;

public:

    RequestController() {};
    ~RequestController()
    {
        // set error for all waiting requests
        MutexGuard guard(requestLock);
        IntHash<Request*>::Iterator it = requestBySeqNum.First();
        int seqNum; Request* request;
        while (it.Next(seqNum, request))
        {
            if (!request) {
                smsc_log_error(logger, "RequestController shutdown: request is null for seqNum=%d", seqNum);
                continue;
            }
            smsc_log_warn(logger, "RequestController shutdown: request for seqNum=%d skipped", seqNum);
            request->setSendResult(SMENOTCONNECTED); // TODO: what errorcode ???
        }

    };

    bool setRequest(int seqNum, Request* request)
    {
        MutexGuard guard(requestLock);
        if (requestBySeqNum.Exist(seqNum)) {
            smsc_log_error(logger, "RequestController: SMPP seqNum=%d already exists", seqNum);
            return false;
        }
        requestBySeqNum.Insert(seqNum, request);
        return true;
    }
    bool stateRequest(int seqNum, int status)
    {
        Request* request = 0;
        {
            MutexGuard guard(requestLock);
            Request** requestPtr = requestBySeqNum.GetPtr(seqNum);
            if (!requestPtr || !(*requestPtr)) {
                smsc_log_error(logger, "RequestController: Request is undefined for seqNum=%d", seqNum);
                return false;
            }
            request = *requestPtr;
            requestBySeqNum.Delete(seqNum);
        }
        request->setSendResult(status);
        return true;
    }
};

class MTSMSmeRequestSender: public RequestSender
{
private:

    SmppSession*        session;
    AliasManager&       aliaser;
    FilterManager&      filter;
    RequestController&  controller;
    Mutex               sendLock;

public:

    MTSMSmeRequestSender(SmppSession* _session, AliasManager& _aliaser,
                         FilterManager& _filter,
                         RequestController& _controller) :
                           RequestSender(), session(_session),
                           aliaser(_aliaser), filter(_filter),
                           controller(_controller) {};

    virtual ~MTSMSmeRequestSender() {
        MutexGuard guard(sendLock);
        session = 0;
    };

    virtual bool send(Request* request)
    {
        MutexGuard guard(sendLock);

        if (!request) {
            smsc_log_error(logger, "Request to send is null in RequestSender");
            return false;
        }
        if (!session) {
            smsc_log_error(logger, "Smpp session is undefined in RequestSender");
            return false;
        }
        SmppTransmitter* asyncTransmitter = session->getAsyncTransmitter();
        if (!asyncTransmitter) {
            smsc_log_error(logger, "Smpp transmitter is undefined in RequestSender");
            return false;
        }

        const SMS* sms = &(request->sms);
        Address oa;
        if (!filter.isSenderAllowed(sms->originatingAddress))
        {
          const char* oaStr = sms->originatingAddress.toString().c_str();
          smsc_log_error(logger, "Invalid source address '%s'", oaStr ? oaStr:"-");
          request->setSendResult(UNDEFSUBSCRIBER);
          return false;
        }
        Address da;
        if (!aliaser.getAliasForAddress(sms->destinationAddress, da)) {
            const char* daStr = sms->destinationAddress.toString().c_str();
            smsc_log_error(logger, "Invalid destination address '%s'", daStr ? daStr:"-");
            request->setSendResult(NOROUTE);
            return false;
        }

        PduSubmitSm  sm;
        sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
        fillSmppPduFromSms(&sm, (SMS *)sms);

        int seqNum = session->getNextSeq();
        sm.get_header().set_sequenceNumber(seqNum);
        sm.get_message().get_dest().set_typeOfNumber(da.type);
        sm.get_message().get_dest().set_numberingPlan(da.plan);
        sm.get_message().get_dest().set_value(da.value);

        smsc_log_info(logger, "Sending sms to smsc oa=%s da=%s imsi=%s",
                      sms->originatingAddress.toString().c_str(),
                      da.toString().c_str(),
                      sms->destinationAddress.toString().c_str());

        // TODO: set transactional mode in ESM_CLASS (by OR) ???
        controller.setRequest(seqNum,request);
        asyncTransmitter->sendPdu(&(sm.get_header()));
        return true;
    }
};

class MTSMSmePduListener: public SmppPduEventListener
{
protected:

    RequestController&  controller;

    SmppTransmitter*    syncTransmitter;
    SmppTransmitter*    asyncTransmitter;

public:

    MTSMSmePduListener(RequestController& _controller) : SmppPduEventListener(),
        controller(_controller), syncTransmitter(0), asyncTransmitter(0) {};
    virtual ~MTSMSmePduListener() {};

    void setSyncTransmitter(SmppTransmitter *transmitter) {
        syncTransmitter = transmitter;
    }
    void setAsyncTransmitter(SmppTransmitter *transmitter) {
        asyncTransmitter = transmitter;
    }

    void processResponce(SmppHeader *pdu)
    {
        if (!pdu) return;

        int seqNum = pdu->get_sequenceNumber();
        int status = pdu->get_commandStatus();

        // search and report status to Request via RequestController
        smsc_log_info(logger, "receive response for seqNum=%d (status=%d)", seqNum, status);
        controller.stateRequest(seqNum, status);
    }
    void handleTimeout(int seqNum)
    {
      int status = DELIVERYTIMEDOUT;
      smsc_log_info(logger, "receive timeout for seqNum=%d (status=%d)", seqNum, status);
      controller.stateRequest(seqNum, status);
    }
    void handleEvent(SmppHeader *pdu)
    {
        if (bMTSMSmeIsConnecting) {
            mtsmSmeReady.Wait(mtsmSmeReadyTimeout);
            if (bMTSMSmeIsConnecting) {
                disposePdu(pdu);
                return;
            }
        }

        switch (pdu->get_commandId())
        {
        case SmppCommandSet::SUBMIT_SM_RESP:
            processResponce(pdu);
            break;
        case SmppCommandSet::ENQUIRE_LINK: case SmppCommandSet::ENQUIRE_LINK_RESP:
            break;
        case SmppCommandSet::DELIVERY_SM:  case SmppCommandSet::CANCEL_SM_RESP:
        default:
            smsc_log_debug(logger, "Received unsupported Pdu %d !", pdu->get_commandId());
            break;
        }

        disposePdu(pdu);
    }
    void handleError(int errorCode)
    {
        smsc_log_error(logger, "Transport error handled! Code is: %d", errorCode);
        setNeedReconnect(true);
    }
};

class MTSMSmeConfig : public SmeConfig
{
private:

    char *strHost, *strSid, *strPassword, *strSysType, *strOrigAddr;

public:

    MTSMSmeConfig(ConfigView* config) throw(ConfigException)
        : SmeConfig(), strHost(0), strSid(0), strPassword(0), strSysType(0), strOrigAddr(0)
    {
        // Mandatory fields
        strHost = config->getString("host", "SMSC host wasn't defined !"); host = strHost;
        strSid = config->getString("sid", "MTSMSme id wasn't defined !");   sid = strSid;

        port = config->getInt("port", "SMSC port wasn't defined !");
        timeOut = config->getInt("timeout", "Connect timeout wasn't defined !");

        // Optional fields
        try {
            strPassword = config->getString("password", "MTSMSme password wasn't defined !");
            password = strPassword;
        } catch (ConfigException& exc) { password = ""; strPassword = 0; }
        try {
            strSysType = config->getString("systemType", "MTSMSme system type wasn't defined !");
            systemType = strSysType;
        } catch (ConfigException& exc) { systemType = ""; strSysType = 0; }
        try {
            strOrigAddr = config->getString("origAddress", "MTSMSme originating address wasn't defined !");
            origAddr = strOrigAddr;
        } catch (ConfigException& exc) { origAddr = ""; strOrigAddr = 0; }
    };

    virtual ~MTSMSmeConfig()
    {
        if (strHost) delete strHost;
        if (strSid) delete strSid;
        if (strPassword) delete strPassword;
        if (strSysType) delete strSysType;
        if (strOrigAddr) delete strOrigAddr;
    };
};
class RequestProcessorConfig {
  public:
    int ssn,user;
    Address msc;
    Address vlr;
    Address hlr;
  private:
    char* msc_str;
    char* vlr_str;
    char* hlr_str;
    Logger* logger;
  public:
    RequestProcessorConfig(Logger* _logger)
    {
      msc_str = NULL;
      vlr_str = NULL;
      hlr_str = NULL;
      logger = _logger;
    }
    void read(Manager& manager)
    {
      if (!manager.findSection("MTSMSme.SCCP"))
        throw ConfigException("\'SCCP\' section is missed");

      ConfigView sccpConfig(manager, "MTSMSme.SCCP");

      try { user = sccpConfig.getInt("user_id");
      } catch (ConfigException& exc) {
        throw ConfigException("\'user_id\' is unknown or missing");
      }

      try { ssn = sccpConfig.getInt("user_ssn");
      } catch (ConfigException& exc) {
        throw ConfigException("\'user_ssn\' is unknown or missing");
      }

      try { msc_str = sccpConfig.getString("msc_gt");
      } catch (ConfigException& exc) {
        throw ConfigException("\'msc_gt\' is unknown or missing");
      }
      msc = Address(msc_str);

      try { vlr_str = sccpConfig.getString("vlr_gt");
      } catch (ConfigException& exc) {
        throw ConfigException("\'vlr_gt\' is unknown or missing");
      }
      vlr = Address(vlr_str);

      try { hlr_str = sccpConfig.getString("hlr_gt");
      } catch (ConfigException& exc) {
        throw ConfigException("\'hlr_gt\' is unknown or missing");
      }
      hlr = Address(hlr_str);
    }
};

class SubscriberList
{
private:
    Manager* source;
public:
  SubscriberList(Manager* _source) { source = _source; }
  void registerto(HLROAM* hlr)
  {
    ConfigView cfg(*source, "MTSMSme.Aliases");
    ConfigView* config = &cfg;

    //smsc_log_info(logger, "Registering subscribers IMSI to database...");
    std::auto_ptr< std::set<std::string> > sections_ptr(config->getShortSectionNames());
    std::set<std::string>* set = sections_ptr.get();
    for (std::set<std::string>::iterator i=set->begin(); i!=set->end(); i++)
    {
      const char* section = (const char *)i->c_str();
      if (!section || !section[0]) continue;

      std::auto_ptr<ConfigView> subscriber_ptr(config->getSubConfig(section));
      ConfigView* subscriber_cfg = subscriber_ptr.get();

      const char* imsi = subscriber_cfg->getString("address");
      if (!imsi || !imsi[0])
        throw ConfigException("\'address\' param is empty or wasn't specified for section %s",section);
      const char* msisdn = subscriber_cfg->getString("msisdn");
      if (!msisdn || !msisdn[0])
        throw ConfigException("\'msisdn\' param is empty or wasn't specified for section %s", section);
      const char* mgt = subscriber_cfg->getString("mgt");
      if (!mgt || !mgt[0])
        throw ConfigException("\'mgt\' param is empty or wasn't specified for section %s", section);
      int period;
      try {
        period = subscriber_cfg->getInt("period");
      } catch (ConfigException& exc) {
            throw ConfigException("\'period\' param is empty or wasn't specified for section %s", section);
      }
      const char* reg_type = subscriber_cfg->getString("reg_type");
      if (!reg_type || !reg_type[0])
      {
        smsc_log_warn(logger,
                      "\'reg_type\' param is empty or wasn't specified "
                      "for section %s, \'external\' type is assumed", section);
        reg_type = "external";
      }
      if (strcmp(reg_type, "external") == 0)
        hlr->registerSubscriber(Address(imsi), Address(msisdn), Address(mgt),
                                period);
      else if (strcmp(reg_type, "internal") == 0)
        hlr->update(Address(imsi), Address(msisdn), Address(mgt));
      else
          throw ConfigException("\'reg_type\' param has wrong value for "
                                "section %s, \'external\' or \'internal\' "
                                "only applied", section);
    }
    //smsc_log_info(logger, "Registering subscribers IMSI to database has been completed");
  }
};

extern "C" void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT)
    {
        smsc_log_info(logger, "Stopping ...");
        setNeedStop(true); // Stop on RequestProcessor ???
    }
}
extern "C" void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
    smsc::logger::Logger::Shutdown();
}

extern "C" void setShutdownHandler(void)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
    if(pthread_sigmask(SIG_UNBLOCK, &set, NULL) != 0) {
        if (logger) smsc_log_error(logger, "Failed to set signal mask (shutdown handler)");
    }
    sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
}
extern "C" void clearSignalMask(void)
{
    sigset_t set;
    sigemptyset(&set);
    for(int i=1;i<=37;i++)
        if(i!=SIGQUIT && i!=SIGBUS && i!=SIGSEGV) sigaddset(&set,i);
    if(pthread_sigmask(SIG_SETMASK, &set, NULL) != 0) {
        if (logger) smsc_log_error(logger, "Failed to clear signal mask");
    }
}

struct ShutdownThread : public Thread
{
    Event shutdownEvent;

    ShutdownThread() : Thread() {};
    virtual ~ShutdownThread() {};

    virtual int Execute() {
        clearSignalMask();
        setShutdownHandler();
        shutdownEvent.Wait();
        return 0;
    };
    void Stop() {
        shutdownEvent.Signal();
    };
} shutdownThread;

int main(void)
{
    smsc::logger::Logger::Init();
    logger = smsc::logger::Logger::getInstance("mt.sme");

    atexit(atExitHandler);
    clearSignalMask();

    shutdownThread.Start();

    int resultCode = 0;
    int requestProcessorCode = 0;
    try
    {
        smsc_log_info(logger, getStrVersion());

        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView smscConfig(manager, "MTSMSme.SMSC");
        MTSMSmeConfig cfg(&smscConfig);

        ConfigView aliasConfig(manager, "MTSMSme.Aliases");
        AliasManager aliaser(&aliasConfig);

        FilterManager filter(logger);
        filter.init(manager);

        RequestProcessorConfig rp_cfg(logger);
        rp_cfg.read(manager);

        RequestProcessorFactory* factory = 0;
        factory = RequestProcessorFactory::getInstance();
        if (!factory) throw Exception("RequestProcessorFactory is undefined");

        requestProcessor = factory->createRequestProcessor();
        if (!requestProcessor) throw Exception("RequestProcessor is undefined");

        requestProcessor->configure(rp_cfg.user,rp_cfg.ssn,rp_cfg.msc,rp_cfg.vlr,rp_cfg.hlr);
        SubscriberList sm(&manager);
        sm.registerto(requestProcessor->getHLROAM());

        while (!isNeedStop())
        {
            RequestController       controller;
            MTSMSmePduListener      listener(controller);
            SmppSession             session(cfg, &listener);
            MTSMSmeRequestSender    sender(&session, aliaser, filter, controller);

            smsc_log_info(logger, "Connecting to SMSC ... ");
            try
            {
                listener.setSyncTransmitter(session.getSyncTransmitter());
                listener.setAsyncTransmitter(session.getAsyncTransmitter());

                bMTSMSmeIsConnecting = true;
                mtsmSmeReady.Wait(0);
                setNeedReconnect(false);
                session.connect();
                requestProcessor->setRequestSender(&sender);
                bMTSMSmeIsConnecting = false;
                mtsmSmeReady.Signal();
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what();
                smsc_log_error(logger, "Connect to SMSC failed. Cause: %s", (msg) ? msg:"unknown");
                bMTSMSmeIsConnecting = false;
                setNeedReconnect(true);
                // ??? if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw exc;
                sleep(cfg.timeOut);
                session.close();
                continue;
            }
            smsc_log_info(logger, "Connected.");

            smsc_log_info(logger, "Running RequestProcessor ...");
            requestProcessorCode = requestProcessor->Run();
            smsc_log_info(logger, "RequestProcessor exited with code=%d", requestProcessorCode);
            if (requestProcessorCode != 0) setNeedStop();

            clearSignalMask();
            smsc_log_info(logger, "Disconnecting from SMSC ...");
            session.close();
        }

    } catch (SmppConnectException& exc) {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            smsc_log_error(logger, "Failed to bind MTSMSme. Exiting");
        resultCode = -1;
    } catch (ConfigException& exc) {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting", exc.what());
        resultCode = -2;
    } catch (Exception& exc) {
        smsc_log_error(logger, "Top level Exception: %s Exiting", exc.what());
        resultCode = -3;
    } catch (exception& exc) {
        smsc_log_error(logger, "Top level exception: %s Exiting", exc.what());
        resultCode = -4;
    } catch (...) {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting");
        resultCode = -5;
    }

    shutdownThread.Stop();
    return resultCode;
}
