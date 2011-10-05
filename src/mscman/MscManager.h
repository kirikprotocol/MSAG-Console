#ifndef SMSC_MSCMAN_MSC_MANAGER
#define SMSC_MSCMAN_MSC_MANAGER

#include <vector>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include "MscExceptions.h"
#include "MscStatus.h"
#include "MscAdmin.h"

namespace smsc {
namespace mscman {

    using smsc::logger::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;

    using namespace core::synchronization;

    class MscManager : public MscStatus, public MscAdmin
    {
    protected:

        static Mutex                startupLock;
        static MscManager*          instance;
        static smsc::logger::Logger *log;

        bool                automaticRegistration;
        uint32_t            failureLimit;
        time_t              singleAttemptTimeout;

        MscManager(Manager& config)
            throw(ConfigException);

    public:

        virtual ~MscManager();

        static void startup(Manager& config)
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

}//mscman
}//smsc

#endif //SMSC_MSCMAN_MSC_MANAGER
