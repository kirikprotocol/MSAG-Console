#include <stdio.h>
#include <stdlib.h>

#include <oci.h>
#include <orl.h>

#include <util/debug.h>
#include "StoreManager.h"

namespace smsc { namespace store
{

/* ----------------------------- StoreManager -------------------------- */
using namespace smsc::sms;
using smsc::util::Logger;
using smsc::util::config::Manager;

const unsigned SMSC_MAX_TRIES_TO_PROCESS_OPERATION = 3;
const unsigned SMSC_MAX_TRIES_TO_PROCESS_OPERATION_LIMIT = 1000;

Mutex        StoreManager::mutex;
Archiver*    StoreManager::archiver = 0;
IDGenerator* StoreManager::generator = 0;
RemoteStore* StoreManager::instance  = 0;

log4cpp::Category& StoreManager::log = Logger::getCategory("smsc.store.StoreManager");
log4cpp::Category& RemoteStore::log = Logger::getCategory("smsc.store.RemoteStore");
log4cpp::Category& CachedStore::log = Logger::getCategory("smsc.store.CachedStore");

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
        log.warn("Config parameter: <MessageStore.Cache.enabled> missed. "
                 "Cache disabled.");
    }
    return cacheIsNeeded;
}
bool StoreManager::needArchiver(Manager& config)
{
    bool archiverIsNeeded = false;
    try
    {
        archiverIsNeeded = config.getBool("MessageStore.Archive.enabled");
    }
    catch (ConfigException& exc)
    {
        log.warn("Config parameter: <MessageStore.Archive.enabled> missed. "
                 "Archiver disabled.");
    }
    return archiverIsNeeded;
}
void StoreManager::startup(Manager& config)
    throw(ConfigException, ConnectionFailedException)
{
    MutexGuard guard(mutex);

    if (!instance)
    {
        log.info("Storage Manager is starting ... ");
        try
        {

#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE
            
            instance = (needCache(config)) ? 
                        new CachedStore(config) : new RemoteStore(config);
            
            archiver = new Archiver(config);
            generator = new IDGenerator(archiver->getLastUsedId());

            if (needArchiver(config)) archiver->Start();
#else
            instance = new RemoteStore(config);
            generator = new IDGenerator(0);
#endif
        }
        catch (StorageException& exc)
        {
            if (instance) { delete instance; instance = 0; }
            if (archiver) { delete archiver; archiver = 0; }
            if (generator) { delete generator; generator = 0; }
            throw ConnectionFailedException(exc);
        }
        log.info("Storage Manager was started up.");
    }
}

void StoreManager::shutdown()
{
    MutexGuard guard(mutex);

    if (instance)
    {
        log.info("Storage Manager is shutting down ...");
        delete instance; instance = 0;
        if (generator) delete generator; generator = 0;
        if (archiver) delete archiver; archiver = 0;
        log.info("Storage Manager was shutdowned.");
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
            log.warn("Max tries count to process operation on MessageStore "
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
        log.warn("Max tries count to process operation on MessageStore "
                 "wasn't specified ! "
                 "Config parameter: <MessageStore.maxTriesCount> "
                 "Using default: %d",
                 SMSC_MAX_TRIES_TO_PROCESS_OPERATION);
    }
}

RemoteStore::RemoteStore(Manager& config)
    throw(ConfigException, StorageException)
        : pool(0), maxTriesCount(SMSC_MAX_TRIES_TO_PROCESS_OPERATION)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE
    loadMaxTriesCount(config);
    pool = new StorageConnectionPool(config);
#endif
}
RemoteStore::~RemoteStore() 
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE
    if (pool) delete pool;
#endif
}
SMSId RemoteStore::getNextId()
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
            SMSId retId;

            connection->check(result);
            needOverwriteStmt->getId(retId);

            OverwriteStatement* overwriteStmt
                = connection->getOverwriteStatement();
            DestroyBodyStatement* destroyBodyStmt
                = connection->getDestroyBodyStatement();

            try
            {
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
                connection->rollback();
                throw exc;
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
        connection->rollback();
        throw exc;
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
                log.warn("Max tries count to store message "
                         "#%d exceeded !\n", id);
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
        connection->rollback();
        throw exc;
    }

    if (!seqNumStmt->wasUpdated())
    {
        connection->rollback();
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
                log.warn("Max tries count to change "
                         "sequence number in concatenneted message "
                         "#%d exceeded !\n", id); 
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
                log.warn("Max tries count to retrive message "
                         "#%d exceeded !\n", id);
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
        connection->rollback();
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
                log.warn("Max tries count to remove message "
                         "#%d exceeded !\n", id);
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
    }
    catch (StorageException& exc)
    {
        connection->rollback();
        throw;
    }

    try 
    {
        body.setBinProperty(Tag::SMPP_SHORT_MESSAGE, (const char*)newMsg,
                            (unsigned)newMsgLen);
        body.setIntProperty(Tag::SMPP_SM_LENGTH, (uint32_t)newMsgLen);
    } 
    catch (...) {
        throw StorageException("Incorrect Sms body data. Set/Get Property failed!");
    }

    ReplaceStatement* replaceStmt;
    if (waitTime == 0 && validTime == 0)
    {
        replaceStmt = connection->getReplaceStatement();
    }
    else if (waitTime == 0)
    {
        replaceStmt = connection->getReplaceVTStatement();
        ((ReplaceVTStatement *)replaceStmt)->bindValidTime(validTime);
    }
    else if (validTime == 0)
    {
        replaceStmt = connection->getReplaceWTStatement();
        ((ReplaceWTStatement *)replaceStmt)->bindWaitTime(waitTime);
    }
    else
    {
        replaceStmt = connection->getReplaceVWTStatement();
        ((ReplaceVWTStatement *)replaceStmt)->bindWaitTime(waitTime);
        ((ReplaceVWTStatement *)replaceStmt)->bindValidTime(validTime);
    }

    replaceStmt->bindId(id);
    replaceStmt->bindOriginatingAddress((Address&) oa);
    replaceStmt->bindBody(body);
    replaceStmt->bindDeliveryReport((dvoid *) &deliveryReport,
                                    (sb4) sizeof(deliveryReport));
    try
    {
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
        connection->rollback();
        throw exc;
    }

    if (!replaceStmt->wasReplaced())
    {
        connection->rollback();
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
                log.warn("Max tries count to replace message "
                         "#%d exceeded !\n", id);
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

void RemoteStore::doChangeSmsStateToEnroute(StorageConnection* connection,
    SMSId id, const Descriptor& dst, uint32_t failureCause, time_t nextTryTime,
        bool skipAttempt)
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
    toEnrouteStmt->bindAttemptsIncrement(skipAttempt ? 0:1);

    try
    {
        connection->check(toEnrouteStmt->execute());
    }
    catch (StorageException& exc)
    {
        connection->rollback();
        throw exc;
    }

    if (!toEnrouteStmt->wasUpdated())
    {
        connection->rollback();
        throw NoSuchMessageException(id);
    }
    connection->commit();
}
void RemoteStore::changeSmsStateToEnroute(SMSId id,
    const Descriptor& dst, uint32_t failureCause, time_t nextTryTime, 
        bool skipAttempt) 
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
                doChangeSmsStateToEnroute(connection, id, dst,
                                          failureCause, nextTryTime, skipAttempt);
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
                log.warn("Max tries count to update message state"
                         "#%d exceeded !\n", id);
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
    sms->attempts++;

    //fakeStore.Insert(id, sms);

#endif
}

void RemoteStore::doChangeSmsStateToDelivered(StorageConnection* connection,
    SMSId id, const Descriptor& dst)
        throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    ToDeliveredStatement* toDeliveredStmt
        = connection->getToDeliveredStatement();

    toDeliveredStmt->bindId(id);
    toDeliveredStmt->bindDestinationDescriptor((Descriptor &)dst);

    try
    {
        connection->check(toDeliveredStmt->execute());
    }
    catch (StorageException& exc)
    {
        connection->rollback();
        throw exc;
    }

    if (!toDeliveredStmt->wasUpdated())
    {
        connection->rollback();
        throw NoSuchMessageException(id);
    }
    connection->commit();
}
void RemoteStore::changeSmsStateToDelivered(SMSId id, const Descriptor& dst)
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
                doChangeSmsStateToDelivered(connection, id, dst);
                StoreManager::incrementFinalizedCount();
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
                log.warn("Max tries count to update message state"
                         "#%d exceeded !\n", id);
                throw;
            }
        }
    }

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

void RemoteStore::doChangeSmsStateToUndeliverable(
    StorageConnection* connection, SMSId id,
        const Descriptor& dst, uint32_t failureCause)
            throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    ToUndeliverableStatement* toUndeliverableStmt
        = connection->getToUndeliverableStatement();

    toUndeliverableStmt->bindId(id);
    toUndeliverableStmt->bindFailureCause((dvoid *)&(failureCause),
                                          (sb4) sizeof(failureCause));
    toUndeliverableStmt->bindDestinationDescriptor((Descriptor &)dst);

    try
    {
        connection->check(toUndeliverableStmt->execute());
    }
    catch (StorageException& exc)
    {
        connection->rollback();
        throw exc;
    }

    if (!toUndeliverableStmt->wasUpdated())
    {
        connection->rollback();
        throw NoSuchMessageException(id);
    }
    connection->commit();
}
void RemoteStore::changeSmsStateToUndeliverable(SMSId id,
    const Descriptor& dst, uint32_t failureCause)
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
                doChangeSmsStateToUndeliverable(connection, id,
                                                dst,failureCause);
                StoreManager::incrementFinalizedCount();
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
                log.warn("Max tries count to update message state"
                         "#%d exceeded !\n", id);
                throw;
            }
        }
    }

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

void RemoteStore::doChangeSmsStateToExpired(
    StorageConnection* connection, SMSId id)
        throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    ToExpiredStatement* toExpiredStmt
        = connection->getToExpiredStatement();

    toExpiredStmt->bindId(id);
    try
    {
        connection->check(toExpiredStmt->execute());
    }
    catch (StorageException& exc)
    {
        connection->rollback();
        throw exc;
    }

    if (!toExpiredStmt->wasUpdated())
    {
        connection->rollback();
        throw NoSuchMessageException(id);
    }
    connection->commit();
}
void RemoteStore::changeSmsStateToExpired(SMSId id)
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
                doChangeSmsStateToExpired(connection, id);
                StoreManager::incrementFinalizedCount();
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
                log.warn("Max tries count to update message state"
                         "#%d exceeded !\n", id);
                throw;
            }
        }
    }

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

void RemoteStore::doChangeSmsStateToDeleted(
    StorageConnection* connection, SMSId id)
        throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    ToDeletedStatement* toDeletedStmt
        = connection->getToDeletedStatement();

    toDeletedStmt->bindId(id);

    try
    {
        connection->check(toDeletedStmt->execute());
    }
    catch (StorageException& exc)
    {
        connection->rollback();
        throw exc;
    }

    if (!toDeletedStmt->wasUpdated())
    {
        connection->rollback();
        throw NoSuchMessageException(id);
    }
    connection->commit();
}
void RemoteStore::changeSmsStateToDeleted(SMSId id)
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
                doChangeSmsStateToDeleted(connection, id);
                StoreManager::incrementFinalizedCount();
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
                log.warn("Max tries count to update message state"
                         "#%d exceeded !\n", id);
                throw;
            }
        }
    }

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
    StorageConnectionPool* _pool, time_t retryTime) 
        throw(StorageException) : IdIterator(), pool(_pool), connection(0)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    connection = pool->getConnection();
    if (!connection) return;
    try
    {
        if (connection && !connection->isAvailable())
            connection->connect();
        
        readyStmt = new ReadyByNextTimeStatement(connection, false);
        if (readyStmt)
        {
            readyStmt->bindRetryTime(retryTime);
            connection->check(readyStmt->execute(OCI_DEFAULT, 0, 0));
        }
    }
    catch (...)
    {
        if (connection) pool->freeConnection(connection);
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

bool RemoteStore::ReadyIdIterator::getNextId(SMSId &id)
    throw(StorageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (readyStmt && connection && connection->isAvailable())
    {
        sword status = readyStmt->fetch();
        if (status != OCI_NO_DATA)
        {
            connection->check(status);
            readyStmt->getSMSId(id);
            return true;
        }
    }
    return false;
#else
    return false;
#endif
}

IdIterator* RemoteStore::getReadyForRetry(time_t retryTime)
    throw(StorageException)
{
    return (new ReadyIdIterator(pool, retryTime));
}

time_t RemoteStore::getNextRetryTime()
    throw(StorageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    time_t minTime = 0;
    Connection* connection = pool->getConnection();
    if (connection)
    {
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
        throw(StorageException) : IdIterator(), pool(_pool), connection(0)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    connection = pool->getConnection();
    if (!connection) return;
    try
    {
        if (connection && !connection->isAvailable()) 
            connection->connect();
        
        deliveryStmt = new DeliveryIdsStatement(connection, da, false);
        if (deliveryStmt)
            connection->check(deliveryStmt->execute(OCI_DEFAULT, 0, 0));
    }
    catch (...)
    {
        pool->freeConnection(connection);
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
bool RemoteStore::DeliveryIdIterator::getNextId(SMSId &id)
    throw(StorageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (deliveryStmt && connection && connection->isAvailable())
    {
        sword status = deliveryStmt->fetch();
        if (status != OCI_NO_DATA)
        {
            connection->check(status);
            deliveryStmt->getSMSId(id);
            return true;
        }
    }
    return false;
#else
    return false;
#endif
}

IdIterator* RemoteStore::getReadyForDelivery(const Address& da)
    throw(StorageException)
{
    return (new DeliveryIdIterator(pool, da));
}

/* --------------------- Service classes & methods -------------------- */

RemoteStore::CancelIdIterator::CancelIdIterator(StorageConnectionPool* _pool, 
    const Address& oa, const Address& da, const char* svc)
        throw(StorageException) : IdIterator(), pool(_pool), connection(0)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    connection = pool->getConnection();
    if (!connection) return;
    try
    {
        if (connection && !connection->isAvailable()) 
            connection->connect();
        
        cancelStmt = new CancelIdsStatement(connection, oa, da, svc, false);
        if (cancelStmt)
            connection->check(cancelStmt->execute(OCI_DEFAULT, 0, 0));
    }
    catch (...)
    {
        pool->freeConnection(connection);
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
bool RemoteStore::CancelIdIterator::getNextId(SMSId &id)
    throw(StorageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (cancelStmt && connection && connection->isAvailable())
    {
        sword status = cancelStmt->fetch();
        if (status != OCI_NO_DATA)
        {
            connection->check(status);
            cancelStmt->getSMSId(id);
            return true;
        }
    }
    return false;
#else
    return false;
#endif
}

IdIterator* RemoteStore::getReadyForCancel(const Address& oa, 
    const Address& da, const char* svcType = 0)
        throw(StorageException)
{
    return (new CancelIdIterator(pool, oa, da, svcType));
}

RemoteStore::ConcatInitIterator::ConcatInitIterator(StorageConnectionPool* _pool)
    throw(StorageException) 
        : ConcatDataIterator(), pool(_pool), connection(0)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    connection = pool->getConnection();
    if (!connection) return;
    try
    {
        if (connection && !connection->isAvailable()) 
            connection->connect();
        
        concatStmt = new ConcatDataStatement(connection, false);
        if (concatStmt)
            connection->check(concatStmt->execute(OCI_DEFAULT, 0, 0));
    }
    catch (...)
    {
        pool->freeConnection(connection);
        throw;
    }
#endif
}
RemoteStore::ConcatInitIterator::~ConcatInitIterator()
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE
    
    if (concatStmt) delete concatStmt;
    if (pool && connection) pool->freeConnection(connection);

#endif
}
bool RemoteStore::ConcatInitIterator::getNext()
    throw(StorageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (concatStmt && connection && connection->isAvailable())
    {
        sword status = concatStmt->fetch();
        if (status != OCI_NO_DATA)
        {
            connection->check(status);
            return true;
        }
    }
    return false;
#else
    return false;
#endif
}

ConcatDataIterator* RemoteStore::getConcatInitInfo()
    throw(StorageException)
{
    return (new ConcatInitIterator(pool));
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
        __trace2__("Cache size is %d. Cleaning SMS cache ...", idCache.Count());
        int toDelete = cacheCapacity/10;
        SMSId curId=lastId;
        
        while (curId<id && toDelete>0)
            if (delSms(curId++)) toDelete--;
        
        lastId = curId;
        __trace2__("Cache size is %d. SMS cache cleaned.", idCache.Count());
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
        log.warn("Config parameter: <MessageStore.Cache.capacity> missed. "
                 "Using default: %d.", maxCacheCapacity);
    }
}

CachedStore::CachedStore(Manager& config) 
    throw(ConfigException, StorageException) 
        : RemoteStore(config), cache(0),
            maxCacheCapacity(SMSC_MAX_SMS_CACHE_CAPACITY)
{
    loadMaxCacheCapacity(config);
    cache = new SmsCache(maxCacheCapacity);
    __require__(cache);
}
CachedStore::~CachedStore() 
{
    MutexGuard  guard(cacheMutex);
    if (cache) delete cache;
}

SMSId CachedStore::createSms(SMS& sms, SMSId id, const CreateMode flag)
    throw(StorageException, DuplicateMessageException)
{
    SMSId retId;

    __trace2__("Creating sms, smsId = %lld, flag = %d", id, flag);
    retId = RemoteStore::createSms(sms, id, flag);
    __trace2__("Created sms, smsId = %lld, retId = %lld", id, retId);
    
    SMS* sm = new SMS(sms);
    MutexGuard cacheGuard(cacheMutex);
    if (retId != id) cache->delSms(retId);
    cache->putSms(id, sm);

    return retId;
}

void CachedStore::retriveSms(SMSId id, SMS &sms)
    throw(StorageException, NoSuchMessageException)
{
    SMS* sm = 0;
    {
        MutexGuard cacheGuard(cacheMutex);
        sm = cache->getSms(id);
        if (sm) { 
            sms = *sm;
            __trace2__("smsId = %lld found in cache.", id);
            return;
        }
    }

    __trace2__("smsId = %lld retriving from DB ...", id);
    RemoteStore::retriveSms(id, sms);
    __trace2__("smsId = %lld retrived DB.", id);
    
    if (sms.state == ENROUTE)
    {
        sm = new SMS(sms);
        MutexGuard cacheGuard(cacheMutex);
        cache->putSms(id, sm);
    }
}

void CachedStore::changeSmsConcatSequenceNumber(SMSId id, int8_t inc) 
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing seqNum for smsId = %lld.", id);
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
void CachedStore::destroySms(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    RemoteStore::destroySms(id);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}

void CachedStore::changeSmsStateToEnroute(SMSId id, const Descriptor& dst, 
                                          uint32_t failureCause, 
                                          time_t nextTryTime, bool skipAttempt)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to ENROUTE for smsId = %lld.", id);
    time_t lastTryTime = time(0);
    RemoteStore::changeSmsStateToEnroute(id, dst, failureCause, nextTryTime, skipAttempt);
    
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
            if (!skipAttempt) sm->attempts++;
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
    __trace2__("Changing to DELIVERED for smsId = %lld.", id);
    RemoteStore::changeSmsStateToDelivered(id, dst);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}
void CachedStore::changeSmsStateToUndeliverable(SMSId id,
                                                const Descriptor& dst, 
                                                uint32_t failureCause)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to UNDELIVERABLE for smsId = %lld.", id);
    RemoteStore::changeSmsStateToUndeliverable(id, dst, failureCause);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}
void CachedStore::changeSmsStateToExpired(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to EXPIRED for smsId = %lld.", id);
    RemoteStore::changeSmsStateToExpired(id);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}
void CachedStore::changeSmsStateToDeleted(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to DELETED for smsId = %lld.", id);
    RemoteStore::changeSmsStateToDeleted(id);
    MutexGuard cacheGuard(cacheMutex);
    cache->delSms(id);
}

/* ------------------------------ Cached Store ------------------------------ */
}}
