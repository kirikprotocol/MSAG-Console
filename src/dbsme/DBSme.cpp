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
#include <dbsme/jobs/SQLJob.h>

#include <system/smscsignalhandlers.h>
#include <sme/SmppBase.hpp>
#include <sms/sms.h>
#include <util/xml/init.h>

#include <admin/service/Component.h>
#include <admin/service/ComponentManager.h>
#include <admin/service/ServiceSocketListener.h>
#include "version.inc"

#include "DBSmeComponent.h"
#include "CommandProcessor.h"

using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::util;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;
using namespace smsc::system;

using namespace smsc::admin;
using namespace smsc::admin::service;

using namespace smsc::dbsme;

static smsc::logger::Logger *logger = 0;

static std::auto_ptr<smsc::admin::service::ServiceSocketListener> adminListener;
static bool bAdminListenerInited = false;

const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;
const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;

Mutex        countersLock;
uint64_t     requestsProcessingCount = 0;
uint64_t     requestsProcessedCount = 0;
uint64_t     failuresNoticedCount = 0;

static Event bDBSmeWaitEvent;

static Mutex needStopLock;
static Mutex needReinitLock;
static Mutex needReconnectLock;

static bool  bDBSmeIsStopped   = false;
static bool  bDBSmeIsConnected = false;
static bool  bDBSmeNeedReinit  = false;

static uint8_t uDBSmeDataCoding = DataCoding::LATIN1;

static void setNeedReinit(bool reinit=true) {
    MutexGuard gauard(needReinitLock);
    bDBSmeNeedReinit = reinit;
    if (reinit) bDBSmeWaitEvent.Signal();
}
static bool isNeedReinit() {
    MutexGuard gauard(needReinitLock);
    return bDBSmeNeedReinit;
}

static void setNeedStop(bool stop=true) {
    MutexGuard gauard(needStopLock);
    bDBSmeIsStopped = stop;
    if (stop) bDBSmeWaitEvent.Signal();
}
static bool isNeedStop() {
    MutexGuard gauard(needStopLock);
    return bDBSmeIsStopped;
}

static void setNeedReconnect(bool reconnect=true) {
    MutexGuard gauard(needReconnectLock);
    bDBSmeIsConnected = !reconnect;
    if (reconnect) bDBSmeWaitEvent.Signal();
}
static bool isNeedReconnect() {
    MutexGuard gauard(needReconnectLock);
    return !bDBSmeIsConnected;
}


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
        bool isReceipt = (request.hasIntProperty(Tag::SMPP_ESM_CLASS)) ?
            ((request.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3C) == 0x4) : false;
        if (isReceipt || ((PduXSm*)pdu)->get_optional().has_receiptedMessageId()) {
            smsc_log_warn(logger, "Unexpected sms receipt handled. Skipped.");
            return;
        }

        uint32_t userMessageReference = request.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
        bool isRequestUSSD = request.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                            (request.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == USSD_PSSR_IND) : false;

        smsc::dbsme::Command command;
        command.setFromAddress(request.getOriginatingAddress());
        command.setToAddress(request.getDestinationAddress());
        command.setJobName(0);

        char smsTextBuff[MAX_ALLOWED_MESSAGE_LENGTH+1];
        int smsTextBuffLen = getSmsText(&request, (char *)&smsTextBuff, sizeof(smsTextBuff));
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
            smsc_log_error(logger,  "std::exception caught while processing command: %s", exc.what() );
        }
        catch (...)
        {
            {
                MutexGuard guard(countersLock);
                failuresNoticedCount++;
            }
            command.setOutData("Unknown error"); // Error processing SMS !;
            smsc_log_error(logger,  "... caught while processing command" );
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
        if (isRequestUSSD) {
            body.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, USSD_PSSR_RESP);
            // clear 0,1 bits and set them to datagram mode
            body.setIntProperty(Tag::SMPP_ESM_CLASS,
                (body.getIntProperty(Tag::SMPP_ESM_CLASS)&~0x03)|0x01);
        }

        char* out = (char *)command.getOutData();
        int outLen = (out) ? strlen(out) : 0;
        __trace2__("Output Data '%s'", (out) ? out:"");

        int outPos = -1;
        while (out && out[++outPos])
            if (out[outPos] == '\r' || out[outPos] == '\n') out[outPos] = ' ';

        char* msgBuf = 0;
        if(hasHighBit(out,outLen) || uDBSmeDataCoding == DataCoding::UCS2)
        {
            int msgLen = outLen*2;
            msgBuf = new char[msgLen];
            ConvertMultibyteToUCS2(out, outLen, (short*)msgBuf, msgLen, CONV_ENCODING_CP1251);
            body.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::UCS2);
            out = msgBuf; outLen = msgLen;
        }
        else if (uDBSmeDataCoding == DataCoding::SMSC7BIT) {
            unsigned msgLen = outLen*2;
            msgBuf = new char[msgLen];
            msgLen = ConvertLatin1ToSMSC7Bit(out, outLen, msgBuf);
            body.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::SMSC7BIT);
            out = msgBuf; outLen = msgLen;
        }
        else {
            body.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::LATIN1);
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
        
        switch (pdu->get_commandId())
        {
        case SmppCommandSet::DELIVERY_SM:
            process();
            break;
        case SmppCommandSet::SUBMIT_SM_RESP:
            break;
        case SmppCommandSet::ENQUIRE_LINK: case SmppCommandSet::ENQUIRE_LINK_RESP:
            break;
        default:
            __trace__("Received unsupported Pdu !");
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
        __trace__("DBSME: deinit DBSmeTaskManager");
        shutdown();
    };

    void shutdown() {
        pool.shutdown();
    }

    void init(ConfigView* config)
        throw(ConfigException)
    {
        try {
            int maxThreads = config->getInt("max");
            pool.setMaxThreads(maxThreads);
        }
        catch (ConfigException& exc) {
            smsc_log_warn(logger, "Maximum thread pool size wasn't specified !");
        }
        try {
            int initThreads = config->getInt("init");
            pool.preCreateThreads(initThreads);
        }
        catch (ConfigException& exc) {
            smsc_log_warn(logger, "Precreated threads count in pool wasn't specified !");
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
        __trace__("DBSME: pdu received. Starting task...");
        manager.startTask(new DBSmeTask(pdu, processor, *trans));
    }

    void handleError(int errorCode)
    {
        smsc_log_error(logger, "Transport error handled! Code is: %d", errorCode);
        setNeedReconnect(true);
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

struct DBSmeAdminHandler : public DBSmeAdmin
{
    virtual void restart() {
        smsc_log_error(logger, "Administrator has requested restart");
        setNeedReinit(true);
    }
    virtual void addJob(std::string providerId, std::string jobId) {
        processor.addJob(providerId, jobId);
    }
    virtual void removeJob(std::string providerId, std::string jobId) {
        processor.removeJob(providerId, jobId);
    }
    virtual void changeJob(std::string providerId, std::string jobId) {
        processor.changeJob(providerId, jobId);
    }
    virtual void setProviderEnabled(std::string providerId, bool enabled) {
        processor.setProviderEnabled(providerId, enabled);
    }

    DBSmeAdminHandler(CommandProcessor& processor)
        : DBSmeAdmin(), processor(processor) {};
    virtual ~DBSmeAdminHandler() {};

private:

    CommandProcessor& processor;
};

extern "C" static void appSignalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT) {
        smsc_log_info(logger, "Stopping ...");
        setNeedStop(true);
    }
}


// added by igork
extern "C" void atExitHandler(void)
{
    //sigsend(P_PID, getppid(), SIGCHLD);
    smsc::util::xml::TerminateXerces();
    smsc::logger::Logger::Shutdown();
}

static void initDataCoding(ConfigView* config)
{
    uDBSmeDataCoding = DataCoding::LATIN1;

    static const char* STR_DATA_CODING_UCS2     = "UCS2";
    static const char* STR_DATA_CODING_LATIN1   = "LATIN1";
    static const char* STR_DATA_CODING_SMSC7BIT = "SMSC7BIT";

    try {
        const char* dcStr = config->getString("forceDataCoding");
        if (!dcStr || dcStr[0] == '\0') throw Exception("DataCoding is NULL.");

        if      (!strcmp(dcStr, STR_DATA_CODING_UCS2))     uDBSmeDataCoding = DataCoding::UCS2;
        else if (!strcmp(dcStr, STR_DATA_CODING_LATIN1))   uDBSmeDataCoding = DataCoding::LATIN1;
        else if (!strcmp(dcStr, STR_DATA_CODING_SMSC7BIT)) uDBSmeDataCoding = DataCoding::SMSC7BIT;
        else throw Exception("DataCoding %s is undefined.", dcStr);

    } catch (std::exception& exc) {
        smsc_log_warn(logger, "DataCoding is not defined properly using default LATIN1. Details: %s", exc.what());        
    } catch (...) {
        smsc_log_warn(logger, "DataCoding is not defined properly using default LATIN1.");        
    }
}

int main(void)
{
    using smsc::db::DataSourceLoader;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    int resultCode = 0;

    atexit(atExitHandler);
    smsc::logger::Logger::Init();
    logger = Logger::getInstance("smsc.dbsme.DBSme");
    adminListener.reset(new smsc::admin::service::ServiceSocketListener());

    SQLJobFactory _sqlJobFactory;
    JobFactory::registerFactory(&_sqlJobFactory,
                                SMSC_DBSME_SQL_JOB_IDENTITY);
    PLSQLJobFactory _plsqlJobFactory;
    JobFactory::registerFactory(&_plsqlJobFactory,
                                SMSC_DBSME_PLSQL_JOB_IDENTITY);

    try
    {
        smsc_log_info(logger, getStrVersion());
        Manager::init("config.xml");

        ConfigView dsConfig(Manager::getInstance(), "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);

        ConfigView cpConfig(Manager::getInstance(), "DBSme");
        initDataCoding(&cpConfig);
        CommandProcessor processor(&cpConfig);

        DBSmeAdminHandler adminHandler(processor);
        ConfigView adminConfig(Manager::getInstance(), "DBSme.Admin");
        DBSmeComponent admin(adminHandler);
        ComponentManager::registerComponent(&admin);
        adminListener->init(adminConfig.getString("host"), adminConfig.getInt("port"));
        bAdminListenerInited = true;
        adminListener->Start();

        while (!isNeedStop())
        {
            if (isNeedReinit())
            {
                processor.clean();
                Manager::reinit();

                DataSourceLoader::unload();
                ConfigView dsConfig(Manager::getInstance(), "StartupLoader");
                DataSourceLoader::loadup(&dsConfig);

                ConfigView cpConfig(Manager::getInstance(), "DBSme");
                initDataCoding(&cpConfig);
                processor.init(&cpConfig);

                setNeedReinit(false);
            }

            ConfigView mnConfig(Manager::getInstance(), "DBSme.ThreadPool");
            ConfigView ssConfig(Manager::getInstance(), "DBSme.SMSC");
            DBSmeConfig cfg(&ssConfig);

            DBSmeTaskManager runner(&mnConfig);
            DBSmePduListener listener(processor, runner);
            SmppSession      session(cfg, &listener);

            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set,smsc::system::SHUTDOWN_SIGNAL);
            sigset(smsc::system::SHUTDOWN_SIGNAL, appSignalHandler);

            smsc_log_info(logger, "Connecting to SMSC ... ");
            try
            {
                listener.setTrans(session.getSyncTransmitter());
                setNeedReconnect(false);
                session.connect();
            }
            catch (SmppConnectException& exc)
            {
                const char* msg = exc.what();
                smsc_log_error(logger, "Connect to SMSC failed. Cause: %s", (msg) ? msg:"unknown");
                setNeedReconnect(true);
                if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw;
                sleep(cfg.timeOut);
                session.close();
                runner.shutdown();
                continue;
            }
            smsc_log_info(logger, "Connected.");

            bDBSmeWaitEvent.Wait(0);
            while (!isNeedStop() && !isNeedReconnect() && !isNeedReinit())
            {
                bDBSmeWaitEvent.Wait();
                /*MutexGuard guard(countersLock);
                __trace2__("\nRequests: %llu processing, %llu processed.\n"
                           "Failures noticed: %llu\n",
                           requestsProcessingCount, requestsProcessedCount,
                           failuresNoticedCount);*/
            };
            smsc_log_info(logger, "Disconnecting from SMSC ...");
            session.close();
            runner.shutdown();
        };
    }
    catch (SmppConnectException& exc) {
        if (exc.getReason() == SmppConnectException::Reason::bindFailed)
            smsc_log_error(logger, "Failed to bind DBSme. Exiting.");
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

    if (bAdminListenerInited) {
        adminListener->shutdown();
        adminListener->WaitFor();
    }

    DataSourceLoader::unload();
    return resultCode;
}
