#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Array.hpp>

#include <util/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/recoder/recode_dll.h>
#include <util/smstext.h>

#include <signal.h>

#include <db/DataSourceLoader.h>

#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <util/xml/init.h>

#include "WSmeProcessor.h"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;

using namespace smsc::wsme;

static bool bWSmeIsStopped   = false;
static bool bWSmeIsConnected = false;

static log4cpp::Category& log = Logger::getCategory("smsc.wsme.WSme");

const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

class WSmeTask : public ThreadedTask
{
protected:

    SmppHeader*         pdu;
    WSmeProcessor&      processor;
    SmppTransmitter&    transmitter;

    void process()
    {
        {
            PduDeliverySmResp smResp;
            smResp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
            smResp.set_messageId("");
            smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
            transmitter.sendDeliverySmResp(smResp);
        }
        
        try 
        {
            SMS request, responce;
            fetchSmsFromSmppPdu((PduXSm*)pdu, &request);
            if (process(request, responce))
            {
                PduSubmitSm sm;
                sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
                fillSmppPduFromSms(&sm, &responce);
                PduSubmitSmResp *resp = transmitter.submit(sm);
                if (resp) disposePdu((SmppHeader*)resp);
            }
        }
        catch (exception& exc)
        {
            // ÆÎÏÀ !!!
        }
    };
    
    // @return need send responce or not
    bool process(SMS& request, SMS& responce) 
        throw (ProcessException)
    {
        std::string in, out;
        processor.processNotification(in, out);
        processor.processReceipt(in);
        return false;
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
        pool.shutdown();
    };

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
            log.warn("Maximum thread pool size wasn't specified !");
        }
        try
        {
            int initThreads = config->getInt("init");
            pool.preCreateThreads(initThreads);
            __trace2__("Precreated threads count: %d\n", initThreads);
        }
        catch (ConfigException& exc) 
        {
            log.warn("Precreated threads count in pool wasn't specified !");
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
        log.error("Oops, Error handled! Code is: %d\n", errorCode);
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

        ConfigView mnConfig(manager, "WSme.ThreadPool");
        ConfigView ssConfig(manager, "WSme.SMSC");
        WSmeConfig cfg(&ssConfig);
        

        while (!bWSmeIsStopped)
        {
            WSmeTaskManager runner(&mnConfig);
            WSmePduListener listener(processor, runner);
            SmppSession     session(cfg, &listener);

            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set,SIGINT);
            sigset(SIGINT , appSignalHandler);

            log.info("Connecting to SMSC ... ");
            try
            {
                session.connect();
                sleep(1);
                listener.setTrans(session.getSyncTransmitter());
                bWSmeIsConnected = true;
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what(); 
                log.error("Connect failed.\nCause: %s\n", (msg) ? msg:"unknown");
                bWSmeIsConnected = false;
                if (exc.getReason() == 
                    SmppConnectException::Reason::bindFailed) throw;
                sleep(cfg.timeOut);
                continue;
            }
            log.info("Connected.\n");
            
            while (!bWSmeIsStopped && bWSmeIsConnected) sleep(2);
            log.info("Disconnecting from SMSC ...\n");
        };
    }
    catch (ConfigException& exc) 
    {
        log.error("Configuration invalid. Details: %s\n", exc.what());
        return -2;
    }
    catch (exception& exc) 
    {
        log.error("Top level exception: %s\n", exc.what());
        return -1;
    }
    return 0;
}
