#include "ProfileLogRollerHardcoded.h"
#include "scag/pvss/profile/RollingFileStream.h"
#include "util/config/Config.h"
#include "informer/io/ConfigWrapper.h"

namespace scag2 {
namespace pvss {

ProfileLogRollerHardcoded::ProfileLogRollerHardcoded( bool backupMode ) :
ProfileLogRoller(),
lastConfigTime_(0), oldmtime_(0),
backupMode_(backupMode) {}
    
bool ProfileLogRollerHardcoded::readConfiguration()
{
    using namespace smsc::util::config;
    using smsc::logger::Logger;
    using eyeline::informer::EmbedRefPtr;

    std::auto_ptr<Config> cfg1;

    time_t now = time(0);
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        do {
            if ( lastConfigTime_ == 0 ) break;
            // consecutive (not the first) attempt
            if ( configReloadInterval_ == 0 || backupMode_ ) return false;
            if ( time_t(lastConfigTime_ + configReloadInterval_) > now ) return false;
        } while (false);
        lastConfigTime_ = now;
    }

    try {
        cfg1.reset( Config::createFromFile("config.xml",&oldmtime_) );
        if ( !cfg1.get() ) {
            // config has not been changed
            return false;
        }
        cfg1.reset( cfg1->getSubConfig("PVSS.backup",true) );
        if ( !cfg1.get() ) {
            // a section is not found
            throw smsc::util::Exception("section 'PVSS.backup' is not found");
        }
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"config exc: %s",e.what());
        throw;
    }

    eyeline::informer::ConfigWrapper cwrap1(*cfg1,log_);
    try {

        // 1. reading common params
        const unsigned reloadInterval = backupMode_ ? 0 :
            cwrap1.getInt("configReloadInterval",60,0,10000);
        if ( reloadInterval > 0 && reloadInterval < 30 ) {
            throw smsc::util::Exception("configReloadInterval is too small (%u)",reloadInterval);
        }
        const std::string prefix = cwrap1.getString("backupPrefix");
        const std::string finalSuffix = cwrap1.getString("finalSuffix",".pvss");
        if ( finalSuffix.empty() || finalSuffix == ".log" ) {
            throw smsc::util::Exception("finalSuffix is not correct ('%s')",finalSuffix.c_str());
        }
        const unsigned rollingInterval = backupMode_ ? 300 :
            cwrap1.getInt("rollingInterval",300,60,3600);

        // adding streams
        const char* streamName = "pvss.bks";
        EmbedRefPtr< ProfileLogStream > pls;
        if ( !backupMode_ ) {
            pls.reset( new RollingFileStream( streamName,
                                              prefix.c_str(),
                                              finalSuffix.c_str(), 
                                              rollingInterval) );
            addLogStream(pls);
            if ( !getLogStream(streamName,pls) ) {
                throw smsc::util::Exception("cannot find log stream '%s'",streamName);
            }
        }

        // adding loggers
        addLogger("pvss.abnt",
                  backupMode_ ? Logger::LEVEL_FATAL : Logger::LEVEL_INFO,
                  pls, true);
        addLogger("pvss.oper",
                  backupMode_ ? Logger::LEVEL_FATAL : Logger::LEVEL_INFO,
                  pls, true);
        addLogger("pvss.serv",
                  backupMode_ ? Logger::LEVEL_FATAL : Logger::LEVEL_INFO,
                  pls, true);
        addLogger("pvss.prov",
                  backupMode_ ? Logger::LEVEL_FATAL : Logger::LEVEL_INFO,
                  pls, true);

        configReloadInterval_ = reloadInterval;

    } catch ( std::exception& e ) {
        smsc_log_error(log_,"config exc: %s", e.what());
        throw;
    }
    return true;
}

}
}
