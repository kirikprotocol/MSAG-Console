
#include "Tasks.h"

namespace smsc { namespace mcisme 
{
Logger*     Task::logger = 0;
DataSource* Task::ds     = 0;
uint64_t    Task::currentId  = 0;
uint64_t    Task::sequenceId = 0;

const char* GET_NEXT_SEQID_ID  = "GET_NEXT_SEQID_ID";
const char* CREATE_NEW_MSG_ID  = "CREATE_NEW_MSG_ID";
const char* UPDATE_MSG_TXT_ID  = "UPDATE_MSG_TXT_ID";
const char* SELECT_MSG_TXT_ID  = "SELECT_MSG_TXT_ID";
const char* ROLLUP_CUR_MSG_ID  = "ROLLUP_CUR_MSG_ID";
const char* DROP_TASK_MSGS_ID  = "DROP_TASK_MSGS_ID";

const char* LOADUP_MESSAGES_ID = "LOADUP_MESSAGES_ID";
const char* INS_CURRENT_MSG_ID = "INS_CURRENT_MSG_ID";
const char* DEL_CURRENT_MSG_ID = "DEL_CURRENT_MSG_ID";
const char* GET_CURRENT_MSG_ID = "GET_CURRENT_MSG_ID";
const char* SET_CURRENT_MSG_ID = "SET_CURRENT_MSG_ID";

/* ----------------------- Access to message ids generation (MCI_MSG_SEQ) -------------------- */

const char* GET_NEXT_SEQID_SQL  = "SELECT MCISME_MSG_SEQ.NEXTVAL FROM DUAL";

/* ----------------------- Access to current messages set (MCI_MSG_SET) ---------------------- */

const char* SELECT_MSG_TXT_SQL  = "SELECT MESSAGE FROM MCISME_MSG_SET WHERE ID=:ID";
const char* CREATE_NEW_MSG_SQL  = "INSERT INTO MCISME_MSG_SET (ID, STATE, ABONENT, MESSAGE, SMSC_ID) "
                                  "VALUES (:ID, :STATE, :ABONENT, :MESSAGE, NULL)";
const char* UPDATE_MSG_TXT_SQL  = "UPDATE MCISME_MSG_SET SET MESSAGE=:MESSAGE WHERE ID=:ID";
const char* ROLLUP_CUR_MSG_SQL  = "UPDATE MCISME_MSG_SET SET STATE=:STATE, SMSC_ID=:SMSC_ID WHERE ID=:ID";
const char* DROP_TASK_MSGS_SQL  = "DELETE FROM MCISME_MSG_SET WHERE ABONENT=:ABONENT";

const char* LOADUP_MESSAGES_SQL = "SELECT ID, SMSC_ID, MESSAGE FROM MCISME_MSG_SET "
                                  "WHERE ABONENT=:ABONENT AND ID>=:ID ORDER BY ID";

/* ----------------------- Access to current message ids (MCI_CUR_MSG) ----------------------- */

const char* INS_CURRENT_MSG_SQL = "INSERT INTO MCISME_CUR_MSG (ABONENT, ID) VALUES (:ABONENT, :ID)";
const char* DEL_CURRENT_MSG_SQL = "DELETE FROM MCISME_CUR_MSG WHERE ABONENT=:ABONENT";
const char* GET_CURRENT_MSG_SQL = "SELECT ID FROM MCISME_CUR_MSG WHERE ABONENT=:ABONENT"; // check is null
const char* SET_CURRENT_MSG_SQL = "UPDATE MCISME_CUR_MSG SET ID=:ID WHERE ABONENT=:ABONENT";

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

/* ----------------------- Main logic implementation ----------------------- */

void Task::loadMessages(Connection* connection/*=0*/)
{
    __require__(ds);

    messages.Clean(); bUpdated = false;

    bool isConnectionGet = false;
    try
    {
        if (!connection) {
            connection = ds->getConnection();
            if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
            isConnectionGet = true;
        }

        Statement* currIdStmt = connection->getStatement(GET_CURRENT_MSG_ID, GET_CURRENT_MSG_SQL);
        if (!currIdStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "messages loadup");

        currIdStmt->setString(1, abonent.c_str());
        std::auto_ptr<ResultSet> currIdRsGuard(currIdStmt->executeQuery());
        ResultSet* currIdRs = currIdRsGuard.get();
        if (!currIdRs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "messages loadup");
        
        if (currIdRs->fetchNext() && !currIdRs->isNull(1))
        {
            uint64_t currId = currIdRs->getUint64(1);
            
            Statement* getMsgStmt = connection->getStatement(LOADUP_MESSAGES_ID, LOADUP_MESSAGES_SQL);
            if (!getMsgStmt)
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "messages loadup");

            getMsgStmt->setString(1, abonent.c_str());
            getMsgStmt->setUint64(2, currId);
            std::auto_ptr<ResultSet> rsGuard(getMsgStmt->executeQuery());
            ResultSet* rs = rsGuard.get();
            if (!rs)
                throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "messages loadup");
            
            while (rs->fetchNext()) //ID, STATE, SMSC_ID, MESSAGE
            {
                uint64_t msgId = rs->getUint64(1);
                std::string msgSmscId = (rs->isNull(3) ? "":rs->getString(3));
                std::string msgText   = (rs->isNull(4) ? "":rs->getString(4));
                
                // set replace flag in getMessage & nextMessage methods only
                Message message(msgId, msgText, msgSmscId, false);
                messages.Push(message);
            }
        }

        if (connection && isConnectionGet) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        if (connection && isConnectionGet) ds->freeConnection(connection);
        throw;
    }
}

void Task::insertNewEvent(Connection* connection, const MissedCallEvent& event, bool setCurrent/*=false*/)
{
    __require__(connection);

    Statement* createMessage = connection->getStatement(CREATE_NEW_MSG_ID, CREATE_NEW_MSG_SQL);
    Statement* insertCurrent = connection->getStatement(INS_CURRENT_MSG_ID, INS_CURRENT_MSG_SQL);
    if (!createMessage || !insertCurrent) 
        throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "new message create");

    Message message(Task::getNextId(connection), abonent, "", "", false);
    message.addEvent(event, true);
    
    createMessage->setUint64(1, message.id);
    createMessage->setUint8 (2, (setCurrent) ? WAIT_RESP:NEW_STATE);
    createMessage->setString(3, abonent.c_str());
    createMessage->setString(4, message.message.c_str());
    createMessage->executeUpdate();

    if (setCurrent)
    {
        Statement* updateCurrent = connection->getStatement(SET_CURRENT_MSG_ID, SET_CURRENT_MSG_SQL);
        if (!updateCurrent) 
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "make new message current");
        
        updateCurrent->setString(1, abonent.c_str());
        updateCurrent->setUint64(2, message.id);
        
        if (updateCurrent->executeUpdate() <= 0)
        {
            Statement* insertCurrent = connection->getStatement(INS_CURRENT_MSG_ID, INS_CURRENT_MSG_SQL);
            if (!insertCurrent) 
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "make new message current");
            insertCurrent->setString(1, abonent.c_str());
            insertCurrent->setUint64(2, message.id);
            insertCurrent->executeUpdate();
        }
    }
    
    connection->commit();
    messages.Push(message);
}
void Task::updateMessageText(Connection* connection, const Message& message)
{
    __require__(connection);

    Statement* updateMessage = connection->getStatement(UPDATE_MSG_TXT_ID, UPDATE_MSG_TXT_SQL);
    if (!updateMessage) 
        throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "update message text");

    updateMessage->setString(1, message.message.c_str());
    updateMessage->setUint64(2, message.id);
    updateMessage->executeUpdate();
    
    connection->commit();
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
bool Message::addEvent(const MissedCallEvent& event, bool force/*=false*/)
{
    if (!force && eventCount >= maxEventsPerMessage) return false;

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
        if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
        int index = messages.Count()-1;

        if (index >= 0)
        {
            Message message = messages[index];
            if (message.addEvent(event, false))
            {   // Active message ok => update text & set replace if current
                updateMessageText(connection, message);
                if (index == 0) { 
                    bUpdated = true;
                    message.replace = true; // ??? if restarted ???
                }
                messages[index] = message;
            }
            else
            {   // Active message is full => add new message, do not set it current
                insertNewEvent(connection, event, false);
            }
        } 
        else
        {   // No active messages found => add new message & set it current
            insertNewEvent(connection, event, true);
        }

        if (connection) ds->freeConnection(connection);
    } 
    catch (...)
    {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }
}

bool Task::getMessage(Message& message)
{
    bool notEmpty = messages.Count() > 0;
    if (notEmpty) { 
        message = messages[0];
        bUpdated = false;
    }
    return notEmpty;
}

bool Task::nextMessage(const char* smsc_id, Message& message)
{
    __require__(ds);

    bool keepCurrent = this->wasUpdated();

    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);

        Statement* changeStmt = connection->getStatement(ROLLUP_CUR_MSG_ID, ROLLUP_CUR_MSG_SQL);
        if (!changeStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "change message state");

        changeStmt->setUint64(1, keepCurrent ? WAIT_RESP:WAIT_RCPT);
        changeStmt->setString(2, smsc_id);
        changeStmt->executeUpdate();

        if (!keepCurrent) // need to roll current message 
        {
            Statement* updateCurrent = connection->getStatement(SET_CURRENT_MSG_ID, SET_CURRENT_MSG_SQL);
            if (!updateCurrent) 
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "roll current message");

            updateCurrent->setString(1, abonent.c_str());
            updateCurrent->setUint64(2, message.id);

            if (updateCurrent->executeUpdate() <= 0)
            {
                Statement* insertCurrent = connection->getStatement(INS_CURRENT_MSG_ID, INS_CURRENT_MSG_SQL);
                if (!insertCurrent) 
                    throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "roll current message");
                
                insertCurrent->setString(1, abonent.c_str());
                insertCurrent->setUint64(2, message.id);
                insertCurrent->executeUpdate();
            }
        }
        
        connection->commit();
        ds->freeConnection(connection);

        if (!keepCurrent) messages.Shift(message); // shift current message
    } 
    catch (...)
    {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection) ds->freeConnection(connection);
        throw;
    }

    return getMessage(message);
}

void Task::deleteAllMessages()
{
    __require__(ds);

    Connection* connection = 0;
    try
    {
        connection = ds->getConnection();
        if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);

        Statement* delCurrIdStmt = connection->getStatement(DEL_CURRENT_MSG_ID, DEL_CURRENT_MSG_SQL);
        if (!delCurrIdStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "task current id delete");
        delCurrIdStmt->setString(1, abonent.c_str());
        delCurrIdStmt->executeUpdate();

        Statement* delStmt = connection->getStatement(DROP_TASK_MSGS_ID, DROP_TASK_MSGS_SQL);
        if (!delStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "task message delete");
        delStmt->setString(1, abonent.c_str());
        delStmt->executeUpdate();

        connection->commit();
        ds->freeConnection(connection);

        messages.Clean();
        bUpdated = false;
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
