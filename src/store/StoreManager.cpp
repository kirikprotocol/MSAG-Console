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
        needOverwriteStmt->bindDealiasedDestinationAddress(
            sms.dealiasedDestinationAddress);

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
        needRejectStmt->bindDealiasedDestinationAddress(
            sms.dealiasedDestinationAddress);
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

    SMSId retId = 0; 
    StorageConnection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try
        {
            connection = (StorageConnection *)pool->getConnection();
            retId = doCreateSms(connection, sms, id, flag);
            pool->freeConnection(connection);
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
            doRetrieveSms(connection, id, sms);
            pool->freeConnection(connection);
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
            doDestroySms(connection, id);
            pool->freeConnection(connection);
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

    body.setBinProperty(Tag::SMPP_SHORT_MESSAGE, (const char*)newMsg,
                        (unsigned)newMsgLen);
    body.setIntProperty(Tag::SMPP_SM_LENGTH, (uint32_t)newMsgLen);

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
            doReplaceSms(connection, id, oa, newMsg, newMsgLen,
                         deliveryReport, validTime, waitTime);
            pool->freeConnection(connection);
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

    sms->getMessageBody().setBinProperty(Tag::SMPP_SHORT_MESSAGE,
        (const char*)newMsg, (unsigned)newMsgLen);
    sms->getMessageBody().setIntProperty(Tag::SMPP_SM_LENGTH,
        (uint32_t)newMsgLen);

    //fakeStore.Insert(id, new SMS(sms));

#endif
}

void RemoteStore::doChangeSmsStateToEnroute(StorageConnection* connection,
    SMSId id, const Descriptor& dst, uint32_t failureCause, time_t nextTryTime)
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
    const Descriptor& dst, uint32_t failureCause, time_t nextTryTime)
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
            doChangeSmsStateToEnroute(connection, id, dst,
                                      failureCause, nextTryTime);
            pool->freeConnection(connection);
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
            doChangeSmsStateToDelivered(connection, id, dst);
            StoreManager::incrementFinalizedCount();
            pool->freeConnection(connection);
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
            doChangeSmsStateToUndeliverable(connection, id,
                                            dst,failureCause);
            StoreManager::incrementFinalizedCount();
            pool->freeConnection(connection);
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
            doChangeSmsStateToExpired(connection, id);
            StoreManager::incrementFinalizedCount();
            pool->freeConnection(connection);
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
            doChangeSmsStateToDeleted(connection, id);
            StoreManager::incrementFinalizedCount();
            pool->freeConnection(connection);
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
        throw(StorageException) : IdIterator(), pool(_pool)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    connection = pool->getConnection();
    try
    {
        if (connection && !connection->isAvailable())
        {
            connection->connect();
        }

        readyStmt = new ReadyByNextTimeStatement(connection, false);
        if (readyStmt)
        {
            readyStmt->bindRetryTime(retryTime);
            connection->check(readyStmt->execute(OCI_DEFAULT, 0, 0));
        }
    }
    catch (...)
    {
        pool->freeConnection(connection);
        throw;
    }
#endif
}
RemoteStore::ReadyIdIterator::~ReadyIdIterator()
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (readyStmt) delete readyStmt;
    pool->freeConnection(connection);

#endif
}

bool RemoteStore::ReadyIdIterator::getNextId(SMSId &id)
    throw(StorageException)
{
#ifndef SMSC_FAKE_MEMORY_MESSAGE_STORE

    if (readyStmt && connection->isAvailable())
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
            if (!connection->isAvailable())
            {
                connection->connect();
            }
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
/* ------------------------------ Remote Store ------------------------------ */

/* ------------------------------- SMS Cache -------------------------------- */

const int SMSC_MAX_SMS_CACHE_CAPACITY = 10000;

SmsCache::SmsCache(int capacity=0) 
    : idCache(capacity), mrCache(capacity), stCache(capacity)
{
}
SmsCache::~SmsCache()
{
    idCache.First();
    SMSId id; IdxSMS* sms = 0;
    while (idCache.Next(id, sms))
        if (sms) delete sms;
    
    idCache.Clean();
    mrCache.Clean();
    stCache.Clean();
}

void SmsCache::putSms(IdxSMS* sm)
{
    __require__(sm);

    idCache.Insert(sm->id, sm);
    /*mrCache.Insert(ComplexMrIdx(sm->originatingAddress,
                                sm->dealiasedDestinationAddress,
                                sm->messageReference), sm);
    stCache.Insert(ComplexStIdx(sm->originatingAddress,
                                sm->dealiasedDestinationAddress,
                                sm->eServiceType), sm);*/
}
bool SmsCache::delSms(SMSId id)
{
    IdxSMS* sm=0;
    if (!idCache.Exists(id)) return false;
    idCache.Get(id, sm);
    if (!sm) return false;
    idCache.Delete(id);
    /*ComplexMrIdx mrIdx(sm->originatingAddress,
                       sm->dealiasedDestinationAddress,
                       sm->messageReference);
    mrCache.Delete(mrIdx);
    ComplexStIdx stIdx(sm->originatingAddress,
                       sm->dealiasedDestinationAddress,
                       sm->eServiceType);
    stCache.Delete(stIdx);*/
    delete sm;
    __trace2__("Message for smsId = %lld deleted from cache", id);
    return true;
}
SMS* SmsCache::getSms(SMSId id)
{
    IdxSMS* sm = 0;
    if (idCache.Exists(id)) idCache.Get(id, sm);
    __trace2__("getSms() = %p smsId = %lld", sm, id);
    return (SMS *)sm;
}
SMS* SmsCache::getSms(const Address& oa, const Address& da, 
                      const char* svc, SMSId& id)
{
    IdxSMS* sm = 0; id = 0;
    ComplexStIdx idx(oa, da, svc);
    if (stCache.Exists(idx)) {
        stCache.Get(idx, sm);
        if (sm) id = sm->id;
    }
    return (SMS *)sm;
}
SMS* SmsCache::getSms(const Address& oa, const Address& da, 
                      uint16_t mr, SMSId& id)
{
    IdxSMS* sm = 0; id = 0;
    ComplexMrIdx idx(oa, da, mr);
    if (mrCache.Exists(idx)) {
        mrCache.Get(idx, sm);
        if (sm) id = sm->id;
    }
    return (SMS *)sm;
}
/* ------------------------------- SMS Cache -------------------------------- */

/* ------------------------------ Cached Store ------------------------------ */

const int SMSC_MAX_UNCOMMITED_UPDATES = 1000;
const int SMSC_MAX_SLEEP_INTERVAL = 60000;

void CachedStore::loadMaxUncommitedCount(Manager& config) 
{
    try
    {
        maxUncommitedCount = 
            (unsigned)config.getInt("MessageStore.Cache.uncommited");
        if (maxUncommitedCount <= 0 ||
            maxUncommitedCount > SMSC_MAX_UNCOMMITED_UPDATES)
        {
            maxUncommitedCount = SMSC_MAX_UNCOMMITED_UPDATES;
            log.warn("Maximum uncommited updates count parameter "
                     "is incorrect (should be between 1 and %u) ! "
                     "Config parameter: <MessageStore.Cache.uncommited> "
                     "Using default: %u", SMSC_MAX_UNCOMMITED_UPDATES,
                     SMSC_MAX_UNCOMMITED_UPDATES);
        }
    }
    catch (ConfigException& exc)
    {
        maxUncommitedCount = SMSC_MAX_UNCOMMITED_UPDATES;
        log.warn("Maximum uncommited updates count parameter "
                 "wasn't specified ! "
                 "Config parameter: <MessageStore.Cache.uncommited> "
                 "Using default: %d", SMSC_MAX_UNCOMMITED_UPDATES);
    }
}
void CachedStore::loadSleepInterval(Manager& config) 
{
    try
    {
        maxSleepInterval = 
            (unsigned)config.getInt("MessageStore.Cache.sleep");
        if (maxSleepInterval < 0 ||
            maxSleepInterval > SMSC_MAX_SLEEP_INTERVAL)
        {
            maxSleepInterval = SMSC_MAX_SLEEP_INTERVAL;
            log.warn("Time interval between updates applies parameter "
                     "is incorrect (should be between 0 and %u) ! "
                     "Config parameter: <MessageStore.Cache.sleep> "
                     "Using default: %u", SMSC_MAX_SLEEP_INTERVAL,
                     SMSC_MAX_SLEEP_INTERVAL);
        }
    }
    catch (ConfigException& exc)
    {
        maxSleepInterval = SMSC_MAX_SLEEP_INTERVAL;
        log.warn("Time interval between updates applies parameter "
                 "wasn't specified ! "
                 "Config parameter: <MessageStore.Cache.sleep> "
                 "Using default: %d", SMSC_MAX_SLEEP_INTERVAL);
    }
}

CachedStore::CachedStore(Manager& config) 
    throw(ConfigException, StorageException) 
        : RemoteStore(config), cache(SMSC_MAX_SMS_CACHE_CAPACITY),
            maxUncommitedCount(SMSC_MAX_UNCOMMITED_UPDATES),
            maxSleepInterval(SMSC_MAX_SLEEP_INTERVAL),
            bStarted(false), bNeedExit(false)
{
    loadMaxUncommitedCount(config);
    loadSleepInterval(config);
    Start();
}
CachedStore::~CachedStore() 
{
    Stop(); // Flush cash here, apply updates & etc ... 
}
void CachedStore::Start() 
{
    MutexGuard  guard(startLock);

    if (!bStarted)
    {
        bNeedExit = false;
        Thread::Start();
        bStarted = true;
    }
}
void CachedStore::Stop() 
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        bNeedExit = true;
        processEvent.Signal();
        exitedEvent.Wait();
        bStarted = false;
    }
}
int CachedStore::Execute() // Thread for updates appling & committing 
{
    while (!bNeedExit)
    {
        processEvent.Wait(maxSleepInterval);
        {
            MutexGuard guard(cacheMutex);
            processUpdates();
        }
    }
    exitedEvent.Signal();
}

/*SMSId CachedStore::createSms(SMS& sms, SMSId id,
                            const CreateMode flag = CREATE_NEW)
    throw(StorageException, DuplicateMessageException)
{
    SMSId retId = 0;

    if (flag == ETSI_REJECT_IF_PRESENT)
    {
        SMS* sm = 0; SMSId oldId;
        
        MutexGuard guard(cacheMutex);
        sm = cache.getSms(sms.originatingAddress, 
                          sms.dealiasedDestinationAddress, 
                          sms.messageReference, oldId);
        if (sm && sm->state == ENROUTE)
            throw DuplicateMessageException(id);

        // Нету в кэше, идём в базу ... если Ok кладём в кэш.
        // Есть в кэше, но состояние уже финальное.
        // Реджектить нельзя, нужно создавать новую.
        __trace2__("CreateSms: creating rejectif smsId = %lld flag = %d ", 
                   id, (!sm) ? ETSI_REJECT_IF_PRESENT:CREATE_NEW );
        retId = RemoteStore::createSms(sms, id, (!sm) ? 
                                       ETSI_REJECT_IF_PRESENT:CREATE_NEW);
        
        cache.putSms(new IdxSMS(id, sms));
    }
    else if (flag == SMPP_OVERWRITE_IF_PRESENT)
    {
        SMS* sm = 0; SMSId oldId=0;
        
        MutexGuard guard(cacheMutex);
        sm = cache.getSms(sms.originatingAddress, 
                          sms.dealiasedDestinationAddress, 
                          sms.eServiceType, oldId);
        
        if (sm && sm->state == ENROUTE) 
        {
            // Есть в кэше, состояние не финальное. 
            // Нужно игнорировать все update'ы этой sms !!!
            __trace2__("CreateSms: creating overwriteif (exist in cache) "
                       "oldId = %lld, smsId = %lld flag = %d", 
                       oldId, id, SMPP_OVERWRITE_IF_PRESENT);
            processUpdates(oldId);
            retId = RemoteStore::createSms(sms, id, SMPP_OVERWRITE_IF_PRESENT);
            cache.delSms(oldId);
            delUpdates(oldId);
            __trace2__("CreateSms: clean up for smsId = %lld (exist)", oldId);
        }
        else
        {
            // Нету в кэше, идём в базу ...  если Ok кладём в кэш.
            __trace2__("CreateSms: creating overwriteif "
                       "(final | not exist in cache) smsId = %lld flag = %d",
                       id, (!sm) ? SMPP_OVERWRITE_IF_PRESENT:CREATE_NEW);
            // Есть в кэше, но состояние уже финальное. 
            // Реплэйсить нельзя, нужно создавать новую.
            retId = RemoteStore::createSms(sms, id, (!sm) ? 
                                           SMPP_OVERWRITE_IF_PRESENT:CREATE_NEW);
            if (!sm) 
            {
                __trace2__("CreateSms: clean up for smsId = %lld (not exist)", 
                           retId);
                cache.delSms(retId); // На всякий случай.
                delUpdates(retId);
            }
            else 
            {
                __trace2__("CreateSms: created overwriteif (final) "
                           "oldId = %lld smsId = %lld", 
                           oldId, id);
                cache.putSms(new IdxSMS(id, sms));
            }
        }
    }
    else // CREATE_NEW
    {
        __trace2__("CreateSms: creating new smsId = %lld", id);
        retId = RemoteStore::createSms(sms, id, CREATE_NEW);
        MutexGuard guard(cacheMutex);
        cache.putSms(new IdxSMS(id, sms));
    }

    return retId;
}*/

SMSId CachedStore::createSms(SMS& sms, SMSId id,
                            const CreateMode flag = CREATE_NEW)
    throw(StorageException, DuplicateMessageException)
{
    SMSId retId = 0;

    if (flag == CREATE_NEW)
    {
        __trace2__("CreateSms: creating new smsId = %lld", id);
        retId = RemoteStore::createSms(sms, id, CREATE_NEW);
        MutexGuard guard(cacheMutex);
        cache.putSms(new IdxSMS(id, sms));
    }
    else 
    {
        __trace2__("CreateSms: creating %d smsId = %lld", flag, id);
        MutexGuard guard(cacheMutex);
        processUpdates();
        retId = RemoteStore::createSms(sms, id, flag);
    }

    return retId;
}

void CachedStore::retriveSms(SMSId id, SMS &sms)
    throw(StorageException, NoSuchMessageException)
{
    MutexGuard guard(cacheMutex);
    sms = *(_retriveSms(id));
}

inline SMS* CachedStore::_retriveSms(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    SMS* sm = cache.getSms(id);
    if (!sm) 
    { 
        __trace2__("smsId = %lld not exist in cache. Retriving from DB...", id);
        SMS sms;
        RemoteStore::retriveSms(id, sms);
        IdxSMS* ism = new IdxSMS(id, sms);
        cache.putSms(ism);
        sm = (SMS *)ism;
        __trace2__("smsId = %lld retrived from DB, att = %d", 
                   id, ism->attempts);
    }
    return sm;
}

void CachedStore::replaceSms(SMSId id, const Address& oa,
                             const uint8_t* newMsg, uint8_t newMsgLen,
                             uint8_t deliveryReport, 
                             time_t validTime, time_t waitTime)
    throw(StorageException, NoSuchMessageException)
{
    SMS* sm = 0;
    MutexGuard guard(cacheMutex);
    sm = cache.getSms(id);
    
    if (!sm) 
    {
        processUpdates(id); // Нужно пропроцессить только для этого id
        cache.delSms(id);   // На всякий случай ...
        RemoteStore::replaceSms(id, oa, newMsg, newMsgLen, deliveryReport,
                                validTime, waitTime);
    }
    else // есть в кэше
    {
        if (!(sm->state == ENROUTE && sm->originatingAddress == oa)) 
            throw NoSuchMessageException(id);

        // Есть в кэше, состояние не финальное. 
        processUpdates(id); // Нужно пропроцессить только для этого id
        cache.delSms(id);
        RemoteStore::replaceSms(id, oa, newMsg, newMsgLen, deliveryReport,
                                validTime, waitTime);
    }
}
void CachedStore::destroySms(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    {
        MutexGuard guard(cacheMutex);
        cache.delSms(id);
        delUpdates(id);
    }
    RemoteStore::destroySms(id);
}

void CachedStore::actualizeUpdate(SMSId id, UpdateRecord* update)
    throw(StorageException, NoSuchMessageException)
{
    SMS* sm = _retriveSms(id); // Зачитываем в кэш
    if (!sm || sm->state != ENROUTE) 
        throw NoSuchMessageException(id);
    
    switch (update->state)
    {
    case ENROUTE:
        sm->state = ENROUTE;
        sm->destinationDescriptor = update->dst;
        sm->lastResult = update->fcs;
        sm->lastTime = time(0);
        sm->nextTime = update->nt;
        sm->attempts++;
        break;
    case DELIVERED:
        sm->state = DELIVERED;
        sm->destinationDescriptor = update->dst;
        sm->lastResult = 0;
        sm->lastTime = time(0);
        sm->nextTime = 0;
        sm->attempts++;
        break;
    case EXPIRED:
        sm->state = EXPIRED;
        sm->nextTime = 0;
        break;
    case UNDELIVERABLE:
        sm->state = UNDELIVERABLE;
        sm->destinationDescriptor = update->dst;
        sm->lastResult = update->fcs;
        sm->lastTime = time(0);
        sm->nextTime = 0;
        sm->attempts++;
        break;
    case DELETED:
        sm->nextTime = 0;
        sm->state = DELETED;
        break;
    default:
        log.error("Unknown state for update: %d", update->state);
        break;
    }

    __trace2__("smsId = %lld, Changing to %d, att = %d", 
               id, update->state, sm->attempts); 
}
void CachedStore::addUpdate(SMSId id, UpdateRecord* update)
    throw(StorageException, NoSuchMessageException)
{
    __require__(update);

    MutexGuard guard(cacheMutex);
    
    actualizeUpdate(id, update);
    updates.insert(std::pair<SMSId, UpdateRecord*>(id, update));
    if (updates.size() >= maxUncommitedCount) processUpdates();
}
bool CachedStore::delUpdates(SMSId id)
{
    // Need to block on cacheMutex before calling this routine   
    
    std::pair<UpdatesIdMap::iterator, 
              UpdatesIdMap::iterator> its = updates.equal_range(id);
    
    UpdatesIdMap::iterator i = its.first;
    UpdatesIdMap::iterator end = its.second;
    
    while (i != end) 
    {
        UpdateRecord* update = (*i++).second;
        if (update) delete update;
    }
    
    int erased = updates.erase(id);
    __trace2__("Erased %d updates for smsId = %lld", erased, id);
    return (erased>0);
}
void CachedStore::processUpdates(SMSId forId) // if forId == 0 => all
{
    // Need to block on cacheMutex before calling this routine

    StorageConnection* connection = 0;
    try 
    {
        connection = (StorageConnection *)pool->getConnection();
        if (!connection) return;
        
        UpdatesIdMap::iterator begin;
        UpdatesIdMap::iterator end;
        UpdatesIdMap::iterator i;

        if (forId)
        {
            std::pair<UpdatesIdMap::iterator, 
                  UpdatesIdMap::iterator> its = updates.equal_range(forId);
            begin = its.first; end = its.second;
        }
        else
        {
            begin = updates.begin(); end = updates.end();
        }

        for (i = begin; i != end; i++)
        {
            try 
            {
                UpdateRecord* update = (*i).second;
                SMSId id = (*i).first;
                if (update) processUpdate(connection, id, update);
            } 
            catch (NoSuchMessageException& exc) 
            {
                log.error("Unable to update message. %s", exc.what());
            }
        }
        connection->commit();
        for (i = begin; i != end; i++)
        {
            UpdateRecord* update = (*i).second;
            if (update) delete update;
        }
        
        if (forId) updates.erase(forId);
        else updates.clear();
    }
    catch (Exception& exc) 
    {
        try 
        {
            if (connection) connection->rollback();
            log.error("Failed to process updates. %s", exc.what());
        }
        catch (Exception& exc) 
        {
            log.error("Failed to rollback updates. %s", exc.what());
        }
    }
    if (connection) pool->freeConnection(connection);
}
void CachedStore::processUpdate(StorageConnection* connection, 
                                SMSId id, UpdateRecord* update)
    throw(StorageException, NoSuchMessageException)
{
    // Need to block on cacheMutex before calling this routine

    __require__(update);
    __require__(connection);
    
    switch (update->state)
    {
    case ENROUTE:
        __trace2__("DB to ENROUTE for smsId = %lld.", id);
        doChangeSmsStateToEnroute(connection, id, update->dst,
                                  update->fcs, update->nt);
        __trace2__("Changed to ENROUTE for smsId = %lld.", id);
        break;
    case DELIVERED:
        __trace2__("DB to DELIVERED for smsId = %lld.", id);
        doChangeSmsStateToDelivered(connection, id, update->dst);
        __trace2__("Changed to DELIVERED for smsId = %lld.", id);
        break;
    case EXPIRED:
        __trace2__("DB to EXPIRED for smsId = %lld.", id);
        doChangeSmsStateToExpired(connection, id);
        __trace2__("Changed to EXPIRED for smsId = %lld.", id);
        break;
    case UNDELIVERABLE:
        __trace2__("DB to UNDELIVERABLE for smsId = %lld.", id);
        doChangeSmsStateToUndeliverable(connection, id,
                                        update->dst, update->fcs);
        __trace2__("Changed to UNDELIVERABLE for smsId = %lld.", id);
        break;
    case DELETED:
        __trace2__("DB to DELETED for smsId = %lld.", id);
        doChangeSmsStateToDeleted(connection, id);
        __trace2__("Changed to DELETED for smsId = %lld.", id);
        break;
    default:
        log.error("Unknown state for update: %d", update->state);
        break;
    }

    if (update->state != ENROUTE) cache.delSms(id);
}

void CachedStore::changeSmsStateToEnroute(SMSId id, const Descriptor& dst, 
                                          uint32_t failureCause, 
                                          time_t nextTryTime)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to ENROUTE for smsId = %lld.", id);
    addUpdate(id, new UpdateRecord(ENROUTE, dst, failureCause, nextTryTime));
}
void CachedStore::changeSmsStateToDelivered(SMSId id,
                                            const Descriptor& dst)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to DELIVERED for smsId = %lld.", id);
    addUpdate(id, new UpdateRecord(DELIVERED, dst));
}
void CachedStore::changeSmsStateToUndeliverable(SMSId id,
                                                const Descriptor& dst, 
                                                uint32_t failureCause)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to UNDELIVERABLE for smsId = %lld.", id);
    addUpdate(id, new UpdateRecord(UNDELIVERABLE, dst, failureCause));
}
void CachedStore::changeSmsStateToExpired(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to EXPIRED for smsId = %lld.", id);
    addUpdate(id, new UpdateRecord(EXPIRED));
}
void CachedStore::changeSmsStateToDeleted(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to DELETED for smsId = %lld.", id);
    addUpdate(id, new UpdateRecord(DELETED));
}
IdIterator* CachedStore::getReadyForRetry(time_t retryTime)
    throw(StorageException)
{
    {
        MutexGuard guard(cacheMutex);
        processUpdates();
    }
    return RemoteStore::getReadyForRetry(retryTime);
}
time_t CachedStore::getNextRetryTime()
    throw(StorageException)
{
    {
        MutexGuard guard(cacheMutex);
        processUpdates();
    }
    return RemoteStore::getNextRetryTime();
}

/* ------------------------------ Cached Store ------------------------------ */
}}
