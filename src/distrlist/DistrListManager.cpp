
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

const char* SELECT_MEMBERS_SQL  = (const char*)
"SELECT ADDRESS FROM DL_MEMBERS WHERE LIST=:LIST";
const char* CHECK_SUBMITTER_SQL = (const char*)
"SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS\
 WHERE LIST=:LIST AND ADDRESS=:ADDRESS";

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

Array<Address> DistrListManager::members(string dlName, const Address& submitter)
    throw(SQLException, IllegalSubmitterException)
{
    Array<Address> members(0);
    FullAddressValue submitterStr;
    submitter.toString(submitterStr, sizeof(submitterStr));
    const char* dlNameStr = dlName.c_str();
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
 
const char* CHECK_DL_SQL = 
"SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE LIST=:LIST";
const char* ADD_DL_SQL = 
"INSERT INTO DL_SET (LIST, MAX_EL, OWNER) VALUES (:LIST, :MAX_EL, :OWNER)";

const char* CHECK_MEMBER_SQL = (const char*)
"SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=:LIST AND ADDRESS=:ADDRESS";
const char* ADD_MEMBER_SQL   = (const char*)
"INSERT INTO DL_MEMBERS (LIST, ADDRESS) VALUES (:LIST, :ADDRESS)";

void DistrListManager::addMember(string dlName, const Address& member) 
    throw(SQLException, ListNotExistsException, MemberAlreadyExistsException)
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
        if (checkMemberRs->getUint32(1))
            throw MemberAlreadyExistsException("Member '%s' already exists in DL '%s'",
                                               memberStr, dlNameStr);

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

const char* DELETE_MEMBERS_SQL =
"DELETE FROM DL_MEMBERS WHERE LIST=:LIST";

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

}}

