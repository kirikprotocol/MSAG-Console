
#include "DistrListManager.h"

namespace smsc { namespace distrlist
{
    using std::string;

    using namespace smsc::db;
    using namespace smsc::sms;
    using namespace core::buffers;
    
log4cpp::Category& DistrListManager::logger = 
    Logger::getCategory("smsc.distribution.DistrListManager");

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

const char* CHECK_DL_SQL = 
"SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE LIST=:LIST";
const char* ADD_DL_SQL =
"INSERT INTO DL_SET (LIST, MAX_EL, OWNER) VALUES (:LIST, :MAX_EL, :OWNER)";
const char* GET_PRINCIPAL_SQL =
"SELECT MAX_LST, MAX_EL FROM DL_PRINCIPALS WHERE ADDRESS=:OWNER";
const char* COUNT_ONWNER_DLS_SQL =
"SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE OWNER=:OWNER";

void DistrListManager::addDistrList(const DistrList& list) 
    throw(SQLException, ListAlreadyExistsException, 
          PrincipalNotExistsException, IllegalListException, ListCountExceededException)
{
    const char* dlNameStr  = list.name.c_str();
    const char* dlOwnerStr = (list.system) ? 0: ((list.owner.length() > 0) ? list.owner.c_str():0);
    logger.debug("DistrListManager: addDistrList() called. dlName:'%s', maxEl:%d, owner: '%s'",
                 dlNameStr, list.maxEl, (dlOwnerStr) ? dlOwnerStr:"system");
    
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
        if (checkListRs->getUint32(1))
            throw ListAlreadyExistsException("DL '%s' already exists", dlNameStr);

        if (dlOwnerStr)
        {

            std::auto_ptr<Statement> getPrincipalGuard(connection->createStatement(GET_PRINCIPAL_SQL));
            Statement* getPrincipal = getPrincipalGuard.get();
            if (!getPrincipal)        
                throw SQLException(FAILED_TO_CREATE_STATEMENT);

            getPrincipal->setString(1, dlOwnerStr);

            std::auto_ptr<ResultSet> getPrincipalRsGuard(checkList->executeQuery());
            ResultSet* getPrincipalRs = getPrincipalRsGuard.get();
            if (!getPrincipalRs) 
                throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
            if (!getPrincipalRs->fetchNext())
                throw PrincipalNotExistsException("Principal for owner '%s' not exists", dlOwnerStr);
            
            int maxLst = getPrincipalRs->getInt32(1);
            int maxEl  = getPrincipalRs->getInt32(2);
            if (list.maxEl<=0  || list.maxEl > maxEl)
                throw IllegalListException("DL's maxEl=%ld is invalid, principal value is %ld",
                                           list.maxEl, maxEl);

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
        }
        
        std::auto_ptr<Statement> addListGuard(connection->createStatement(ADD_DL_SQL));
        Statement* addList = addListGuard.get();
        if (!addList)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        addList->setString(1, dlNameStr);
        addList->setInt32 (2, list.maxEl);
        addList->setString(3, dlOwnerStr, !dlOwnerStr);
        addList->executeUpdate();
        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

const char* DELETE_MEMBERS_SQL = "DELETE FROM DL_MEMBERS WHERE LIST=:LIST";
const char* DELETE_DL_SUB_SQL  = "DELETE FROM DL_SUBMITTERS WHERE LIST=:LIST";
const char* DELETE_DL_SQL      = "DELETE FROM DL_SET WHERE LIST=:LIST";

void DistrListManager::deleteDistrList(string dlName)
    throw(SQLException, ListNotExistsException)
{
    const char* dlNameStr  = dlName.c_str();
    logger.debug("DistrListManager: deleteDistrList() called. dlName:'%s'", dlNameStr);
    
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
        if (!deleteList->executeUpdate())
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
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

const char* GET_DL_SQL = "SELECT OWNER, MAX_EL FROM DL_SET WHERE LIST=:LIST";

DistrList DistrListManager::getDistrList(string dlName)
    throw(SQLException, ListNotExistsException)
{
    DistrList list;
    const char* dlNameStr = dlName.c_str();

    logger.debug("DistrListManager: getDistrList() called. dlName:'%s'", dlNameStr);
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
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
    return list;
}

const char* LIST_DL_SQL = "SELECT LIST, OWNER, MAX_EL FROM DL_SET";

Array<DistrList> DistrListManager::list()
    throw(SQLException)
{
    Array<DistrList> lists(0);
    
    logger.debug("DistrListManager: list() called");
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
            const char* owner = (selectListRs->isNull(2)) ? 0:selectListRs->getString(2);
            int maxEl = selectListRs->getInt32(3);
            lists.Push(DistrList(name, maxEl, !owner, (owner) ? owner:""));
        }
    }
    catch(Exception& exc) {
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
    return lists;
}

const char* SELECT_MEMBERS_SQL  = (const char*)
"SELECT ADDRESS FROM DL_MEMBERS WHERE LIST=:LIST";
const char* CHECK_SUBMITTER_SQL = (const char*)
"SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE LIST=:LIST AND ADDRESS=:ADDRESS";

Array<Address> DistrListManager::members(string dlName, const Address& submitter)
    throw(SQLException, IllegalSubmitterException)
{
    Array<Address> members(0);
    
    const char* dlNameStr = dlName.c_str();
    const char* submitterStr = submitter.toString().c_str();
    logger.debug("DistrListManager: members() called for dl '%s', submitter '%s'",
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
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
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
    const char* submitterStr = submitter.toString().c_str();
    logger.debug("DistrListManager: checkPermission() called for dl '%s', submitter '%s'",
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
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
    return result;
}

const char* CHECK_PRINCIPAL_SQL =  (const char*)
"SELECT NVL(COUNT(*), 0) FROM DL_PRINCIPALS WHERE ADDRESS=:ADDRESS";
const char* ADD_PRINCIPAL_SQL   =  (const char*)
"INSERT INTO DL_PRINCIPALS (ADDRESS, MAX_LST, MAX_EL) "
"VALUES (:ADDRESS, :MAX_LST, :MAX_EL)";

void DistrListManager::addPrincipal(const Principal& prc)
    throw(SQLException, PrincipalAlreadyExistsException)
{
    const char* prcAddressStr = prc.address.c_str();
    logger.debug("DistrListManager: addPrincipal() called. Addr:'%s' maxLst=%d, maxEl=%d",
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
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}
 
const char* CHECK_MEMBER_SQL = (const char*)
"SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=:LIST AND ADDRESS=:ADDRESS";
const char* COUNT_MEMBERS_SQL = (const char*)
"SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=:LIST";
const char* ADD_MEMBER_SQL   = (const char*)
"INSERT INTO DL_MEMBERS (LIST, ADDRESS) VALUES (:LIST, :ADDRESS)";

void DistrListManager::addMember(string dlName, const Address& member) 
    throw(SQLException, ListNotExistsException, 
          MemberAlreadyExistsException, MemberCountExceededException)
{
    const char* dlNameStr = dlName.c_str();
    const char* memberStr = member.toString().c_str();
    logger.debug("DistrListManager: addMember() called. dlName:'%s' member:'%s'",
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
        
        checkMember->setString(1, memberStr);
        checkMember->setString(2, dlNameStr);
        
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
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

const char* DELETE_MEMBER_SQL =
"DELETE FROM DL_MEMBERS WHERE LIST=:LIST AND ADDRESS=:ADDRESS";

void DistrListManager::deleteMember(string dlName, const Address& member) 
    throw(SQLException, ListNotExistsException, MemberNotExistsException)
{
    const char* dlNameStr = dlName.c_str();
    const char* memberStr = member.toString().c_str();
    logger.debug("DistrListManager: deleteMember() called. dlName:'%s' member:'%s'",
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
        
        std::auto_ptr<Statement> checkMemberGuard(connection->createStatement(CHECK_MEMBER_SQL));
        Statement* checkMember = checkMemberGuard.get();
        if (!checkMember)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);
        
        checkMember->setString(1, memberStr);
        checkMember->setString(2, dlNameStr);
        
        std::auto_ptr<ResultSet> checkMemberRsGuard(checkMember->executeQuery());
        ResultSet* checkMemberRs = checkMemberRsGuard.get();
        if (!checkMemberRs || !checkMemberRs->fetchNext()) 
            throw SQLException(FAILED_TO_OBTAIN_RESULTSET);
        if (!checkMemberRs->getUint32(1))
            throw MemberNotExistsException("Member '%s' not exists in DL '%s'",
                                           memberStr, dlNameStr);

        std::auto_ptr<Statement> deleteMemberGuard(connection->createStatement(DELETE_MEMBER_SQL));
        Statement* deleteMember = deleteMemberGuard.get();
        if (!deleteMember)        
            throw SQLException(FAILED_TO_CREATE_STATEMENT);

        deleteMember->setString(1, dlNameStr);
        deleteMember->setString(2, memberStr);
        
        deleteMember->executeUpdate();
        connection->commit();
    }
    catch(Exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

void DistrListManager::deleteMembers(string dlName) 
    throw(SQLException, ListNotExistsException)
{
    const char* dlNameStr = dlName.c_str();
    logger.debug("DistrListManager: deleteMembers() called. dlName:'%s'", dlNameStr);
    
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
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

const char* ADD_SUBMITTER_SQL = 
"INSERT INTO DL_SUBMITTERS (LIST, ADDRESS) VALUES (:LIST, :ADDRESS)";

void DistrListManager::grantPosting(string dlName, const Address& submitter) 
    throw(SQLException, ListNotExistsException, 
          PrincipalNotExistsException, SubmitterAlreadyExistsException)
{
    const char* dlNameStr = dlName.c_str();
    const char* submitterStr = submitter.toString().c_str();
    logger.debug("DistrListManager: grantPosting() called. dlName:'%s', submitter: '%s'",
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
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

const char* DELETE_SUBMITTER_SQL =
"DELETE FROM DL_SUBMITTERS WHERE LIST=:LIST AND ADDRESS=:ADDRESS";

void DistrListManager::revokePosting(string dlName, const Address& submitter)
    throw(SQLException, ListNotExistsException, SubmitterNotExistsException)
{
    const char* dlNameStr = dlName.c_str();
    const char* submitterStr = submitter.toString().c_str();
    logger.debug("DistrListManager: revokePosting() called. dlName:'%s', submitter: '%s'",
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
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(std::exception& exc) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        logger.error("%s", exc.what());
        throw;
    }
    catch(...) {
        try { if (connection) connection->rollback(); }
        catch (...) { logger.error(FAILED_TO_ROLLBACK_TRANSACTION); }
        if (connection) ds.freeConnection(connection);
        Exception exc("... exception handled");
        logger.error("%s", exc.what());
        throw exc;
    }
    
    if (connection) ds.freeConnection(connection);
}

}}

