
#include "Profiler.h"

namespace smsc { namespace mcisme 
{

DataSource* AbonentProfiler::ds     = 0;
Logger*     AbonentProfiler::logger = 0;

/* ----------------------- Error messages templates set ------------------------------------------------- */

extern const char* ROLLBACK_TRANSACT_ERROR_MESSAGE;
extern const char* OBTAIN_CONNECTION_ERROR_MESSAGE;
extern const char* OBTAIN_STATEMENT_ERROR_MESSAGE;
extern const char* OBTAIN_RESULTSET_ERROR_MESSAGE;

/* ----------------------- Access to abonents profiles (inform, notify & separate) (MCISME_ABONENTS) ---- */

const char* GET_ABONENT_PRO_ID  = "GET_ABONENT_PRO_ID";
const char* SET_ABONENT_PRO_ID  = "SET_ABONENT_PRO_ID";
const char* INS_ABONENT_PRO_ID  = "INS_ABONENT_PRO_ID";
const char* DEL_ABONENT_PRO_ID  = "DEL_ABONENT_PRO_ID";

const char* GET_ABONENT_PRO_SQL = "SELECT INFORM, NOTIFY, INFORM_ID, NOTIFY_ID FROM MCISME_ABONENTS "
                                  "WHERE ABONENT=:ABONENT";
const char* SET_ABONENT_PRO_SQL = "UPDATE MCISME_ABONENTS SET "
                                  "INFORM=:INFORM, NOTIFY=:NOTIFY, INFORM_ID=:INFORM_ID, NOTIFY_ID=:NOTIFY_ID "
                                  "WHERE ABONENT=:ABONENT";
const char* INS_ABONENT_PRO_SQL = "INSERT INTO MCISME_ABONENTS (ABONENT, INFORM, INFORM_ID, NOTIFY_ID) "
                                  "VALUES (:ABONENT, :INFORM, :INFORM_ID, :NOTIFY_ID)";
const char* DEL_ABONENT_PRO_SQL = "DELETE FROM MCISME_ABONENTS WHERE ABONENT=:ABONENT";

/* ----------------------- Main logic implementation (static functions) --------------------------------- */

void AbonentProfiler::init(DataSource* _ds)
{
    AbonentProfiler::logger = Logger::getInstance("smsc.mcisme.AbonentProfiler");
    AbonentProfiler::ds = _ds;
}
bool AbonentProfiler::delProfile(const char* abonent, Connection* connection/* =0 */)
{
    __require__(ds);

    bool result = false;
    bool isConnectionGet = false;
    try
    {   
        if (!connection) {
            connection = ds->getConnection();
            if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
            isConnectionGet = true;
        }
        
        /* DELETE MCISME_ABONENTS WHERE ABONENT=:ABONENT */
        Statement* delProStmt = connection->getStatement(DEL_ABONENT_PRO_ID, DEL_ABONENT_PRO_SQL);
        if (!delProStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "delete abonent profile");
        
        delProStmt->setString(1, abonent);
        result = (delProStmt->executeUpdate()) ? true:false;
        
        connection->commit();
        if (isConnectionGet) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "%s", exc.what());
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection && isConnectionGet) ds->freeConnection(connection);
    }
    return result;
}
void AbonentProfiler::setProfile(const char* abonent, const AbonentProfile& profile, Connection* connection /* =0 */)
{
    __require__(ds);

    bool isConnectionGet = false;
    try
    {   
        if (!connection) {
            connection = ds->getConnection();
            if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
            isConnectionGet = true;
        }
        
        /* UPDATE MCISME_ABONENTS SET INFORM=:INFORM, NOTIFY=:NOTIFY, 
           INFORM_ID=:INFORM_ID, NOTIFY_ID=:NOTIFY_ID WHERE ABONENT=:ABONENT */
        Statement* setProStmt = connection->getStatement(SET_ABONENT_PRO_ID, SET_ABONENT_PRO_SQL);
        if (!setProStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "update abonent profile");
                                                    
        setProStmt->setString(1, (profile.inform)   ? "Y":"N");
        setProStmt->setString(2, (profile.notify)   ? "Y":"N");
        setProStmt->setUint32(3, profile.informTemplateId, profile.informTemplateId < 0);
        setProStmt->setUint32(4, profile.notifyTemplateId, profile.notifyTemplateId < 0);
        
        if (!setProStmt->executeUpdate())
        {
            /* INSERT INTO MCISME_ABONENTS (ABONENT, INFORM, INFORM_ID, NOTIFY_ID)
               VALUES (:ABONENT, :INFORM, :INFORM_ID, :NOTIFY_ID) */
            Statement* insProStmt = connection->getStatement(INS_ABONENT_PRO_ID, INS_ABONENT_PRO_SQL);
            if (!insProStmt)
                throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "insert abonent profile");
            
            insProStmt->setString(1, abonent);
            insProStmt->setString(2, (profile.inform)   ? "Y":"N");
            insProStmt->setString(3, (profile.notify)   ? "Y":"N");
            setProStmt->setUint32(4, profile.informTemplateId, profile.informTemplateId < 0);
            setProStmt->setUint32(5, profile.notifyTemplateId, profile.notifyTemplateId < 0);

            if (!insProStmt->executeUpdate())
                throw Exception("Failed to insert new profile record for abonent: %s", abonent);
        }
        
        connection->commit();
        if (isConnectionGet) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "%s", exc.what());
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, ROLLBACK_TRANSACT_ERROR_MESSAGE); }
        if (connection && isConnectionGet) ds->freeConnection(connection);
    }
}
AbonentProfile AbonentProfiler::getProfile(const char* abonent, Connection* connection /* =0 */)
{
    __require__(ds);

    AbonentProfile profile;
    bool isConnectionGet = false;
    try
    {   
        if (!connection) {
            connection = ds->getConnection();
            if (!connection) throw Exception(OBTAIN_CONNECTION_ERROR_MESSAGE);
            isConnectionGet = true;
        }

        /* SELECT INFORM, NOTIFY, INFORM_ID, NOTIFY_ID FROM MCISME_ABONENTS WHERE ABONENT=:ABONENT */
        Statement* getProStmt = connection->getStatement(GET_ABONENT_PRO_ID, GET_ABONENT_PRO_SQL);
        if (!getProStmt)
            throw Exception(OBTAIN_STATEMENT_ERROR_MESSAGE, "obtain abonent profile");
        
        getProStmt->setString(1, abonent);
        std::auto_ptr<ResultSet> rsGuard(getProStmt->executeQuery());
        ResultSet* rs = rsGuard.get();
        if (!rs)
            throw Exception(OBTAIN_RESULTSET_ERROR_MESSAGE, "obtain abonent profile");
        
        if (rs->fetchNext())
        {
            const char* infStr = rs->isNull(1) ? 0:rs->getString(1);
            const char* notStr = rs->isNull(2) ? 0:rs->getString(2);
            profile.inform   = (infStr && (infStr[0]=='Y' || infStr[0]=='y'));
            profile.notify   = (notStr && (notStr[0]=='Y' || notStr[0]=='y'));
            profile.informTemplateId = rs->isNull(3) ? -1:rs->getUint32(3);
            profile.notifyTemplateId = rs->isNull(4) ? -1:rs->getUint32(4);
        }
        
        if (isConnectionGet) ds->freeConnection(connection);
    }
    catch (Exception& exc) {
        smsc_log_error(logger, "%s", exc.what());
        if (connection && isConnectionGet) ds->freeConnection(connection);
    }
    return profile;
}

}}

