
#include "Tasks.h"

namespace smsc { namespace mcisme 
{
Logger*     Task::logger = 0;
DataSource* Task::ds     = 0;
uint64_t    Task::currentId  = 0;
uint64_t    Task::sequenceId = 0;

/* ----------------------- Access to message ids generation (MCI_MSG_SEQ) -------------------- */

const char* GET_NEXT_SEQID_ID   = "GET_NEXT_SEQID_ID";
const char* GET_NEXT_SEQID_SQL  = "SELECT MCISME_MSG_SEQ.NEXTVAL FROM DUAL";

/* ----------------------- Access to current messages set (MCI_MSG_SET) ---------------------- */

const char* CREATE_NEW_MSG_ID   = "CREATE_NEW_MSG_ID";
const char* DELETE_ANY_MSG_ID   = "DELETE_ANY_MSG_ID";
const char* LOADUP_CUR_MSG_ID   = "LOADUP_CUR_MSG_ID";
const char* ROLLUP_CUR_MSG_ID   = "ROLLUP_CUR_MSG_ID";

const char* CREATE_NEW_MSG_SQL  = "INSERT INTO MCISME_MSG_SET (ID, STATE, ABONENT, SMSC_ID) "
                                  "VALUES (:ID, :STATE, :ABONENT, NULL)";
const char* DELETE_ANY_MSG_SQL  = "DELETE FROM MCISME_MSG_SET WHERE ID=:ID";
const char* LOADUP_CUR_MSG_SQL  = "SELECT STATE, SMSC_ID FROM MCISME_MSG_SET WHERE ID=:ID";
const char* ROLLUP_CUR_MSG_SQL  = "UPDATE MCISME_MSG_SET SET STATE=:STATE, SMSC_ID=:SMSC_ID WHERE ID=:ID";

/* ----------------------- Access to current events set (MCI_EVT_SET) ------------------------ */

const char* LOADUP_MSG_EVT_ID   = "LOADUP_MSG_EVT_ID";
const char* CREATE_NEW_EVT_ID   = "CREATE_NEW_EVT_ID";
const char* UPDATE_MSG_EVT_ID   = "UPDATE_MSG_EVT_ID";
const char* DELETE_MSG_EVT_ID   = "DELETE_MSG_EVT_ID";
const char* DELETE_ALL_EVT_ID   = "DELETE_ALL_EVT_ID";

const char* LOADUP_MSG_EVT_SQL  = "SELECT ID, DT, CALLER, MSG_ID FROM MCISME_EVT_SET "
                                  "WHERE ABONENT=:ABONENT AND (MSG_ID=:MSG_ID OR MSG_ID IS NULL) ORDER BY ID";
const char* CREATE_NEW_EVT_SQL  = "INSERT INTO MCISME_EVT_SET (ID, ABONENT, DT, CALLER, MSG_ID) "
                                  "VALUES (:ID, :ABONENT, :DT, :CALLER, NULL)"; // MSG_ID is not assigned
const char* UPDATE_MSG_EVT_SQL  = "UPDATE MCISME_EVT_SET SET MSG_ID=:MSG_ID WHERE ID=:ID"; // assigns MSG_ID
const char* DELETE_MSG_EVT_SQL  = "DELETE FROM MCISME_EVT_SET WHERE ID=:ID";
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

void Task::init(DataSource* _ds)
{
    Task::logger = Logger::getInstance("smsc.mcisme.Task");
    Task::ds = _ds; Task::currentId  = 0; Task::sequenceId = 0;
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
            if (connection && isConnectionGet) ds->freeConnection(connection);
            throw;
        }
    }
    return ++currentId;
}

bool Task::getMessage(const char* smsc_id, Message& message)
{
    // TODO: implement it !!!
    return false;
}

Hash<Task *> Task::loadupAll()
{
    __require__(ds);

    Hash<Task *> tasks;
    
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
                tasks.Insert(abonent, task);
            }
            else smsc_log_error(logger, "Dublicate current message found for abonent: %s");
        }
        
        if (connection) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
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
            throw Exception("Current task message is null for abonent: %s", abonent.c_str());
        
        uint8_t msgState = curRs->getUint8(1); // TODO: For what ???
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
            events.Push(event);
        }
        
        if (connection && isConnectionGet) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
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
        if (rs->fetchNext() && rs->isNull(1)) {
            currentMessageId = rs->getUint64(1);
            loadup(currentMessageId, connection);
        } else {
            currentMessageId = 0;
            events.Clean();
        }
        
        if (connection) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        if (connection) ds->freeConnection(connection);
        throw;
    }
}

static const char*  constShortEngMonthesNames[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static int maxEventsPerMessage = 10; // TODO: do it configurable

int Message::countEvents(const std::string& message)
{
    int counter = 0;
    const char* msg = message.c_str();
    while (msg && *msg) if (*msg++ == '\n') counter++;
    return counter;
}
bool Message::isFull()
{
    return (eventCount >= maxEventsPerMessage);
}
bool Message::addEvent(const MissedCallEvent& event, bool force/*=false*/)
{
    if (!force && isFull()) return false;

    char eventMessage[256];
    tm dt; localtime_r(&event.time, &dt);
    sprintf(eventMessage, "%s\n%s at %02d %s %02d:%02d", (eventCount > 0) ? "":"Missed call(s):", 
            event.from.c_str(), dt.tm_mday, constShortEngMonthesNames[dt.tm_mon], dt.tm_hour, dt.tm_min);
    message += eventMessage; eventCount++;
    return true;
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
        if (!createStmt->executeUpdate())
            throw Exception("Failed to insert new event record. To %s From %s #%lld", 
                            newEvent.to.c_str(), newEvent.from.c_str(), newEvent.id);
        
        connection->commit();
        ds->freeConnection(connection);
        events.Push(newEvent);
    } 
    catch (...)
    {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
}

void Task::deleteMessages()
{
    smsc_log_info(logger, "Deleting last events for abonent: %s", abonent.c_str());
    // TODO: delete current message & all active events for task (MSG_ID=CUR_MSG_ID or NULL)
}
bool Task::formatMessage(Message& message)
{
    smsc_log_info(logger, "Formatting message for abonent: %s", abonent.c_str());

    __require__(ds);
    
    message.id = currentMessageId; message.attempts = 0;
    message.abonent = abonent; message.message = ""; message.smsc_id = "";
    message.eventCount = 0; message.replace = false; // ???

    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection)
            throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        
        if (currentMessageId == 0) {
            // TODO: create new current message if not exists !!!
        }

        /* UPDATE MCISME_EVT_SET SET MSG_ID=:MSG_ID WHERE ID=:ID */
        Statement* assignEvtStmt = connection->getStatement(UPDATE_MSG_EVT_ID, UPDATE_MSG_EVT_SQL);
        if (!assignEvtStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "assign event to message");

        bool bUpdated = false;
        for (int i=0; i<events.Count(); i++) // add maximum events from chain to message
        {
            if (!message.addEvent(events[i], false)) break;
            if (events[i].msg_id == 0)
            {
                events[i].msg_id = currentMessageId;
                
                assignEvtStmt->setUint64(1, events[i].msg_id);
                assignEvtStmt->setUint64(2, events[i].id);

                if (!assignEvtStmt->executeUpdate())
                    throw Exception("Failed to assign event #%lld to message #%lld",
                                    events[i].id, events[i].msg_id);
                bUpdated = true;
            }
        }

        if (bUpdated) connection->commit();
        ds->freeConnection(connection);

        if (message.isFull()) currentMessageId = 0; // TODO: create new current message here ???
    } 
    catch (...)
    {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
    return (message.eventCount > 0);
}

// sets current message WAIT_RESPONCE, SMSC_ID=smsc_id.
void Task::waitResponce(const char* smsc_id)
{
    smsc_log_info(logger, "Wait responce for abonent: %s", abonent.c_str());

    __require__(ds);

    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection)
            throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        
        /* UPDATE MCISME_MSG_SET SET STATE=:STATE, SMSC_ID=:SMSC_ID WHERE ID=:ID */
        Statement* updateMsgStmt = connection->getStatement(ROLLUP_CUR_MSG_ID, ROLLUP_CUR_MSG_SQL);
        if (!updateMsgStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "wait responce");
        
        updateMsgStmt->setUint8 (1, WAIT_RESP);
        updateMsgStmt->setString(2, smsc_id, (!smsc_id || smsc_id[0]=='\0'));
        updateMsgStmt->setUint64(3, currentMessageId);

        if (!updateMsgStmt->executeUpdate())
            throw Exception("Current message #%lld not found (responce)", currentMessageId);
        
        connection->commit();
        ds->freeConnection(connection);
    } 
    catch (...)
    {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
}
void Task::waitReceipt(const char* smsc_id)
{
    // TODO: implement all waitXXX methods via doWait(const char* smsc_id, const MessageState& state)
}
void Task::waitReceipt(int eventCount, const char* smsc_id)
{
    smsc_log_info(logger, "Wait receipt for abonent: %s", abonent.c_str());

    if (events.Count()>0 && eventCount>0) // shifting events
        events.Delete(0, (events.Count()>=eventCount) ? eventCount:events.Count());

    __require__(ds);

    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection)
            throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        
        /* UPDATE MCISME_MSG_SET SET STATE=:STATE, SMSC_ID=:SMSC_ID WHERE ID=:ID */
        Statement* updateMsgStmt = connection->getStatement(ROLLUP_CUR_MSG_ID, ROLLUP_CUR_MSG_SQL);
        if (!updateMsgStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "wait responce");
        
        updateMsgStmt->setUint8 (1, WAIT_RCPT);
        updateMsgStmt->setString(2, smsc_id, (!smsc_id || smsc_id[0]=='\0'));
        updateMsgStmt->setUint64(3, currentMessageId);

        if (!updateMsgStmt->executeUpdate())
            throw Exception("Current message #%lld not found (receipt)", currentMessageId);
        
        connection->commit();
        ds->freeConnection(connection);
    } 
    catch (...)
    {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
}

}}
