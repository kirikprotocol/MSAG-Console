#include "TaskScheduler.h"

namespace smsc {
namespace infosme2 {

TaskScheduler::TaskScheduler( TaskProcessorAdapter& adapter ) :
log_(smsc::logger::Logger::getInstance("is2.sched")),
processor_(&adapter),
started_(false),
stopping_(false)
{
    smsc_log_debug(log_,"ctor");
}


TaskScheduler::~TaskScheduler()
{
    stop();
    smsc_log_debug(log_,"dtor");
}


void TaskScheduler::init( smsc::util::config::ConfigView* config )
{
    smsc_log_info(log_,"init");
    if ( !config ) throw smsc::util::config::ConfigException("no config");
    MutexGuard mg(startMon_);
    if ( started_ ) throw smsc::util::config::ConfigException("cannot init while started");
    // FIXME: clean internal state
    // FIXME: load schedules
    smsc_log_info(log_,"init finished");
}


void TaskScheduler::start()
{
    MutexGuard mg(startMon_);
    if ( started_ ) return;
    smsc_log_debug(log_,"starting task scheduler");
    stopping_ = false;
    Start();
    started_ = true;
}


void TaskScheduler::stop()
{
    MutexGuard mg(startMon_);
    if (!started_) return;
    smsc_log_debug(log_,"stopping task scheduler");
    stopping_ = true;
    startMon_.notifyAll();
    while ( started_ ) {
        startMon_.wait();
    }
    WaitFor();
}


int TaskScheduler::Execute()
{
    smsc_log_info(log_,"execute started");
    try {
        while ( true ) {
            {
                MutexGuard mg(startMon_);
                if ( stopping_ ) break;
                // FIXME: sleeping
            }
        }
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"exc in execute, stopping: %s", e.what());
    }
    MutexGuard mg(startMon_);
    started_ = false;
    startMon_.notifyAll();
    smsc_log_info(log_,"execute finished");
    return 0;
}


}
}
