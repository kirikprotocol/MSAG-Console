
#include "DistrListManager.h"

namespace smsc { namespace distrlist
{
    using std::string;

    using namespace smsc::db;
    using namespace smsc::sms;
    using namespace core::buffers;
    
log4cpp::Category& DistrListManager::log = 
    Logger::getCategory("smsc.distribution.DistrListManager");

const char* DistrListManager::selectMembersSql = (const char*)
"SELECT ADDRESS FROM DL_MEMBERS WHERE LIST=:LIST";
const char* DistrListManager::checkSubmitterSql = (const char*)
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

Array<Address> DistrListManager::members(
    string dlName, const Address& submitter)
        throw(SQLException, IllegalSubmitterException)
{
    Array<Address> members(0);
    FullAddressValue submitterStr;
    submitter.toString(submitterStr, sizeof(submitterStr));
    const char* dlNameStr = dlName.c_str();
    __trace2__("DistrListManager: members() called for dl '%s', "
               "submitter '%s'", dlNameStr, submitterStr);
    
    Connection* connection = ds.getConnection();
    if (!connection) 
        throw SQLException("Failed to obtain connection to DB");
    
    Statement* stmt = 0; ResultSet* rs = 0;
    try 
    {
        stmt = connection->createStatement(checkSubmitterSql);
        if (!stmt) throw SQLException("Failed to create DataSource statement!");

        stmt->setString(1, dlNameStr);
        stmt->setString(2, submitterStr);
        rs = stmt->executeQuery();
        if (!rs || !rs->fetchNext()) 
            throw SQLException("Failed to create DataSource result set!");
        if (!rs->getUint32(1))
            throw IllegalSubmitterException(
                "Submitter '%s' undefined for dl '%s'",
                submitterStr, dlNameStr);
        delete rs; rs = 0;
        delete stmt; stmt = 0;
        
        stmt = connection->createStatement(selectMembersSql);
        if (!stmt) throw SQLException("Failed to create DataSource statement!");
        stmt->setString(1, dlNameStr);
        rs = stmt->executeQuery();
        if (!rs) 
            throw SQLException("Failed to create DataSource result set!");
        while (rs->fetchNext())
        {
            const char* addr = rs->getString(1);
            if (!addr || addr[0] == '\0') continue;
            
            Address address(addr);
            members.Push(address);
        }
        delete rs; rs = 0;
        delete stmt; stmt = 0;

        ds.freeConnection(connection);
    }
    catch(Exception& exc)
    {
        if (rs) delete rs;
        if (stmt) delete stmt;
        ds.freeConnection(connection);
        log.error(exc.what());
        throw;
    }

    return members;
}
 

}}

