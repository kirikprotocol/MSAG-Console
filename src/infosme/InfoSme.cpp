#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Array.hpp>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/recoder/recode_dll.h>
#include <util/smstext.h>

#include <db/DataSourceLoader.h>
#include <core/synchronization/EventMonitor.hpp>

#include <system/smscsignalhandlers.h>
#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <util/xml/init.h>

#include <admin/service/Component.h>
#include <admin/service/ComponentManager.h>
#include <admin/service/ServiceSocketListener.h>
#include <system/status.h>

#include <util/timeslotcounter.hpp>

#include "TaskProcessor.h"
#include "InfoSmeComponent.h"
#include "util/mirrorfile/mirrorfile.h"

#include <sms/sms.h>
#include <util/config/route/RouteStructures.h>
#include <util/config/route/RouteConfig.h>

#include <util/config/region/RegionsConfig.hpp>
#include <util/config/region/Region.hpp>
#include <util/config/region/RegionFinder.hpp>
#include "infosme/TaskLock.hpp"

#include "version.inc"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;
using namespace smsc::system;

using namespace smsc::admin;
using namespace smsc::admin::service;

using namespace smsc::infosme;
using smsc::util::TimeSlotCounter;
using smsc::core::synchronization::EventMonitor;

const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

static smsc::logger::Logger *logger = 0;

static ServiceSocketListener* adminListener = 0;
static bool bAdminListenerInited = false;

static Event infoSmeWaitEvent;
static Event infoSmeReady;
static int   infoSmeReadyTimeout = 60000;

static Mutex needStopLock;
static Mutex needReconnectLock;

static sig_atomic_t  infoSmeIsStopped    = 0;
static bool  bInfoSmeIsConnected  = false;
static bool  bInfoSmeIsConnecting = false;

static int signaling_ReadFd = -1, signaling_WriteFd = -1;
static int reconnect_ReadFd = -1, reconnect_WriteFd = -1;

static sigset_t blocked_signals, original_signal_mask;

static void setNeedStop()
{
  infoSmeIsStopped = 1;
}

static bool isNeedStop() {
  sigprocmask(SIG_SETMASK, &original_signal_mask, NULL); // unlock all signals and deliver any pending signals
  sigprocmask(SIG_SETMASK, &blocked_signals, NULL); // lock all signals.
  return infoSmeIsStopped == 1;
}

static void setNeedReconnect() {
    MutexGuard gauard(needReconnectLock);
    smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.infosme.InfoSme");
    smsc_log_info(logger, "setNeedReconnect:: Enter it");
    if ( bInfoSmeIsConnected ) {
      bInfoSmeIsConnected = false;
      unsigned char oneByte=0;
      smsc_log_info(log, "setNeedReconnect:: write to reconnect_WriteFd");
      write(reconnect_WriteFd, &oneByte, sizeof(oneByte));
    }
}

static void setConnected() {
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.infosme.InfoSme");
  MutexGuard gauard(needReconnectLock);
  bInfoSmeIsConnected=true;
  smsc_log_info(log, "setConnected:: bInfoSmeIsConnected=true");
}

extern bool isMSISDNAddress(const char* string)
{
    try { Address converted(string); } catch (...) { return false;}
    return true;
}
extern bool convertMSISDNStringToAddress(const char* string, Address& address)
{
    try {
        Address converted(string);
        address = converted;
    } catch (...) {
        return false;
    }
    return true;
};

//static int  unrespondedMessagesSleep = 10;
//static int  unrespondedMessagesMax   = 3;
static int  maxMessagesPerSecond     = 50;

#include "TrafficControl.hpp"

void TrafficControl::incOutgoing()
{
  MutexGuard guard(trafficMonitor);
  outgoing.Inc();
  if (TrafficControl::stopped) return;

  int out = outgoing.Get();

  while (out >= maxMessagesPerSecond) {
    // traffic limit reached
    smsc_log_info(logger, "wait limit (out=%d, max=%d)",
                  out, maxMessagesPerSecond);
    trafficMonitor.wait(1000/maxMessagesPerSecond);
    out = outgoing.Get();
  }
}

void TrafficControl::incIncoming()
{
  MutexGuard guard(trafficMonitor);
  incoming.Inc();
  if (TrafficControl::stopped) return;
  int out = outgoing.Get();// int inc = incoming.Get();
  //int difference = out-inc;
  bool trafficLimitOk = (out < maxMessagesPerSecond);

  if (trafficLimitOk) {
    trafficMonitor.notifyAll();
  }
}

void TrafficControl::stopControl()
{
  MutexGuard guard(trafficMonitor);
  TrafficControl::stopped = true;
  trafficMonitor.notifyAll();
}

void TrafficControl::startControl()
{
  MutexGuard guard(trafficMonitor);
  TrafficControl::stopped = false;
}

EventMonitor         TrafficControl::trafficMonitor;
TimeSlotCounter<int> TrafficControl::incoming(1, 1);
TimeSlotCounter<int> TrafficControl::outgoing(1, 1);
bool                 TrafficControl::stopped = false;

class InfoSmeConfig : public SmeConfig
{
private:

    char *strHost, *strSid, *strPassword, *strSysType, *strOrigAddr;

public:

    InfoSmeConfig(ConfigView* config)
        throw(ConfigException)
            : SmeConfig(), strHost(0), strSid(0), strPassword(0),
                strSysType(0), strOrigAddr(0)
    {
        // Mandatory fields
        strHost = config->getString("host", "SMSC host wasn't defined !");
        host = strHost;
        strSid = config->getString("sid", "InfoSme id wasn't defined !");
        sid = strSid;

        port = config->getInt("port", "SMSC port wasn't defined !");
        timeOut = config->getInt("timeout", "Connect timeout wasn't defined !");

        // Optional fields
        try
        {
            strPassword = config->getString("password",
                                            "InfoSme password wasn't defined !");
            password = strPassword;
        }
        catch (ConfigException& exc) { password = ""; strPassword = 0; }
        try
        {
            strSysType = config->getString("systemType",
                                           "InfoSme system type wasn't defined !");
            systemType = strSysType;
        }
        catch (ConfigException& exc) { systemType = ""; strSysType = 0; }
        try
        {
            strOrigAddr = config->getString("origAddress",
                                            "InfoSme originating address wasn't defined !");
            origAddr = strOrigAddr;
        }
        catch (ConfigException& exc) { origAddr = ""; strOrigAddr = 0; }
    };

    virtual ~InfoSmeConfig()
    {
        if (strHost) delete strHost;
        if (strSid) delete strSid;
        if (strPassword) delete strPassword;
        if (strSysType) delete strSysType;
        if (strOrigAddr) delete strOrigAddr;
    };
};

class InfoSmeMessageSender: public MessageSender
{
private:

    TaskProcessor&  processor;
    SmppSession*    session;
    Mutex           sendLock;

public:

    InfoSmeMessageSender(TaskProcessor& processor, SmppSession* session)
        : MessageSender(), processor(processor), session(session) {};
    virtual ~InfoSmeMessageSender() {
        MutexGuard guard(sendLock);
        session = 0;
    };

    virtual int getSequenceNumber()
    {
        MutexGuard guard(sendLock);
        return (session) ? session->getNextSeq():0;
    }
    virtual bool send(std::string abonent, std::string message, TaskInfo info, int seqNum)
    {
        MutexGuard guard(sendLock);

        if (!session) {
            smsc_log_error(logger, "Smpp session is undefined for MessageSender.");
            return false;
        }
        SmppTransmitter* asyncTransmitter = session->getAsyncTransmitter();
        if (!asyncTransmitter) {
            smsc_log_error(logger, "Smpp transmitter is undefined for MessageSender.");
            return false;
        }

        Address oa, da;
        const char* oaStr = info.address.c_str();
        if (!oaStr || !oaStr[0]) oaStr = processor.getAddress();
        if (!oaStr || !convertMSISDNStringToAddress(oaStr, oa)) {
            smsc_log_error(logger, "Invalid originating address '%s'", oaStr ? oaStr:"-");
            return false;
        }
        const char* daStr = abonent.c_str();
        if (!daStr || !convertMSISDNStringToAddress(daStr, da)) {
            smsc_log_error(logger, "Invalid destination address '%s'", daStr ? daStr:"-");
            return false;
        }

        SMS sms;
        sms.setOriginatingAddress(oa);
        sms.setDestinationAddress(da);
        sms.setArchivationRequested(false);
        sms.setDeliveryReport(1);
        sms.setValidTime( (info.validityDate <= 0 || info.validityPeriod > 0) ?
                           time(NULL)+info.validityPeriod : info.validityDate );

        sms.setIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG,
                           (info.replaceIfPresent) ? 1:0);
        sms.setEServiceType( (info.replaceIfPresent && info.svcType.length()>0) ?
                              info.svcType.c_str():processor.getSvcType() );

        sms.setIntProperty(Tag::SMPP_PROTOCOL_ID, processor.getProtocolId());
        sms.setIntProperty(Tag::SMPP_ESM_CLASS, (info.transactionMode) ? 2:0);
        sms.setIntProperty(Tag::SMPP_PRIORITY, 0);
        sms.setIntProperty(Tag::SMPP_REGISTRED_DELIVERY, 1);

        if(info.flash)
        {
          sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,1);
        }

        const char* out = message.c_str();
        int outLen = message.length();
        char* msgBuf = 0;
        if(hasHighBit(out,outLen)) {
            int msgLen = outLen*2;
            msgBuf = new char[msgLen];
            ConvertMultibyteToUCS2(out, outLen, (short*)msgBuf, msgLen,
                                   CONV_ENCODING_CP1251);
            sms.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::UCS2);
            out = msgBuf; outLen = msgLen;
        } else {
            sms.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::LATIN1);
        }

        try
        {
            if (outLen <= MAX_ALLOWED_MESSAGE_LENGTH) {
                sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, out, outLen);
                sms.setIntProperty(Tag::SMPP_SM_LENGTH, outLen);
            } else {
                sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, out,
                                   (outLen <= MAX_ALLOWED_PAYLOAD_LENGTH) ?
                                    outLen :  MAX_ALLOWED_PAYLOAD_LENGTH);
            }
        }
        catch (...) {
            smsc_log_error(logger, "Something is wrong with message body. Set/Get property failed");
            if (msgBuf) delete msgBuf; msgBuf = 0;
            return false;
        }
        if (msgBuf) delete msgBuf;

        PduSubmitSm sm;
        sm.get_header().set_sequenceNumber(seqNum);
        sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
        fillSmppPduFromSms(&sm, &sms);
        asyncTransmitter->sendPdu(&(sm.get_header()));

        TrafficControl::incOutgoing();
        return true;
    }
};

class InfoSmePduListener: public SmppPduEventListener
{
protected:

    TaskProcessor&          processor;

    SmppTransmitter*    syncTransmitter;
    SmppTransmitter*    asyncTransmitter;

public:

    InfoSmePduListener(TaskProcessor& proc) : SmppPduEventListener(),
        processor(proc), syncTransmitter(0), asyncTransmitter(0) {};
    virtual ~InfoSmePduListener() {}; // ???

    void setSyncTransmitter(SmppTransmitter *transmitter) {
        syncTransmitter = transmitter;
    }
    void setAsyncTransmitter(SmppTransmitter *transmitter) {
        asyncTransmitter = transmitter;
    }

    void processReceipt (SmppHeader *pdu)
    {
        if (!pdu || !asyncTransmitter) return;
        bool bNeedResponce = true;

        SMS sms;
        fetchSmsFromSmppPdu((PduXSm*)pdu, &sms);
        bool isReceipt = (sms.hasIntProperty(Tag::SMPP_ESM_CLASS)) ?
            ((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3C) == 0x4) : false;

        if (isReceipt && ((PduXSm*)pdu)->get_optional().has_receiptedMessageId())
        {
            const char* msgid = ((PduXSm*)pdu)->get_optional().get_receiptedMessageId();
            if (msgid && msgid[0] != '\0')
            {
                bool delivered = false;
                bool retry = false;

                if (sms.hasIntProperty(Tag::SMPP_MSG_STATE))
                {
                    int msgState = sms.getIntProperty(Tag::SMPP_MSG_STATE);
                    switch (msgState)
                    {
                    case SmppMessageState::DELIVERED:
                        delivered = true;
                        break;
                    case SmppMessageState::EXPIRED:
                    case SmppMessageState::DELETED:
                        retry = true;
                        break;
                    case SmppMessageState::ENROUTE:
                    case SmppMessageState::UNKNOWN:
                    case SmppMessageState::ACCEPTED:
                    case SmppMessageState::REJECTED:
                    case SmppMessageState::UNDELIVERABLE:
                        break;
                    default:
                        smsc_log_warn(logger, "Invalid state=%d received in reciept !", msgState);
                        break;
                    }
                }

                bNeedResponce = processor.invokeProcessReceipt(msgid, delivered, retry);
            }
        }

        if (bNeedResponce)
        {
            PduDeliverySmResp smResp;
            smResp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
            smResp.set_messageId("");
            smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
            asyncTransmitter->sendDeliverySmResp(smResp);
        }
    }

    void processResponce(SmppHeader *pdu)
    {
        if (!pdu) return;

        int seqNum = pdu->get_sequenceNumber();
        int status = pdu->get_commandStatus();

        bool accepted  = (status == Status::OK);
        bool retry     = (!Status::isErrorPermanent(status));

        bool immediate = (status == Status::MSGQFUL   ||
                          status == Status::THROTTLED ||
        status == Status::LICENSELIMITREJECT
        /*|| commented for Lugovoj's request
                                                        status == Status::SUBSCRBUSYMT*/);

        bool trafficst = (status == Status::MSGQFUL                   ||
                          status == Status::THROTTLED                 ||
        status == Status::LICENSELIMITREJECT        ||
                          status == Status::MAP_RESOURCE_LIMITATION   ||
                          status == Status::MAP_NO_RESPONSE_FROM_PEER ||
                          status == Status::SMENOTCONNECTED           ||
                          status == Status::SYSFAILURE);

        const char* msgid = ((PduXSmResp*)pdu)->get_messageId();
        std::string msgId = "";
        if (!msgid || msgid[0] == '\0') accepted = false;
        else msgId = msgid;

        if (!accepted)
            smsc_log_info(logger, "SMS #%s seqNum=%d wasn't accepted, errcode=%d",
                          msgId.c_str(), seqNum, status);

        processor.invokeProcessResponce(seqNum, accepted, retry, immediate, msgId, trafficst);
    }

    void handleEvent(SmppHeader *pdu)
    {
        if (bInfoSmeIsConnecting) {
            infoSmeReady.Wait(infoSmeReadyTimeout);
            if (bInfoSmeIsConnecting) {
                disposePdu(pdu);
                return;
            }
        }

        switch (pdu->get_commandId())
        {
        case SmppCommandSet::DELIVERY_SM:
            processReceipt(pdu);
            break;
        case SmppCommandSet::SUBMIT_SM_RESP:
            processResponce(pdu);
            break;
        case SmppCommandSet::ENQUIRE_LINK: case SmppCommandSet::ENQUIRE_LINK_RESP:
            break;
        default:
            smsc_log_debug(logger, "Received unsupported Pdu !");
            break;
        }

        disposePdu(pdu);
    }

    void handleError(int errorCode)
    {
        smsc_log_error(logger, "Transport error handled! Code is: %d", errorCode);
        setNeedReconnect();
    }
};

extern "C" static void appSignalHandler(int sig)
{
  if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT) {
    if ( signaling_WriteFd > -1 ) close (signaling_WriteFd);
    setNeedStop();
  }
}

static smsc::util::config::region::RegionsConfig *regionsConfig;

extern "C" static void atExitHandler(void)
{
  delete regionsConfig;
  smsc::util::xml::TerminateXerces();
  smsc::logger::Logger::Shutdown();
}

static void
doRegionsInitilization(const char* regions_xml_file, const char* route_xml_file)
{
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.infosme.InfoSme");

  regionsConfig = new smsc::util::config::region::RegionsConfig(regions_xml_file);

  smsc::util::config::region::RegionsConfig::status st = regionsConfig->load();
  if ( st == smsc::util::config::region::RegionsConfig::success )
    smsc_log_info(log, "config file %s has been loaded successful", regions_xml_file);
  else
    throw smsc::util::config::ConfigException("can't load config file %s", regions_xml_file);

  smsc::util::config::route::RouteConfig routeConfig;
  if ( routeConfig.load(route_xml_file) == smsc::util::config::route::RouteConfig::success )
    smsc_log_info(log, "config file %s has been loaded successful", route_xml_file);
  else
    throw smsc::util::config::ConfigException("can't load config file %s", route_xml_file);

  smsc::util::config::region::Region* region;
  smsc::util::config::region::RegionsConfig::RegionsIterator regsIter = regionsConfig->getIterator();

  while (regsIter.fetchNext(region) == smsc::util::config::region::RegionsConfig::success) {
    region->expandSubjectRefs(routeConfig);
    smsc::util::config::region::Region::MasksIterator maskIter = region->getMasksIterator();
    std::string addressMask;
    while(maskIter.fetchNext(addressMask)) {
      smsc::util::config::region::RegionFinder::getInstance().registerAddressMask(addressMask, region);
    }
  }

  smsc::util::config::region::RegionFinder::getInstance().registerDefaultRegion(&(regionsConfig->getDefaultRegion()));
}

int main(int argc, char** argv)
{
  try {
    if ( argc > 1 && !strcmp(argv[1], "-v") ) {
      printf("%s\n", getStrVersion());
      return 0;
    }

    int fds[2];
    pipe(fds);
    signaling_ReadFd = fds[0]; signaling_WriteFd = fds[1]; 
    pipe(fds);
    reconnect_ReadFd = fds[0]; reconnect_WriteFd = fds[1];

    int resultCode = 0;

    Logger::Init();
    logger = Logger::getInstance("smsc.infosme.InfoSme");

    std::auto_ptr<ServiceSocketListener> adml(new ServiceSocketListener());
    adminListener = adml.get();

    atexit(atExitHandler);

    sigset_t set;
    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, &original_signal_mask);
    sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
    sigset(SIGINT, appSignalHandler);
    sigset(SIGPIPE, SIG_IGN);

    try
    {
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);

        ConfigView tpConfig(manager, "InfoSme");
        maxMessagesPerSecond++;

        try { maxMessagesPerSecond = tpConfig.getInt("maxMessagesPerSecond"); } catch (...) {};
        if (maxMessagesPerSecond <= 0) {
            maxMessagesPerSecond = 50;
            smsc_log_warn(logger, "Parameter 'maxMessagesPerSecond' value is invalid. Using default %d",
                          maxMessagesPerSecond);
        }
        if (maxMessagesPerSecond > 100) {
            smsc_log_warn(logger, "Parameter 'maxMessagesPerSecond' value '%d' is too big. "
                          "The preffered max value is 100", maxMessagesPerSecond);
        }

        const char* route_xml_file = tpConfig.getString("route_config_filename");
        const char* regions_xml_file = tpConfig.getString("regions_config_filename");

        {
          std::string fn=tpConfig.getString("storeLocation");
          if(fn.length() && *fn.rbegin()!='/')
          {
            fn+='/';
          }
          fn+="taskslock.bin";
          TaskLock::Init(fn.c_str());
        }

        doRegionsInitilization(regions_xml_file, route_xml_file);

        sigfillset(&blocked_signals);
        sigdelset(&blocked_signals, SIGKILL);
        sigdelset(&blocked_signals, SIGALRM);
        sigdelset(&blocked_signals, SIGSEGV); sigdelset(&blocked_signals, SIGBUS);
        sigdelset(&blocked_signals, SIGFPE); sigdelset(&blocked_signals, SIGILL);

        sigprocmask(SIG_SETMASK, &blocked_signals, &original_signal_mask);

        ConfigView adminConfig(manager, "InfoSme.Admin");
        adminListener->init(adminConfig.getString("host"), adminConfig.getInt("port"));
        bAdminListenerInited = true;

        TaskProcessor processor(&tpConfig);

        InfoSmeComponent admin(processor);
        ComponentManager::registerComponent(&admin);
        adminListener->Start();

        ConfigView smscConfig(manager, "InfoSme.SMSC");
        InfoSmeConfig cfg(&smscConfig);

        bool haveSysError=false;
        while (!isNeedStop() && !haveSysError)
        {
          // after call to isNeedStop() was completed all signals is locked.
          // any thread being started from this point has signal mask with all signals locked 
            InfoSmePduListener      listener(processor);
            SmppSession             session(cfg, &listener);
            InfoSmeMessageSender    sender(processor, &session);

            smsc_log_info(logger, "Connecting to SMSC ... ");
            try
            {
                listener.setSyncTransmitter(session.getSyncTransmitter());
                listener.setAsyncTransmitter(session.getAsyncTransmitter());

                bInfoSmeIsConnecting = true;
                infoSmeReady.Wait(0);
                TrafficControl::startControl();
                session.connect();
                processor.assignMessageSender(&sender);
                processor.Start();
                bInfoSmeIsConnecting = false;
                infoSmeReady.Signal();
                setConnected();
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what();
                smsc_log_error(logger, "Connect to SMSC failed. Cause: %s", (msg) ? msg:"unknown");
                bInfoSmeIsConnecting = false;
                if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw;
                sleep(cfg.timeOut);
                session.close();
                continue;
            }
            smsc_log_info(logger, "Connected.");
            sigprocmask(SIG_SETMASK, &original_signal_mask, NULL); // unlock all signals and deliver any pending signals
            int st;
            fd_set rFdSet;
            FD_ZERO(&rFdSet);
            FD_SET(signaling_ReadFd, &rFdSet); FD_SET(reconnect_ReadFd, &rFdSet); 
            if ( (st=::select(std::max(signaling_ReadFd, reconnect_ReadFd)+1, &rFdSet, NULL, NULL, NULL)) > 0 ) {
              smsc_log_info(logger, "select return %d", st);
              if ( FD_ISSET(reconnect_ReadFd, &rFdSet) ) {
                unsigned char oneByte;
                if ( read(reconnect_ReadFd, &oneByte, sizeof(oneByte)) != sizeof(oneByte) )
                  haveSysError = true;
              }
            } else if ( st == -1 ) {
              smsc_log_info(logger, "select return -1: errno=%d", errno);
              if ( errno != EINTR )
                haveSysError = true;
            }

            smsc_log_info(logger, "Disconnecting from SMSC ...");
            TrafficControl::stopControl();
            processor.Stop();
            processor.assignMessageSender(0);
            session.close();
            // sleep for 2 secs.
            struct timeval sleepTimeout;
            sleepTimeout.tv_sec = 2;
            sleepTimeout.tv_usec = 0;
            ::select(0, NULL, NULL, NULL, &sleepTimeout);
        }
    }
    catch (SmppConnectException& exc)
    {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            smsc_log_error(logger, "Failed to bind InfoSme. Exiting.");
        resultCode = -1;
    }
    catch (ConfigException& exc)
    {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc)
    {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (exception& exc)
    {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    }
    catch (...)
    {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -5;
    }

    if (bAdminListenerInited)
    {
        adminListener->shutdown();
        adminListener->WaitFor();
    }

    DataSourceLoader::unload();
    return resultCode;
  } catch (std::exception& ex) {
    fprintf(stderr, "catch unexpected exception[%s]\n",ex.what());
    return -1;
  }
}
