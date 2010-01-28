#include "Counter.h"
#include "Manager.h"

namespace {
smsc::core::synchronization::Mutex logMutex;
}

namespace scag2 {
namespace counter {

smsc::logger::Logger* Counter::log_ = 0;
smsc::logger::Logger* Counter::loga_ = 0;

Counter::Counter( const std::string& name,
                  Observer*          observer,
                  counttime_type     disposeDelayTime ) :
observer_(observer),
oldsev_(NORMAL),
maxval_(0),
name_(name),
disposeDelayTime_(disposeDelayTime),
usage_(0)
{
    if (!log_) {
        MutexGuard mg(logMutex);
        if (!log_) log_ = smsc::logger::Logger::getInstance("cnt.cntr");
        if (!loga_) loga_ = smsc::logger::Logger::getInstance("cnt.alloc");
    }
    // if (observer_) observer_->ref(true);
    // smsc_log_debug(loga_,"ctor: '%s', delay=%d", name_.c_str(), int(disposeDelayTime));
}

Counter::~Counter()
{
    // if (observer_) observer_->ref(false);
    // smsc_log_debug(loga_,"dtor: '%s', usage=%u",name_.c_str(),usage_);
}


void Counter::scheduleDisposal()
{
    register Manager& mgr = Manager::getInstance();
    disposeTime_ = mgr.getWakeTime() + disposeDelayTime_;
    smsc_log_debug(loga_,"'%s': scheduling disposal at %d",name_.c_str(),int(disposeTime_));
    mgr.scheduleDisposal(*this);
}

}
}
