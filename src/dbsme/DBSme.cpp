#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Array.hpp>

#include <util/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSourceLoader.h>
#include <dbsme/jobs/SQLJob.h>

#include <sme/SmppBase.hpp>
#include <sms/sms.h>

#include "CommandProcessor.h"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;

using namespace smsc::dbsme;

bool        bDBSmeIsStopped = false;

uint64_t    requestsProcessingCount = 0;
uint64_t    requestsProcessedCount = 0;
uint64_t    failuresNoticedCount = 0;
uint64_t    errorsHandledCount = 0;

Mutex       countersLock;

using smsc::util::Logger;
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

        Body& body = sms.getMessageBody();

        std::string input = body.getStrProperty(Tag::SMPP_SHORT_MESSAGE);
        command.setInData(input.c_str());

        try 
        {
            processor.process(command);
        }
        catch (Exception& exc)
        {
            {
                MutexGuard guard(countersLock);
                if (requestsProcessingCount) requestsProcessingCount--;
                failuresNoticedCount++;
            }
            //command.setOutData("Error processing SMS !");
            command.setOutData(exc.what());
        }

        sms.setDestinationAddress(command.getFromAddress());
        sms.setOriginatingAddress(command.getToAddress());
        sms.setArchivationRequested(false);
        sms.setDeliveryReport(0);
        sms.setValidTime(time(NULL)+3600);

        char* out = (char *)command.getOutData();
        int   outLen = (out) ? strlen(out) : 0;
        char  buff[MAX_ALLOWED_MESSAGE_LENGTH+1];
        PduSubmitSmResp* dlResp = 0;
        while (outLen > 0)
        {
            int strLen = (outLen <= MAX_ALLOWED_MESSAGE_LENGTH) ?
                          outLen : MAX_ALLOWED_MESSAGE_LENGTH;
            strncpy(buff, out, strLen); buff[strLen] = '\0';

            body.setIntProperty(Tag::SMPP_SM_LENGTH, strLen);
            body.setStrProperty(Tag::SMPP_SHORT_MESSAGE, buff);

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
            printf("\nReceived DELIVERY_SM Pdu. Message: '%s'\n",
                   ((PduXSm*)pdu)->get_message().get_shortMessage());
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

int main(void) 
{
    using smsc::db::DataSourceLoader;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

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
