#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#include <oci.h>
//#include <orl.h>


#include "StoreManager.h"

#define __FUNCTION__ __func__

namespace smsc { namespace store
{

/* ----------------------------- StoreManager -------------------------- */


#if 0
using namespace smsc::sms;
using smsc::logger::Logger;
using smsc::util::config::Manager;

const unsigned SMSC_MAX_TRIES_TO_PROCESS_OPERATION = 3;
const unsigned SMSC_MAX_TRIES_TO_PROCESS_OPERATION_LIMIT = 1000;

Mutex        StoreManager::mutex;
RemoteStore* StoreManager::instance  = 0;

smsc::logger::Logger* StoreManager::log = 0;

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
void StoreManager::startup(Manager& config, SchedTimer* sched)
    throw(ConfigException, ConnectionFailedException)
{
    MutexGuard guard(mutex);
    if (!log) log = Logger::getInstance("smsc.store.StoreManager");

    if (!instance)
    {
        smsc_log_info(log, "Storage Manager is starting ... ");
        try
        {

#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

            instance = (needCache(config)) ?
                        new CachedStore(config, sched) :
                        new RemoteStore(config, sched);
#else
            instance = new RemoteStore(config, sched);
#endif
        }
        catch (StorageException& exc)
        {
            if (instance) { delete instance; instance = 0; }
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
                      "wasn't specified ! Config parameter: <MessageStore.maxTriesCount> "
                      "Using default: %d", SMSC_MAX_TRIES_TO_PROCESS_OPERATION);
    }
}

RemoteStore::RemoteStore(Manager& config, SchedTimer* sched)
    throw(ConfigException, StorageException)
        : Thread(), log(Logger::getInstance("smsc.store.RemoteStore")),
            bStarted(false), bNeedExit(false), currentId(0), sequenceId(0),
                pool(0), scheduleTimer(sched), maxTriesCount(SMSC_MAX_TRIES_TO_PROCESS_OPERATION)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE
    loadMaxTriesCount(config);
    pool = new StorageConnectionPool(config);
#endif

    billingStorage.init(config);
    archiveStorage.init(config);

    Start();
}
RemoteStore::~RemoteStore()
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE
    smsc_log_info(log, "RemoteStore: Destroying connection pool");
    if (pool) delete pool;
    smsc_log_info(log, "RemoteStore: Connection pool destroyed");
#endif

    Stop();
}

SMSId RemoteStore::getNextId()
    throw(StorageException)
{
    static const int SMS_ID_PRELOAD_COUNT = 1000;

    MutexGuard  guard(sequenceIdLock);

    if (!currentId || !sequenceId || currentId-sequenceId >= SMS_ID_PRELOAD_COUNT)
    {
        StorageConnection* connection = 0L;
        unsigned iteration=1;
        while (true)
        {
            try
            {
                connection = (StorageConnection *)pool->getConnection();
                if (!connection) throw StorageException("Failed to obtain DB connection!");
                smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
                GetSeqIdStatement* nextIdStmt = connection->getNextSeqIdStatement();
                if (!nextIdStmt) throw StorageException("Failed to obtain nextId statement!");
                connection->check(nextIdStmt->execute());
                nextIdStmt->getSMSId(sequenceId);
                currentId = sequenceId;
                pool->freeConnection(connection);
                break;
            }
            catch (StorageException& exc)
            {
                if (connection) pool->freeConnection(connection);
                if (iteration++ >= maxTriesCount) {
                    smsc_log_warn(log, "Max tries count to get next message id exceeded!");
                    throw;
                }
            }
        }

    }

    return ++currentId;
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
                bindEServiceType((dvoid *) sms.eServiceType, (sb4) sizeof(sms.eServiceType));

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

                OverwriteStatement* overwriteStmt = connection->getOverwriteStatement();
                DestroyBodyStatement* destroyBodyStmt = connection->getDestroyBodyStatement();

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
                throw;
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
        throw;
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
                smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
                smsc_log_warn(log, "Max tries count to store message #%lld exceeded!", id);
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
        throw;
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
                smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
                              "sequence number in concatenneted message #%lld exceeded!", id);
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
                smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
                smsc_log_warn(log, "Max tries count to retrive message #%lld exceeded!", id);
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

    DestroyStatement* destroyStmt = connection->getDestroyStatement();
    DestroyBodyStatement* destroyBodyStmt = connection->getDestroyBodyStatement();

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
                smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
            if (iteration++ >= maxTriesCount) {
                smsc_log_warn(log, "Max tries count to remove message #%lld exceeded!", id);
                throw;
            }
        }
    }

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id)) throw NoSuchMessageException(id);
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
    RetrieveBodyStatement* retrieveBodyStmt = connection->getRetrieveBodyStatement();
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
            GetBodyStatement* getBodyStmt = connection->getGetBodyStatement();

            getBodyStmt->setSMSId(id);
            getBodyStmt->getBody(body);
            //connection->commit(); // Need to reset BLOB (SELECT FOR UPDATE)

            DestroyBodyStatement* destroyBodyStmt = connection->getDestroyBodyStatement();

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
        replaceStmt->bindDeliveryReport((dvoid *) &deliveryReport, (sb4) sizeof(deliveryReport));

        connection->check(replaceStmt->execute());

        int bodyLen = body.getBufferLength();
        if (bodyLen > MAX_BODY_LENGTH)
        {
            SetBodyStatement* setBodyStmt = connection->getSetBodyStatement();
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
                smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
            if (iteration++ >= maxTriesCount) {
                smsc_log_warn(log, "Max tries count to replace message #%lld exceeded!", id);
                throw;
            }
        }
    }

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id)) throw NoSuchMessageException(id);

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
        sms->getMessageBody().setIntProperty(Tag::SMPP_SM_LENGTH, (uint32_t)newMsgLen);
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

    ReplaceAllStatement* replaceStmt = connection->getReplaceAllStatement();
    RetrieveBodyStatement* retrieveBodyStmt = connection->getRetrieveBodyStatement();

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
                DestroyBodyStatement* destroyBodyStmt = connection->getDestroyBodyStatement();
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
            SetBodyStatement* setBodyStmt = connection->getSetBodyStatement();
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
    sms.attempts = 0;

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
                smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
            if (iteration++ >= maxTriesCount) {
                smsc_log_warn(log, "Max tries count to replace message #%lld exceeded!", id);
                throw;
            }
        }
    }

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id)) throw NoSuchMessageException(id);
    delete fakeStore.Get(id);
    fakeStore.Delete(id);
    fakeStore.Insert(id, new SMS(sms));

#endif
}


void RemoteStore::doChangeSmsStateToEnroute(StorageConnection* connection,
    SMSId id, const Descriptor& dst, uint32_t failureCause, time_t nextTryTime, uint32_t attempts)
            throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    ToEnrouteStatement* toEnrouteStmt = connection->getToEnrouteStatement();

    toEnrouteStmt->bindId(id);
    toEnrouteStmt->bindNextTime(nextTryTime);
    toEnrouteStmt->bindFailureCause((dvoid *)&(failureCause), (sb4) sizeof(failureCause));
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
        throw;
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
    const Descriptor& dst, uint32_t failureCause, time_t nextTryTime, uint32_t attempts)
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
                smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
            if (iteration++ >= maxTriesCount) {
                smsc_log_warn(log, "Max tries count to update message state #%lld exceeded!", id);
                throw;
            }
        }
    }

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id)) throw NoSuchMessageException(id);
    SMS *sms=fakeStore.Get(id);
    if (sms->getState() != ENROUTE) throw NoSuchMessageException(id);

    sms->destinationDescriptor = dst;
    sms->lastResult = failureCause;
    sms->nextTime = nextTryTime;
    sms->attempts = attempts;

    //fakeStore.Insert(id, sms);

#endif
}

int RemoteStore::Execute()
{
    const int SERVICE_SLEEP = 3600;   // in seconds

    int32_t  billingRest = billingStorage.getStorageInterval();
    int32_t  archiveRest = archiveStorage.getStorageInterval();
    bool     billing, archive;
    uint32_t toSleep;

    while (!bNeedExit)
    {
        if (billingRest < archiveRest) {
            archiveRest -= billingRest;
            billing = true; archive = false;
            toSleep = billingRest;
            billingRest = billingStorage.getStorageInterval();
        }
        else if (billingRest > archiveRest) {
            billingRest -= archiveRest;
            billing = false; archive = true;
            toSleep = archiveRest;
            archiveRest = archiveStorage.getStorageInterval();
        }
        else {
            billing = true; archive = true;
            toSleep = archiveRest;
            archiveRest = archiveStorage.getStorageInterval();
            billingRest = billingStorage.getStorageInterval();
        }

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

        try { // rool billing file
            if (billing || bNeedExit) billingStorage.roll();
        } catch (StorageException& exc) {
            awake.Wait(0); smsc_log_error(log, "%s", exc.what());
        }

        try {  // rool archive file
            if (archive || bNeedExit) archiveStorage.roll();
        } catch (StorageException& exc) {
            awake.Wait(0); smsc_log_error(log, "%s", exc.what());
        }
    }

    billingStorage.close();
    archiveStorage.close();

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

void RemoteStore::doFinalizeSms(SMSId id, SMS& sms, bool needDelete)
    throw(StorageException, NoSuchMessageException)
{
    smsc_log_debug(log, "Finalizing msg#%lld", id);

    if (sms.needArchivate) archiveStorage.createRecord(id, sms);
    if (sms.billingRecord) billingStorage.createRecord(id, sms);

    if (needDelete)
    {
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
                    smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
                if (iteration++ >= maxTriesCount) {
                    smsc_log_warn(log, "Max tries count to finalize message #%lld exceeded!", id);
                    throw;
                }
            }
            catch (...)
            {
                if (connection) pool->freeConnection(connection);
                throw StorageException("Unknown exception thrown");
            }
        }
    }

    smsc_log_debug(log, "Finalized msg#%lld" , id);
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

    char buf[MAX_ADDRESS_VALUE_LENGTH*4+12];
    if(sms.originatingDescriptor.mscLength && sms.originatingDescriptor.imsiLength &&
       sms.destinationDescriptor.mscLength && sms.destinationDescriptor.imsiLength)
    {
      sprintf(buf,"Org: %s/%s, Dst:%s/%s",sms.originatingDescriptor.msc,sms.originatingDescriptor.imsi,
                         sms.destinationDescriptor.msc,sms.destinationDescriptor.imsi
                        );
    }else if(sms.originatingDescriptor.mscLength && sms.originatingDescriptor.imsiLength)
    {
      sprintf(buf,"Org: %s/%s",sms.originatingDescriptor.msc,sms.originatingDescriptor.imsi);
    }else if(sms.destinationDescriptor.mscLength && sms.destinationDescriptor.imsiLength)
    {
      sprintf(buf,"Dst:%s/%s",sms.destinationDescriptor.msc,sms.destinationDescriptor.imsi);
    }
    sms.setStrProperty(Tag::SMSC_DESCRIPTORS,buf);

    doFinalizeSms(id, sms, true);

#else

    MutexGuard guard(fakeMutex);

    if (!fakeStore.Exist(id)) throw NoSuchMessageException(id);
    SMS *sms = fakeStore.Get(id);
    if (sms->getState() != ENROUTE) throw NoSuchMessageException(id);
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

    if (!fakeStore.Exist(id)) throw NoSuchMessageException(id);
    SMS *sms = fakeStore.Get(id);
    if (sms->getState() != ENROUTE) throw NoSuchMessageException(id);
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

    if (!fakeStore.Exist(id)) throw NoSuchMessageException(id);
    SMS *sms = fakeStore.Get(id);
    if (sms->getState() != ENROUTE) throw NoSuchMessageException(id);
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

    if (!fakeStore.Exist(id)) throw NoSuchMessageException(id);
    SMS *sms = fakeStore.Get(id);
    if (sms->getState() != ENROUTE) throw NoSuchMessageException(id);
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
        //smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
bool RemoteStore::ReadyIdIterator::getDstSmeId(char* buffer)
    throw(StorageException)
{
    __require__(buffer);
    if (!isNull && readyStmt && connection && connection->isAvailable()) {
        return readyStmt->getDstSmeId(buffer);
    }
    buffer[0] = '\0';
    return false;
}

bool RemoteStore::ReadyIdIterator::getDda(char* buffer)
    throw(StorageException)
{
    __require__(buffer);
    if (!isNull && readyStmt && connection && connection->isAvailable()) {
        return readyStmt->getDda(buffer);
    }
    buffer[0] = '\0';
    return false;
}

int RemoteStore::ReadyIdIterator::getAttempts()
    throw(StorageException)
{
    if (!isNull && readyStmt && connection && connection->isAvailable()) {
        return readyStmt->getAttempts();
    }
    return 0;
}

time_t RemoteStore::ReadyIdIterator::getValidTime()
    throw(StorageException)
{
    if (!isNull && readyStmt && connection && connection->isAvailable()) {
        return readyStmt->getValidTime();
    }
    return 0;
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
        smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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

RemoteStore::DeliveryIdIterator::DeliveryIdIterator(StorageConnectionPool* _pool, const Address& da)
    throw(StorageException)
        : IdIterator(), pool(_pool), connection(0), deliveryStmt(0)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    isNull = false;
    connection = pool->getConnection();
    if (!connection) return;
    try
    {
        //smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
        //smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
        smsc_log_debug(log, "got connection %p for %s", connection, __FUNCTION__);
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
        int toDelete = cacheCapacity/10;
        SMSId curId=lastId;

        while (curId<id && toDelete>0)
            if (delSms(curId++)) toDelete--;

        lastId = curId;
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
        smsc_log_warn(log, "Config parameter: <MessageStore.Cache.capacity> missed. "
                      "Using default: %d.", maxCacheCapacity);
    }
}

CachedStore::CachedStore(Manager& config, SchedTimer* sched)
    throw(ConfigException, StorageException)
        : RemoteStore(config, sched), log(Logger::getInstance("smsc.store.CachedStore")),
            cache(0), maxCacheCapacity(SMSC_MAX_SMS_CACHE_CAPACITY)
{
    loadMaxCacheCapacity(config);
    cache = new SmsCache(maxCacheCapacity);
    __require__(cache);
}
CachedStore::~CachedStore()
{
    MutexGuard  guard(cacheMutex);
    smsc_log_info(log, "CachedStore: cleaning cache..." );
    if (cache) delete cache;
    smsc_log_info(log, "CachedStore: cache cleaned." );
}

SMSId CachedStore::createSms(SMS& sms, SMSId id, const CreateMode flag)
    throw(StorageException, DuplicateMessageException)
{
    SMSId retId;

    smsc_log_debug(log,"Creating sms, smsId = %lld, flag = %d", id, flag);
    retId = RemoteStore::createSms(sms, id, flag);
    smsc_log_debug(log,"Created sms, smsId = %lld, retId = %lld", id, retId);

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
        smsc_log_warn(log, "smsId = %lld 'empty sms' already have concatinfo", id);

    SMS* sm = 0;
    {
        MutexGuard cacheGuard(cacheMutex);
        sm = cache->getSms(id);
        if (sm) {
            sms = *sm;
            smsc_log_debug(log, "smsId = %lld found in cache.", id);
            return;
        }
    }

    smsc_log_debug(log, "smsId = %lld retriving from DB ...", id);
    RemoteStore::retriveSms(id, sms);
    smsc_log_debug(log, "smsId = %lld retrived DB. concat=%d", id, sms.hasBinProperty(Tag::SMSC_CONCATINFO));

    if (sms.state == ENROUTE)
    {
        sm = new SMS(sms);
        MutexGuard cacheGuard(cacheMutex);
        cache->putSms(id, sm);

        SMS* sm1 = cache->getSms(id);
        smsc_log_debug(log, "smsId = %lld concat check=%d", id, sm1->hasBinProperty(Tag::SMSC_CONCATINFO));
    }
}

void CachedStore::changeSmsConcatSequenceNumber(SMSId id, int8_t inc)
    throw(StorageException, NoSuchMessageException)
{
    smsc_log_debug(log, "Changing seqNum for smsId = %lld.", id);
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
    sms.attempts = 0;
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
    smsc_log_debug(log, "Changing to ENROUTE for smsId = %lld.", id);
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
    smsc_log_debug(log, "Changing to DELIVERED for smsId = %lld.", id);
    RemoteStore::changeSmsStateToDelivered(id, dst);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}
void CachedStore::changeSmsStateToUndeliverable(SMSId id,
                                                const Descriptor& dst,
                                                uint32_t failureCause)
    throw(StorageException, NoSuchMessageException)
{
    smsc_log_debug(log, "Changing to UNDELIVERABLE for smsId = %lld.", id);
    RemoteStore::changeSmsStateToUndeliverable(id, dst, failureCause);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}
void CachedStore::changeSmsStateToExpired(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    smsc_log_debug(log, "Changing to EXPIRED for smsId = %lld.", id);
    RemoteStore::changeSmsStateToExpired(id);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}
void CachedStore::changeSmsStateToDeleted(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    smsc_log_debug(log, "Changing to DELETED for smsId = %lld.", id);
    RemoteStore::changeSmsStateToDeleted(id);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}

#endif //0

/* ------------------------------ Cached Store ------------------------------ */
}}
