#include "Packet.h"
#include "core/synchronization/Mutex.hpp"

namespace {
smsc::core::synchronization::Mutex logMtx;

}

namespace scag2 {
namespace pvss {

smsc::logger::Logger* Packet::log_ = 0;
smsc::logger::Logger* Packet::logtm_ = 0;

void Packet::initLog()
{
    if (!log_) {
        smsc::core::synchronization::MutexGuard mg(::logMtx);
        if (!log_) {
            log_ = smsc::logger::Logger::getInstance("pvss.pck");
            logtm_ = smsc::logger::Logger::getInstance("pvss.pcktm");
        }
    }
    smsc_log_debug(log_,"ctor: %p %s",this,this->toString().c_str());
}

void Packet::logDtor()
{
    smsc_log_debug(log_,"dtor: %p %s",this,this->toString().c_str());
}

} // namespace pvss
} // namespace scag2
