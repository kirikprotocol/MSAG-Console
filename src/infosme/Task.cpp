
#include "Task.h"
#include "SQLAdapters.h"

namespace smsc { namespace infosme 
{

time_t parseDateTime(const char* str)
{
    int year, month, day, hour, minute, second;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d.%02d.%4d %02d:%02d:%02d", 
                    &day, &month, &year, &hour, &minute, &second) != 6) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = hour; dt.tm_min = minute; dt.tm_sec = second;

    //printf("%02d.%02d.%04d %02d:%02d:%02d = %ld\n", day, month, year, hour, minute, second, time);
    return mktime(&dt);
}
time_t parseDate(const char* str)
{
    int year, month, day;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d.%02d.%4d", 
                    &day, &month, &year) != 3) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = 0; dt.tm_min = 0; dt.tm_sec = 0;
    
    //printf("%02d:%02d:%04d = %ld\n", day, month, year, time);
    return mktime(&dt);
}
int parseTime(const char* str)
{
    int hour, minute, second;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d:%02d:%02d", 
                    &hour, &minute, &second) != 3) return -1;
    
    //printf("%02d:%02d:%02d = %ld\n", hour, minute, second, time);
    return hour*3600+minute*60+second;
}

void Task::init(ConfigView* config, std::string taskId, std::string tablePrefix)
{
    __require__(config);

    const int MAX_PRIORITY_VALUE = 1000;

    info.id = taskId;
    try { info.name = config->getString("name"); } catch (...) {}
    info.tablePrefix = tablePrefix;
    info.enabled = config->getBool("enabled");
    info.priority = config->getInt("priority");
    if (info.priority <= 0 || info.priority > MAX_PRIORITY_VALUE)
        throw ConfigException("Task priority should be positive and less than %d.", 
                              MAX_PRIORITY_VALUE);
    info.retryOnFail = config->getBool("retryOnFail");
    info.replaceIfPresent = config->getBool("replaceMessage");
    info.endDate = parseDateTime(config->getString("endDate"));
    info.retryTime = parseTime(config->getString("retryTime"));
    if (info.retryOnFail && info.retryTime <= 0)
        throw ConfigException("Task retry time specified incorrectly."); 
    info.validityPeriod = parseTime(config->getString("validityPeriod"));
    info.validityDate = parseDateTime(config->getString("validityDate"));
    if (info.validityPeriod <= 0 && info.validityDate <= 0)
        throw ConfigException("Message validity period/date specified incorrectly.");
    info.activePeriodStart = parseTime(config->getString("activePeriodStart"));
    info.activePeriodEnd = parseTime(config->getString("activePeriodEnd"));
    if ((info.activePeriodStart < 0 && info.activePeriodEnd >= 0) ||
        (info.activePeriodStart >= 0 && info.activePeriodEnd < 0) ||
        (info.activePeriodStart >= 0 && info.activePeriodEnd >= 0 && 
         info.activePeriodStart >= info.activePeriodEnd))
        throw ConfigException("Task active period specified incorrectly."); 
    const char* query_sql = config->getString("query");
    if (!query_sql || query_sql[0] == '\0')
        throw ConfigException("Sql query for task empty or wasn't specified.");
    info.querySql = query_sql;
    const char* msg_template = config->getString("template");
    if (!msg_template || msg_template[0] == '\0')
        throw ConfigException("Message template for task empty or wasn't specified.");
    info.msgTemplate = msg_template;
    const char* svc_type = config->getString("svcType");
    if (info.replaceIfPresent && (!svc_type || svc_type[0] == '\0'))
        throw ConfigException("Service type task empty or wasn't specified.");
    info.svcType = svc_type;
    dsOwnTimeout = 0;
    try { dsOwnTimeout = config->getInt("dsOwnTimeout"); } catch(...) {}
    if (dsOwnTimeout < 0) dsOwnTimeout = 0;
    dsIntTimeout = 0;
    try { dsIntTimeout = config->getInt("dsIntTimeout"); } catch(...) {}
    if (dsIntTimeout < 0) dsIntTimeout = 0;
    dsUncommited = 1;
    try { dsUncommited = config->getInt("uncommited"); } catch(...) {}
    if (dsUncommited < 0) dsUncommited = 1;
}

bool Task::isInProcess()
{
    MutexGuard guard(inProcessLock);
    return bInProcess;
}

Statement* Task::getStatement(Connection* connection, const char* id, const char* sql)
{
    if (!connection || !id) return 0;

    MutexGuard guard(statementLock);
    
    Statement* statement = connection->getStatement(id);
    if (!statement)
    {
        if (!sql) return 0;
        statement = connection->createStatement(sql);
        connection->registerStatement(id, statement);
    }
    return statement;
}

const char* USER_QUERY_STATEMENT_ID = "USER_QUERY_STATEMENT_ID";
const char* NEW_MESSAGE_STATEMENT_ID = "NEW_MESSAGE_STATEMENT_ID";

const char* NEW_MESSAGE_STATEMENT_SQL = (const char*)
"INSERT INTO %s (ABONENT, MESSAGE) "
"VALUES (:ABONENT, :MESSAGE)";

const char* NEW_TABLE_STATEMENT_SQL = (const char*)
"CREATE TABLE %s ("
"VARCHAR2(30)   ABONENT,"
"VARCHAR2       MESSAGE"
")";
void Task::createTable(Connection* connection)
{
    MutexGuard guard(createTableLock);
    
    // TODO: пытаться создавать только один раз при старте !!!
    if (!bTableCreated && connection)
    {
        Statement* statement = 0;
        try
        {
            char createTableSql[2048];
            sprintf(createTableSql, NEW_TABLE_STATEMENT_SQL, (info.tablePrefix+info.id).c_str());
            statement = connection->createStatement(createTableSql);
            if (!statement) 
                throw Exception("Failed to create statement.");
            statement->execute();
            connection->commit();
            bTableCreated = true;
        } 
        catch (Exception& exc) {
            logger.error("Task '%s'. Failed to create internal table. "
                         "Details: %s", info.id.c_str(), exc.what());
        }
        catch (...) {
            logger.error("Task '%s'. Failed to create internal table.",
                         info.id.c_str());
        }
        if (statement) delete statement;
    }
}
void Task::beginProcess()
{
    {
        MutexGuard guard(inProcessLock);
        if (bInProcess) return;
        else bInProcess = true;
    }

    Connection* ownConnection = 0;
    Connection* intConnection = 0;
    int wdOwnTimerId = -1;
    int wdIntTimerId = -1;

    try
    {
        intConnection = dsInt->getConnection();
        if (!intConnection)
            throw Exception("Failed to obtain connection to internal data source.");
        ownConnection = dsOwn->getConnection();
        if (!ownConnection)
            throw Exception("Failed to obtain connection to own data source.");
        
        createTable(intConnection);
        Statement* userQuery = getStatement(ownConnection, USER_QUERY_STATEMENT_ID, 
                                            info.querySql.c_str());
        if (!userQuery)
            throw Exception("Failed to create user query statement on own data source.");
        
        char newMessageSql[2048];
        sprintf(newMessageSql, NEW_MESSAGE_STATEMENT_SQL, (info.tablePrefix+info.id).c_str());
        Statement* newMessage = getStatement(intConnection, NEW_MESSAGE_STATEMENT_ID, 
                                             newMessageSql);
        if (!newMessage)
            throw Exception("Failed to create statement for message generation.");
            
        wdOwnTimerId = dsOwn->startTimer(ownConnection, dsOwnTimeout);
        std::auto_ptr<ResultSet> rsGuard(userQuery->executeQuery());
        ResultSet* rs = rsGuard.get();
        dsOwn->stopTimer(wdOwnTimerId);
        
        SQLGetAdapter       getAdapter(rs);
        ContextEnvironment  context;

        int uncommitted = 0;
        while (bInProcess)
        {
            wdOwnTimerId = dsOwn->startTimer(ownConnection, dsOwnTimeout);
            bool fetched = rs->fetchNext();
            dsOwn->stopTimer(wdOwnTimerId);
            if (!fetched) break;

            const char* abonentAddress = rs->getString(1);
            // TODO: check abonent address here !!! continue if invalid
            if (!abonentAddress || abonentAddress[0] == '\0') {
                logger.warn("Invalid abonent number selected.");
                continue;
            }
            
            std::string message = "";
            formatter->format(message, getAdapter, context);

            newMessage->setString(1, abonentAddress);
            newMessage->setString(2, message.c_str());
            newMessage->execute();

            if (++uncommitted >= dsUncommited) {
                intConnection->commit();
                uncommitted = 0;
            }
            inProcessEvent.Wait(10);
        }
        if (uncommitted > 0) intConnection->commit();
    }
    catch (Exception& exc)
    {
        try { if (intConnection) intConnection->rollback(); }
        catch (Exception& exc) {
            logger.error("Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            logger.error("Failed to roolback transaction on internal data source.");
        }
        logger.error("Task '%s'. Messages generation process failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...)
    {
        try { if (intConnection) intConnection->rollback(); }
        catch (Exception& exc) {
            logger.error("Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            logger.error("Failed to roolback transaction on internal data source.");
        }
        logger.error("Task '%s'. Messages generation process failure.",
                     info.id.c_str());
    }
    
    if (wdOwnTimerId >= 0) dsOwn->stopTimer(wdOwnTimerId);
    if (wdIntTimerId >= 0) dsInt->stopTimer(wdIntTimerId);
    if (ownConnection) dsOwn->freeConnection(ownConnection);
    if (intConnection) dsInt->freeConnection(intConnection);
    
    processEndEvent.Signal();
}
void Task::endProcess()
{
    MutexGuard guard(inProcessLock);
    if (!bInProcess) return;
    
    bInProcess = false;
    inProcessEvent.Signal();
    processEndEvent.Wait();
}
void Task::doNotifyMessage(StateInfo& info)
{
}
void Task::dropAllMessages()
{
}
bool Task::getNextMessage(Connection* connection, Message& message)
{
    return false;
}

}}
