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

const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

static bool  bInfoSmeIsStopped   = false;
static bool  bInfoSmeIsConnected = false;

static Event infoSmeReady;
static int   infoSmeReadyTimeout = 10000;

static int   unrespondedMessagesSleep = 1000;
static int   unrespondedMessagesCount = 0;
static int   unrespondedMessagesMax   = 100;
static Event unrespondedMessagesEvent;
static Mutex unrespondedMessagesLock;

static log4cpp::Category& logger = Logger::getCategory("smsc.infosme.InfoSme");

static smsc::admin::service::ServiceSocketListener adminListener; 
static bool bAdminListenerInited = false;

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
        int count = 0;
        {
            MutexGuard guard(unrespondedMessagesLock);
            count = ++unrespondedMessagesCount;
        }
        while (count >= unrespondedMessagesMax && !bInfoSmeIsStopped && bInfoSmeIsConnected)
        {
            unrespondedMessagesEvent.Wait(unrespondedMessagesSleep);
            MutexGuard guard(unrespondedMessagesLock);
            count = unrespondedMessagesCount;
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
            logger.error("Smpp session is undefined for MessageSender.");
            return false;
        }
        SmppTransmitter* asyncTransmitter = session->getAsyncTransmitter();
        if (!asyncTransmitter) {
            logger.error("Smpp transmitter is undefined for MessageSender.");
            return false;
        }
        
        Address oa, da;
        const char* oaStr = processor.getAddress();
        if (!oaStr || !convertMSISDNStringToAddress(oaStr, oa)) {
            logger.error("Invalid originating address '%s'", oaStr ? oaStr:"-");
            return false;
        }
        const char* daStr = abonent.c_str();
        if (!daStr || !convertMSISDNStringToAddress(daStr, da)) {
            logger.error("Invalid destination address '%s'", daStr ? daStr:"-");
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
            logger.error("Something is wrong with message body. Set/Get property failed"); 
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

    void respondMessage()
    {
        MutexGuard guard(unrespondedMessagesLock);
        if (unrespondedMessagesCount-- >= unrespondedMessagesMax) {
            unrespondedMessagesEvent.Signal();
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
                        logger.warn("Invalid state=%d received in reciept !", msgState);
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
        respondMessage();

        int seqNum = pdu->get_sequenceNumber();
        int status = pdu->get_commandStatus();
        bool accepted =  (status == SmppStatusSet::ESME_ROK);
        bool retry    =  (status == SmppStatusSet::ESME_RX_T_APPN ||
                          status == SmppStatusSet::ESME_RMSGQFUL  ||
                          status == SmppStatusSet::ESME_RTHROTTLED);
        bool immediate = (status == SmppStatusSet::ESME_RMSGQFUL  ||
                          status == SmppStatusSet::ESME_RTHROTTLED);

        const char* msgid = ((PduXSmResp*)pdu)->get_messageId();
        std::string msgId = ""; 
        if (!msgid || msgid[0] == '\0') accepted = false;
        else msgId = msgid;

        processor.invokeProcessResponce(seqNum, accepted, retry, immediate, msgId);
    }

    void handleEvent(SmppHeader *pdu)
    {
        if (!bInfoSmeIsConnected) {
            infoSmeReady.Wait(infoSmeReadyTimeout);
            if (!bInfoSmeIsConnected) {
                disposePdu(pdu);
                return;
            }
        }

        switch (pdu->get_commandId())
        {
        case SmppCommandSet::DELIVERY_SM:
            //logger.debug("Received DELIVERY_SM Pdu.");
            processReceipt(pdu);
            break;
        case SmppCommandSet::SUBMIT_SM_RESP:
            //logger.debug("Received SUBMIT_SM_RESP Pdu.");
            processResponce(pdu);
            break;
        default:
            logger.debug("Received unsupported Pdu !");
            break;
        }
        
        disposePdu(pdu);
    }
    
    void handleError(int errorCode)
    {
        logger.error("Transport error handled! Code is: %d", errorCode);
        bInfoSmeIsConnected = false;
    }
};

static void appSignalHandler(int sig)
{
    logger.debug("Signal %d handled !", sig);
    if (sig==smsc::system::SHUTDOWN_SIGNAL || sig==SIGINT)
    {
        logger.info("Stopping ...");
        if (bAdminListenerInited) adminListener.shutdown();
        bInfoSmeIsStopped = true;
    }
}
void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
}

int main(void)
{
    int resultCode = 0;

    Logger::Init("log4cpp.infosme");
    
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
            unrespondedMessagesMax = 100;
            logger.warn("Parameter 'unrespondedMessagesMax' value is invalid. Using default %d",
                        unrespondedMessagesMax);
        }
        try { unrespondedMessagesSleep = tpConfig.getInt("unrespondedMessagesSleep"); } catch (...) {};
        if (unrespondedMessagesSleep <= 0) {
            unrespondedMessagesSleep = 1000;
            logger.warn("'unrespondedMessagesSleep' value is invalid. Using default %d",
                        unrespondedMessagesSleep);
        }
        TaskProcessor processor(&tpConfig);
        
        ConfigView adminConfig(manager, "InfoSme.Admin");
        InfoSmeComponent admin(processor);                   
        ComponentManager::registerComponent(&admin); 
        adminListener.init(adminConfig.getString("host"), adminConfig.getInt("port"));               
        bAdminListenerInited = true;
        adminListener.Start();
        
        ConfigView smscConfig(manager, "InfoSme.SMSC");
        InfoSmeConfig cfg(&smscConfig);

        while (!bInfoSmeIsStopped)
        {
            InfoSmePduListener      listener(processor);
            SmppSession             session(cfg, &listener);
            InfoSmeMessageSender    sender(processor, &session);
            
            logger.info("Connecting to SMSC ... ");
            try
            {
                listener.setSyncTransmitter(session.getSyncTransmitter());
                listener.setAsyncTransmitter(session.getAsyncTransmitter());
                
                infoSmeReady.Wait(0);
                session.connect();
                processor.assignMessageSender(&sender);
                processor.Start();
                bInfoSmeIsConnected = true;
                infoSmeReady.Signal();
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what(); 
                logger.error("Connect to SMSC failed. Cause: %s", (msg) ? msg:"unknown");
                bInfoSmeIsConnected = false;
                if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw exc;
                sleep(cfg.timeOut);
                session.close();
                continue;
            }
            logger.info("Connected.");
            
            sigemptyset(&set);
            //sigaddset(&set, SIGINT);
            sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
            sigprocmask(SIG_SETMASK, &set, &old);
            
            Event aaa;
            while (!bInfoSmeIsStopped && bInfoSmeIsConnected) {
                aaa.Wait(100);
                //sleep(1);
                TaskStat stat;
                if (processor.getStatistics("Task1", stat)) {
                    printf("Generated:%6d Delivered:%6d Retried:%6d Failed:%6d\r",
                           stat.generated, stat.delivered, stat.retried, stat.failed);
                }
            }
            logger.info("Disconnecting from SMSC ...");
            unrespondedMessagesEvent.Signal();
            processor.Stop();
            processor.assignMessageSender(0);
            session.close();
        }
    }
    catch (SmppConnectException& exc)
    {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            logger.error("Failed to bind InfoSme. Exiting.");
        resultCode = -1;
    }
    catch (ConfigException& exc) 
    {
        logger.error("Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc) 
    {
        logger.error("Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) 
    {
        logger.error("Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    }
    catch (...) 
    {
        logger.error("Unknown exception: '...' caught. Exiting.");
        resultCode = -5;
    }
    
    if (bAdminListenerInited)
    {
        adminListener.shutdown();
        adminListener.WaitFor();
    }
    return resultCode;
}
