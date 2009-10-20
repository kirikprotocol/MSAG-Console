#include "core/synchronization/Mutex.hpp"
#include "Message.h"

using namespace smsc::core::synchronization;

namespace {
Mutex messageIdLock;
}

namespace scag2 {
namespace prototypes {
namespace infosme {

unsigned Message::getNextId() {
    MutexGuard mg(messageIdLock);
    static unsigned id = 0;
    return id++;
}

}
}
}
