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

const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

static int   unrespondedMessagesSleep = 1000;
static int   unrespondedMessagesMax   = 100;
static TimeSlotCounter<int> outgoingTraffic(1, 10);
static TimeSlotCounter<int> incomingTraffic(1, 10);
static Event trafficControlEvent; 
static Mutex trafficControlLock;

static smsc::logger::Logger *logger = 0;

static std::auto_ptr<smsc::admin::service::ServiceSocketListener> adminListener; 
static bool bAdminListenerInited = false;

static Event infoSmeWaitEvent;
static Event infoSmeReady;
static int   infoSmeReadyTimeout = 60000;

static Mutex needStopLock;
static Mutex needReconnectLock;

static bool  bInfoSmeIsStopped    = false;
static bool  bInfoSmeIsConnected  = false;
static bool  bInfoSmeIsConnecting = false;

static void setNeedStop(bool stop=true) {
    MutexGuard gauard(needStopLock);
    bInfoSmeIsStopped = stop;
    if (stop) infoSmeWaitEvent.Signal();
}
static bool isNeedStop() {
    MutexGuard gauard(needStopLock);
    return bInfoSmeIsStopped;
}

static void setNeedReconnect(bool reconnect=true) {
    MutexGuard gauard(needReconnectLock);
    bInfoSmeIsConnected = !reconnect;
    if (reconnect) infoSmeWaitEvent.Signal();
}
static bool isNeedReconnect() {
    MutexGuard gauard(needReconnectLock);
    return !bInfoSmeIsConnected;
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

    void waitUnrespondedMessages()
    {
        int difference = 0;
        {
            MutexGuard guard(trafficControlLock);
            outgoingTraffic.Inc();
            difference = outgoingTraffic.Get() - incomingTraffic.Get();
        }
        if (difference >= unrespondedMessagesMax) {
            // slow down sending after 10% limit reached
            trafficControlEvent.Wait(difference*unrespondedMessagesSleep);
        }
    }

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
        const char* oaStr = processor.getAddress();
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
        
        waitUnrespondedMessages();
        return true;
    }
};

class InfoSmePduListener: public SmppPduEventListener
{
protected:
    
    TaskProcessor&          processor;
    
    SmppTransmitter*    syncTransmitter;
    SmppTransmitter*    asyncTransmitter;

    void acceptMessage()
    {
        MutexGuard guard(trafficControlLock);
        incomingTraffic.Inc();
        if ((outgoingTraffic.Get()-incomingTraffic.Get()) < unrespondedMessagesMax) {
            trafficControlEvent.Signal();
        }
    }

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
        
        const int SMSC_SMPP_INT_ENROUTE_STATE       = 1;
        const int SMSC_SMPP_INT_DELIVERED_STATE     = 2;
        const int SMSC_SMPP_INT_EXPIRED_STATE       = 3;
        const int SMSC_SMPP_INT_DELETED_STATE       = 4;
        const int SMSC_SMPP_INT_UNDELIVERABLE_STATE = 5;
        const int SMSC_SMPP_INT_ACCEPTED_STATE      = 6;
        const int SMSC_SMPP_INT_UNKNOWN_STATE       = 7;
        const int SMSC_SMPP_INT_REJECTED_STATE      = 8;

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
                    case SMSC_SMPP_INT_DELIVERED_STATE:
                        delivered = true;
                        break;
                    case SMSC_SMPP_INT_EXPIRED_STATE:
                    case SMSC_SMPP_INT_DELETED_STATE:
                        retry = true;
                        break;
                    case SMSC_SMPP_INT_ENROUTE_STATE:
                    case SMSC_SMPP_INT_ACCEPTED_STATE:
                    case SMSC_SMPP_INT_UNKNOWN_STATE:
                    case SMSC_SMPP_INT_REJECTED_STATE:
                    case SMSC_SMPP_INT_UNDELIVERABLE_STATE:
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
        bool accepted =  (status == Status::OK);
        bool retry    =  (status == Status::SYSERR                      ||  // 8     08h
                          status == Status::MSGQFUL                     ||  // 20    14h
                          status == Status::SUBMITFAIL                  ||  // 69    45h
                          status == Status::THROTTLED                   ||  // 88    58h
                          status == Status::RX_T_APPN                   ||  // 100   64h  
                          status == Status::UNKNOWNERR                  ||  // 255   FFh  
                          status == Status::DELIVERYTIMEDOUT            ||  // 1027  403h
                          status == Status::SMENOTCONNECTED             ||  // 1028  404h
                          status == Status::MAP_RESOURCE_LIMITATION     ||  // 1139  473h
                          status == Status::MAP_NO_RESPONSE_FROM_PEER   ||  // 1143  477h
                          status == Status::ABSENTSUBSCR                ||  // 1179  49bh
                          status == Status::SUBSCRBUSYMT                ||  // 1183  49fh
                          status == Status::SMDELIFERYFAILURE           ||  // 1184  4a0h
                          status == Status::SYSFAILURE);                    // 1186  4a2h
        bool immediate = (status == Status::MSGQFUL   ||
                          status == Status::THROTTLED ||
                          status == Status::SUBSCRBUSYMT);
        
        if (accepted) acceptMessage();

        const char* msgid = ((PduXSmResp*)pdu)->get_messageId();
        std::string msgId = ""; 
        if (!msgid || msgid[0] == '\0') accepted = false;
        else msgId = msgid;

        processor.invokeProcessResponce(seqNum, accepted, retry, immediate, msgId);
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
            //smsc_log_debug(logger, "Received DELIVERY_SM Pdu.");
            processReceipt(pdu);
            break;
        case SmppCommandSet::SUBMIT_SM_RESP:
            //smsc_log_debug(logger, "Received SUBMIT_SM_RESP Pdu.");
            processResponce(pdu);
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

static void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT)
    {
        smsc_log_info(logger, "Stopping ...");
        if (bAdminListenerInited) adminListener->shutdown();
        setNeedStop(true);
    }
}
void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
    smsc::logger::Logger::Shutdown();
}

int main(void)
{
    int resultCode = 0;

    Logger::Init();
    logger = Logger::getInstance("smsc.infosme.InfoSme");
    adminListener.reset(new smsc::admin::service::ServiceSocketListener());
    
    sigset_t set, old;
    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, &old);
    sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
    //sigset(SIGINT, appSignalHandler);

    atexit(atExitHandler);

    try 
    {
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);
        
        ConfigView tpConfig(manager, "InfoSme");
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
        TaskProcessor processor(&tpConfig);
        
        ConfigView adminConfig(manager, "InfoSme.Admin");
        InfoSmeComponent admin(processor);                   
        ComponentManager::registerComponent(&admin); 
        adminListener->init(adminConfig.getString("host"), adminConfig.getInt("port"));               
        bAdminListenerInited = true;
        adminListener->Start();
        
        ConfigView smscConfig(manager, "InfoSme.SMSC");
        InfoSmeConfig cfg(&smscConfig);

        while (!isNeedStop())
        {
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
                setNeedReconnect(false);
                session.connect();
                processor.assignMessageSender(&sender);
                processor.Start();
                bInfoSmeIsConnecting = false;
                infoSmeReady.Signal();
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what(); 
                smsc_log_error(logger, "Connect to SMSC failed. Cause: %s", (msg) ? msg:"unknown");
                bInfoSmeIsConnecting = false;
                setNeedReconnect(true);
                if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw exc;
                sleep(cfg.timeOut);
                session.close();
                continue;
            }
            smsc_log_info(logger, "Connected.");
            
            sigemptyset(&set);
            //sigaddset(&set, SIGINT);
            sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
            sigprocmask(SIG_SETMASK, &set, &old);
            
            infoSmeWaitEvent.Wait(0);
            while (!isNeedStop() && !isNeedReconnect())
            {
                infoSmeWaitEvent.Wait(100);
                /*TaskStat stat;
                if (processor.getStatistics("Task1", stat)) {
                    printf("Generated:%6d Delivered:%6d Retried:%6d Failed:%6d\r",
                           stat.generated, stat.delivered, stat.retried, stat.failed);
                }*/
            }
            smsc_log_info(logger, "Disconnecting from SMSC ...");
            trafficControlEvent.Signal();
            processor.Stop();
            processor.assignMessageSender(0);
            session.close();
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
}
