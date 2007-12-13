#include "Task.h"
#include "SQLAdapters.h"
#include <time.h>
#include <sstream>
#include <list>
#include <util/sleep.h>
#include <sms/sms.h>
#include <util/config/region/Region.hpp>
#include <util/config/region/RegionFinder.hpp>

extern bool isMSISDNAddress(const char* string);

namespace smsc { namespace infosme 
{

const char* USER_QUERY_STATEMENT_ID           = "%s_USER_QUERY_STATEMENT_ID";   // Own

const char* DELETE_GENERATING_STATEMENT_ID    = "DELETE_GENERATING_STATEMENT_ID";
const char* INSERT_GENERATING_STATEMENT_ID    = "INSERT_GENERATING_STATEMENT_ID"; 
const char* DELETE_NEW_MESSAGES_STATEMENT_ID  = "%s_DELETE_NEW_MESSAGES_STATEMENT_ID";
const char* SELECT_MESSAGES_STATEMENT_ID      = "%s_SELECT_MESSAGES_STATEMENT_ID";
const char* NEW_MESSAGE_STATEMENT_ID          = "%s_NEW_MESSAGE_STATEMENT_ID";
const char* CLEAR_MESSAGES_STATEMENT_ID       = "%s_CLEAR_MESSAGES_STATEMENT_ID";
const char* RESET_MESSAGES_STATEMENT_ID       = "%s_RESET_MESSAGES_STATEMENT_ID";
const char* DO_STATE_MESSAGE_STATEMENT_ID     = "%s_DO_STATE_MESSAGE_STATEMENT_ID";
const char* DO_RETRY_MESSAGE_STATEMENT_ID     = "%s_DO_RETRY_MESSAGE_STATEMENT_ID";
const char* DO_DELETE_MESSAGE_STATEMENT_ID    = "%s_DO_DELETE_MESSAGE_STATEMENT_ID";
const char* DO_ENROUTE_MESSAGE_STATEMENT_ID   = "%s_DO_ENROUTE_MESSAGE_STATEMENT_ID";

const char* DELETE_GENERATING_STATEMENT_SQL   = "DELETE FROM INFOSME_GENERATING_TASKS WHERE TASK_ID=:TASK_ID";
const char* INSERT_GENERATING_STATEMENT_SQL   = "INSERT INTO INFOSME_GENERATING_TASKS TASK_ID VALUES(:TASK_ID)";
const char* DELETE_NEW_MESSAGES_STATEMENT_SQL = "DELETE FROM %s WHERE STATE=:NEW";
const char* CLEAR_MESSAGES_STATEMENT_SQL      = "DELETE FROM %s WHERE STATE=:NEW AND ABONENT=:ABONENT";
const char* NEW_SD_INDEX_STATEMENT_SQL        = "CREATE INDEX %s_SD_IDX ON %s (STATE, SEND_DATE)";
const char* NEW_AB_INDEX_STATEMENT_SQL        = "CREATE INDEX %s_AB_IDX ON %s (STATE, ABONENT)";
const char* RESET_MESSAGES_STATEMENT_SQL      = "UPDATE %s SET STATE=:NEW WHERE STATE=:WAIT";
const char* DO_STATE_MESSAGE_STATEMENT_SQL    = "UPDATE %s SET STATE=:STATE WHERE ID=:ID";
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

const char* DROP_INFOSME_T_INDEX_STATEMENT_SQL = "DROP INDEX ON %s";

Task::Task(ConfigView* config, std::string taskId, std::string tablePrefix, 
           DataSource* _dsOwn, DataSource* _dsInt)
    : logger(Logger::getInstance("smsc.infosme.Task")), formatter(0),
      usersCount(0), bFinalizing(false), bSelectedAll(false), dsOwn(_dsOwn), dsInt(_dsInt), 
      bInProcess(false), bInGeneration(false), bGenerationSuccess(false),
      infoSme_T_storageWasDestroyed(false), lastMessagesCacheEmpty(0), currentPriorityFrameCounter(0)
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
        
        std::string delNewMessagesId(prepareSqlCall(DELETE_NEW_MESSAGES_STATEMENT_ID));
        dsInt->closeRegisteredQueries(delNewMessagesId.c_str());
        std::string newMessageId(prepareSqlCall(NEW_MESSAGE_STATEMENT_ID));
        dsInt->closeRegisteredQueries(newMessageId.c_str());
        std::string clearMessagesId(prepareSqlCall(CLEAR_MESSAGES_STATEMENT_ID));
        dsInt->closeRegisteredQueries(clearMessagesId.c_str());
        std::string resetMessagesId(prepareSqlCall(RESET_MESSAGES_STATEMENT_ID));
        dsInt->closeRegisteredQueries(resetMessagesId.c_str());
        std::string retryMessageId(prepareSqlCall(DO_RETRY_MESSAGE_STATEMENT_ID));
        dsInt->closeRegisteredQueries(retryMessageId.c_str());
        std::string deleteMessageId(prepareSqlCall(DO_DELETE_MESSAGE_STATEMENT_ID));
        dsInt->closeRegisteredQueries(deleteMessageId.c_str());
        std::string enrouteMessageId(prepareSqlCall(DO_ENROUTE_MESSAGE_STATEMENT_ID));
        dsInt->closeRegisteredQueries(enrouteMessageId.c_str());
        std::string selectMessageId(prepareSqlCall(SELECT_MESSAGES_STATEMENT_ID));
        dsInt->closeRegisteredQueries(selectMessageId.c_str());
        std::string stateMessageId(prepareSqlCall(DO_STATE_MESSAGE_STATEMENT_ID));
        dsInt->closeRegisteredQueries(stateMessageId.c_str());
    }
    if (dsOwn)
    {
        std::string userQueryId(prepareSqlCall(USER_QUERY_STATEMENT_ID));
        dsOwn->closeRegisteredQueries(userQueryId.c_str());
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
    try { info.address = config->getString("address"); }
    catch (...) { 
        smsc_log_warn(logger, "<address> parameter missed for task '%s'. "
                              "Using global definitions", info.id.c_str());
        info.address = "";
    }
    info.retryOnFail = config->getBool("retryOnFail");
    info.replaceIfPresent = config->getBool("replaceMessage");
    info.transactionMode = config->getBool("transactionMode");
    info.trackIntegrity = config->getBool("trackIntegrity");
    info.keepHistory = config->getBool("keepHistory");
    info.endDate = parseDateTime(config->getString("endDate"));
    if (info.endDate>0 && time(0)>=info.endDate) {
      // preload InfoSme_T_ storage without index building
      Connection* connection = 0;
      connection = dsInt->getConnection();

      std::string createTableSql(prepareSqlCall(NEW_TABLE_STATEMENT_SQL));
      std::auto_ptr<Statement> statementGuard(connection->createStatement(createTableSql.c_str()));
      Statement* statement = statementGuard.get();
      if (!statement) 
        throw Exception("Failed to create table statement.");
      statement->execute();  

      // and set flag in order to don't try destroy storage needed only for queries execution
      infoSme_T_storageWasDestroyed = true;
    }

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
    
    const char* awd = 0;
    try { awd = config->getString("activeWeekDays"); } 
    catch (...) { 
        smsc_log_warn(logger, "<activeWeekDays> parameter missed for task '%s'. "
                              "Using default: Mon,Tue,Wed,Thu,Fri", info.id.c_str());
        info.activeWeekDays.weekDays = 0x7c; awd = 0;
    }
    if (awd && awd[0]) {
        if (!info.activeWeekDays.setWeekDays(awd))
            throw ConfigException("Task active week days set listed incorrectly."); 
    }
    else info.activeWeekDays.weekDays = 0;

    if (dsOwn != 0)
    {
        const char* query_sql = config->getString("query");
        if (!query_sql || query_sql[0] == '\0')
            throw ConfigException("Sql query for task empty or wasn't specified.");
        info.querySql = query_sql;
        const char* msg_template = config->getString("template");
        if (!msg_template || msg_template[0] == '\0')
            throw ConfigException("Message template for task empty or wasn't specified.");
        info.msgTemplate = msg_template;
    }
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
    try { 
      bGenerationSuccess = config->getBool("messagesHaveLoaded");
      bInGeneration = false;
    } catch (...) {}
}

void Task::doFinalization()
{
  smsc_log_error(logger, "Task::doFinalization::: taskId=%s",getId().c_str());
    {
        MutexGuard guard(finalizingLock);
        bFinalizing = true;
    }
    endGeneration();

    while (true) {
        usersCountEvent.Wait(10);
        MutexGuard guard(usersCountLock);
        if (usersCount <= 0) break;
    }
}
void Task::finalize()
{
    doFinalization();
    delete this;
}
bool Task::destroy()
{
  smsc_log_error(logger, "Task::destroy::: taskId=%s",getId().c_str());
    doFinalization();
    bool result = false;
    try { dropTable(); result = true;
    } catch (std::exception& exc) {
        smsc_log_error(logger, "Drop table failed for task '%s'. Reason: %s",
                       info.id.c_str(), exc.what());
    } catch (...) {
        smsc_log_error(logger, "Drop table failed for task '%s'. Reason is unknown",
                       info.id.c_str());
    }
    delete this;
    return result;
}
bool Task::shutdown()
{
  smsc_log_error(logger, "Task::shutdown::: taskId=%s",getId().c_str());
    doFinalization(); bool result = false;
    try { resetWaiting(); result = true;
    } catch (std::exception& exc) {
        smsc_log_error(logger, "Reset waiting failed for task '%s'. Reason: %s",
                       info.id.c_str(), exc.what());
    } catch (...) {
        smsc_log_error(logger, "Reset waiting failed for task '%s'. Reason is unknown",
                       info.id.c_str());
    }
    delete this;
    return result;
}

std::string Task::prepareSqlCall(const char* sql)
{
    if (!sql || sql[0] == '\0') return 0;
    std::string tableName = info.tablePrefix+info.id;
    char* sqlCall = new char[strlen(sql)+tableName.length()+1];
    sprintf(sqlCall, sql, tableName.c_str());
    std::string sqlStatement(sqlCall);
    delete [] sqlCall;
    return sqlStatement;
}
std::string Task::prepareDoubleSqlCall(const char* sql)
{
    if (!sql || sql[0] == '\0') return 0;
    std::string tableName = info.tablePrefix+info.id;
    char* sqlCall = new char[strlen(sql)+tableName.length()*2+1];
    sprintf(sqlCall, sql, tableName.c_str(), tableName.c_str());
    std::string sqlStatement(sqlCall);
    delete [] sqlCall;
    return sqlStatement;
}

void Task::trackIntegrity(bool clear, bool del, Connection* connection)
{
    smsc_log_debug(logger, "trackIntegrity method being called on task '%s'",
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
            std::auto_ptr<Statement> delGenerating(connection->createStatement(DELETE_GENERATING_STATEMENT_SQL));
            if (!delGenerating.get()) 
                throw Exception("Failed to obtain statement for track integrity.");
            
            delGenerating->setString(1, info.id.c_str());
            if (delGenerating->executeUpdate() > 0 && del)
            {
                std::string delGeneratedId(prepareSqlCall(DELETE_NEW_MESSAGES_STATEMENT_ID));
                std::string delGeneratedSql(prepareSqlCall(DELETE_NEW_MESSAGES_STATEMENT_SQL));
                std::auto_ptr<Statement> delGenerated(connection->createStatement(delGeneratedSql.c_str()));
                if (!delGenerated.get()) 
                    throw Exception("Failed to obtain statement for track integrity.");
                delGenerated->setUint8(1, MESSAGE_NEW_STATE);
                delGenerated->executeUpdate();
            }
        }
        else
        {
            std::auto_ptr<Statement> insGenerating(connection->createStatement(INSERT_GENERATING_STATEMENT_SQL));
            if (!insGenerating.get()) 
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
    smsc_log_debug(logger, "createTable method being called on task '%s'", info.id.c_str());
    
    MutexGuard guard(createTableLock);
    
    Connection* connection = 0;
    try
    {
        connection = dsInt->getConnection();
        if (!connection)
            throw Exception("Failed to obtain connection to internal data source.");

        {
            std::string createTableSql(prepareSqlCall(NEW_TABLE_STATEMENT_SQL));
            std::auto_ptr<Statement> statementGuard(connection->createStatement(createTableSql.c_str()));
            Statement* statement = statementGuard.get();
            if (!statement) 
                throw Exception("Failed to create table statement.");
            statement->execute();  
        } 
        {
            std::string createIndexSql(prepareDoubleSqlCall(NEW_SD_INDEX_STATEMENT_SQL));
            std::auto_ptr<Statement> statementGuard(connection->createStatement(createIndexSql.c_str()));
            Statement* statement = statementGuard.get();
            if (!statement) 
                throw Exception("Failed to create index1 statement.");
            statement->execute();
        }
        {
            std::string createIndexSql(prepareDoubleSqlCall(NEW_AB_INDEX_STATEMENT_SQL));
            std::auto_ptr<Statement> statementGuard(connection->createStatement(createIndexSql.c_str()));
            Statement* statement = statementGuard.get();
            if (!statement) 
                throw Exception("Failed to create index2 statement.");
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
    smsc_log_debug(logger, "dropTable method being called on task '%s'", info.id.c_str());
    
    MutexGuard guard(createTableLock);
    
    Connection* connection = 0;
    try
    {
        connection = dsInt->getConnection();
        if (!connection)
            throw Exception("Failed to obtain connection to internal data source.");

        trackIntegrity(true, false, connection); // delete flag only

        std::string dropTableSql(prepareSqlCall(DROP_TABLE_STATEMENT_SQL));
        std::auto_ptr<Statement> statementGuard(connection->createStatement(dropTableSql.c_str()));
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
    smsc_log_debug(logger, "beginGeneration method being called on task '%s'",
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

        std::auto_ptr<Statement> userQuery(ownConnection->createStatement(info.querySql.c_str()));
        if (!userQuery.get())
            throw Exception("Failed to create user query statement on own data source.");

        std::string newMessageSql(prepareSqlCall(NEW_MESSAGE_STATEMENT_SQL));
        std::auto_ptr<Statement> newMessage(intConnection->createStatement(newMessageSql.c_str()));
        if (!newMessage.get())
            throw Exception("Failed to create statement for message generation.");

        std::auto_ptr<Statement> clearMessages;
        if (info.replaceIfPresent) {
            std::string clearMessagesSql(prepareSqlCall(CLEAR_MESSAGES_STATEMENT_SQL));
            clearMessages.reset(intConnection->createStatement(clearMessagesSql.c_str()));
            if (!clearMessages.get())
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
                    if (clearMessages.get()) {
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
    smsc_log_debug(logger, "endGeneration method being called on task '%s'",
                 info.id.c_str());
    {
        MutexGuard guard(inGenerationLock);
        if (!bInGeneration) return;
        bInGeneration = false;
    }
    generationEndEvent.Wait();
}

void Task::dropNewMessages(Statistics* statistics)
{
    smsc_log_debug(logger, "dropAllMessages method being called on task '%s'",
                   info.id.c_str());

    endGeneration();
    
    Connection* connection = 0;
    int wdTimerId = -1;
    try
    {
        connection = dsInt->getConnection();
        if (!connection)
            throw Exception("Failed to obtain connection to internal data source.");

        std::string deleteMessagesSql(prepareSqlCall(DELETE_NEW_MESSAGES_STATEMENT_SQL));
        std::auto_ptr<Statement> deleteMessages(connection->createStatement(deleteMessagesSql.c_str()));
        if (!deleteMessages.get())
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
    smsc_log_debug(logger, "resetWaiting method being called on task '%s'",
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
        
        std::string resetMessagesSql(prepareSqlCall(RESET_MESSAGES_STATEMENT_SQL));
        std::auto_ptr<Statement> resetMessages(connection->createStatement(resetMessagesSql.c_str()));
        if (!resetMessages.get())
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
    smsc_log_debug(logger, "retryMessage method being called on task '%s' for id=%lld",
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

        std::string retryMessageSql(prepareSqlCall(DO_RETRY_MESSAGE_STATEMENT_SQL));
        std::auto_ptr<Statement> retryMessage(connection->createStatement(retryMessageSql.c_str()));
        if (!retryMessage.get())
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

bool Task::finalizeMessage(uint64_t msgId, MessageState state, Connection* connection)
{   
    if (state == NEW || state == WAIT || state == ENROUTE) {
        smsc_log_warn(logger, "Invalid state=%d to finalize message on task '%s' for id=%lld",
                      state, info.id.c_str(), msgId);
        return false;
    } else {
        smsc_log_debug(logger, "finalizeMessage(%d) method being called on task '%s' for id=%lld",
                       state, info.id.c_str(), msgId);
    }

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
            throw Exception("finalizeMessage(): Failed to obtain connection");

        std::auto_ptr<Statement> finalizeMessage;
        if (info.keepHistory)
        {
            // Was DO_STATE_MESSAGE_STATEMENT
            std::string changeStateSql(prepareSqlCall(DO_RETRY_MESSAGE_STATEMENT_SQL));
            finalizeMessage.reset(connection->createStatement(changeStateSql.c_str()));
            if (finalizeMessage.get()) {
                finalizeMessage->setUint8   (1, state);
                finalizeMessage->setDateTime(2, time(NULL));
                finalizeMessage->setUint64  (3, msgId);
            }
        }
        else
        {
            std::string deleteMessageSql(prepareSqlCall(DO_DELETE_MESSAGE_STATEMENT_SQL));
            finalizeMessage.reset(connection->createStatement(deleteMessageSql.c_str()));
            if (finalizeMessage.get()) finalizeMessage->setUint64(1, msgId);
        }
        
        if (!finalizeMessage.get())
            throw Exception("finalizeMessage(): Failed to create statement for messages access.");
        
        wdTimerId = dsInt->startTimer(connection, info.dsTimeout);
        result = (finalizeMessage->executeUpdate() > 0);
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
        smsc_log_error(logger, "Task '%s'. finalizeMessage(): Messages access failure. "
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
        smsc_log_error(logger, "Task '%s'. finalizeMessage(): Messages access failure.", 
                     info.id.c_str());
    }
    
    dsInt->stopTimer(wdTimerId);
    if (connectionInternal && connection) dsInt->freeConnection(connection);
    return result;
}

bool Task::enrouteMessage(uint64_t msgId, Connection* connection)
{
    smsc_log_debug(logger, "enrouteMessage method being called on task '%s' for id=%lld",
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
            throw Exception("enrouteMessage(): Failed to obtain connection");
        
        std::string enrouteMessageSql(prepareSqlCall(DO_ENROUTE_MESSAGE_STATEMENT_SQL));
        std::auto_ptr<Statement> enrouteMessage(connection->createStatement(enrouteMessageSql.c_str()));
        if (!enrouteMessage.get())
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

void
Task::putToSuspendedMessagesQueue(const Message& suspendedMessage)
{
  smsc_log_info(logger, "Task::putToSuspendedMessagesQueue:::  task '%s': exceeded region bandwidth (regionId=%s), return message with id=%d to messagesCache",
                info.id.c_str(), suspendedMessage.regionId.c_str(), suspendedMessage.id);
  MutexGuard guard(messagesCacheLock);
  _suspendedRegions.insert(suspendedMessage.regionId);
  messagesCache.push_back(suspendedMessage);
}

bool
Task::fetchMessageFromCache(Message& message)
{
  MutexGuard guard(messagesCacheLock);
  std::string regionForPreviouslyFetchedMessage;
  while(_messagesCacheIter != messagesCache.end() ) {
    const Message& fetchedMessage = *_messagesCacheIter;
    if ( regionForPreviouslyFetchedMessage != fetchedMessage.regionId && 
         _suspendedRegions.find(fetchedMessage.regionId) == _suspendedRegions.end() ) {
      message = fetchedMessage; setInProcess(isEnabled());
      _messagesCacheIter = messagesCache.erase(_messagesCacheIter);
      smsc_log_debug(logger, "Task::fetchMessageFromCache::: task '%s', fetched next message [id=%ld,abonent=%s,regionId=%s]",info.id.c_str(), message.id, message.abonent.c_str(), message.regionId.c_str());
      return true;
    }
    ++_messagesCacheIter;
    regionForPreviouslyFetchedMessage = fetchedMessage.regionId;
  }
  smsc_log_debug(logger, "Task::fetchMessageFromCache::: cache bypassed, there are no messages meeting search criterion; messagesCache.size = %d", messagesCache.size());
  return false;
}

void
Task::resetSuspendedRegions()
{
  MutexGuard guard(messagesCacheLock);
  smsc_log_debug(logger, "Task::resetSuspendedRegions::: method being called on task '%s'",info.id.c_str());
  _messagesCacheIter = messagesCache.begin();
  _suspendedRegions.clear();
}

bool Task::getNextMessage(Connection* connection, Message& message)
{
    smsc_log_debug(logger, "getNextMessage method being called on task '%s'",
                   info.id.c_str());
    __require__(connection);

    if (!isEnabled())
      return setInProcess(false);

    // selecting from cache
    if ( fetchMessageFromCache(message) )
      return true;

    // Cache is empty here or maybe we have bypassed all cache elements but there are ones with region id values for which suspended condition is true

    if (info.trackIntegrity && !isGenerationSucceeded())
      return setInProcess(false); // for track integrity check that generation finished ok

    time_t currentTime = time(NULL);
    size_t messagesCacheSz;
    {
      MutexGuard guard(messagesCacheLock);
      messagesCacheSz = messagesCache.size();
    }

    if ( messagesCacheSz > 0 ) return false;
    else if (currentTime-lastMessagesCacheEmpty > info.messagesCacheSleep)
      lastMessagesCacheEmpty = currentTime;   // timeout reached, set new sleep timeout & go to fill cache
    else if (bSelectedAll && !isInGeneration()) 
      return setInProcess(false);             // if all selected from DB (on last time) & no generation => return
    smsc_log_info(logger, "Selecting messages from DB. getNextMessage method on task '%s'",
                  info.id.c_str());

    // Selecting cache from DB
    int wdTimerId = -1;
    try
    {
        std::string selectMessageSql(prepareSqlCall(SELECT_MESSAGES_STATEMENT_SQL));
        std::auto_ptr<Statement> selectMessage(connection->createStatement(selectMessageSql.c_str()));
        if (!selectMessage.get())
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
            uint64_t    msgId = rs->getUint64(1);
            const char* msgAbonent = rs->isNull(2) ? 0 : rs->getString(2);
            const char* msgMessage = rs->isNull(3) ? 0 : rs->getString(3);
            const char* regionId = rs->getString(4);

            dsInt->stopTimer(wdTimerId);
            
            if (!msgAbonent || !msgMessage) {
                smsc_log_warn(logger, "Selected NULL value for %s", !msgAbonent ? "abonent":"message");
                wdTimerId = dsInt->startTimer(connection, info.dsTimeout);
                --fetched; continue;
            }
            Message fetchedMessage(msgId, msgAbonent, msgMessage, regionId);

            std::string waitMessageSql(prepareSqlCall(DO_STATE_MESSAGE_STATEMENT_SQL));
            std::auto_ptr<Statement> waitMessage(connection->createStatement(waitMessageSql.c_str()));
            if (!waitMessage.get())
                throw Exception("Failed to create statement for messages access.");
            
            wdTimerId = dsInt->startTimer(connection, info.dsTimeout);
            waitMessage->setUint8 (1, MESSAGE_WAIT_STATE);
            waitMessage->setUint64(2, fetchedMessage.id);
            if (waitMessage->executeUpdate() <= 0)
                smsc_log_warn(logger, "Failed to update message in getNextMessage() !!!");
                // TODO: analyse it !
            {
                MutexGuard guard(messagesCacheLock);
                messagesCache.push_back(fetchedMessage);
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
    smsc_log_debug(logger, "Selected %d messages from DB for task '%s'",
                   messagesCache.size(), info.id.c_str());

    // selecting from cache
    if ( fetchMessageFromCache(message) )
      return true;
    else {
      lastMessagesCacheEmpty = time(NULL);
      bSelectedAll = true;
      return setInProcess(false);
    }
}

bool Task::isReady(time_t time, bool checkActivePeriod)
{
  if ( !isEnabled() || isFinalizing() || 
       (info.endDate>0 && time>=info.endDate) ||
       (info.validityDate>0 && time>=info.validityDate) ) { 
    smsc_log_debug(logger, "Task::isReady::: task id =%s,info.endDate=%ld,info.validityDate=%ld,time()=%ld",info.id.c_str(),info.endDate,info.validityDate,::time(0));
    if ( !infoSme_T_storageWasDestroyed && !isInProcess() &&
         ((info.endDate>0 && time>=info.endDate) ||
          (info.validityDate>0 && time>=info.validityDate)) ) {
      destroy_InfoSme_T_Storage();
      infoSme_T_storageWasDestroyed = true;
    }
    return false;
  }

    if (checkActivePeriod) 
    {
        tm dt; localtime_r(&time, &dt);

        if (!info.activeWeekDays.isWeekDay((dt.tm_wday == 0) ? 6:(dt.tm_wday-1)))
            return false;

        if (info.activePeriodStart > 0 && info.activePeriodEnd > 0)
        {
            if (info.activePeriodStart > info.activePeriodEnd) return false;

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
    }
    return true;
}

bool Task::insertDeliveryMessage(uint8_t msgState,
                                 const std::string& address,
                                 time_t messageDate,
                                 const std::string& msg)
{
  Connection* intConnection = dsInt->getConnection();
  if (!intConnection)
    throw Exception("Failed to obtain connection to internal data source.");

  std::string newMessageSql(prepareSqlCall(NEW_MESSAGE_STATEMENT_SQL));
  std::auto_ptr<Statement> newMessage(intConnection->createStatement(newMessageSql.c_str()));
  if (!newMessage.get())
    throw Exception("Failed to create statement for message generation.");

  smsc::sms::Address parsedAddr(address.c_str());
  smsc_log_debug(logger, "Task::insertDeliveryMessage::: try map telephone number [%s] to Region", address.c_str());
  const smsc::util::config::region::Region* foundRegion = smsc::util::config::region::RegionFinder::getInstance().findRegionByAddress(parsedAddr.toString());
  if ( foundRegion )
    smsc_log_debug(logger, "Task::insertDeliveryMessage::: telephone number = %s matches to mask for region with id %s", address.c_str(), foundRegion->getId().c_str());
  else
    throw Exception("Task::insertDeliveryMessage::: Wrong configuraiton - can't find region definition");

  if (msg.length() > 0)
  {
    newMessage->setUint8(1, MESSAGE_NEW_STATE);
    newMessage->setString(2, address.c_str());
    newMessage->setDateTime(3, messageDate);
    newMessage->setString(4, msg.c_str());
    newMessage->setString(5, foundRegion->getId().c_str());

    newMessage->executeUpdate();
  }
  intConnection->commit();

  return true;
}
bool Task::changeDeliveryMessageInfoByRecordId(uint8_t msgState,
                                               time_t unixTime,
                                               const std::string& recordId)
{
  uint64_t msgId = atol(recordId.c_str());
  smsc_log_debug(logger, "Task::changeDeliveryMessageInfoByRecordId method being called on task '%s' for id=%lld",
                 info.id.c_str(), msgId);

  try
  {
    Connection* intConnection = dsInt->getConnection();
    if (!intConnection)
      throw Exception("Failed to obtain connection to internal data source.");

    std::string updateMessageSql(prepareSqlCall(DO_RETRY_MESSAGE_STATEMENT_SQL));
    std::auto_ptr<Statement> updateMessage(intConnection->createStatement(updateMessageSql.c_str()));
    if (!updateMessage.get())
      throw Exception("changeDeliveryMessageInfoByRecordId(): Failed to create statement for messages access.");

    updateMessage->setUint8   (1, msgState);
    updateMessage->setDateTime(2, unixTime); 
    updateMessage->setUint64  (3, msgId);
        
    updateMessage->executeUpdate();

    return true;
  }
  catch (Exception& exc) {
    smsc_log_error(logger, "Task '%s'. changeDeliveryMessageInfoByRecordId(): Messages access failure. "
                   "Details: %s", info.id.c_str(), exc.what());
  }
  catch (...) {
    smsc_log_error(logger, "Task '%s'. changeDeliveryMessageInfoByRecordId(): Messages access failure.", 
                   info.id.c_str());
  }
    
  return false;
}

static const char* DO_FULL_TABLESCAN_DELIVERYMESSAGE_STATEMENT_ID  = "%s_FULL_TABLE_SCAN_ID";
static const char* DO_FULL_TABLESCAN_DELIVERYMESSAGE_STATEMENT_SQL = "FULL_TABLE_SCAN FROM %s";

bool Task::changeDeliveryMessageInfoByCompositCriterion(uint8_t msgState,
                                                        time_t unixTime,
                                                        const InfoSme_T_SearchCriterion& searchCrit)
{
  smsc_log_debug(logger, "Task::changeDeliveryMessageInfoByCompositCriterion method being called on task '%s'",
                 info.id.c_str());

  Connection* intConnection = dsInt->getConnection();
  if (!intConnection)
    throw Exception("Failed to obtain connection to internal data source.");

  std::string fullTableScanMessageSql(prepareSqlCall(DO_FULL_TABLESCAN_DELIVERYMESSAGE_STATEMENT_SQL));
  std::auto_ptr<Statement> selectMessage(intConnection->createStatement(fullTableScanMessageSql.c_str()));
  if (!selectMessage.get())
    throw Exception("changeDeliveryMessageInfoByCompositCriterion(): Failed to create statement for messages access.");

  std::auto_ptr<ResultSet> rsGuard(selectMessage->executeQuery());

  ResultSet* rs = rsGuard.get();
  if (!rs)
    throw Exception("Failed to obtain result set for message access.");
        
  int fetched = 0, numOfRowsProcessed=0;
  while (rs->fetchNext()) {
    smsc_log_debug(logger, "Task::changeDeliveryMessageInfoByCompositCriterion::: check next record");
    if ( doesMessageConformToCriterion(rs, searchCrit) ) {
      char recordId[32];
      sprintf(recordId, "%lld", rs->getUint64(1));

      changeDeliveryMessageInfoByRecordId(msgState,
                                          unixTime,
                                          recordId);
    }
    if ( ++numOfRowsProcessed % 5000 == 0 )
      smsc::util::millisleep(1);
    if ( numOfRowsProcessed % 100 == 0 )
      --unixTime;
  }
  return true;
}

bool Task::doesMessageConformToCriterion(ResultSet* rs, const InfoSme_T_SearchCriterion& searchCrit)
{
  if ( searchCrit.isSetAbonentAddress() &&
       searchCrit.getAbonentAddress() != rs->getString(3) ) {
    smsc_log_debug(logger, "Task::doesMessageConformToCriterion::: searchCrit.getAbonentAddress()=%s != rs->getString(3)=%s", searchCrit.getAbonentAddress().c_str(), rs->getString(3));
    return false;
  }
  if ( searchCrit.isSetState() &&
       searchCrit.getState() != rs->getUint8(2) ) {
    smsc_log_debug(logger, "Task::doesMessageConformToCriterion::: searchCrit.getState=%d != rs->getUint8(2)=%d", searchCrit.getState(), rs->getUint8(2));
    return false;
  }
  if ( searchCrit.isSetFromDate() && 
       searchCrit.getFromDate() > rs->getDateTime(4) ) {
    smsc_log_debug(logger, "Task::doesMessageConformToCriterion::: searchCrit.getFromDate()=%ld > rs->getDateTime(4)=%ld", searchCrit.getFromDate(), rs->getDateTime(4));
    return false;
  }
  if ( searchCrit.isSetToDate() && 
       searchCrit.getToDate() < rs->getDateTime(4) ) {
    smsc_log_debug(logger, "Task::doesMessageConformToCriterion::: searchCrit.getToDate()=%ld < rs->getDateTime(4)=%ld", searchCrit.getToDate(), rs->getDateTime(4));
    return false;
  }
  return true;
}

bool Task::deleteDeliveryMessageByRecordId(const std::string& recordId)
{
  smsc_log_debug(logger, "Task::deleteDeliveryMessageByRecordId method being called on task '%s'",
                 info.id.c_str());

  Connection* intConnection = dsInt->getConnection();
  if (!intConnection)
    throw Exception("Failed to obtain connection to internal data source.");

  std::string deleteMessageSql(prepareSqlCall(DO_DELETE_MESSAGE_STATEMENT_SQL));
  std::auto_ptr<Statement> deleteMessage(intConnection->createStatement(deleteMessageSql.c_str()));

  if (!deleteMessage.get())
    throw Exception("deleteDeliveryMessageByRecordId(): Failed to create statement for messages access.");
  uint64_t msgId = atol(recordId.c_str());
  smsc_log_debug(logger, "Task::deleteDeliveryMessageByRecordId::: call deleteMessage->executeUpdate for msgId=%ld",msgId);
  deleteMessage->setUint64(1, msgId);
  deleteMessage->executeUpdate();
  return true;
}

bool Task::deleteDeliveryMessagesByCompositCriterion(const InfoSme_T_SearchCriterion& searchCrit)
{
  smsc_log_debug(logger, "Task::deleteDeliveryMessagesByCompositCriterion method being called on task '%s'",
                 info.id.c_str());

  Connection* intConnection = dsInt->getConnection();
  if (!intConnection)
    throw Exception("Failed to obtain connection to internal data source.");

  std::string fullTableScanMessageSql(prepareSqlCall(DO_FULL_TABLESCAN_DELIVERYMESSAGE_STATEMENT_SQL));
  std::auto_ptr<Statement> selectMessage(intConnection->createStatement(fullTableScanMessageSql.c_str()));
  if (!selectMessage.get())
    throw Exception("deleteDeliveryMessagesByCompositCriterion(): Failed to create statement for messages access.");

  std::auto_ptr<ResultSet> rsGuard(selectMessage->executeQuery());

  ResultSet* rs = rsGuard.get();
  if (!rs)
    throw Exception("Failed to obtain result set for message access.");
        
  int fetched = 0, numOfRowsProcessed=0;
  smsc_log_debug(logger, "Task::deleteDeliveryMessagesByCompositCriterion::: search records matching criterion");
  while (rs->fetchNext()) {
    smsc_log_debug(logger, "Task::deleteDeliveryMessagesByCompositCriterion::: check next record");
    if ( doesMessageConformToCriterion(rs, searchCrit) ) {
      char recordId[32];
      sprintf(recordId, "%lld", rs->getUint64(1));
      smsc_log_debug(logger, "Task::deleteDeliveryMessagesByCompositCriterion: record found[recordId=%s]",recordId);
      deleteDeliveryMessageByRecordId(recordId);
    }
    if ( ++numOfRowsProcessed % 5000 == 0 )
      smsc::util::millisleep(1);
  }
  return true;

}

struct MessageDescription {
  MessageDescription(uint64_t anId, uint8_t aState, 
                     const std::string& anAbonentAddress,
                     time_t aSendDate,
                     const std::string& aMessage)
    : id(anId), state(aState), abonentAddress(anAbonentAddress), sendDate(aSendDate), message(aMessage){}

  std::string toString() {
    std::ostringstream obuf;
    obuf << "id=[" << id 
         << "],state=[" << uint_t(state)
         << "],abonentAddress=[" << abonentAddress
         << "],sendDate=[" << sendDate
         << "],message=[" << message 
         << "]";
    return obuf.str();
  }
  uint64_t id;
  uint8_t state;
  std::string abonentAddress;
  time_t sendDate;
  std::string message;
};

static bool orderByState(MessageDescription& lhs, MessageDescription& rhs)
{
  if ( lhs.state < rhs.state ) return true;
  else return false;
}

static bool reverseOrderByState(MessageDescription& lhs, MessageDescription& rhs)
{  
  if ( lhs.state > rhs.state ) return true;
  else return false;
}

static bool orderByAbonent(MessageDescription& lhs, MessageDescription& rhs)
{
  if ( lhs.abonentAddress < rhs.abonentAddress ) return true;
  else return false;
}

static bool reverseOrderByAbonent(MessageDescription& lhs, MessageDescription& rhs)
{
  if ( lhs.abonentAddress > rhs.abonentAddress ) return true;
  else return false;
}

static bool orderBySendDate(MessageDescription& lhs, MessageDescription& rhs)
{
  if ( lhs.sendDate < rhs.sendDate ) return true;
  else return false;
}

static bool reverseOrderBySendDate(MessageDescription& lhs, MessageDescription& rhs)
{
  if ( lhs.sendDate > rhs.sendDate ) return true;
  else return false;
}

Array<std::string>
Task::selectDeliveryMessagesByCompositCriterion(const InfoSme_T_SearchCriterion& searchCrit)
{
  smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion method being called on task '%s'",
                 info.id.c_str());

  Connection* intConnection = dsInt->getConnection();
  if (!intConnection)
    throw Exception("Failed to obtain connection to internal data source.");

  std::string fullTableScanMessageSql(prepareSqlCall(DO_FULL_TABLESCAN_DELIVERYMESSAGE_STATEMENT_SQL));
  std::auto_ptr<Statement> selectMessage(intConnection->createStatement(fullTableScanMessageSql.c_str()));
  if (!selectMessage.get())
    throw Exception("selectDeliveryMessagesByCompositCriterion(): Failed to create statement for messages access.");

  std::auto_ptr<ResultSet> rsGuard(selectMessage->executeQuery());
  ResultSet* rs = rsGuard.get();
  if (!rs)
    throw Exception("Failed to obtain result set for message access.");

  typedef std::list<MessageDescription> MessagesList_t;
  MessagesList_t messagesList;

  int fetched = 0;
  size_t msgLimit=0;
  if ( searchCrit.isSetMsgLimit() )
    msgLimit = searchCrit.getMsgLimit();

  size_t fetchedCount=0, numOfRowsProcessed=0;
  while (rs->fetchNext()) {
    if ( doesMessageConformToCriterion(rs, searchCrit) ) {
      if ( msgLimit && fetchedCount++ >= msgLimit ) continue;
      smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion::: add message [%lld,%d,%s,%lld,%s] into messagesList", rs->getUint64(1), rs->getUint8(2), rs->getString(3), rs->getDateTime(4), rs->getString(5));
      messagesList.push_back(MessageDescription(rs->getUint64(1),
                                                rs->getUint8(2),
                                                rs->getString(3),
                                                rs->getDateTime(4),
                                                rs->getString(5)));
    }
    if ( ++numOfRowsProcessed % 5000 == 0 )
      smsc::util::millisleep(1);
  }

  if ( searchCrit.isSetOrderByCriterion() ) {
    smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion::: try sort taskMessages");
    bool (*predicate)(MessageDescription& lhs, MessageDescription& rhs) = NULL;

    if ( searchCrit.getOrderByCriterion() == "state" ) {
      if ( searchCrit.isSetOrderDirection() && searchCrit.getOrderDirection() == "desc" )
        predicate = reverseOrderByState;
      else
        predicate = orderByState;
    } else if ( searchCrit.getOrderByCriterion() == "abonent" ) {
      if ( searchCrit.isSetOrderDirection() && searchCrit.getOrderDirection() == "desc" )
        predicate = reverseOrderByAbonent;
      else
        predicate = orderByAbonent;
    } else if ( searchCrit.getOrderByCriterion() == "send_date" ) {
      if ( searchCrit.isSetOrderDirection() && searchCrit.getOrderDirection() == "desc" )
        predicate = reverseOrderBySendDate;
      else
        predicate = orderBySendDate;
    }
    if ( predicate ) 
      messagesList.sort(predicate);
  }
  smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion:::  prepare taskMessages");
  Array<std::string> taskMessages;
  for(MessagesList_t::iterator iter=messagesList.begin(); iter!=messagesList.end(); ++iter) {
    std::ostringstream messageBuf;
    messageBuf << iter->id << "|"
               << uint_t(iter->state) << "|"
               << iter->abonentAddress << "|"
               << unixTimeToStringFormat(iter->sendDate) << "|"
               << iter->message;
    smsc_log_debug(logger, "Task::selectDeliveryMessagesByCompositCriterion::: add message=[%s] to taskMessages", messageBuf.str().c_str());
    taskMessages.Push(messageBuf.str());
  }
  std::ostringstream fetchedCountAsStr;
  fetchedCountAsStr << fetchedCount;
  taskMessages.Push(fetchedCountAsStr.str());
  return taskMessages;
}

void
Task::endDeliveryMessagesGeneration()
{
  smsc_log_debug(logger, "Task::endDeliveryMessagesGeneration method being called on task '%s'",
                 info.id.c_str());

  MutexGuard guard(inGenerationLock);
  bInGeneration = false; bGenerationSuccess = true;
  /*  Manager& configManager = Manager::getInstance();

  configManager.setBool(("InfoSme.Tasks."+info.id+".taskLoaded").c_str(), true);
  configManager.save();*/
  generationEndEvent.Signal();
}

void
Task::destroy_InfoSme_T_Storage()
{
  smsc_log_debug(logger, "Task::destroy_InfoSme_T_Storage method being called on task '%s'", info.id.c_str());

  try {
    Connection* connection = 0;
    connection = dsInt->getConnection();

    std::string dropInfoSme_T_Idx(prepareSqlCall(DROP_INFOSME_T_INDEX_STATEMENT_SQL));
    std::auto_ptr<Statement> statementGuard(connection->createStatement(dropInfoSme_T_Idx.c_str()));
    Statement* statement = statementGuard.get();
    if (!statement) 
      throw Exception("can' create statement (null pointer value)");

    statement->execute();  
  } catch (std::exception& ex) {
    smsc_log_info(logger, "Task::destroy_InfoSme_T_Storage::: catch exception [%s] for task '%s'",
                  ex.what(), info.id.c_str());
  }
}

static const char * CHANGE_DELIVERY_MESSAGE_BY_ID_STATEMENT_SQL = "UPDATE %s SET MESSAGE = :MSG WHERE ID = :ID";

bool Task::changeDeliveryTextMessageByCompositCriterion(const std::string& newTextMsg,
                                                        const InfoSme_T_SearchCriterion& searchCrit)
{
  smsc_log_debug(logger, "Task::changeDeliveryTextMessageByCompositCriterion method being called on task '%s'",
                 info.id.c_str());

  Connection* intConnection = dsInt->getConnection();
  if (!intConnection)
    throw Exception("Failed to obtain connection to internal data source.");

  std::string fullTableScanMessageSql(prepareSqlCall(DO_FULL_TABLESCAN_DELIVERYMESSAGE_STATEMENT_SQL));
  std::auto_ptr<Statement> selectMessage(intConnection->createStatement(fullTableScanMessageSql.c_str()));
  if (!selectMessage.get())
    throw Exception("changeDeliveryMessageInfoByCompositCriterion(): Failed to create statement for messages access.");

  std::auto_ptr<ResultSet> rsGuard(selectMessage->executeQuery());

  ResultSet* rs = rsGuard.get();
  if (!rs)
    throw Exception("Failed to obtain result set for message access.");
        
  int fetched = 0, numOfRowsProcessed=0;
  while (rs->fetchNext()) {
    smsc_log_debug(logger, "Task::changeDeliveryTextMessageByCompositCriterion::: check next record");
    if ( doesMessageConformToCriterion(rs, searchCrit) ) {
      std::string updateMessageSql(prepareSqlCall(CHANGE_DELIVERY_MESSAGE_BY_ID_STATEMENT_SQL));
      std::auto_ptr<Statement> updateMessage(intConnection->createStatement(updateMessageSql.c_str()));
      if (!updateMessage.get())
        throw Exception("changeDeliveryMessageInfoByRecordId(): Failed to create statement for messages access.");

      updateMessage->setString  (1, newTextMsg.c_str());
      updateMessage->setUint64  (2, rs->getUint64(1));
        
      updateMessage->executeUpdate();
    }
    if ( ++numOfRowsProcessed % 5000 == 0 )
      smsc::util::millisleep(1);
  }
  return true;
}

}}
