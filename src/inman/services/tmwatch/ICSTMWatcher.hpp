/* ************************************************************************** *
 * TimeWatchers service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_TIME_WATCHER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TIME_WATCHER_HPP

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/tmwatch/TimeWatcher.hpp"

namespace smsc {
namespace inman {

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::TimeSlice;

using smsc::core::timers::TimeWatchersRegistry;
using smsc::core::timers::TimeWatchersRegistryITF;
using smsc::core::timers::TimeWatcher;
using smsc::core::timers::TimeWatcherTMO;

//TimeWatchers service.
class ICSTMWatcher : public ICServiceAC, public TimeWatchersRegistryITF {
protected:
    mutable Mutex   _sync;
    std::auto_ptr<TimeWatchersRegistry> twReg;

    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void) { return ICServiceAC::icsRcOk; }
    //Starts service verifying that all dependent services are started
    RCode _icsStart(void)
    {
        return twReg->StartAll() ? ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
    }
    //Stops service
    void  _icsStop(bool do_wait = false)
    {
        twReg->StopAll(do_wait);
    }

public:
    ICSTMWatcher(const ICServicesHostITF * svc_host, Logger * use_log)
        : ICServiceAC(ICSIdent::icsIdTimeWatcher, svc_host, use_log)
        , twReg(new TimeWatchersRegistry(use_log))
    {
        _icsState = ICServiceAC::icsStConfig;
    }
    ~ICSTMWatcher()
    {
        ICSStop(true);
    }

    //Returns TimeWatchersRegistryITF
    void * Interface(void) const
    {
        return (TimeWatchersRegistryITF*)this;
    }

    // -------------------------------------
    // TimeWatchersRegistryITF interface methods:
    // -------------------------------------
    //Returns generic TimeWatcher (optionally started)
    TimeWatcher *
        getTimeWatcher(uint32_t num_tmrs = 0, bool do_start = true)
    {
        MutexGuard grd(_sync);
        return twReg->getTimeWatcher(num_tmrs, do_start);
    }
    //Returns TimeWatcherTMO responsible for given timeout (optionally started)
    TimeWatcherTMO *
        getTmoTimeWatcher(const TimeSlice & use_tmo,
                          uint32_t num_tmrs = 0, bool do_start = true)
    {
        MutexGuard grd(_sync);
        return twReg->getTmoTimeWatcher(use_tmo, num_tmrs, do_start);
    }
};

} //inman
} //smsc
#endif /* __INMAN_ICS_TIME_WATCHER_HPP */

