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

static bool bInfoSmeIsStopped   = false;
static bool bInfoSmeIsConnected = false;

static log4cpp::Category& logger = Logger::getCategory("smsc.infosme.InfoSme");

static smsc::admin::service::ServiceSocketListener adminListener; 
static bool bAdminListenerInited = false;

class InfoSmeEventHandler : public ThreadedTask
{
protected:

    SmppHeader*         pdu;
    TaskProcessor&      processor;
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

            logger.debug("SMPP_ESM_CLASS=%d", request.getIntProperty(Tag::SMPP_ESM_CLASS));
            
            if (isReceipt)
            {
                logger.debug("Got receipt");
                if (((PduXSm*)pdu)->get_optional().has_receiptedMessageId())
                {
                    const char* msgid = 
                        ((PduXSm*)pdu)->get_optional().get_receiptedMessageId();
                    if (msgid && msgid[0] != '\0') {
                        logger.debug("Got receipt, msgid=%s", msgid);
                        //processor.processReceipt(msgid, true);
                    }
                        
                    // ??? TODO: check receipted or not
                } 
                return;
            }
        }
        catch (exception& exc) {
            logger.error("Error handled: %s", exc.what());
        }
        catch (...) {
            logger.error("Unknown error handled.");
        }
    };
    
public:
    
    InfoSmeEventHandler(SmppHeader* pdu, TaskProcessor& tp, SmppTransmitter& trans) 
        : ThreadedTask(), pdu(pdu), processor(tp), transmitter(trans) {};
    virtual ~InfoSmeEventHandler() {};

    virtual const char* taskName() {
        return "InfoSmeEventHandler";
    };

    virtual int Execute()
    {
        __require__(pdu);
        logger.debug("InfoSmeEventHandler: Processing PDU.");
        
        switch (pdu->get_commandId())
        {
        case SmppCommandSet::DELIVERY_SM:
            logger.debug("Received DELIVERY_SM Pdu.");
            process();
            break;
        case SmppCommandSet::SUBMIT_SM_RESP:
            logger.debug("Received SUBMIT_SM_RESP Pdu.");
            break;
        default:
            logger.debug("Received unsupported Pdu !");
            break;
        }
        
        disposePdu(pdu); pdu=0;
        return 0;
    };
};

class InfoSmeEventManager 
{
private:

    ThreadPool          pool;
    
public:

    InfoSmeEventManager() {};
    InfoSmeEventManager(ConfigView* config)
        throw(ConfigException) 
    {
        init(config);
    };
    
    virtual ~InfoSmeEventManager() {
        logger.debug("InfoSme: deinit InfoSmeEventManager\n");
        shutdown();
    };
    void shutdown() {
        pool.shutdown();
    }

    void init(ConfigView* config)
        throw(ConfigException)
    {
        try 
        {
            int maxThreads = config->getInt("max");
            pool.setMaxThreads(maxThreads);
            logger.debug("Max threads count: %d\n", maxThreads);
        }
        catch (ConfigException& exc) {
            logger.warn("Maximum thread pool size wasn't specified !");
        }

        try
        {
            int initThreads = config->getInt("init");
            pool.preCreateThreads(initThreads);
            logger.debug("Precreated threads count: %d\n", initThreads);
        }
        catch (ConfigException& exc) {
            logger.warn("Precreated threads count in pool wasn't specified !");
        }
    };
    
    void startHandler(InfoSmeEventHandler* handler) {
        pool.startTask(handler);
    };
};

class InfoSmePduListener: public SmppPduEventListener
{
protected:
    
    TaskProcessor&          processor;
    InfoSmeEventManager&    manager;
    SmppTransmitter*        trans;

public:
    
    InfoSmePduListener(TaskProcessor& proc, InfoSmeEventManager& man)
        : SmppPduEventListener(), processor(proc), manager(man), trans(0) {};

    void handleEvent(SmppHeader *pdu) {
        logger.debug("InfoSme: pdu received. Starting task...\n");
        manager.startHandler(new InfoSmeEventHandler(pdu, processor, *trans));
    }
    
    void handleError(int errorCode) {
        bInfoSmeIsConnected = false;
        logger.error("Oops, transport error handled! Code is: %d\n", errorCode);
    }
    
    void setTrans(SmppTransmitter *t) {
        trans=t;
    }
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
        strSid = config->getString("sid", "WSme id wasn't defined !");
        sid = strSid;
        
        port = config->getInt("port", "SMSC port wasn't defined !");
        timeOut = config->getInt("timeout", "Connect timeout wasn't defined !");
        
        // Optional fields
        try
        {
            strPassword = config->getString("password",
                                            "WSme password wasn't defined !");
            password = strPassword;
        }
        catch (ConfigException& exc) { password = ""; strPassword = 0; }
        try
        {
            strSysType = config->getString("systemType", 
                                           "WSme system type wasn't defined !");
            systemType = strSysType;
        }
        catch (ConfigException& exc) { systemType = ""; strSysType = 0; }
        try
        {
            strOrigAddr = config->getString("origAddress", 
                                            "WSme originating address wasn't defined !");
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

static void appSignalHandler(int sig)
{
    logger.debug("Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
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
    atexit(atExitHandler);

    try 
    {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
        sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);
        
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();

        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);
        
        ConfigView tpConfig(manager, "InfoSme");
        TaskProcessor processor(&tpConfig);
        
        ConfigView emanConfig(manager, "WSme.EventsThreadPool");
        ConfigView smscConfig(manager, "WSme.SMSC");
        InfoSmeConfig cfg(&smscConfig);
        
        while (!bInfoSmeIsStopped)
        {
            InfoSmeEventManager runner(&emanConfig);
            InfoSmePduListener  listener(processor, runner);
            SmppSession         session(cfg, &listener);

            logger.info("Connecting to SMSC ... ");
            try
            {
                listener.setTrans(session.getSyncTransmitter());
                session.connect();
                processor.assignMessageSender(0); // TODO: implement message sender
                processor.Start();
                bInfoSmeIsConnected = true;
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what(); 
                logger.error("Connect to SMSC failed. Cause: %s\n", (msg) ? msg:"unknown");
                bInfoSmeIsConnected = false;
                if (exc.getReason() == 
                    SmppConnectException::Reason::bindFailed) throw exc;
                sleep(cfg.timeOut);
                session.close();
                runner.shutdown();
                continue;
            }
            logger.info("Connected.\n");
            
            while (!bInfoSmeIsStopped && bInfoSmeIsConnected) sleep(2);
            logger.info("Disconnecting from SMSC ...\n");
            processor.Stop();
            session.close();
            runner.shutdown();
        };
        
        /*processor.Start();
        Event aaa;
        aaa.Wait(100000);*/
    }
    catch (SmppConnectException& exc)
    {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            logger.error("Failed to bind InfoSme. Exiting.\n");
        resultCode = -1;
    }
    catch (ConfigException& exc) 
    {
        logger.error("Configuration invalid. Details: %s Exiting.\n", exc.what());
        resultCode = -2;
    }
    catch (Exception& exc) 
    {
        logger.error("Top level Exception: %s Exiting.\n", exc.what());
        resultCode = -3;
    }
    catch (exception& exc) 
    {
        logger.error("Top level exception: %s Exiting.\n", exc.what());
        resultCode = -4;
    }
    catch (...) 
    {
        logger.error("Unknown exception: '...' caught. Exiting.\n");
        resultCode = -5;
    }
    
    if (bAdminListenerInited)
    {
        adminListener.shutdown();
        adminListener.WaitFor();
    }
    return resultCode;
}
