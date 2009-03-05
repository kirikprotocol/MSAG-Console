#include "Response.h"

namespace {
smsc::core::synchronization::Mutex logMtx;
}


namespace scag2 {
namespace pvss {

smsc::logger::Logger* Response::log_ = 0;

void Response::initLogger()
{
    if (!log_) {
        MutexGuard mg(logMtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("pvss.resp");
    }
    smsc_log_debug(log_,"ctor: %p",this);
}

Response::~Response()
{
    smsc_log_debug(log_,"dtor: %p",this);
}

} // namespace pvss
} // namespace scag2
