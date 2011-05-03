#include "CommandResponse.h"
#include "core/synchronization/Mutex.hpp"

namespace {
smsc::core::synchronization::Mutex logMtx;
}


namespace scag2 {
namespace pvss {

smsc::logger::Logger* CommandResponse::log_ = 0;

void CommandResponse::initLog()
{
    if (!log_) {
        smsc::core::synchronization::MutexGuard mg(::logMtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("pvss.cmd");
    }
    smsc_log_debug(log_,"ctor: %p %s",this,this->toString().c_str());
}

void CommandResponse::logDtor()
{
    smsc_log_debug(log_,"dtor: %p %s",this,this->toString().c_str());
}

} // namespace pvss
} // namespace scag2
