
#include "WSmeProcessor.h"

namespace smsc { namespace wsme 
{

/* ----------------------------- WSmeProcessor ----------------------------- */

WSmeProcessor::WSmeProcessor(ConfigView* config)
    throw(ConfigException, InitException)
        : WSmeAdmin(), log(Logger::getCategory("smsc.wsme.WSmeProcessor")),
            ds(0), visitorManager(0), langManager(0), adManager(0)
{
    try
    {
        init(config);

        visitorManager = new VisitorManager(*ds, config);
        langManager = new LangManager(*ds, config);
        adManager = new AdManager(*ds, config);
    } 
    catch (Exception& exc)
    {
        if (visitorManager) delete visitorManager; visitorManager = 0;
        if (langManager) delete langManager; langManager = 0;
        if (adManager) delete adManager; adManager = 0;
        if (ds) delete ds;

        throw;
    }
}

void WSmeProcessor::init(ConfigView* config)
    throw(ConfigException)
{
    ConfigView* dsConfig = config->getSubConfig("DataSource");
    char* dsIdentity = 0;
    try
    {
        dsIdentity = dsConfig->getString("type");
        try 
        {
            ds = DataSourceFactory::getDataSource(dsIdentity);
            if (ds) ds->init(dsConfig);
            else throw ConfigException("DataSource for '%s' identity "
                                       "wasn't registered !", dsIdentity);
        }
        catch (ConfigException& exc)
        {
            if (ds) delete ds; ds = 0;
            throw;
        }
        if (dsIdentity) delete dsIdentity;
    }
    catch (ConfigException& exc)
    {
        if (dsIdentity) delete dsIdentity;
        if (dsConfig) delete dsConfig;
        throw;
    }
    delete dsConfig;
}

WSmeProcessor::~WSmeProcessor()
{
    if (visitorManager) delete visitorManager;
    if (langManager) delete langManager;
    if (adManager) delete adManager;
    if (ds) delete ds;
}

bool WSmeProcessor::processNotification(const std::string msisdn, std::string& out)
    throw (ProcessException)
{
    __require__(visitorManager && langManager && adManager);

    if (visitorManager->isVisitor(msisdn))
    {
        std::string lang = langManager->getLangCode(msisdn);
        __trace2__("Lang is '%s'", lang.c_str());
        return adManager->getAd(msisdn, lang, out);
    }
    return false;
}
void WSmeProcessor::processResponce(const std::string msisdn, 
                                    const std::string msgid, bool responded)
    throw (ProcessException)
{
    __require__(adManager);
    __trace__("Processing responce message ...");
    adManager->respondAd(msisdn, msgid, responded);
    __trace__("Processed responce message.");
}

void WSmeProcessor::processReceipt(const std::string msgid, bool receipted)
    throw (ProcessException)
{
    __require__(adManager);
    __trace__("Processing receipt message ...");
    adManager->receiptAd(msgid, receipted);
    __trace__("Processed receipt message.");
}

void WSmeProcessor::addVisitor(const std::string msisdn)
    throw (ProcessException)
{
    __require__(visitorManager);
    visitorManager->addVisitor(msisdn);
}
void WSmeProcessor::removeVisitor(const std::string msisdn)
    throw (ProcessException)
{
    __require__(visitorManager);
    visitorManager->removeVisitor(msisdn);
}
void WSmeProcessor::addLang(const std::string mask, std::string lang)
    throw (ProcessException)
{
    __require__(langManager);
    langManager->addLang(mask, lang);
}
void WSmeProcessor::removeLang(const std::string mask)
    throw (ProcessException)
{
    __require__(langManager);
    langManager->removeLang(mask);
}
void WSmeProcessor::addAd(int id, const std::string lang, std::string ad)
    throw (ProcessException)
{
    __require__(adManager);
    adManager->addAd(id, lang, ad);
}
void WSmeProcessor::removeAd(int id, const std::string lang)
    throw (ProcessException)
{
    __require__(adManager);
    adManager->removeAd(id, lang);
}

/* ------------------------ Managers Implementation ------------------------ */

/* ------------------------ VisitorManager ------------------------ */
VisitorManager::VisitorManager(DataSource& _ds, ConfigView* config) 
    throw (InitException)
        : log(Logger::getCategory("smsc.wsme.WSmeVisitorManager")), ds(_ds) 
{
    loadUpVisitors();
}
VisitorManager::~VisitorManager()
{
}

const char* SQL_GET_VISITORS = "SELECT MASK FROM WSME_VISITORS";
void VisitorManager::loadUpVisitors()
    throw (InitException)
{
    MutexGuard  guard(visitorsLock);

    ResultSet* rs = 0;
    Statement* statement = 0; 
    Connection* connection = 0;
    
    try
    {
        connection = ds.getConnection();
        if (!connection)
            throw Exception("Get connection failed");
        
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(SQL_GET_VISITORS);
        if (!statement)
            throw Exception("Create statement failed");
        
        ResultSet* rs = statement->executeQuery();
        if (!rs)
            throw Exception("Get results failed");
        
        while (rs->fetchNext())
        {
            const char* mask = rs->getString(1);
            if (mask && !visitors.Exists(mask))
                visitors.Insert(mask, true);
        }
        
        if (rs) delete rs;
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (rs) delete rs;
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
        throw InitException(exc.what());
    }
}

bool VisitorManager::isVisitor(const std::string msisdn) 
    throw (ProcessException)
{ 
    MutexGuard  guard(visitorsLock);

    visitors.First();
    char* mask = 0; bool avail = false;
    while (visitors.Next(mask, avail))
    {
        if (mask && mask[0] != '\0')
        {
            __trace2__("Checking visitor '%s' by mask '%s'", 
                       msisdn.c_str(), mask);
            if (compareMaskAndAddress(mask, msisdn)) {
                __trace2__("Visitor '%s' conform with mask '%s'", 
                           msisdn.c_str(), mask);
                return avail;
            }
        }
    }
    return false;
}

const char* SQL_ADD_NEW_VISITOR = 
"INSERT INTO WSME_VISITORS (MASK) VALUES (:MASK)";
void VisitorManager::addVisitor(const std::string msisdn)
    throw (ProcessException)
{
    MutexGuard  guard(visitorsLock);

    const char* addr = msisdn.c_str();
    if (visitors.Exists(addr))
        throw ProcessException("Visitor '%s' already exists", addr);

    Statement* statement = 0; 
    Connection* connection = 0;
    try
    {
        connection = ds.getConnection();
        if (!connection) 
            throw Exception("Get connection failed");
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(SQL_ADD_NEW_VISITOR);
        if (!statement) 
            throw Exception("Create statement failed");
        
        statement->setString(1, addr);
        statement->executeUpdate();
        connection->commit();
        
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (statement) delete statement;
        if (connection) {
            try { connection->rollback(); } catch (Exception& eee) {
                log.error("Rollback failed, Cause: %s", eee.what());
            }
            ds.freeConnection(connection);
        }
        throw ProcessException(exc.what());
    }
    
    visitors.Insert(addr, true);
}

const char* SQL_REMOVE_VISITOR = 
"DELETE FROM WSME_VISITORS WHERE MASK=:MASK";
void VisitorManager::removeVisitor(const std::string msisdn)
    throw (ProcessException)
{
    MutexGuard  guard(visitorsLock);

    const char* addr = msisdn.c_str();
    if (!visitors.Exists(addr))
        throw ProcessException("Visitor '%s' not exists", addr);

    Statement* statement = 0; 
    Connection* connection = 0;
    try
    {
        connection = ds.getConnection();
        if (!connection) 
            throw Exception("Get connection failed");
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(SQL_REMOVE_VISITOR);
        if (!statement) 
            throw Exception("Create statement failed");
        
        statement->setString(1, addr);
        statement->executeUpdate();
        connection->commit();
        
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (statement) delete statement;
        if (connection) {
            try { connection->rollback(); } catch (Exception& eee) {
                log.error("Rollback failed, Cause: %s", eee.what());
            }
            ds.freeConnection(connection);
        }
        throw ProcessException(exc.what());
    }
    
    visitors.Delete(addr);
}


/* ------------------------ LangManager ------------------------ */

LangManager::LangManager(DataSource& _ds, ConfigView* config)
    throw (InitException)
        : log(Logger::getCategory("smsc.wsme.WSmeLangManager")), ds(_ds)
{
    ConfigView* langConfig = 0;
    try
    {
        langConfig = config->getSubConfig("LangManager");
        const char* lang = langConfig->getString("defaultLang");
        if (!lang || lang[0] == '\0')
            throw InitException("LangManager: defaultLang is not defined (empty)");
        defaultLang = lang;
        if (langConfig) delete langConfig;
    }
    catch (Exception& exc)
    {
        if (langConfig) delete langConfig;
        throw InitException(exc.what());
    }

    loadUpLangs();
}
LangManager::~LangManager()
{
    MutexGuard  guard(langsLock);
}

void convertStrToUpperCase(const char* str, char* upper)
{
    __require__(str && upper);
    
    do *upper++=toupper(*str);
    while (*str++); 
}

const char* SQL_GET_LANGS = "SELECT MASK, LANG FROM WSME_LANGS";
void LangManager::loadUpLangs()
    throw (InitException)
{
    MutexGuard  guard(langsLock);

    ResultSet* rs = 0;
    Statement* statement = 0; 
    Connection* connection = 0;
    
    try
    {
        connection = ds.getConnection();
        if (!connection)
            throw Exception("Get connection failed");
        
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(SQL_GET_LANGS);
        if (!statement)
            throw Exception("Create statement failed");
        
        ResultSet* rs = statement->executeQuery();
        if (!rs)
            throw Exception("Get results failed");
        
        while (rs->fetchNext())
        {
            const char* mask = (rs->isNull(1)) ? 0:rs->getString(1);
            const char* lang = (rs->isNull(2)) ? 0:rs->getString(2);
            
            if (!mask || mask[0]=='\0' || 
                !lang || lang[0]=='\0') continue;
            
            if (!langs.Exists(mask)) {
                std::string langStr = lang;
                langs.Insert(mask, langStr);
            }
        }
        
        if (rs) delete rs;
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (rs) delete rs;
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
        throw InitException(exc.what());
    }
}
std::string LangManager::getLangCode(const std::string msisdn)
    throw (ProcessException)
{
    MutexGuard  guard(langsLock);

    langs.First();
    char* mask = 0; std::string langStr;
    while (langs.Next(mask, langStr))
    {
        if (mask && mask[0] != '\0' && compareMaskAndAddress(mask, msisdn))
        {
            const char* lang = langStr.c_str();
            if (lang && lang[0] != '\0') return langStr;
            else break;
        }
    }
    
    return defaultLang;
}
std::string LangManager::getDefaultLang()
{
    MutexGuard guard(langsLock);
    std::string lang = defaultLang;
    return lang;
}

const char* SQL_ADD_NEW_LANG = 
"INSERT INTO WSME_LANGS (MASK, LANG) VALUES (:MASK, :LANG)";
void LangManager::addLang(const std::string mask, std::string lang)
    throw (ProcessException)
{
    MutexGuard  guard(langsLock);

    const char* addr = mask.c_str();
    if (langs.Exists(addr))
        throw ProcessException("Mask '%s' already defined", addr);
    
    Statement* statement = 0; 
    Connection* connection = 0;
    try
    {
        connection = ds.getConnection();
        if (!connection) 
            throw Exception("Get connection failed");
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(SQL_ADD_NEW_LANG);
        if (!statement) 
            throw Exception("Create statement failed");
        
        statement->setString(1, addr);
        statement->setString(2, lang.c_str());
        statement->executeUpdate();
        connection->commit();
        
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (statement) delete statement;
        if (connection) {
            try { connection->rollback(); } catch (Exception& eee) {
                log.error("Rollback failed, Cause: %s", eee.what());
            }
            ds.freeConnection(connection);
        }
        throw ProcessException(exc.what());
    }
    
    langs.Insert(addr, lang);
}

const char* SQL_REMOVE_LANG = 
"DELETE FROM WSME_LANGS WHERE MASK=:MASK";
void LangManager::removeLang(const std::string mask)
    throw (ProcessException)
{
    MutexGuard  guard(langsLock);

    const char* addr = mask.c_str();
    if (!langs.Exists(addr))
        throw ProcessException("Mask '%s' not defined", addr);
    
    Statement* statement = 0; 
    Connection* connection = 0;
    try
    {
        connection = ds.getConnection();
        if (!connection) 
            throw Exception("Get connection failed");
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(SQL_REMOVE_LANG);
        if (!statement) 
            throw Exception("Create statement failed");
        
        statement->setString(1, addr);
        statement->executeUpdate();
        connection->commit();
        
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (statement) delete statement;
        if (connection) {
            try { connection->rollback(); } catch (Exception& eee) {
                log.error("Rollback failed, Cause: %s", eee.what());
            }
            ds.freeConnection(connection);
        }
        throw ProcessException(exc.what());
    }
    
    langs.Delete(addr);
}

/* ------------------------ AdRepository ------------------------ */

AdRepository::AdRepository(DataSource& _ds, ConfigView* config)
    throw(ConfigException, InitException) 
        : log(Logger::getCategory("smsc.wsme.WSmeAdRepository")), 
            ds(_ds), minAdId(0), maxAdId(0)
{
    loadUpAds();
}
AdRepository::~AdRepository()
{
    MutexGuard  guard(adsLock);

    IntHash<Hash<std::string>*>::Iterator it = ads.First();
    int id; Hash<std::string>* ids = 0;
    while (it.Next(id, ids))
        if (ids) delete ids;
}

const char* SQL_LOAD_ADS = 
"SELECT ID, LANG, AD FROM WSME_AD ORDER BY ID ASC";
void AdRepository::loadUpAds()
    throw(InitException)
{
    MutexGuard  guard(adsLock);

    ResultSet* rs = 0;
    Statement* statement = 0; 
    Connection* connection = 0;

    try
    {
        connection = ds.getConnection();
        if (!connection)
            throw InitException("Get connection failed");
        if (!connection->isAvailable()) connection->connect();
        statement = connection->createStatement(SQL_LOAD_ADS);
        ResultSet* rs = statement->executeQuery();
        if (!rs) 
            throw InitException("Get results failed");

        while (rs->fetchNext())
        {
            uint32_t id = rs->getUint32(1);
            const char* lang = (rs->isNull(2)) ? 0:rs->getString(2);
            const char* ad   = (rs->isNull(3)) ? 0:rs->getString(3);

            Hash<std::string>* ids = 
                (ads.Exist(id)) ? ads.Get(id):new Hash<std::string>(0);
            std::string adStr = ad ? ad:"";
            if (ids && !ids->Exists(lang))
                ids->Insert(lang, adStr);
            if (!ads.Exist(id))
                ads.Insert(id, ids);

            if (id > maxAdId) maxAdId = id;
            if (id < minAdId) minAdId = id;
        }

        if (rs) delete rs;
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (rs) delete rs;
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
        InitException eee("Load ads failed, Cause: ", exc.what());
        log.error(eee.what());
        throw eee;
    }
}

bool AdRepository::getAd(int id, const std::string lang, std::string& ad)
    throw (ProcessException)
{
    MutexGuard  guard(adsLock);

    Hash<std::string>* ids = (ads.Exist(id)) ? ads.Get(id):0;
    if (!ids || !ids->Exists(lang.c_str())) return false;
    ad = ids->Get(lang.c_str());
    return true;
}
int AdRepository::getFirstId()
{
    return minAdId;
}
int AdRepository::getNextId(int id)
{
    return (id+1)%(maxAdId+1);
}

const char* SQL_ADD_NEW_AD = 
"INSERT INTO WSME_AD (ID, LANG, AD) VALUES (:ID, :LANG, :AD)";
void AdRepository::addAd(int id, const std::string lang, std::string ad)
    throw (ProcessException)
{
    MutexGuard  guard(adsLock);

    Hash<std::string>* newIds = ads.Exist(id) ? 0:new Hash<std::string>(0);
    Hash<std::string>* ids = (newIds) ? newIds:ads.Get(id);
    
    __require__(ids);

    const char* langStr = lang.c_str();
    const char* adStr = ad.c_str();
    if (ids->Exists(langStr)) {
        if (newIds) delete newIds;
        throw ProcessException("Ad with id='%d' and lang='%s' already defined", 
                               id, langStr);
    }
    
    Statement* statement = 0; 
    Connection* connection = 0;
    try
    {
        connection = ds.getConnection();
        if (!connection) 
            throw Exception("Get connection failed");
        if (!connection->isAvailable()) connection->connect();
        statement = connection->createStatement(SQL_ADD_NEW_AD);
        if (!statement) 
            throw Exception("Create statement failed");
        
        statement->setUint32(1, id);
        statement->setString(2, langStr);
        statement->setString(3, adStr);
        statement->executeUpdate();
        connection->commit();
        
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (newIds) delete newIds; newIds=0;
        if (statement) delete statement;
        if (connection) {
            try { connection->rollback(); } catch (Exception& eee) {
                log.error("Rollback failed, Cause: %s", eee.what());
            }
            ds.freeConnection(connection);
        }
        throw ProcessException("Failed to add Ad with id='%d' and lang='%s'"
                               ", Cause: %s", id, langStr, exc.what());
    }
    
    ids->Insert(langStr, ad);
    if (!ads.Exist(id)) ads.Insert(id, ids);
}

const char* SQL_REMOVE_AD = 
"DELETE FROM WSME_AD WHERE ID=:ID AND LANG=:LANG";
void AdRepository::removeAd(int id, const std::string lang)
    throw (ProcessException)
{
    MutexGuard  guard(adsLock);

    Hash<std::string>* ids = (ads.Exist(id)) ? ads.Get(id):0;
    if (!ids)
        throw ProcessException("Ad with id='%d' and lang='%s' not defined", 
                               id, lang.c_str());
    __require__(ids);

    const char* langStr = lang.c_str();
    Statement* statement = 0; 
    Connection* connection = 0;
    try
    {
        connection = ds.getConnection();
        if (!connection) 
            throw Exception("Get connection failed");
        if (!connection->isAvailable()) connection->connect();
        statement = connection->createStatement(SQL_REMOVE_AD);
        if (!statement) 
            throw Exception("Create statement failed");
        
        statement->setUint32(1, id);
        statement->setString(2, langStr);
        statement->executeUpdate();
        connection->commit();
        
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (statement) delete statement;
        if (connection) {
            try { connection->rollback(); } catch (Exception& eee) {
                log.error("Rollback failed, Cause: %s", eee.what());
            }
            ds.freeConnection(connection);
        }
        throw ProcessException("Failed to remove Ad with id='%d' and lang='%s'"
                               ", Cause: %s", id, langStr, exc.what());
    }

    ids->Delete(langStr);
    if (ids->GetCount() <= 0) {
        ads.Delete(id); delete ids;
    }
}

/* ------------------------ AdHistory ------------------------ */

AdHistory::AdHistory(DataSource& _ds, ConfigView* config, AdIdManager& idman)
    throw(ConfigException, InitException) 
        : Thread(), log(Logger::getCategory("smsc.wsme.WSmeAdHistory")), 
            ds(_ds), idManager(idman), bStarted(false)

{
    historyAge    = config->getInt("age")*3600*24; // in days
    cleanupPeriod = config->getInt("cleanup");     // in seconds
    messageLife   = config->getInt("messageLife"); // in seconds

    Start();
}
AdHistory::~AdHistory()
{
    Stop();
}
void AdHistory::Start()
{
    MutexGuard  guard(startLock);
    
    if (!bStarted)
    {
        Thread::Start();
        bStarted = true;
    }
}
void AdHistory::Stop()
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        exit.Signal();
        job.Signal();
        exited.Wait();
        bStarted = false;
    }
}

int AdHistory::Execute()
{
    while (!exit.isSignaled())
    {
        job.Wait(cleanupPeriod*1000);
        if (exit.isSignaled()) break;

        try 
        {
            job.Signal(); cleanup(); job.Wait(0);
        } 
        catch (ProcessException& exc) 
        {
            log.error("Exception occurred during history cleanup : %s",
                      exc.what());
        }
    } 
    
    exit.Wait(0);
    exited.Signal();
    return 0;
}

const char* SQL_DELETE_HISTORY_INFO =
"DELETE FROM WSME_HISTORY WHERE LAST_UPDATE<:CT";
void AdHistory::cleanup()
    throw (ProcessException)
{
    Statement* statement = 0; 
    Connection* connection = 0;

    try
    {
        connection = ds.getConnection();
        if (!connection)
            throw ProcessException("Get connection failed");
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(SQL_DELETE_HISTORY_INFO);
        statement->setDateTime(1, time(NULL)-historyAge);
        statement->executeUpdate();
        connection->commit();
        
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (statement) delete statement;
        if (connection) {
            try {
                connection->rollback();
            } catch (Exception& eee) {
                log.error("Rollback failed, Cause: %s", eee.what());
            }
            ds.freeConnection(connection);
        }
        log.error(exc.what());
        throw ProcessException(exc.what());
    }
}

const char* SQL_SELECT_HISTORY_INFO =
"SELECT ID, ST, LAST_UPDATE FROM WSME_HISTORY WHERE MSISDN=:MSISDN FOR UPDATE";
const char* SQL_INSERT_HISTORY_INFO =
"INSERT INTO WSME_HISTORY (MSISDN, ST, ID, MSG_ID, LAST_UPDATE)\
 VALUES (:MSISDN, 0, :ID, NULL, :CT)";
const char* SQL_UPDATE_NOTIFY_HISTORY_INFO =
"UPDATE WSME_HISTORY SET ST=0, ID=:ID, MSG_ID=NULL, LAST_UPDATE=:CT\
 WHERE MSISDN=:MSISDN";
bool AdHistory::getId(const std::string msisdn, int& id)
    throw (ProcessException)
{
    bool result = false;

    ResultSet* rs = 0;
    Statement* selectStmt  = 0; 
    Statement* insertStmt  = 0; 
    Statement* updateStmt = 0; 
    Connection* connection = 0;

    __trace2__("AdHistory::getId called for '%s'.", msisdn.c_str());

    try
    {
        connection = ds.getConnection();
        if (!connection)
            throw ProcessException("Get connection failed");
        if (!connection->isAvailable()) connection->connect();

        selectStmt = connection->createStatement(SQL_SELECT_HISTORY_INFO);
        selectStmt->setString(1, msisdn.c_str());
        
        ResultSet* rs = selectStmt->executeQuery();
        if (!rs || !rs->fetchNext()) 
        {
            id = idManager.getFirstId();
            __trace2__("History for '%s' not found, inserting id=%d",
                       msisdn.c_str(), id);
            insertStmt = connection->createStatement(SQL_INSERT_HISTORY_INFO);
            insertStmt->setString  (1, msisdn.c_str());
            insertStmt->setInt32   (2, id);
            insertStmt->setDateTime(3, time(NULL));
            insertStmt->executeUpdate();
            delete insertStmt; insertStmt = 0;
            result = true;
            __trace__("Inserted");
        }
        else
        {
            __trace2__("History for '%s' found", msisdn.c_str());
            uint32_t last_id = rs->getInt32(1);
            uint8_t  st = rs->getUint8(2);
            time_t interval = time(NULL) - rs->getDateTime(3);
            
            if (st == 0 && (interval <= messageLife)) {
                __trace2__("Life period is not expired, interval=%d !", 
                           interval);
                result = false;
            } else {
                id = idManager.getNextId(last_id);
                __trace2__("Last id=%d, newid=%d", last_id, id);
                updateStmt = connection->createStatement(SQL_UPDATE_NOTIFY_HISTORY_INFO);
                updateStmt->setInt32   (1, id);
                updateStmt->setDateTime(2, time(NULL));
                updateStmt->setString  (3, msisdn.c_str());
                updateStmt->executeUpdate();
                delete updateStmt; updateStmt = 0;
                result = true;
                __trace2__("Updated");
            }
        }
        
        connection->commit();

        if (rs) delete rs;
        if (selectStmt) delete selectStmt;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        log.error(exc.what());
        if (rs) delete rs;
        if (selectStmt) delete selectStmt;
        if (insertStmt) delete insertStmt;
        if (updateStmt) delete updateStmt;
        if (connection) {
            try {
                connection->rollback();
            } catch (Exception& eee) {
                log.error("Rollback failed, Cause: %s", eee.what());
            }
            ds.freeConnection(connection);
        }
        throw ProcessException(exc.what());
    }
    return result;
}

const char* SQL_UPDATE_RESPONCE_HISTORY_INFO =
"UPDATE WSME_HISTORY SET MSG_ID=:MSG_ID, LAST_UPDATE=:CT\
 WHERE MSISDN=:MSISDN AND ST=0";
void AdHistory::respondAd(const std::string msisdn, 
                          const std::string msgid, bool responded)
    throw (ProcessException)
{
    Statement* statement = 0; 
    Connection* connection = 0;

    try
    {
        connection = ds.getConnection();
        if (!connection)
            throw ProcessException("Get connection failed");
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(SQL_UPDATE_RESPONCE_HISTORY_INFO);
        statement->setString  (1, msgid.c_str());
        statement->setDateTime(2, time(NULL));
        statement->setString  (3, msisdn.c_str());
        statement->executeUpdate();
        connection->commit();
        
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (statement) delete statement;
        if (connection) {
            try {
                connection->rollback();
            } catch (Exception& eee) {
                log.error("Rollback failed, Cause: %s", eee.what());
            }
            ds.freeConnection(connection);
        }
        log.error(exc.what());
        throw ProcessException(exc.what());
    }
}

const char* SQL_UPDATE_RECEIPT_HISTORY_INFO =
"UPDATE WSME_HISTORY SET ST=10, LAST_UPDATE=:CT\
 WHERE MSG_ID=:MSG_ID AND ST=0";
void AdHistory::receiptAd(const std::string msgid, bool receipted) 
    throw (ProcessException)
{
    Statement* statement = 0; 
    Connection* connection = 0;

    try
    {
        connection = ds.getConnection();
        if (!connection)
            throw ProcessException("Get connection failed");
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(SQL_UPDATE_RECEIPT_HISTORY_INFO);
        statement->setDateTime(1, time(NULL));
        statement->setString  (2, msgid.c_str());
        statement->executeUpdate();
        connection->commit();
        
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (statement) delete statement;
        if (connection) {
            try {
                connection->rollback();
            } catch (Exception& eee) {
                log.error("Rollback failed, Cause: %s", eee.what());
            }
            ds.freeConnection(connection);
        }
        log.error(exc.what());
        throw ProcessException(exc.what());
    }
}

/* ------------------------ AdManager ------------------------ */

AdManager::AdManager(DataSource& _ds, ConfigView* config)
    throw(ConfigException, InitException) 
        : log(Logger::getCategory("smsc.wsme.WSmeAdManager")), ds(_ds),
            history(0), repository(0)
{
    MutexGuard  guard(initLock);

    ConfigView* adConfig = 0;
    ConfigView* historyConfig = 0;
    ConfigView* repositoryConfig = 0;

    try
    {
        adConfig = config->getSubConfig("AdManager");
        historyConfig = adConfig->getSubConfig("History");
        repositoryConfig = adConfig->getSubConfig("Repository");
        
        repository = new AdRepository(ds, repositoryConfig);
        history = new AdHistory(ds, historyConfig, *repository);
    } 
    catch (Exception& exc) 
    {
        if (history) delete history;
        if (repository) delete repository;
        
        if (repositoryConfig) delete repositoryConfig;
        if (historyConfig) delete historyConfig;
        if (adConfig) delete adConfig;
        
        throw;
    }
    
    if (repositoryConfig) delete repositoryConfig;
    if (historyConfig) delete historyConfig;
    if (adConfig) delete adConfig;
}
AdManager::~AdManager() 
{
    MutexGuard  guard(initLock);

    if (history) delete history;
    if (repository) delete repository;
}

bool AdManager::getAd(const std::string msisdn, const std::string lang,
                      std::string& ad)
    throw (ProcessException)
{
    __require__(history && repository);
    
    __trace__("AdManager::getAd called");
    int id = 0;
    if (!(history->getId(msisdn, id))) return false;
    return repository->getAd(id, lang, ad);
}

void AdManager::respondAd(const std::string msisdn, 
                          const std::string msgid, bool responded)
    throw (ProcessException)
{
    __require__(history);
    history->respondAd(msisdn, msgid, responded);
}
void AdManager::receiptAd(const std::string msgid, bool receipted) 
    throw (ProcessException)
{
    __require__(history);
    history->receiptAd(msgid, receipted);
}

void AdManager::addAd(int id, const std::string lang, std::string ad)
    throw (ProcessException)
{
    __require__(repository);
    repository->addAd(id, lang, ad);
}
void AdManager::removeAd(int id, const std::string lang)
    throw (ProcessException)
{
    __require__(repository);
    repository->removeAd(id, lang);
}


}}


