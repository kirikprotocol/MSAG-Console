#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Array.hpp>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
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

#include "WSmeProcessor.h"
#include "WSmeComponent.h"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;
using namespace smsc::system;

using namespace smsc::admin;
using namespace smsc::admin::service;

using namespace smsc::wsme;

static bool bWSmeIsStopped   = false;
static bool bWSmeIsConnected = false;

static log4cpp::Category& logger = Logger::getCategory("smsc.wsme.WSme");
static int messageLifePeriod;

const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

static smsc::admin::service::ServiceSocketListener adminListener; 
static bool bAdminListenerInited = false;

static bool convertMSISDNStringToAddress(const char* string, Address& address)
{
    try {
        Address converted(string);
        address = converted;
    } catch (...) {
        return false;
    }
    return true;
};

/*bool compareMaskAndAddress(const std::string mask, 
                           const std::string addr)
{
    try 
    {
        Address ma(mask.c_str());
        Address aa(addr.c_str());
        
        if (ma.length != aa.length || ma.type != aa.type || ma.plan != aa.plan)
            return false;

        for (int i=0; i<ma.length; i++)
            if ((ma.value[i] != '?') && (ma.value[i] != aa.value[i])) 
                return false;
    } 
    catch (...) 
    {
        return false;
    }
    return true;
}*/

class WSmeTask : public ThreadedTask
{
protected:

    SmppHeader*         pdu;
    WSmeProcessor&      processor;
    SmppTransmitter&    transmitter;

    void process()
    {
        {
            // Send DeliverySmResp here for accepted DeliverySm
            PduDeliverySmResp smResp;
            smResp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
            smResp.set_messageId("");
            smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
            transmitter.sendDeliverySmResp(smResp);
        }
        
        try 
        {
            SMS request;
            fetchSmsFromSmppPdu((PduXSm*)pdu, &request);
            bool isReceipt = (request.hasIntProperty(Tag::SMPP_ESM_CLASS)) ? 
                ((request.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3C) == 0x4) : false;

            __trace2__("SMPP_ESM_CLASS=%d", 
                       request.getIntProperty(Tag::SMPP_ESM_CLASS));
            
            if (isReceipt)
            {
                __trace__("Got receipt");
                if (((PduXSm*)pdu)->get_optional().has_receiptedMessageId())
                {
                    const char* msgid = 
                        ((PduXSm*)pdu)->get_optional().get_receiptedMessageId();
                    if (msgid && msgid[0] != '\0') {
                        __trace2__("Got receipt, msgid=%s", msgid);
                        processor.processReceipt(msgid, true);
                    }
                        
                    // ??? TODO: check receipted or not
                } 
                return;
            }
            
            char smsTextBuff[MAX_ALLOWED_MESSAGE_LENGTH+1];
            int smsTextBuffLen = getSmsText(&request, 
                (char *)&smsTextBuff, sizeof(smsTextBuff));
            __require__(smsTextBuffLen < MAX_ALLOWED_MESSAGE_LENGTH);
            
            __trace2__("Got notification message: %s", smsTextBuff);

            std::string msisdn;
            Address visitorAddress;
            if (convertMSISDNStringToAddress(smsTextBuff, visitorAddress)) {
                visitorAddress.toString(smsTextBuff, sizeof(smsTextBuff));
                msisdn = smsTextBuff;
            }
            else
                throw Exception("Visitor '%s' is not valid MSISDN address",
                                (smsTextBuff) ? smsTextBuff:"");
            
            __trace__("Processing notification message ...");
            std::string outMsgStr = ""; 
            if (processor.processNotification(msisdn, outMsgStr))
            {
                SMS responce; 
                responce.setDestinationAddress(visitorAddress);
                responce.setOriginatingAddress(request.getDestinationAddress());
                responce.setArchivationRequested(false);
                responce.setValidTime(time(NULL)+messageLifePeriod);
                responce.setDeliveryReport(1);
                responce.setEServiceType(processor.getSvcType());

                responce.setIntProperty(Tag::SMPP_PROTOCOL_ID, processor.getProtocolId());
                responce.setIntProperty(Tag::SMPP_ESM_CLASS, 0 /*xx0000xx*/);
                responce.setIntProperty(Tag::SMPP_PRIORITY, 0);
                responce.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
                                        request.getIntProperty(
                                        Tag::SMPP_USER_MESSAGE_REFERENCE));
                responce.setIntProperty(Tag::SMPP_REGISTRED_DELIVERY, 1);

                const char* out = outMsgStr.c_str();
                int outLen = outMsgStr.length();
                char* msgBuf = 0;
                if(hasHighBit(out,outLen)) {
                    int msgLen = outLen*2+4;
                    msgBuf = new char[msgLen];
                    ConvertMultibyteToUCS2(out, outLen, (short*)msgBuf, msgLen, 
                                           CONV_ENCODING_CP1251);
                    responce.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::UCS2);
                    out = msgBuf; outLen = msgLen;
                } else {
                    responce.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::DEFAULT);
                }

                try {
                    if (outLen <= MAX_ALLOWED_MESSAGE_LENGTH) {
                        responce.setBinProperty(Tag::SMPP_SHORT_MESSAGE, out, outLen);
                        responce.setIntProperty(Tag::SMPP_SM_LENGTH, outLen);
                    } else {
                        responce.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, out, 
                                                (outLen <= MAX_ALLOWED_PAYLOAD_LENGTH) ? 
                                                outLen : MAX_ALLOWED_PAYLOAD_LENGTH);
                    }
                } catch (...) {
                    __trace__("Something is wrong with message body. "
                              "Set/Get property failed"); 
                    if (msgBuf) delete msgBuf; msgBuf = 0;
                    throw; // ???
                }
                if (msgBuf) delete msgBuf;

                PduSubmitSm sm;
                sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
                fillSmppPduFromSms(&sm, &responce);
                PduSubmitSmResp *resp = transmitter.submit(sm);
                if (resp) 
                {
                    // check responded or not, get msgid here
                    const char* msgid = ((PduXSmResp*)resp)->get_messageId();
                    if (msgid && msgid[0] != '\0') {
                        __trace2__("Got responce, msgid=%s", msgid);
                        processor.processResponce(msisdn, msgid, true);
                    }
                    
                    disposePdu((SmppHeader*)resp);
                }
            }
            __trace__("End message processing.");
        }
        catch (exception& exc)
        {
            __trace2__("Error handled: %s", exc.what());
            logger.error(exc.what());
        }
    };
    
public:
    
    WSmeTask(SmppHeader* pdu, WSmeProcessor& cp, SmppTransmitter& trans) 
        : ThreadedTask(), pdu(pdu), processor(cp), transmitter(trans) {};
    virtual ~WSmeTask() {};

    virtual const char* taskName() {
        return "WSmeTask";
    };

    virtual int Execute()
    {
        __require__(pdu);
        __trace2__("WSme: Processing PDU.\n");
        
        switch (pdu->get_commandId())
        {
        case SmppCommandSet::DELIVERY_SM:
            __trace2__("\nReceived DELIVERY_SM Pdu.\n");
            process();
            break;
        case SmppCommandSet::SUBMIT_SM_RESP:
            __trace2__("\nReceived SUBMIT_SM_RESP Pdu.\n");
            break;
        default:
            __trace2__("\nReceived unsupported Pdu !\n");
            break;
        }
        
        disposePdu(pdu); pdu=0;
        return 0;
    };
};

class WSmeTaskManager 
{
private:

    ThreadPool          pool;
    
public:

    WSmeTaskManager() {};
    WSmeTaskManager(ConfigView* config)
        throw(ConfigException) 
    {
        init(config);
    };
    virtual ~WSmeTaskManager() 
    {
        __trace2__("WSme: deinit WSmeTaskManager\n");
        shutdown();
    };
    void shutdown()
    {
        pool.shutdown();
    }

    void init(ConfigView* config)
        throw(ConfigException)
    {
        try
        {
            int maxThreads = config->getInt("max");
            pool.setMaxThreads(maxThreads);
            __trace2__("Max threads count: %d\n", maxThreads);
        }
        catch (ConfigException& exc) 
        {
            logger.warn("Maximum thread pool size wasn't specified !");
        }
        try
        {
            int initThreads = config->getInt("init");
            pool.preCreateThreads(initThreads);
            __trace2__("Precreated threads count: %d\n", initThreads);
        }
        catch (ConfigException& exc) 
        {
            logger.warn("Precreated threads count in pool wasn't specified !");
        }
    };
    
    void startTask(WSmeTask* task)
    {
        pool.startTask(task);
    };
};

class WSmePduListener: public SmppPduEventListener
{
protected:
    
    WSmeProcessor&      processor;
    WSmeTaskManager&    manager;
    SmppTransmitter*    trans;

public:
    
    WSmePduListener(WSmeProcessor& proc, WSmeTaskManager& man)
        : SmppPduEventListener(), processor(proc), manager(man) {};

    void handleEvent(SmppHeader *pdu)
    {
        __trace2__("WSme: pdu received. Starting task...\n");
        manager.startTask(new WSmeTask(pdu, processor, *trans));
    }
    
    void handleError(int errorCode)
    {
        bWSmeIsConnected = false;
        __trace2__("Transport error !!!\n");
        logger.error("Oops, Error handled! Code is: %d\n", errorCode);
    }
    
    void setTrans(SmppTransmitter *t)
    {
        trans=t;
    }
};

class WSmeConfig : public SmeConfig
{
private:
    
    char *strHost, *strSid, *strPassword, *strSysType, *strOrigAddr;

public:
    
    WSmeConfig(ConfigView* config)
        throw(ConfigException)
            : SmeConfig(), strHost(0), strSid(0), strPassword(0), 
                strSysType(0), strOrigAddr(0)
    {
        // Mandatory fields
        strHost = config->getString("host", "SMSC host wasn't defined !");
        host = strHost;
        strSid = config->getString("sid", "WSme id wasn't defined !");
        sid = strSid;
        
        port = config->getInt("port", "SMSC port wasn't defined !");
        timeOut = config->getInt("timeout", "Connect timeout wasn't defined !");
        
        // Optional fields
        try
        {
            strPassword = 
                config->getString("password",
                                  "WSme password wasn't defined !");
            password = strPassword;
        }
        catch (ConfigException& exc) { password = ""; strPassword = 0; }
        try
        {
            strSysType = 
                config->getString("systemType", 
                                  "WSme system type wasn't defined !");
            systemType = strSysType;
        }
        catch (ConfigException& exc) { systemType = ""; strSysType = 0; }
        try
        {
            strOrigAddr = 
                config->getString("origAddress", 
                                  "WSme originating address wasn't defined !");
            origAddr = strOrigAddr;
        }
        catch (ConfigException& exc) { origAddr = ""; strOrigAddr = 0; }
    };

    virtual ~WSmeConfig()
    {
        if (strHost) delete strHost;
        if (strSid) delete strSid;
        if (strPassword) delete strPassword;
        if (strSysType) delete strSysType;
        if (strOrigAddr) delete strOrigAddr;
    };
};

static void appSignalHandler(int sig)
{
    __trace2__("Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        __trace__("Stopping ...");
        if (bAdminListenerInited) adminListener.shutdown();
        bWSmeIsStopped = true;
    }
}


// added by igork
void atExitHandler(void)
{
    //sigsend(P_PID, getppid(), SIGCHLD);
    smsc::util::xml::TerminateXerces();
}

int main(void) 
{
    int resultCode = 0;
    Logger::Init("log4cpp.wsme");

    using smsc::db::DataSourceLoader;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    //added by igork
    atexit(atExitHandler);
    
    try 
    {
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();
        
        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);

        ConfigView cpConfig(manager, "WSme");
        WSmeProcessor processor(&cpConfig);

        ConfigView adminConfig(manager, "WSme.Admin");
        WSmeComponent wsmeAdmin(processor);                   
        ComponentManager::registerComponent(&wsmeAdmin); 
        adminListener.init(adminConfig.getString("host"), 
                           adminConfig.getInt("port"));               
        bAdminListenerInited = true;
        adminListener.Start();                                     
        
        ConfigView mnConfig(manager, "WSme.ThreadPool");
        ConfigView ssConfig(manager, "WSme.SMSC");
        WSmeConfig cfg(&ssConfig);
        
        messageLifePeriod = cpConfig.getInt("AdManager.History.messageLife", 
                                            "Message life period wasn't defined !");
        
        while (!bWSmeIsStopped)
        {
            WSmeTaskManager runner(&mnConfig);
            WSmePduListener listener(processor, runner);
            SmppSession     session(cfg, &listener);

            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
            sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
            
            logger.info("Connecting to SMSC ... ");
            try
            {
                listener.setTrans(session.getSyncTransmitter());
                session.connect();
                bWSmeIsConnected = true;
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what(); 
                logger.error("Connect failed.\nCause: %s\n", (msg) ? msg:"unknown");
                bWSmeIsConnected = false;
                if (exc.getReason() == 
                    SmppConnectException::Reason::bindFailed) throw;
                sleep(cfg.timeOut);
                session.close();
                runner.shutdown();
                continue;
            }
            logger.info("Connected.\n");
            
            while (!bWSmeIsStopped && bWSmeIsConnected) sleep(2);
            logger.info("Disconnecting from SMSC ...\n");
            session.close();
            runner.shutdown();
        };
    }
    catch (ConfigException& exc) 
    {
        logger.error("Configuration invalid. Details: %s\n", exc.what());
        resultCode = -2;
    }
    catch (exception& exc) 
    {
        logger.error("Top level exception: %s\n", exc.what());
        resultCode = -1;
    }
    
    if (bAdminListenerInited) adminListener.WaitFor();
    return resultCode;
}

