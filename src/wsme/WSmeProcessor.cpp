
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
        return adManager->getAd(msisdn, lang, out);
    }
    return false;
}
void WSmeProcessor::processResponce(const std::string msisdn, const std::string msgid)
    throw (ProcessException)
{
    __require__(adManager);

    // TODO : update adManager::history.
}

void WSmeProcessor::processReceipt(const std::string msgid)
    throw (ProcessException)
{
    __require__(adManager);

    /* TODO: implement it
        
        1) check whether receipt is valid (???)
        2) get msg_id from receipt
        3) notify adManager to update history by msg_id
    */

    adManager->reportAd(msgid);
}


/* ------------------------ Managers Implementation ------------------------ */


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

bool compareMaskAndAddress(const std::string mask, 
                           const std::string addr)
{
    // TODO: implement check
    return true;
}

bool VisitorManager::isVisitor(const std::string msisdn) 
    throw (ProcessException)
{ 
    MutexGuard  guard(visitorsLock);

    for (int i=0; i<visitors.Count(); i++)
        if (compareMaskAndAddress(visitors[i], msisdn)) 
            return true;

    return false;
}

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

AdRepository::AdRepository(DataSource& _ds, ConfigView* config)
    throw(ConfigException, InitException) 
        : log(Logger::getCategory("smsc.wsme.AdRepository")), ds(_ds)
{
}
AdRepository::~AdRepository()
{
}

const char* SQL_GET_AD_BY_ID_LANG =
"SELECT AD FROM WSME_AD WHERE ID=:ID AND (LANG=:LANG OR LANG IS NULL)";
bool AdRepository::getAd(int id, const std::string lang, std::string& ad)
    throw (ProcessException)
{
    ResultSet* rs = 0;
    Statement* statement = 0; 
    Connection* connection = 0;

    try
    {
        connection = ds.getConnection();
        if (!connection)
            throw ProcessException("Get connection failed");
        if (!connection->isAvailable()) connection->connect();

        statement = connection->createStatement(SQL_GET_AD_BY_ID_LANG);
        if (!statement)
            throw ProcessException("Create statement failed");
        statement->setInt32 (1, id);
        statement->setString(2, lang.c_str());    

        ResultSet* rs = statement->executeQuery();
        if (!rs)
            throw ProcessException("Get results failed");
        
        if (!rs->fetchNext()) return false;
        ad = rs->getString(1);
        
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

AdHistory::AdHistory(DataSource& _ds, ConfigView* config)
    throw(ConfigException, InitException) 
        : log(Logger::getCategory("smsc.wsme.AdHistory")), 
            ds(_ds), historyPeriod(0)

{
    historyPeriod = config->getInt("period");
}
AdHistory::~AdHistory()
{

}

int AdHistory::getNextId(const std::string msisdn)
    throw (ProcessException)
{
    // TODO: implement it
    return 0;
}

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
        
        history = new AdHistory(ds, historyConfig);
        repository = new AdRepository(ds, repositoryConfig);
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
    
    int id = history->getNextId(msisdn);
    return repository->getAd(id, lang, ad);
}
void AdManager::reportAd(const std::string msgid) 
    throw (ProcessException)
{
    __require__(history);

    // TODO : implement it
}


}}


