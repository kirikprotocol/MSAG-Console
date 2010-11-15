/* ************************************************************************** *
 * Signalling Task Sheduler service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_SCHEDULER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_SCHEDULER_HPP

#include "inman/common/ObjRegistryT.hpp"

#include "inman/services/ICSrvDefs.hpp"
#include "inman/services/scheduler/TaskScheduler.hpp"

namespace smsc {
namespace inman {

using smsc::util::POBJRegistry_T;

using smsc::util::TaskSchedulerITF;
using smsc::util::TaskSchedulerAC;
using smsc::util::TaskSchedulerMT;
using smsc::util::TaskSchedulerSEQ;
using smsc::util::TaskSchedulerFactoryITF;


//Signalling Task Sheduler service.
class ICSTaskScheduler : public ICServiceAC, public TaskSchedulerFactoryITF {
protected:
    typedef POBJRegistry_T<TaskSchedulerITF::SchedulerType,
                                            TaskSchedulerAC> TSchedRegistry;
    mutable Mutex   _sync;
    TSchedRegistry  tschedReg;

    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void) { return ICServiceAC::icsRcOk; }
    //Starts service verifying that all dependent services are started
    RCode _icsStart(void)  { return ICServiceAC::icsRcOk; }
    //Stops service
    void  _icsStop(bool do_wait = false)
    {
        unsigned i = 0, iMax = do_wait ? 2 : 1;
        do {
            TSchedRegistry::const_iterator it = tschedReg.begin();
            for (; it != tschedReg.end(); ++it)
                it->second->Stop(!i ? 0 : TaskSchedulerAC::_SHUTDOWN_TIMEOUT);
        } while ((++i) < iMax);
    }

public:
    ICSTaskScheduler(const ICServicesHostITF * svc_host, Logger * use_log)
        : ICServiceAC(ICSIdent::icsIdScheduler, svc_host, use_log)
    {
        _icsState = ICServiceAC::icsStConfig;
    }
    ~ICSTaskScheduler()
    {
        ICSStop(true);
    }

    //Returns TaskSchedulerFactoryITF
    void * Interface(void) const
    {
        return (TaskSchedulerFactoryITF*)this;
    }

    // ------------------------------------------
    // TaskSchedulerFactoryITF interface methods:
    // ------------------------------------------
    //Returns started scheduler
    TaskSchedulerITF * getScheduler(TaskSchedulerITF::SchedulerType sched_type)
    {
        {
            MutexGuard grd(_sync);
            TaskSchedulerAC * pSched = tschedReg.find(sched_type);
            if (pSched)
                return pSched;
        }
        //create new scheduler
        std::auto_ptr<TaskSchedulerAC> pShed;
        if (sched_type == TaskSchedulerITF::schedMT)
            pShed.reset(new TaskSchedulerMT(logger));
        else
            pShed.reset(new TaskSchedulerSEQ(logger));

        if (pShed->Start()) {
            MutexGuard grd(_sync);
            tschedReg.insert(sched_type, pShed.get());
            return pShed.release();
        }
        smsc_log_error(logger, "Sched: failed to start %s ..", pShed->Name());
        return NULL;
    }
};

} //inman
} //smsc
#endif /* __INMAN_ICS_SCHEDULER_HPP */

