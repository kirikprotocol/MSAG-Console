
#include "DistrListManager.h"

namespace smsc { namespace distrlist
{
    using std::string;

    using namespace smsc::db;
    using namespace smsc::sms;
    using namespace core::buffers;
    
smsc::logger::Logger *DistrListManager::logger = 
    Logger::getInstance("smsc.distribution.DistrListManager");

const char* FAILED_TO_OBTAIN_CONNECTION     = "Failed to obtain connection to DB";
const char* FAILED_TO_CREATE_STATEMENT      = "Failed to create statement";
const char* FAILED_TO_OBTAIN_RESULTSET      = "Failed to obtain result set";
const char* FAILED_TO_ROLLBACK_TRANSACTION  = "Failed to rollback transaction";

DistrListManager::DistrListManager(DataSource& _ds, Manager& config)
    throw(ConfigException)
        : DistrListAdmin(), ds(_ds)
{
    // TODO: Loadup parameters from config !
}
DistrListManager::~DistrListManager()
{
    // Do nothing ?
}

const char* CHECK_DL_SQL    = "SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE LIST=:LIST";
const char* LIST_DL_SQL     = "SELECT LIST, OWNER, MAX_EL FROM DL_SET";
const char* LIST_OWN_DL_SQL = "SELECT LIST, OWNER, MAX_EL FROM DL_SET WHERE OWNER=:OWNER";
const char* ADD_DL_SQL =   "INSERT INTO DL_SET (LIST, MAX_EL, OWNER) VALUES (:LIST, :MAX_EL, :OWNER)";
const char* GET_DL_SQL =   "SELECT OWNER, MAX_EL FROM DL_SET WHERE LIST=:LIST";

const char* GET_PRINCIPAL_SQL =    "SELECT MAX_LST, MAX_EL FROM DL_PRINCIPALS WHERE ADDRESS=:OWNER";
const char* COUNT_ONWNER_DLS_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE OWNER=:OWNER";
const char* CHECK_PRINCIPAL_SQL =  "SELECT NVL(COUNT(*), 0) FROM DL_PRINCIPALS WHERE ADDRESS=:ADDRESS";
const char* ADD_PRINCIPAL_SQL   =  "INSERT INTO DL_PRINCIPALS (ADDRESS, MAX_LST, MAX_EL) VALUES (:ADDRESS, :MAX_LST, :MAX_EL)";
const char* CHANGE_PRINCIPAL_SQL = "UPDATE DL_PRINCIPALS SET MAX_EL=:MAX_EL, MAX_LST=:MAX_LST WHERE ADDRESS=:ADDRESS";
const char* DELETE_PRINCIPAL_SQL =    "DELETE FROM DL_PRINCIPALS WHERE ADDRESS=:ADDRESS";
const char* CHECK_SUB_PRINCIPAL_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE ADDRESS=:ADDRESS";

const char* CHECK_SUBMITTER_SQL =  "SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE LIST=:LIST AND ADDRESS=:ADDRESS";
const char* ADD_SUBMITTER_SQL =    "INSERT INTO DL_SUBMITTERS (LIST, ADDRESS) VALUES (:LIST, :ADDRESS)";
const char* DELETE_SUBMITTER_SQL = "DELETE FROM DL_SUBMITTERS WHERE LIST=:LIST AND ADDRESS=:ADDRESS";

const char* SELECT_MEMBERS_SQL  =  "SELECT ADDRESS FROM DL_MEMBERS WHERE LIST=:LIST";
const char* DELETE_MEMBER_SQL =    "DELETE FROM DL_MEMBERS WHERE LIST=:LIST AND ADDRESS=:ADDRESS";
const char* CHECK_MEMBER_SQL =     "SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=:LIST AND ADDRESS=:ADDRESS";
const char* COUNT_MEMBERS_SQL =    "SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=:LIST";
const char* ADD_MEMBER_SQL   =     "INSERT INTO DL_MEMBERS (LIST, ADDRESS) VALUES (:LIST, :ADDRESS)";

const char* MAX_ONWNER_MEMBERS_SQL =
"SELECT NVL(MAX(COUNT(LIST)), 0) FROM DL_MEMBERS WHERE LIST IN"
"   (SELECT LIST FROM DL_SET WHERE OWNER=:OWNER) GROUP BY LIST";

const char* DELETE_MEMBERS_SQL = "DELETE FROM DL_MEMBERS WHERE LIST=:LIST";
const char* DELETE_DL_SUB_SQL  = "DELETE FROM DL_SUBMITTERS WHERE LIST=:LIST";
const char* DELETE_DL_SQL      = "DELETE FROM DL_SET WHERE LIST=:LIST";

void DistrListManager::addDistrList(string dlName, const Address& dlOwner) 
    throw(SQLException, ListAlreadyExistsException, 
          PrincipalNotExistsException, ListCountExceededException)
{
    const char* dlNameStr  = dlName.c_str();
    string dlOwnerStdStr = dlOwner.toString();
    const char* dlOwnerStr = dlOwnerStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: addDistrList() called. dlName:'%s', dlOwner: '%s'",
                 dlNameStr, dlOwnerStr);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        // Check for list existence
        std::auto_ptr<Statement> checkListGuard(connection->createStatement(CHECK_DL_SQL));
        Statement* checkList = checkListGuard.get();
        if (!checkList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        checkList->setString(1, dlNameStr);
        
        std::auto_ptr<ResultSet> checkListRsGuard(checkList->executeQuery());
        ResultSet* checkListRs = checkListRsGuard.get();
        if (!checkListRs || !checkListRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (checkListRs->getUint32(1))
            throw ListAlreadyExistsException("DL '%s' already exists", dlNameStr);

        // Check for principal existence
        std::auto_ptr<Statement> getPrincipalGuard(connection->createStatement(GET_PRINCIPAL_SQL));
        Statement* getPrincipal = getPrincipalGuard.get();
        if (!getPrincipal)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        getPrincipal->setString(1, dlOwnerStr);

        std::auto_ptr<ResultSet> getPrincipalRsGuard(getPrincipal->executeQuery());
        ResultSet* getPrincipalRs = getPrincipalRsGuard.get();
        if (!getPrincipalRs) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (!getPrincipalRs->fetchNext())
            throw PrincipalNotExistsException("Principal for owner '%s' not exists", dlOwnerStr);

        int maxLst = getPrincipalRs->getInt32(1);
        int maxEl  = getPrincipalRs->getInt32(2);

        // Check for principal maxLst constaint
        std::auto_ptr<Statement> countDlsGuard(connection->createStatement(COUNT_ONWNER_DLS_SQL));
        Statement* countDls = countDlsGuard.get();
        if (!countDls)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        countDls->setString(1, dlOwnerStr);

        std::auto_ptr<ResultSet> countDlsRsGuard(countDls->executeQuery());
        ResultSet* countDlsRs = countDlsRsGuard.get();
        if (!countDlsRs || !countDlsRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);

        int lists = countDlsRs->getInt32(1);
        if (lists >= maxLst)
            throw ListCountExceededException("DL count exceeded for owner '%s', maximum is %ld", 
                                             dlOwnerStr, maxLst); 
        
        // Add distribution list
        std::auto_ptr<Statement> addListGuard(connection->createStatement(ADD_DL_SQL));
        Statement* addList = addListGuard.get();
        if (!addList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        addList->setString(1, dlNameStr);
        addList->setInt32 (2, maxEl);
        addList->setString(3, dlOwnerStr);
        addList->executeUpdate();

        // Check for submitter existence
        std::auto_ptr<Statement> checkSubmitterGuard(connection->createStatement(CHECK_SUBMITTER_SQL));
        Statement* checkSubmitter = checkSubmitterGuard.get();
        if (!checkSubmitter)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        checkSubmitter->setString(1, dlNameStr);
        checkSubmitter->setString(2, dlOwnerStr);
        
        std::auto_ptr<ResultSet> checkSubmitterRsGuard(checkSubmitter->executeQuery());
        ResultSet* checkSubmitterRs = checkSubmitterRsGuard.get();
        if (!checkSubmitterRs || !checkSubmitterRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        
        if (!checkSubmitterRs->getUint32(1)) // owner as submitter not exists
        {
            // Add owner as distribution list submitter
            std::auto_ptr<Statement> addSubmitterGuard(connection->createStatement(ADD_SUBMITTER_SQL));
            Statement* addSubmitter = addSubmitterGuard.get();
            if (!addSubmitter)        
                throw SQLException(FAILED_TO_CREATE_STATEMENT);

            addSubmitter->setString(1, dlNameStr);
            addSubmitter->setString(2, dlOwnerStr);
            addSubmitter->executeUpdate();
        }
        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

void DistrListManager::deleteDistrList(string dlName)
    throw(SQLException, ListNotExistsException)
{
    const char* dlNameStr  = dlName.c_str();
    smsc_log_debug(logger, "DistrListManager: deleteDistrList() called. dlName:'%s'", dlNameStr);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> deleteListGuard(connection->createStatement(DELETE_DL_SQL));
        Statement* deleteList = deleteListGuard.get();
        if (!deleteList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        deleteList->setString(1, dlNameStr);
        if (deleteList->executeUpdate() <= 0)
            throw ListNotExistsException("DL '%s' not exists", dlNameStr);

        std::auto_ptr<Statement> deleteMembersGuard(connection->createStatement(DELETE_MEMBERS_SQL));
        Statement* deleteMembers = deleteMembersGuard.get();
        if (!deleteMembers)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        deleteMembers->setString(1, dlNameStr);
        deleteMembers->executeUpdate();

        std::auto_ptr<Statement> deleteSubmittersGuard(connection->createStatement(DELETE_DL_SUB_SQL));
        Statement* deleteSubmitters = deleteSubmittersGuard.get();
        if (!deleteSubmitters)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        deleteSubmitters->setString(1, dlNameStr);
        deleteSubmitters->executeUpdate();

        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

DistrList DistrListManager::getDistrList(string dlName)
    throw(SQLException, ListNotExistsException)
{
    DistrList list;
    const char* dlNameStr = dlName.c_str();

    smsc_log_debug(logger, "DistrListManager: getDistrList() called. dlName:'%s'", dlNameStr);
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> selectListGuard(connection->createStatement(GET_DL_SQL));
        Statement* selectList = selectListGuard.get();
        if (!selectList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        selectList->setString(1, dlNameStr);
        std::auto_ptr<ResultSet> selectListRsGuard(selectList->executeQuery());
        ResultSet* selectListRs = selectListRsGuard.get();
        if (!selectList || !(selectListRs->fetchNext())) 
            throw ListNotExistsException("DL '%s' not exists", dlNameStr);

        list.name   = dlName;
        list.system = selectListRs->isNull(1);
        list.owner  = (list.system) ? "":selectListRs->getString(1);
        list.maxEl  = selectListRs->getInt32(2);
    }
    catch(Exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
    return list;
}

Array<DistrList> DistrListManager::list(const Address& dlOwner)
    throw(SQLException, PrincipalNotExistsException)
{
    Array<DistrList> lists(0);
    
    string dlOwnerStdStr = dlOwner.toString();
    const char* dlOwnerStr = dlOwnerStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: list() called for owner '%s'", dlOwnerStr);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> selectListGuard(connection->createStatement(LIST_OWN_DL_SQL));
        Statement* selectList = selectListGuard.get();
        if (!selectList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        selectList->setString(1, dlOwnerStr);

        std::auto_ptr<ResultSet> selectListRsGuard(selectList->executeQuery());
        ResultSet* selectListRs = selectListRsGuard.get();
        if (!selectList) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);

        while (selectListRs->fetchNext())
        {
            const char* name = selectListRs->getString(1);
            if (!name || name[0] == '\0') continue;
            int maxEl = selectListRs->getInt32(3);
            if (selectListRs->isNull(2)) {
                lists.Push(DistrList(name, maxEl));
            } else {
                const char* owner = selectListRs->getString(2);
                if (!owner || owner[0] == '\0') continue;
                lists.Push(DistrList(Address(owner), name, maxEl));
            }
        }

        if (lists.Count() <= 0) // Check principal existence
        {
            std::auto_ptr<Statement> checkPrincipalGuard(connection->createStatement(CHECK_PRINCIPAL_SQL));
            Statement* checkPrincipal = checkPrincipalGuard.get();
            if (!checkPrincipal)        
                throw SQLException(FAILED_TO_CREATE_STATEMENT);

            checkPrincipal->setString(1, dlOwnerStr);

            std::auto_ptr<ResultSet> checkRsGuard(checkPrincipal->executeQuery());
            ResultSet* checkRs = checkRsGuard.get();
            if (!checkRs || !checkRs->fetchNext()) 
                throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
            if (!checkRs->getUint32(1))
                throw PrincipalNotExistsException("Principal for address '%s' not exists",
                                                  dlOwnerStr);
        }
    }
    catch(Exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
    return lists;
}

Array<DistrList> DistrListManager::list()
    throw(SQLException)
{
    Array<DistrList> lists(0);
    
    smsc_log_debug(logger, "DistrListManager: list() called");
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> selectListGuard(connection->createStatement(LIST_DL_SQL));
        Statement* selectList = selectListGuard.get();
        if (!selectList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        std::auto_ptr<ResultSet> selectListRsGuard(selectList->executeQuery());
        ResultSet* selectListRs = selectListRsGuard.get();
        if (!selectList) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);

        while (selectListRs->fetchNext())
        {
            const char* name = selectListRs->getString(1);
            if (!name || name[0] == '\0') continue;
            int maxEl = selectListRs->getInt32(3);
            if (selectListRs->isNull(2)) {
                lists.Push(DistrList(name, maxEl));
            } else {
                const char* owner = selectListRs->getString(2);
                if (!owner || owner[0] == '\0') continue;
                lists.Push(DistrList(Address(owner), name, maxEl));
            }
        }
    }
    catch(Exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
    return lists;
}

Array<Address> DistrListManager::members(string dlName, const Address& submitter)
    throw(SQLException, IllegalSubmitterException)
{
    Array<Address> members(0);
    
    const char* dlNameStr = dlName.c_str();
    string submitterStdStr = submitter.toString();
    const char* submitterStr = submitterStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: members() called for dl '%s', submitter '%s'",
                 dlNameStr, submitterStr);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> checkSubmitterGuard(connection->createStatement(CHECK_SUBMITTER_SQL));
        Statement* checkSubmitter = checkSubmitterGuard.get();
        if (!checkSubmitter)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        checkSubmitter->setString(1, dlNameStr);
        checkSubmitter->setString(2, submitterStr);
        
        std::auto_ptr<ResultSet> checkRsGuard(checkSubmitter->executeQuery());
        ResultSet* checkRs = checkRsGuard.get();
        if (!checkRs || !checkRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (!checkRs->getUint32(1))
            throw IllegalSubmitterException("Submitter '%s' undefined for dl '%s'",
                                            submitterStr, dlNameStr);
        
        std::auto_ptr<Statement> selectMembersGuard(connection->createStatement(SELECT_MEMBERS_SQL));
        Statement* selectMembers = selectMembersGuard.get();
        if (!selectMembers)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        
        selectMembers->setString(1, dlNameStr);
        
        std::auto_ptr<ResultSet> selectRsGuard(selectMembers->executeQuery());
        ResultSet* selectRs = selectRsGuard.get();
        if (!selectRs) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        
        while (selectRs->fetchNext())
        {
            const char* addr = selectRs->getString(1);
            if (!addr || addr[0] == '\0') continue;
            
            Address address(addr);
            members.Push(address);
        }

    }
    catch(Exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
    return members;
}

bool DistrListManager::checkPermission(string dlName, const Address& submitter)
    throw(SQLException, ListNotExistsException)
{
    bool result = false;
    const char* dlNameStr = dlName.c_str();
    string submitterStdStr = submitter.toString();
    const char* submitterStr = submitterStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: checkPermission() called for dl '%s', submitter '%s'",
                 dlNameStr, submitterStr);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> checkSubmitterGuard(connection->createStatement(CHECK_SUBMITTER_SQL));
        Statement* checkSubmitter = checkSubmitterGuard.get();
        if (!checkSubmitter)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        checkSubmitter->setString(1, dlNameStr);
        checkSubmitter->setString(2, submitterStr);
        
        std::auto_ptr<ResultSet> checkRsGuard(checkSubmitter->executeQuery());
        ResultSet* checkRs = checkRsGuard.get();
        if (!checkRs || !checkRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        
        result = (checkRs->getUint32(1)) ? true:false;
    }
    catch(Exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
    return result;
}

void DistrListManager::addPrincipal(const Principal& prc)
    throw(SQLException, PrincipalAlreadyExistsException)
{
    string prcAddressStdStr = prc.address.toString();
    const char* prcAddressStr = prcAddressStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: addPrincipal() called. Addr:'%s' maxLst=%d, maxEl=%d",
                 (prcAddressStr) ? prcAddressStr:"null", prc.maxLst, prc.maxEl);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> checkPrincipalGuard(connection->createStatement(CHECK_PRINCIPAL_SQL));
        Statement* checkPrincipal = checkPrincipalGuard.get();
        if (!checkPrincipal)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        
        checkPrincipal->setString(1, prcAddressStr);
        
        std::auto_ptr<ResultSet> checkRsGuard(checkPrincipal->executeQuery());
        ResultSet* checkRs = checkRsGuard.get();
        if (!checkRs || !checkRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (checkRs->getUint32(1))
            throw PrincipalAlreadyExistsException("Principal for address '%s' already exists",
                                                  prcAddressStr);

        std::auto_ptr<Statement> addPrincipalGuard(connection->createStatement(ADD_PRINCIPAL_SQL));
        Statement* addPrincipal = addPrincipalGuard.get();
        if (!addPrincipal)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        addPrincipal->setString(1, prcAddressStr);
        addPrincipal->setInt32 (2, prc.maxLst);
        addPrincipal->setInt32 (3, prc.maxEl);
        
        addPrincipal->executeUpdate();
        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

void DistrListManager::deletePrincipal(const Address& address) 
    throw(SQLException, PrincipalNotExistsException, PrincipalInUseException)
{
    string prcAddressStdStr = address.toString();
    const char* prcAddressStr = prcAddressStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: deletePrincipal() called. Addr:'%s'",
                 (prcAddressStr) ? prcAddressStr:"null");
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        // Check principal use as owner
        std::auto_ptr<Statement> checkDlsPrincipalGuard(connection->createStatement(COUNT_ONWNER_DLS_SQL));
        Statement* checkDlsPrincipal = checkDlsPrincipalGuard.get();
        if (!checkDlsPrincipal)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        
        checkDlsPrincipal->setString(1, prcAddressStr);
        
        std::auto_ptr<ResultSet> checkDlsPrincipalRsGuard(checkDlsPrincipal->executeQuery());
        ResultSet* checkDlsPrincipalRs = checkDlsPrincipalRsGuard.get();
        if (!checkDlsPrincipalRs) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (checkDlsPrincipalRs->fetchNext() && (checkDlsPrincipalRs->getInt32(1) > 0))
            throw PrincipalInUseException("Principal for address '%s' is in use as DL owner(s)",
                                          prcAddressStr);
        
        // Check principal use as submitter
        std::auto_ptr<Statement> checkSubPrincipalGuard(connection->createStatement(CHECK_SUB_PRINCIPAL_SQL));
        Statement* checkSubPrincipal = checkSubPrincipalGuard.get();
        if (!checkSubPrincipal)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        
        checkSubPrincipal->setString(1, prcAddressStr);
        
        std::auto_ptr<ResultSet> checkSubPrincipalRsGuard(checkSubPrincipal->executeQuery());
        ResultSet* checkSubPrincipalRs = checkSubPrincipalRsGuard.get();
        if (!checkSubPrincipalRs) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (checkSubPrincipalRs->fetchNext() && (checkSubPrincipalRs->getInt32(1) > 0))
            throw PrincipalInUseException("Principal for address '%s' is in use as DL submitter(s)",
                                          prcAddressStr);
        
        // Delete principal
        std::auto_ptr<Statement> deletePrincipalGuard(connection->createStatement(DELETE_PRINCIPAL_SQL));
        Statement* deletePrincipal = deletePrincipalGuard.get();
        if (!deletePrincipal)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        deletePrincipal->setString(1, prcAddressStr);
        if (deletePrincipal->executeUpdate() <= 0)
            throw PrincipalNotExistsException("Principal for address '%s' not exists", prcAddressStr);

        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

void DistrListManager::changePrincipal(const Principal& prc) 
    throw(SQLException, PrincipalNotExistsException, IllegalPrincipalException)
{
    string prcAddressStdStr = prc.address.toString();
    const char* prcAddressStr = prcAddressStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: changePrincipal() called. Addr:'%s', maxLst=%ld, maxEl=%ld",
                 prcAddressStr, prc.maxEl, prc.maxLst);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        // Check principal's dl count
        std::auto_ptr<Statement> checkDlsPrincipalGuard(connection->createStatement(COUNT_ONWNER_DLS_SQL));
        Statement* checkDlsPrincipal = checkDlsPrincipalGuard.get();
        if (!checkDlsPrincipal)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        
        checkDlsPrincipal->setString(1, prcAddressStr);
        
        std::auto_ptr<ResultSet> checkDlsPrincipalRsGuard(checkDlsPrincipal->executeQuery());
        ResultSet* checkDlsPrincipalRs = checkDlsPrincipalRsGuard.get();
        if (!checkDlsPrincipalRs) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (checkDlsPrincipalRs->fetchNext())
        {
            int dlsCount = checkDlsPrincipalRs->getInt32(1);
            if (dlsCount > prc.maxLst)
                throw IllegalPrincipalException("Principal for address '%s' is in use %ld times as DL owner. "
                                                "Can't change maximum lists count to %ld",
                                                prcAddressStr, dlsCount, prc.maxLst);
        }
        
        // Check principal's members count in dl(s) 
        std::auto_ptr<Statement> checkMembersGuard(connection->createStatement(MAX_ONWNER_MEMBERS_SQL));
        Statement* checkMembers = checkMembersGuard.get();
        if (!checkMembers)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        
        checkMembers->setString(1, prcAddressStr);
        
        std::auto_ptr<ResultSet> checkMembersRsGuard(checkMembers->executeQuery());
        ResultSet* checkMembersRs = checkMembersRsGuard.get();
        if (!checkMembersRs) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (checkMembersRs->fetchNext())
        {
            int membersCount = checkMembersRs->getInt32(1);
            if (membersCount > prc.maxEl)
                throw IllegalPrincipalException("Principal for address '%s' is in use as DL owner that has %ld elements. "
                                                "Can't change maximum elements count to %ld",
                                                prcAddressStr, membersCount, prc.maxEl);
        }
        
        // Change principal
        std::auto_ptr<Statement> changePrincipalGuard(connection->createStatement(CHANGE_PRINCIPAL_SQL));
        Statement* changePrincipal = changePrincipalGuard.get();
        if (!changePrincipal)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        changePrincipal->setInt32 (1, prc.maxEl);
        changePrincipal->setInt32 (2, prc.maxLst);
        changePrincipal->setString(3, prcAddressStr);

        if (changePrincipal->executeUpdate() <= 0)
            throw PrincipalNotExistsException("Principal for address '%s' not exists", prcAddressStr);

        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

Principal DistrListManager::getPrincipal(const Address& address) 
    throw(SQLException, PrincipalNotExistsException)
{
    string prcAddressStdStr = address.toString();
    const char* prcAddressStr = prcAddressStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: getPrincipal() called. Addr:'%s'", prcAddressStr);
    
    Principal prc;
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> getPrincipalGuard(connection->createStatement(GET_PRINCIPAL_SQL));
        Statement* getPrincipal = getPrincipalGuard.get();
        if (!getPrincipal)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        getPrincipal->setString(1, prcAddressStr);

        std::auto_ptr<ResultSet> getPrincipalRsGuard(getPrincipal->executeQuery());
        ResultSet* getPrincipalRs = getPrincipalRsGuard.get();
        if (!getPrincipalRs) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (!getPrincipalRs->fetchNext())
            throw PrincipalNotExistsException("Principal for address '%s' not exists", prcAddressStr);

        prc.maxLst = getPrincipalRs->getInt32(1);
        prc.maxEl  = getPrincipalRs->getInt32(2);
        prc.address = address;
    }
    catch(Exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
    return prc;
}

void DistrListManager::addMember(string dlName, const Address& member) 
    throw(SQLException, ListNotExistsException, 
          MemberAlreadyExistsException, MemberCountExceededException)
{
    const char* dlNameStr = dlName.c_str();
    string memberStdStr = member.toString();
    const char* memberStr = memberStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: addMember() called. dlName:'%s' member:'%s'",
                 dlNameStr, memberStr);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        // Check dl existence & get maxEl constraint
        std::auto_ptr<Statement> checkListGuard(connection->createStatement(GET_DL_SQL));
        Statement* checkList = checkListGuard.get();
        if (!checkList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        checkList->setString(1, dlNameStr);
        
        std::auto_ptr<ResultSet> checkListRsGuard(checkList->executeQuery());
        ResultSet* checkListRs = checkListRsGuard.get();
        if (!checkListRs) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (!checkListRs->fetchNext())
            throw ListNotExistsException("DL '%s' not exists", dlNameStr);

        int maxMembers = checkListRs->getInt32(2);
            
        // Check member existence
        std::auto_ptr<Statement> checkMemberGuard(connection->createStatement(CHECK_MEMBER_SQL));
        Statement* checkMember = checkMemberGuard.get();
        if (!checkMember)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        
        checkMember->setString(1, dlNameStr);
        checkMember->setString(2, memberStr);
        
        std::auto_ptr<ResultSet> checkMemberRsGuard(checkMember->executeQuery());
        ResultSet* checkMemberRs = checkMemberRsGuard.get();
        if (!checkMemberRs || !checkMemberRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (checkMemberRs->getUint32(1))
            throw MemberAlreadyExistsException("Member '%s' already exists in DL '%s'",
                                               memberStr, dlNameStr);
        
        // Check members count constraint 
        std::auto_ptr<Statement> countMembersGuard(connection->createStatement(COUNT_MEMBERS_SQL));
        Statement* countMembers = countMembersGuard.get();
        if (!countMembers)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        
        countMembers->setString(1, dlNameStr);
        
        std::auto_ptr<ResultSet> countMembersRsGuard(countMembers->executeQuery());
        ResultSet* countMembersRs = countMembersRsGuard.get();
        if (!countMembersRs || !countMembersRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);

        int members = countMembersRs->getUint32(1);
        if (members >= maxMembers)
            throw MemberCountExceededException("Members count exceeded in DL '%s', maximum is %ld ",
                                               dlNameStr, maxMembers);
        
        // Add member
        std::auto_ptr<Statement> addMemberGuard(connection->createStatement(ADD_MEMBER_SQL));
        Statement* addMember = addMemberGuard.get();
        if (!addMember)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        addMember->setString(1, dlNameStr);
        addMember->setString(2, memberStr);
        addMember->executeUpdate();
        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

void DistrListManager::deleteMember(string dlName, const Address& member) 
    throw(SQLException, ListNotExistsException, MemberNotExistsException)
{
    const char* dlNameStr = dlName.c_str();
    string memberStdStr = member.toString();
    const char* memberStr = memberStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: deleteMember() called. dlName:'%s' member:'%s'",
                 dlNameStr, memberStr);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> checkListGuard(connection->createStatement(CHECK_DL_SQL));
        Statement* checkList = checkListGuard.get();
        if (!checkList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        checkList->setString(1, dlNameStr);
        
        std::auto_ptr<ResultSet> checkListRsGuard(checkList->executeQuery());
        ResultSet* checkListRs = checkListRsGuard.get();
        if (!checkListRs || !checkListRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (!checkListRs->getUint32(1))
            throw ListNotExistsException("DL '%s' not exists", dlNameStr);
        
        std::auto_ptr<Statement> deleteMemberGuard(connection->createStatement(DELETE_MEMBER_SQL));
        Statement* deleteMember = deleteMemberGuard.get();
        if (!deleteMember)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        deleteMember->setString(1, dlNameStr);
        deleteMember->setString(2, memberStr);
        
        if (deleteMember->executeUpdate() <= 0)
            throw MemberNotExistsException("Member '%s' not exists in DL '%s'",
                                           memberStr, dlNameStr);

        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

void DistrListManager::deleteMembers(string dlName) 
    throw(SQLException, ListNotExistsException)
{
    const char* dlNameStr = dlName.c_str();
    smsc_log_debug(logger, "DistrListManager: deleteMembers() called. dlName:'%s'", dlNameStr);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> checkListGuard(connection->createStatement(CHECK_DL_SQL));
        Statement* checkList = checkListGuard.get();
        if (!checkList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        checkList->setString(1, dlNameStr);
        
        std::auto_ptr<ResultSet> checkListRsGuard(checkList->executeQuery());
        ResultSet* checkListRs = checkListRsGuard.get();
        if (!checkListRs || !checkListRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (!checkListRs->getUint32(1))
            throw ListNotExistsException("DL '%s' not exists", dlNameStr);
        
        std::auto_ptr<Statement> deleteMembersGuard(connection->createStatement(DELETE_MEMBERS_SQL));
        Statement* deleteMembers = deleteMembersGuard.get();
        if (!deleteMembers)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        deleteMembers->setString(1, dlNameStr);
        deleteMembers->executeUpdate();
        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

void DistrListManager::grantPosting(string dlName, const Address& submitter) 
    throw(SQLException, ListNotExistsException, 
          PrincipalNotExistsException, SubmitterAlreadyExistsException)
{
    const char* dlNameStr = dlName.c_str();
    string submitterStdStr = submitter.toString();
    const char* submitterStr = submitterStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: grantPosting() called. dlName:'%s', submitter: '%s'",
                 dlNameStr, submitterStr);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> checkListGuard(connection->createStatement(CHECK_DL_SQL));
        Statement* checkList = checkListGuard.get();
        if (!checkList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        checkList->setString(1, dlNameStr);
        
        std::auto_ptr<ResultSet> checkListRsGuard(checkList->executeQuery());
        ResultSet* checkListRs = checkListRsGuard.get();
        if (!checkListRs || !checkListRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (!checkListRs->getUint32(1))
            throw ListNotExistsException("DL '%s' not exists", dlNameStr);
        
        std::auto_ptr<Statement> checkPrincipalGuard(connection->createStatement(CHECK_PRINCIPAL_SQL));
        Statement* checkPrincipal = checkPrincipalGuard.get();
        if (!checkPrincipal)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        
        checkPrincipal->setString(1, submitterStr);
        
        std::auto_ptr<ResultSet> checkPrincipalRsGuard(checkPrincipal->executeQuery());
        ResultSet* checkPrincipalRs = checkPrincipalRsGuard.get();
        if (!checkPrincipalRs || !checkPrincipalRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (!checkPrincipalRs->getUint32(1))
            throw PrincipalNotExistsException("Principal for address '%s' not exists",
                                              submitterStr);
        
        std::auto_ptr<Statement> checkSubmitterGuard(connection->createStatement(CHECK_SUBMITTER_SQL));
        Statement* checkSubmitter = checkSubmitterGuard.get();
        if (!checkSubmitter)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        checkSubmitter->setString(1, dlNameStr);
        checkSubmitter->setString(2, submitterStr);
        
        std::auto_ptr<ResultSet> checkSubmitterRsGuard(checkSubmitter->executeQuery());
        ResultSet* checkSubmitterRs = checkSubmitterRsGuard.get();
        if (!checkSubmitterRs || !checkSubmitterRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (checkSubmitterRs->getUint32(1))
            throw SubmitterAlreadyExistsException("Submitter '%s' already exists in DL '%s'",
                                                  submitterStr, dlNameStr);

        std::auto_ptr<Statement> addSubmitterGuard(connection->createStatement(ADD_SUBMITTER_SQL));
        Statement* addSubmitter = addSubmitterGuard.get();
        if (!addSubmitter)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        addSubmitter->setString(1, dlNameStr);
        addSubmitter->setString(2, submitterStr);
        addSubmitter->executeUpdate();
        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

void DistrListManager::revokePosting(string dlName, const Address& submitter)
    throw(SQLException, ListNotExistsException, 
          SubmitterNotExistsException, IllegalSubmitterException)
{
    const char* dlNameStr = dlName.c_str();
    string submitterStdStr = submitter.toString();
    const char* submitterStr = submitterStdStr.c_str();
    smsc_log_debug(logger, "DistrListManager: revokePosting() called. dlName:'%s', submitter: '%s'",
                 dlNameStr, submitterStr);
    
    Connection* connection = 0;
    try
    {
        if (!(connection = ds.getConnection())) 
            throw SQLException(FAILED_TO_OBTAIN_CONNECTION);
        
        std::auto_ptr<Statement> getListGuard(connection->createStatement(GET_DL_SQL));
        Statement* getList = getListGuard.get();
        if (!getList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        getList->setString(1, dlNameStr);
        
        std::auto_ptr<ResultSet> getListRsGuard(getList->executeQuery());
        ResultSet* getListRs = getListRsGuard.get();
        if (!getListRs) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (!getListRs->fetchNext())
            throw ListNotExistsException("DL '%s' not exists", dlNameStr);
        
        const char* owner = getListRs->isNull(1) ? 0:getListRs->getString(1);
        if (owner && (strcmp(owner, submitterStr) == 0))
            throw IllegalSubmitterException("Submitter '%s' is an owner of DL '%s'",
                                            submitterStr, dlNameStr);

        std::auto_ptr<Statement> deleteSubmitterGuard(connection->createStatement(DELETE_SUBMITTER_SQL));
        Statement* deleteSubmitter = deleteSubmitterGuard.get();
        if (!deleteSubmitter)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        deleteSubmitter->setString(1, dlNameStr);
        deleteSubmitter->setString(2, submitterStr);
        
        if (!deleteSubmitter->executeUpdate())
            throw SubmitterNotExistsException("Submitter '%s' not exists in DL '%s'",
                                              submitterStr, dlNameStr);
        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        smsc_log_error(logger, "%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { smsc_log_error(logger, FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        smsc_log_error(logger, "%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

}}

