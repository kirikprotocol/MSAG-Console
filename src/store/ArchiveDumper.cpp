
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <system/smscsignalhandlers.h>

#include <sms/sms.h>
#include <util/xml/init.h>

#include "version.inc"
#include "ConnectionManager.h"
#include "FileStorage.h"

using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::system;
using namespace smsc::store;

static smsc::logger::Logger* logger = 0;

static Mutex needStopLock;
static bool  bServiceIsStopped = false;

static void setNeedStop(bool stop=true) {
    MutexGuard gauard(needStopLock);
    bServiceIsStopped = stop;
}
static bool isNeedStop() {
    MutexGuard gauard(needStopLock);
    return bServiceIsStopped;
}

class GetSmsStatement : public GetIdStatement
{
static const char* sql;
protected:

    uint8_t     uState;
    OCIDate     validTime, submitTime, lastTime, nextTime;

    int         bodyBufferLen;
    uint8_t     bodyBuffer[MAX_BODY_LENGTH];

    sb2         indSrcMsc, indSrcImsi, indSrcSme;
    sb2         indDstMsc, indDstImsi, indDstSme;
    sb2         indSvc, indBody, indLastTime, indNextTime;
    sb2         indRouteId, indSrcSmeId, indDstSmeId, indMsgRef;

    FullAddressValue    oa, da, dda;

public:

    GetSmsStatement(Connection* connection)
        throw(StorageException);
    virtual ~GetSmsStatement() {};

    void defineSms(SMS& sms)
        throw(StorageException);
    bool getSms(SMS& sms);
};

const char* GetSmsStatement::sql = (const char*)
"SELECT ID, ST, MR, OA, DA, DDA,\
 SRC_MSC, SRC_IMSI, SRC_SME_N, DST_MSC, DST_IMSI, DST_SME_N, VALID_TIME, SUBMIT_TIME,\
 ATTEMPTS, LAST_RESULT, LAST_TRY_TIME, NEXT_TRY_TIME, SVC_TYPE, DR, BR, BODY, BODY_LEN,\
 ROUTE_ID, SVC_ID, PRTY, SRC_SME_ID, DST_SME_ID FROM SMS_ARC ORDER BY LAST_TRY_TIME";
GetSmsStatement::GetSmsStatement(Connection* connection)
    throw(StorageException)
        : GetIdStatement(connection, GetSmsStatement::sql, false)
{
    smsc_log_debug(logger, "Retrive statement created");
}

void GetSmsStatement::defineSms(SMS& sms)
    throw(StorageException)
{
    ub4 i=2;
    define(i++, SQLT_UIN, (dvoid *) &(uState), (sb4) sizeof(uState));
    define(i++, SQLT_UIN, (dvoid *) &(sms.messageReference), (sb4) sizeof(sms.messageReference));

    define(i++, SQLT_STR, (dvoid *) (oa), (sb4) sizeof(oa));
    define(i++, SQLT_STR, (dvoid *) (da), (sb4) sizeof(da));
    define(i++, SQLT_STR, (dvoid *) (dda), (sb4) sizeof(dda));

    define(i++, SQLT_STR, (dvoid *) (sms.originatingDescriptor.msc),
           (sb4) sizeof(sms.originatingDescriptor.msc), (dvoid *)&(indSrcMsc));
    define(i++, SQLT_STR, (dvoid *) (sms.originatingDescriptor.imsi),
           (sb4) sizeof(sms.originatingDescriptor.imsi), (dvoid *)&(indSrcImsi));
    define(i++, SQLT_UIN, (dvoid *) &(sms.originatingDescriptor.sme),
           (sb4) sizeof(sms.originatingDescriptor.sme),  (dvoid *)&(indSrcSme));
    define(i++, SQLT_STR, (dvoid *) (sms.destinationDescriptor.msc),
           (sb4) sizeof(sms.destinationDescriptor.msc),  (dvoid *)&indDstMsc);
    define(i++, SQLT_STR, (dvoid *) (sms.destinationDescriptor.imsi),
           (sb4) sizeof(sms.destinationDescriptor.imsi), (dvoid *)&(indDstImsi));
    define(i++, SQLT_UIN, (dvoid *) &(sms.destinationDescriptor.sme),
           (sb4) sizeof(sms.destinationDescriptor.sme),  (dvoid *)&(indDstSme));
    define(i++, SQLT_ODT, (dvoid *) &(validTime),  (sb4) sizeof(validTime));
    define(i++, SQLT_ODT, (dvoid *) &(submitTime), (sb4) sizeof(submitTime));
    define(i++, SQLT_UIN, (dvoid *) &(sms.attempts),   (sb4) sizeof(sms.attempts));
    define(i++, SQLT_UIN, (dvoid *) &(sms.lastResult), (sb4) sizeof(sms.lastResult));
    define(i++, SQLT_ODT, (dvoid *) &(lastTime), (sb4) sizeof(lastTime),
           (dvoid *)&(indLastTime));
    define(i++, SQLT_ODT, (dvoid *) &(nextTime), (sb4) sizeof(nextTime),
           (dvoid *)&(indNextTime));
    define(i++, SQLT_STR, (dvoid *) &(sms.eServiceType), (sb4) sizeof(sms.eServiceType),
           (dvoid *) &(indSvc));
    define(i++, SQLT_UIN, (dvoid *) &(sms.deliveryReport), (sb4) sizeof(sms.deliveryReport));
    define(i++, SQLT_UIN, (dvoid *) &(sms.billingRecord), (sb4) sizeof(sms.billingRecord));
    define(i++, SQLT_BIN, (dvoid *) (bodyBuffer), (sb4) sizeof(bodyBuffer), &indBody);
    define(i++, SQLT_UIN, (dvoid *) &(bodyBufferLen), (sb4) sizeof(bodyBufferLen));

    define(i++, SQLT_STR, (dvoid *) (sms.routeId),   (sb4) sizeof(sms.routeId), &indRouteId);
    define(i++, SQLT_INT, (dvoid *)&(sms.serviceId), (sb4) sizeof(sms.serviceId));
    define(i++, SQLT_INT, (dvoid *)&(sms.priority),  (sb4) sizeof(sms.priority));

    define(i++, SQLT_STR, (dvoid *) (sms.srcSmeId),  (sb4) sizeof(sms.srcSmeId), &indSrcSmeId);
    define(i++, SQLT_STR, (dvoid *) (sms.dstSmeId),  (sb4) sizeof(sms.dstSmeId), &indDstSmeId);
}

bool GetSmsStatement::getSms(SMS& sms)
{
    sms.state = (State) uState;
    sms.needArchivate = true;

    convertStringToAddress(oa, sms.originatingAddress);
    convertStringToAddress(da, sms.destinationAddress);
    convertStringToAddress(dda, sms.dealiasedDestinationAddress);

    if (indSrcImsi == OCI_IND_NOTNULL) {
        sms.originatingDescriptor.imsiLength
            = strlen(sms.originatingDescriptor.imsi);
    } else {
        sms.originatingDescriptor.imsi[0] = '\0';
        sms.originatingDescriptor.imsiLength = 0;
    }
    if (indSrcMsc == OCI_IND_NOTNULL) {
        sms.originatingDescriptor.mscLength
            = strlen(sms.originatingDescriptor.msc);
    } else {
        sms.originatingDescriptor.msc[0] = '\0';
        sms.originatingDescriptor.mscLength = 0;
    }
    if (indDstImsi == OCI_IND_NOTNULL) {
        sms.destinationDescriptor.imsiLength
            = strlen(sms.destinationDescriptor.imsi);
    } else {
        sms.destinationDescriptor.imsi[0] = '\0';
        sms.destinationDescriptor.imsiLength = 0;
    }
    if (indDstMsc == OCI_IND_NOTNULL) {
        sms.destinationDescriptor.mscLength
            = strlen(sms.destinationDescriptor.msc);
    } else {
        sms.destinationDescriptor.msc[0] = '\0';
        sms.destinationDescriptor.mscLength = 0;
    }

    if (indSrcSme != OCI_IND_NOTNULL)   sms.originatingDescriptor.sme = 0;
    if (indDstSme != OCI_IND_NOTNULL)   sms.destinationDescriptor.sme = 0;
    if (indSvc != OCI_IND_NOTNULL)      sms.eServiceType[0] = '\0';
    if (indRouteId != OCI_IND_NOTNULL)  sms.routeId[0] = '\0';
    if (indSrcSmeId != OCI_IND_NOTNULL) sms.srcSmeId[0] = '\0';
    if (indDstSmeId != OCI_IND_NOTNULL) sms.dstSmeId[0] = '\0';
    if (indMsgRef != OCI_IND_NOTNULL) {
        sms.concatMsgRef = 0;
        sms.concatSeqNum = 0;
    }

    if (indLastTime != OCI_IND_NOTNULL) sms.lastTime = 0;
    else convertOCIDateToDate(&lastTime, &(sms.lastTime));
    if (indNextTime != OCI_IND_NOTNULL) sms.nextTime = 0;
    else convertOCIDateToDate(&nextTime, &(sms.nextTime));

    convertOCIDateToDate(&submitTime, &(sms.submitTime));
    convertOCIDateToDate(&validTime, &(sms.validTime));

    bool result = (bodyBufferLen <= MAX_BODY_LENGTH);
    if (indBody != OCI_IND_NOTNULL || bodyBufferLen == 0) {
        sms.messageBody.setBuffer(0,0);
    } else {
        uint8_t* setBuff = new uint8_t[bodyBufferLen];
        memcpy(setBuff, bodyBuffer, bodyBufferLen);
        sms.messageBody.setBuffer(setBuff, bodyBufferLen);
    }
    return result;
}


extern "C" void signalHandler(int sig)
{
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT) {
        smsc_log_info(logger, "Stopping ...");
        setNeedStop(true);
    }
}
extern "C" void atExitHandler(void)
{
    smsc::util::xml::TerminateXerces();
}

static int maxSmsPerFile = 1000;
static std::string arcDestination;
static std::string dbInstance;
static std::string dbUserName;
static std::string dbUserPassword;

static void initDumper(ConfigView* config)
{
    arcDestination = config->getString("destination");
    maxSmsPerFile = config->getInt("maxSmsPerFile");
    if (maxSmsPerFile <= 0)
        throw ConfigException("Parameter 'maxSmsPerFile' is invalid! Should be positive.");

    std::auto_ptr<ConfigView> storageCfgGuard(config->getSubConfig("Storage"));
    ConfigView* storageCfg = storageCfgGuard.get();
    
    dbInstance = storageCfg->getString("dbInstance");
    dbUserName = storageCfg->getString("dbUserName");
    dbUserPassword = storageCfg->getString("dbUserPassword");
}

int main(void)
{
    Logger::Init();
    logger = Logger::getInstance("smsc.store.ArchiveDumper");

    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    atexit(atExitHandler);
    
    sigset_t set; sigemptyset(&set);
    sigaddset(&set, SIGINT); sigaddset(&set, SIGTERM);
    sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
    pthread_sigmask(SIG_SETMASK, &set, NULL);
    sigset(SIGINT, signalHandler); sigset(SIGTERM, signalHandler);
    sigset(smsc::system::SHUTDOWN_SIGNAL, signalHandler);
    
    PersistentStorage* archiveStorage = 0;

    int resultCode = 0;
    try 
    {
        smsc_log_info(logger, getStrVersion());
        Manager::init("dumper.xml");
        
        ConfigView apConfig(Manager::getInstance(), "ArchiveDumper");
        initDumper(&apConfig);

        // TODO: open DB connection & prepare SELECT statement
        std::auto_ptr<Connection> connectionGuard(new Connection(dbInstance.c_str(), 
                                                                 dbUserName.c_str(),
                                                                 dbUserPassword.c_str()));
        Connection* connection = connectionGuard.get();
        if (!connection)
            throw Exception("Failed to create DB connection.");
        connection->connect();
        std::auto_ptr<GetSmsStatement> getSmsStatementGuard(new GetSmsStatement(connection));
        GetSmsStatement* getSmsStatement = getSmsStatementGuard.get();
        std::auto_ptr<GetBodyStatement> getBodyStatementGuard(new GetBodyStatement(connection, false));
        GetBodyStatement* getBodyStatement = getBodyStatementGuard.get();
        if (!getSmsStatement || !getBodyStatement)
            throw Exception("Failed to create DB statement(s).");
        
        SMSId smsId = 0; SMS sms;
        getSmsStatement->defineSms(sms);
        sword status = getSmsStatement->execute(OCI_DEFAULT, 1, 0);

        smsc_log_info(logger, "Dumping messages from archive ...");
        uint64_t totalMessages = 0;

        while (!isNeedStop() && status != OCI_NO_DATA)
        {
            connection->check(status);
            getSmsStatement->getSMSId(smsId);
            if (!getSmsStatement->getSms(sms))
            {
                getBodyStatement->setSMSId(smsId);
                getBodyStatement->getBody(sms.getMessageBody());
                connection->commit();
            }
            
            if (!archiveStorage || !(totalMessages%maxSmsPerFile))
            {
                if (archiveStorage) delete archiveStorage;
                archiveStorage = 0;

                char arcFileName[256];
                sprintf(arcFileName, "%08lld_%08lld.arc", totalMessages, totalMessages+maxSmsPerFile);
                archiveStorage = new PersistentStorage(arcDestination, arcFileName);
                smsc_log_debug(logger, "Created '%s' file", arcFileName);
            }
            archiveStorage->writeRecord(smsId, sms, 0);
            
            totalMessages++;
            status = getSmsStatement->fetch();
        }

        smsc_log_info(logger, "Total %lld messages dumped.", totalMessages);

    } catch (ConfigException& exc) {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -1;
    } catch (Exception& exc) {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -2;
    } catch (std::exception& exc) {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -3;
    } catch (...) {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -4;
    }

    if (archiveStorage) delete archiveStorage;
    return resultCode;
}
