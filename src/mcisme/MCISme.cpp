#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Array.hpp>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/recoder/recode_dll.h>
#include <util/smstext.h>

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
#include <system/status.h>

#include <util/timeslotcounter.hpp>

#include "TaskProcessor.h"
//#include "MCISmeComponent.h"

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
    if (stop) mciSmeWaitEvent.Signal(); // TODO: replace by processor EventMonitor
}
static bool isNeedStop() {
    MutexGuard gauard(needStopLock);
    return bMCISmeIsStopped;
}

static void setNeedReconnect(bool reconnect=true) {
    MutexGuard gauard(needReconnectLock);
    bMCISmeIsConnected = !reconnect;
    if (reconnect) mciSmeWaitEvent.Signal();
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
    virtual bool send(const Message& message)
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
        
        smsc_log_debug(logger, "Sending message '%s' for %s ...", 
                       message.message.c_str(), message.abonent.c_str());

        // TODO: implement message sending 

        TrafficControl::incOutgoing();
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
    
    void processReceipt (SmppHeader *pdu)
    {
        if (!pdu || !asyncTransmitter) return;
        bool bNeedResponce = true;

        // TODO: implement receipt processing

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

        // TODO: implement responce processing
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
        case SmppCommandSet::SUBMIT_SM_RESP:
            processResponce(pdu);
            break;
        
        // TODO: Add replace responce processing

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

extern "C" static void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT)
    {
        smsc_log_info(logger, "Stopping ...");
        if (bAdminListenerInited) adminListener->shutdown();
        setNeedStop(true);
    }
}
extern "C" static void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
    smsc::logger::Logger::Shutdown();
}

int main(void)
{
    int resultCode = 0;

    Logger::Init();
    logger = Logger::getInstance("smsc.mcisme.MCISme");
    
    std::auto_ptr<ServiceSocketListener> adml(new ServiceSocketListener());
    adminListener = adml.get();
    
    sigset_t set, old;
    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, &old);
    sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);

    atexit(atExitHandler);

    try 
    {
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
        
        /*
        MCISmeComponent admin(processor);                   
        ComponentManager::registerComponent(&admin); 
        adminListener->Start();
        */
        
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
                if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw exc;
                sleep(cfg.timeOut);
                session.close();
                continue;
            }
            smsc_log_info(logger, "Connected.");
            
            sigemptyset(&set);
            sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
            sigprocmask(SIG_SETMASK, &set, &old);
            
            mciSmeWaitEvent.Wait(0);
            while (!isNeedStop() && !isNeedReconnect())
            {
                // TODO: processor.Run();
                mciSmeWaitEvent.Wait(100);
            }
            smsc_log_info(logger, "Disconnecting from SMSC ...");
            TrafficControl::stopControl();
            processor.Stop();
            processor.assignMessageSender(0);
            session.close();
        }
    }
    catch (SmppConnectException& exc) {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            smsc_log_error(logger, "Failed to bind MCISme. Exiting.");
        resultCode = -1;
    }
    catch (ConfigException& exc) {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    }
    catch (...) {
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
}
