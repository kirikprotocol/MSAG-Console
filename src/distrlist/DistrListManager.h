
#ifndef SMSC_DISTRIBUTION_LIST_MANAGER
#define SMSC_DISTRIBUTION_LIST_MANAGER

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include <logger/Logger.h>
#include <util/config/Manager.h>

#include <db/DataSource.h>

#include "DistrListAdmin.h"

namespace smsc { namespace distrlist
{
    using namespace smsc::db;
    
    using smsc::util::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;
    
    using namespace core::synchronization;
    
    class DistrListManager : public DistrListAdmin
    {
    private:

        static const char* selectMembersSql;
        static const char* checkSubmitterSql;

    protected:
        
        static log4cpp::Category&   log;
        
        DataSource&     ds;
        
    public:
        
        DistrListManager(DataSource& ds, Manager& config)
            throw(ConfigException);
        virtual ~DistrListManager();

        virtual Array<Address> members(
            string dlName, const Address& submitter)
                throw(SQLException, IllegalSubmitterException);
    };

}}

#endif //SMSC_DISTRIBUTION_LIST_MANAGER

