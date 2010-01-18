#include "Counter.h"

namespace {
smsc::core::synchronization::Mutex logMutex;
}

namespace scag2 {
namespace counter {

smsc::logger::Logger* Counter::log_ = 0;
smsc::logger::Logger* Counter::loga_ = 0;

Counter::Counter( const std::string& name,
                  counttime_type     disposeDelayTime,
                  Observer*          observer ) :
disposer_(0), observer_(observer),
name_(name),
disposeDelayTime_(disposeDelayTime),
usage_(0)
{
    if (!log_) {
        MutexGuard mg(logMutex);
        if (!log_) log_ = smsc::logger::Logger::getInstance("cnt.cntr");
        if (!loga_) loga_ = smsc::logger::Logger::getInstance("cnt.alloc");
    }
    smsc_log_debug(loga_,"ctor: '%s', delay=%d", name_.c_str(), int(disposeDelayTime));
}

Counter::~Counter()
{
    smsc_log_debug(loga_,"dtor: '%s', usage=%u",name_.c_str(),usage_);
}

}
}
