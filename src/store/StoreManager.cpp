#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <oci.h>
#include <orl.h>

#include <util/debug.h>
#include <system/status.h>
#include <util/csv/CSVFileEncoder.h>
#include <smpp/smpp_structures.h>

#include "StoreManager.h"

#define __FUNCTION__ __func__

namespace smsc { namespace store
{

/* ----------------------------- StoreManager -------------------------- */

using namespace smsc::sms;
using namespace smsc::system;
using smsc::logger::Logger;
using smsc::util::config::Manager;
using smsc::smpp::DataCoding;

using namespace smsc::util::csv;

const unsigned SMSC_MAX_TRIES_TO_PROCESS_OPERATION = 3;
const unsigned SMSC_MAX_TRIES_TO_PROCESS_OPERATION_LIMIT = 1000;

const char* SMSC_LAST_BILLING_FILE_EXTENSION = "lst";
const char* SMSC_PREV_BILLING_FILE_EXTENSION = "csv";

const unsigned SMSC_MIN_BILLING_INTERVAL = 10;

Mutex        StoreManager::mutex;
Cleaner*     StoreManager::cleaner = 0;
RemoteStore* StoreManager::instance  = 0;

smsc::logger::Logger *StoreManager::log = Logger::getInstance("smsc.store.StoreManager");
smsc::logger::Logger *RemoteStore::log = Logger::getInstance("smsc.store.RemoteStore");
smsc::logger::Logger *CachedStore::log = Logger::getInstance("smsc.store.CachedStore");

#ifdef SMSC_FAKE_MEMORY_MESSAGE_STORE
IntHash<SMS*> RemoteStore::fakeStore(100000);
Mutex RemoteStore::fakeMutex;
#endif

/* ------------------------------ Store Manager ----------------------------- */
bool StoreManager::needCache(Manager& config)
{
    bool cacheIsNeeded = false;
    try
    {
        cacheIsNeeded = config.getBool("MessageStore.Cache.enabled");
    }
    catch (ConfigException& exc)
    {
        smsc_log_warn(log, "Config parameter: <MessageStore.Cache.enabled> missed. "
                 "Cache disabled.");
    }
    return cacheIsNeeded;
}
bool StoreManager::needCleaner(Manager& config)
{
    bool cleanerIsNeeded = false;
    try
    {
        cleanerIsNeeded = config.getBool("MessageStore.Cleaner.enabled");
    }
    catch (ConfigException& exc)
    {
        smsc_log_warn(log, "Config parameter: <MessageStore.Cleaner.enabled> missed. "
                 "Cleaner disabled.");
    }
    return cleanerIsNeeded;
}
void StoreManager::startup(Manager& config, SchedTimer* sched)
    throw(ConfigException, ConnectionFailedException)
{
    MutexGuard guard(mutex);

    if (!instance)
    {
        smsc_log_info(log, "Storage Manager is starting ... ");
        try
        {

#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

            instance = (needCache(config)) ?
                        new CachedStore(config, sched) :
                        new RemoteStore(config, sched);

            cleaner = new Cleaner(config);
            if (needCleaner(config)) cleaner->Start();
#else
            cleaner = new Cleaner(config);
            instance = new RemoteStore(config, sched);
#endif
        }
        catch (StorageException& exc)
        {
            if (instance) { delete instance; instance = 0; }
            if (cleaner) { delete cleaner; cleaner = 0; }
            throw ConnectionFailedException(exc);
        }
        smsc_log_info(log, "Storage Manager was started up.");
    }
}

void StoreManager::shutdown()
{
    MutexGuard guard(mutex);

    if (instance)
    {
        smsc_log_info(log, "Storage Manager is shutting down ...");
        delete instance; instance = 0;
        smsc_log_info(log, "Storage Manager shutting down cleaner...");
        if (cleaner) delete cleaner; cleaner = 0;
        smsc_log_info(log, "Storage Manager was shutdowned.");
    }
}
/* ------------------------------ Store Manager ----------------------------- */

/* ------------------------------ Remote Store ------------------------------ */
void RemoteStore::loadMaxTriesCount(Manager& config)
{
    try
    {
        maxTriesCount = (unsigned)config.getInt("MessageStore.maxTriesCount");
        if (!maxTriesCount ||
            maxTriesCount > SMSC_MAX_TRIES_TO_PROCESS_OPERATION_LIMIT)
        {
            maxTriesCount = SMSC_MAX_TRIES_TO_PROCESS_OPERATION;
            smsc_log_warn(log, "Max tries count to process operation on MessageStore "
                     "is incorrect (should be between 1 and %u) ! "
                     "Config parameter: <MessageStore.maxTriesCount> "
                     "Using default: %u",
                     SMSC_MAX_TRIES_TO_PROCESS_OPERATION_LIMIT,
                     SMSC_MAX_TRIES_TO_PROCESS_OPERATION);
        }
    }
    catch (ConfigException& exc)
    {
        maxTriesCount = SMSC_MAX_TRIES_TO_PROCESS_OPERATION;
        smsc_log_warn(log, "Max tries count to process operation on MessageStore "
                 "wasn't specified ! "
                 "Config parameter: <MessageStore.maxTriesCount> "
                 "Using default: %d",
                 SMSC_MAX_TRIES_TO_PROCESS_OPERATION);
    }
}

RemoteStore::RemoteStore(Manager& config, SchedTimer* sched)
    throw(ConfigException, StorageException)
        : Thread(), bStarted(false), bNeedExit(false), billingFile(0), billingInterval(0),
            pool(0), scheduleTimer(sched), maxTriesCount(SMSC_MAX_TRIES_TO_PROCESS_OPERATION)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE
    loadMaxTriesCount(config);
    pool = new StorageConnectionPool(config);
#endif
    initBilling(config);
    Start();
}
RemoteStore::~RemoteStore()
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE
    smsc_log_info(log,  "RemoteStore: Destroying connection pool" );
    if (pool) delete pool;
    smsc_log_info(log,  "RemoteStore: Connection pool destroyed" );
#endif

    Stop();
}

SMSId RemoteStore::getNextId()
    throw(StorageException)
{
    return StoreManager::getNextId();
}

SMSId RemoteStore::doCreateSms(StorageConnection* connection,
    SMS& sms, SMSId id, const CreateMode flag)
        throw(StorageException, DuplicateMessageException)
{
    __require__(connection);

    Descriptor  dsc;
    sms.state = ENROUTE;
    sms.destinationDescriptor = dsc;
    sms.lastTime = 0; sms.lastResult = 0; sms.attempts = 0;

    if (flag == SMPP_OVERWRITE_IF_PRESENT)
    {
        NeedOverwriteStatement* needOverwriteStmt;

        if (!sms.eServiceType[0])
        {
            needOverwriteStmt = connection->getNeedOverwriteStatement();
        }
        else
        {
            needOverwriteStmt = connection->getNeedOverwriteSvcStatement();
            ((NeedOverwriteSvcStatement *)needOverwriteStmt)->
                bindEServiceType((dvoid *) sms.eServiceType,
                                 (sb4) sizeof(sms.eServiceType));

        }
        needOverwriteStmt->bindOriginatingAddress(sms.originatingAddress);
        needOverwriteStmt->bindDestinationAddress(sms.destinationAddress);

        sword result = needOverwriteStmt->execute();
        if (result != OCI_NO_DATA)
        {
            connection->check(result);
            SMSId retId;

            try
            {
                needOverwriteStmt->getId(retId);

                OverwriteStatement* overwriteStmt
                    = connection->getOverwriteStatement();
                DestroyBodyStatement* destroyBodyStmt
                    = connection->getDestroyBodyStatement();

                destroyBodyStmt->setSMSId(retId);
                destroyBodyStmt->destroyBody();

                overwriteStmt->bindOldId(retId);
                overwriteStmt->bindNewId(id);
                overwriteStmt->bindSms(sms);

                connection->check(overwriteStmt->execute());

                int bodyLen = sms.getMessageBody().getBufferLength();
                if (bodyLen > MAX_BODY_LENGTH)
                {
                    SetBodyStatement* setBodyStmt
                        = connection->getSetBodyStatement();

                    setBodyStmt->setSMSId(id);
                    setBodyStmt->setBody(sms.getMessageBody());
                }

                connection->commit();
            }
            catch (StorageException& exc)
            {
                try { connection->rollback(); } catch (...) {
                    smsc_log_error(log, "Failed to rollback");
                }
                throw exc;
            }
            catch (...) {
                try { connection->rollback(); } catch (...) {
                    smsc_log_error(log, "Failed to rollback");
                }
                throw StorageException("unknown exception");
            }
            return retId;
        }
    }
    else if (flag == ETSI_REJECT_IF_PRESENT)
    {
        NeedRejectStatement* needRejectStmt
            = connection->getNeedRejectStatement();

        needRejectStmt->bindOriginatingAddress(sms.originatingAddress);
        needRejectStmt->bindDestinationAddress(sms.destinationAddress);
        needRejectStmt->bindMr((dvoid *)&(sms.messageReference),
                               (sb4) sizeof(sms.messageReference));

        connection->check(needRejectStmt->execute());
        if (needRejectStmt->needReject())
        {
            throw DuplicateMessageException();
        }
    }

    StoreStatement* storeStmt
        = connection->getStoreStatement();

    storeStmt->bindId(id);
    storeStmt->bindSms(sms);
    try
    {
        connection->check(storeStmt->execute(OCI_DEFAULT));

        int bodyLen = sms.getMessageBody().getBufferLength();
        if (bodyLen > MAX_BODY_LENGTH)
        {
            SetBodyStatement* setBodyStmt
                = connection->getSetBodyStatement();

            setBodyStmt->setSMSId(id);
            setBodyStmt->setBody(sms.getMessageBody());
        }

        connection->commit();
    }
    catch (StorageException& exc)
    {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw exc;
    }
    catch (...) {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw StorageException("unknown exception");
    }

    return id;
}
SMSId RemoteStore::createSms(SMS& sms, SMSId id, const CreateMode flag)
    throw(StorageException, DuplicateMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    __require__(pool);

    SMSId retId = id;

    StorageConnection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try
        {
            connection = (StorageConnection *)pool->getConnection();
            if (connection)
            {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
                retId = doCreateSms(connection, sms, id, flag);
                pool->freeConnection(connection);
            }
            break;
        }
        catch (DuplicateMessageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount)
            {
                smsc_log_warn(log, "Max tries count to store message "
                         "#%lld exceeded !\n", id);
                throw;
            }
        }
    }

    return retId;

#else

    Descriptor  dsc;
    sms.state = ENROUTE;
    sms.destinationDescriptor = dsc;
    sms.lastTime = 0; sms.lastResult = 0; sms.attempts = 0;

    MutexGuard guard(fakeMutex);

    if (flag == SMPP_OVERWRITE_IF_PRESENT && fakeStore.Exist(id))
    {
        delete fakeStore.Get(id);
        fakeStore.Delete(id);
    }
    else if (flag == ETSI_REJECT_IF_PRESENT && fakeStore.Exist(id))
    {
        throw DuplicateMessageException();
    }

    fakeStore.Insert(id, new SMS(sms));
    return id;
#endif
}

void RemoteStore::doChangeSmsConcatSequenceNumber(
    StorageConnection* connection, SMSId id, int8_t inc)
        throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    UpdateSeqNumStatement* seqNumStmt =
        connection->getUpdateSeqNumStatement();

    seqNumStmt->bindId(id);
    seqNumStmt->bindInc(inc);

    try
    {
        connection->check(seqNumStmt->execute());
    }
    catch (StorageException& exc)
    {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw exc;
    }

    if (!seqNumStmt->wasUpdated())
    {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw NoSuchMessageException(id);
    }
    connection->commit();
}

void RemoteStore::changeSmsConcatSequenceNumber(SMSId id, int8_t inc)
    throw(StorageException, NoSuchMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    __require__(pool);

    StorageConnection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try
        {
            connection = (StorageConnection *)pool->getConnection();
            if (connection)
            {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
                doChangeSmsConcatSequenceNumber(connection, id, inc);
                pool->freeConnection(connection);
            }
            break;
        }
        catch (NoSuchMessageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount)
            {
                smsc_log_warn(log, "Max tries count to change "
                         "sequence number in concatenneted message "
                         "#%lld exceeded !\n", id);
                throw;
            }
        }
    }

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id))
        throw NoSuchMessageException(id);

    SMS* sms = fakeStore.Get(id);
    if (sms)
    {
        if (!sms->hasBinProperty(Tag::Tag::SMSC_CONCATINFO))
            throw NoSuchMessageException(id);

        sms->concatSeqNum += inc;
    }
    else throw NoSuchMessageException(id);

#endif
}


void RemoteStore::doRetrieveSms(StorageConnection* connection,
    SMSId id, SMS &sms)
        throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    RetrieveStatement* retriveStmt
        = connection->getRetrieveStatement();

    retriveStmt->bindId(id);
    retriveStmt->defineSms(sms);
    sword status = retriveStmt->execute(OCI_DEFAULT);
    if (status == OCI_NO_DATA)
    {
        throw NoSuchMessageException(id);
    }
    else
    {
        retriveStmt->check(status);
        if (!retriveStmt->getSms(sms)) // Need to load up attachment
        {
            GetBodyStatement* getBodyStmt
                = connection->getGetBodyStatement();

            getBodyStmt->setSMSId(id);
            getBodyStmt->getBody(sms.getMessageBody());

            connection->commit();
        }
    }
}
void RemoteStore::retriveSms(SMSId id, SMS &sms)
    throw(StorageException, NoSuchMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    __require__(pool);

    StorageConnection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try
        {
            connection = (StorageConnection *)pool->getConnection();
            if (connection)
            {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
                doRetrieveSms(connection, id, sms);
                pool->freeConnection(connection);
            }
            break;
        }
        catch (NoSuchMessageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount)
            {
                smsc_log_warn(log, "Max tries count to retrive message "
                         "#%lld exceeded !\n", id);
                throw;
            }
        }
    }

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id))
        throw NoSuchMessageException(id);

    sms = *fakeStore.Get(id);

#endif
}

void RemoteStore::doDestroySms(StorageConnection* connection, SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    DestroyStatement* destroyStmt
        = connection->getDestroyStatement();
    DestroyBodyStatement* destroyBodyStmt
        = connection->getDestroyBodyStatement();

    try
    {
        destroyStmt->bindId(id);
        connection->check(destroyStmt->execute());
        if (!destroyStmt->wasDestroyed()) throw NoSuchMessageException(id);
        destroyBodyStmt->setSMSId(id);
        destroyBodyStmt->destroyBody();
    }
    catch (StorageException& exc)
    {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw;
    }

    connection->commit();
}
void RemoteStore::destroySms(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    __require__(pool);

    StorageConnection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try
        {
            connection = (StorageConnection *)pool->getConnection();
            if (connection)
            {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
                doDestroySms(connection, id);
                pool->freeConnection(connection);
            }
            break;
        }
        catch (NoSuchMessageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount)
            {
                smsc_log_warn(log, "Max tries count to remove message "
                         "#%lld exceeded !\n", id);
                throw;
            }
        }
    }

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id))
        throw NoSuchMessageException(id);

    delete fakeStore.Get(id);
    fakeStore.Delete(id);

#endif
}

void RemoteStore::doReplaceSms(StorageConnection* connection,
    SMSId id, const Address& oa,
    const uint8_t* newMsg, uint8_t newMsgLen,
    uint8_t deliveryReport, time_t validTime, time_t waitTime)
        throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    Body    body;
    ReplaceStatement* replaceStmt = 0;
    RetrieveBodyStatement* retrieveBodyStmt
        = connection->getRetrieveBodyStatement();
    try
    {
        retrieveBodyStmt->bindId(id);
        retrieveBodyStmt->bindOriginatingAddress(oa);

        sword status = retrieveBodyStmt->execute();
        if (status == OCI_NO_DATA) throw NoSuchMessageException(id);
        else connection->check(status);

        if (!retrieveBodyStmt->getBody(body) ||
             retrieveBodyStmt->getBodyLength() > MAX_BODY_LENGTH)
        {
            GetBodyStatement* getBodyStmt
                = connection->getGetBodyStatement();

            getBodyStmt->setSMSId(id);
            getBodyStmt->getBody(body);
            //connection->commit(); // Need to reset BLOB (SELECT FOR UPDATE)

            DestroyBodyStatement* destroyBodyStmt
                = connection->getDestroyBodyStatement();

            destroyBodyStmt->setSMSId(id);
            destroyBodyStmt->destroyBody();
            //connection->commit();
        }

        try {
            body.setBinProperty(Tag::SMPP_SHORT_MESSAGE, (const char*)newMsg,
                                (unsigned)newMsgLen);
            body.setIntProperty(Tag::SMPP_SM_LENGTH, (uint32_t)newMsgLen);
        } catch (...) {
            throw StorageException("Incorrect Sms body data. Set/Get Property failed!");
        }

        if (waitTime == 0 && validTime == 0) {
            replaceStmt = connection->getReplaceStatement();
        } else if (waitTime == 0) {
            replaceStmt = connection->getReplaceVTStatement();
            ((ReplaceVTStatement *)replaceStmt)->bindValidTime(validTime);
        } else if (validTime == 0) {
            replaceStmt = connection->getReplaceWTStatement();
            ((ReplaceWTStatement *)replaceStmt)->bindWaitTime(waitTime);
        } else {
            replaceStmt = connection->getReplaceVWTStatement();
            ((ReplaceVWTStatement *)replaceStmt)->bindWaitTime(waitTime);
            ((ReplaceVWTStatement *)replaceStmt)->bindValidTime(validTime);
        }

        replaceStmt->bindId(id);
        replaceStmt->bindOriginatingAddress((Address&) oa);
        replaceStmt->bindBody(body);
        replaceStmt->bindDeliveryReport((dvoid *) &deliveryReport,
                                        (sb4) sizeof(deliveryReport));

        connection->check(replaceStmt->execute());

        int bodyLen = body.getBufferLength();
        if (bodyLen > MAX_BODY_LENGTH)
        {
            SetBodyStatement* setBodyStmt
                = connection->getSetBodyStatement();

            setBodyStmt->setSMSId(id);
            setBodyStmt->setBody(body);
        }
    }
    catch (StorageException& exc)
    {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw;
    }

    if (!replaceStmt->wasReplaced())
    {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw NoSuchMessageException(id);
    }
    connection->commit();
}
void RemoteStore::replaceSms(SMSId id, const Address& oa,
    const uint8_t* newMsg, uint8_t newMsgLen,
    uint8_t deliveryReport, time_t validTime, time_t waitTime)
        throw(StorageException, NoSuchMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    __require__(pool);

    StorageConnection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try
        {
            connection = (StorageConnection *)pool->getConnection();
            if (connection)
            {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
                doReplaceSms(connection, id, oa, newMsg, newMsgLen,
                             deliveryReport, validTime, waitTime);
                pool->freeConnection(connection);
            }
            break;
        }
        catch (NoSuchMessageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount)
            {
                smsc_log_warn(log, "Max tries count to replace message "
                         "#%lld exceeded !\n", id);
                throw;
            }
        }
    }

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id))
        throw NoSuchMessageException(id);

    SMS *sms=fakeStore.Get(id);

    sms->setOriginatingAddress(oa);
    sms->setDeliveryReport(deliveryReport);
    sms->validTime = validTime;
    sms->nextTime = waitTime;
    sms->attempts = 0;

    try
    {
        sms->getMessageBody().setBinProperty(Tag::SMPP_SHORT_MESSAGE,
            (const char*)newMsg, (unsigned)newMsgLen);
        sms->getMessageBody().setIntProperty(Tag::SMPP_SM_LENGTH,
            (uint32_t)newMsgLen);
    }
    catch (...) {
        throw StorageException("Incorrect Sms body data. Set/Get Property failed!");
    }

    //fakeStore.Insert(id, new SMS(sms));

#endif
}

void RemoteStore::doReplaceSms(StorageConnection* connection, SMSId id, SMS& sms)
    throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    ReplaceAllStatement* replaceStmt
        = connection->getReplaceAllStatement();
    RetrieveBodyStatement* retrieveBodyStmt
        = connection->getRetrieveBodyStatement();

    try
    {
        retrieveBodyStmt->bindId(id);
        retrieveBodyStmt->bindOriginatingAddress(sms.getOriginatingAddress());

        sword status = retrieveBodyStmt->execute();
        if (status == OCI_NO_DATA) throw NoSuchMessageException(id);
        else connection->check(status);

        {
            Body    oldBody;
            if (!retrieveBodyStmt->getBody(oldBody) ||
                 retrieveBodyStmt->getBodyLength() > MAX_BODY_LENGTH)
            {
                DestroyBodyStatement* destroyBodyStmt
                    = connection->getDestroyBodyStatement();

                destroyBodyStmt->setSMSId(id);
                destroyBodyStmt->destroyBody();
            }
        }

        Body& body = sms.getMessageBody();
        replaceStmt->bindId(id);
        replaceStmt->bindSms(sms);

        connection->check(replaceStmt->execute());

        int bodyLen = body.getBufferLength();
        if (bodyLen > MAX_BODY_LENGTH)
        {
            SetBodyStatement* setBodyStmt
                = connection->getSetBodyStatement();

            setBodyStmt->setSMSId(id);
            setBodyStmt->setBody(body);
        }
    }
    catch (StorageException& exc)
    {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw;
    }

    if (!replaceStmt->wasReplaced())
    {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw NoSuchMessageException(id);
    }
    connection->commit();
}

void RemoteStore::replaceSms(SMSId id, SMS& sms)
    throw(StorageException, NoSuchMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    __require__(pool);

    StorageConnection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try
        {
            connection = (StorageConnection *)pool->getConnection();
            if (connection)
            {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
                doReplaceSms(connection, id, sms);
                pool->freeConnection(connection);
            }
            break;
        }
        catch (NoSuchMessageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount)
            {
                smsc_log_warn(log, "Max tries count to replace message "
                         "#%lld exceeded !\n", id);
                throw;
            }
        }
    }

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id))
        throw NoSuchMessageException(id);

    delete fakeStore.Get(id);
    fakeStore.Delete(id);
    fakeStore.Insert(id, new SMS(sms));

#endif
}


void RemoteStore::doChangeSmsStateToEnroute(StorageConnection* connection,
    SMSId id, const Descriptor& dst, uint32_t failureCause, time_t nextTryTime,
        uint32_t attempts)
            throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    ToEnrouteStatement* toEnrouteStmt
        = connection->getToEnrouteStatement();

    toEnrouteStmt->bindId(id);
    toEnrouteStmt->bindNextTime(nextTryTime);
    toEnrouteStmt->bindFailureCause((dvoid *)&(failureCause),
                                    (sb4) sizeof(failureCause));
    toEnrouteStmt->bindDestinationDescriptor((Descriptor &)dst);
    toEnrouteStmt->bindAttempts(attempts);

    try
    {
        connection->check(toEnrouteStmt->execute());
    }
    catch (StorageException& exc)
    {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw exc;
    }

    if (!toEnrouteStmt->wasUpdated())
    {
        try { connection->rollback(); } catch (...) {
            smsc_log_error(log, "Failed to rollback");
        }
        throw NoSuchMessageException(id);
    }
    connection->commit();
}
void RemoteStore::changeSmsStateToEnroute(SMSId id,
    const Descriptor& dst, uint32_t failureCause, time_t nextTryTime,
        uint32_t attempts)
            throw(StorageException, NoSuchMessageException)
{
    if (scheduleTimer)
        scheduleTimer->ChangeSmsSchedule(id, nextTryTime);

#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    __require__(pool);

    StorageConnection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try
        {
            connection = (StorageConnection *)pool->getConnection();
            if (connection)
            {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
                doChangeSmsStateToEnroute(connection, id, dst,
                                          failureCause, nextTryTime, attempts);
                pool->freeConnection(connection);
            }
            break;
        }
        catch (NoSuchMessageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            throw;
        }
        catch (StorageException& exc)
        {
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount)
            {
                smsc_log_warn(log, "Max tries count to update message state"
                         "#%lld exceeded !\n", id);
                throw;
            }
        }
    }

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id))
        throw NoSuchMessageException(id);

    SMS *sms=fakeStore.Get(id);
    if (sms->getState() != ENROUTE)
        throw NoSuchMessageException(id);

    sms->destinationDescriptor = dst;
    sms->lastResult = failureCause;
    sms->nextTime = nextTryTime;
    sms->attempts = attempts;

    //fakeStore.Insert(id, sms);

#endif
}

static void findFiles(std::string location, const char* ext, Array<std::string>& files)
    throw(StorageException)
{
    if (!ext || !ext[0]) return;
    int extFileLen  = strlen(ext) + 1;
    const char* locationStr = location.c_str();

    DIR *locationDir = 0;
    if (!(locationDir = opendir(locationStr))) {
      Exception exc("Failed to open directory '%s'. Details: %s",
                      locationStr, strerror(errno));
        throw StorageException(exc.what());
    }

    char entry[4096];//sizeof(struct dirent)+pathconf(locationStr, _PC_NAME_MAX)];
    //printf("Max name len=%d\n", pathconf(locationStr, _PC_NAME_MAX));
    struct dirent* pentry = 0;

    while (locationDir)
    {
        errno = 0;
        int result = readdir_r(locationDir, (struct dirent *)entry, &pentry);
        if (!result && pentry != NULL)
        {
            std::string fileName = location;
            fileName += '/'; fileName += pentry->d_name;
            struct stat description;
            if (stat(fileName.c_str(), &description) != 0) {
                Exception exc("Failed to obtain '%s' file info. Details: %s",
                              fileName.c_str(), strerror(errno));
                if (locationDir) closedir(locationDir);
                throw StorageException(exc.what());
            }
            //printf("%s\tmode:%d\n", pentry->d_name, description.st_mode);
            if (!(description.st_mode & S_IFDIR)) {
                int fileNameLen = strlen(pentry->d_name);
                if (fileNameLen > extFileLen)
                {
                    const char* extPos = pentry->d_name+(fileNameLen-extFileLen);
                    if ((*extPos == '.') && !strcmp(extPos+1, ext)) files.Push(pentry->d_name);
                }
            }
        }
        else
        {
            if (errno == 0) break;
            Exception exc("Failed to scan directory '%s' contents. Details: %s",
                          locationStr, strerror(errno));
            if (locationDir) closedir(locationDir);
            throw StorageException(exc.what());
        }
    }

    if (locationDir) closedir(locationDir);
}
static void changeFileExtension(std::string location, const char* fileName)
    throw(StorageException)
{
    std::string fullOldFile = location; fullOldFile += '/'; fullOldFile += fileName;
    std::string fullNewFile = location; fullNewFile += '/'; fullNewFile += fileName;
    fullOldFile += '.'; fullOldFile += SMSC_LAST_BILLING_FILE_EXTENSION;
    fullNewFile += '.'; fullNewFile += SMSC_PREV_BILLING_FILE_EXTENSION;
    if (rename(fullOldFile.c_str(), fullNewFile.c_str()) != 0) {
        Exception exc("Failed to rename file '%s' to '%s'. Details: %s",
                      fullOldFile.c_str(), fullNewFile.c_str(), strerror(errno));
        throw StorageException(exc.what());
    }
}
void RemoteStore::initBilling(Manager& config)
    throw(ConfigException, StorageException)
{
    billingLocation = config.getString("MessageStore.billingDir");
    int bi = config.getInt("MessageStore.billingInterval");
    if (bi < SMSC_MIN_BILLING_INTERVAL) {
        throw ConfigException("Parameter 'billingInterval' should be more than %u seconds",
                              SMSC_MIN_BILLING_INTERVAL);
    }
    else billingInterval = bi;

    Array<std::string> files;
    findFiles(billingLocation, SMSC_LAST_BILLING_FILE_EXTENSION, files);
    int extLen = strlen(SMSC_LAST_BILLING_FILE_EXTENSION)+1;

    for (int i=0; i<files.Count(); i++)
    {
        std::string file = files[i];
        int fileNameLen = file.length();
        const char* fileNameStr = file.c_str();
        smsc_log_debug(log, "Found old billing file: %s", fileNameStr);

        char fileName[4096];//fileNameLen];
        strncpy(fileName, fileNameStr, fileNameLen-extLen);
        fileName[fileNameLen-extLen] = '\0';
        changeFileExtension(billingLocation, fileName);
    }
}

// MUST be executed under billingFileLock
void RemoteStore::openBillingFile()
    throw(StorageException)
{
    if (!billingFile)
    {
        time_t current = time(NULL);
        tm dt; gmtime_r(&current, &dt);
        const char* billingFileNamePattern = "%04d%02d%02d_%02d%02d%02d";
        sprintf(billingFileName, billingFileNamePattern,
                dt.tm_year+1900, dt.tm_mon+1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);

        std::string fullFilePath = billingLocation;
        fullFilePath += '/'; fullFilePath += (const char*)billingFileName;
        fullFilePath += '.'; fullFilePath += SMSC_LAST_BILLING_FILE_EXTENSION;
        const char* fullFilePathStr = fullFilePath.c_str();
        billingFile = fopen(fullFilePathStr, "r");

        bool needFile = true;
        if (billingFile) { // file exists
            fclose(billingFile); billingFile = 0;
            needFile = false;
        }

        billingFile = fopen(fullFilePathStr, "a+");
        if (!billingFile) {
            Exception exc("Failed to create billing file '%s'. Details: %s",
                          fullFilePathStr, strerror(errno));
            throw StorageException(exc.what());
        }

        if (needFile) {
            writeToBillingFile("MSG_ID,RECORD_TYPE,MEDIA_TYPE,BEARER_TYPE,SUBMIT,FINALIZED,STATUS,"
                               "SRC_ADDR,SRC_IMSI,SRC_MSC,SRC_SME_ID,DST_ADDR,DST_IMSI,DST_MSC,DST_SME_ID,"
                               "DIVERTED_FOR,ROUTE_ID,SERVICE_ID,USER_MSG_REF,DATA_LENGTH\n");
        }
    }
}
// MUST be executed under billingFileLock
void RemoteStore::writeToBillingFile(std::string out)
    throw(StorageException)
{
    if (fwrite(out.c_str(), out.length(), 1, billingFile) != 1) {
        Exception exc("Failed to write to billing file. Details: %s", strerror(errno));
        fclose(billingFile); billingFile = 0;
        throw StorageException(exc.what());
    }
    if (fflush(billingFile)) {
        Exception exc("Failed to flush billing file. Details: %s", strerror(errno));
        fclose(billingFile); billingFile = 0;
        throw StorageException(exc.what());
    }
}

int RemoteStore::Execute()
{
    while (!bNeedExit)
    {
        const int SERVICE_SLEEP = 3600;   // in seconds
        uint32_t toSleep = billingInterval;
        while (toSleep > 0 && !bNeedExit)
        {
            if (toSleep > SERVICE_SLEEP) {
                toSleep -= SERVICE_SLEEP;
                awake.Wait(SERVICE_SLEEP*1000);
            }
            else {
                awake.Wait(toSleep*1000);
                break;
            }
        }

        try
        {
            MutexGuard guard(billingFileLock);
            if (billingFile)
            {
                fclose(billingFile); billingFile = 0;
                changeFileExtension(billingLocation, billingFileName);
            }
        }
        catch (StorageException& exc)
        {
            awake.Wait(0); smsc_log_error(log, "%s", exc.what());
        }
    }

    MutexGuard guard(billingFileLock);
    if (billingFile) {
        fclose(billingFile); billingFile = 0;
    }

    exited.Signal();
    return 0;
}
void RemoteStore::Start()
{
    MutexGuard  guard(startLock);

    if (!bStarted)
    {
        bNeedExit = false;
        awake.Wait(0);
        Thread::Start();
        bStarted = true;
    }
}
void RemoteStore::Stop()
{
    MutexGuard  guard(startLock);

    if (bStarted)
    {
        bNeedExit = true;
        awake.Signal();
        exited.Wait();
        bStarted = false;
    }
}

/*
MSG_ID                    -- msg id
RECORD_TYPE               -- 0 SMS, 1 Diverted SMS
MEDIA_TYPE                -- 0 SMS text, 1 SMS binary
BEARER_TYPE               -- 0 SMS, 1 USSD
SUBMIT                    -- submit time
FINALIZED                 -- finalized time
STATUS                    -- LAST_RESULT from SMS_MSG
SRC_ADDR                  -- OA  (.1.1.7865765 format)
SRC_IMSI                  -- SRC IMSI
SRC_MSC                   -- SRC MSC
SRC_SME_ID                --
DST_ADDR                  -- DDA (.1.1.7865765 format)
DST_IMSI                  -- DST IMSI
DST_MSC                   -- DST MSC
DST_SME_ID                --
DIVERTED_FOR              -- message originally was for DIVERTED_FOR address, but was delivered to DST_ADDR
ROUTE_ID                  -- ROUTE_ID from SMS_MSG
SERVICE_ID                -- SERVICE_ID from SMS_MSG
USER_MSG_REF
DATA_LENGTH               -- text or binary length (add it to SMS_MSG insteed of TXT_LENGTH)
*/
void RemoteStore::createBillingRecord(SMSId id, SMS& sms)
    throw(StorageException)
{
    MutexGuard guard(billingFileLock);

    openBillingFile();

    std::string out = "";
    bool isDiverted = sms.hasStrProperty(Tag::SMSC_DIVERTED_TO);
    bool isBinary   = sms.hasIntProperty(Tag::SMPP_DATA_CODING) ?
                     (sms.getIntProperty(Tag::SMPP_DATA_CODING) == DataCoding::BINARY) : false;

    CSVFileEncoder::addUint64(out, id);
    CSVFileEncoder::addUint8 (out, isDiverted ? 1:0);
    CSVFileEncoder::addUint8 (out, isBinary   ? 1:0);
    CSVFileEncoder::addUint8 (out, sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ? 1:0);
    CSVFileEncoder::addDateTime(out, sms.submitTime);
    CSVFileEncoder::addDateTime(out, sms.lastTime);
    CSVFileEncoder::addUint32(out, sms.lastResult);

    std::string oa = sms.originatingAddress.toString();
    CSVFileEncoder::addString(out, oa.c_str());
    CSVFileEncoder::addString(out, sms.originatingDescriptor.imsi);
    CSVFileEncoder::addString(out, sms.originatingDescriptor.msc);
    //CSVFileEncoder.addUint32(out, sms.originatingDescriptor.sme);
    CSVFileEncoder::addString(out, sms.srcSmeId);

    std::string dda = sms.dealiasedDestinationAddress.toString();
    CSVFileEncoder::addString(out, dda.c_str());
    CSVFileEncoder::addString(out, sms.destinationDescriptor.imsi);
    CSVFileEncoder::addString(out, sms.destinationDescriptor.msc);
    //CSVFileEncoder.addUint32(out, sms.destinationDescriptor.sme);
    CSVFileEncoder::addString(out, sms.dstSmeId);

    if (isDiverted) {
        std::string divertedTo = sms.getStrProperty(Tag::SMSC_DIVERTED_TO);
        CSVFileEncoder::addString(out, divertedTo.c_str());
    }
    else CSVFileEncoder::addString(out, 0);
    CSVFileEncoder::addString(out, sms.routeId);
    CSVFileEncoder::addInt32 (out, sms.serviceId);
    if (sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE))
        CSVFileEncoder::addUint32(out, sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
    else
        CSVFileEncoder::addSeparator(out);
    CSVFileEncoder::addUint32(out, sms.messageBody.getShortMessageLength(), true);

    //smsc_log_debug(log, "Billing record is:\n%s", out.c_str());

    writeToBillingFile(out);
}

void RemoteStore::createArchiveRecord(SMSId id, SMS& sms)
    throw(StorageException)
{
    // TODO: implement it
}
void RemoteStore::doFinalizeSms(SMSId id, SMS& sms, bool needDelete)
    throw(StorageException, NoSuchMessageException)
{
    smsc_log_debug(log, "Finalizing msg#%lld", id);

    // TODO: move it after destroySms for valid rollback
    if (sms.billingRecord) createBillingRecord(id, sms);
    if (sms.needArchivate) createArchiveRecord(id, sms);

    if (!needDelete && !sms.needArchivate) { // TODO: remove && !sms.needArchivate here
        smsc_log_debug(log, "Finalized msg#%lld" , id);
        return;
    }

    __require__(pool);
    StorageConnection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try
        {
            connection = (StorageConnection *)pool->getConnection();
            if (connection)
            {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);

                /* TODO: remove it !!! */
                ToFinalStatement* toFinalStatement
                    = connection->getToFinalStatement();

                toFinalStatement->bindSms(id, sms, needDelete);
                connection->check(toFinalStatement->execute());
                connection->commit();
                /* TODO: remove it !!! */

                /* TODO: enable this !!!
                doDestroySms(connection, id);*/
                pool->freeConnection(connection);
                smsc_log_debug(log, "Finalized msg#%lld" , id);
            }
            break;
        }
        catch (StorageException& exc)
        {
            try { connection->rollback(); } catch (...) { smsc_log_error(log, "Failed to rollback"); } // TODO: remove it !!
            if (connection) pool->freeConnection(connection);
            if (iteration++ >= maxTriesCount)
            {
                smsc_log_warn(log, "Max tries count to finalize message "
                         "#%lld exceeded !\n", id);
                throw;
            }
        }
        catch (...)
        {
            try { connection->rollback(); } catch (...) { smsc_log_error(log, "Failed to rollback"); } // TODO: remove it !!
            if (connection) pool->freeConnection(connection);
            throw StorageException("Unknown exception thrown");
        }
    }
}

void RemoteStore::createFinalizedSms(SMSId id, SMS& sms)
    throw(StorageException, DuplicateMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    __require__(sms.state != ENROUTE);

    sms.lastTime = time(NULL);
    sms.nextTime = 0;
    doFinalizeSms(id, sms, false); // not need to delete sms

#else

    MutexGuard guard(fakeMutex);

    if (fakeStore.Exist(id))
        throw DuplicateMessageException(id);
#endif
}

void RemoteStore::changeSmsStateToDelivered(SMSId id, const Descriptor& dst)
    throw(StorageException, NoSuchMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    SMS sms;
    retriveSms(id, sms);
    sms.state = smsc::sms::DELIVERED;
    sms.attempts++;
    sms.lastTime = time(NULL);
    sms.nextTime = 0;
    sms.lastResult = 0;
    sms.destinationDescriptor = dst;
    doFinalizeSms(id, sms, true);

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id))
        throw NoSuchMessageException(id);

    SMS *sms = fakeStore.Get(id);
    if (sms->getState() != ENROUTE)
        throw NoSuchMessageException(id);

    delete sms;
    fakeStore.Delete(id);

#endif
}

void RemoteStore::changeSmsStateToUndeliverable(SMSId id,
    const Descriptor& dst, uint32_t failureCause)
       throw(StorageException, NoSuchMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    SMS sms;
    retriveSms(id, sms);
    sms.state = smsc::sms::UNDELIVERABLE;
    sms.attempts++;
    sms.lastTime = time(NULL);
    sms.nextTime = 0;
    sms.lastResult = failureCause;
    sms.destinationDescriptor = dst;
    doFinalizeSms(id, sms, true);

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id))
        throw NoSuchMessageException(id);

    SMS *sms = fakeStore.Get(id);
    if (sms->getState() != ENROUTE)
        throw NoSuchMessageException(id);

    delete sms;
    fakeStore.Delete(id);

#endif
}

void RemoteStore::changeSmsStateToExpired(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    SMS sms;
    retriveSms(id, sms);
    sms.state = smsc::sms::EXPIRED;
    sms.lastTime = time(NULL);
    sms.nextTime = 0;
    sms.lastResult = smsc::system::Status::EXPIRED;
    doFinalizeSms(id, sms, true);

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id))
        throw NoSuchMessageException(id);

    SMS *sms = fakeStore.Get(id);
    if (sms->getState() != ENROUTE)
        throw NoSuchMessageException(id);

    fakeStore.Delete(id);

#endif
}

void RemoteStore::changeSmsStateToDeleted(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    SMS sms;
    retriveSms(id, sms);
    sms.state = smsc::sms::DELETED;
    sms.lastTime = time(NULL);
    sms.nextTime = 0;
    sms.lastResult = smsc::system::Status::DELETED;
    doFinalizeSms(id, sms, true);

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id))
        throw NoSuchMessageException(id);

    SMS *sms = fakeStore.Get(id);
    if (sms->getState() != ENROUTE)
        throw NoSuchMessageException(id);

    delete sms;
    fakeStore.Delete(id);

#endif
}

/* --------------------- Sheduler's classes & methods -------------------- */

RemoteStore::ReadyIdIterator::ReadyIdIterator(
    StorageConnectionPool* _pool, time_t retryTime, bool immediate)
        throw(StorageException)
            : TimeIdIterator(), pool(_pool), connection(0), readyStmt(0)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    isNull = false;
    connection = pool->getConnection();
    if (!connection) return;
    try
    {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
        if (connection && !connection->isAvailable())
            connection->connect();

        readyStmt = new ReadyByNextTimeStatement(connection, immediate, false);
        readyStmt->bindRetryTime(retryTime);
        sword status = readyStmt->execute(OCI_DEFAULT, 0, 0);
        if (status != OCI_NO_DATA) connection->check(status);
        else isNull = true;
    }
    catch (...)
    {
        if (readyStmt) delete readyStmt;
        if (connection) pool->freeConnection(connection);
        connection = 0; readyStmt = 0;
        throw;
    }
#endif
}
RemoteStore::ReadyIdIterator::~ReadyIdIterator()
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (readyStmt) delete readyStmt;
    if (pool && connection) pool->freeConnection(connection);

#endif
}

bool RemoteStore::ReadyIdIterator::next()
    throw(StorageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (!isNull && readyStmt && connection && connection->isAvailable())
    {
        isNull = true;
        sword status = readyStmt->fetch();
        if (status != OCI_NO_DATA)
        {
            connection->check(status);
            isNull = false;
            return true;
        }
    }
    return false;
#else
    return false;
#endif
}
SMSId RemoteStore::ReadyIdIterator::getId()
    throw(StorageException)
{
    SMSId id = 0;
    if (!isNull && readyStmt && connection && connection->isAvailable()) {
        readyStmt->getSMSId(id);
    }
    return id;
}
time_t RemoteStore::ReadyIdIterator::getTime()
    throw(StorageException)
{
    time_t nextTime = 0;
    if (!isNull && readyStmt && connection && connection->isAvailable()) {
        nextTime = readyStmt->getNextTime();
    }
    return nextTime;
}

TimeIdIterator* RemoteStore::getReadyForRetry(time_t retryTime, bool immediate)
    throw(StorageException)
{
    return (new ReadyIdIterator(pool, retryTime, immediate));
}

time_t RemoteStore::getNextRetryTime()
    throw(StorageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    time_t minTime = 0;
    Connection* connection = pool->getConnection();
    if (connection)
    {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
        MinNextTimeStatement* minTimeStmt = 0L;
        try
        {
            if (!connection->isAvailable()) connection->connect();
            minTimeStmt = new MinNextTimeStatement(connection, false);
        }
        catch (...)
        {
            pool->freeConnection(connection);
            throw;
        }

        if (minTimeStmt)
        {
            try
            {
                sword status = minTimeStmt->execute();
                if (status != OCI_NO_DATA)
                {
                    connection->check(status);
                    minTime = minTimeStmt->getMinNextTime();
                }
                delete minTimeStmt;
            }
            catch (...)
            {
                delete minTimeStmt;
                RemoteStore::pool->freeConnection(connection);
                throw;
            }
        }
        pool->freeConnection(connection);
    }
    return minTime;
#else
    return ((uint32_t)-1);
#endif
}

RemoteStore::DeliveryIdIterator::DeliveryIdIterator(
    StorageConnectionPool* _pool, const Address& da)
        throw(StorageException)
            : IdIterator(), pool(_pool), connection(0), deliveryStmt(0)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    isNull = false;
    connection = pool->getConnection();
    if (!connection) return;
    try
    {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
        if (connection && !connection->isAvailable())
            connection->connect();

        deliveryStmt = new DeliveryIdsStatement(connection, da, false);
        sword status = deliveryStmt->execute(OCI_DEFAULT, 0, 0);
        if (status != OCI_NO_DATA) connection->check(status);
        else isNull = true;
    }
    catch (...)
    {
        if (deliveryStmt) delete deliveryStmt;
        if (connection) pool->freeConnection(connection);
        connection = 0; deliveryStmt = 0;
        throw;
    }
#endif
}
RemoteStore::DeliveryIdIterator::~DeliveryIdIterator()
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (deliveryStmt) delete deliveryStmt;
    if (pool && connection) pool->freeConnection(connection);

#endif
}
bool RemoteStore::DeliveryIdIterator::next()
    throw(StorageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (!isNull && deliveryStmt && connection && connection->isAvailable())
    {
        isNull = true;
        sword status = deliveryStmt->fetch();
        if (status != OCI_NO_DATA)
        {
            connection->check(status);
            isNull = false;
            return true;
        }
    }
    return false;
#else
    return false;
#endif
}
SMSId RemoteStore::DeliveryIdIterator::getId()
    throw(StorageException)
{
    SMSId id = 0;
    if (!isNull && deliveryStmt && connection && connection->isAvailable()) {
        deliveryStmt->getSMSId(id);
    }
    return id;
}

IdIterator* RemoteStore::getReadyForDelivery(const Address& da)
    throw(StorageException)
{
    return (new DeliveryIdIterator(pool, da));
}

/* --------------------- Service classes & methods -------------------- */

RemoteStore::CancelIdIterator::CancelIdIterator(StorageConnectionPool* _pool,
    const Address& oa, const Address& da, const char* svc)
        throw(StorageException)
            : IdIterator(), pool(_pool), connection(0), cancelStmt(0)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    isNull = false;
    connection = pool->getConnection();
    if (!connection) return;
    try
    {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
        if (connection && !connection->isAvailable())
            connection->connect();

        cancelStmt = new CancelIdsStatement(connection, oa, da, svc, false);
        sword status = cancelStmt->execute(OCI_DEFAULT, 0, 0);
        if (status != OCI_NO_DATA) connection->check(status);
        else isNull = true;
    }
    catch (...)
    {
        if (cancelStmt) delete cancelStmt;
        if (connection) pool->freeConnection(connection);
        connection = 0; cancelStmt = 0;
        throw;
    }
#endif
}
RemoteStore::CancelIdIterator::~CancelIdIterator()
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (cancelStmt) delete cancelStmt;
    if (pool && connection) pool->freeConnection(connection);

#endif
}
bool RemoteStore::CancelIdIterator::next()
    throw(StorageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE
    if (!isNull && cancelStmt && connection && connection->isAvailable())
    {
        isNull = true;
        sword status = cancelStmt->fetch();
        if (status != OCI_NO_DATA)
        {
            connection->check(status);
            isNull = false;
            return true;
        }
    }
    return false;
#else
    return false;
#endif
}
SMSId RemoteStore::CancelIdIterator::getId()
    throw(StorageException)
{
    SMSId id = 0;
    if (!isNull && cancelStmt && connection && connection->isAvailable()) {
        cancelStmt->getSMSId(id);
    }
    return id;
}

IdIterator* RemoteStore::getReadyForCancel(const Address& oa,
    const Address& da, const char* svcType)
        throw(StorageException)
{
    return (new CancelIdIterator(pool, oa, da, svcType));
}

int RemoteStore::getConcatMessageReference(const Address& dda)
    throw(StorageException)
{
    int msgRef = -1;

#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    Connection* connection = pool->getConnection();
    if (!connection)
        throw StorageException("Get connection failed");
    ConcatDataStatement* concatStmt = 0;

    try
    {
                __trace2__( "got connection %p for %s", connection, __FUNCTION__);
        if (connection && !connection->isAvailable())
            connection->connect();

        concatStmt = new ConcatDataStatement(connection, false);
        concatStmt->setDestination(dda);

        sword status = concatStmt->execute();
        if (status != OCI_NO_DATA) {
            connection->check(status);
            msgRef = concatStmt->getMessageReference();
        }
        else msgRef=-1;

        if (concatStmt) delete concatStmt;
        if (connection) pool->freeConnection(connection);
    }
    catch (...)
    {
        if (concatStmt) delete concatStmt;
        if (connection) pool->freeConnection(connection);
        throw;
    }

    return msgRef;
#else

    return -1;
#endif
}

/* ------------------------------ Remote Store ------------------------------ */

/* ------------------------------- SMS Cache -------------------------------- */

SmsCache::SmsCache(unsigned capacity, unsigned initsize)
    : idCache(initsize), cacheCapacity(capacity), lastId(0)
{
}
SmsCache::~SmsCache()
{
    clean();
}

void SmsCache::clean()
{
    idCache.First(); SMSId id; SMS* sms = 0;
    while (idCache.Next(id, sms))
        if (sms) delete sms;
    idCache.Clean();
}
void SmsCache::putSms(SMSId id, SMS* sm)
{
    __require__(sm);

    if (idCache.Count() == 0) lastId = id;
    if (idCache.Count() >= cacheCapacity)
    {
//        __debug2__(log, "Cache size is %d. Cleaning SMS cache ...", idCache.Count());
        int toDelete = cacheCapacity/10;
        SMSId curId=lastId;

        while (curId<id && toDelete>0)
            if (delSms(curId++)) toDelete--;

        lastId = curId;
//        __debug2__(log, "Cache size is %d. SMS cache cleaned.", idCache.Count());
    }
    idCache.Insert(id, sm);
}
bool SmsCache::delSms(SMSId id)
{
    SMS** psm = idCache.GetPtr(id);
    if (psm)
    {
        if (*psm) delete (*psm);
        idCache.Delete(id);
        return true;
    }
    return false;
}
SMS* SmsCache::getSms(SMSId id)
{
    SMS** psm = idCache.GetPtr(id);
    return ((psm) ? *psm:0);
}

/* ------------------------------- SMS Cache -------------------------------- */

/* ------------------------------ Cached Store ------------------------------ */

void CachedStore::loadMaxCacheCapacity(Manager& config)
{
    try
    {
        maxCacheCapacity = config.getInt("MessageStore.Cache.capacity");
    }
    catch (ConfigException& exc)
    {
        maxCacheCapacity = SMSC_MAX_SMS_CACHE_CAPACITY;
        __warn2__((&log),"Config parameter: <MessageStore.Cache.capacity> missed. "
                 "Using default: %d.", maxCacheCapacity);
    }
}

CachedStore::CachedStore(Manager& config, SchedTimer* sched)
    throw(ConfigException, StorageException)
        : RemoteStore(config, sched), cache(0),
            maxCacheCapacity(SMSC_MAX_SMS_CACHE_CAPACITY)
{
    loadMaxCacheCapacity(config);
    cache = new SmsCache(maxCacheCapacity);
    __require__(cache);
}
CachedStore::~CachedStore()
{
    MutexGuard  guard(cacheMutex);
    __info__((&log), "CachedStore: cleaning cache..." );
    if (cache) delete cache;
    __info__((&log), "CachedStore: cache cleaned." );
}

SMSId CachedStore::createSms(SMS& sms, SMSId id, const CreateMode flag)
    throw(StorageException, DuplicateMessageException)
{
    SMSId retId;

    __debug2__((&log),"Creating sms, smsId = %lld, flag = %d", id, flag);
    retId = RemoteStore::createSms(sms, id, flag);
    __debug2__((&log),"Created sms, smsId = %lld, retId = %lld", id, retId);

    SMS* sm = new SMS(sms);
    MutexGuard cacheGuard(cacheMutex);
    if (retId != id) cache->delSms(retId);
    cache->putSms(id, sm);

    return retId;
}

void CachedStore::retriveSms(SMSId id, SMS &sms)
    throw(StorageException, NoSuchMessageException)
{
    if( sms.hasBinProperty(Tag::SMSC_CONCATINFO) )
      __warn2__((&log),"smsId = %lld 'empty sms' already have concatinfo", id);
    SMS* sm = 0;
    {
        MutexGuard cacheGuard(cacheMutex);
        sm = cache->getSms(id);
        if (sm) {
            sms = *sm;
            __debug2__((&log),"smsId = %lld found in cache.", id);
            return;
        }
    }

    __debug2__((&log), "smsId = %lld retriving from DB ...", id);
    RemoteStore::retriveSms(id, sms);
    __debug2__((&log),"smsId = %lld retrived DB. concat=%d", id, sms.hasBinProperty(Tag::SMSC_CONCATINFO));

    if (sms.state == ENROUTE)
    {
        sm = new SMS(sms);
        MutexGuard cacheGuard(cacheMutex);
        cache->putSms(id, sm);

        SMS* sm1 = cache->getSms(id);
        __debug2__((&log),"smsId = %lld concat check=%d", id, sm1->hasBinProperty(Tag::SMSC_CONCATINFO));
    }
}

void CachedStore::changeSmsConcatSequenceNumber(SMSId id, int8_t inc)
    throw(StorageException, NoSuchMessageException)
{
    __debug2__((&log), "Changing seqNum for smsId = %lld.", id);
    RemoteStore::changeSmsConcatSequenceNumber(id, inc);

    SMS* sm = 0;
    {
        MutexGuard cacheGuard(cacheMutex);
        sm = cache->getSms(id);
        if (sm)
        {
            sm->concatSeqNum += inc;
            return;
        }
    }

    SMS sms; RemoteStore::retriveSms(id, sms);
    if (sms.state == ENROUTE)
    {
        sm = new SMS(sms);
        MutexGuard cacheGuard(cacheMutex);
        cache->putSms(id, sm);
    }
}

void CachedStore::replaceSms(SMSId id, const Address& oa,
                             const uint8_t* newMsg, uint8_t newMsgLen,
                             uint8_t deliveryReport,
                             time_t validTime, time_t waitTime)
    throw(StorageException, NoSuchMessageException)
{

    RemoteStore::replaceSms(id, oa, newMsg, newMsgLen, deliveryReport,
                            validTime, waitTime);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}
void CachedStore::replaceSms(SMSId id, SMS& sms)
    throw(StorageException, NoSuchMessageException)
{

    RemoteStore::replaceSms(id, sms);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
    SMS* sm = new SMS(sms);
    cache->putSms(id, sm);
}
void CachedStore::destroySms(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    RemoteStore::destroySms(id);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}

void CachedStore::changeSmsStateToEnroute(SMSId id, const Descriptor& dst,
                                          uint32_t failureCause,
                                          time_t nextTryTime, uint32_t attempts)
    throw(StorageException, NoSuchMessageException)
{
    __debug2__((&log), "Changing to ENROUTE for smsId = %lld.", id);
    time_t lastTryTime = time(0);
    RemoteStore::changeSmsStateToEnroute(id, dst, failureCause, nextTryTime, attempts);

    SMS* sm = 0;
    {
        MutexGuard cacheGuard(cacheMutex);
        sm = cache->getSms(id);
        if (sm)
        {
            sm->state = ENROUTE;
            sm->destinationDescriptor = dst;
            sm->lastResult = failureCause;
            sm->lastTime = lastTryTime;
            sm->nextTime = nextTryTime;
            sm->attempts = attempts;
            return;
        }
    }

    SMS sms; RemoteStore::retriveSms(id, sms);
    if (sms.state == ENROUTE)
    {
        sm = new SMS(sms);
        MutexGuard cacheGuard(cacheMutex);
        cache->putSms(id, sm);
    }
}
void CachedStore::changeSmsStateToDelivered(SMSId id,
                                            const Descriptor& dst)
    throw(StorageException, NoSuchMessageException)
{
    __debug2__((&log), "Changing to DELIVERED for smsId = %lld.", id);
    RemoteStore::changeSmsStateToDelivered(id, dst);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}
void CachedStore::changeSmsStateToUndeliverable(SMSId id,
                                                const Descriptor& dst,
                                                uint32_t failureCause)
    throw(StorageException, NoSuchMessageException)
{
    __debug2__((&log), "Changing to UNDELIVERABLE for smsId = %lld.", id);
    RemoteStore::changeSmsStateToUndeliverable(id, dst, failureCause);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}
void CachedStore::changeSmsStateToExpired(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    __debug2__((&log), "Changing to EXPIRED for smsId = %lld.", id);
    RemoteStore::changeSmsStateToExpired(id);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}
void CachedStore::changeSmsStateToDeleted(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    __debug2__((&log), "Changing to DELETED for smsId = %lld.", id);
    RemoteStore::changeSmsStateToDeleted(id);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}

/* ------------------------------ Cached Store ------------------------------ */
}}
