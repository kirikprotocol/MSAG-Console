
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
const char* INS_CURRENT_MSG_ID = "INS_CURRENT_MSG_ID";
const char* DEL_CURRENT_MSG_ID = "DEL_CURRENT_MSG_ID";
const char* GET_CURRENT_MSG_ID = "GET_CURRENT_MSG_ID";
const char* SET_CURRENT_MSG_ID = "SET_CURRENT_MSG_ID";

/* ----------------------- Access to message ids generation (MCI_MSG_SEQ) -------------------- */
const char* GET_NEXT_SEQID_SQL  = "SELECT MCI_MSG_SEQ.NEXTVAL FROM DUAL";

/* ----------------------- Access to current messages set (MCI_MSG_SET) ---------------------- */
const char* SELECT_MSG_TXT_SQL  = "SELECT MSG FROM MCI_MSG_SET SET WHERE ID=:ID";
const char* CREATE_NEW_MSG_SQL  = "INSERT INTO MCI_MSG_SET (ID, ST, ABONENT, MSG, SMSC_ID) "
                                  "VALUES (:ID, :ST, :ABONENT, :MSG, NULL)";
const char* UPDATE_MSG_TXT_SQL  = "UPDATE MCI_MSG_SET SET MSG=:MSG WHERE ID=:ID";
const char* LOADUP_MESSAGES_SQL = "SELECT MSG FROM MCI_MSG_SET SET WHERE ID>=:ID AND ST=:ST"; // ???

/* ----------------------- Access to current message ids (MCI_CUR_MSG) ----------------------- */
const char* INS_CURRENT_MSG_SQL = "INSERT INTO MCI_CUR_MSG (ABONENT, ID) VALUES (:ABONENT, :ID)";
const char* DEL_CURRENT_MSG_SQL = "DELETE FROM MCI_CUR_MSG WHERE ABONENT=:ABONENT";
const char* GET_CURRENT_MSG_SQL = "SELECT ID FROM MCI_CUR_MSG WHERE ABONENT=:ABONENT"; // check is null
const char* SET_CURRENT_MSG_SQL = "UPDATE MCI_CUR_MSG SET ID=:ID WHERE ABONENT=:ABONENT";

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

void Task::load()
{
    // TODO: loadup messages on startup
}
void Task::roll()
{
    // TODO: roll to next message (if available)
}

void Task::insertNewEvent(Connection* connection, const MissedCallEvent& event, bool setCurrent/*=false*/)
{
    __require__(connection);

    Statement* createMessage = connection->getStatement(CREATE_NEW_MSG_ID, CREATE_NEW_MSG_SQL);
    Statement* insertCurrent = connection->getStatement(INS_CURRENT_MSG_ID, INS_CURRENT_MSG_SQL);
    if (!createMessage || !insertCurrent) 
        throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "new message create");

    Message message(Task::getNextId(connection), abonent, "", false, false);
    message.addEvent(event, true);
    
    createMessage->setUint64(1, message.id);
    createMessage->setUint8 (2, MESSAGE_NEW_STATE); // ? MESSAGE_WAIT_STATE
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
static int maxMessageTextSize = 256; // TODO: do it configurable

bool Message::addEvent(const MissedCallEvent& event, bool force/*=false*/)
{
    if (!force && (eventCount >= maxEventsPerMessage || 
                   message.length() >= maxMessageTextSize)) return false;

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
            {   // Active messages ok => update text & set replace if current
                updateMessageText(connection, message);
                if (index == 0) message.replace = true; // ??? if restarted ???
                messages[index] = message;
            }
            else
            {   // Active messages is full => add new message, do not set it current
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
void Task::formatMessage(Message& message)
{
}

}}
