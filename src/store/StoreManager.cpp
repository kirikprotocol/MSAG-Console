#include <stdio.h>
#include <stdlib.h>

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

Mutex StoreManager::mutex;
Archiver* StoreManager::archiver = 0L;
ConnectionPool* StoreManager::pool = 0L;
IDGenerator* StoreManager::generator = 0L;
StoreManager* StoreManager::instance = 0L;
unsigned StoreManager::maxTriesCount = SMSC_MAX_TRIES_TO_PROCESS_OPERATION;
log4cpp::Category& StoreManager::log = Logger::getCategory("smsc.store.StoreManager");

void StoreManager::loadMaxTriesCount(Manager& config)
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

void StoreManager::startup(Manager& config)
    throw(ConfigException, ConnectionFailedException)
{
    MutexGuard guard(mutex);
    
    if (!instance)
    {
        log.info("Storage Manager is starting ... ");
        loadMaxTriesCount(config);
        try 
        {
            pool = new ConnectionPool(config);
            archiver = new Archiver(config);
            generator = new IDGenerator(archiver->getLastUsedId());
            //archiver->Start();
        }
        catch (StorageException& exc)
        {
            if (pool) {
                delete pool; pool = 0L;
            }
            if (archiver) {
                delete archiver; archiver = 0L;
            }
            if (generator) {
                delete generator; generator = 0L;
            }
            throw ConnectionFailedException(exc);
        }
        instance = new StoreManager();
        log.info("Storage Manager was started up.");
    }
}
        
void StoreManager::shutdown() 
{
    MutexGuard guard(mutex);

    if (pool && instance && generator && archiver)
    {
        log.info("Storage Manager is shutting down ...");
        delete pool; pool = 0L;
        delete instance; instance = 0L;
        delete archiver; archiver = 0L;
        delete generator; generator = 0L;
        log.info("Storage Manager was shutdowned.");
    }   
}

SMSId StoreManager::doCreateSms(Connection* connection,
    SMS& sms, const CreateMode flag)
        throw(StorageException, DuplicateMessageException)
{
    __require__(connection);
    
    SMSId id;
    
    Descriptor  dsc;
    sms.state = ENROUTE;
    sms.destinationDescriptor = dsc;
    sms.lastTime = 0; sms.nextTime = 0; sms.failureCause = 0; sms.attempts = 0;
    
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
            needOverwriteStmt->getId(id);
            
            OverwriteStatement* overwriteStmt 
                = connection->getOverwriteStatement();
            
            overwriteStmt->bindId(id);
            overwriteStmt->bindSms(sms);
            try
            {
                connection->check(overwriteStmt->execute());
                connection->commit();
            } 
            catch (StorageException& exc) 
            {
                connection->rollback();
                throw exc;
            }
            return id; 
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

    storeStmt->bindId(id=generator->getNextId());
    storeStmt->bindSms(sms);
    try 
    {
        connection->check(storeStmt->execute(OCI_DEFAULT));
        connection->commit();
    } 
    catch (StorageException& exc) 
    {
        connection->rollback();
        throw exc;
    }
    
    return id;
}
SMSId StoreManager::createSms(SMS& sms, const CreateMode flag)
    throw(StorageException, DuplicateMessageException)
{
    __require__(pool && generator);
    
    SMSId id=0;

    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            id = doCreateSms(connection, sms, flag);
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
    return id;
}

void StoreManager::doRetriveSms(Connection* connection, 
    SMSId id, SMS &sms)
        throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    RetriveStatement* retriveStmt 
        = connection->getRetriveStatement();

    retriveStmt->bindId(id);
    retriveStmt->defineSms(sms);
    sword status = retriveStmt->execute(OCI_DEFAULT);
    if ((status) == OCI_NO_DATA)
    {
        throw NoSuchMessageException(id);
    }
    else 
    {
        retriveStmt->check(status);
        retriveStmt->getSms(sms);
    }
}
void StoreManager::retriveSms(SMSId id, SMS &sms)
    throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            doRetriveSms(connection, id, sms);
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
}

void StoreManager::doDestroySms(Connection* connection, SMSId id) 
    throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

    DestroyStatement* destroyStmt
        = connection->getDestroyStatement();
    
    destroyStmt->bindId(id);
    try 
    {
        connection->check(destroyStmt->execute());
    }
    catch (StorageException& exc) 
    {
        connection->rollback();
        throw exc;
    }
    
    if (!destroyStmt->wasDestroyed()) 
    {
        connection->rollback();
        throw NoSuchMessageException(id);
    }
    connection->commit();
}
void StoreManager::destroySms(SMSId id) 
    throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
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
}

void StoreManager::doReplaceSms(Connection* connection, 
    SMSId id, const Address& oa, 
    const Body& newBody, uint8_t deliveryReport,
    time_t validTime, time_t waitTime)
        throw(StorageException, NoSuchMessageException)
{
    __require__(connection);

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
    replaceStmt->bindBody((Body&) newBody);
    replaceStmt->bindDeliveryReport((dvoid *) &deliveryReport,
                                    (sb4) sizeof(deliveryReport));
    try 
    {
        connection->check(replaceStmt->execute());
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
void StoreManager::replaceSms(SMSId id, const Address& oa,
    const Body& newBody, uint8_t deliveryReport,
    time_t validTime, time_t waitTime)
        throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            doReplaceSms(connection, id, oa, newBody,
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
} 

void StoreManager::doChangeSmsStateToEnroute(Connection* connection,
    SMSId id, const Descriptor& dst, uint8_t failureCause, time_t nextTryTime) 
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
void StoreManager::changeSmsStateToEnroute(SMSId id,
    const Descriptor& dst, uint8_t failureCause, time_t nextTryTime) 
        throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
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
}

void StoreManager::doChangeSmsStateToDelivered(Connection* connection, 
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
void StoreManager::changeSmsStateToDelivered(SMSId id, const Descriptor& dst) 
    throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            doChangeSmsStateToDelivered(connection, id, dst);
            archiver->incrementFinalizedCount();
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
}

void StoreManager::doChangeSmsStateToUndeliverable(Connection* connection, 
    SMSId id, const Descriptor& dst, uint8_t failureCause)
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
void StoreManager::changeSmsStateToUndeliverable(SMSId id, 
    const Descriptor& dst, uint8_t failureCause)
       throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            doChangeSmsStateToUndeliverable(connection, id,
                                            dst,failureCause);
            archiver->incrementFinalizedCount();
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
}

void StoreManager::doChangeSmsStateToExpired(Connection* connection, SMSId id)
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
void StoreManager::changeSmsStateToExpired(SMSId id) 
    throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            doChangeSmsStateToExpired(connection, id);
            archiver->incrementFinalizedCount();
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
}

void StoreManager::doChangeSmsStateToDeleted(Connection* connection, SMSId id) 
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
void StoreManager::changeSmsStateToDeleted(SMSId id) 
    throw(StorageException, NoSuchMessageException)
{
    __require__(pool);
    
    Connection* connection = 0L;
    unsigned iteration=1;
    while (true)
    {
        try 
        {
            connection = pool->getConnection();
            doChangeSmsStateToDeleted(connection, id);
            archiver->incrementFinalizedCount();
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
}

}}

