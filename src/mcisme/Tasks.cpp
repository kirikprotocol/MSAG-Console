
#include <vector>
#include "util/regexp/RegExp.hpp"

#include "Tasks.h"

namespace smsc { namespace mcisme 
{

using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;

DataSource* Task::ds     = 0;
Logger*     Task::logger = 0;
Statistics* Task::statistics = 0;

uint64_t    Task::currentId  = 0;
uint64_t    Task::sequenceId = 0;
bool        Task::bInformAll   = false;
bool        Task::bNotifyAll   = false;
time_t      Task::validityTime = 0;
int         Task::maxCallersCount  = -1; // Disable distinct callers count check
int         Task::maxMessagesCount = -1; // Disable messages count for abonent check

bool        TimeOffsetManager::inited = false;

struct TimeRule
{
    RegExp exp;
    vector<SMatch> m;
    int n, offset;

    TimeRule(const char* name, const char* rx, int off, Logger* logger) : offset(off)
    {
      if (!exp.Compile(rx))
        smsc_log_error(logger, "Time: rule='%s' rx='%s' compiling failed", name, rx);
      m.resize(exp.GetBracketsCount());
      n = m.size();
    }
    int match(const char* abonent, int& off)
    {
        int res = exp.Match(abonent, &m[0], n);
        if (res) off = offset;
        return res;
    }
};
static vector<TimeRule *> timeRules;

void TimeOffsetManager::init(ConfigView* config)
{
    smsc::util::regexp::RegExp::InitLocale();

    if (config && !TimeOffsetManager::inited)
    {
        std::auto_ptr< std::set<std::string> > rulesSetGuard(config->getShortSectionNames());
        std::set<std::string>* rulesSet = rulesSetGuard.get();
        for (std::set<std::string>::iterator i=rulesSet->begin();i!=rulesSet->end();i++)
        {
            const char* ruleName = (const char *)i->c_str();
            if (!ruleName || !ruleName[0]) continue;

            std::auto_ptr<ConfigView> ruleCfgGuard(config->getSubConfig(ruleName));
            ConfigView* ruleCfg = ruleCfgGuard.get();

            const char* rx = ruleCfg->getString("regexp");
            int offset     = ruleCfg->getInt("offset");
            TimeRule* rule = new TimeRule(ruleName, rx, offset, Task::logger);
            timeRules.push_back(rule);
        }
        
        TimeOffsetManager::inited = true;
    }
}
int TimeOffsetManager::getOffset(const char* abonent)
{
    int offset = 0;
    if (TimeOffsetManager::inited)
    {
        vector<TimeRule *>::iterator it = timeRules.begin(), end = timeRules.end();
        for(;it != end;++it)
        {
            TimeRule* rule = *it; 
            if (rule->match(abonent, offset))
            {
              smsc_log_debug(Task::logger, "Time: matched abonent '%s' offset=%d", abonent, offset);
              break;
            }
        }
    }
    return offset;
}

/* ----------------------- Access to message ids generation (MCI_MSG_SEQ) -------------------- */

const char* GET_NEXT_SEQID_ID   = "GET_NEXT_SEQID_ID";
const char* GET_NEXT_SEQID_SQL  = "SELECT MCISME_MSG_SEQ.NEXTVAL FROM DUAL";

/* ----------------------- Access to current messages set (MCI_MSG_SET) ---------------------- */

const char* CREATE_NEW_MSG_ID   = "CREATE_NEW_MSG_ID";
const char* DELETE_ANY_MSG_ID   = "DELETE_ANY_MSG_ID";
const char* SELECT_OLD_MSG_ID   = "SELECT_OLD_MSG_ID";
const char* ROLLUP_CUR_MSG_ID   = "ROLLUP_CUR_MSG_ID";
const char* LOADUP_TO_SKIP_ID   = "LOADUP_TO_SKIP_ID";
const char* ROLLUP_TO_SKIP_ID   = "ROLLUP_TO_SKIP_ID";
const char* LOADUP_TASK_ST_ID   = "LOADUP_TASK_ST_ID";

const char* CREATE_NEW_MSG_SQL  = "INSERT INTO MCISME_MSG_SET (ID, STATE, ABONENT, SMSC_ID) "
                                  "VALUES (:ID, :STATE, :ABONENT, NULL)";
const char* DELETE_ANY_MSG_SQL  = "DELETE FROM MCISME_MSG_SET WHERE ID=:ID";
const char* SELECT_OLD_MSG_SQL  = "SELECT ID, STATE, ABONENT FROM MCISME_MSG_SET WHERE SMSC_ID=:SMSC_ID";
const char* ROLLUP_CUR_MSG_SQL  = "UPDATE MCISME_MSG_SET SET STATE=:STATE, SMSC_ID=:SMSC_ID WHERE ID=:ID";

const char* LOADUP_SKIPPED_SQL  = "SELECT ABONENT, ID, SMSC_ID FROM MCISME_MSG_SET "
                                  "WHERE STATE=:STATE ORDER BY ABONENT, ID";
const char* LOADUP_TO_SKIP_SQL  = "SELECT ID, SMSC_ID FROM MCISME_MSG_SET WHERE ABONENT=:ABONENT "
                                  "AND STATE=:STATE ORDER BY ID FOR UPDATE";
const char* ROLLUP_TO_SKIP_SQL  = "UPDATE MCISME_MSG_SET SET STATE=:STATE WHERE ID=:ID";

const char* LOADUP_ALL_CUR_SQL  = 
"SELECT MCISME_CUR_MSG.ABONENT, MCISME_CUR_MSG.ID, "
       "MCISME_ABONENTS.INFORM, MCISME_ABONENTS.NOTIFY, "
       "MCISME_ABONENTS.INFORM_ID, MCISME_ABONENTS.NOTIFY_ID, MCISME_ABONENTS.EVENT_MASK, "
       "MCISME_MSG_SET.STATE, MCISME_MSG_SET.SMSC_ID, "
       "MCISME_EVT_SET.ID, MCISME_EVT_SET.DT, MCISME_EVT_SET.CALLER, MCISME_EVT_SET.MSG_ID "
"FROM   MCISME_CUR_MSG, MCISME_ABONENTS, MCISME_MSG_SET, MCISME_EVT_SET "
"WHERE  MCISME_ABONENTS.ABONENT (+)= MCISME_CUR_MSG.ABONENT "
   "AND MCISME_MSG_SET.ID = MCISME_CUR_MSG.ID "
   "AND MCISME_EVT_SET.ABONENT = MCISME_CUR_MSG.ABONENT "
   "AND (MCISME_EVT_SET.MSG_ID=MCISME_CUR_MSG.ID OR MCISME_EVT_SET.MSG_ID IS NULL) "
"ORDER  BY MCISME_CUR_MSG.ID, MCISME_EVT_SET.ID";

const char* LOADUP_TASK_ST_SQL  = 
"SELECT MCISME_CUR_MSG.ID, MCISME_MSG_SET.STATE, MCISME_MSG_SET.SMSC_ID, "
       "MCISME_EVT_SET.ID, MCISME_EVT_SET.DT, MCISME_EVT_SET.CALLER, MCISME_EVT_SET.MSG_ID "
"FROM   MCISME_CUR_MSG, MCISME_MSG_SET, MCISME_EVT_SET "
"WHERE  MCISME_CUR_MSG.ABONENT = :ABONENT "
   "AND MCISME_MSG_SET.ID = MCISME_CUR_MSG.ID "
   "AND MCISME_EVT_SET.ABONENT = MCISME_MSG_SET.ABONENT "
   "AND (MCISME_EVT_SET.MSG_ID=MCISME_CUR_MSG.ID OR MCISME_EVT_SET.MSG_ID IS NULL) "
"ORDER  BY MCISME_EVT_SET.ID";

const char* DROP_OLD_EVT_CUR_SQL = 
"DELETE FROM MCISME_CUR_MSG WHERE ID IN (SELECT MSG_ID FROM MCISME_EVT_SET WHERE DT<=:DT)";
const char* DROP_OLD_EVT_MSG_SQL = 
"DELETE FROM MCISME_MSG_SET WHERE ID IN (SELECT MSG_ID FROM MCISME_EVT_SET WHERE DT<=:DT)";
const char* DROP_OLD_EVT_ALL_SQL = 
"DELETE FROM MCISME_EVT_SET WHERE DT<=:DT";

/* ----------------------- Access to current events set (MCI_EVT_SET) ------------------------ */

const char* GET_EVT_CALLER_ID   = "GET_EVT_CALLER_ID";
const char* CREATE_NEW_EVT_ID   = "CREATE_NEW_EVT_ID";
const char* UPDATE_MSG_EVT_ID   = "UPDATE_MSG_EVT_ID";
const char* DELETE_ALL_EVT_ID   = "DELETE_ALL_EVT_ID";
const char* DELETE_ANY_EVT_ID   = "DELETE_ANY_EVT_ID";
const char* COUNT_CALLERS_ID    = "COUNT_CALLERS_ID";

const char* GET_EVT_CALLER_SQL  = "SELECT DISTINCT CALLER FROM MCISME_EVT_SET WHERE MSG_ID=:MSG_ID";
const char* CREATE_NEW_EVT_SQL  = "INSERT INTO MCISME_EVT_SET (ID, ABONENT, DT, CALLER, MSG_ID) "
                                  "VALUES (:ID, :ABONENT, :DT, :CALLER, NULL)"; // MSG_ID is not assigned
const char* UPDATE_MSG_EVT_SQL  = "UPDATE MCISME_EVT_SET SET MSG_ID=:MSG_ID WHERE ID=:ID"; // assigns MSG_ID
const char* DELETE_ANY_EVT_SQL  = "DELETE FROM MCISME_EVT_SET WHERE ID=:ID";
const char* DELETE_ALL_EVT_SQL  = "DELETE FROM MCISME_EVT_SET WHERE MSG_ID=:MSG_ID";

const char* COUNT_ALL_CALLERS_SQL = "SELECT ABONENT, COUNT(DISTINCT CALLER) FROM MCISME_EVT_SET GROUP BY ABONENT";
const char* COUNT_CALLERS_SQL     = "SELECT COUNT(DISTINCT CALLER) FROM MCISME_EVT_SET WHERE ABONENT=:ABONENT";

/* ----------------------- Access to current message ids (MCI_CUR_MSG) ----------------------- */

const char* INS_CURRENT_MSG_ID  = "INS_CURRENT_MSG_ID";
const char* DEL_CURRENT_MSG_ID  = "DEL_CURRENT_MSG_ID";
const char* SET_CURRENT_MSG_ID  = "SET_CURRENT_MSG_ID";

const char* INS_CURRENT_MSG_SQL = "INSERT INTO MCISME_CUR_MSG (ABONENT, ID) VALUES (:ABONENT, :ID)";
const char* DEL_CURRENT_MSG_SQL = "DELETE FROM MCISME_CUR_MSG WHERE ABONENT=:ABONENT";
const char* SET_CURRENT_MSG_SQL = "UPDATE MCISME_CUR_MSG SET ID=:ID WHERE ABONENT=:ABONENT";

/* ----------------------- Error messages templates set ------------------------------------- */

const char* ROLLBACK_TRANSACT_ERROR_MESSAGE = "Failed to roolback transaction on system data source.";
const char* OBTAIN_CONNECTION_ERROR_MESSAGE = "Failed to obtain connection to system data source.";
const char* OBTAIN_STATEMENT_ERROR_MESSAGE  = "Failed to obtain statement for %s.";
const char* OBTAIN_RESULTSET_ERROR_MESSAGE  = "Failed to obtain result set for %s.";

/* ----------------------- Static part ----------------------- */

void Task::init(DataSource* _ds, Statistics* _statistics, time_t _validityTime,
                int _rowsPerMessage, int _maxCallersCount/*=-1*/, int _maxMessagesCount/*=-1*/)
{
    Task::logger = Logger::getInstance("smsc.mcisme.Task");
    Task::ds = _ds; Task::statistics = _statistics;
    Task::currentId  = 0; Task::sequenceId = 0;
    Task::validityTime = _validityTime;
    Task::maxCallersCount  = _maxCallersCount;
    Task::maxMessagesCount = _maxMessagesCount;
    Message::maxRowsPerMessage = _rowsPerMessage;
}
uint64_t Task::getNextId(Connection* connection/*=0*/)
{
    static const int MSG_ID_PRELOAD_COUNT = 1000;
    static Mutex idGenerationLock;

    __require__(ds);
    
    MutexGuard  guard(idGenerationLock);
    
    if (!currentId || !sequenceId || currentId-sequenceId >= MSG_ID_PRELOAD_COUNT)
    {
        bool isConnectionGet = false;
        try
        {
            if (!connection) {
                connection = ds->getConnection();
                if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
                isConnectionGet = true;
            }
            
            Statement* nextIdStmt = connection->getStatement(GET_NEXT_SEQID_ID, GET_NEXT_SEQID_SQL);
            if (!nextIdStmt)
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "sequence id generation");
            
            std::auto_ptr<ResultSet> rsGuard(nextIdStmt->executeQuery());
            ResultSet* rs = rsGuard.get();
            if (!rs || !rs->fetchNext())
                throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "sequence id generation");
            
            currentId = sequenceId = rs->getUint64(1);
            if (connection && isConnectionGet) ds->freeConnection(connection);
        }
        catch (Exception& exc) {
            smsc_log_error(logger, "%s", exc.what());
            if (connection && isConnectionGet) ds->freeConnection(connection);
            throw;
        }
    }
    return ++currentId;
}

bool Task::getMessage(const char* smsc_id, Message& message, 
                      MessageState& state, Connection* connection/*=0*/)
{
    state = UNKNOWNST;
    smsc_log_info(logger, "Task: get message by smscId=%s", smsc_id ? smsc_id:"-");
    if (!smsc_id || !smsc_id[0]) return false;

    bool messageExists = false;
    bool isConnectionGet = false;
    try
    {
        if (!connection) {
            connection = ds->getConnection();
            if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
            isConnectionGet = true;
        }
    
        /* SELECT ID, STATE, ABONENT FROM MCISME_MSG_SET WHERE SMSC_ID=:SMSC_ID */
        Statement* getStmt = connection->getStatement(SELECT_OLD_MSG_ID, SELECT_OLD_MSG_SQL);
        if (!getStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "message loadup by smscId");    
        getStmt->setString(1, smsc_id);
        std::auto_ptr<ResultSet> rsGuard(getStmt->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (rs && rs->fetchNext())
        {
            uint64_t msg_id  = rs->getUint64(1);
            uint8_t msgState = rs->getUint8(2);
            message.reset(rs->getString(3));
            message.id = msg_id; message.smsc_id = smsc_id;
            if (msgState == MESSAGE_WAIT_CNCL)      { 
                state = WAIT_CNCL; message.cancel = true;
            } 
            else if (msgState == MESSAGE_WAIT_SKIP) { 
                state = WAIT_SKIP; message.cancel = true; message.skip = true;
            } 
            else if (msgState == MESSAGE_WAIT_RESP)   state = WAIT_RESP;
            else if (msgState == MESSAGE_WAIT_RCPT)   state = WAIT_RCPT;
            else state = UNKNOWNST;
            messageExists = true;
        }

        if (connection && isConnectionGet) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "%s", exc.what());
        if (connection && isConnectionGet) ds->freeConnection(connection);
        throw;
    }
    return messageExists;
}

/*
const char* LOADUP_ALL_CUR_SQL = 
"SELECT MCISME_CUR_MSG.ABONENT, MCISME_CUR_MSG.ID, "
       "MCISME_ABONENTS.INFORM, MCISME_ABONENTS.NOTIFY, "
       "MCISME_ABONENTS.INFORM_ID, MCISME_ABONENTS.NOTIFY_ID, MCISME_ABONENTS.EVENT_MASK, "
       "MCISME_MSG_SET.STATE, MCISME_MSG_SET.SMSC_ID, "
       "MCISME_EVT_SET.ID, MCISME_EVT_SET.DT, MCISME_EVT_SET.CALLER, MCISME_EVT_SET.MSG_ID "
"FROM   MCISME_CUR_MSG, MCISME_ABONENTS, MCISME_MSG_SET, MCISME_EVT_SET "
"WHERE  MCISME_ABONENTS.ABONENT (+)= MCISME_CUR_MSG.ABONENT "
   "AND MCISME_MSG_SET.ID = MCISME_CUR_MSG.ID "
   "AND MCISME_EVT_SET.ABONENT = MCISME_CUR_MSG.ABONENT "
   "AND (MCISME_EVT_SET.MSG_ID=MCISME_CUR_MSG.ID OR MCISME_EVT_SET.MSG_ID IS NULL) "
"ORDER  BY MCISME_CUR_MSG.ID, MCISME_EVT_SET.ID";
*/
Hash<Task *> Task::loadupAll()
{
    Hash<Task *> tasks;
    
    __require__(ds);
    Connection* connection = 0;

    try
    {   
        connection = ds->getConnection();
        if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        
        smsc_log_debug(logger, "Task: expired events cleanup...");
        try // cleanup old messages & events
        { 
            time_t expirationTime = time(NULL) - Task::validityTime; 
            int32_t deletedItems = 0;
            std::auto_ptr<Statement> delOldEvtCurStmtGuard(connection->createStatement(DROP_OLD_EVT_CUR_SQL));
            Statement* delOldEvtCurStmt = delOldEvtCurStmtGuard.get();
            if (!delOldEvtCurStmt)
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "expired events cleanup 1");
            delOldEvtCurStmt->setDateTime(1, expirationTime);
            deletedItems = delOldEvtCurStmt->executeUpdate();
            smsc_log_debug(logger, "Task: %d current messages expired", deletedItems);
            std::auto_ptr<Statement> delOldEvtMsgStmtGuard(connection->createStatement(DROP_OLD_EVT_MSG_SQL));
            Statement* delOldEvtMsgStmt = delOldEvtMsgStmtGuard.get();
            if (!delOldEvtMsgStmt)
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "expired events cleanup 2");
            delOldEvtMsgStmt->setDateTime(1, expirationTime);
            deletedItems = delOldEvtMsgStmt->executeUpdate();
            smsc_log_debug(logger, "Task: %d total messages expired", deletedItems);
            std::auto_ptr<Statement> delOldEvtAllStmtGuard(connection->createStatement(DROP_OLD_EVT_ALL_SQL));
            Statement* delOldEvtAllStmt = delOldEvtAllStmtGuard.get();
            if (!delOldEvtAllStmt)
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "expired events cleanup 3");
            delOldEvtAllStmt->setDateTime(1, expirationTime);
            deletedItems = delOldEvtAllStmt->executeUpdate();
            smsc_log_debug(logger, "Task: %d events expired", deletedItems);
            connection->commit();
        } 
        catch (Exception& exc) {
            try { connection->rollback(); } catch (...) { smsc_log_error(logger, "failed to rollback"); }
            throw;
        }
        smsc_log_debug(logger, "Task: expired events cleanup done");

        std::auto_ptr<Statement> allMsgsStmtGuard(connection->createStatement(LOADUP_ALL_CUR_SQL));
        Statement* allMsgsStmt = allMsgsStmtGuard.get();
        if (!allMsgsStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "all tasks loadup");
        
        smsc_log_debug(logger, "Task: running load all tasks statement ...");
        std::auto_ptr<ResultSet> rsGuard(allMsgsStmt->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "all tasks loadup");
        smsc_log_debug(logger, "Task: all tasks fetching ...");

        Task* task = 0;
        uint64_t lastMsgId = 0;
        while (rs->fetchNext())
        {
            const char* abonent = rs->getString(1);
            uint64_t currentMsgId = rs->getUint64(2);
            if (!task || lastMsgId != currentMsgId)
            {
                if (!abonent || tasks.Exists(abonent)) {
                    smsc_log_error(logger, "Task: duplicate current message found for abonent %s", 
                                   abonent ? abonent:"-");
                    continue;
                }
                lastMsgId = currentMsgId;
                
                // switch task
                if (task) {
                    const char*  taskAbonent = task->getAbonent().c_str();
                    MessageState taskState = task->getCurrentState();
                    int eventsCount = task->getEventsCount();
                    int newEventsCount = task->getNewEventsCount();
                    smsc_log_debug(logger, "Task: loaded for abonent %s (state=%d, events: all=%d new=%d)",
                                   taskAbonent, taskState, eventsCount, newEventsCount);
                    task->callersCount = 0;
                    if (eventsCount > 0 && 
                        (taskState != WAIT_RCPT || newEventsCount > 0)) tasks.Insert(taskAbonent, task);
                    else delete task;
                    task = 0;
                }
                
                // loadup profile for task
                AbonentProfile profile; // default profile used if not defined for abonent
                if (!rs->isNull(3)) {
                    const char* infStr = rs->getString(3);
                    profile.inform = (infStr && (infStr[0]=='Y' || infStr[0]=='y'));
                } 
                if (!rs->isNull(4)) {
                    const char* notStr = rs->getString(4);
                    profile.notify = (notStr && (notStr[0]=='Y' || notStr[0]=='y'));
                }
                profile.informTemplateId = rs->isNull(5) ?   -1:rs->getUint32(5);
                profile.notifyTemplateId = rs->isNull(6) ?   -1:rs->getUint32(6);
                if (!rs->isNull(7)) profile.eventMask = rs->getUint8 (7);
                

                // loadup current message for task
                uint8_t msgState = rs->getUint8(8);
                if (msgState != MESSAGE_WAIT_RESP && msgState != MESSAGE_WAIT_CNCL && 
                    msgState != MESSAGE_WAIT_RCPT && msgState != MESSAGE_UNKNOWNST) 
                    throw Exception("Invalid message state %d for message #%lld", msgState, currentMsgId);
                const char* smsc_id = (rs->isNull(9)) ? 0:rs->getString(9);
                
                task = new Task(abonent);
                task->cur_smsc_id = (smsc_id) ? smsc_id:"";
                task->currentMessageState = (MessageState)msgState;
                task->currentMessageId = currentMsgId;
                task->abonentProfile = profile;
            }

            // loadup events for task (add next event)
            TaskEvent event; event.to = abonent;
            event.id     = rs->getUint64  (10);
            event.time   = rs->getDateTime(11);
            event.from   = (rs->isNull(12)) ? "":rs->getString(12);
            event.msg_id = (rs->isNull(13)) ?  0:rs->getUint64(13);
            if (event.msg_id <= 0) task->newEventsCount++;
            task->events.Push(event);
        }

        if (task) { // add last task
            const char*  taskAbonent = task->getAbonent().c_str();
            MessageState taskState = task->getCurrentState();
            int eventsCount = task->getEventsCount();
            int newEventsCount = task->getNewEventsCount();
            smsc_log_debug(logger, "Task: loaded for abonent %s (state=%d, events: all=%d new=%d)",
                           taskAbonent, taskState, eventsCount, newEventsCount);
            task->callersCount = 0;
            if (eventsCount > 0 && 
                (taskState != WAIT_RCPT || newEventsCount > 0)) tasks.Insert(taskAbonent, task);
            else delete task;
            task = 0;
        }

        if (Task::maxCallersCount > 0) // if distinct callers check enabled
        {
            /* SELECT ABONENT, COUNT(DISTINCT CALLER) FROM MCISME_EVT_SET GROUP BY ABONENT */
            std::auto_ptr<Statement> countCallersStmtGuard(connection->createStatement(COUNT_ALL_CALLERS_SQL));
            Statement* countCallersStmt = countCallersStmtGuard.get();
            if (!countCallersStmt)
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "all callers count");

            std::auto_ptr<ResultSet> rsGuard(countCallersStmt->executeQuery());
            ResultSet* rs = rsGuard.get();
            if (!rs)
                throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "all callers count");

            while (rs->fetchNext())
            {
                const char* taskAbonent = rs->isNull(1) ? 0:rs->getString(1);
                uint32_t callersCount = rs->isNull(2) ? 0: rs->getUint32(2);
                if (!taskAbonent || !taskAbonent[0]) continue;
                Task** taskPtr = tasks.GetPtr(taskAbonent);
                if (!taskPtr || *taskPtr) continue;
                else (*taskPtr)->callersCount = callersCount;
            }
        }

        if (connection) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "%s", exc.what());
        if (connection) ds->freeConnection(connection);
        //if (task) delete task; // TODO: ???
        throw;
    }
    
    smsc_log_debug(logger, "Task: all tasks loaded");
    return tasks;
}

/* ----------------------- Main logic implementation ----------------------- */

void Task::loadCallersCount(Connection* connection)
{
    callersCount = 0;
    if (Task::maxCallersCount > 0) // if distinct callers check enabled
    {
        /* SELECT COUNT(DISTINCT CALLER) FROM MCISME_EVT_SET WHERE ABONENT=:ABONENT */
        Statement* countCallersStmt = connection->getStatement(COUNT_CALLERS_ID, COUNT_CALLERS_SQL);
        if (!countCallersStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "count callers (loadup)");
        
        countCallersStmt->setString(1, abonent.c_str());
        std::auto_ptr<ResultSet> countRsGuard(countCallersStmt->executeQuery());
        ResultSet* countRs = countRsGuard.get();
        if (!countRs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "count callers (loadup)");
        if (countRs->fetchNext() && !(countRs->isNull(1))) 
            callersCount = countRs->getUint32(1);
    }
}

// Used from loadupTasks()
bool Task::loadupSkippedMessages(Array<Message>& cancels) // static
{
    // if max messages count check enabled
    if (Task::maxMessagesCount <= 0) return false; 
    cancels.Clean();

    __require__(ds);

    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        
        /* SELECT ABONENT, ID, SMSC_ID FROM MCISME_MSG_SET WHERE STATE=:STATE ORDER BY ABONENT, ID */
        std::auto_ptr<Statement> skippedStmtGuard(connection->createStatement(LOADUP_SKIPPED_SQL));
        Statement* skippedStmt = skippedStmtGuard.get();
        if (!skippedStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "skipped messages loadup");
        skippedStmt->setUint8(1, WAIT_SKIP);

        smsc_log_debug(logger, "Task: running load skipped messages statement ...");
        std::auto_ptr<ResultSet> rsGuard(skippedStmt->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "skipped messages loadup");
        smsc_log_debug(logger, "Task: skipped messages fetching ...");

        Message cancel; cancel.cancel = true; cancel.skip = true;
        while (rs->fetchNext())
        {
            const char* abonent = rs->isNull(1) ? 0:rs->getString(1);
            uint64_t id = rs->isNull(2) ? 0:rs->getUint64(2);
            const char* smsc_id = rs->isNull(3) ? 0:rs->getString(3);
            if (!abonent || !abonent[0] || !id || !smsc_id || !smsc_id[0]) {
                smsc_log_warn(logger, "Invalid skip message #%lld for abonent %s (smscId=%s)",
                              id, abonent ? abonent:"-", smsc_id ? smsc_id:"-");
                continue;
            }
            cancel.id = id; cancel.abonent = abonent; 
            cancel.smsc_id = smsc_id;
            cancels.Push(cancel);
        }

        if (connection) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "%s", exc.what());
        if (connection) ds->freeConnection(connection);
        throw;
    }
    
    smsc_log_debug(logger, "Task: %d skipped messages loaded", cancels.Count());
    return (cancels.Count() > 0);
}

// Used for each task on new event processing (if check enabled)
bool Task::checkMessagesToSkip(Array<Message>& cancels)
{
    // if max messages count check enabled
    if (Task::maxMessagesCount <= 0) return false;
    cancels.Clean(); 

    __require__(ds);

    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        
        /* SELECT ID, SMSC_ID FROM MCISME_MSG_SET WHERE ABONENT=:ABONENT 
                  AND STATE=:STATE ORDER BY ID FOR UPDATE */
        Statement* loadupStmt = connection->getStatement(LOADUP_TO_SKIP_ID, LOADUP_TO_SKIP_SQL);
        if (!loadupStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "messages to skip loadup");
        loadupStmt->setString(1, abonent.c_str());
        loadupStmt->setUint8 (2, WAIT_RCPT);

        smsc_log_debug(logger, "Task: running messages to skip statement ...");
        std::auto_ptr<ResultSet> rsGuard(loadupStmt->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "messages to skip loadup");
        smsc_log_debug(logger, "Task: messages to skip fetching ...");
        
        Message cancel; cancel.reset(abonent);
        cancel.cancel = true; cancel.skip = true;
        while (rs->fetchNext())
        {
            uint64_t id = rs->isNull(1) ? 0:rs->getUint64(1);
            if (id == currentMessageId) continue;
            const char* smsc_id = rs->isNull(2) ? 0:rs->getString(2);
            if (!id || !smsc_id || !smsc_id[0]) {
                smsc_log_warn(logger, "Invalid message #%lld for abonent %s (smscId=%s)",
                              id, abonent.c_str() ? abonent.c_str():"-", smsc_id ? smsc_id:"-");
                continue;
            }
            cancel.id = id; cancel.smsc_id = smsc_id;
            cancels.Push(cancel);
        }
        // +1 - current message
        int toSkip = (cancels.Count()+1) - Task::maxMessagesCount;
        if (toSkip > 0) // has messages to skip
        { 
            /* UPDATE MCISME_MSG_SET SET STATE=:STATE WHERE ID=:ID */
            Statement* skipStmt = connection->getStatement(ROLLUP_TO_SKIP_ID, ROLLUP_TO_SKIP_SQL);
            if (!skipStmt)
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "message(s) skip");
            skipStmt->setUint8(1, WAIT_SKIP);

            int toDelete = cancels.Count()-toSkip;
            if (toDelete > 0) cancels.Delete(toSkip, toDelete);
            for (int i=0; i<cancels.Count(); i++) {
                skipStmt->setUint64(2, cancels[i].id);
                skipStmt->executeUpdate();
            }
            connection->commit();
        } 
        else // no messages to skip
        { 
            try { connection->rollback(); } catch (...) { smsc_log_error(logger, "failed to rollback"); }
            cancels.Clean(); 
        }

        if (connection) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "%s", exc.what());
        if (connection) { 
            try { connection->rollback(); } catch (...) { smsc_log_error(logger, "failed to rollback"); }
            ds->freeConnection(connection);
        }
        throw;
    }
    
    smsc_log_debug(logger, "Task: %d messages to skip", cancels.Count());
    return (cancels.Count() > 0);
}

/*
const char* LOADUP_TASK_ST_SQL  = 
"SELECT MCISME_CUR_MSG.ID, MCISME_MSG_SET.STATE, MCISME_MSG_SET.SMSC_ID, "
       "MCISME_EVT_SET.ID, MCISME_EVT_SET.DT, MCISME_EVT_SET.CALLER, MCISME_EVT_SET.MSG_ID "
"FROM   MCISME_CUR_MSG, MCISME_MSG_SET, MCISME_EVT_SET "
"WHERE  MCISME_CUR_MSG.ABONENT = :ABONENT "
   "AND MCISME_MSG_SET.ID = MCISME_CUR_MSG.ID "
   "AND MCISME_EVT_SET.ABONENT = MCISME_MSG_SET.ABONENT "
   "AND (MCISME_EVT_SET.MSG_ID=MCISME_CUR_MSG.ID OR MCISME_EVT_SET.MSG_ID IS NULL) "
"ORDER  BY MCISME_EVT_SET.ID";
*/
void Task::loadup()
{
    __require__(ds);
    
    currentMessageId=0; currentMessageState=UNKNOWNST; cur_smsc_id="";
    events.Clean(); newEventsCount=0; callersCount = 0;
    
    Connection* connection = 0;
    try
    {   
        connection = ds->getConnection();
        if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);

        abonentProfile = AbonentProfiler::getProfile(abonent.c_str(), connection);

        Statement* loadupStmt = connection->getStatement(LOADUP_TASK_ST_ID, LOADUP_TASK_ST_SQL);
        if (!loadupStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "task loadup");
        loadupStmt->setString(1, abonent.c_str());
        
        std::auto_ptr<ResultSet> rsGuard(loadupStmt->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "task loadup");

        if (rs->fetchNext() && !rs->isNull(1)) // current message defined
        {
            currentMessageId = rs->getUint64(1);
            
            uint8_t msgState = rs->getUint8 (2);
            if (msgState != MESSAGE_WAIT_RESP && msgState != MESSAGE_WAIT_CNCL && 
                msgState != MESSAGE_WAIT_RCPT && msgState != MESSAGE_UNKNOWNST) 
                throw Exception("Invalid message state %d for message #%lld", msgState, currentMessageId);
            currentMessageState = (MessageState)msgState;

            const char* smsc_id = (rs->isNull(3)) ? 0:rs->getString(3);
            cur_smsc_id = (smsc_id) ? smsc_id:"";
            
            TaskEvent event; event.to = abonent;
            do // loadup task events
            {
                event.id     = rs->getUint64  (4);
                event.time   = rs->getDateTime(5);
                event.from   = (rs->isNull(6)) ? "":rs->getString(6);
                event.msg_id = (rs->isNull(7)) ?  0:rs->getUint64(7);
                if (event.msg_id <= 0) newEventsCount++;
                events.Push(event);
            } 
            while (rs->fetchNext());
        }

        loadCallersCount(connection);
        
        if (connection) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "%s", exc.what());
        if (connection) ds->freeConnection(connection);
        throw;
    }
}

void Task::addEvent(const MissedCallEvent& event)
{
    __require__(ds);
    
    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection)
            throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
            
        /* INSERT INTO MCISME_EVT_SET (ID, ABONENT, DT, CALLER, MSG_ID)
           VALUES (:ID, :ABONENT, :DT, :CALLER, NULL) */
        Statement* createStmt = connection->getStatement(CREATE_NEW_EVT_ID, CREATE_NEW_EVT_SQL);
        if (!createStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "create new missed call event");
        
        TaskEvent newEvent(event, getNextId(connection), 0);
        createStmt->setUint64  (1, newEvent.id);
        createStmt->setString  (2, newEvent.to.c_str());
        createStmt->setDateTime(3, newEvent.time);
        createStmt->setString  (4, newEvent.from.c_str(), (newEvent.from.length() <= 0));
        if (createStmt->executeUpdate() <= 0)
            throw Exception("Failed to insert new event record. To %s From %s #%lld", 
                            newEvent.to.c_str(), newEvent.from.c_str(), newEvent.id);
        loadCallersCount(connection); // ???
        connection->commit();
        ds->freeConnection(connection);
        newEvent.msg_id = 0; newEventsCount++; 
        // TODO: Optimize. Inc distinct callers count via events scan !!!
        events.Push(newEvent);
    } 
    catch (Exception& exc)
    {
        smsc_log_error(logger, "%s", exc.what());
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
    
    if (statistics) statistics->incMissed(abonent.c_str());
}

bool Task::formatMessage(Message& message)
{
    smsc_log_info(logger, "Task: formatting message for abonent %s", abonent.c_str());

    __require__(ds && templateFormatter);
    
    message.reset(abonent);
    Connection* connection = 0;
    uint64_t oldCurrentMessageId = currentMessageId;
    int oldNewEventsCount = newEventsCount;
    try
    {
        connection = ds->getConnection();
        if (!connection)
            throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);

        // scan events for old entries & delete it
        Statement* delAnyEvtStmt = connection->getStatement(DELETE_ANY_EVT_ID, DELETE_ANY_EVT_SQL);
        if (!delAnyEvtStmt) 
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "drop exired event");
        time_t expirationTime = time(NULL) - Task::validityTime; 
        int expiredCount = 0;
        for (int i=0; i<events.Count(); i++) {
            TaskEvent event = events[i];
            if (event.time > expirationTime) continue;
            delAnyEvtStmt->setUint64(1, event.id);
            delAnyEvtStmt->executeUpdate();
            smsc_log_debug(logger, "Task: event #%lld expired for abonent %s",
                           event.id, abonent.c_str());
            if (event.msg_id <= 0 && newEventsCount > 0) newEventsCount--;
            events.Delete(i); i--; expiredCount++;
        }
        if (expiredCount > 0) connection->commit();
        oldNewEventsCount = newEventsCount;
        
        // create new current message if needed
        if (currentMessageId <= 0) doNewCurrent(connection); 
        message.id = currentMessageId; message.smsc_id = cur_smsc_id;
        
        /* UPDATE MCISME_EVT_SET SET MSG_ID=:MSG_ID WHERE ID=:ID */
        Statement* assignEvtStmt = connection->getStatement(UPDATE_MSG_EVT_ID, UPDATE_MSG_EVT_SQL);
        if (!assignEvtStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "assign event to message");

        // add maximum events from task chain to message
        MessageFormatter formatter(templateFormatter); 
        for (int i=0; i<events.Count() && formatter.canAdd(events[i]); i++) 
        {
            bool bWasNew = (events[i].msg_id <= 0);
            events[i].msg_id = currentMessageId;

            assignEvtStmt->setUint64(1, events[i].msg_id);
            assignEvtStmt->setUint64(2, events[i].id);
            if (assignEvtStmt->executeUpdate() <= 0) {
                smsc_log_warn(logger, "Task for abonent %s: failed to assign event #%lld to message #%lld. "
                              "Possible event was receipted", abonent.c_str(), events[i].id, events[i].msg_id);
                if (bWasNew && oldNewEventsCount) oldNewEventsCount--;
                events.Delete(i); 
            }
            else formatter.addEvent(events[i]);
            
            if (bWasNew && newEventsCount > 0) newEventsCount--; 
        }

        /*
        int timeOffset = TimeOffsetManager::getOffset(abonent.c_str());
        formatter.formatMessage(message, timeOffset); 
        smsc_log_debug(logger, "Message for '%s' off=%d: %s", 
                       abonent.c_str(), timeOffset, message.message.c_str());*/
        formatter.formatMessage(message, TimeOffsetManager::getOffset(abonent.c_str())); 

        if (message.eventsCount > 0) connection->commit();
        else { 
            currentMessageId = oldCurrentMessageId; 
            newEventsCount = oldNewEventsCount;
            connection->rollback();
        }

        ds->freeConnection(connection);
    } 
    catch (Exception& exc)
    {
        currentMessageId = oldCurrentMessageId;
        newEventsCount = oldNewEventsCount;
        smsc_log_error(logger, "%s", exc.what());
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
    return (message.eventsCount > 0);
}

/*
 1) Берёт новый ID для сообщения (из сиквенса)
 2) Создаёт новое сообщение в MCISME_MSG_SET с SMSC_ID == NULL.
 3) Изменяет (или создаёт) запись в MCISME_CUR_MSG с новым ID'ом сообщения
*/
void Task::doNewCurrent(Connection* connection)
{
    __require__(connection);
    
    uint64_t msgId = Task::getNextId();
    
    /* INSERT INTO MCISME_MSG_SET (ID, STATE, ABONENT, SMSC_ID)
       VALUES (:ID, :STATE, :ABONENT, NULL) */
    Statement* newMsgStmt = connection->getStatement(CREATE_NEW_MSG_ID, CREATE_NEW_MSG_SQL);
    if (!newMsgStmt)
        throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "create new message");
    
    newMsgStmt->setUint64(1, msgId);
    newMsgStmt->setUint8 (2, WAIT_RESP);
    newMsgStmt->setString(3, abonent.c_str());

    if (newMsgStmt->executeUpdate() <= 0)
        throw Exception("Failed to create new message #%lld", msgId);

    /* UPDATE MCISME_CUR_MSG SET ID=:ID WHERE ABONENT=:ABONENT */
    Statement* updateCurStmt = connection->getStatement(SET_CURRENT_MSG_ID, SET_CURRENT_MSG_SQL);
    if (!updateCurStmt)
        throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "update current id");

    updateCurStmt->setUint64(1, msgId);
    updateCurStmt->setString(2, abonent.c_str());

    if (updateCurStmt->executeUpdate() <= 0)
    {
        /* INSERT INTO MCISME_CUR_MSG (ABONENT, ID) VALUES (:ABONENT, :ID) */
        Statement* createCurStmt = connection->getStatement(INS_CURRENT_MSG_ID, INS_CURRENT_MSG_SQL);
        if (!createCurStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "create current id");

        createCurStmt->setString(1, abonent.c_str());
        createCurStmt->setUint64(2, msgId);
        
        if (createCurStmt->executeUpdate() <= 0)
            throw Exception("Failed to set message #%lld current", msgId);
    }
    currentMessageId = msgId; cur_smsc_id = "";
}
void Task::clearCurrent(Connection* connection)
{
    __require__(connection);
    
    if (currentMessageId)
    {
        /* DELETE FROM MCISME_CUR_MSG WHERE ABONENT=:ABONENT */
        Statement* delCurMsgStmt = connection->getStatement(DEL_CURRENT_MSG_ID, DEL_CURRENT_MSG_SQL);
        if (!delCurMsgStmt) 
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "remove current message");
        delCurMsgStmt->setString(1, abonent.c_str());
        if (delCurMsgStmt->executeUpdate() <= 0)
            smsc_log_debug(logger, "Task: failed to remove current message #%lld for abonent %s",
                           currentMessageId, abonent.c_str());
        currentMessageId = 0;
    }
}

void Task::doWait(Connection* connection, const char* smsc_id, const MessageState& state)
{
    __require__(connection);
    
    if (!currentMessageId) {
        smsc_log_warn(logger, "Task: current message is undefined for abonent %s (do wait %s). "
                      "Possible was already receipted", abonent.c_str(),
                      ((state == WAIT_RESP) ? "responce":(state == WAIT_RCPT ? "receipt":"cancel")));
        return;
    }

    /* UPDATE MCISME_MSG_SET SET STATE=:STATE, SMSC_ID=:SMSC_ID WHERE ID=:ID */
    Statement* updateMsgStmt = connection->getStatement(ROLLUP_CUR_MSG_ID, ROLLUP_CUR_MSG_SQL);
    if (!updateMsgStmt)
        throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "change message state");

    updateMsgStmt->setUint8 (1, (uint8_t)state);
    updateMsgStmt->setString(2, smsc_id, (!smsc_id || smsc_id[0]=='\0'));
    updateMsgStmt->setUint64(3, currentMessageId);

    if (updateMsgStmt->executeUpdate() <= 0) {
        smsc_log_warn(logger, "Task: message #%lld not found for abonent %s (%s). Possible was receipted",
                      currentMessageId, abonent.c_str(), 
                      ((state == WAIT_RESP) ? "responce":(state == WAIT_RCPT ? "receipt":"cancel")));
    }
    currentMessageState = state;
}
void Task::waitCancel  (const char* smsc_id)
{
    smsc_log_info(logger, "Task: wait cancel on smscId=%s for abonent %s", 
                  smsc_id ? smsc_id:"-", abonent.c_str());

    __require__(ds);
    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection)
            throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        
        doWait(connection, smsc_id, WAIT_CNCL);
        
        connection->commit();
        ds->freeConnection(connection);
    } 
    catch (Exception& exc)
    {
        smsc_log_error(logger, "%s", exc.what());
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
}
void Task::waitResponce()
{
    smsc_log_info(logger, "Task: wait responce for abonent %s", abonent.c_str());

    __require__(ds);
    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection)
            throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        
        doWait(connection, 0, WAIT_RESP);
        
        connection->commit();
        ds->freeConnection(connection);
    } 
    catch (Exception& exc)
    {
        smsc_log_error(logger, "%s", exc.what());
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
}
void Task::waitReceipt(const char* smsc_id)
{
    smsc_log_info(logger, "Task: wait receipt on smscId=%s for abonent %s",
                  smsc_id ? smsc_id:"-", abonent.c_str());
    
    __require__(ds);
    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection)
            throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        
        // NOTE: keeps currentMessageId (message is possible incomlete)
        doWait(connection, smsc_id, WAIT_RCPT);
        
        connection->commit();
        ds->freeConnection(connection);
    } 
    catch (Exception& exc)
    {
        smsc_log_error(logger, "%s", exc.what());
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
}
void Task::waitReceipt(int eventCount, const char* smsc_id)
{
    smsc_log_info(logger, "Task: wait receipt & roll(%ld) on smscId=%s for abonent %s", 
                  eventCount, smsc_id ? smsc_id:"-", abonent.c_str());

    __require__(ds);
    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection)
            throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        
        // NOTE: clears current message & set currentMessageId = 0
        doWait(connection, smsc_id, WAIT_RCPT);
        clearCurrent(connection); 

        connection->commit();
        ds->freeConnection(connection);
    
        if (events.Count()>0 && eventCount>0) // shifting events
            events.Delete(0, (events.Count()>=eventCount) ? eventCount:events.Count());
    } 
    catch (Exception& exc)
    {
        smsc_log_error(logger, "%s", exc.what());
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
}

// Delete message (by msg_id if defined, else getMessage(smsc_id)) & all assigned events
// Return set of deleted events (for notification(s) processing) by caller
Array<std::string> Task::finalizeMessage(const char* smsc_id, bool delivered,
                                         uint64_t msg_id/*=0*/, bool needCallers/*=true*/)
{
    smsc_log_debug(logger, "Task: finalizing message #%lld smscId=%s delivered=%d",
                   msg_id, (smsc_id) ? smsc_id:"-", (int)delivered);
    
    if (statistics) {
        if (!delivered) statistics->incDelivered(abonent.c_str());
        else statistics->incFailed(abonent.c_str());
    }
    
    __require__(ds);

    Array<std::string> callers;
    Connection* connection = 0;
    uint64_t oldCurrentMessageId = currentMessageId;
    try
    {
        connection = ds->getConnection();
        if (!connection)
            throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);

        if (!msg_id) {
            Message message; MessageState state;
            if (!Task::getMessage(smsc_id, message, state, connection)) // load msg_id by smsc_id
                throw Exception("Task for abonent %s: Message for smscId=%s not found",
                                abonent.c_str(), (smsc_id) ? smsc_id:"-");
            msg_id = message.id;
        }

        // clear current message & set currentMessageId = 0 if exists
        if (currentMessageId > 0 && msg_id == currentMessageId) clearCurrent(connection);

        /* DELETE FROM MCISME_MSG_SET WHERE ID=:ID */
        Statement* delMsgStmt = connection->getStatement(DELETE_ANY_MSG_ID, DELETE_ANY_MSG_SQL);
        if (!delMsgStmt) 
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "finalize message");
        delMsgStmt->setUint64(1, msg_id);
        if (delMsgStmt->executeUpdate() <= 0)
            smsc_log_warn(logger, "Task for abonent %s: failed to delete message #%lld. Message not found",
                          abonent.c_str(), msg_id);

        if (needCallers)
        {
            /* SELECT DISTINCT(CALLER) FROM MCISME_EVT_SET WHERE MSG_ID=:MSG_ID */
            Statement* getEvtStmt = connection->getStatement(GET_EVT_CALLER_ID, GET_EVT_CALLER_SQL);
            if (!getEvtStmt) 
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "finalize message events");
            getEvtStmt->setUint64(1, msg_id);
            std::auto_ptr<ResultSet> rsGuard(getEvtStmt->executeQuery());
            ResultSet* rs = rsGuard.get();
            if (!rs)
                throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "finalize message events");
            while (rs->fetchNext()) {
                const char* caller = (rs->isNull(1)) ? 0:rs->getString(1);
                if (caller && caller[0]) callers.Push(caller);
            }
        }

        /* DELETE FROM MCISME_EVT_SET WHERE MSG_ID=:MSG_ID */
        Statement* delEvtStmt = connection->getStatement(DELETE_ALL_EVT_ID, DELETE_ALL_EVT_SQL);
        if (!delEvtStmt) 
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "finalize message events");
        delEvtStmt->setUint64(1, msg_id);
        delEvtStmt->executeUpdate();

        connection->commit();
        ds->freeConnection(connection);
    } 
    catch (Exception& exc)
    {
        if (msg_id > 0 && msg_id == oldCurrentMessageId) currentMessageId = oldCurrentMessageId;
        smsc_log_error(logger, "%s", exc.what());
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
    
    return callers;
}

}}
