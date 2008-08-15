#include "TransportRule.h"
#include "core/synchronization/Mutex.hpp"

using namespace smsc::core::synchronization;

namespace {
Mutex loggerMutex;
}

namespace scag2 {
namespace re {

smsc::logger::Logger* TransportRule::log_ = 0;

TransportRule::TransportRule()
{
    if ( !log_ ) {
        MutexGuard mg(loggerMutex);
        if ( ! log_ ) log_ = smsc::logger::Logger::getInstance("re.trule");
    }
}

} // namespace re
} // namespace scag2
