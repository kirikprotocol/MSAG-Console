
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

Task::Task(TaskInfo& info, DataSource* dsOwn, DataSource* dsInt) 
    : logger(Logger::getCategory("smsc.infosme.Task")), formatter(0),
        usersCount(0), bFinalizing(false), dsOwn(dsOwn), dsInt(dsInt), 
            bInProcess(false), bTableCreated(false), 
                lastMessagesCacheEmpty(0), currentPriorityFrameCounter(0)
{
    __require__(dsOwn && dsInt);
    this->info = info; this->dsOwn = dsOwn; this->dsInt = dsInt;
    formatter = new OutputFormatter(info.msgTemplate.c_str());
    createTable();
}
Task::Task(ConfigView* config, std::string taskId, std::string tablePrefix, 
     DataSource* dsOwn, DataSource* dsInt)
    : logger(Logger::getCategory("smsc.infosme.Task")), formatter(0),
        usersCount(0), bFinalizing(false), dsOwn(dsOwn), dsInt(dsInt), 
            bInProcess(false), bTableCreated(false),
                lastMessagesCacheEmpty(0), currentPriorityFrameCounter(0)
{
    init(config, taskId, tablePrefix);
    formatter = new OutputFormatter(info.msgTemplate.c_str());
    createTable();
}
Task::~Task()
{
    if (formatter) delete formatter;
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
    info.dsOwnTimeout = 0;
    try { info.dsOwnTimeout = config->getInt("dsOwnTimeout"); } catch(...) {}
    if (info.dsOwnTimeout < 0) info.dsOwnTimeout = 0;
    info.dsIntTimeout = 0;
    try { info.dsIntTimeout = config->getInt("dsIntTimeout"); } catch(...) {}
    if (info.dsIntTimeout < 0) info.dsIntTimeout = 0;
    info.dsUncommited = 1;
    try { info.dsUncommited = config->getInt("uncommited"); } catch(...) {}
    if (info.dsUncommited < 0) info.dsUncommited = 1;
}

bool Task::isInProcess()
{
    MutexGuard guard(inProcessLock);
    return bInProcess;
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

Statement* Task::getStatement(Connection* connection, const char* id, const char* sql)
{
    if (!connection || !id) return 0;

    static Mutex     statementLock;
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

const char* NEW_TABLE_STATEMENT_SQL = 
"CREATE TABLE %s (\n"
"ID             NUMBER          NOT NULL,\n"
"STATE          NUMBER(3)       NOT NULL,\n"
"ABONENT        VARCHAR2(30)    NOT NULL,\n"
"SEND_DATE      DATE            NOT NULL,\n"
"MESSAGE        VARCHAR2(2000)  NULL,    \n"
"PRIMARY KEY    (ID)                     \n"
")";
const char* NEW_SD_INDEX_STATEMENT_SQL = 
"CREATE INDEX %s_SD_IDX ON %s (STATE, SEND_DATE)";

void Task::createTable()
{
    MutexGuard guard(createTableLock);
    
    if (!bTableCreated)
    {
        Connection* connection = 0;
        Statement* statement = 0;
        try
        {
            connection = dsInt->getConnection();
            if (!connection)
                throw Exception("Failed to obtain connection to internal data source.");
            
            std::auto_ptr<char> createTableSql(prepareSqlCall(NEW_TABLE_STATEMENT_SQL));
            statement = connection->createStatement(createTableSql.get());
            if (!statement) 
                throw Exception("Failed to create table statement.");
            statement->execute();
            delete statement;

            std::auto_ptr<char> createIndexSql(prepareDoubleSqlCall(NEW_SD_INDEX_STATEMENT_SQL));
            statement = connection->createStatement(createIndexSql.get());
            if (!statement) 
                throw Exception("Failed to create index statement.");
            statement->execute();
            
            connection->commit();
            bTableCreated = true;
        } 
        catch (Exception& exc)
        {
            try { if (connection) connection->rollback(); }
            catch (...) {
                logger.error("Failed to roolback transaction on internal data source.");
            }
            logger.error("Task '%s'. Failed to create internal table. "
                         "Details: %s", info.id.c_str(), exc.what());
        }
        catch (...) {
            try { if (connection) connection->rollback(); }
            catch (...) {
                logger.error("Failed to roolback transaction on internal data source.");
            }
            logger.error("Task '%s'. Failed to create internal table.",
                         info.id.c_str());
        }
        if (statement) delete statement;
        if (connection) dsInt->freeConnection(connection);
    }
}

const char* USER_QUERY_STATEMENT_ID = "%s_USER_QUERY_STATEMENT_ID";

const char* NEW_MESSAGE_STATEMENT_ID = "%s_NEW_MESSAGE_STATEMENT_ID";
const char* NEW_MESSAGE_STATEMENT_SQL = (const char*)
"INSERT INTO %s (ID, STATE, ABONENT, SEND_DATE, MESSAGE) "
"VALUES (INFOSME_MESSAGES_SEQ.NEXTVAL, :STATE, :ABONENT, :SEND_DATE, :MESSAGE)";

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
        
        std::auto_ptr<char> userQueryId(prepareSqlCall(USER_QUERY_STATEMENT_ID));
        Statement* userQuery = getStatement(ownConnection, userQueryId.get(), 
                                            info.querySql.c_str());
        if (!userQuery)
            throw Exception("Failed to create user query statement on own data source.");
        
        std::auto_ptr<char> newMessageId(prepareSqlCall(NEW_MESSAGE_STATEMENT_ID));
        std::auto_ptr<char> newMessageSql(prepareSqlCall(NEW_MESSAGE_STATEMENT_SQL));
        Statement* newMessage = getStatement(intConnection, newMessageId.get(), 
                                             newMessageSql.get());
        if (!newMessage)
            throw Exception("Failed to create statement for message generation.");
            
        wdOwnTimerId = dsOwn->startTimer(ownConnection, info.dsOwnTimeout);
        std::auto_ptr<ResultSet> rsGuard(userQuery->executeQuery());
        ResultSet* rs = rsGuard.get();
        dsOwn->stopTimer(wdOwnTimerId);
        if (!rs)
            throw Exception("Failed to obtain result set for message generation.");

        SQLGetAdapter       getAdapter(rs);
        ContextEnvironment  context;

        int uncommitted = 0;
        while (bInProcess)
        {
            wdOwnTimerId = dsOwn->startTimer(ownConnection, info.dsOwnTimeout);
            bool fetched = rs->fetchNext();
            dsOwn->stopTimer(wdOwnTimerId);
            if (!fetched) break;

            const char* abonentAddress = rs->getString(1);
            if (!abonentAddress || abonentAddress[0] == '\0' || !isMSISDNAddress(abonentAddress)) {
                logger.warn("Invalid abonent number '%s' selected.", 
                            abonentAddress ? abonentAddress:"-");
                continue;
            }
            
            std::string message = "";
            formatter->format(message, getAdapter, context);
            if (message.length() > 0)
            {
                newMessage->setUint8(1, MESSAGE_NEW_STATE);
                newMessage->setString(2, abonentAddress);
                newMessage->setDateTime(3, time(NULL));
                newMessage->setString(4, message.c_str());
                newMessage->executeUpdate();

                //logger.info("Message '%s' for '%s' generated.", message.c_str(), abonentAddress);

                if (info.dsUncommited <= 0 || ++uncommitted >= info.dsUncommited) {
                    intConnection->commit();
                    uncommitted = 0;
                }
            }
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
    
    {
        MutexGuard guard(inProcessLock);
        bInProcess = false;
    }
    
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

const char* DELETE_MESSAGES_STATEMENT_ID = "%s_DELETE_MESSAGES_STATEMENT_ID";
const char* DELETE_MESSAGES_STATEMENT_SQL = 
"DELETE FROM %s WHERE STATE=:NEW";

void Task::dropAllMessages()
{
    endProcess();
    
    Connection* connection = 0;
    int wdTimerId = -1;
    try
    {
        connection = dsInt->getConnection();
        if (!connection)
            throw Exception("Failed to obtain connection to internal data source.");

        std::auto_ptr<char> deleteMessagesId(prepareSqlCall(DELETE_MESSAGES_STATEMENT_ID));
        std::auto_ptr<char> deleteMessagesSql(prepareSqlCall(DELETE_MESSAGES_STATEMENT_SQL));
        Statement* deleteMessages = getStatement(connection, deleteMessagesId.get(), 
                                                 deleteMessagesSql.get());
        if (!deleteMessages)
            throw Exception("Failed to create statement for messages access.");

        deleteMessages->setUint8(1, MESSAGE_NEW_STATE);
        
        wdTimerId = dsInt->startTimer(connection, info.dsIntTimeout);
        deleteMessages->executeUpdate();
        connection->commit();
    }
    catch (Exception& exc)
    {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            logger.error("Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            logger.error("Failed to roolback transaction on internal data source.");
        }
        logger.error("Task '%s'. Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...)
    {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            logger.error("Failed to roolback transaction on internal data source. "
                         "Details: %s", exc.what());
        } catch (...) {
            logger.error("Failed to roolback transaction on internal data source.");
        }
        logger.error("Task '%s'. Messages access failure.", 
                     info.id.c_str());
    }

    dsInt->stopTimer(wdTimerId);
    if (connection) dsInt->freeConnection(connection);
}

const char* RESET_MESSAGES_STATEMENT_ID = "%s_RESET_MESSAGES_STATEMENT_ID";
const char* RESET_MESSAGES_STATEMENT_SQL = 
"UPDATE %s SET STATE=:NEW WHERE STATE=:WAIT";

void Task::resetWaiting(Connection* connection)
{
    logger.debug("resetWaiting called");

    try
    {
        std::auto_ptr<char> resetMessagesId(prepareSqlCall(RESET_MESSAGES_STATEMENT_ID));
        std::auto_ptr<char> resetMessagesSql(prepareSqlCall(RESET_MESSAGES_STATEMENT_SQL));
        Statement* resetMessages = getStatement(connection, resetMessagesId.get(), 
                                                resetMessagesSql.get());
        if (!resetMessages)
            throw Exception("Failed to create statement for messages access.");
        
        resetMessages->setUint8(1, MESSAGE_NEW_STATE);
        resetMessages->setUint8(2, MESSAGE_WAIT_STATE);

        resetMessages->executeUpdate();
    }
    catch (Exception& exc) {
        logger.error("Task '%s'. Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...) {
        logger.error("Task '%s'. Messages access failure.", info.id.c_str());
    }
}

const char* DO_RETRY_MESSAGE_STATEMENT_ID = "%s_DO_RETRY_MESSAGE_STATEMENT_ID";
const char* DO_RETRY_MESSAGE_STATEMENT_SQL = 
"UPDATE %s SET STATE=:NEW_OR_FAILED, SEND_DATE=:SEND_DATE "
"WHERE ID=:ID AND (STATE=:ENROUTE OR STATE=:WAIT)";

bool Task::doRetry(Connection* connection, uint64_t msgId)
{
    logger.debug("doRetry(): called for id=%lld", msgId);

    if (!info.retryOnFail || info.retryTime <= 0) return false;

    __require__(connection);

    int wdTimerId = -1;
    bool result = false;
    try
    {
        std::auto_ptr<char> retryMessageId(prepareSqlCall(DO_RETRY_MESSAGE_STATEMENT_ID));
        std::auto_ptr<char> retryMessageSql(prepareSqlCall(DO_RETRY_MESSAGE_STATEMENT_SQL));
        Statement* retryMessage = getStatement(connection, retryMessageId.get(), 
                                               retryMessageSql.get());
        if (!retryMessage)
            throw Exception("doRetry(): Failed to create statement for messages access.");
        
        time_t ct = time(NULL);
        time_t nt = ct+info.retryTime;

        retryMessage->setUint8   (1, (info.endDate>0 && nt>=ct) ? 
                                     MESSAGE_FAILED_STATE:MESSAGE_NEW_STATE);
        retryMessage->setDateTime(2, nt); 
        retryMessage->setUint64  (3, msgId);
        retryMessage->setUint8   (4, MESSAGE_ENROUTE_STATE);
        retryMessage->setUint8   (5, MESSAGE_WAIT_STATE);
        
        wdTimerId = dsInt->startTimer(connection, info.dsIntTimeout);
        result = (retryMessage->executeUpdate() > 0);
    }
    catch (Exception& exc) {
        logger.error("Task '%s'. doRetry(): Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...) {
        logger.error("Task '%s'. doRetry(): Messages access failure.", 
                     info.id.c_str());
    }
    dsInt->stopTimer(wdTimerId);
    return result;
}

const char* DO_ENROUTE_MESSAGE_STATEMENT_ID = "%s_DO_ENROUTE_MESSAGE_STATEMENT_ID";
const char* DO_ENROUTE_MESSAGE_STATEMENT_SQL = 
"UPDATE %s SET STATE=:ENROUTE WHERE ID=:ID AND STATE=:WAIT";

bool Task::doEnroute(Connection* connection, uint64_t msgId)
{
    logger.debug("doEnroute(): called for id=%lld", msgId);

    __require__(connection);

    int wdTimerId = -1;
    bool result = false;
    try
    {
        std::auto_ptr<char> enrouteMessageId(prepareSqlCall(DO_ENROUTE_MESSAGE_STATEMENT_ID));
        std::auto_ptr<char> enrouteMessageSql(prepareSqlCall(DO_ENROUTE_MESSAGE_STATEMENT_SQL));
        Statement* enrouteMessage = getStatement(connection, enrouteMessageId.get(), 
                                                 enrouteMessageSql.get());
        if (!enrouteMessage)
            throw Exception("doEnroute(): Failed to create statement for messages access.");
        
        enrouteMessage->setUint8 (1, MESSAGE_ENROUTE_STATE);
        enrouteMessage->setUint64(2, msgId);
        enrouteMessage->setUint8 (3, MESSAGE_WAIT_STATE);

        wdTimerId = dsInt->startTimer(connection, info.dsIntTimeout);
        result = (enrouteMessage->executeUpdate() > 0);
    }
    catch (Exception& exc) {
        logger.error("Task '%s'. doEnroute(): Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...) {
        logger.error("Task '%s'. doEnroute(): Messages access failure.", 
                     info.id.c_str());
    }
    dsInt->stopTimer(wdTimerId);
    return result;
}

const char* DO_FAILED_MESSAGE_STATEMENT_ID = "%s_DO_FAILED_MESSAGE_STATEMENT_ID";
const char* DO_FAILED_MESSAGE_STATEMENT_SQL = 
"UPDATE %s SET STATE=:FAILED WHERE ID=:ID AND (STATE=:ENROUTE OR STATE=:WAIT)";

bool Task::doFailed(Connection* connection, uint64_t msgId)
{
    logger.debug("doFailed(): called for id=%lld", msgId);

    __require__(connection);
    
    int wdTimerId = -1;
    bool result = false;
    try
    {
        std::auto_ptr<char> failedMessageId(prepareSqlCall(DO_FAILED_MESSAGE_STATEMENT_ID));
        std::auto_ptr<char> failedMessageSql(prepareSqlCall(DO_FAILED_MESSAGE_STATEMENT_SQL));
        Statement* failedMessage = getStatement(connection, failedMessageId.get(), 
                                                failedMessageSql.get());
        if (!failedMessage)
            throw Exception("doFailed(): Failed to create statement for messages access.");
        
        failedMessage->setUint8 (1, MESSAGE_FAILED_STATE);
        failedMessage->setUint64(2, msgId);
        failedMessage->setUint8 (3, MESSAGE_ENROUTE_STATE);
        failedMessage->setUint8 (4, MESSAGE_WAIT_STATE);

        wdTimerId = dsInt->startTimer(connection, info.dsIntTimeout);
        result = (failedMessage->executeUpdate() > 0);
    }
    catch (Exception& exc) {
        logger.error("Task '%s'. doFailed(): Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...) {
        logger.error("Task '%s'. doFailed(): Messages access failure.", 
                     info.id.c_str());
    }
    dsInt->stopTimer(wdTimerId);
    return result;
}
        
const char* DO_DELIVERED_MESSAGE_STATEMENT_ID = "%s_DO_DELIVERED_MESSAGE_STATEMENT_ID";
const char* DO_DELIVERED_MESSAGE_STATEMENT_SQL = 
"UPDATE %s SET STATE=:DELIVERED WHERE ID=:ID AND STATE=:ENROUTE";

bool Task::doDelivered(Connection* connection, uint64_t msgId)
{
    logger.debug("doDelivered(): called for id=%lld", msgId);

    __require__(connection);

    int wdTimerId = -1;
    bool result = false;
    try
    {
        std::auto_ptr<char> deliveredMessageId(prepareSqlCall(DO_DELIVERED_MESSAGE_STATEMENT_ID));
        std::auto_ptr<char> deliveredMessageSql(prepareSqlCall(DO_DELIVERED_MESSAGE_STATEMENT_SQL));
        Statement* deliveredMessage = getStatement(connection, deliveredMessageId.get(), 
                                                   deliveredMessageSql.get());
        if (!deliveredMessage)
            throw Exception("doDelivered(): Failed to create statement for messages access.");
        
        deliveredMessage->setUint8 (1, MESSAGE_DELIVERED_STATE);
        deliveredMessage->setUint64(2, msgId);
        deliveredMessage->setUint8 (3, MESSAGE_ENROUTE_STATE);
        
        wdTimerId = dsInt->startTimer(connection, info.dsIntTimeout);
        result = (deliveredMessage->executeUpdate() > 0);
    }
    catch (Exception& exc) {
        logger.error("Task '%s'. doDelivered(): Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...) {
        logger.error("Task '%s'. doDelivered(): Messages access failure.", 
                     info.id.c_str());
    }
    dsInt->stopTimer(wdTimerId);
    return result;
}

const char* SELECT_MESSAGES_STATEMENT_ID = "%s_SELECT_MESSAGES_STATEMENT_ID";
const char* SELECT_MESSAGES_STATEMENT_SQL = (const char*)
"SELECT ID, ABONENT, MESSAGE FROM %s WHERE "
"STATE=:STATE AND SEND_DATE<=:SEND_DATE ORDER BY ID ASC FOR UPDATE";

const char* DO_WAIT_MESSAGE_STATEMENT_ID = "%s_DO_WAIT_MESSAGE_STATEMENT_ID";
const char* DO_WAIT_MESSAGE_STATEMENT_SQL = (const char*)
"UPDATE %s SET STATE=:WAIT WHERE ID=:ID";

bool Task::getNextMessage(Connection* connection, Message& message)
{
    __require__(connection);
    
    {
        MutexGuard guard(messagesCacheLock);
        if (messagesCache.Count() > 0) {
            messagesCache.Shift(message);
            return true;
        }
    }

    const int sleepOnEmptyInterval = 1;
    const int fetchSize = 1000;

    time_t currentTime = time(NULL);
    if (currentTime-lastMessagesCacheEmpty > sleepOnEmptyInterval)
        lastMessagesCacheEmpty = currentTime;
    //else return false;
    
    int wdTimerId = -1;
    try
    {
        std::auto_ptr<char> selectMessageId(prepareSqlCall(SELECT_MESSAGES_STATEMENT_ID));
        std::auto_ptr<char> selectMessageSql(prepareSqlCall(SELECT_MESSAGES_STATEMENT_SQL));
        Statement* selectMessage = getStatement(connection, selectMessageId.get(), 
                                                selectMessageSql.get());
        if (!selectMessage)
            throw Exception("Failed to create statement for messages access.");

        selectMessage->setUint8   (1, MESSAGE_NEW_STATE);
        selectMessage->setDateTime(2, currentTime);

        wdTimerId = dsInt->startTimer(connection, info.dsIntTimeout);

        std::auto_ptr<ResultSet> rsGuard(selectMessage->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
            throw Exception("Failed to obtain result set for message access.");
        
        int fetched = 0;
        while (rs->fetchNext() && ++fetched <= fetchSize)
        {
            Message fetchedMessage(rs->getUint64(1), rs->getString(2), rs->getString(3));
            dsInt->stopTimer(wdTimerId);
            
            std::auto_ptr<char> waitMessageId(prepareSqlCall(DO_WAIT_MESSAGE_STATEMENT_ID));
            std::auto_ptr<char> waitMessageSql(prepareSqlCall(DO_WAIT_MESSAGE_STATEMENT_SQL));
            Statement* waitMessage = getStatement(connection, waitMessageId.get(), 
                                                  waitMessageSql.get());
            if (!waitMessage)
                throw Exception("Failed to create statement for messages access.");
            
            wdTimerId = dsInt->startTimer(connection, info.dsIntTimeout);
            waitMessage->setUint8 (1, MESSAGE_WAIT_STATE);
            waitMessage->setUint64(2, fetchedMessage.id);
            if (waitMessage->executeUpdate() <= 0)
                logger.warn("Failed to update message in getNextMessage() !!!");
                // TODO: analyse it !
            
            MutexGuard guard(messagesCacheLock);
            messagesCache.Push(fetchedMessage);
        }
        connection->commit();
    }
    catch (Exception& exc)
    {
        try { if (connection) connection->rollback(); }
        catch (...) {
            logger.error("Failed to roolback transaction on internal data source.");
        }
        logger.error("Task '%s'. Messages access failure. "
                     "Details: %s", info.id.c_str(), exc.what());
    }
    catch (...)
    {
        try { if (connection) connection->rollback(); }
        catch (...) {
            logger.error("Failed to roolback transaction on internal data source.");
        }
        logger.error("Task '%s'. Messages access failure.", 
                     info.id.c_str());
    }
    
    dsInt->stopTimer(wdTimerId);
    
    MutexGuard guard(messagesCacheLock);
    if (messagesCache.Count() > 0) {
        messagesCache.Shift(message);
        return true;
    } 
    else lastMessagesCacheEmpty = time(NULL);
    return false;
}

bool Task::isReady(time_t time)
{
    if (!isEnabled() || isFinalizing() || 
        (info.endDate>0 && time>=info.endDate)) return false;

    if (info.activePeriodStart > 0 && info.activePeriodEnd > 0)
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
