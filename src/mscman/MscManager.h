
#ifndef SMSC_MSCMAN_MSC_MANAGER
#define SMSC_MSCMAN_MSC_MANAGER

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include <util/Logger.h>
#include <util/config/Manager.h>

#include <db/DataSource.h>

#include "MscExceptions.h"
#include "MscStatus.h"
#include "MscAdmin.h"

namespace smsc { namespace mscman 
{
    using smsc::db::DataSource;

    using smsc::util::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;
    
    using namespace core::synchronization;
    
    class MscManager : public MscStatus, public MscAdmin
    {
    protected:

        static Mutex                startupLock;
        static MscManager*          instance;
        static log4cpp::Category&   log;
        
        DataSource&         ds;
        bool                automaticRegistration;
        int                 failureLimit;
        
        MscManager(DataSource& ds, Manager& config)
            throw(ConfigException);
    
    public:
        
        virtual ~MscManager();

        static void startup(DataSource& ds, Manager& config)
            throw(ConfigException, InitException); 
        static void shutdown();
        
        static MscManager& getInstance() 
            throw(InitException);

        static MscStatus& getMscStatus() throw(InitException) {
            return getInstance();
        };
        static MscAdmin& getMscAdmin() throw(InitException) {
            return getInstance();
        };
    };

}}

#endif //SMSC_MSCMAN_MSC_MANAGER

