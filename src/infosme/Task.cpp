
#include "Task.h"
#include "SQLAdapters.h"

extern bool isMSISDNAddress(const char* string);

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

const char* USER_QUERY_STATEMENT_ID           = "%s_USER_QUERY_STATEMENT_ID";   // Own

const char* DELETE_GENERATING_STATEMENT_ID    = "DELETE_GENERATING_STATEMENT_ID";
const char* INSERT_GENERATING_STATEMENT_ID    = "INSERT_GENERATING_STATEMENT_ID"; 
const char* DELETE_NEW_MESSAGES_STATEMENT_ID  = "%s_DELETE_NEW_MESSAGES_STATEMENT_ID";
const char* SELECT_MESSAGES_STATEMENT_ID      = "%s_SELECT_MESSAGES_STATEMENT_ID";
const char* DO_WAIT_MESSAGE_STATEMENT_ID      = "%s_DO_WAIT_MESSAGE_STATEMENT_ID";
const char* NEW_MESSAGE_STATEMENT_ID          = "%s_NEW_MESSAGE_STATEMENT_ID";
const char* CLEAR_MESSAGES_STATEMENT_ID       = "%s_CLEAR_MESSAGES_STATEMENT_ID";
const char* DELETE_MESSAGES_STATEMENT_ID      = "%s_DELETE_MESSAGES_STATEMENT_ID";
const char* RESET_MESSAGES_STATEMENT_ID       = "%s_RESET_MESSAGES_STATEMENT_ID";
const char* DO_RETRY_MESSAGE_STATEMENT_ID     = "%s_DO_RETRY_MESSAGE_STATEMENT_ID";
const char* DO_DELETE_MESSAGE_STATEMENT_ID    = "%s_DO_DELETE_MESSAGE_STATEMENT_ID";
const char* DO_ENROUTE_MESSAGE_STATEMENT_ID   = "%s_DO_ENROUTE_MESSAGE_STATEMENT_ID";

const char* DELETE_GENERATING_STATEMENT_SQL   = "DELETE FROM INFOSME_GENERATING_TASKS WHERE TASK_ID=:TASK_ID";
const char* INSERT_GENERATING_STATEMENT_SQL   = "INSERT INTO INFOSME_GENERATING_TASKS TASK_ID VALUES(:TASK_ID)";
const char* DELETE_NEW_MESSAGES_STATEMENT_SQL = "DELETE FROM %s WHERE STATE=:NEW";
const char* DO_WAIT_MESSAGE_STATEMENT_SQL     = "UPDATE %s SET STATE=:WAIT WHERE ID=:ID";
const char* CLEAR_MESSAGES_STATEMENT_SQL      = "DELETE FROM %s WHERE STATE=:NEW AND ABONENT=:ABONENT";
const char* NEW_SD_INDEX_STATEMENT_SQL        = "CREATE INDEX %s_SD_IDX ON %s (STATE, SEND_DATE)";
const char* DELETE_MESSAGES_STATEMENT_SQL     = "DELETE FROM %s WHERE STATE=:NEW";
const char* RESET_MESSAGES_STATEMENT_SQL      = "UPDATE %s SET STATE=:NEW WHERE STATE=:WAIT";
const char* DO_RETRY_MESSAGE_STATEMENT_SQL    = "UPDATE %s SET STATE=:NEW, SEND_DATE=:SEND_DATE WHERE ID=:ID";
const char* DO_DELETE_MESSAGE_STATEMENT_SQL   = "DELETE FROM %s WHERE ID=:ID";
const char* DO_ENROUTE_MESSAGE_STATEMENT_SQL  = "UPDATE %s SET STATE=:ENROUTE WHERE ID=:ID AND STATE=:WAIT";
const char* DROP_TABLE_STATEMENT_SQL          = "DROP TABLE %s";
const char* NEW_TABLE_STATEMENT_SQL           = "CREATE TABLE %s (\n"
                                                "ID             NUMBER          NOT NULL,\n"
                                                "STATE          NUMBER(3)       NOT NULL,\n"
                                                "ABONENT        VARCHAR2(30)    NOT NULL,\n"
                                                "SEND_DATE      DATE            NOT NULL,\n"
                                                "MESSAGE        VARCHAR2(2000)  NULL,    \n"
                                                "PRIMARY KEY    (ID)                     \n"
                                                ")";
const char* NEW_MESSAGE_STATEMENT_SQL         = "INSERT INTO %s (ID, STATE, ABONENT, SEND_DATE, MESSAGE) "
                                                "VALUES (INFOSME_MESSAGES_SEQ.NEXTVAL, :STATE, :ABONENT, "
                                                ":SEND_DATE, :MESSAGE)";
const char* SELECT_MESSAGES_STATEMENT_SQL     = "SELECT ID, ABONENT, MESSAGE FROM %s WHERE "
                                                "STATE=:STATE AND SEND_DATE<=:SEND_DATE ORDER BY SEND_DATE ASC";


Task::Task(TaskInfo& _info, DataSource* _dsOwn, DataSource* _dsInt) 
    : logger(Logger::getInstance("smsc.infosme.Task")), formatter(0),
        usersCount(0), bFinalizing(false), bSelectedAll(false), dsOwn(dsOwn), dsInt(dsInt), 
            bInProcess(false), bInGeneration(false), bGenerationSuccess(true),
                lastMessagesCacheEmpty(0), currentPriorityFrameCounter(0)
{
    __require__(_dsOwn && _dsInt);
    this->info = _info; this->dsOwn = _dsOwn; this->dsInt = _dsInt;
    formatter = new OutputFormatter(info.msgTemplate.c_str());
    trackIntegrity(true, true); // delete flag & generated messages
}
Task::Task(ConfigView* config, std::string taskId, std::string tablePrefix, 
           DataSource* _dsOwn, DataSource* _dsInt)
    : logger(Logger::getInstance("smsc.infosme.Task")), formatter(0),
        usersCount(0), bFinalizing(false), bSelectedAll(false), dsOwn(_dsOwn), dsInt(_dsInt), 
            bInProcess(false), bInGeneration(false), bGenerationSuccess(true),
                lastMessagesCacheEmpty(0), currentPriorityFrameCounter(0)
{
    init(config, taskId, tablePrefix);
    formatter = new OutputFormatter(info.msgTemplate.c_str());
    trackIntegrity(true, true); // delete flag & generated messages
}
Task::~Task()
{
    if (formatter) delete formatter;

    if (dsInt) 
    {
        /* Used externally by other tasks
        dsInt->closeRegisteredQueries(INSERT_GENERATING_STATEMENT_ID);
        dsInt->closeRegisteredQueries(DELETE_GENERATING_STATEMENT_ID);
        */
        
        std::auto_ptr<char> delGeneratedId(prepareSqlCall(DELETE_NEW_MESSAGES_STATEMENT_ID));
        dsInt->closeRegisteredQueries(delGeneratedId.get());
        std::auto_ptr<char> newMessageId(prepareSqlCall(NEW_MESSAGE_STATEMENT_ID));
        dsInt->closeRegisteredQueries(newMessageId.get());
        std::auto_ptr<char> clearMessagesId(prepareSqlCall(CLEAR_MESSAGES_STATEMENT_ID));
        dsInt->closeRegisteredQueries(clearMessagesId.get());
        std::auto_ptr<char> deleteMessagesId(prepareSqlCall(DELETE_MESSAGES_STATEMENT_ID));
        dsInt->closeRegisteredQueries(deleteMessagesId.get());
        std::auto_ptr<char> resetMessagesId(prepareSqlCall(RESET_MESSAGES_STATEMENT_ID));
        dsInt->closeRegisteredQueries(resetMessagesId.get());
        std::auto_ptr<char> retryMessageId(prepareSqlCall(DO_RETRY_MESSAGE_STATEMENT_ID));
        dsInt->closeRegisteredQueries(retryMessageId.get());
        std::auto_ptr<char> deleteMessageId(prepareSqlCall(DO_DELETE_MESSAGE_STATEMENT_ID));
        dsInt->closeRegisteredQueries(deleteMessageId.get());
        std::auto_ptr<char> enrouteMessageId(prepareSqlCall(DO_ENROUTE_MESSAGE_STATEMENT_ID));
        dsInt->closeRegisteredQueries(enrouteMessageId.get());
        std::auto_ptr<char> selectMessageId(prepareSqlCall(SELECT_MESSAGES_STATEMENT_ID));
        dsInt->closeRegisteredQueries(selectMessageId.get());
        std::auto_ptr<char> waitMessageId(prepareSqlCall(DO_WAIT_MESSAGE_STATEMENT_ID));
        dsInt->closeRegisteredQueries(waitMessageId.get());
    }
    if (dsOwn)
    {
        std::auto_ptr<char> userQueryId(prepareSqlCall(USER_QUERY_STATEMENT_ID));
        dsOwn->closeRegisteredQueries(userQueryId.get());
    }
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
    info.transactionMode = config->getBool("transactionMode");
    info.trackIntegrity = config->getBool("trackIntegrity");
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
    info.svcType = "";
    if (info.replaceIfPresent) {
        try         { info.svcType = config->getString("svcType"); } 
        catch (...) { info.svcType = "";}
    }

    info.dsTimeout = 0;
    try { info.dsTimeout = config->getInt("dsTimeout"); } catch(...) {}
    if (info.dsTimeout < 0) info.dsTimeout = 0;
    
    info.dsUncommitedInProcess = 1;
    try { info.dsUncommitedInProcess = config->getInt("uncommitedInProcess"); } catch(...) {}
    if (info.dsUncommitedInProcess < 0) info.dsUncommitedInProcess = 1;
    info.dsUncommitedInGeneration = 1;
    try { info.dsUncommitedInGeneration = config->getInt("uncommitedInGeneration"); } catch(...) {}
    if (info.dsUncommitedInGeneration < 0) info.dsUncommitedInGeneration = 1;
    info.messagesCacheSize = 100;
    try { info.messagesCacheSize = config->getInt("messagesCacheSize"); } catch(...) {}
    if (info.messagesCacheSize <= 0) info.messagesCacheSize = 100;
    info.messagesCacheSleep = 1;
    try { info.messagesCacheSleep = config->getInt("messagesCacheSleep"); } catch(...) {}
    if (info.messagesCacheSleep <= 0) info.messagesCacheSleep = 1;
}

char* Task::prepareSqlCall(const char* sql)
{
    if (!sql || sql[0] == '\0') return 0;
    std::string tableName = info.tablePrefix+info.id;
    char* sqlCall = new char[strlen(sql)+tableName.length()+1];
    sprintf(sqlCall, sql, tableName.c_str());
    return sqlCall;
}
char* Task::prepareDoubleSqlCall(const char* sql)
{
    if (!sql || sql[0] == '\0') return 0;
    std::string tableName = info.tablePrefix+info.id;
    char* sqlCall = new char[strlen(sql)+tableName.length()*2+1];
    sprintf(sqlCall, sql, tableName.c_str(), tableName.c_str());
    return sqlCall;
}

void Task::trackIntegrity(bool clear, bool del, Connection* connection)
{
    smsc_log_debug(logger, "trackIntegrity method called on task '%s'",
                 info.id.c_str());
    
    if (!info.trackIntegrity) return;

    bool connectionInternal = false;
    try
    {
        if (!connection) {
            connection = dsInt->getConnection();
            connectionInternal = true;
        }
        if (!connection)
            throw Exception("Failed to obtain connection to internal data source.");

        if (clear)
        {
            Statement* delGenerating = connection->getStatement(DELETE_GENERATING_STATEMENT_ID,
                                                                DELETE_GENERATING_STATEMENT_SQL);
            if (!delGenerating) 
                throw Exception("Failed to obtain statement for track integrity.");
            
            delGenerating->setString(1, info.id.c_str());
            if (delGenerating->executeUpdate() > 0 && del)
            {
                std::auto_ptr<char> delGeneratedId(prepareSqlCall(DELETE_NEW_MESSAGES_STATEMENT_ID));
                std::auto_ptr<char> delGeneratedSql(prepareSqlCall(DELETE_NEW_MESSAGES_STATEMENT_SQL));
                Statement* delGenerated = connection->getStatement(delGeneratedId.get(),
                                                                   delGeneratedSql.get());
                if (!delGenerated) 
                    throw Exception("Failed to obtain statement for track integrity.");
                delGenerated->setUint8(1, MESSAGE_NEW_STATE);
                delGenerated->executeUpdate();
            }
        }
        else
        {
            Statement* insGenerating = connection->getStatement(INSERT_GENERATING_STATEMENT_ID,
                                                                INSERT_GENERATING_STATEMENT_SQL);
            if (!insGenerating) 
                throw Exception("Failed to obtain statement for track integrity.");
            
            insGenerating->setString(1, info.id.c_str());
            insGenerating->executeUpdate();
        }
        connection->commit();
    } 
    catch (Exception& exc)
    {
        try { if (connection) connection->rollback(); }
        catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. Failed to track task integrity. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...) {
        try { if (connection) connection->rollback(); }
        catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. Failed to track task integrity.",
                     info.id.c_str());
    }
    if (connection && connectionInternal) dsInt->freeConnection(connection);
}

void Task::createTable()
{
    smsc_log_debug(logger, "createTable method called on task '%s'", info.id.c_str());
    
    MutexGuard guard(createTableLock);
    
    Connection* connection = 0;
    try
    {
        connection = dsInt->getConnection();
        if (!connection)
            throw Exception("Failed to obtain connection to internal data source.");

        {
            std::auto_ptr<char> createTableSql(prepareSqlCall(NEW_TABLE_STATEMENT_SQL));
            std::auto_ptr<Statement> statementGuard(connection->createStatement(createTableSql.get()));
            Statement* statement = statementGuard.get();
            if (!statement) 
                throw Exception("Failed to create table statement.");
            statement->execute();  
        } 
        {
            std::auto_ptr<char> createIndexSql(prepareDoubleSqlCall(NEW_SD_INDEX_STATEMENT_SQL));
            std::auto_ptr<Statement> statementGuard(connection->createStatement(createIndexSql.get()));
            Statement* statement = statementGuard.get();
            if (!statement) 
                throw Exception("Failed to create index statement.");
            statement->execute();
        }

        connection->commit();
    } 
    catch (Exception& exc)
    {
        try { 
            if (connection) { 
                connection->rollback();
                dsInt->freeConnection(connection);
            }
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        Exception eee("Task '%s'. Failed to create internal table. Details: %s",
                       info.id.c_str(), exc.what());
        smsc_log_error(logger, "%s", eee.what());
        throw eee;
    }
    catch (...) {
        try { 
            if (connection) { 
                connection->rollback();
                dsInt->freeConnection(connection);
            }
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        Exception eee("Task '%s'. Failed to create internal table. Cause is unknown.",
                       info.id.c_str());
        smsc_log_error(logger, "%s", eee.what());
        throw eee;
    }
    if (connection) dsInt->freeConnection(connection);
}

void Task::dropTable()
{
    smsc_log_debug(logger, "dropTable method called on task '%s'", info.id.c_str());
    
    MutexGuard guard(createTableLock);
    
    Connection* connection = 0;
    try
    {
        connection = dsInt->getConnection();
        if (!connection)
            throw Exception("Failed to obtain connection to internal data source.");

        trackIntegrity(true, false, connection); // delete flag only

        std::auto_ptr<char> dropTableSql(prepareSqlCall(DROP_TABLE_STATEMENT_SQL));
        std::auto_ptr<Statement> statementGuard(connection->createStatement(dropTableSql.get()));
        Statement* statement = statementGuard.get();
        if (!statement) 
            throw Exception("Failed to create table statement.");
        statement->execute();
        connection->commit();
    } 
    catch (Exception& exc)
    {
        try { 
            if (connection) { 
                connection->rollback();
                dsInt->freeConnection(connection);
            }
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        Exception eee("Task '%s'. Failed to drop internal table. Details: %s",
                       info.id.c_str(), exc.what());
        smsc_log_error(logger, "%s", eee.what());
        throw eee;
    }
    catch (...) {
        try { 
            if (connection) { 
                connection->rollback();
                dsInt->freeConnection(connection);
            }
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        Exception eee("Task '%s'. Failed to drop internal table. Cause is unknown.",
                       info.id.c_str());
        smsc_log_error(logger, "%s", eee.what());
        throw eee;
    }
    if (connection) dsInt->freeConnection(connection);
}

bool Task::beginGeneration(Statistics* statistics)
{
    smsc_log_debug(logger, "beginGeneration method called on task '%s'",
                 info.id.c_str());

    uint64_t totalGenerated = 0;

    {
        MutexGuard guard(inGenerationLock);
        if (bInGeneration || (info.trackIntegrity && isInProcess())) return false;
        bInGeneration = true; bGenerationSuccess = false;
    }

    Connection* ownConnection = 0;
    Connection* intConnection = 0;
    int wdTimerId = -1;

    try
    {
        intConnection = dsInt->getConnection();
        if (!intConnection)
            throw Exception("Failed to obtain connection to internal data source.");
        ownConnection = dsOwn->getConnection();
        if (!ownConnection)
            throw Exception("Failed to obtain connection to own data source.");
        
        trackIntegrity(false, false, intConnection); // insert flag

        std::auto_ptr<char> userQueryId(prepareSqlCall(USER_QUERY_STATEMENT_ID));
        Statement* userQuery = ownConnection->getStatement(userQueryId.get(),
                                                           info.querySql.c_str());
        if (!userQuery)
            throw Exception("Failed to create user query statement on own data source.");
        
        std::auto_ptr<char> newMessageId(prepareSqlCall(NEW_MESSAGE_STATEMENT_ID));
        std::auto_ptr<char> newMessageSql(prepareSqlCall(NEW_MESSAGE_STATEMENT_SQL));
        Statement* newMessage = intConnection->getStatement(newMessageId.get(),
                                                            newMessageSql.get());
        if (!newMessage)
            throw Exception("Failed to create statement for message generation.");
            
        Statement* clearMessages = 0;
        if (info.replaceIfPresent) {
            std::auto_ptr<char> clearMessagesId(prepareSqlCall(CLEAR_MESSAGES_STATEMENT_ID));
            std::auto_ptr<char> clearMessagesSql(prepareSqlCall(CLEAR_MESSAGES_STATEMENT_SQL));
            clearMessages = intConnection->getStatement(clearMessagesId.get(),
                                                        clearMessagesSql.get());
            if (!clearMessages)
                throw Exception("Failed to create statement for message(s) replace.");
        }
        
        std::auto_ptr<ResultSet> rsGuard(userQuery->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
            throw Exception("Failed to obtain result set for message generation.");

        SQLGetAdapter       getAdapter(rs);
        ContextEnvironment  context;

        int uncommited = 0;
        while (isInGeneration() && rs->fetchNext())
        {
            const char* abonentAddress = rs->getString(1);
            if (!abonentAddress || abonentAddress[0] == '\0' || !isMSISDNAddress(abonentAddress)) {
                smsc_log_warn(logger, "Task '%s'. Invalid abonent number '%s' selected.", 
                              info.id.c_str(), abonentAddress ? abonentAddress:"-");
            }
            else
            {
                std::string message = "";
                formatter->format(message, getAdapter, context);
                if (message.length() > 0)
                {
                    wdTimerId = dsInt->startTimer(intConnection, info.dsTimeout);
                    if (clearMessages) {
                        clearMessages->setUint8(1, MESSAGE_NEW_STATE);
                        clearMessages->setString(2, abonentAddress);
                        clearMessages->executeUpdate();
                    }
                    newMessage->setUint8(1, MESSAGE_NEW_STATE);
                    newMessage->setString(2, abonentAddress);
                    newMessage->setDateTime(3, time(NULL));
                    newMessage->setString(4, message.c_str());
                    newMessage->executeUpdate();
                    if (wdTimerId >= 0) dsInt->stopTimer(wdTimerId);

                    if (statistics) statistics->incGenerated(info.id, 1);

                    if (info.dsUncommitedInGeneration <= 0 || ++uncommited >= info.dsUncommitedInGeneration) {
                        intConnection->commit();
                        uncommited = 0;
                    }
                    totalGenerated++;
                }
            }
        }
        if (uncommited > 0) intConnection->commit();
        
        {
            MutexGuard guard(inGenerationLock);
            if (info.trackIntegrity && !bInGeneration) bGenerationSuccess = false;
            else bGenerationSuccess = true;
        }

        trackIntegrity(true, false, intConnection); // delete flag only
    }
    catch (Exception& exc)
    {
        try { if (intConnection) intConnection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. Messages generation process failure. "
                     "Details: %s", info.id.c_str(), exc.what());
        
        trackIntegrity(true, true, intConnection); // delete flag & generated messages
        MutexGuard guard(inGenerationLock);
        bGenerationSuccess = false;
    }
    catch (...)
    {
        try { if (intConnection) intConnection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. Messages generation process failure.",
                     info.id.c_str());

        trackIntegrity(true, true, intConnection); // delete flag & generated messages
        MutexGuard guard(inGenerationLock);
        bGenerationSuccess = false;
    }
    
    if (wdTimerId >= 0) dsInt->stopTimer(wdTimerId);
    if (intConnection) dsInt->freeConnection(intConnection);
    if (ownConnection) dsOwn->freeConnection(ownConnection);
    
    {
        MutexGuard guard(inGenerationLock);
        bInGeneration = false;
    }
    generationEndEvent.Signal();
    return (bGenerationSuccess && totalGenerated > 0);
}
void Task::endGeneration()
{
    smsc_log_debug(logger, "endGeneration method called on task '%s'",
                 info.id.c_str());
    {
        MutexGuard guard(inGenerationLock);
        if (!bInGeneration) return;
        bInGeneration = false;
    }
    generationEndEvent.Wait();
}

void Task::dropAllMessages(Statistics* statistics)
{
    smsc_log_debug(logger, "dropAllMessages method called on task '%s'",
                   info.id.c_str());

    endGeneration();
    
    Connection* connection = 0;
    int wdTimerId = -1;
    try
    {
        connection = dsInt->getConnection();
        if (!connection)
            throw Exception("Failed to obtain connection to internal data source.");

        std::auto_ptr<char> deleteMessagesId(prepareSqlCall(DELETE_MESSAGES_STATEMENT_ID));
        std::auto_ptr<char> deleteMessagesSql(prepareSqlCall(DELETE_MESSAGES_STATEMENT_SQL));
        Statement* deleteMessages = connection->getStatement(deleteMessagesId.get(), 
                                                             deleteMessagesSql.get());
        if (!deleteMessages)
            throw Exception("Failed to create statement for messages access.");

        deleteMessages->setUint8(1, MESSAGE_NEW_STATE);
        
        wdTimerId = dsInt->startTimer(connection, info.dsTimeout);
        uint32_t deleted = deleteMessages->executeUpdate();
        connection->commit();
        if (statistics) statistics->incFailed(info.id, deleted);
    }
    catch (Exception& exc)
    {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...)
    {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. Messages access failure.", 
                     info.id.c_str());
    }

    dsInt->stopTimer(wdTimerId);
    if (connection) dsInt->freeConnection(connection);
}

void Task::resetWaiting(Connection* connection)
{
    smsc_log_debug(logger, "resetWaiting method called on task '%s'",
                 info.id.c_str());
    
    bool connectionInternal = false;
    try
    {
        if (!connection) {
            connection = dsInt->getConnection();
            connectionInternal = true;
        }
        if (!connection) 
            throw Exception("resetWaiting(): Failed to obtain connection");
        
        std::auto_ptr<char> resetMessagesId(prepareSqlCall(RESET_MESSAGES_STATEMENT_ID));
        std::auto_ptr<char> resetMessagesSql(prepareSqlCall(RESET_MESSAGES_STATEMENT_SQL));
        Statement* resetMessages = connection->getStatement(resetMessagesId.get(), 
                                                            resetMessagesSql.get());
        if (!resetMessages)
            throw Exception("resetWaiting(): Failed to create statement for messages access.");
        
        resetMessages->setUint8(1, MESSAGE_NEW_STATE);
        resetMessages->setUint8(2, MESSAGE_WAIT_STATE);

        resetMessages->executeUpdate();
        connection->commit();
    }
    catch (Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...) {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. Messages access failure.", info.id.c_str());
    }

    if (connectionInternal && connection) dsInt->freeConnection(connection);
}

bool Task::retryMessage(uint64_t msgId, time_t nextTime, Connection* connection)
{
    smsc_log_debug(logger, "retryMessage method called on task '%s' for id=%lld",
                 info.id.c_str(), msgId);

    int wdTimerId = -1;
    bool result = false;
    bool connectionInternal = false;
    try
    {
        if (!connection) {
            connection = dsInt->getConnection();
            connectionInternal = true;
        }
        if (!connection) 
            throw Exception("doRetry(): Failed to obtain connection");

        std::auto_ptr<char> retryMessageId(prepareSqlCall(DO_RETRY_MESSAGE_STATEMENT_ID));
        std::auto_ptr<char> retryMessageSql(prepareSqlCall(DO_RETRY_MESSAGE_STATEMENT_SQL));
        Statement* retryMessage = connection->getStatement(retryMessageId.get(), 
                                                           retryMessageSql.get());
        if (!retryMessage)
            throw Exception("doRetry(): Failed to create statement for messages access.");
        
        wdTimerId = dsInt->startTimer(connection, info.dsTimeout);

        retryMessage->setUint8   (1, MESSAGE_NEW_STATE);
        retryMessage->setDateTime(2, nextTime); 
        retryMessage->setUint64  (3, msgId);
        
        result = (retryMessage->executeUpdate() > 0);
        if (result) connection->commit();
        else connection->rollback();
    }
    catch (Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. doRetry(): Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...) {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. doRetry(): Messages access failure.", 
                     info.id.c_str());
    }
    
    dsInt->stopTimer(wdTimerId);
    if (connectionInternal && connection) dsInt->freeConnection(connection);
    return result;
}

bool Task::deleteMessage(uint64_t msgId, Connection* connection)
{
    smsc_log_debug(logger, "deleteMessage method called on task '%s' for id=%lld",
                 info.id.c_str(), msgId);

    int wdTimerId = -1;
    bool result = false;
    bool connectionInternal = false;
    try
    {
        if (!connection) {
            connection = dsInt->getConnection();
            connectionInternal = true;
        }
        if (!connection) 
            throw Exception("deleteMessage(): Failed to obtain connection");

        std::auto_ptr<char> deleteMessageId(prepareSqlCall(DO_DELETE_MESSAGE_STATEMENT_ID));
        std::auto_ptr<char> deleteMessageSql(prepareSqlCall(DO_DELETE_MESSAGE_STATEMENT_SQL));
        Statement* deleteMessage = connection->getStatement(deleteMessageId.get(), 
                                                            deleteMessageSql.get());
        if (!deleteMessage)
            throw Exception("deleteMessage(): Failed to create statement for messages access.");
        
        wdTimerId = dsInt->startTimer(connection, info.dsTimeout);
        deleteMessage->setUint64  (1, msgId);
        result = (deleteMessage->executeUpdate() > 0);
        if (result) connection->commit();
        else connection->rollback();
    }
    catch (Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. deleteMessage(): Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...) {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. deleteMessage(): Messages access failure.", 
                     info.id.c_str());
    }
    
    dsInt->stopTimer(wdTimerId);
    if (connectionInternal && connection) dsInt->freeConnection(connection);
    return result;
}

bool Task::enrouteMessage(uint64_t msgId, Connection* connection)
{
    smsc_log_debug(logger, "enrouteMessage method called on task '%s' for id=%lld",
                 info.id.c_str(), msgId);

    int wdTimerId = -1;
    bool result = false;
    bool connectionInternal = false;
    try
    {
        if (!connection) {
            connection = dsInt->getConnection();
            connectionInternal = true;
        }
        if (!connection) 
            throw Exception("deleteMessage(): Failed to obtain connection");
        
        std::auto_ptr<char> enrouteMessageId(prepareSqlCall(DO_ENROUTE_MESSAGE_STATEMENT_ID));
        std::auto_ptr<char> enrouteMessageSql(prepareSqlCall(DO_ENROUTE_MESSAGE_STATEMENT_SQL));
        Statement* enrouteMessage = connection->getStatement(enrouteMessageId.get(), 
                                                             enrouteMessageSql.get());
        if (!enrouteMessage)
            throw Exception("doEnroute(): Failed to create statement for messages access.");
        
        enrouteMessage->setUint8 (1, MESSAGE_ENROUTE_STATE);
        enrouteMessage->setUint64(2, msgId);
        enrouteMessage->setUint8 (3, MESSAGE_WAIT_STATE);

        wdTimerId = dsInt->startTimer(connection, info.dsTimeout);
        result = (enrouteMessage->executeUpdate() > 0);
        if (result) connection->commit();
        else connection->rollback();
    }
    catch (Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. doEnroute(): Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...) {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. doEnroute(): Messages access failure.", 
                     info.id.c_str());
    }
    
    dsInt->stopTimer(wdTimerId);
    if (connectionInternal && connection) dsInt->freeConnection(connection);
    return result;
}

bool Task::getNextMessage(Connection* connection, Message& message)
{
    /*smsc_log_debug(logger, "getNextMessage method called on task '%s'",
                   info.id.c_str());*/
    __require__(connection);

    if (!isEnabled())
        return setInProcess(false);

    {
        // selecting from cache
        MutexGuard guard(messagesCacheLock); 
        if (messagesCache.Count() > 0) {
            messagesCache.Shift(message);
            setInProcess(isEnabled());
            return true;
        }
    }   // Cache is empty here

    if (info.trackIntegrity && !isGenerationSucceeded())
        return setInProcess(false); // for track integrity check that generation finished ok

    time_t currentTime = time(NULL);
    if (currentTime-lastMessagesCacheEmpty > info.messagesCacheSleep)
        lastMessagesCacheEmpty = currentTime;   // timeout reached, set new sleep timeout & go to fill cache
    else if (bSelectedAll && !isInGeneration()) 
        return setInProcess(false);             // if all selected from DB (on last time) & no generation => return
    
    smsc_log_debug(logger, "Selecting messages from DB. getNextMessage method on task '%s'",
                   info.id.c_str());

    // Selecting cache from DB
    int wdTimerId = -1;
    try
    {
        std::auto_ptr<char> selectMessageId(prepareSqlCall(SELECT_MESSAGES_STATEMENT_ID));
        std::auto_ptr<char> selectMessageSql(prepareSqlCall(SELECT_MESSAGES_STATEMENT_SQL));
        Statement* selectMessage = connection->getStatement(selectMessageId.get(), 
                                                            selectMessageSql.get());
        if (!selectMessage)
            throw Exception("Failed to create statement for messages access.");

        selectMessage->setUint8   (1, MESSAGE_NEW_STATE);
        selectMessage->setDateTime(2, currentTime);

        wdTimerId = dsInt->startTimer(connection, info.dsTimeout);

        std::auto_ptr<ResultSet> rsGuard(selectMessage->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
            throw Exception("Failed to obtain result set for message access.");
        
        int fetched = 0; int uncommited = 0;
        while (rs->fetchNext() && ++fetched <= info.messagesCacheSize)
        {
            Message fetchedMessage(rs->getUint64(1), rs->getString(2), rs->getString(3));
            dsInt->stopTimer(wdTimerId);
            
            std::auto_ptr<char> waitMessageId(prepareSqlCall(DO_WAIT_MESSAGE_STATEMENT_ID));
            std::auto_ptr<char> waitMessageSql(prepareSqlCall(DO_WAIT_MESSAGE_STATEMENT_SQL));
            Statement* waitMessage = connection->getStatement(waitMessageId.get(), 
                                                              waitMessageSql.get());
            if (!waitMessage)
                throw Exception("Failed to create statement for messages access.");
            
            wdTimerId = dsInt->startTimer(connection, info.dsTimeout);
            waitMessage->setUint8 (1, MESSAGE_WAIT_STATE);
            waitMessage->setUint64(2, fetchedMessage.id);
            if (waitMessage->executeUpdate() <= 0)
                smsc_log_warn(logger, "Failed to update message in getNextMessage() !!!");
                // TODO: analyse it !
            {
                MutexGuard guard(messagesCacheLock);
                messagesCache.Push(fetchedMessage);
            }
            
            if (info.dsUncommitedInProcess <= 0 || ++uncommited >= info.dsUncommitedInProcess) {
                connection->commit();
                uncommited = 0;
            }
        }
        bSelectedAll = (fetched < info.messagesCacheSize);
        if (uncommited > 0) connection->commit();
    }
    catch (Exception& exc)
    {
        try { if (connection) connection->rollback(); }
        catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...)
    {
        try { if (connection) connection->rollback(); }
        catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction on internal data source.");
        }
        smsc_log_error(logger, "Task '%s'. Messages access failure.", 
                     info.id.c_str());
    }
    
    dsInt->stopTimer(wdTimerId);
    
    MutexGuard guard(messagesCacheLock);
    if (messagesCache.Count() > 0) {
        messagesCache.Shift(message);
        setInProcess(isEnabled());
        return true;
    } 
    else {
        lastMessagesCacheEmpty = time(NULL);
        bSelectedAll = true;
    }
    return setInProcess(false);
}

bool Task::isReady(time_t time, bool checkActivePeriod)
{
    if (!isEnabled() || isFinalizing() || 
        (info.endDate>0 && time>=info.endDate)) return false;

    if (checkActivePeriod && info.activePeriodStart > 0 && info.activePeriodEnd > 0)
    {
        if (info.activePeriodStart > info.activePeriodEnd) return false;
        
        tm dt; localtime_r(&time, &dt);
        
        dt.tm_isdst = -1;
        dt.tm_hour = info.activePeriodStart/3600;
        dt.tm_min  = (info.activePeriodStart%3600)/60;
        dt.tm_sec  = (info.activePeriodStart%3600)%60;
        time_t apst = mktime(&dt);

        dt.tm_isdst = -1;
        dt.tm_hour = info.activePeriodEnd/3600;
        dt.tm_min  = (info.activePeriodEnd%3600)/60;
        dt.tm_sec  = (info.activePeriodEnd%3600)%60;
        time_t apet = mktime(&dt);

        if (time < apst || time > apet) return false;
    }

    return true;
}

}}
