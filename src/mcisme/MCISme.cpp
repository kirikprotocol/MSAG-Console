#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Array.hpp>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/recoder/recode_dll.h>
#include <util/smstext.h>

#include <pthread.h>
#include <signal.h>

#include <db/DataSourceLoader.h>
#include <core/synchronization/EventMonitor.hpp>

#include <system/smscsignalhandlers.h>
#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <util/xml/init.h>

#include <admin/service/Component.h>
#include <admin/service/ComponentManager.h>
#include <admin/service/ServiceSocketListener.h>
#include <system/smscsignalhandlers.h>
#include <system/status.h>

#include <util/timeslotcounter.hpp>

#include "TaskProcessor.h"
#include "MCISmeComponent.h"

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

using namespace smsc::mcisme;
using smsc::util::TimeSlotCounter;
using smsc::core::synchronization::EventMonitor;

const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

static smsc::logger::Logger *logger = 0;

static TaskProcessor*         taskProcessor = 0;
static ServiceSocketListener* adminListener = 0; 
static bool bAdminListenerInited = false;

static Event mciSmeWaitEvent;
static Event mciSmeReady;
static int   mciSmeReadyTimeout = 60000;

static Mutex needStopLock;
static Mutex needReconnectLock;

static bool  bMCISmeIsStopped    = false;
static bool  bMCISmeIsConnected  = false;
static bool  bMCISmeIsConnecting = false;

static void setNeedStop(bool stop=true) {
    MutexGuard gauard(needStopLock);
    bMCISmeIsStopped = stop;
    if (stop && taskProcessor) taskProcessor->Stop();
}
static bool isNeedStop() {
    MutexGuard gauard(needStopLock);
    return bMCISmeIsStopped;
}

static void setNeedReconnect(bool reconnect=true) {
    MutexGuard gauard(needReconnectLock);
    bMCISmeIsConnected = !reconnect;
    if (reconnect && taskProcessor) taskProcessor->Stop();
}
static bool isNeedReconnect() {
    MutexGuard gauard(needReconnectLock);
    return !bMCISmeIsConnected;
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

static int   unrespondedMessagesSleep = 10;
static int   unrespondedMessagesMax   = 3;

class TrafficControl
{
    static EventMonitor   trafficMonitor; 
    static TimeSlotCounter<int> incoming;
    static TimeSlotCounter<int> outgoing;
    static bool                 stopped; 

public:

    static void incOutgoing()
    {
        MutexGuard guard(trafficMonitor);
        outgoing.Inc(); 
        if (TrafficControl::stopped) return;

        int out = outgoing.Get();
        int inc = incoming.Get();
        int difference = out-inc;
        
        if (difference >= unrespondedMessagesMax) {
            smsc_log_debug(logger, "wait %d/%d (o=%d, i=%d)", 
                           difference, difference*unrespondedMessagesSleep, out, inc);
            trafficMonitor.wait(difference*unrespondedMessagesSleep);
        } else {
            smsc_log_debug(logger, "nowait");
        }
    };
    static bool getTraffic(int& in, int& out)
    {
        MutexGuard guard(trafficMonitor);
        in = incoming.Get()/10; out = outgoing.Get()/10; 
        return (!TrafficControl::stopped);
    };
    static void incIncoming()
    {
        MutexGuard guard(trafficMonitor);
        incoming.Inc();
        if (TrafficControl::stopped) return;
        if ((outgoing.Get()-incoming.Get()) < unrespondedMessagesMax) {
            trafficMonitor.notifyAll();
        }
    };
    static void stopControl()
    {
        MutexGuard guard(trafficMonitor);
        TrafficControl::stopped = true;
        trafficMonitor.notifyAll();
    };
    static void startControl()
    {
        MutexGuard guard(trafficMonitor);
        TrafficControl::stopped = false;
    };
};

EventMonitor         TrafficControl::trafficMonitor;
TimeSlotCounter<int> TrafficControl::incoming(10, 10);
TimeSlotCounter<int> TrafficControl::outgoing(10, 10);
bool                 TrafficControl::stopped = false;

class MCISmeConfig : public SmeConfig
{
private:
    
    char *strHost, *strSid, *strPassword, *strSysType, *strOrigAddr;

public:
    
    MCISmeConfig(ConfigView* config)
        throw(ConfigException)
            : SmeConfig(), strHost(0), strSid(0), strPassword(0), 
                strSysType(0), strOrigAddr(0)
    {
        // Mandatory fields
        strHost = config->getString("host", "SMSC host wasn't defined !");
        host = strHost;
        strSid = config->getString("sid", "MCISme id wasn't defined !");
        sid = strSid;
        
        port = config->getInt("port", "SMSC port wasn't defined !");
        timeOut = config->getInt("timeout", "Connect timeout wasn't defined !");
        
        // Optional fields
        try {
            strPassword = config->getString("password", "MCISme password wasn't defined !");
            password = strPassword;
        } catch (ConfigException& exc) { password = ""; strPassword = 0; }
        try {
            strSysType = config->getString("systemType", "MCISme system type wasn't defined !");
            systemType = strSysType;
        } catch (ConfigException& exc) { systemType = ""; strSysType = 0; }
        try {
            strOrigAddr = config->getString("origAddress", "MCISme originating address wasn't defined !");
            origAddr = strOrigAddr;
        } catch (ConfigException& exc) { origAddr = ""; strOrigAddr = 0; }
    };

    virtual ~MCISmeConfig()
    {
        if (strHost) delete strHost;
        if (strSid) delete strSid;
        if (strPassword) delete strPassword;
        if (strSysType) delete strSysType;
        if (strOrigAddr) delete strOrigAddr;
    };
};

class MCISmeMessageSender: public MessageSender
{
private:
    
    TaskProcessor&  processor;
    SmppSession*    session;
    Mutex           sendLock;

public:
    
    MCISmeMessageSender(TaskProcessor& processor, SmppSession* session) 
        : MessageSender(), processor(processor), session(session) {};
    virtual ~MCISmeMessageSender() {
        MutexGuard guard(sendLock);
        session = 0;
    };

    virtual int getSequenceNumber()
    {
        MutexGuard guard(sendLock);
        return (session) ? session->getNextSeq():0;
    }
    virtual bool send(int seqNumber, const Message& message)
    {
        MutexGuard guard(sendLock);
        
        if (!session) {
            smsc_log_error(logger, "Smpp session is undefined for MessageSender");
            return false;
        }
        SmppTransmitter* asyncTransmitter = session->getAsyncTransmitter();
        if (!asyncTransmitter) {
            smsc_log_error(logger, "Smpp transmitter is undefined for MessageSender");
            return false;
        }
        
        smsc_log_debug(logger, "Sender: %s%s message #%lld seqNum=%ld for %s", (message.cancel) ? "Canceling ":"Sending",
                       (message.cancel) ? message.smsc_id.c_str():"", message.id, seqNumber, message.abonent.c_str());

        Address oa, da;
        const char* oaStr = processor.getAddress(); // TODO: caller address for notifications
        if (!oaStr || !convertMSISDNStringToAddress(oaStr, oa)) {
            smsc_log_error(logger, "Invalid originating address '%s'", oaStr ? oaStr:"-");
            return false;
        }
        const char* daStr = message.abonent.c_str();
        if (!daStr || !convertMSISDNStringToAddress(daStr, da)) {
            smsc_log_error(logger, "Invalid destination address '%s'", daStr ? daStr:"-");
            return false;
        }
        
        if (message.cancel)
        {
            //smsc_log_debug(logger, "CANCEL SMSC_ID=%s da=%s", message.smsc_id.c_str(), daStr);
            PduCancelSm sm;

            sm.set_messageId(message.smsc_id.c_str());
            sm.get_source().set_typeOfNumber(oa.type);
            sm.get_source().set_numberingPlan(oa.plan);
            sm.get_source().set_value(oa.value);
            sm.get_dest()  .set_typeOfNumber(da.type);
            sm.get_dest()  .set_numberingPlan(da.plan);
            sm.get_dest()  .set_value(da.value);
            
            sm.get_header().set_commandLength(sm.size());
            sm.get_header().set_commandId(SmppCommandSet::CANCEL_SM);
            sm.get_header().set_commandStatus(0);
            sm.get_header().set_sequenceNumber(seqNumber);
            
            asyncTransmitter->sendPdu(&(sm.get_header()));
        }
        else
        {
            int outLen = message.message.length();
            int msgLen = outLen*2;
            std::auto_ptr<char> msgBufGuard(new char[msgLen+2]);
            char* msgBuf = msgBufGuard.get();
            ConvertMultibyteToUCS2(message.message.c_str(), outLen, 
                                   (short *)msgBuf, msgLen, CONV_ENCODING_CP1251);
            /*smsc_log_debug(logger, "1251 >> UCS2. Message %p Len:%d/%d",
                           message.message.c_str(), outLen, msgLen);*/
            short* msgBufConv = (short *)msgBuf;
            for (int p=0; p<outLen; p++) msgBufConv[p] = htons(msgBufConv[p]);

            time_t smsValidityDate = time(NULL) + processor.getDaysValid()*3600*24;
            
            EService svcType;
            strncpy(svcType, processor.getSvcType(), MAX_ESERVICE_TYPE_LENGTH);
            svcType[MAX_ESERVICE_TYPE_LENGTH]='\0';

            PduSubmitSm  sm;
            
            sm.get_message().set_serviceType(svcType);
            sm.get_message().get_source().set_typeOfNumber(oa.type);
            sm.get_message().get_source().set_numberingPlan(oa.plan);
            sm.get_message().get_source().set_value(oa.value);
            sm.get_message().get_dest()  .set_typeOfNumber(da.type);
            sm.get_message().get_dest()  .set_numberingPlan(da.plan);
            sm.get_message().get_dest()  .set_value(da.value);
            sm.get_message().set_esmClass(0); // default mode (not transactional)
            sm.get_message().set_protocolId(processor.getProtocolId());
            sm.get_message().set_priorityFlag(0);
            char timeBuffer[64];
            cTime2SmppTime(smsValidityDate, timeBuffer);
            sm.get_message().set_validityPeriod(timeBuffer);
            cTime2SmppTime(time(NULL)+60*message.attempts, timeBuffer);
            sm.get_message().set_scheduleDeliveryTime(timeBuffer);
            sm.get_message().set_registredDelivery((message.notification) ? 0:1);
            sm.get_message().set_replaceIfPresentFlag(0);

            sm.get_message().set_smDefaultMsgId(0);
            sm.get_message().set_dataCoding(DataCoding::UCS2);
            
            if (msgLen > MAX_ALLOWED_MESSAGE_LENGTH)
            {
                if (msgLen > MAX_ALLOWED_PAYLOAD_LENGTH) {
                    smsc_log_error(logger, "Message #%lld is too large to send (%d bytes)", message.id, msgLen);
                    return false;
                }
                sm.get_optional().set_payloadType(0);
                sm.get_optional().set_messagePayload(msgBuf, msgLen);
            } 
            else sm.get_message().set_shortMessage(msgBuf, msgLen);
            
            sm.get_header().set_commandLength(sm.size(false));
            sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
            sm.get_header().set_commandStatus(0);
            sm.get_header().set_sequenceNumber(seqNumber);
            
            asyncTransmitter->sendPdu(&(sm.get_header()));
            TrafficControl::incOutgoing();
        }
        return true;
    }
};

class MCISmePduListener: public SmppPduEventListener
{
protected:
    
    TaskProcessor&      processor;
    
    SmppTransmitter*    syncTransmitter;
    SmppTransmitter*    asyncTransmitter;

public:
    
    MCISmePduListener(TaskProcessor& proc) : SmppPduEventListener(),
        processor(proc), syncTransmitter(0), asyncTransmitter(0) {};
    virtual ~MCISmePduListener() {};

    void setSyncTransmitter(SmppTransmitter *transmitter) {
        syncTransmitter = transmitter;
    }
    void setAsyncTransmitter(SmppTransmitter *transmitter) {
        asyncTransmitter = transmitter;
    }
    
    void processResponce(SmppHeader *pdu, bool cancel)
    {
        if (!pdu) return;
        
        int seqNum = pdu->get_sequenceNumber();
        int status = pdu->get_commandStatus();
        
        bool accepted       = (status == Status::OK);
        bool cancel_failed  = (cancel && (status == Status::CANCELFAIL));
        bool retry          = (cancel_failed) ? false:(!accepted && !Status::isErrorPermanent(status));
        
        bool immediate      = (status == Status::MSGQFUL   ||
                               status == Status::THROTTLED ||
                               status == Status::SUBSCRBUSYMT);

        bool trafficst      = (status == Status::MSGQFUL                   ||
                               status == Status::THROTTLED                 ||
                               status == Status::MAP_RESOURCE_LIMITATION   ||
                               status == Status::MAP_NO_RESPONSE_FROM_PEER ||
                               status == Status::SMENOTCONNECTED           ||
                               status == Status::SYSFAILURE);

        if (!trafficst && !cancel) TrafficControl::incIncoming();

        std::string msgId = ""; 
        if (!cancel) {
            const char* msgid = ((PduXSmResp*)pdu)->get_messageId();
            if (!msgid || msgid[0] == '\0') accepted = false;
            else msgId = msgid;
        }

        processor.invokeProcessResponce(seqNum, accepted, retry, immediate,
                                        cancel, cancel_failed, msgId);
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
                bool expired   = false;
                bool deleted   = false;
                
                if (sms.hasIntProperty(Tag::SMPP_MSG_STATE))
                {
                    int msgState = sms.getIntProperty(Tag::SMPP_MSG_STATE);
                    switch (msgState)
                    {
                    case SmppMessageState::DELIVERED:
                        delivered = true;
                        break;
                    case SmppMessageState::EXPIRED:
                        expired = true;
                        break;
                    case SmppMessageState::DELETED:
                        deleted = true;
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
                
                bNeedResponce = processor.invokeProcessReceipt(msgid, delivered, expired, deleted);
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

    void handleEvent(SmppHeader *pdu)
    {
        if (bMCISmeIsConnecting) {
            mciSmeReady.Wait(mciSmeReadyTimeout);
            if (bMCISmeIsConnecting) {
                disposePdu(pdu);
                return;
            }
        }

        switch (pdu->get_commandId())
        {
        case SmppCommandSet::DELIVERY_SM:
            processReceipt(pdu);
            break;
        case SmppCommandSet::CANCEL_SM_RESP:
            processResponce(pdu, true);
            break;
        case SmppCommandSet::SUBMIT_SM_RESP:
            processResponce(pdu, false);
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
        setNeedReconnect(true);
    }
};

extern "C" void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT)
    {
        smsc_log_info(logger, "Stopping ...");
        if (bAdminListenerInited) adminListener->shutdown();
        setNeedStop(true);
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

class MCISmeAdministrator : public MCISmeAdmin
{
private:
    
    AdminInterface& admin;

public:
    
    MCISmeAdministrator(AdminInterface& _admin) : MCISmeAdmin(), admin(_admin) {};
    virtual ~MCISmeAdministrator() {};

    virtual void flushStatistics() {
        admin.flushStatistics();
    };
    virtual EventsStat  getStatistics() {
        return admin.getStatistics();
    };
    virtual RuntimeStat getRuntimeStatistics()
    {
        int inSpeed = 0; int outSpeed = 0;
        int activeTasks  = admin.getActiveTasksCount();
        int inQueueSize  = admin.getInQueueSize();
        int outQueueSize = admin.getOutQueueSize();
        TrafficControl::getTraffic(inSpeed, outSpeed);
        return RuntimeStat(activeTasks, inQueueSize, outQueueSize, inSpeed, outSpeed);
    };
};

int main(void)
{
    Logger::Init();
    logger = Logger::getInstance("smsc.mcisme.MCISme");
    
    atexit(atExitHandler);
    clearSignalMask();

    shutdownThread.Start();

    std::auto_ptr<ServiceSocketListener> adml(new ServiceSocketListener());
    adminListener = adml.get();
    
    int resultCode = 0;
    try 
    {
        smsc_log_info(logger, getStrVersion());

        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);
        
        ConfigView tpConfig(manager, "MCISme");
        try { unrespondedMessagesMax = tpConfig.getInt("unrespondedMessagesMax"); } catch (...) {};
        if (unrespondedMessagesMax <= 0) {
            unrespondedMessagesMax = 1;
            smsc_log_warn(logger, "Parameter 'unrespondedMessagesMax' value is invalid. Using default %d",
                          unrespondedMessagesMax);
        }
        if (unrespondedMessagesMax > 500) {
            smsc_log_warn(logger, "Parameter 'unrespondedMessagesMax' value '%d' is too big. "
                          "The preffered max value is 500", unrespondedMessagesMax);
        }
        try { unrespondedMessagesSleep = tpConfig.getInt("unrespondedMessagesSleep"); } catch (...) {};
        if (unrespondedMessagesSleep <= 0) {
            unrespondedMessagesSleep = 100;
            smsc_log_warn(logger, "'unrespondedMessagesSleep' value is invalid. Using default %dms",
                          unrespondedMessagesSleep);
        }
        if (unrespondedMessagesSleep > 500) {
            smsc_log_warn(logger, "Parameter 'unrespondedMessagesSleep' value '%d' is too big. "
                          "The preffered max value is 500ms", unrespondedMessagesSleep);
        }
        
        ConfigView adminConfig(manager, "MCISme.Admin");
        adminListener->init(adminConfig.getString("host"), adminConfig.getInt("port"));               
        bAdminListenerInited = true;
        
        TaskProcessor processor(&tpConfig);
        taskProcessor = &processor;
        
        MCISmeAdministrator adminHandler(processor);
        MCISmeComponent admin(adminHandler);                   
        ComponentManager::registerComponent(&admin); 
        adminListener->Start();
        
        ConfigView smscConfig(manager, "MCISme.SMSC");
        MCISmeConfig cfg(&smscConfig);

        while (!isNeedStop())
        {
            MCISmePduListener       listener(processor);
            SmppSession             session(cfg, &listener);
            MCISmeMessageSender     sender(processor, &session);
            
            smsc_log_info(logger, "Connecting to SMSC ... ");
            try
            {
                listener.setSyncTransmitter(session.getSyncTransmitter());
                listener.setAsyncTransmitter(session.getAsyncTransmitter());
                
                bMCISmeIsConnecting = true;
                mciSmeReady.Wait(0);
                TrafficControl::startControl();
                setNeedReconnect(false);
                session.connect();
                processor.assignMessageSender(&sender);
                processor.Start();
                bMCISmeIsConnecting = false;
                mciSmeReady.Signal();
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what(); 
                smsc_log_error(logger, "Connect to SMSC failed. Cause: %s", (msg) ? msg:"unknown");
                bMCISmeIsConnecting = false;
                setNeedReconnect(true);
                // ??? if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw exc;
                sleep(cfg.timeOut);
                session.close();
                continue;
            }
            smsc_log_info(logger, "Connected.");
            
            smsc_log_info(logger, "Running messages send loop...");
            //setShutdownHandler();
            processor.Run();
            clearSignalMask();
            smsc_log_info(logger, "Message send loop exited");
            
            smsc_log_info(logger, "Disconnecting from SMSC ...");
            TrafficControl::stopControl();
            processor.assignMessageSender(0);
            session.close();
        }
        taskProcessor = 0;
    }
    catch (SmppConnectException& exc) {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            smsc_log_error(logger, "Failed to bind MCISme. Exiting");
        resultCode = -1;
    }
    catch (ConfigException& exc) {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "Top level Exception: %s Exiting", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) {
        smsc_log_error(logger, "Top level exception: %s Exiting", exc.what());
        resultCode = -4;
    }
    catch (...) {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting");
        resultCode = -5;
    }
    
    if (bAdminListenerInited)
    {
        adminListener->shutdown();
        adminListener->WaitFor();
    }

    DataSourceLoader::unload();
    //smsc_log_debug(logger, "Exited !!!");
    shutdownThread.Stop();
    return resultCode;
}
