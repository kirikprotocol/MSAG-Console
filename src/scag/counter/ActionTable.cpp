#include "ActionTable.h"

namespace {
smsc::core::synchronization::Mutex logMutex;
}

namespace scag2 {
namespace counter {

smsc::logger::Logger* ActionTable::log_ = 0;

ActionTable::ActionTable() :
ref_(0)
{
    if (!log_) {
        MutexGuard mg(logMutex);
        if (!log_) log_ = smsc::logger::Logger::getInstance("cnt.atbl");
    }
    smsc_log_debug(log_,"ctor %p",this);
}


ActionTable::~ActionTable()
{
    smsc_log_debug(log_,"dtor %p",this);
}


void ActionTable::modified( Counter& counter, int64_t value )
{
    smsc_log_debug(log_,"%p: counter '%s' modified, new val=%llu",this,counter.getName().c_str(),value);
}


void ActionTable::ref(bool add)
{
    bool destroy;
    {
        MutexGuard mg(lock_);
        if (add) {
            ++ref_;
            smsc_log_debug(log_,"%p: ref +1 => %u",this,ref_);
            return;
        }
        destroy = !--ref_;
        smsc_log_debug(log_,"%p: ref -1 => %u%s",this,ref_,destroy?" (need dtor)":"");
    }
    if (destroy) delete this;
}

}
}
