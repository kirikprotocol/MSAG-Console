
#include "Tasks.h"

namespace smsc { namespace mcisme 
{

DataSource* Task::ds     = 0;
Logger*     Task::logger = 0;
Statistics* Task::statistics = 0;

uint64_t    Task::currentId  = 0;
uint64_t    Task::sequenceId = 0;
bool        Task::bInformAll   = false;
bool        Task::bNotifyAll   = false;

/* ----------------------- Access to message ids generation (MCI_MSG_SEQ) -------------------- */

const char* GET_NEXT_SEQID_ID   = "GET_NEXT_SEQID_ID";
const char* GET_NEXT_SEQID_SQL  = "SELECT MCISME_MSG_SEQ.NEXTVAL FROM DUAL";

/* ----------------------- Access to current messages set (MCI_MSG_SET) ---------------------- */

const char* CREATE_NEW_MSG_ID   = "CREATE_NEW_MSG_ID";
const char* DELETE_ANY_MSG_ID   = "DELETE_ANY_MSG_ID";
const char* LOADUP_CUR_MSG_ID   = "LOADUP_CUR_MSG_ID";
const char* SELECT_OLD_MSG_ID   = "SELECT_OLD_MSG_ID";
const char* ROLLUP_CUR_MSG_ID   = "ROLLUP_CUR_MSG_ID";

const char* CREATE_NEW_MSG_SQL  = "INSERT INTO MCISME_MSG_SET (ID, STATE, ABONENT, SMSC_ID) "
                                  "VALUES (:ID, :STATE, :ABONENT, NULL)";
const char* DELETE_ANY_MSG_SQL  = "DELETE FROM MCISME_MSG_SET WHERE ID=:ID";
const char* LOADUP_CUR_MSG_SQL  = "SELECT STATE, SMSC_ID FROM MCISME_MSG_SET WHERE ID=:ID";
const char* SELECT_OLD_MSG_SQL  = "SELECT ID, STATE, ABONENT FROM MCISME_MSG_SET WHERE SMSC_ID=:SMSC_ID";
const char* ROLLUP_CUR_MSG_SQL  = "UPDATE MCISME_MSG_SET SET STATE=:STATE, SMSC_ID=:SMSC_ID WHERE ID=:ID";

/* ----------------------- Access to current events set (MCI_EVT_SET) ------------------------ */

const char* LOADUP_MSG_EVT_ID   = "LOADUP_MSG_EVT_ID";
const char* GET_EVT_CALLER_ID   = "GET_EVT_CALLER_ID";
const char* CREATE_NEW_EVT_ID   = "CREATE_NEW_EVT_ID";
const char* UPDATE_MSG_EVT_ID   = "UPDATE_MSG_EVT_ID";
//const char* DELETE_MSG_EVT_ID   = "DELETE_MSG_EVT_ID";
const char* DELETE_ALL_EVT_ID   = "DELETE_ALL_EVT_ID";

const char* LOADUP_MSG_EVT_SQL  = "SELECT ID, DT, CALLER, MSG_ID FROM MCISME_EVT_SET "
                                  "WHERE ABONENT=:ABONENT AND (MSG_ID=:MSG_ID OR MSG_ID IS NULL) ORDER BY ID";
const char* GET_EVT_CALLER_SQL  = "SELECT DISTINCT CALLER FROM MCISME_EVT_SET WHERE MSG_ID=:MSG_ID";
const char* CREATE_NEW_EVT_SQL  = "INSERT INTO MCISME_EVT_SET (ID, ABONENT, DT, CALLER, MSG_ID) "
                                  "VALUES (:ID, :ABONENT, :DT, :CALLER, NULL)"; // MSG_ID is not assigned
const char* UPDATE_MSG_EVT_SQL  = "UPDATE MCISME_EVT_SET SET MSG_ID=:MSG_ID WHERE ID=:ID"; // assigns MSG_ID
//const char* DELETE_MSG_EVT_SQL  = "DELETE FROM MCISME_EVT_SET WHERE ID=:ID";
const char* DELETE_ALL_EVT_SQL  = "DELETE FROM MCISME_EVT_SET WHERE MSG_ID=:MSG_ID";

/* ----------------------- Access to current message ids (MCI_CUR_MSG) ----------------------- */

const char* INS_CURRENT_MSG_ID  = "INS_CURRENT_MSG_ID";
const char* DEL_CURRENT_MSG_ID  = "DEL_CURRENT_MSG_ID";
const char* GET_CURRENT_MSG_ID  = "GET_CURRENT_MSG_ID";
const char* SET_CURRENT_MSG_ID  = "SET_CURRENT_MSG_ID";

const char* INS_CURRENT_MSG_SQL = "INSERT INTO MCISME_CUR_MSG (ABONENT, ID) VALUES (:ABONENT, :ID)";
const char* DEL_CURRENT_MSG_SQL = "DELETE FROM MCISME_CUR_MSG WHERE ABONENT=:ABONENT";
const char* ALL_CURRENT_MSG_SQL = "SELECT ABONENT, ID FROM MCISME_CUR_MSG";
const char* GET_CURRENT_MSG_SQL = "SELECT ID FROM MCISME_CUR_MSG WHERE ABONENT=:ABONENT"; // check is null
const char* SET_CURRENT_MSG_SQL = "UPDATE MCISME_CUR_MSG SET ID=:ID WHERE ABONENT=:ABONENT";

/* ----------------------- Error messages templates set ------------------------------------- */

const char* ROLLBACK_TRANSACT_ERROR_MESSAGE = "Failed to roolback transaction on system data source.";
const char* OBTAIN_CONNECTION_ERROR_MESSAGE = "Failed to obtain connection to system data source.";
const char* OBTAIN_STATEMENT_ERROR_MESSAGE  = "Failed to obtain statement for %s.";
const char* OBTAIN_RESULTSET_ERROR_MESSAGE  = "Failed to obtain result set for %s.";

/* ----------------------- Static part ----------------------- */

void Task::init(DataSource* _ds, Statistics* _statistics, int rowsPerMessage)
{
    Task::logger = Logger::getInstance("smsc.mcisme.Task");
    Task::ds = _ds; Task::statistics = _statistics;
    Task::currentId  = 0; Task::sequenceId = 0;
    Message::maxRowsPerMessage = rowsPerMessage;
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
    smsc_log_info(logger, "Get message by smscId=%s", smsc_id ? smsc_id:"-");
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
            if (msgState == MESSAGE_WAIT_CNCL)    { state = WAIT_CNCL; message.cancel = true; }
            else if (msgState == MESSAGE_WAIT_RESP) state = WAIT_RESP;
            else if (msgState == MESSAGE_WAIT_RCPT) state = WAIT_RCPT;
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

Hash<Task *> Task::loadupAll()
{
    Hash<Task *> tasks;
    
    __require__(ds);
    Connection* connection = 0;
    try
    {   
        connection = ds->getConnection();
        if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);

        /* SELECT ABONENT, ID FROM MCISME_CUR_MSG */
        std::auto_ptr<Statement> allMsgsStmtGuard(connection->createStatement(ALL_CURRENT_MSG_SQL));
        Statement* allMsgsStmt = allMsgsStmtGuard.get();
        if (!allMsgsStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "all tasks loadup");
        
        std::auto_ptr<ResultSet> rsGuard(allMsgsStmt->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "all tasks loadup");

        while (rs->fetchNext())
        {
            const char* abonent = rs->getString(1);
            if (!tasks.Exists(abonent)) {
                Task* task = new Task(abonent);
                task->loadup(rs->getUint64(2), connection);
                if (task->getEventsCount()) tasks.Insert(abonent, task);
                else delete task;
            }
            else smsc_log_error(logger, "Duplicate current message found for abonent %s");
        }
        
        if (connection) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "%s", exc.what());
        if (connection) ds->freeConnection(connection);
        throw;
    }
    
    return tasks;
}

/* ----------------------- Main logic implementation ----------------------- */

void Task::loadup(uint64_t currId, Connection* connection/*=0*/) // private
{
    __require__(ds);

    currentMessageId = currId;
    events.Clean();

    bool isConnectionGet = false;
    try
    {
        if (!connection) {
            connection = ds->getConnection();
            if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
            isConnectionGet = true;
        }

        abonentProfile = AbonentProfiler::getProfile(abonent.c_str(), connection);

        /* SELECT STATE, SMSC_ID FROM MCISME_MSG_SET WHERE ID=:ID */
        Statement* curMsgStmt = connection->getStatement(LOADUP_CUR_MSG_ID, LOADUP_CUR_MSG_SQL);
        if (!curMsgStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "current task message loadup");
        
        curMsgStmt->setUint64(1, currentMessageId);
        
        std::auto_ptr<ResultSet> curRsGuard(curMsgStmt->executeQuery());
        ResultSet* curRs = curRsGuard.get();
        if (!curRs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "current task message loadup");
        if (!curRs->fetchNext()) 
            throw Exception("Current task message is null for abonent %s", abonent.c_str());
        
        uint8_t msgState = curRs->getUint8(1);
        if (msgState != MESSAGE_WAIT_RESP && msgState != MESSAGE_WAIT_CNCL && msgState != MESSAGE_WAIT_RCPT) 
            throw Exception("Invalid message state %d for message #%lld", msgState, currentMessageId);
        currentMessageState = (MessageState)msgState;

        const char* smsc_id = (curRs->isNull(2)) ? 0:curRs->getString(2);
        cur_smsc_id = (smsc_id) ? smsc_id:"";
        
        /* SELECT ID, DT, CALLER, MSG_ID FROM MCISME_EVT_SET 
           WHERE ABONENT=:ABONENT AND (MSG_ID=:MSG_ID OR MSG_ID IS NULL) ORDER BY ID */
        Statement* msgEvtStmt = connection->getStatement(LOADUP_MSG_EVT_ID, LOADUP_MSG_EVT_SQL);
        if (!msgEvtStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "current task events loadup");
        
        msgEvtStmt->setString(1, abonent.c_str());
        msgEvtStmt->setUint64(2, currentMessageId);
        
        std::auto_ptr<ResultSet> evtRsGuard(msgEvtStmt->executeQuery());
        ResultSet* evtRs = evtRsGuard.get();
        if (!evtRs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "current task events loadup");
        
        TaskEvent event; event.to = abonent;
        while (evtRs->fetchNext())
        {
            event.id     = evtRs->getUint64  (1);
            event.time   = evtRs->getDateTime(2);
            event.from   = (evtRs->isNull(3)) ? "":evtRs->getString(3);
            event.msg_id = (evtRs->isNull(4)) ?  0:evtRs->getUint64(4);
            if (event.msg_id <= 0) newEventsCount++;
            events.Push(event);
        }
        
        if (connection && isConnectionGet) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "%s", exc.what());
        if (connection && isConnectionGet) ds->freeConnection(connection);
        throw;
    }
}   

void Task::loadup()
{
    __require__(ds);
    
    Connection* connection = 0;
    try
    {   
        connection = ds->getConnection();
        if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);

        /* SELECT ID FROM MCISME_CUR_MSG WHERE ABONENT=:ABONENT */
        Statement* currMsgIdStmt = connection->getStatement(GET_CURRENT_MSG_ID, GET_CURRENT_MSG_SQL);
        if (!currMsgIdStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "task loadup");
        
        currMsgIdStmt->setString(1, abonent.c_str());
        
        std::auto_ptr<ResultSet> rsGuard(currMsgIdStmt->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "task loadup");
        if (rs->fetchNext() && !rs->isNull(1)) {
            currentMessageId = rs->getUint64(1);
            loadup(currentMessageId, connection);
        } else {
            abonentProfile = AbonentProfiler::getProfile(abonent.c_str(), connection);
            currentMessageId=0; currentMessageState=UNKNOWNST; cur_smsc_id="";
            events.Clean(); newEventsCount=0;
        }
        
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
        connection->commit();
        ds->freeConnection(connection);
        newEvent.msg_id = 0; newEventsCount++; 
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
    
    if (statistics) statistics->incMissed();
}

bool Task::formatMessage(Message& message)
{
    smsc_log_info(logger, "Formatting message for abonent %s", abonent.c_str());

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
            bool bWasNew = (events[i].msg_id <= 0) ;
            events[i].msg_id = currentMessageId;

            assignEvtStmt->setUint64(1, events[i].msg_id);
            assignEvtStmt->setUint64(2, events[i].id);
            if (assignEvtStmt->executeUpdate() <= 0) {
                smsc_log_debug(logger, "Failed to assign event #%lld to message #%lld. "
                               "Possible event was receipted", events[i].id, events[i].msg_id);
                if (bWasNew && oldNewEventsCount) oldNewEventsCount--;
                events.Delete(i); 
            }
            else formatter.addEvent(events[i]);
            
            if (bWasNew && newEventsCount > 0) newEventsCount--; 
        }

        formatter.formatMessage(message); // TODO: catch exception here ???
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
            smsc_log_debug(logger, "Failed to remove current message #%lld for abonent %s",
                           currentMessageId, abonent.c_str());
        currentMessageId = 0;
    }
}

void Task::doWait(Connection* connection, const char* smsc_id, const MessageState& state)
{
    __require__(connection);
    
    if (!currentMessageId) {
        smsc_log_warn(logger, "Current message is undefined for abonent %s (do wait %s). "
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
        smsc_log_warn(logger, "Message #%lld not found for abonent %s (%s). Possible was receipted",
                      currentMessageId, abonent.c_str(), 
                      ((state == WAIT_RESP) ? "responce":(state == WAIT_RCPT ? "receipt":"cancel")));
    }
    currentMessageState = state;
}
void Task::waitCancel  (const char* smsc_id)
{
    smsc_log_info(logger, "Wait cancel on smscId=%s for abonent %s", 
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
    smsc_log_info(logger, "Wait responce for abonent %s", abonent.c_str());

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
    smsc_log_info(logger, "Wait receipt on smscId=%s for abonent %s",
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
    smsc_log_info(logger, "Wait receipt & roll(%ld) on smscId=%s for abonent %s", 
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

Array<std::string> Task::finalizeMessage(const char* smsc_id, 
                                         bool delivered, bool retry, uint64_t msg_id/*=0*/)
{
    smsc_log_debug(logger, "Finalizing message #%lld smscId=%s delivered=%d",
                   msg_id, (smsc_id) ? smsc_id:"-", (int)delivered);

    Array<std::string> callers;
    
    // Delete message (by msg_id if defined, else getMessage(smsc_id)) & all assigned events
    // Return set of deleted events (for notification(s) processing) by caller
    
    __require__(ds);
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
                throw Exception("Message for smscId=%s not found", (smsc_id) ? smsc_id:"-");
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
            throw Exception("Failed to delete message #%lld. Message not found", msg_id);

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

        /* DELETE FROM MCISME_EVT_SET WHERE MSG_ID=:MSG_ID */
        Statement* delEvtStmt = connection->getStatement(DELETE_ALL_EVT_ID, DELETE_ALL_EVT_SQL);
        if (!delEvtStmt) 
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "finalize message events");
        delEvtStmt->setUint64(1, msg_id);
        delEvtStmt->executeUpdate();

        connection->commit();
        ds->freeConnection(connection);

        if (statistics) {
            if (delivered) statistics->incDelivered();
            else statistics->incFailed();
        }
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
