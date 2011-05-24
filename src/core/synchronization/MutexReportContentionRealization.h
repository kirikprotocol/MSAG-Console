/// NOTE: this file is intended to be included in main()
/// Do not use it otherwise.

#ifdef CHECKCONTENTION

#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace smsc {
namespace core {
namespace synchronization {
unsigned Mutex::contentionLimit = 100000;

void Mutex::reportContention( const char* from,
                              unsigned    howlong,
                              unsigned    oldcount ) const throw()
{
    static smsc::logger::Logger* log_ = 0;
    if ( !log_ ) {
        log_ = smsc::logger::Logger::getInstance("lock");
    }
    smsc_log_debug(log_,"%s(%p) @%s contented for %u usec by %u@%s for count=%u/%+d",
                   what, this, from ? from:"unk",
                   howlong,
                   unsigned(wasid),
                   wasfrom?wasfrom:"unk",
                   lockCount,
                   int(lockCount-oldcount));
}

}
}
}
#endif
