
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
    __trace2__("DistrListManager: members called for list '%s'", 
               dlName.c_str());

    Array<Address> members;
    return members;
}
 

}}

