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
#include <dbsme/jobs/SQLJob.h>

#include <sme/SmppBase.hpp>
#include <sms/sms.h>

#include "CommandProcessor.h"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;

using namespace smsc::dbsme;

static bool bDBSmeIsStopped = false;

uint64_t    requestsProcessingCount = 0;
uint64_t    requestsProcessedCount = 0;
uint64_t    failuresNoticedCount = 0;
uint64_t    errorsHandledCount = 0;

Mutex       countersLock;

static log4cpp::Category& log = Logger::getCategory("smsc.dbsme.DBSme");

const int   MAX_ALLOWED_MESSAGE_LENGTH = 200;

class DBSmeTask : public ThreadedTask
{
protected:

    SmppHeader*         pdu;
    CommandProcessor&   processor;
    SmppTransmitter&    transmitter;

public:
    
    DBSmeTask(SmppHeader* pdu, CommandProcessor& cp, SmppTransmitter& trans) 
        : ThreadedTask(), pdu(pdu), processor(cp), transmitter(trans) {};
    virtual ~DBSmeTask() {};

    virtual const char* taskName() 
    {
        return "DBSmeTask";
    };

    void process()
    {
        {
            MutexGuard guard(countersLock);
            requestsProcessingCount++;
        }

        PduDeliverySmResp smResp;
        smResp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
        smResp.set_messageId("");
        smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        transmitter.sendDeliverySmResp(smResp);

        SMS sms;
        fetchSmsFromSmppPdu((PduXSm*)pdu, &sms);

        Command command;
        command.setFromAddress(sms.getOriginatingAddress());
        command.setToAddress(sms.getDestinationAddress());
        command.setJobName(0);

        char smsTextBuff[MAX_ALLOWED_MESSAGE_LENGTH+1];
        getSmsText(&sms, (char *)&smsTextBuff);
        command.setInData((const char*)smsTextBuff);
        __trace2__("Input Data for DBSme '%s'", smsTextBuff);
        
        try 
        {
            processor.process(command);
        }
        catch (CommandProcessException& exc)
        {
            {
                MutexGuard guard(countersLock);
                failuresNoticedCount++;
            }
            command.setOutData(exc.what()); // Error processing SMS !;
        }

        sms.setDestinationAddress(command.getFromAddress());
        sms.setOriginatingAddress(command.getToAddress());
        sms.setEServiceType(processor.getSvcType());
        sms.setArchivationRequested(false);
        sms.setValidTime(time(NULL)+3600);
        sms.setDeliveryReport(0);
        
        Body& body = sms.getMessageBody();
        body.setIntProperty(Tag::SMPP_PROTOCOL_ID, processor.getProtocolId());
        body.setIntProperty(Tag::SMPP_ESM_CLASS, 0 /*xx0000xx*/);
        body.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::DEFAULT);
        body.setIntProperty(Tag::SMPP_PRIORITY, 0);
        
        char* out = (char *)command.getOutData();
        __trace2__("Output Data '%s'", (out) ? out:"");

        int   outLen = (out) ? strlen(out) : 0;
        char  buff[MAX_ALLOWED_MESSAGE_LENGTH+1];
        PduSubmitSmResp* dlResp = 0;
        while (outLen > 0)
        {
            int strLen = (outLen <= MAX_ALLOWED_MESSAGE_LENGTH) ?
                          outLen : MAX_ALLOWED_MESSAGE_LENGTH;
            
            int convLen = ConvertTextTo7Bit(out, strLen, buff, sizeof(buff), 
                                            CONV_ENCODING_ANSI);

            body.setBinProperty(Tag::SMPP_SHORT_MESSAGE, buff, convLen);
            body.setIntProperty(Tag::SMPP_SM_LENGTH, convLen);
            
            PduSubmitSm sm;
            sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
            fillSmppPduFromSms(&sm,&sms);
            dlResp = transmitter.submit(sm);
            if (dlResp) disposePdu((SmppHeader*)dlResp); dlResp = 0;

            out += strLen; outLen -= strLen;
        }

        /*printf((dlResp && dlResp->get_header().get_commandStatus()==0) ?
               "Responce sent !\n" : "Responce wasn't send !\n");*/
        {
            MutexGuard guard(countersLock);
            if (requestsProcessingCount) requestsProcessingCount--;
            requestsProcessedCount++;
        }
    };

    virtual int Execute()
    {
        __require__(pdu);

        switch (pdu->get_commandId())
        {
        case SmppCommandSet::DELIVERY_SM:
            
            //((PduXSm*)pdu)->dump(TRACE_LOG_STREAM);
            printf("\nReceived DELIVERY_SM Pdu.\n");
            process();
            break;
        case SmppCommandSet::SUBMIT_SM_RESP:
            printf("\nReceived SUBMIT_SM_RESP Pdu.\n");
            break;
        default:
            printf("\nReceived unsupported Pdu !\n");
            break;
        }
        
        disposePdu(pdu); pdu=0;
        return 0;
    };
};

class DBSmeTaskManager 
{
private:

    ThreadPool          pool;
    
public:

    DBSmeTaskManager() {};
    DBSmeTaskManager(ConfigView* config)
        throw(ConfigException) 
    {
        init(config);
    };
    virtual ~DBSmeTaskManager() 
    {
        pool.shutdown();
    };

    void init(ConfigView* config)
        throw(ConfigException)
    {
        try
        {
            int maxThreads = config->getInt("max");
            pool.setMaxThreads(maxThreads);
            printf("Max threads count: %d\n", maxThreads);
        }
        catch (ConfigException& exc) 
        {
            log.warn("Maximum thread pool size wasn't specified !");
        }
        try
        {
            int initThreads = config->getInt("init");
            pool.preCreateThreads(initThreads);
            printf("Precreated threads count: %d\n", initThreads);
        }
        catch (ConfigException& exc) 
        {
            log.warn("Precreated threads count in pool wasn't specified !");
        }
    };
    
    void startTask(DBSmeTask* task)
    {
        pool.startTask(task);
    };
};

class DBSmePduListener: public SmppPduEventListener
{
protected:
    
    CommandProcessor&   processor;
    DBSmeTaskManager&   manager;
    SmppTransmitter*    trans;

public:
    
    DBSmePduListener(CommandProcessor& proc, DBSmeTaskManager& man)
        : SmppPduEventListener(), processor(proc), manager(man) {};

    void handleEvent(SmppHeader *pdu)
    {
        manager.startTask(new DBSmeTask(pdu, processor, *trans));
    }
    
    void handleError(int errorCode)
    {
        {
            MutexGuard guard(countersLock);
            if (requestsProcessingCount) requestsProcessingCount--;
            errorsHandledCount++;
        }
        log.error("Oops, Error handled! Code is: %d\n", errorCode);
    }
    
    void setTrans(SmppTransmitter *t)
    {
        trans=t;
    }
};

class DBSmeConfig : public SmeConfig
{
private:
    
    char *strHost, *strSid, *strPassword, *strSysType, *strOrigAddr;

public:
    
    DBSmeConfig(ConfigView* config)
        throw(ConfigException)
            : SmeConfig(), strHost(0), strSid(0), strPassword(0), 
                strSysType(0), strOrigAddr(0)
    {
        // Mandatory fields
        strHost = config->getString("host", "SMSC host wasn't defined !");
        host = strHost;
        strSid = config->getString("sid", "DBSme id wasn't defined !");
        sid = strSid;
        
        port = config->getInt("port", "SMSC port wasn't defined !");
        timeOut = config->getInt("timeout", "Connect timeout wasn't defined !");
        
        // Optional fields
        try
        {
            strPassword = 
                config->getString("password",
                                  "DBSme password wasn't defined !");
            password = strPassword;
        }
        catch (ConfigException& exc) { password = ""; strPassword = 0; }
        try
        {
            strSysType = 
                config->getString("systemType", 
                                  "DBSme system type wasn't defined !");
            systemType = strSysType;
        }
        catch (ConfigException& exc) { systemType = ""; strSysType = 0; }
        try
        {
            strOrigAddr = 
                config->getString("origAddress", 
                                  "DBSme originating address wasn't defined !");
            origAddr = strOrigAddr;
        }
        catch (ConfigException& exc) { origAddr = ""; strOrigAddr = 0; }
    };

    virtual ~DBSmeConfig()
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
    printf("Signal %d handled !\n", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        printf("Stopping ... \n");
        bDBSmeIsStopped = true;
    }
}

int main(void) 
{
    using smsc::db::DataSourceLoader;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    sigset(SIGTERM, appSignalHandler);
    sigset(SIGINT , appSignalHandler);
    
    /*while (!bDBSmeIsStopped) sleep(1);
    printf("Stopped.\n");
    return 0;*/
    
    SQLJobFactory _sqlJobFactory;
    JobFactory::registerFactory(&_sqlJobFactory, SMSC_DBSME_SQL_JOB_IDENTITY);
    
    try 
    {
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();
        
        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);

        ConfigView cpConfig(manager, "DBSme");
        CommandProcessor processor(&cpConfig);

        ConfigView mnConfig(manager, "DBSme.ThreadPool");
        DBSmeTaskManager runner(&mnConfig);
        
        DBSmePduListener listener(processor, runner);
        
        ConfigView ssConfig(manager, "DBSme.SMSC");
        DBSmeConfig cfg(&ssConfig);
        
        SmppSession session(cfg, &listener);
        session.connect();
        sleep(1);
        SmppTransmitter *tr = session.getSyncTransmitter();
        //SmppTransmitter *atr=ss.getAsyncTransmitter();
        listener.setTrans(tr);
        
        while (!bDBSmeIsStopped) 
        {
            sleep(2);
            MutexGuard guard(countersLock);
            printf("\nRequests: %llu processing, %llu processed.\n"
                   "Failures noticed: %llu\n"
                   "SMPP transport errors handled: %llu\n",
                   requestsProcessingCount, requestsProcessedCount,
                   failuresNoticedCount, errorsHandledCount);
        };
    }
    catch (Exception& exc) 
    {
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    return 0;
}
