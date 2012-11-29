#include "Session.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace smpp {

smsc::logger::Logger* Session::log_ = 0;
smsc::core::synchronization::AtomicCounter<unsigned> Session::total_;

namespace {
Mutex logMutex;
}

void Session::initLog()
{
    MutexGuard mg(logMutex);
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("smpp.sess");
    }
}

Session::Session( const char* sessionId ) :
ref_(0), sessionId_(sessionId) 
{
    if (!log_) {
        initLog();
    }
    const unsigned total = total_.inc();
    smsc_log_debug(log_,"ctor S'%s' @%p total=%u",
                   sessionId_.c_str(),this,total);
}


Session::~Session()
{
    const unsigned total = total_.dec();
    smsc_log_debug(log_,"dtor S'%s' @%p total=%u",
                   sessionId_.c_str(),this,total);
}

}
}
