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
#include <util/xml/init.h>

#include "CommandProcessor.h"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;

using namespace smsc::dbsme;

static bool bDBSmeIsStopped   = false;
static bool bDBSmeIsConnected = false;

uint64_t    requestsProcessingCount = 0;
uint64_t    requestsProcessedCount = 0;
uint64_t    failuresNoticedCount = 0;

Mutex       countersLock;

static log4cpp::Category& log = Logger::getCategory("smsc.dbsme.DBSme");

const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

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

        SMS request;
        fetchSmsFromSmppPdu((PduXSm*)pdu, &request);
        Body& requestBody = request.getMessageBody();
        uint32_t userMessageReference =
            requestBody.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
        bool isRequestUSSD = 
            requestBody.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ? 
                (requestBody.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)
                  == USSD_PSSR_IND) : false;

        Command command;
        command.setFromAddress(request.getOriginatingAddress());
        command.setToAddress(request.getDestinationAddress());
        command.setJobName(0);
        
        char smsTextBuff[MAX_ALLOWED_MESSAGE_LENGTH+1];
        int smsTextBuffLen = getSmsText(&request, 
            (char *)&smsTextBuff, sizeof(smsTextBuff));
        __require__(smsTextBuffLen < MAX_ALLOWED_MESSAGE_LENGTH);
        
        int textPos = 0;
        const char* inputData = (const char *)&smsTextBuff[0];
        if (isRequestUSSD)
        {
            // replace '*' & '#' by spaces
            while (smsTextBuff[textPos] && textPos<smsTextBuffLen) 
            {
                if (smsTextBuff[textPos] == '*' ||
                    smsTextBuff[textPos] == '#') smsTextBuff[textPos] = ' ';
                textPos++;
            }
            
            // skip first paramenter
            bool paramScanned = false; textPos = 0;
            while (smsTextBuff[textPos] && textPos<smsTextBuffLen)
            {
                if (!isspace(smsTextBuff[textPos]))
                {
                    if (paramScanned) break;
                    while (smsTextBuff[textPos] && textPos<smsTextBuffLen &&
                           !isspace(smsTextBuff[textPos])) textPos++;
                    paramScanned = true;
                } else textPos++;
            }
            
            inputData = (textPos >= smsTextBuffLen || !paramScanned) ?
                        0 : (const char *)&smsTextBuff[textPos];
        }
        command.setInData(inputData);
        __trace2__("Input Data for DBSme '%s'", (inputData) ? inputData:"");
        
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
        catch (std::exception& exc) 
        {
            {
                MutexGuard guard(countersLock);
                failuresNoticedCount++;
            }
            command.setOutData(exc.what()); // Error processing SMS !;
            log.error( "std::exception caught while processing command: %s", exc.what() );
        }
        catch (...)
        {
            {
                MutexGuard guard(countersLock);
                failuresNoticedCount++;
            }
            command.setOutData("Unknown error"); // Error processing SMS !;
            log.error( "... caught while processing command" );
        }

        SMS response;

        response.setDestinationAddress(command.getFromAddress());
        response.setOriginatingAddress(command.getToAddress());
        response.setEServiceType(processor.getSvcType());
        response.setArchivationRequested(false);
        response.setValidTime(time(NULL)+3600);
        response.setDeliveryReport(0);
        
        Body& body = response.getMessageBody();
        body.setIntProperty(Tag::SMPP_PROTOCOL_ID, processor.getProtocolId());
        body.setIntProperty(Tag::SMPP_ESM_CLASS, 0 /*xx0000xx*/);
        body.setIntProperty(Tag::SMPP_REGISTRED_DELIVERY, 0);
        body.setIntProperty(Tag::SMPP_PRIORITY, 0);
        body.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
                            userMessageReference);
        if (isRequestUSSD) 
            body.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, USSD_PSSR_RESP);
        
        char* out = (char *)command.getOutData();
        int outLen = (out) ? strlen(out) : 0;
        __trace2__("Output Data '%s'", (out) ? out:"");

        int outPos = -1;
        while (out && out[++outPos]) 
            if (out[outPos] == '\r' || out[outPos] == '\n') out[outPos] = ' ';
        
        char* msgBuf = 0;
        if(hasHighBit(out,outLen)) 
        {
            int msgLen = outLen*2+4;
            msgBuf = new char[msgLen];
            ConvertMultibyteToUCS2(out, outLen, (short*)msgBuf, msgLen, 
                                   CONV_ENCODING_CP1251);
            body.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::UCS2);
            out = msgBuf; outLen = msgLen;
        } 
        else {
            body.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::DEFAULT);
        }
        
        try 
        {
            if (outLen <= MAX_ALLOWED_MESSAGE_LENGTH) {
                body.setBinProperty(Tag::SMPP_SHORT_MESSAGE, out, outLen);
                body.setIntProperty(Tag::SMPP_SM_LENGTH, outLen);
            } else {
                body.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, out, 
                                    (outLen <= MAX_ALLOWED_PAYLOAD_LENGTH) ? 
                                     outLen : MAX_ALLOWED_PAYLOAD_LENGTH);
            }
        }
        catch (...)
        {
            __trace__("Something is wrong with message body. "
                      "Set/Get property failed"); 
            if (msgBuf) delete msgBuf;
            return;
        }

        PduSubmitSm sm;
        sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
        fillSmppPduFromSms(&sm, &response);
        PduSubmitSmResp *resp = transmitter.submit(sm);
        if (resp) disposePdu((SmppHeader*)resp);
        if (msgBuf) delete msgBuf; 
        
        {
            MutexGuard guard(countersLock);
            if (requestsProcessingCount) requestsProcessingCount--;
            requestsProcessedCount++;
        }
    };

    virtual int Execute()
    {
        __require__(pdu);
        __trace2__("DBSME: Processing PDU.\n");
        switch (pdu->get_commandId())
        {
        case SmppCommandSet::DELIVERY_SM:
            
            //((PduXSm*)pdu)->dump(TRACE_LOG_STREAM);
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
        __trace2__("DBSME: deinit DBSmeTaskManager\n");
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
        __trace2__("DBSME: pdu received. Starting task...\n");
        manager.startTask(new DBSmeTask(pdu, processor, *trans));
    }
    
    void handleError(int errorCode)
    {
        bDBSmeIsConnected = false;
        __trace2__("Transport error !!!\n");
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
    __trace2__("Signal %d handled !\n", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        printf("Stopping ... \n");
        bDBSmeIsStopped = true;
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
    
    SQLJobFactory _sqlJobFactory;
    JobFactory::registerFactory(&_sqlJobFactory, 
                                SMSC_DBSME_SQL_JOB_IDENTITY);
    PLSQLJobFactory _plsqlJobFactory;
    JobFactory::registerFactory(&_plsqlJobFactory, 
                                SMSC_DBSME_PLSQL_JOB_IDENTITY);

    try 
    {
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();
        
        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);

        ConfigView cpConfig(manager, "DBSme");
        CommandProcessor processor(&cpConfig);

        ConfigView mnConfig(manager, "DBSme.ThreadPool");
        ConfigView ssConfig(manager, "DBSme.SMSC");
        DBSmeConfig cfg(&ssConfig);
        

        while (!bDBSmeIsStopped)
        {
            DBSmeTaskManager runner(&mnConfig);
            DBSmePduListener listener(processor, runner);
            SmppSession      session(cfg, &listener);

            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set,SIGINT);
//            sigset(SIGTERM, appSignalHandler);
            sigset(SIGINT , appSignalHandler);

            log.info("Connecting to SMSC ... ");
            try
            {
                session.connect();
                sleep(1);
                listener.setTrans(session.getSyncTransmitter());
                bDBSmeIsConnected = true;
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what(); 
                log.error("Connect failed.\nCause: %s\n", (msg) ? msg:"unknown");
                bDBSmeIsConnected = false;
                if (exc.getReason() == 
                    SmppConnectException::Reason::bindFailed) throw;
                sleep(cfg.timeOut);
                session.close();
                runner.shutdown();
                continue;
            }
            log.info("Connected.\n");
            
            while (!bDBSmeIsStopped && bDBSmeIsConnected) 
            {
                sleep(2);
                /*MutexGuard guard(countersLock);
                __trace2__("\nRequests: %llu processing, %llu processed.\n"
                           "Failures noticed: %llu\n",
                           requestsProcessingCount, requestsProcessedCount,
                           failuresNoticedCount);*/
            };
            log.info("Disconnecting from SMSC ...\n");
            session.close();
            runner.shutdown();
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
