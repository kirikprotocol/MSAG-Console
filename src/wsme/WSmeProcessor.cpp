
#include "WSmeProcessor.h"

namespace smsc { namespace wsme 
{

/* ----------------------------- WSmeProcessor ----------------------------- */

WSmeProcessor::WSmeProcessor(ConfigView* config)
    throw(ConfigException, InitException)
        : log(Logger::getCategory("smsc.wsme.WSmeProcessor")),
            ds(0), visitorManager(0), langManager(0), adManager(0)
{
    try
    {
        init(config);

        visitorManager = new VisitorManager(*ds);
        langManager = new LangManager(*ds);
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
        std::string lang;
        bool isLang = langManager->getLangCode(msisdn, lang);
        __trace2__("Lang is '%s'", isLang ? lang.c_str():"DEFAULT");
        return adManager->getAd(msisdn, lang, isLang, out);
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


/* ------------------------ Managers Implementation ------------------------ */

/* ------------------------ VisitorManager ------------------------ */
VisitorManager::VisitorManager(DataSource& _ds) 
    throw (InitException)
        : ds(_ds) 
{
    loadUpVisitors();
}
VisitorManager::~VisitorManager()
{
    MutexGuard  guard(visitorsLock);
    visitors.Clean();
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
            visitors.Push(rs->getString(1));
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

    for (int i=0; i<visitors.Count(); i++) {
        __trace2__("Checking visitor '%s' by mask '%s'", 
                   msisdn.c_str(), visitors[i].c_str());
        if (compareMaskAndAddress(visitors[i], msisdn)) {
            __trace2__("Visitor '%s' conform with mask '%s'", 
                       msisdn.c_str(), visitors[i].c_str());
            return true;
        }
    }

    return false;
}

/* ------------------------ LangManager ------------------------ */

LangManager::LangManager(DataSource& _ds)
    throw (InitException)
        : ds(_ds)
{
    loadUpLangs();
}
LangManager::~LangManager()
{
    MutexGuard  guard(langsLock);
    langs.Clean();
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
            LangInfo info(rs->getString(1), rs->getString(2));
            langs.Push(info);
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
bool LangManager::getLangCode(const std::string msisdn, std::string& lang)
    throw (ProcessException)
{
    MutexGuard  guard(langsLock);

    for (int i=0; i<langs.Count(); i++) {
        if (compareMaskAndAddress(langs[i].mask, msisdn)) {
            lang = langs[i].lang;
            return true;
        }
    }
    
    lang = "";
    return false;
}

/* ------------------------ AdRepository ------------------------ */

AdRepository::AdRepository(DataSource& _ds, ConfigView* config)
    throw(ConfigException, InitException) 
        : log(Logger::getCategory("smsc.wsme.AdRepository")), ds(_ds) 
{
    loadMaxAdId();
}
AdRepository::~AdRepository()
{
}

const char* SQL_MAX_AD_ID = "SELECT NVL(MAX(ID), 0) FROM WSME_AD";
void AdRepository::loadMaxAdId()
    throw(InitException)
{
    ResultSet* rs = 0;
    Statement* statement = 0; 
    Connection* connection = 0;

    try
    {
        connection = ds.getConnection();
        if (!connection)
            throw InitException("Get connection failed");
        if (!connection->isAvailable()) connection->connect();
        statement = connection->createStatement(SQL_MAX_AD_ID);
        ResultSet* rs = statement->executeQuery();
        if (!rs || !rs->fetchNext())
            throw InitException("Get results failed");
        maxAdId = rs->getInt32(1);
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

const char* SQL_GET_AD_BY_ID =
"SELECT AD FROM WSME_AD WHERE ID=:ID AND LANG IS NULL";
const char* SQL_GET_AD_BY_ID_LANG =
"SELECT AD FROM WSME_AD WHERE ID=:ID AND LANG=:LANG";
bool AdRepository::getAd(int id, const std::string lang, bool isLang, 
                         std::string& ad)
    throw (ProcessException)
{
    ResultSet* rs = 0;
    Statement* statement = 0; 
    Connection* connection = 0;

    __trace2__("AdRepository::getAd called, id=%d", id);

    try
    {
        connection = ds.getConnection();
        if (!connection)
            throw ProcessException("Get connection failed");
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(
            (isLang) ? SQL_GET_AD_BY_ID_LANG : SQL_GET_AD_BY_ID);
        if (!statement)
            throw ProcessException("Create statement failed");
        statement->setInt32 (1, id);
        if (isLang) statement->setString(2, lang.c_str());    

        ResultSet* rs = statement->executeQuery();
        if (!rs)
            throw ProcessException("Get results failed");
        
        if (!rs->fetchNext()) return false;
        ad = rs->getString(1);
        __trace2__("Got ad: '%s'", ad.c_str());
        
        if (rs) delete rs;
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
    }
    catch (Exception& exc)
    {
        if (rs) delete rs;
        if (statement) delete statement;
        if (connection) ds.freeConnection(connection);
        log.error(exc.what());
        throw ProcessException(exc.what());
    }
    return true;
}

int AdRepository::getFirstId()
{
    return 0;
}
int AdRepository::getNextId(int id)
{
    return (id+1)%(maxAdId+1);
}

/* ------------------------ AdHistory ------------------------ */

AdHistory::AdHistory(DataSource& _ds, ConfigView* config, AdIdManager& idman)
    throw(ConfigException, InitException) 
        : log(Logger::getCategory("smsc.wsme.AdHistory")), 
            ds(_ds), idManager(idman)

{
    keepPeriod = config->getInt("keepPeriod");
    lifePeriod = config->getInt("lifePeriod");
}
AdHistory::~AdHistory()
{

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
            
            if (st == 0 && (interval <= lifePeriod)) {
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
        : log(Logger::getCategory("smsc.wsme.AdManager")), ds(_ds),
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

bool AdManager::getAd(const std::string msisdn, 
                      const std::string lang, bool isLang, 
                      std::string& ad)
    throw (ProcessException)
{
    __require__(history && repository);
    
    __trace__("AdManager::getAd called");
    int id = 0;
    if (!(history->getId(msisdn, id))) return false;
    return repository->getAd(id, lang, isLang, ad);
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


}}


