
#include "Tasks.h"

namespace smsc { namespace mcisme 
{
smsc::logger::Logger* Task::logger = 0;
DataSource*  Task::ds = 0;

const char* GET_NEXT_SEQID_ID  = "GET_NEXT_SEQID_ID";
const char* CREATE_NEW_MSG_ID  = "CREATE_NEW_MSG_ID";
const char* UPDATE_MSG_TXT_ID  = "UPDATE_MSG_TXT_ID";
const char* SELECT_MSG_TXT_ID  = "SELECT_MSG_TXT_ID";
const char* INS_CURRENT_MSG_ID = "INS_CURRENT_MSG_ID";
const char* DEL_CURRENT_MSG_ID = "DEL_CURRENT_MSG_ID";
const char* GET_CURRENT_MSG_ID = "GET_CURRENT_MSG_ID";
const char* SET_CURRENT_MSG_ID = "SET_CURRENT_MSG_ID";

const char* GET_NEXT_SEQID_SQL  = "SELECT MCI_MSG_SEQ.NEXTVAL FROM DUAL";
const char* SELECT_MSG_TXT_SQL  = "SELECT MSG FROM MCI_MSG_SET SET WHERE ID=:ID";
const char* CREATE_NEW_MSG_SQL  = "INSERT INTO MCI_MSG_SET (ID, ST, ABONENT, MSG) "
                                  "VALUES (:ID, :ST, :ABONENT, :MSG)";
const char* UPDATE_MSG_TXT_SQL  = "UPDATE MCI_MSG_SET SET MSG=:MSG WHERE ID=:ID";

const char* INS_CURRENT_MSG_SQL = "INSERT INTO MCI_CUR_MSG (ABONENT, ID) VALUES (:ABONENT, :ID)";
const char* DEL_CURRENT_MSG_SQL = "DELETE FROM MCI_CUR_MSG WHERE ABONENT=:ABONENT";
const char* GET_CURRENT_MSG_SQL = "SELECT ID FROM MCI_CUR_MSG WHERE ABONENT=:ABONENT"; // check is null
const char* SET_CURRENT_MSG_SQL = "UPDATE MCI_CUR_MSG SET ID=:ID WHERE ABONENT=:ABONENT";

void Task::load()
{
    // TODO: loadup messages on startup
}
void Task::roll()
{
    // TODO: roll to next message (if available)
}
void Task::addEvent(const MissedCallEvent& event)
{
    if (messages.Count() <= 0) {
        // create new message & set it current
    } else {

    }
}
void Task::formatMessage(Message& message)
{
}

}}
