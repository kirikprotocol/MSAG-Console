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

void RemoteStore::doCreateSms(StorageConnection* connection,
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
            return;
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
}
void RemoteStore::createSms(SMS& sms, SMSId id, const CreateMode flag)
    throw(StorageException, DuplicateMessageException)
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
            doCreateSms(connection, sms, id, flag);
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
    : idCache(capacity)
{
}
SmsCache::~SmsCache()
{
    clean();
}

void SmsCache::clean()
{
    idCache.First();
    SMSId id; IdxSMS* sms = 0;
    while (idCache.Next(id, sms)) {
        if (sms) delete sms;
        __trace2__("Message for smsId = %lld deleted from cache", id);
    }
        
    idCache.Clean();
}
void SmsCache::putSms(IdxSMS* sm)
{
    __require__(sm);
    idCache.Insert(sm->id, sm);
}
bool SmsCache::delSms(SMSId id)
{
    IdxSMS* sm=0;
    if (!idCache.Exists(id)) return false;
    idCache.Get(id, sm);
    if (!sm) return false;
    idCache.Delete(id);
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
            switcher(false),
            maxUncommitedCount(SMSC_MAX_UNCOMMITED_UPDATES),
            maxSleepInterval(SMSC_MAX_SLEEP_INTERVAL),
            bStarted(false), bNeedExit(false),
            processingUpdates(false)
{
    loadMaxUncommitedCount(config);
    loadSleepInterval(config);
    updates = &buffers[switcher ? 1:0];
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
    int current_pri = 0;
    if (thr_getprio(thr_self(), &current_pri) ||
        thr_setprio(thr_self(), current_pri+2))
    {
        log.warn("Can't change thread priority.");
    }
    
    while (!bNeedExit)
    {
        processEvent.Wait(maxSleepInterval);
        
        {
            MutexGuard g(processingUpdatesMutex);
            if( processingUpdates ) continue;
            else processingUpdates = true;
        }
        
        MutexGuard updatesGuard(processUpdatesMutex);
        processUpdates();
        
        {
            MutexGuard g(processingUpdatesMutex);
            processingUpdates = false;
        }
    }
    exitedEvent.Signal();
}

void CachedStore::createSms(SMS& sms, SMSId id,
                            const CreateMode flag = CREATE_NEW)
    throw(StorageException, DuplicateMessageException)
{
    if (flag == CREATE_NEW)
    {
        __trace2__("Creating new smsId = %lld", id);
        RemoteStore::createSms(sms, id, CREATE_NEW);
        IdxSMS* ism = new IdxSMS(id, sms);
        
        MutexGuard cacheGuard(cacheMutex);
        cache.putSms(ism);
    }
    else 
    {
        __trace2__("Creating <%s> smsId = %lld", 
                   (flag == ETSI_REJECT_IF_PRESENT) ?
                   "ETSI_REJECT_IF_PRESENT":"SMPP_OVERWRITE_IF_PRESENT", id);
        
        MutexGuard updatesGuard(processUpdatesMutex);
        processUpdates();
        
        MutexGuard cacheGuard(cacheMutex);
        RemoteStore::createSms(sms, id, flag);
        __trace2__("Created <%s> smsId = %lld", 
                   (flag == ETSI_REJECT_IF_PRESENT) ?
                   "ETSI_REJECT_IF_PRESENT":"SMPP_OVERWRITE_IF_PRESENT", id);
        if (flag == SMPP_OVERWRITE_IF_PRESENT) 
        {
            __trace2__("Cleaning SMS cache ...");
            cache.clean();
            __trace2__("SMS Cache cleaned.");
        }
    }
}

void CachedStore::retriveSms(SMSId id, SMS &sms)
    throw(StorageException, NoSuchMessageException)
{
    {
        MutexGuard cacheGuard(cacheMutex);
        SMS* sm = cache.getSms(id);
        if (sm) { 
            sms = *sm;
            __trace2__("smsId = %lld found in cache.", id);
            return;
        }
    }

    __trace2__("smsId = %lld retriving from DB ...", id);
    RemoteStore::retriveSms(id, sms);
    __trace2__("smsId = %lld retrived DB.", id);
    
    IdxSMS* ism = new IdxSMS(id, sms);
    MutexGuard cacheGuard(cacheMutex);
    cache.putSms(ism);
}

void CachedStore::replaceSms(SMSId id, const Address& oa,
                             const uint8_t* newMsg, uint8_t newMsgLen,
                             uint8_t deliveryReport, 
                             time_t validTime, time_t waitTime)
    throw(StorageException, NoSuchMessageException)
{
    
    MutexGuard updatesGuard(processUpdatesMutex);
    processUpdates();
    RemoteStore::replaceSms(id, oa, newMsg, newMsgLen, deliveryReport,
                            validTime, waitTime);
    MutexGuard cacheGuard(cacheMutex);
    cache.delSms(id);
}
void CachedStore::destroySms(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    MutexGuard updatesGuard(processUpdatesMutex);
    processUpdates();
    
    MutexGuard cacheGuard(cacheMutex);
    cache.delSms(id);
    RemoteStore::destroySms(id);
}

void CachedStore::actualizeCache(SMS* sm, UpdateRecord* update)
{
    __require__(sm);
    __require__(update);

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
}
void CachedStore::actualizeUpdate(UpdateRecord* update)
    throw(StorageException, NoSuchMessageException)
{
    __require__(update);

    {
        MutexGuard cacheGuard(cacheMutex);
        SMS* sm = cache.getSms(update->id);
        if (sm) 
        {
            if (sm->state != ENROUTE) 
            {
                SMSId id = update->id;
                if (update) delete update;
                throw NoSuchMessageException(id);
            }
            actualizeCache(sm, update);
            return;
        }
    }
    
    SMS sms;
    try 
    {
        RemoteStore::retriveSms(update->id, sms);
        if (sms.state != ENROUTE) 
            throw NoSuchMessageException(update->id);
    }
    catch (Exception& exc)
    {
        if (update) delete update;
        throw;
    }
    
    IdxSMS* ism = new IdxSMS(update->id, sms);
    MutexGuard cacheGuard(cacheMutex);
    cache.putSms(ism);
    actualizeCache(ism, update);
}
void CachedStore::addUpdate(UpdateRecord* update)
    throw(StorageException, NoSuchMessageException)
{
    __require__(updates);
    __require__(update);

    int updatesCount = 0;
    {
        MutexGuard guard(updatesMutex);
        actualizeUpdate(update);
        
        updates->Push(update);
        updatesCount = updates->Count();
    }
    if (updatesCount >= maxUncommitedCount) 
    {
        // ∆дЄм пока предыдущий процессинг пройдЄт
        MutexGuard updatesGuard(processUpdatesMutex);
        {
            MutexGuard g(processingUpdatesMutex);
            processingUpdates = true;
        }
        if (updates->Count() >= maxUncommitedCount) processUpdates();
        {
            MutexGuard g(processingUpdatesMutex);
            processingUpdates = false;
        }
//      processEvent.Signal();
    }
}

static time_t lastProcessUpdatesTime = time(0);

void CachedStore::processUpdates()
{
    // Need to block on processUpdatesMutex before calling this routine
    time_t thisProcessUpdatesTime = time(0); 
    
    UpdatesIdMap* drop = 0;
    {
        // Switching to another updates buffer
        MutexGuard guard(updatesMutex);
        drop = updates;  
        updates = &buffers[(switcher = !switcher) ? 1:0];
        __trace2__("Process updates called (after %d seconds)."
                   "%d updates to drop. Switched to buffer %d", 
                   thisProcessUpdatesTime-lastProcessUpdatesTime,
                   drop->Count(), (switcher) ? 1:0);
        lastProcessUpdatesTime = thisProcessUpdatesTime;
    } 
    
    StorageConnection* connection = 0;
    try 
    {
        connection = (StorageConnection *)pool->getConnection();
        __require__(connection);
        
        for (int i=0; i<drop->Count(); i++)
        {
            try 
            {
                UpdateRecord* update = (*drop)[i];
                if (update) processUpdate(connection, update);
            } 
            catch (NoSuchMessageException& exc) 
            {
                log.error("Unable to update message. %s", exc.what());
            }
        }
        connection->commit();
        for (int i=0; i<drop->Count(); i++)
        {
            UpdateRecord* update = (*drop)[i];
            if (update) delete update;
        }
        drop->Clean();
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
                                UpdateRecord* update)
    throw(StorageException, NoSuchMessageException)
{
    // Need to block on processUpdatesMutex before calling this routine

    __require__(update);
    __require__(connection);
    
    switch (update->state)
    {
    case ENROUTE:
        __trace2__("DB to ENROUTE for smsId = %lld.", update->id);
        doChangeSmsStateToEnroute(connection, update->id, update->dst,
                                  update->fcs, update->nt);
        __trace2__("Changed to ENROUTE for smsId = %lld.", update->id);
        break;
    case DELIVERED:
        __trace2__("DB to DELIVERED for smsId = %lld.", update->id);
        doChangeSmsStateToDelivered(connection, update->id, update->dst);
        __trace2__("Changed to DELIVERED for smsId = %lld.", update->id);
        break;
    case EXPIRED:
        __trace2__("DB to EXPIRED for smsId = %lld.", update->id);
        doChangeSmsStateToExpired(connection, update->id);
        __trace2__("Changed to EXPIRED for smsId = %lld.", update->id);
        break;
    case UNDELIVERABLE:
        __trace2__("DB to UNDELIVERABLE for smsId = %lld.", update->id);
        doChangeSmsStateToUndeliverable(connection, update->id,
                                        update->dst, update->fcs);
        __trace2__("Changed to UNDELIVERABLE for smsId = %lld.", update->id);
        break;
    case DELETED:
        __trace2__("DB to DELETED for smsId = %lld.", update->id);
        doChangeSmsStateToDeleted(connection, update->id);
        __trace2__("Changed to DELETED for smsId = %lld.", update->id);
        break;
    default:
        log.error("Unknown state for update: %d", update->state);
        break;
    }

    if (update->state != ENROUTE) 
    {
        MutexGuard cacheGuard(cacheMutex);
        cache.delSms(update->id);
    }
}

void CachedStore::changeSmsStateToEnroute(SMSId id, const Descriptor& dst, 
                                          uint32_t failureCause, 
                                          time_t nextTryTime)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to ENROUTE for smsId = %lld.", id);
    addUpdate(new UpdateRecord(id, ENROUTE, dst, failureCause, nextTryTime));
}
void CachedStore::changeSmsStateToDelivered(SMSId id,
                                            const Descriptor& dst)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to DELIVERED for smsId = %lld.", id);
    addUpdate(new UpdateRecord(id, DELIVERED, dst));
}
void CachedStore::changeSmsStateToUndeliverable(SMSId id,
                                                const Descriptor& dst, 
                                                uint32_t failureCause)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to UNDELIVERABLE for smsId = %lld.", id);
    addUpdate(new UpdateRecord(id, UNDELIVERABLE, dst, failureCause));
}
void CachedStore::changeSmsStateToExpired(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to EXPIRED for smsId = %lld.", id);
    addUpdate(new UpdateRecord(id, EXPIRED));
}
void CachedStore::changeSmsStateToDeleted(SMSId id)
    throw(StorageException, NoSuchMessageException)
{
    __trace2__("Changing to DELETED for smsId = %lld.", id);
    addUpdate(new UpdateRecord(id, DELETED));
}
IdIterator* CachedStore::getReadyForRetry(time_t retryTime)
    throw(StorageException)
{
    MutexGuard updatesGuard(processUpdatesMutex);
    processUpdates();
    return RemoteStore::getReadyForRetry(retryTime);
}
time_t CachedStore::getNextRetryTime()
    throw(StorageException)
{
    MutexGuard updatesGuard(processUpdatesMutex);
    processUpdates();
    return RemoteStore::getNextRetryTime();
}

/* ------------------------------ Cached Store ------------------------------ */
}}
